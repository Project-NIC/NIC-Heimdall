/* The head's two cards on a PC: three addresses written to two simulated cards of 24 MB with
 * 3 MB kept free, the writer handing each segment to every card that is in; the circle keeping the
 * reserve on both, oldest first by first second, never an open file, the unheld ones counted; a
 * card let out while the station records and the circle turns, put back and brought level while
 * the writer goes on — the open files' gaps copied first, the files the circle took deleted from
 * it — then every file on both cards the writer's bytes; an empty card filled the same way; a card
 * pulled with its last entry torn, made whole. */
#include "../nic_cards.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "hmc.h"

static int fails;
#define CHECK(x) do { if (!(x)) { fails++; fprintf(stderr, "%s:%d: %s\n", __FILE__, __LINE__, #x); } } while (0)

#define CAP (24u << 20)

/* ------------------------------------------------------------------ the cards */

typedef struct { int used; uint8_t addr; uint16_t seq; uint8_t *img; size_t size; } cfile;
static cfile card[2][64];

static cfile *cfind(int k, uint8_t addr, uint16_t seq)
{
    for (int i = 0; i < 64; i++)
        if (card[k][i].used && card[k][i].addr == addr && card[k][i].seq == seq)
            return &card[k][i];
    return 0;
}

static uint64_t io_free(void *ctx, int k)
{
    (void)ctx;
    uint64_t used = 0;
    for (int i = 0; i < 64; i++)
        if (card[k][i].used) used += card[k][i].size;
    return CAP - used;
}

static int io_read(void *ctx, int k, uint8_t addr, uint16_t seq, uint32_t off, uint32_t len, uint8_t *buf)
{
    (void)ctx;
    cfile *f = cfind(k, addr, seq);
    if (!f || (size_t)off + len > f->size) return -1;
    memcpy(buf, f->img + off, len);
    return 0;
}

static int in_step, sync_writes, sync_first_live = -1;
static uint16_t live_of[256];

static int io_write(void *ctx, int k, uint8_t addr, uint16_t seq, uint32_t off, uint32_t len, const uint8_t *buf)
{
    (void)ctx;
    cfile *f = cfind(k, addr, seq);
    if (!f || (size_t)off + len > f->size) return -1;
    memcpy(f->img + off, buf, len);
    if (in_step && sync_writes++ == 0)
        sync_first_live = seq == live_of[addr];
    return 0;
}

static int io_create(void *ctx, int k, const nic_card_file *nf)
{
    (void)ctx;
    for (int i = 0; i < 64; i++)
        if (!card[k][i].used) {
            card[k][i] = (cfile){1, nf->addr, nf->seq, calloc(1, (size_t)HMC_MB << nf->size), (size_t)HMC_MB << nf->size};
            return 0;
        }
    return -1;
}

static int io_remove(void *ctx, int k, uint8_t addr, uint16_t seq)
{
    (void)ctx;
    cfile *f = cfind(k, addr, seq);
    if (!f) return -1;
    free(f->img);
    f->used = 0;
    return 0;
}

static int io_span(void *ctx, int k, uint8_t addr, uint16_t *first, uint16_t *live)
{
    (void)ctx;
    int any = 0;
    for (int i = 0; i < 64; i++)
        if (card[k][i].used && card[k][i].addr == addr) {
            if (!any || card[k][i].seq < *first) *first = card[k][i].seq;
            if (!any || card[k][i].seq > *live) *live = card[k][i].seq;
            any = 1;
        }
    return any ? 0 : -1;
}

static int io_held(void *ctx, uint8_t addr, uint16_t seq) { (void)ctx; (void)addr; return seq % 2; }

static uint32_t gone_first[256];
static uint16_t first_of[256];
static int ngone, gone_open, unheld, levels, level_card = -1;
static void io_gone(void *ctx, uint8_t addr, uint16_t seq, int held);
static void io_level(void *ctx, int k) { (void)ctx; levels++; level_card = k; }

static const nic_card_io IO = {io_free, io_read, io_write, io_create, io_remove, io_span, io_held, io_gone, io_level, 0};
static nic_cards c;

/* ------------------------------------------------------------------ the writer */

typedef struct { int used; uint8_t addr; uint16_t seq; hmc_writer w; size_t end, entries; } master;
static master m[64];
static uint32_t clock_s = 1790856000u;

static master *mfind(uint8_t addr, uint16_t seq)
{
    for (int i = 0; i < 64; i++)
        if (m[i].used && m[i].addr == addr && m[i].seq == seq)
            return &m[i];
    return 0;
}

static void io_gone(void *ctx, uint8_t addr, uint16_t seq, int held)
{
    (void)ctx;
    master *x = mfind(addr, seq);
    gone_first[ngone % 256] = x ? (uint32_t)(x->w.image[12] | x->w.image[13] << 8 | x->w.image[14] << 16 | (uint32_t)x->w.image[15] << 24) : 0;
    ngone++;
    if (seq == live_of[addr]) gone_open++;
    first_of[addr] = (uint16_t)(seq + 1);
    if (!held) unheld++;
    if (x) {
        hmc_writer_close(&x->w);
        x->used = 0;
    }
}


static void open_file(uint8_t addr, uint16_t seq)
{
    static uint8_t sec[8 + 11] = {HMC_UNIT, 0, 0, 0, 11, 0, 0, 0, 1, 5, 1, 1, 1, 128, 0, 0, 0, 0, 0};
    master *x = 0;
    for (int i = 0; i < 64 && !x; i++)
        if (!m[i].used) x = &m[i];
    hmc_header h = {addr, seq, clock_s, 0x123456, 0, 0, sec, sizeof sec};
    x->used = 1;
    x->addr = addr;
    x->seq = seq;
    CHECK(!hmc_writer_open(&x->w, &h));
    x->end = x->w.end;
    x->entries = 0;
    nic_card_file nf = {addr, seq, clock_s, 0};
    for (int k = 0; k < 2; k++)
        if (c.in[k]) {
            CHECK(!io_create(0, k, &nf));
            CHECK(!io_write(0, k, addr, seq, 0, (uint32_t)x->end, x->w.image));
        }
    live_of[addr] = seq;
    CHECK(!nic_cards_files(&c, addr, first_of[addr], seq));
}

/* a segment, written to every card in: the data, then the entry */
static void segment(uint8_t addr, size_t n)
{
    static hmc_event lines[1400];
    for (size_t i = 0; i < n; i++)
        lines[i] = (hmc_event){clock_s + (uint32_t)(i * 8 / n), addr, (uint8_t)i, (uint16_t)(i * 31), (uint32_t)(i * 2654435761u)};
    master *x = mfind(addr, live_of[addr]);
    int e = hmc_writer_events(&x->w, lines, n);
    if (e == HMC_FULL) {
        open_file(addr, (uint16_t)(live_of[addr] + 1));
        x = mfind(addr, live_of[addr]);
        e = hmc_writer_events(&x->w, lines, n);
    }
    CHECK(!e);
    clock_s += 8;
    for (int k = 0; k < 2; k++)
        if (c.in[k]) {
            CHECK(!io_write(0, k, addr, x->seq, (uint32_t)x->end, (uint32_t)(x->w.end - x->end), x->w.image + x->end));
            size_t at = x->w.size - 16 * x->w.entries;
            CHECK(!io_write(0, k, addr, x->seq, (uint32_t)at, (uint32_t)(16 * (x->w.entries - x->entries)), x->w.image + at));
        }
    x->end = x->w.end;
    x->entries = x->w.entries;
    CHECK(nic_cards_circle(&c) >= 0);
    for (int k = 0; k < 2; k++)
        CHECK(!c.in[k] || io_free(0, k) >= c.reserve);
}

static const uint8_t ADDRS[3] = {0x00, 0x52, 0x16};

static void record(int rounds)
{
    for (int r = 0; r < rounds; r++) {
        segment(0x52, 1300);
        segment(0x52, 1300);
        segment(0x16, 600);
        if (r % 4 == 0) segment(0x00, 50);
    }
}

/* every file of every address on both cards, the writer's bytes; nothing else on either */
static int level(void)
{
    int ok = 1;
    for (int i = 0; i < c.n; i++) {
        nic_card_addr *a = &c.a[i];
        for (uint16_t s = a->first;; s++) {
            master *x = mfind(a->addr, s);
            for (int k = 0; k < 2; k++) {
                cfile *f = cfind(k, a->addr, s);
                if (!x || !f || f->size != x->w.size || memcmp(f->img, x->w.image, f->size)) {
                    fprintf(stderr, "  card %d file %02X/%u differs\n", k, a->addr, s);
                    ok = 0;
                }
            }
            if (s == a->live) break;
        }
    }
    for (int k = 0; k < 2; k++)
        for (int j = 0; j < 64; j++)
            if (card[k][j].used) {
                nic_card_addr *a = 0;
                for (int i = 0; i < c.n; i++)
                    if (c.a[i].addr == card[k][j].addr) a = &c.a[i];
                uint16_t s = card[k][j].seq;
                if (!a || (uint16_t)(s - a->first) > (uint16_t)(a->live - a->first)) {
                    fprintf(stderr, "  card %d holds a stale %02X/%u\n", k, card[k][j].addr, s);
                    ok = 0;
                }
            }
    return ok;
}

static void bring_level(int k)
{
    levels = 0;
    sync_writes = 0;
    sync_first_live = -1;
    CHECK(!nic_cards_back(&c, k));
    int steps = 0;
    for (;;) {
        in_step = 1;
        int more = nic_cards_step(&c, 65536);
        in_step = 0;
        steps++;
        if (!more) break;
        if (steps % 3 == 0) record(1);               /* the station goes on recording */
    }
    CHECK(levels == 1 && level_card == k);
}

int main(void)
{
    nic_cards_init(&c, &IO, 1, 1);
    c.reserve = 3u << 20;
    for (int i = 0; i < 3; i++) {
        first_of[ADDRS[i]] = 0;
        open_file(ADDRS[i], 0);
    }

    /* the circle on both cards */
    record(1000);
    CHECK(ngone > 10 && !gone_open);
    for (int i = 1; i < ngone && i < 256; i++)
        CHECK(gone_first[i] >= gone_first[i - 1]);  /* the oldest first */
    CHECK(unheld > 0 && unheld < ngone && (uint32_t)unheld == c.gone_unheld);
    CHECK(level());

    /* card 1 let out; the station records, the circle turns on card 0 alone; put back */
    nic_cards_out(&c, 1);
    int before_gone = ngone;
    record(150);
    CHECK(ngone > before_gone);
    bring_level(1);
    CHECK(sync_first_live == 1);                     /* the open files' gaps first */
    CHECK(level());

    /* an empty card in slot 1 */
    nic_cards_out(&c, 1);
    for (int j = 0; j < 64; j++)
        if (card[1][j].used) { free(card[1][j].img); card[1][j].used = 0; }
    record(20);
    bring_level(1);
    CHECK(level());

    /* card 0 pulled with no command, its Quake file's last entry torn */
    nic_cards_out(&c, 0);
    {
        nic_card_addr *q = 0;
        for (int i = 0; i < c.n; i++) if (c.a[i].addr == 0x52) q = &c.a[i];
        master *x = mfind(0x52, q->live);
        cfile *f = cfind(0, 0x52, q->live);
        CHECK(x->w.entries > 0);
        memset(f->img + f->size - 16 * x->w.entries, 0, 16);
    }
    record(10);
    bring_level(0);
    CHECK(level());
    CHECK(c.copied > 0);

    for (int k = 0; k < 2; k++)
        for (int j = 0; j < 64; j++)
            if (card[k][j].used) free(card[k][j].img);
    for (int i = 0; i < 64; i++)
        if (m[i].used) hmc_writer_close(&m[i].w);
    if (fails) {
        printf("test_cards: %d FAILED\n", fails);
        return 1;
    }
    printf("test_cards: all passed (%d files taken by the circle, %u bytes copied)\n", ngone, c.copied);
    return 0;
}

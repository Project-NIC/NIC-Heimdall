/* The head's uplink on a PC: files of four addresses on the cards, the event log's among them,
 * sent from an empty cursor and rebuilt by a server to the cards' bytes; the event log first, the
 * others in turn; every packet within its budget; the live file followed as it grows and into the
 * next; a cursor in the middle of an older file, one before the cards' oldest and one past the
 * live file, each rewound as the session says, the server's files unchanged by a resend; the
 * sequence across its wrap; a damaged entry passed over; ACK and how far behind the server is;
 * WINDOW answered before the forward stream, for one address and for all, each file's cut the
 * archive's reference cut (ses_window_cut) entry for entry; the queue's bound. */
#include "../nic_uplink.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int fails;
#define CHECK(x) do { if (!(x)) { fails++; fprintf(stderr, "%s:%d: %s\n", __FILE__, __LINE__, #x); } } while (0)

static const uint8_t KEY[16] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16};
static const uint8_t NONCE[8] = {0xA1, 0xB2, 0xC3, 0xD4, 0xE5, 0xF6, 0x07, 0x18};
#define STATION 0x123456u

/* ------------------------------------------------------------------ the cards */

typedef struct { uint8_t addr; uint16_t seq; hmc_writer w; } card_file;
static card_file card[16];
static int ncard;
static uint32_t clock_s = 1790856000u;

static card_file *card_find(uint8_t addr, uint16_t seq)
{
    for (int i = 0; i < ncard; i++)
        if (card[i].addr == addr && card[i].seq == seq)
            return &card[i];
    return 0;
}

static void card_open(uint8_t addr, uint16_t seq)
{
    static uint8_t sec[8 + 11] = {HMC_UNIT, 0, 0, 0, 11, 0, 0, 0, 1, 5, 1, 1, 1, 128, 0, 0, 0, 0, 0};
    card_file *f = &card[ncard++];
    hmc_header h = {addr, seq, clock_s, STATION, 0, 0, sec, sizeof sec};
    f->addr = addr;
    f->seq = seq;
    CHECK(!hmc_writer_open(&f->w, &h));
}

/* a segment of n event lines over 8 s */
static void card_segment(uint8_t addr, uint16_t seq, size_t n)
{
    static hmc_event lines[1400];
    for (size_t i = 0; i < n; i++)
        lines[i] = (hmc_event){clock_s + (uint32_t)(i * 8 / n), addr, (uint8_t)(i * 7), (uint16_t)(i * 31),
                               (uint32_t)(i * 2654435761u)};
    clock_s += 8;
    CHECK(!hmc_writer_events(&card_find(addr, seq)->w, lines, n));
}

static size_t st_header(void *ctx, uint8_t addr, uint16_t seq, uint8_t *buf, size_t max)
{
    (void)ctx;
    card_file *f = card_find(addr, seq);
    if (!f)
        return 0;
    const uint8_t *im = f->w.image;
    size_t len = (size_t)im[8] | (size_t)im[9] << 8 | (size_t)im[10] << 16 | (size_t)im[11] << 24;
    if (len > max)
        return 0;
    memcpy(buf, im, len);
    return len;
}

static uint32_t st_entries(void *ctx, uint8_t addr, uint16_t seq)
{
    (void)ctx;
    card_file *f = card_find(addr, seq);
    return f ? (uint32_t)f->w.entries : 0;
}

static int st_entry(void *ctx, uint8_t addr, uint16_t seq, uint32_t i, uint8_t out[16])
{
    (void)ctx;
    card_file *f = card_find(addr, seq);
    memcpy(out, f->w.image + f->w.size - 16 * ((size_t)i + 1), 16);
    return 0;
}

static int st_read(void *ctx, uint8_t addr, uint16_t seq, uint32_t offset, uint32_t len, uint8_t *buf)
{
    (void)ctx;
    card_file *f = card_find(addr, seq);
    if ((size_t)offset + len > f->w.size)
        return -1;
    memcpy(buf, f->w.image + offset, len);
    return 0;
}

static const nic_store STORE = {st_header, st_entries, st_entry, st_read, 0};

/* ------------------------------------------------------------------ the server */

typedef struct { uint8_t addr; uint16_t seq; uint8_t *image; size_t size; uint32_t held; } srv_file;
static srv_file srv[16];
static int nsrv;
static ses_chain chain;

static srv_file *srv_find(uint8_t addr, uint16_t seq)
{
    for (int i = 0; i < nsrv; i++)
        if (srv[i].addr == addr && srv[i].seq == seq)
            return &srv[i];
    return 0;
}

static void srv_hello(const uint8_t *msg, size_t n)
{
    ses_hello_t h;
    CHECK(n == 26 && msg[0] == SES_HELLO);
    CHECK(!ses_hello_parse(KEY, msg + 1, n - 1, &h));
    CHECK(h.station == STATION && h.counter == 41 && !memcmp(h.nonce, NONCE, 8) && h.board == 1);
    ses_chain_init(&chain, KEY, h.nonce, h.tag);
}

typedef struct { uint8_t addr; uint16_t seq; uint32_t first; size_t count, hlen, objects; } seen;
static seen log_[4096];
static size_t nlog;
typedef struct { uint8_t addr; uint16_t seq; uint32_t index; } got;
static got objs_[4096];
static size_t nobj;

static void srv_take(const uint8_t *msg, size_t n)
{
    ses_archive_t a;
    CHECK(msg[0] == SES_ARCHIVE);
    if (ses_archive_verify(&chain, msg + 1, n - 1, &a)) {
        CHECK(!"a packet refused");
        return;
    }
    srv_file *f = srv_find(a.address, a.sequence);
    if (a.header_len) {
        hmc_header h;
        CHECK(!hmc_header_parse(a.header, a.header_len, &h));
        if (!f) {
            f = &srv[nsrv++];
            f->addr = a.address;
            f->seq = a.sequence;
            f->size = (size_t)HMC_MB << h.size;
            f->image = calloc(1, f->size);
            f->held = SES_NONE_ENTRY;
        }
        memcpy(f->image, a.header, a.header_len);
    }
    CHECK(f != 0);
    size_t bytes = 0;
    for (size_t i = 0; f && i < a.count; i++) {
        hmc_entry e;
        CHECK(!hmc_entry_parse(a.objects[i].entry, &e));
        memcpy(f->image + e.offset, a.objects[i].cobs, a.objects[i].cobs_len);
        f->image[e.offset + e.length] = 0;
        memcpy(f->image + f->size - 16 * ((size_t)a.objects[i].index + 1), a.objects[i].entry, 16);
        if (f->held == SES_NONE_ENTRY || a.objects[i].index > f->held)
            f->held = a.objects[i].index;
        bytes += 20 + a.objects[i].cobs_len;
        objs_[nobj++] = (got){a.address, a.sequence, a.objects[i].index};
    }
    CHECK(a.count == 1 || bytes <= NIC_UP_BUDGET);
    log_[nlog++] = (seen){a.address, a.sequence, a.count ? a.objects[0].index : 0, a.count,
                          a.header_len, bytes};
    free(a.objects);
}

/* CURSOR or ACK body from what the server holds: the latest file of each address */
static size_t srv_marks(uint8_t *body)
{
    size_t k = 0;
    for (int i = 0; i < nsrv; i++) {
        srv_file *f = &srv[i];
        int latest = 1;
        for (int j = 0; j < nsrv; j++)
            if (srv[j].addr == f->addr && nic_up_before(f->seq, srv[j].seq))
                latest = 0;
        if (!latest)
            continue;
        uint8_t *m = body + 2 + 7 * k++;
        m[0] = f->addr;
        m[1] = (uint8_t)f->seq;
        m[2] = (uint8_t)(f->seq >> 8);
        for (int b = 0; b < 4; b++)
            m[3 + b] = (uint8_t)(f->held >> (8 * b));
    }
    body[0] = (uint8_t)k;
    body[1] = (uint8_t)(k >> 8);
    return 2 + 7 * k;
}

static int same_as_cards(void)
{
    int ok = 1;
    for (int i = 0; i < ncard; i++) {
        srv_file *f = srv_find(card[i].addr, card[i].seq);
        if (!f || f->size != card[i].w.size || memcmp(f->image, card[i].w.image, f->size)) {
            fprintf(stderr, "  file %02X/%u differs\n", card[i].addr, card[i].seq);
            ok = 0;
        }
    }
    return ok;
}

/* ------------------------------------------------------------------ the session */

static nic_uplink up;

static void open_session(const uint8_t *cursor, size_t n)
{
    uint8_t hello[26];
    CHECK(nic_up_hello(&up, STATION, 41, NONCE, 1, hello) == 26);
    srv_hello(hello, 26);
    CHECK(nic_up_next(&up) == 0);                 /* nothing before the server's CURSOR */
    CHECK(!nic_up_cursor(&up, cursor, n));
}

static size_t drain(void)
{
    size_t before = nlog, len;
    while ((len = nic_up_next(&up)) != 0)
        srv_take(up.pkt, len);
    return nlog - before;
}

int main(void)
{
    /* the cards: the event log in two files, a Quake in three, a Pluvius from its sixth, a Gauss
     * across the wrap of its sequence */
    card_open(0x00, 0); card_open(0x52, 0); card_open(0x16, 5); card_open(0x12, 65535);
    for (int i = 0; i < 6; i++) card_segment(0x00, 0, 40 + 13 * i);
    for (int i = 0; i < 9; i++) card_segment(0x52, 0, 300 + 120 * i);
    for (int i = 0; i < 5; i++) card_segment(0x16, 5, 900);
    for (int i = 0; i < 3; i++) card_segment(0x12, 65535, 500);
    card_open(0x00, 1); card_open(0x52, 1); card_open(0x16, 6); card_open(0x12, 0);
    for (int i = 0; i < 3; i++) card_segment(0x00, 1, 20);
    static const uint8_t cfg[8 + 2] = {HMC_RULES, 0, 0, 0, 2, 0, 0, 0, 1, 0};
    uint32_t t_mid = clock_s + 20;
    CHECK(!hmc_writer_config(&card_find(0x52, 1)->w, clock_s, cfg, sizeof cfg));
    for (int i = 0; i < 7; i++) {
        card_segment(0x52, 1, 1300);
        if (i == 3) CHECK(!hmc_writer_config(&card_find(0x52, 1)->w, clock_s, cfg, sizeof cfg));
    }
    for (int i = 0; i < 4; i++) card_segment(0x16, 6, 100);
    for (int i = 0; i < 2; i++) card_segment(0x12, 0, 700);
    card_open(0x52, 2);
    for (int i = 0; i < 2; i++) card_segment(0x52, 2, 50);

    nic_up_init(&up, &STORE, KEY);
    CHECK(!nic_up_files(&up, 0x00, 0, 1));
    CHECK(!nic_up_files(&up, 0x52, 0, 2));
    CHECK(!nic_up_files(&up, 0x16, 5, 6));
    CHECK(!nic_up_files(&up, 0x12, 65535, 0));
    uint32_t all = 6 + 3 + 9 + 9 + 2 + 5 + 4 + 3 + 2;
    CHECK(nic_up_behind(&up, 0x00) + nic_up_behind(&up, 0x52) + nic_up_behind(&up, 0x16)
          + nic_up_behind(&up, 0x12) == all);

    /* an empty cursor: everything, from each address's oldest file */
    uint8_t body[2 + 7 * 16] = {0, 0};
    open_session(body, 2);
    size_t n = drain();
    CHECK(same_as_cards());
    size_t i = 0;
    while (i < n && log_[i].addr == 0x00) i++;
    CHECK(i == 2);                                   /* the event log's two files first */
    for (size_t k = i; k < n; k++)
        CHECK(log_[k].addr != 0x00);
    CHECK(log_[i].addr == 0x52 && log_[i + 1].addr == 0x16 && log_[i + 2].addr == 0x12
          && log_[i + 3].addr == 0x52);             /* the others in turn */
    for (size_t k = 0; k < n; k++) {                 /* a header opens each file, and only it */
        int opens = 1;
        for (size_t j = 0; j < k; j++)
            if (log_[j].addr == log_[k].addr && log_[j].seq == log_[k].seq)
                opens = 0;
        CHECK(!!log_[k].hlen == opens);
    }
    int gauss = 0, wrapped = 0;                      /* 65535 then 0 */
    for (size_t k = 0; k < n; k++)
        if (log_[k].addr == 0x12) {
            if (!gauss++) CHECK(log_[k].seq == 65535);
            if (log_[k].seq == 0) wrapped = 1;
            else CHECK(!wrapped);
        }
    CHECK(wrapped);

    /* ACK: nothing behind */
    size_t bl = srv_marks(body);
    CHECK(!nic_up_ack(&up, body, bl));
    CHECK(nic_up_behind(&up, 0x00) == 0 && nic_up_behind(&up, 0x52) == 0 && nic_up_behind(&up, 0x12) == 0);

    /* the live file grows, then the next opens: they follow, and nothing else */
    card_segment(0x52, 2, 60);
    card_segment(0x00, 1, 5);
    CHECK(nic_up_behind(&up, 0x52) == 1 && nic_up_behind(&up, 0x00) == 1);
    n = drain();
    CHECK(n == 2 && log_[nlog - 2].addr == 0x00 && log_[nlog - 1].addr == 0x52 && log_[nlog - 1].first == 2
          && !log_[nlog - 1].hlen);
    card_open(0x52, 3);
    card_segment(0x52, 3, 80);
    CHECK(!nic_up_files(&up, 0x52, 0, 3));
    n = drain();
    CHECK(n == 1 && log_[nlog - 1].seq == 3 && log_[nlog - 1].hlen && log_[nlog - 1].count == 1);
    CHECK(same_as_cards());

    /* a new session: the Quake's cursor in the middle of file 1, the Pluvius's before the cards'
     * oldest, the Gauss's past its live file, the event log not named */
    uint8_t cur[2 + 7 * 3] = {3, 0, 0x52, 1, 0, 2, 0, 0, 0, 0x16, 2, 0, 0xFF, 0xFF, 0xFF, 0xFF,
                              0x12, 9, 0, 0, 0, 0, 0};
    open_session(cur, sizeof cur);
    size_t from = nlog;
    n = drain();
    CHECK(same_as_cards());                          /* a resend changes nothing */
    int q = 0, p = 0, g = 0, e = 0;
    for (size_t k = from; k < nlog; k++) {
        seen *s = &log_[k];
        if (s->addr == 0x52 && !q++) CHECK(s->seq == 1 && s->first == 3 && !s->hlen);
        if (s->addr == 0x16 && !p++) CHECK(s->seq == 5 && s->first == 0 && s->hlen);
        if (s->addr == 0x12 && !g++) CHECK(s->seq == 0 && s->first == 0 && s->hlen);
        if (s->addr == 0x00 && !e++) CHECK(s->seq == 0 && s->first == 0 && s->hlen && k == from);
    }
    CHECK(q && p && g && e);

    /* a damaged entry: passed over, counted, the rest of the file sent */
    card_open(0x61, 0);
    for (int k = 0; k < 4; k++) card_segment(0x61, 0, 200);
    card_find(0x61, 0)->w.image[card_find(0x61, 0)->w.size - 16 * 2 + 3] ^= 0x40;   /* entry 1 */
    CHECK(!nic_up_files(&up, 0x61, 0, 0));
    uint32_t skipped = up.skipped;
    from = nlog;
    drain();
    size_t objs = 0;
    for (size_t k = from; k < nlog; k++)
        if (log_[k].addr == 0x61) objs += log_[k].count;
    CHECK(objs == 3 && up.skipped == skipped + 1);

    /* WINDOW: asked while the forward stream has something, answered first; one address, then
     * every one, each file's objects the reference cut's */
    static const uint32_t spans[3][2] = {{0, 30}, {0, 5}, {0, 400}};
    for (int t = 0; t < 3; t++) {
        uint8_t who = t == 1 ? 0x52 : 0xFF;
        uint32_t at = t_mid + (uint32_t)t * 37;
        card_segment(0x52, 3, 40);
        CHECK(!nic_up_window_ask(&up, who, at, spans[t][1]));
        size_t from_obj = nobj, from_pkt = nlog;
        drain();
        CHECK(log_[nlog - 1].addr == 0x52 && log_[nlog - 1].seq == 3);   /* the forward one last */
        size_t g = from_obj;
        /* the expected objects, address by address in the head's order, file by file */
        static const uint8_t order[5] = {0x00, 0x52, 0x16, 0x12, 0x61};
        for (int o = 0; o < 5; o++) {
            if (who != 0xFF && order[o] != who)
                continue;
            for (int q = 0; q < 4; q++) {
                uint16_t sq = order[o] == 0x16 ? (uint16_t)(5 + q) : order[o] == 0x12 ? (uint16_t)(65535 + q) : (uint16_t)q;
                card_file *f = card_find(order[o], sq);
                if (!f)
                    continue;
                hmc_reader r;
                uint32_t idx[64];
                CHECK(!hmc_reader_open(&r, f->w.image, f->w.size));
                size_t m = ses_window_cut(&r, at, spans[t][1], idx);
                for (size_t k = 0; k < m; k++, g++)
                    CHECK(g < nobj && objs_[g].addr == order[o] && objs_[g].seq == sq && objs_[g].index == idx[k]);
            }
        }
        CHECK(g == nobj - 1);                        /* and then the forward segment */
        CHECK(g > from_obj + 1);
        CHECK(nlog - from_pkt >= 2);
    }

    /* the queue: eight wait, the ninth is refused, as is an address with no files */
    for (int k = 0; k < 8; k++)
        CHECK(!nic_up_window_ask(&up, 0x16, t_mid, 1));
    CHECK(nic_up_window_ask(&up, 0x16, t_mid, 1) == -1);
    drain();
    CHECK(nic_up_window_ask(&up, 0x77, t_mid, 1) == -1);
    CHECK(!nic_up_window_ask(&up, 0x16, t_mid, 1));

    for (int k = 0; k < nsrv; k++)
        free(srv[k].image);
    for (int k = 0; k < ncard; k++)
        hmc_writer_close(&card[k].w);
    if (fails) {
        printf("test_uplink: %d FAILED\n", fails);
        return 1;
    }
    printf("test_uplink: all passed (%zu packets)\n", nlog);
    return 0;
}

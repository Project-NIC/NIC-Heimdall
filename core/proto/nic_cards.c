/* The head's two cards — see nic_cards.h. */
#include "nic_cards.h"

#include <string.h>

#include "hmc.h"

static uint32_t get32(const uint8_t *p)
{
    return (uint32_t)p[0] | (uint32_t)p[1] << 8 | (uint32_t)p[2] << 16 | (uint32_t)p[3] << 24;
}

static int before(uint16_t a, uint16_t b) { return ((uint16_t)(a - b) & 0x8000u) != 0; }

void nic_cards_init(nic_cards *c, const nic_card_io *io, int in0, int in1)
{
    memset(c, 0, sizeof *c);
    c->io = *io;
    c->reserve = NIC_CARDS_RESERVE;
    c->in[0] = in0 ? 1 : 0;
    c->in[1] = in1 ? 1 : 0;
    c->sync_card = -1;
}

/* The card the others are made from: one that is in and not being brought level. */
static int primary(const nic_cards *c)
{
    if (c->in[0] && c->sync_card != 0)
        return 0;
    return c->in[1] ? 1 : -1;
}

/* A file's header: its length, first second and size; -1 when the card has no such file. */
static int header(nic_cards *c, int card, uint8_t addr, uint16_t seq, uint32_t *len, nic_card_file *f)
{
    uint8_t h[24];
    if (card < 0 || c->io.read(c->io.ctx, card, addr, seq, 0, sizeof h, h) || memcmp(h, "HMC", 4))
        return -1;
    *len = get32(h + 8);
    f->addr = addr;
    f->seq = seq;
    f->first_second = get32(h + 12);
    f->size = h[19];
    return f->size > 7 ? -1 : 0;
}

static int entry(nic_cards *c, int card, uint8_t addr, uint16_t seq, uint8_t size, uint32_t i, hmc_entry *e)
{
    uint8_t b[16];
    uint32_t bytes = HMC_MB << size;
    if ((uint64_t)16 * (i + 1) > bytes || c->io.read(c->io.ctx, card, addr, seq, bytes - 16 * (i + 1), 16, b))
        return -1;
    return hmc_entry_parse(b, e) ? -1 : 0;
}

uint32_t nic_cards_good(nic_cards *c, int card, uint8_t addr, uint16_t seq, uint8_t size)
{
    uint32_t lo = 0, hi = (HMC_MB << size) / 16;
    while (lo < hi) {
        uint32_t m = lo + (hi - lo) / 2;
        hmc_entry e;
        if (!entry(c, card, addr, seq, size, m, &e))
            lo = m + 1;
        else
            hi = m;
    }
    return lo;
}

static nic_card_addr *find(nic_cards *c, uint8_t addr)
{
    for (uint8_t i = 0; i < c->n; i++)
        if (c->a[i].addr == addr)
            return &c->a[i];
    return 0;
}

static void first_second(nic_cards *c, nic_card_addr *a)
{
    uint32_t len;
    nic_card_file f;
    a->first_second = header(c, primary(c), a->addr, a->first, &len, &f) ? 0 : f.first_second;
}

int nic_cards_files(nic_cards *c, uint8_t addr, uint16_t first, uint16_t live)
{
    nic_card_addr *a = find(c, addr);
    if (!a) {
        if (c->n == NIC_CARDS_ADDRS)
            return -1;
        a = &c->a[c->n++];
        memset(a, 0, sizeof *a);
        a->addr = addr;
    }
    a->first = first;
    a->live = live;
    first_second(c, a);
    return 0;
}

static int short_of_reserve(nic_cards *c)
{
    for (int k = 0; k < 2; k++)
        if (c->in[k] && c->io.free_bytes(c->io.ctx, k) < c->reserve)
            return 1;
    return 0;
}

int nic_cards_circle(nic_cards *c)
{
    int taken = 0;
    while (short_of_reserve(c)) {
        nic_card_addr *old = 0;
        for (uint8_t i = 0; i < c->n; i++) {
            nic_card_addr *a = &c->a[i];
            if (a->first != a->live && (!old || a->first_second < old->first_second))
                old = a;
        }
        if (!old)
            return -1;
        for (int k = 0; k < 2; k++)
            if (c->in[k])
                c->io.remove(c->io.ctx, k, old->addr, old->first);
        int held = c->io.held(c->io.ctx, old->addr, old->first);
        if (!held)
            c->gone_unheld++;
        c->io.gone(c->io.ctx, old->addr, old->first, held);
        if (c->copying && c->c_addr == old->addr && c->c_seq == old->first)
            c->copying = 0;
        old->first++;
        first_second(c, old);
        taken++;
    }
    return taken;
}

void nic_cards_out(nic_cards *c, int card)
{
    c->in[card] = 0;
    if (c->sync_state != NIC_SYNC_OFF) {      /* the card being levelled, or its source, gone */
        c->sync_state = NIC_SYNC_OFF;
        c->sync_card = -1;
        c->copying = 0;
    }
}

int nic_cards_back(nic_cards *c, int card)
{
    c->in[card] = 1;
    c->copying = 0;
    if (!c->in[1 - card]) {                   /* nothing to level against */
        c->sync_state = NIC_SYNC_OFF;
        c->sync_card = -1;
        return 0;
    }
    c->sync_card = (int8_t)card;
    int p = primary(c);
    for (uint8_t i = 0; i < c->n; i++) {
        nic_card_addr *a = &c->a[i];
        uint32_t len;
        nic_card_file f, g;
        a->live_then = a->live;
        a->gap_due = 0;
        if (header(c, p, a->addr, a->live, &len, &f))
            continue;
        a->gap_to = nic_cards_good(c, p, a->addr, a->live, f.size);
        if (header(c, card, a->addr, a->live, &len, &g)) {
            if (c->io.create(c->io.ctx, card, &f))
                return -1;
            a->gap_from = 0;
            a->gap_due = 1;
        } else {
            a->gap_from = nic_cards_good(c, card, a->addr, a->live, f.size);
            a->gap_due = a->gap_from < a->gap_to;
        }
        a->cursor = (uint16_t)(a->live - 1);
        a->cursor_on = a->live != a->first;
    }
    c->sync_state = NIC_SYNC_GAPS;
    c->sync_k = 0;
    return 0;
}

/* A copy of entries [from, to) of a file: its data — the header with it from the first entry —
 * then the entries. */
static int start_copy(nic_cards *c, uint8_t addr, uint16_t seq, uint32_t from, uint32_t to)
{
    int p = primary(c);
    uint32_t hlen;
    nic_card_file f;
    hmc_entry e;
    if (header(c, p, addr, seq, &hlen, &f))
        return -1;
    uint32_t bytes = HMC_MB << f.size;
    c->c_addr = addr;
    c->c_seq = seq;
    c->c_off = from && !entry(c, p, addr, seq, f.size, from - 1, &e) ? e.offset + e.length + 1 : 0;
    c->c_end = hlen;
    for (uint32_t i = to; i > from; i--)
        if (!entry(c, p, addr, seq, f.size, i - 1, &e)) {
            c->c_end = e.offset + e.length + 1;
            break;
        }
    if (c->c_end < c->c_off)
        c->c_end = c->c_off;
    c->e_off = bytes - 16 * to;
    c->e_end = bytes - 16 * from;
    c->copying = 1;
    return 0;
}

static uint32_t copy(nic_cards *c, uint32_t *off, uint32_t end, uint32_t budget)
{
    uint32_t len = end - *off;
    if (len > NIC_CARDS_CHUNK) len = NIC_CARDS_CHUNK;
    if (len > budget) len = budget;
    if (c->io.read(c->io.ctx, primary(c), c->c_addr, c->c_seq, *off, len, c->buf)
        || c->io.write(c->io.ctx, c->sync_card, c->c_addr, c->c_seq, *off, len, c->buf)) {
        c->copying = 0;                      /* the file went, or the card did */
        return len;
    }
    *off += len;
    c->copied += len;
    return len;
}

/* Every file on the card put back older than the address's oldest: taken by the circle meanwhile,
 * with gaps where files opened and went while it was out. */
static void sweep_old(nic_cards *c)
{
    for (uint8_t i = 0; i < c->n; i++) {
        nic_card_addr *a = &c->a[i];
        uint16_t first, live;
        if (c->io.span(c->io.ctx, c->sync_card, a->addr, &first, &live))
            continue;
        for (uint16_t s = first; before(s, a->first); s++) {
            c->io.remove(c->io.ctx, c->sync_card, a->addr, s);
            if (s == live)
                break;
        }
    }
}

int nic_cards_step(nic_cards *c, uint32_t budget)
{
    while (budget && c->sync_state != NIC_SYNC_OFF) {
        if (c->copying) {
            if (c->c_off < c->c_end)
                budget -= copy(c, &c->c_off, c->c_end, budget);
            else if (c->e_off < c->e_end)
                budget -= copy(c, &c->e_off, c->e_end, budget);
            else
                c->copying = 0;
            continue;
        }
        if (c->sync_state == NIC_SYNC_GAPS) {
            while (c->sync_k < c->n && !c->a[c->sync_k].gap_due)
                c->sync_k++;
            if (c->sync_k < c->n) {
                nic_card_addr *a = &c->a[c->sync_k];
                a->gap_due = 0;
                start_copy(c, a->addr, a->live_then, a->gap_from, a->gap_to);
            } else {
                c->sync_state = NIC_SYNC_FILES;
                sweep_old(c);
            }
            continue;
        }
        /* the newest closed file left, whatever the address */
        int p = primary(c);
        nic_card_addr *best = 0;
        nic_card_file bf = {0, 0, 0, 0};
        for (uint8_t i = 0; i < c->n; i++) {
            nic_card_addr *a = &c->a[i];
            uint32_t len;
            nic_card_file f;
            if (!a->cursor_on)
                continue;
            if (before(a->cursor, a->first)) {
                a->cursor_on = 0;
                continue;
            }
            if (header(c, p, a->addr, a->cursor, &len, &f)) {
                a->cursor_on = a->cursor != a->first;
                a->cursor--;
                continue;
            }
            if (!best || f.first_second > bf.first_second) {
                best = a;
                bf = f;
            }
        }
        if (!best) {
            int card = c->sync_card;
            c->sync_state = NIC_SYNC_OFF;
            c->sync_card = -1;
            c->io.level(c->io.ctx, card);
            return 0;
        }
        uint16_t s = best->cursor;
        best->cursor_on = s != best->first;
        best->cursor--;
        uint32_t len, from;
        nic_card_file g;
        uint32_t to = nic_cards_good(c, p, best->addr, s, bf.size);
        if (header(c, c->sync_card, best->addr, s, &len, &g)) {
            if (c->io.create(c->io.ctx, c->sync_card, &bf))
                continue;
            from = 0;
        } else {
            from = nic_cards_good(c, c->sync_card, best->addr, s, bf.size);
            if (from >= to)
                continue;
        }
        start_copy(c, best->addr, s, from, to);
    }
    return c->sync_state != NIC_SYNC_OFF;
}

/* The head's side of the uplink session — see nic_uplink.h. */
#include "nic_uplink.h"

#include <string.h>

static void put16(uint8_t *p, uint32_t v) { p[0] = (uint8_t)v; p[1] = (uint8_t)(v >> 8); }
static void put32(uint8_t *p, uint32_t v) { put16(p, v & 0xFFFF); put16(p + 2, v >> 16); }
static uint32_t get16(const uint8_t *p) { return (uint32_t)p[0] | (uint32_t)p[1] << 8; }
static uint32_t get32(const uint8_t *p) { return get16(p) | get16(p + 2) << 16; }

void nic_up_init(nic_uplink *u, const nic_store *store, const uint8_t key[16])
{
    memset(u, 0, sizeof *u);
    u->store = *store;
    memcpy(u->key, key, 16);
}

static nic_up_addr *find(nic_uplink *u, uint8_t addr)
{
    for (uint8_t i = 0; i < u->n; i++)
        if (u->a[i].addr == addr)
            return &u->a[i];
    return 0;
}

static void from_first(nic_up_addr *a)
{
    a->seq = a->first;
    a->next = 0;
    a->header_due = 1;
}

int nic_up_files(nic_uplink *u, uint8_t addr, uint16_t first, uint16_t live)
{
    nic_up_addr *a = find(u, addr);
    if (!a) {
        if (u->n == NIC_UP_ADDRS)
            return -1;
        a = &u->a[u->n++];
        memset(a, 0, sizeof *a);
        a->addr = addr;
        a->first = first;
        from_first(a);
    }
    a->first = first;
    a->live = live;
    if (nic_up_before(a->seq, first))       /* the file being sent is no longer held */
        from_first(a);
    return 0;
}

size_t nic_up_hello(nic_uplink *u, uint32_t station, uint32_t counter, const uint8_t nonce[8],
                    int board, uint8_t out[26])
{
    out[0] = SES_HELLO;
    out[1] = 1;
    out[2] = (uint8_t)station;
    put16(out + 3, station >> 8);
    put32(out + 5, counter);
    memcpy(out + 9, nonce, 8);
    out[17] = board ? 1 : 0;
    const uint8_t *p = out;
    size_t l = 18;
    ses_tag(u->key, &p, &l, 1, out + 18);
    ses_chain_init(&u->chain, u->key, nonce, out + 18);
    u->state = NIC_UP_HELLO;
    return 26;
}

/* Where an address starts: the file and entry the server holds, or its oldest file; whatever the
 * cards cannot place is rewound to what they hold. */
static void place(nic_up_addr *a, uint16_t seq, uint32_t held)
{
    if (nic_up_before(seq, a->first) || nic_up_before(a->live, seq)) {
        from_first(a);
        if (nic_up_before(a->live, seq))            /* past the live file: the live file again */
            a->seq = a->live;
        return;
    }
    a->seq = seq;
    a->next = held == SES_NONE_ENTRY ? 0 : held + 1;
    a->header_due = held == SES_NONE_ENTRY;
}

int nic_up_cursor(nic_uplink *u, const uint8_t *body, size_t n)
{
    if (n < 2 || n != 2 + 7 * (size_t)get16(body))
        return -1;
    for (uint8_t i = 0; i < u->n; i++)
        from_first(&u->a[i]);
    for (size_t k = 0; k < get16(body); k++) {
        const uint8_t *m = body + 2 + 7 * k;
        nic_up_addr *a = find(u, m[0]);
        if (a)
            place(a, (uint16_t)get16(m + 1), get32(m + 3));
    }
    u->turn = 0;
    u->state = NIC_UP_SENDING;
    return 0;
}

/* The file an address sends from: a finished file left for the next, while one is held. */
static uint32_t pending(nic_uplink *u, nic_up_addr *a)
{
    for (;;) {
        uint32_t have = u->store.entries(u->store.ctx, a->addr, a->seq);
        if (a->next < have)
            return have;
        if (a->seq == a->live)
            return 0;
        a->seq++;
        a->next = 0;
        a->header_due = 1;
    }
}

/* A segment's first and last second, from the head of its COBS bytes: -1 when it is not one. */
static int seconds_of(nic_uplink *u, uint8_t addr, uint16_t seq, const hmc_entry *e, uint32_t *first,
                      uint32_t *last)
{
    uint8_t in[24], out[16];
    uint32_t n = e->length < sizeof in ? e->length : (uint32_t)sizeof in;
    size_t o = 0, i = 0;
    if (u->store.read(u->store.ctx, addr, seq, e->offset, n, in))
        return -1;
    while (o < sizeof out && i < n) {
        uint8_t c = in[i++];
        if (!c)
            return -1;
        for (uint8_t k = 1; k < c && o < sizeof out; k++) {
            if (i >= n)
                return -1;
            out[o++] = in[i++];
        }
        if (o < sizeof out && c < 0xFF)
            out[o++] = 0;
    }
    if (o < sizeof out || memcmp(out, "HSEG", 4))
        return -1;
    *first = get32(out + 8);
    *last = get32(out + 12);
    return 0;
}

static int touches(nic_uplink *u, uint8_t addr, uint16_t seq, const hmc_entry *e, const nic_up_window *w)
{
    uint32_t first, last;
    return e->type == HMC_SEG && !seconds_of(u, addr, seq, e, &first, &last) && first <= w->hi
           && last >= w->lo;
}

/* One ARCHIVE packet of a file's entries from *i up to `end`: every good one, or under a window
 * the configs before `wlast` and the segments that touch it. */
static size_t packet(nic_uplink *u, uint8_t addr, uint16_t seq, int header, uint32_t *i, uint32_t end,
                     const nic_up_window *w, uint32_t wlast)
{
    uint8_t *p = u->pkt;
    size_t hlen = 0;
    p[0] = SES_ARCHIVE;
    p[1] = addr;
    put16(p + 2, seq);
    put32(p + 4, u->chain.packet + 1);
    if (header) {
        hlen = u->store.header(u->store.ctx, addr, seq, p + 12, NIC_UP_HEADER_MAX);
        if (!hlen)
            return 0;
    }
    put32(p + 8, (uint32_t)hlen);
    size_t at = 12 + hlen, count_at = at, size = 0;
    uint32_t count = 0;
    at += 2;
    for (; *i < end; ++*i) {
        uint8_t ent[16];
        hmc_entry e;
        if (u->store.entry(u->store.ctx, addr, seq, *i, ent) || hmc_entry_parse(ent, &e)
            || e.length > NIC_UP_OBJECT_MAX) {
            u->skipped++;
            continue;
        }
        if (w && !(e.type == HMC_CFG ? *i < wlast : touches(u, addr, seq, &e, w)))
            continue;
        if (count && size + 20 + e.length > NIC_UP_BUDGET)
            break;
        put32(p + at, *i);
        memcpy(p + at + 4, ent, 16);
        if (u->store.read(u->store.ctx, addr, seq, e.offset, e.length, p + at + 20)) {
            u->skipped++;
            continue;
        }
        at += 20 + e.length;
        size += 20 + e.length;
        count++;
    }
    if (!count && !hlen)
        return 0;
    put16(p + count_at, count);
    const uint8_t *parts[3] = {u->chain.nonce, u->chain.last, p};
    size_t lens[3] = {8, 8, at};
    ses_tag(u->chain.key, parts, lens, 3, p + at);
    memcpy(u->chain.last, p + at, 8);
    u->chain.packet++;
    return at + 8;
}

static size_t build(nic_uplink *u, nic_up_addr *a, uint32_t have)
{
    size_t len = packet(u, a->addr, a->seq, a->header_due, &a->next, have, 0, 0);
    if (len)
        a->header_due = 0;
    return len;
}

int nic_up_window_ask(nic_uplink *u, uint8_t addr, uint32_t second, uint32_t span)
{
    if (u->queued == NIC_UP_WINDOWS || (addr != 0xFF && !find(u, addr)))
        return -1;
    nic_up_window *w = &u->queue[u->queued++];
    w->addr = addr;
    w->lo = second > span ? second - span : 0;
    w->hi = second > 0xFFFFFFFFu - span ? 0xFFFFFFFFu : second + span;
    return 0;
}

/* The first entry whose second is at or after `lo`: the seconds never go back. */
static uint32_t search(nic_uplink *u, uint8_t addr, uint16_t seq, uint32_t have, uint32_t lo)
{
    uint32_t a = 0, b = have;
    while (a < b) {
        uint32_t m = a + (b - a) / 2;
        uint8_t ent[16];
        hmc_entry e;
        int ok = !u->store.entry(u->store.ctx, addr, seq, m, ent) && !hmc_entry_parse(ent, &e);
        if (ok && e.second < lo)
            a = m + 1;
        else
            b = m;
    }
    return a;
}

/* A file's part in the answer: the last segment that touches the window, -1 none; 1 in *beyond
 * when the file opened past the search's end, so no later file of the address need be looked at. */
static int64_t file_last(nic_uplink *u, uint8_t addr, uint16_t seq, const nic_up_window *w, int *beyond)
{
    uint64_t stop = (uint64_t)w->hi + NIC_UP_LATE;
    size_t hlen = u->store.header(u->store.ctx, addr, seq, u->pkt, NIC_UP_HEADER_MAX);
    *beyond = 0;
    if (hlen < 16)
        return -1;
    if (get32(u->pkt + 12) > stop) {
        *beyond = 1;
        return -1;
    }
    uint32_t have = u->store.entries(u->store.ctx, addr, seq);
    int64_t last = -1;
    for (uint32_t i = search(u, addr, seq, have, w->lo); i < have; i++) {
        uint8_t ent[16];
        hmc_entry e;
        if (u->store.entry(u->store.ctx, addr, seq, i, ent) || hmc_entry_parse(ent, &e))
            continue;
        if (e.second > stop)
            break;
        if (touches(u, addr, seq, &e, w))
            last = i;
    }
    return last;
}

static size_t answer(nic_uplink *u)
{
    nic_up_answer *s = &u->ans;
    for (;;) {
        if (!s->on) {
            if (!u->queued)
                return 0;
            memset(s, 0, sizeof *s);
            s->w = u->queue[0];
            memmove(u->queue, u->queue + 1, --u->queued * sizeof u->queue[0]);
            s->on = 1;
            s->k = 0;
            if (s->w.addr != 0xFF)
                s->k = (uint8_t)(find(u, s->w.addr) - u->a);
            s->seq = u->a[s->k].first;
        }
        nic_up_addr *a = &u->a[s->k];
        int done_addr = 0;
        if (!s->ready) {
            int beyond;
            int64_t last = file_last(u, a->addr, s->seq, &s->w, &beyond);
            if (last >= 0) {
                s->ready = 1;
                s->last = (uint32_t)last;
                s->i = 0;
                s->header_due = 1;
            } else if (beyond || s->seq == a->live) {
                done_addr = 1;
            } else {
                s->seq++;
            }
        }
        if (s->ready) {
            size_t len = packet(u, a->addr, s->seq, s->header_due, &s->i, s->last + 1, &s->w, s->last);
            if (len) {
                s->header_due = 0;
                return len;
            }
            s->ready = 0;
            if (s->seq == a->live)
                done_addr = 1;
            else
                s->seq++;
        }
        if (done_addr) {
            if (s->w.addr == 0xFF && s->k + 1 < u->n) {
                s->k++;
                s->seq = u->a[s->k].first;
            } else {
                s->on = 0;
            }
        }
    }
}

size_t nic_up_next(nic_uplink *u)
{
    if (u->state != NIC_UP_SENDING || !u->n)
        return 0;
    size_t len = answer(u);
    if (len)
        return len;
    nic_up_addr *log = find(u, 0);
    if (log) {
        uint32_t have = pending(u, log);
        if (have) {
            size_t len = build(u, log, have);
            if (len)
                return len;
        }
    }
    for (uint8_t k = 0; k < u->n; k++) {
        nic_up_addr *a = &u->a[(u->turn + k) % u->n];
        if (a->addr == 0)
            continue;
        uint32_t have = pending(u, a);
        if (!have)
            continue;
        size_t len = build(u, a, have);
        if (len) {
            u->turn = (uint8_t)((u->turn + k + 1) % u->n);
            return len;
        }
    }
    return 0;
}

int nic_up_ack(nic_uplink *u, const uint8_t *body, size_t n)
{
    if (n < 2 || n != 2 + 7 * (size_t)get16(body))
        return -1;
    for (size_t k = 0; k < get16(body); k++) {
        const uint8_t *m = body + 2 + 7 * k;
        nic_up_addr *a = find(u, m[0]);
        uint16_t seq = (uint16_t)get16(m + 1);
        uint32_t e = get32(m + 3);
        if (!a || e == SES_NONE_ENTRY)
            continue;
        if (!a->acked || nic_up_before(a->ack_seq, seq) || (a->ack_seq == seq && e > a->ack_entry)) {
            a->acked = 1;
            a->ack_seq = seq;
            a->ack_entry = e;
        }
    }
    return 0;
}

uint32_t nic_up_behind(const nic_uplink *u, uint8_t addr)
{
    nic_up_addr *a = find((nic_uplink *)u, addr);
    if (!a)
        return 0;
    uint16_t s = a->first;
    uint32_t from = 0, behind = 0;
    if (a->acked && !nic_up_before(a->ack_seq, a->first) && !nic_up_before(a->live, a->ack_seq)) {
        s = a->ack_seq;
        from = a->ack_entry + 1;
    }
    for (;;) {
        uint32_t have = u->store.entries(u->store.ctx, addr, s);
        behind += have > from ? have - from : 0;
        if (s == a->live)
            return behind;
        s++;
        from = 0;
    }
}

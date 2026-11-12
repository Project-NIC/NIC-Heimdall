/* The card's data path; see nic_card.h. */
#include "nic_card.h"

#include <string.h>

/* a cell's state for its period */
enum { C_EMPTY = 0, C_FRAME, C_MISS1, C_MISS2, C_SLIPPED, C_HEARD };

int nic_slot_inside(uint32_t arrival, uint32_t round_start, uint8_t slot, uint32_t width,
                    uint32_t byte_time)
{
    if (slot == 0)
        return 0;
    int64_t at = (int64_t)round_start + (int64_t)(slot - 1) * width;
    int64_t d = (int64_t)arrival - at;
    return d >= -2 * (int64_t)byte_time && d <= 2 * (int64_t)byte_time;
}

void nic_unitbuf_init(nic_unitbuf *u, nic_cell *cells, uint16_t depth, uint16_t delay,
                      uint16_t rows, uint8_t addr, uint8_t slot)
{
    u->cells = cells;
    u->depth = depth;
    u->delay = delay;
    u->rows = rows;
    u->addr = addr;
    u->slot = slot;
    u->wrong = 0;
    for (uint16_t i = 0; i < depth; i++) {
        cells[i].period = 0xFFFFFFFFu;
        cells[i].state = C_EMPTY;
    }
}

static nic_cell *cell_for(nic_unitbuf *u, uint32_t period)
{
    nic_cell *c = &u->cells[period % u->depth];
    if (c->period != period) {         /* a new period takes the place over */
        c->period = period;
        c->state = C_EMPTY;
    }
    return c;
}

enum nic_arrival nic_unitbuf_arrive(nic_unitbuf *u, uint32_t period, const uint8_t *frame,
                                    size_t len, int inside)
{
    nic_cell *c = cell_for(u, period);
    nic_header h;
    const uint8_t *p;
    size_t plen;
    if (len != NIC_SPUR_LEN) {               /* a sonde's 16 B go to the tiling, not here */
        if (c->state == C_EMPTY)
            c->state = C_HEARD;
        return NIC_WRONG;
    }
    if (nic_data_parse(frame, len, &h, &p, &plen)) {
        if (c->state == C_MISS1 || c->state == C_MISS2) {
            c->state = C_MISS2;
            return NIC_GIVEN_UP;
        }
        if (c->state != C_FRAME)
            c->state = C_MISS1;
        return c->state == C_FRAME ? NIC_STORED : NIC_ASK_RESEND;
    }
    if (!inside || h.slot != u->slot || h.addr != u->addr) {
        if (c->state == C_EMPTY)
            c->state = C_HEARD;
        return ++u->wrong >= 3 ? NIC_LADDER : NIC_WRONG;
    }
    u->wrong = 0;
    uint32_t second = period / u->rows;
    if (h.unix0 != (uint8_t)second || h.frame != (uint8_t)(period % u->rows)) {
        c->state = C_SLIPPED;
        return NIC_WRONG;
    }
    memcpy(c->buf + 3, frame, len);
    c->state = C_FRAME;
    return NIC_STORED;
}

void nic_unitbuf_heard(nic_unitbuf *u, uint32_t period)
{
    nic_cell *c = cell_for(u, period);
    if (c->state == C_EMPTY)
        c->state = C_HEARD;
}

void nic_unitbuf_release(nic_unitbuf *u, uint32_t now, int labelled, uint8_t out[NIC_TRUNK_LEN])
{
    uint32_t period = now - u->delay, second = period / u->rows;
    nic_cell *c = &u->cells[period % u->depth];
    int have = c->period == period;
    uint8_t reason = !labelled ? NIC_FILL_UNLABELLED
                     : !have || c->state == C_EMPTY ? NIC_FILL_MISSING
                     : c->state == C_HEARD ? NIC_FILL_EMPTY
                     : c->state == C_MISS1 || c->state == C_MISS2 ? NIC_FILL_CORRUPT
                     : c->state == C_SLIPPED ? NIC_FILL_SLIPPED : 0;
    if (reason == 0) {
        memcpy(out, c->buf, NIC_TRUNK_LEN);
        nic_trunk_complete(out, second);
        return;
    }
    uint8_t payload[NIC_PAYLOAD] = {reason};
    nic_header h = {(uint8_t)second, (uint8_t)(period % u->rows), u->addr, u->slot, NIC_FILLER, 0};
    nic_data_build(&h, payload, NIC_PAYLOAD, out + 3);
    nic_trunk_complete(out, second);
}

/* ------------------------------------------------------------------ the card's own frames */

void nic_segment_table(const nic_seg_row rows[8], uint8_t out[NIC_PAYLOAD])
{
    for (int i = 0; i < 8; i++) {
        out[4 * i] = rows[i].addr;
        out[4 * i + 1] = rows[i].addr ? rows[i].port : 0;
        out[4 * i + 2] = rows[i].addr ? rows[i].rung : 0;
        out[4 * i + 3] = rows[i].addr ? rows[i].status : 0;
    }
}

static void put16(uint8_t *p, uint16_t v) { p[0] = (uint8_t)v; p[1] = (uint8_t)(v >> 8); }

void nic_ports_payload(const uint16_t vbus[5], const uint16_t current[5], const uint16_t leak[4],
                       int16_t temp, uint8_t out[NIC_PAYLOAD])
{
    memset(out, 0, NIC_PAYLOAD);
    for (int i = 0; i < 5; i++) {
        put16(out + 4 * i, vbus[i]);
        put16(out + 4 * i + 2, current[i]);
    }
    for (int i = 0; i < 4; i++)
        put16(out + 20 + 2 * i, leak[i]);
    put16(out + 28, (uint16_t)temp);
}

/* ------------------------------------------------------------------ an Argus's tiling */

void nic_tile_init(nic_tile *t)
{
    memset(t, 0, sizeof *t);
}

void nic_tile_place(nic_tile *t, unsigned pos, uint8_t status, const uint8_t payload[NIC_MINI_PAYLOAD])
{
    if (pos > 3)
        return;
    memcpy(t->payload + 8 * pos, payload, NIC_MINI_PAYLOAD);
    t->present |= (uint8_t)(1u << pos);
    t->flags |= status & (NIC_ALARM | NIC_SUPPLY | NIC_FAULT);
    if ((status & 15) > t->worst)
        t->worst = status & 15;
}

uint8_t nic_tile_status(const nic_tile *t, uint8_t seated)
{
    uint8_t worst = t->worst;
    if ((seated & ~t->present) & 15 && worst < NIC_SENSOR)
        worst = NIC_SENSOR;
    return (uint8_t)(t->flags | worst);
}

/* ------------------------------------------------------------------ ranging */

int32_t nic_route_of(uint32_t measured, uint32_t ccr)
{
    return ((int32_t)measured - (int32_t)(ccr + 3)) / 2;
}

int32_t nic_median3(int32_t a, int32_t b, int32_t c)
{
    if (a > b) { int32_t t = a; a = b; b = t; }
    if (b > c) { int32_t t = b; b = c; c = t; }
    if (a > b) { int32_t t = a; a = b; b = t; }
    return b;
}

static int32_t absdiff(int32_t a, int32_t b) { return a > b ? a - b : b - a; }

enum nic_range_action nic_range_update(nic_ranger *r, int ok, int32_t route)
{
    if (!ok) {
        r->unranged = 1;
        return NIC_RANGE_LOST;
    }
    r->unranged = 0;
    if (!r->have) {
        r->route = route;
        r->have = 1;
        return NIC_RANGE_WRITE;
    }
    int32_t d = absdiff(route, r->route);
    if (r->pending) {
        r->pending = 0;
        if (d > 128 && absdiff(route, r->candidate) <= 2) {   /* two agreeing: the cable moved */
            r->route = route;
            r->changes++;
            return NIC_RANGE_WRITE;
        }
        if (d <= 128) {                                       /* the far one was a lone outlier */
            r->outliers++;
        } else {                                               /* far again and not agreeing */
            r->outliers++;
            r->candidate = route;
            r->pending = 1;
            return NIC_RANGE_AGAIN;
        }
    }
    if (d <= 2)
        return NIC_RANGE_KEEP;
    if (d <= 128) {
        r->route = route;
        r->changes++;
        return NIC_RANGE_WRITE;
    }
    r->candidate = route;
    r->pending = 1;
    return NIC_RANGE_AGAIN;
}

/* ------------------------------------------------------------------ the ladder */

void nic_ladder_fault(nic_ladder *l, uint32_t now)
{
    if (l->step != NIC_L_OK)
        return;
    l->step = NIC_L_WAIT;
    l->since = now;
    l->attempts = 0;
}

void nic_ladder_alive(nic_ladder *l)
{
    l->step = NIC_L_OK;
    l->attempts = 0;
}

enum nic_ladder_action nic_ladder_tick(nic_ladder *l, uint32_t now, uint32_t round_us)
{
    uint32_t dt = now - l->since;
    switch (l->step) {
    case NIC_L_WAIT:                       /* step 2: eight rounds for the unit to rejoin itself */
        if ((uint64_t)dt * 1000 < (uint64_t)NIC_LADDER_ROUNDS * round_us)
            return NIC_LA_NONE;
        break;
    case NIC_L_OFF:                        /* step 3: the feed off for a second */
        if (dt < NIC_LADDER_OFF_MS)
            return NIC_LA_NONE;
        l->step = NIC_L_REJOIN;
        l->since = now;
        return NIC_LA_POWER_ON;
    case NIC_L_REJOIN:                     /* step 4: four seconds to come back */
        if (dt < NIC_LADDER_WAIT_MS)
            return NIC_LA_NONE;
        if (l->attempts >= NIC_LADDER_TRIES) {
            l->step = NIC_L_DEAD;          /* step 5 */
            return NIC_LA_DEAD;
        }
        break;
    default:
        return NIC_LA_NONE;
    }
    l->attempts++;
    l->step = NIC_L_OFF;
    l->since = now;
    return NIC_LA_POWER_OFF;
}

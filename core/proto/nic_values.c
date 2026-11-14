/* The head's values frame on the modem — see nic_values.h. */
#include "nic_values.h"

#include <string.h>

static uint16_t crc16(const uint8_t *b, size_t n)      /* CCITT-FALSE, the station's */
{
    uint16_t c = 0xFFFF;
    while (n--) {
        c ^= (uint16_t)(*b++ << 8);
        for (int i = 0; i < 8; i++)
            c = (uint16_t)(c & 0x8000 ? (c << 1) ^ 0x1021 : c << 1);
    }
    return c;
}

static void put(uint8_t *p, uint32_t v, int n)
{
    for (int i = 0; i < n; i++)
        p[i] = (uint8_t)(v >> (8 * i));
}

static uint32_t get(const uint8_t *p, int n)
{
    uint32_t v = 0;
    for (int i = 0; i < n; i++)
        v |= (uint32_t)p[i] << (8 * i);
    return v;
}

void nic_val_build(const nic_val_frame *f, uint8_t out[NIC_VAL_LEN])
{
    put(out, f->station, 3);
    put(out + 3, f->second, 4);
    put(out + 7, f->pack_mv, 2);
    out[9] = (uint8_t)f->temperature;
    put(out + 10, f->events, 2);
    memcpy(out + 12, f->alive, 16);
    out[28] = (uint8_t)((f->clock & 3) | (f->marker ? 4 : 0) | (f->why & 7) << 3 | (f->power & 3) << 6);
    out[29] = f->peak;
    put(out + 30, crc16(out, 30), 2);
}

int nic_val_parse(const uint8_t in[NIC_VAL_LEN], nic_val_frame *f)
{
    if (crc16(in, 30) != (uint16_t)get(in + 30, 2))
        return -1;
    f->station = get(in, 3);
    f->second = get(in + 3, 4);
    f->pack_mv = (uint16_t)get(in + 7, 2);
    f->temperature = (int8_t)in[9];
    f->events = (uint16_t)get(in + 10, 2);
    memcpy(f->alive, in + 12, 16);
    f->clock = in[28] & 3;
    f->marker = in[28] >> 2 & 1;
    f->why = in[28] >> 3 & 7;
    f->power = in[28] >> 6;
    f->peak = in[29];
    return 0;
}

void nic_val_init(nic_values *v, uint32_t station, uint32_t now, uint32_t airtime_ms, uint32_t cap_ms)
{
    memset(v, 0, sizeof *v);
    v->station = station;
    v->last_sent = now;
    v->airtime_ms = airtime_ms;
    v->cap_ms = cap_ms;
}

void nic_val_weak(nic_values *v)
{
    v->events++;
}

int nic_val_marker(nic_values *v, uint32_t now, uint8_t why, uint8_t peak)
{
    if (v->due || (v->marked && now - v->last_marker < NIC_VAL_MINUTE)) {
        v->events++;
        return 0;
    }
    v->due = 1;
    v->due_why = why;
    v->due_peak = peak;
    return 1;
}

/* The airtime of the hour before `now`, a send more allowed under the cap. */
static int room(const nic_values *v, uint32_t now)
{
    if (!v->cap_ms)
        return 1;
    uint32_t used = 0;
    for (uint8_t i = 0; i < v->nsends; i++)
        if (now - v->sends[i] < NIC_VAL_HOUR)
            used += v->airtime_ms;
    return used + v->airtime_ms <= v->cap_ms;
}

static void spent(nic_values *v, uint32_t now)
{
    v->sends[v->sends_at] = now;
    v->sends_at = (uint8_t)((v->sends_at + 1) % NIC_VAL_SENDS);
    if (v->nsends < NIC_VAL_SENDS)
        v->nsends++;
}

int nic_val_tick(nic_values *v, uint32_t now, const nic_val_state *st, uint8_t out[NIC_VAL_LEN])
{
    int heartbeat = now - v->last_sent >= NIC_VAL_HOUR;
    if (!v->due && !heartbeat)
        return 0;
    if (!room(v, now)) {
        v->held++;
        return 0;
    }
    nic_val_frame f;
    memset(&f, 0, sizeof f);
    f.station = v->station;
    f.second = now;
    f.pack_mv = st->pack_mv;
    f.temperature = st->temperature;
    f.events = (uint16_t)(v->events > 0xFFFF ? 0xFFFF : v->events);
    memcpy(f.alive, st->alive, 16);
    f.clock = st->clock;
    f.power = st->power;
    if (v->due) {
        f.marker = 1;
        f.why = v->due_why;
        f.peak = v->due_peak;
        v->due = 0;
        v->marked = 1;
        v->last_marker = now;
    }
    nic_val_build(&f, out);
    v->events = 0;
    v->last_sent = now;
    memcpy(v->kept[v->kept_at], out, NIC_VAL_LEN);
    v->kept_at = (uint8_t)((v->kept_at + 1) % NIC_VAL_KEPT);
    if (v->nkept < NIC_VAL_KEPT)
        v->nkept++;
    spent(v, now);
    return 1;
}

int nic_val_resend(nic_values *v, uint32_t now, uint32_t second, uint8_t out[NIC_VAL_LEN])
{
    for (uint8_t i = 0; i < v->nkept; i++)
        if (get(v->kept[i] + 3, 4) == second) {
            if (!room(v, now))
                return 0;
            memcpy(out, v->kept[i], NIC_VAL_LEN);
            spent(v, now);
            return 1;
        }
    return 0;
}

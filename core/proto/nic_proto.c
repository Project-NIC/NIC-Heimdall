/* The protocol's core; see nic_proto.h. */
#include "nic_proto.h"

#include <string.h>

uint16_t nic_crc16(const uint8_t *b, size_t n)
{
    uint16_t c = 0xFFFF;
    for (size_t i = 0; i < n; i++) {
        c ^= (uint16_t)(b[i] << 8);
        for (int k = 0; k < 8; k++)
            c = (c & 0x8000) ? (uint16_t)((c << 1) ^ 0x1021) : (uint16_t)(c << 1);
    }
    return c;
}

static void put_crc(uint8_t *f, size_t n)
{
    uint16_t c = nic_crc16(f, n);
    f[n] = (uint8_t)c;
    f[n + 1] = (uint8_t)(c >> 8);
}

static int crc_ok(const uint8_t *f, size_t n)
{
    return nic_crc16(f, n) == (uint16_t)(f[n] | f[n + 1] << 8);
}

enum nic_frame_type nic_classify(size_t len)
{
    switch (len) {
    case NIC_SPUR_LEN: return NIC_F_SPUR;
    case NIC_MINI_LEN: return NIC_F_MINI;
    case NIC_TRUNK_LEN: return NIC_F_TRUNK;
    case NIC_CTRL_LEN: return NIC_F_CTRL;
    default: return NIC_BAD;
    }
}

/* ------------------------------------------------------------------ addresses */

int nic_addr_pack(enum nic_bus bus, unsigned type, unsigned number, uint8_t *addr)
{
    if (bus == NIC_MODBUS) {
        if (type < 4 || type > 61 || number > 3)
            return -1;
        *addr = (uint8_t)(type << 2 | number);
        return 0;
    }
    if (type < 1 || type > 14 || number < 1 || number > 15)
        return -1;
    *addr = (uint8_t)(type << 4 | number);
    return 0;
}

int nic_addr_unpack(enum nic_bus bus, uint8_t addr, unsigned *type, unsigned *number)
{
    unsigned t = bus == NIC_MODBUS ? addr >> 2 : addr >> 4;
    unsigned n = bus == NIC_MODBUS ? (addr & 3u) : (addr & 15u);
    uint8_t back;
    if (nic_addr_pack(bus, t, n, &back))
        return -1;
    *type = t;
    *number = n;
    return 0;
}

/* ------------------------------------------------------------------ DATA */

size_t nic_data_build(const nic_header *h, const uint8_t *payload, size_t plen, uint8_t *out)
{
    if (plen != NIC_PAYLOAD && plen != NIC_MINI_PAYLOAD)
        return 0;
    out[0] = h->unix0;
    out[1] = h->frame;
    out[2] = h->addr;
    out[3] = h->slot;
    out[4] = h->kind;
    out[5] = h->status;
    memcpy(out + 6, payload, plen);
    put_crc(out, 6 + plen);
    return 8 + plen;
}

int nic_data_parse(const uint8_t *f, size_t len, nic_header *h, const uint8_t **payload,
                   size_t *plen)
{
    if ((len != NIC_SPUR_LEN && len != NIC_MINI_LEN) || !crc_ok(f, len - 2))
        return -1;
    h->unix0 = f[0];
    h->frame = f[1];
    h->addr = f[2];
    h->slot = f[3];
    h->kind = f[4];
    h->status = f[5];
    *payload = f + 6;
    *plen = len - 8;
    return 0;
}

int nic_trunk_complete(uint8_t buf[NIC_TRUNK_LEN], uint32_t card_second)
{
    uint8_t *spur = buf + 3;
    if (!crc_ok(spur, NIC_SPUR_LEN - 2))
        return -1;
    /* the second whose low byte is the node's unix.0, nearest the card's own */
    uint32_t s = (card_second & ~0xFFu) | spur[0];
    if (s > card_second && s - card_second > 128 && s >= 256)
        s -= 256;
    else if (s < card_second && card_second - s > 128)
        s += 256;
    buf[0] = (uint8_t)(s >> 24);
    buf[1] = (uint8_t)(s >> 16);
    buf[2] = (uint8_t)(s >> 8);
    memset(buf + 41, 0, 5);
    put_crc(buf, 46);
    return 0;
}

int nic_trunk_parse(const uint8_t f[NIC_TRUNK_LEN], uint32_t *second, nic_header *h,
                    const uint8_t **payload)
{
    if (!crc_ok(f, 46))
        return -1;
    *second = (uint32_t)f[0] << 24 | (uint32_t)f[1] << 16 | (uint32_t)f[2] << 8 | f[3];
    h->unix0 = f[3];
    h->frame = f[4];
    h->addr = f[5];
    h->slot = f[6];
    h->kind = f[7];
    h->status = f[8];
    *payload = f + 9;
    return 0;
}

/* ------------------------------------------------------------------ CONTROL */

void nic_ctrl_build(const nic_ctrl *c, uint8_t out[NIC_CTRL_LEN])
{
    out[0] = c->unix0;
    out[1] = c->frame;
    out[2] = c->addr;
    out[3] = c->sub;
    out[4] = c->op;
    out[5] = c->arg0;
    out[6] = c->arg1;
    out[7] = c->arg2;
    out[8] = out[9] = 0;
    put_crc(out, 10);
}

int nic_ctrl_parse(const uint8_t f[NIC_CTRL_LEN], nic_ctrl *c)
{
    if (!crc_ok(f, 10))
        return -1;
    c->unix0 = f[0];
    c->frame = f[1];
    c->addr = f[2];
    c->sub = f[3];
    c->op = f[4];
    c->arg0 = f[5];
    c->arg1 = f[6];
    c->arg2 = f[7];
    return 0;
}

int nic_argus_down(uint8_t f[NIC_CTRL_LEN])
{
    if (f[3] == 0)
        return -1;
    f[2] = f[3];
    f[3] = 0;
    put_crc(f, 10);
    return 0;
}

void nic_argus_up(uint8_t f[NIC_CTRL_LEN], uint8_t argus_addr)
{
    f[3] = f[2];
    f[2] = argus_addr;
    put_crc(f, 10);
}

void nic_register_payload(uint16_t reg, const uint8_t *value, size_t len, uint8_t out[NIC_PAYLOAD])
{
    memset(out, 0, NIC_PAYLOAD);
    out[0] = (uint8_t)reg;
    out[1] = (uint8_t)(reg >> 8);
    memcpy(out + 2, value, len > NIC_PAYLOAD - 2 ? NIC_PAYLOAD - 2 : len);
}

uint16_t nic_register_of(const uint8_t payload[NIC_PAYLOAD])
{
    return (uint16_t)(payload[0] | payload[1] << 8);
}

/* ------------------------------------------------------------------ the roster */

void nic_roster_init(nic_roster *r)
{
    memset(r->by_addr, 0xFF, sizeof r->by_addr);
    r->n = 0;
}

uint16_t nic_roster_put(nic_roster *r, const nic_unit *u)
{
    uint16_t i = r->by_addr[u->bus][u->addr];
    if (i == NIC_NONE) {
        if (r->n == NIC_ROSTER_MAX)
            return NIC_NONE;
        i = r->n++;
    }
    r->rows[i] = *u;
    r->by_addr[u->bus][u->addr] = i;
    return i;
}

uint16_t nic_roster_drop(nic_roster *r, enum nic_bus bus, uint8_t addr)
{
    uint16_t top = r->by_addr[bus][addr];
    if (top == NIC_NONE)
        return 0;
    uint8_t gone[NIC_ROSTER_MAX] = {0};
    uint16_t moved[NIC_ROSTER_MAX];
    gone[top] = 1;
    for (int changed = 1; changed;) {          /* everything under it, at any depth */
        changed = 0;
        for (uint16_t i = 0; i < r->n; i++)
            if (!gone[i] && r->rows[i].parent != NIC_NONE && gone[r->rows[i].parent])
                gone[i] = changed = 1;
    }
    uint16_t m = 0, dropped = 0;
    for (uint16_t i = 0; i < r->n; i++) {
        if (gone[i]) {
            moved[i] = NIC_NONE;
            dropped++;
        } else {
            moved[i] = m;
            r->rows[m++] = r->rows[i];
        }
    }
    r->n = m;
    memset(r->by_addr, 0xFF, sizeof r->by_addr);
    for (uint16_t i = 0; i < m; i++) {
        if (r->rows[i].parent != NIC_NONE)
            r->rows[i].parent = moved[r->rows[i].parent];
        r->by_addr[r->rows[i].bus][r->rows[i].addr] = i;
    }
    return dropped;
}

/* The head's side of the ModBus tunnel — see nic_tunnel.h. */
#include "nic_tunnel.h"

#include <string.h>

uint16_t nic_mb_crc(const uint8_t *b, size_t n)
{
    uint16_t c = 0xFFFF;
    while (n--) {
        c ^= *b++;
        for (int i = 0; i < 8; i++)
            c = (uint16_t)(c & 1 ? (c >> 1) ^ 0xA001 : c >> 1);
    }
    return c;
}

static size_t close_pkg(uint8_t *p, size_t n)
{
    uint16_t c = nic_mb_crc(p, n);
    p[n] = (uint8_t)c;
    p[n + 1] = (uint8_t)(c >> 8);
    return n + 2;
}

size_t nic_mb_read(uint8_t slave, uint16_t start, uint8_t count, uint8_t out[NIC_TUN_MAX])
{
    if (!count || count > NIC_TUN_READ_MAX)
        return 0;
    uint8_t p[] = {slave, NIC_MB_READ, (uint8_t)(start >> 8), (uint8_t)start, 0, count};
    memcpy(out, p, sizeof p);
    return close_pkg(out, sizeof p);
}

size_t nic_mb_write1(uint8_t slave, uint16_t reg, uint16_t value, uint8_t out[NIC_TUN_MAX])
{
    uint8_t p[] = {slave, NIC_MB_WRITE1, (uint8_t)(reg >> 8), (uint8_t)reg, (uint8_t)(value >> 8),
                   (uint8_t)value};
    memcpy(out, p, sizeof p);
    return close_pkg(out, sizeof p);
}

size_t nic_mb_writen(uint8_t slave, uint16_t start, const uint16_t *values, uint8_t count,
                     uint8_t out[NIC_TUN_MAX])
{
    if (!count || count > NIC_TUN_WRITE_MAX)
        return 0;
    uint8_t p[] = {slave, NIC_MB_WRITEN, (uint8_t)(start >> 8), (uint8_t)start, 0, count,
                   (uint8_t)(2 * count)};
    memcpy(out, p, sizeof p);
    for (uint8_t i = 0; i < count; i++) {
        out[7 + 2 * i] = (uint8_t)(values[i] >> 8);      /* ModBus is big-endian */
        out[8 + 2 * i] = (uint8_t)values[i];
    }
    return close_pkg(out, 7u + 2u * count);
}

void nic_tun_init(nic_tunnel *t)
{
    memset(t, 0, sizeof *t);
}

static nic_tun_slot *slot_of(nic_tunnel *t, uint8_t bridge, int make)
{
    nic_tun_slot *free_one = 0;
    for (unsigned i = 0; i < NIC_TUN_BRIDGES; i++) {
        if (t->s[i].open && t->s[i].bridge == bridge)
            return &t->s[i];
        if (!t->s[i].open && !free_one)
            free_one = &t->s[i];
    }
    return make ? free_one : 0;
}

static void payload(const nic_tun_slot *s, uint8_t out[32])
{
    memset(out, 0, 32);
    out[0] = s->len;
    memcpy(out + 1, s->pkg, s->len);
}

int nic_tun_send(nic_tunnel *t, uint8_t bridge, const uint8_t *pkg, size_t len, uint32_t now,
                 uint8_t out[32])
{
    if (len < 4 || len > NIC_TUN_MAX || nic_mb_crc(pkg, len - 2) != (uint16_t)(pkg[len - 2] | pkg[len - 1] << 8)
        || slot_of(t, bridge, 0))
        return -1;
    nic_tun_slot *s = slot_of(t, bridge, 1);
    if (!s)
        return -1;
    s->bridge = bridge;
    s->open = 1;
    s->tries = 1;
    s->sent = now;
    s->len = (uint8_t)len;
    memcpy(s->pkg, pkg, len);
    payload(s, out);
    return 0;
}

/* The package sent again once, else given up. */
static enum nic_tun_result again_or_fail(nic_tunnel *t, nic_tun_slot *s, uint32_t now, uint8_t again[32])
{
    if (s->tries < 2) {
        s->tries++;
        s->sent = now;
        payload(s, again);
        return NIC_TUN_WAIT;
    }
    s->open = 0;
    t->failures++;
    return NIC_TUN_FAILED;
}

enum nic_tun_result nic_tun_reply(nic_tunnel *t, uint8_t bridge, const uint8_t p[32], uint32_t now,
                                  nic_tun_answer *a, uint8_t again[32])
{
    nic_tun_slot *s = slot_of(t, bridge, 0);
    if (!s) {
        t->strays++;
        return NIC_TUN_STRAY;
    }
    uint8_t len = p[0];
    const uint8_t *r = p + 1;
    int ok = len >= 5 && len <= NIC_TUN_MAX
             && nic_mb_crc(r, len - 2u) == (uint16_t)(r[len - 2] | r[len - 1] << 8)
             && r[0] == s->pkg[0] && (r[1] & 0x7F) == s->pkg[1];
    if (ok && (r[1] & 0x80))
        ok = len == 5;                                  /* an exception: address · function · code */
    if (!ok) {
        t->bad++;
        return again_or_fail(t, s, now, again);
    }
    memset(a, 0, sizeof *a);
    a->slave = r[0];
    a->function = r[1] & 0x7F;
    s->open = 0;
    if (r[1] & 0x80) {
        a->exception = r[2];
        return NIC_TUN_EXCEPTION;
    }
    a->len = (uint8_t)(len - 4);
    memcpy(a->data, r + 2, a->len);
    return NIC_TUN_OK;
}

enum nic_tun_result nic_tun_error(nic_tunnel *t, uint8_t bridge, uint32_t now, uint8_t again[32])
{
    nic_tun_slot *s = slot_of(t, bridge, 0);
    if (!s) {
        t->strays++;
        return NIC_TUN_STRAY;
    }
    return again_or_fail(t, s, now, again);
}

enum nic_tun_result nic_tun_tick(nic_tunnel *t, uint32_t now, uint8_t *bridge, uint8_t again[32])
{
    for (unsigned i = 0; i < NIC_TUN_BRIDGES; i++) {
        nic_tun_slot *s = &t->s[i];
        if (s->open && now - s->sent >= NIC_TUN_TIMEOUT) {
            *bridge = s->bridge;
            return again_or_fail(t, s, now, again);
        }
    }
    return NIC_TUN_WAIT;
}

int nic_mb_registers(const nic_tun_answer *a, uint8_t count, uint16_t *values)
{
    if (a->function != NIC_MB_READ || a->len != 1u + 2u * count || a->data[0] != 2u * count)
        return -1;
    for (uint8_t i = 0; i < count; i++)
        values[i] = (uint16_t)(a->data[1 + 2 * i] << 8 | a->data[2 + 2 * i]);
    return count;
}

int nic_mb_confirms(const nic_tun_answer *a, const uint8_t *pkg, size_t len)
{
    if (len < 8 || a->slave != pkg[0] || a->function != pkg[1] || a->len != 4)
        return 0;
    if (pkg[1] != NIC_MB_WRITE1 && pkg[1] != NIC_MB_WRITEN)
        return 0;
    return !memcmp(a->data, pkg + 2, 4);                /* FC06 echoes reg · value, FC16 start · count */
}

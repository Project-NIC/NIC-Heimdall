/* The sweep; see nic_sweep.h. */
#include "nic_sweep.h"

#include <string.h>

uint16_t nic_tag(const uint8_t uid[12])
{
    return nic_crc16(uid, 12);
}

void nic_sweep_unit_init(nic_sweep_unit *u, const uint8_t uid[12], uint8_t addr, int enrolled)
{
    memset(u, 0, sizeof *u);
    memcpy(u->uid, uid, 12);
    u->tag = nic_tag(uid);
    u->addr = addr;
    u->enrolled = (uint8_t)(enrolled != 0);
}

void nic_sweep_unit_discover(nic_sweep_unit *u, uint32_t now)
{
    if (u->enrolled || u->answered || u->sending)
        return;
    u->attempt = 0;
    u->delay = u->tag;
    u->waiting = 1;
    u->due = now + (uint32_t)u->delay * NIC_SWEEP_STEP_US;
}

void nic_sweep_unit_heard(nic_sweep_unit *u, uint32_t now)
{
    if (u->waiting)
        u->due = now + (uint32_t)u->delay * NIC_SWEEP_STEP_US;
}

int nic_sweep_unit_due(nic_sweep_unit *u, uint32_t now, int bus_busy, uint8_t out[NIC_CTRL_LEN])
{
    if (!u->waiting || now < u->due)
        return 0;
    if (bus_busy) {                       /* listen before talking: wait for quiet, then again */
        u->due = now + (uint32_t)u->delay * NIC_SWEEP_STEP_US;
        return 0;
    }
    nic_ctrl c = {(uint8_t)u->tag, (uint8_t)(u->tag >> 8), u->addr, 0, NIC_DISCOVER, 0, 0, 0};
    nic_ctrl_build(&c, out);
    u->waiting = 0;
    u->sending = 1;
    return 1;
}

void nic_sweep_unit_echo(nic_sweep_unit *u, int whole, uint32_t now)
{
    u->sending = 0;
    if (whole) {
        u->answered = 1;
        return;
    }
    uint8_t b[13];                        /* a new wait: CRC-16 over the UID and the attempt */
    memcpy(b, u->uid, 12);
    b[12] = ++u->attempt;
    u->delay = nic_crc16(b, 13);
    u->waiting = 1;
    u->due = now + (uint32_t)u->delay * NIC_SWEEP_STEP_US;
}

int nic_sweep_unit_assign(nic_sweep_unit *u, const uint8_t f[NIC_CTRL_LEN], uint8_t ack[NIC_CTRL_LEN])
{
    nic_ctrl c;
    if (nic_ctrl_parse(f, &c) || c.op != NIC_ASSIGN_ADDR || (uint16_t)(c.unix0 | c.frame << 8) != u->tag)
        return 0;
    u->addr = c.arg0;
    u->slot = c.arg1;
    u->enrolled = 1;
    u->waiting = 0;
    nic_ctrl a = {0, 0, u->addr, 0, NIC_ACK, NIC_ASSIGN_ADDR, 0, 0};
    nic_ctrl_build(&a, ack);
    return 1;
}

/* ------------------------------------------------------------------ the host */

static void discover(uint8_t out[NIC_CTRL_LEN])
{
    nic_ctrl c = {0, 0, NIC_BCAST, 0, NIC_DISCOVER, 0, 0, 0};
    nic_ctrl_build(&c, out);
}

void nic_sweep_host_start(nic_sweep_host *h, uint32_t now, uint8_t out[NIC_CTRL_LEN])
{
    memset(h, 0, sizeof *h);
    h->windows = 1;
    h->quiet_since = now;
    discover(out);
}

void nic_sweep_host_rx(nic_sweep_host *h, const uint8_t *f, size_t len, uint32_t now)
{
    nic_ctrl c;
    h->quiet_since = now;
    h->heard_in_window = 1;
    if (len != NIC_CTRL_LEN || nic_ctrl_parse(f, &c) || c.op != NIC_DISCOVER || c.addr == NIC_BCAST)
        return;
    uint16_t tag = (uint16_t)(c.unix0 | c.frame << 8);
    for (uint8_t i = 0; i < h->n; i++)
        if (h->found[i].tag == tag)
            return;
    if (h->n < NIC_SWEEP_MAX) {
        h->found[h->n].tag = tag;
        h->found[h->n].addr = c.addr;
        h->n++;
    }
}

int nic_sweep_host_tick(nic_sweep_host *h, uint32_t now, uint8_t out[NIC_CTRL_LEN])
{
    if (h->done || now - h->quiet_since < NIC_SWEEP_SILENCE_US)
        return 0;
    if (h->windows >= 2 && !h->heard_in_window) {   /* a second silent window: complete */
        h->done = 1;
        return 0;
    }
    h->windows++;
    h->heard_in_window = 0;
    h->quiet_since = now;
    discover(out);
    return 1;
}

void nic_assign(uint16_t tag, uint8_t addr_said, uint8_t addr, uint8_t slot, uint8_t out[NIC_CTRL_LEN])
{
    nic_ctrl c = {(uint8_t)tag, (uint8_t)(tag >> 8), addr_said, 0, NIC_ASSIGN_ADDR, addr, slot, 0};
    nic_ctrl_build(&c, out);
}

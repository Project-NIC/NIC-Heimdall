/* The sweep, as core/PROTOCOL.md §2 fixes it: DISCOVER broadcast; every unit not enrolled answers
 * once, its tag — CRC-16 over the whole 96-bit UID — in the two time bytes and its second tag —
 * CRC-16 over the UID turned by six bytes — in the two reserve bytes, after tag × 10 µs, listening
 * before it talks and starting its wait again when it hears the bus; a collision, a bad CRC on its
 * own echo, sends it to a new wait, CRC-16 over the UID turned by the attempt's bytes; the host closes
 * a window on 700 ms of silence, broadcasts once more, and a second silent window ends the round;
 * ASSIGN_ADDR by both tags. Time is the firmware's, in µs; the wire is its. */
#ifndef NIC_SWEEP_H
#define NIC_SWEEP_H

#include <stddef.h>
#include <stdint.h>

#include "nic_proto.h"

#define NIC_SWEEP_STEP_US     10u
#define NIC_SWEEP_SILENCE_US  700000u

uint16_t nic_tag(const uint8_t uid[12]);
/* CRC-16 over the UID turned left by `r` bytes: the second tag at 6, a wait after the k-th
 * collision at 1 + (k − 1) mod 11 — a turn breaks the CRC's linearity, so two UIDs of one tag part. */
uint16_t nic_tag_turned(const uint8_t uid[12], unsigned r);

typedef struct {
    uint8_t uid[12];
    uint16_t tag, tag2, delay; /* delay in steps of 10 µs */
    uint8_t addr, slot;        /* the default or the persisted name; the slot once assigned */
    uint8_t enrolled, answered, waiting, sending, attempt;
    uint32_t due;
} nic_sweep_unit;

void nic_sweep_unit_init(nic_sweep_unit *u, const uint8_t uid[12], uint8_t addr, int enrolled);
/* A DISCOVER heard at `now`: a unit not enrolled, not yet answered, starts its wait. */
void nic_sweep_unit_discover(nic_sweep_unit *u, uint32_t now);
/* Someone else's frame on the bus, ended at `now`: a waiting unit starts its wait again. */
void nic_sweep_unit_heard(nic_sweep_unit *u, uint32_t now);
/* Its wait over and the bus quiet: 1 and the reply to send. */
int nic_sweep_unit_due(nic_sweep_unit *u, uint32_t now, int bus_busy, uint8_t out[NIC_CTRL_LEN]);
/* Its own echo, whole or not, at the end of its frame. */
void nic_sweep_unit_echo(nic_sweep_unit *u, int whole, uint32_t now);
/* An ASSIGN_ADDR: 1 when its tag is this unit's — the address and slot taken, the ACK built
 * under the new address. */
int nic_sweep_unit_assign(nic_sweep_unit *u, const uint8_t f[NIC_CTRL_LEN], uint8_t ack[NIC_CTRL_LEN]);

#define NIC_SWEEP_MAX 64u

typedef struct {
    uint16_t tag, tag2;
    uint8_t addr;              /* TYPE|NUM as the unit said it */
} nic_found;

typedef struct {
    nic_found found[NIC_SWEEP_MAX];
    uint8_t n, windows, heard_in_window, done;
    uint32_t quiet_since;
} nic_sweep_host;

void nic_sweep_host_start(nic_sweep_host *h, uint32_t now, uint8_t discover[NIC_CTRL_LEN]);
/* Anything on the wire at `now` keeps the window open; a whole DISCOVER reply is collected. */
void nic_sweep_host_rx(nic_sweep_host *h, const uint8_t *f, size_t len, uint32_t now);
/* 1 when DISCOVER goes out again; h->done when the round is over. */
int nic_sweep_host_tick(nic_sweep_host *h, uint32_t now, uint8_t discover[NIC_CTRL_LEN]);
void nic_assign(const nic_found *f, uint8_t addr, uint8_t slot, uint8_t out[NIC_CTRL_LEN]);

#endif

/* The card's data path without its hardware, as bifrost/FIRMWARE.md §6–§8 describe it: the slot
 * check, the circular buffer per unit — a frame written when it arrives, read at its own period
 * plus the card's delay, as the frame or as a FILLER with its reason — the segment table, the
 * PORTS frame, and an Argus's tiling of four sondes under one NUMBER. The firmware gives it the
 * arrival times and the periods; the timers, DMA and USARTs stay the firmware's. */
#ifndef NIC_CARD_H
#define NIC_CARD_H

#include <stddef.h>
#include <stdint.h>

#include "nic_proto.h"

#define NIC_BIFROST_DELAY 96u
#define NIC_ARGUS_DELAY   16u

enum nic_filler {               /* a FILLER's reason, payload byte 0 */
    NIC_FILL_MISSING = 1,       /* silent in its slot */
    NIC_FILL_EMPTY = 2,         /* the slot heard, no data */
    NIC_FILL_CORRUPT = 3,       /* two CRC misses */
    NIC_FILL_SLIPPED = 4,       /* the unix.0 · frame check failed */
    NIC_FILL_UNLABELLED = 5     /* the card has no second */
};

/* The slot's window: round start + (slot − 1) × width, ± two guard bytes, in the firmware's
 * ticks. 1 inside. */
int nic_slot_inside(uint32_t arrival, uint32_t round_start, uint8_t slot, uint32_t width,
                    uint32_t byte_time);

/* One period's place in a unit's buffer. */
typedef struct {
    uint32_t period;            /* second · rows + frame, the period it belongs to */
    uint8_t state;              /* see nic_card.c */
    uint8_t buf[NIC_TRUNK_LEN]; /* the node's frame lands 3 B in, as the DMA lays it */
} nic_cell;

typedef struct {
    nic_cell *cells;
    uint16_t depth, delay, rows;
    uint8_t addr, slot;         /* the unit's, from the port table */
    uint8_t wrong;              /* frames in a row in the wrong place */
} nic_unitbuf;

void nic_unitbuf_init(nic_unitbuf *u, nic_cell *cells, uint16_t depth, uint16_t delay,
                      uint16_t rows, uint8_t addr, uint8_t slot);

enum nic_arrival {
    NIC_STORED = 0,
    NIC_ASK_RESEND,             /* the first CRC miss: ask RESEND frame · unix.0 once */
    NIC_GIVEN_UP,               /* the second: the period leaves as a FILLER, arrived corrupt */
    NIC_WRONG,                  /* a whole frame in the wrong place or slot: dropped, counted */
    NIC_LADDER                  /* the third such in a row: the unit goes on the ladder */
};

/* A spur frame of `len` at buf + 3 for `period` — the period the slot's time says, or the one a
 * RESEND asked for. `inside` is the slot check's verdict on its arrival. */
enum nic_arrival nic_unitbuf_arrive(nic_unitbuf *u, uint32_t period, const uint8_t *frame,
                                    size_t len, int inside);
/* The slot was heard but held no data frame (a CONTROL, junk). */
void nic_unitbuf_heard(nic_unitbuf *u, uint32_t period);
/* What leaves at period `now`: the frame of now − delay, stamped with its second, or a FILLER.
 * `labelled` 0 while the card has no second: every period leaves unlabelled. */
void nic_unitbuf_release(nic_unitbuf *u, uint32_t now, int labelled, uint8_t out[NIC_TRUNK_LEN]);

/* ------------------------------------------------------------------ the card's own frames */

typedef struct {
    uint8_t addr, port, rung, status;   /* status: 0 running · 1 unranged · 2 ladder · 3 dead ·
                                           4 unknown board · 5 port faulted */
} nic_seg_row;

/* kind 4: eight rows of four bytes in slot order, zeros for an empty slot. */
void nic_segment_table(const nic_seg_row rows[8], uint8_t out[NIC_PAYLOAD]);

/* kind 7, the card's map: five sockets' VBUS and CURRENT raw (up first), the leak watch of down
 * ports 1–4, the card's temperature in 0,01 °C, two reserve. */
void nic_ports_payload(const uint16_t vbus[5], const uint16_t current[5], const uint16_t leak[4],
                       int16_t temp, uint8_t out[NIC_PAYLOAD]);

/* ------------------------------------------------------------------ an Argus's tiling */

typedef struct {
    uint8_t payload[NIC_PAYLOAD];
    uint8_t present;            /* bit per position */
    uint8_t flags;              /* the OR of the sondes' ALARM, SUPPLY, FAULT */
    uint8_t worst;              /* the most severe state code */
} nic_tile;

void nic_tile_init(nic_tile *t);
/* A sonde's 8 B payload and its status at its position 0..3. */
void nic_tile_place(nic_tile *t, unsigned pos, uint8_t status, const uint8_t payload[NIC_MINI_PAYLOAD]);
/* The NUMBER's status: the flags OR'd, the most severe code, a seated position with no frame as
 * SENSOR. `seated` the positions the segment table fills. */
uint8_t nic_tile_status(const nic_tile *t, uint8_t seated);

#endif

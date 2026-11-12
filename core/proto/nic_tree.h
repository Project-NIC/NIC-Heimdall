/* The head's station tree, as ../../mayak/FIRMWARE.md §7 and ../PROTOCOL.md §2 and §10 fix it.
 * Each card's `kind` 4 table — eight rows, one a slot, `unit address · port · rung · status` — and
 * each Argus's, the same shape, its rows 0–3 the positions under the NUMBER that sent it and 4–7
 * under its second NUMBER, are composed into one list: card → slot → unit → position → sonde. The
 * tags, read once per slot with `GET`, name the boxes; a box of several slots has one tag on
 * several rows. Against the mirror the head then decides each row's NUMBER — confirmed, moved
 * back, renumbered, a stranger, new, or put to the human as a replacement — and the decisions go
 * down as `ASSIGN_ADDR` by tag through the card. The roster is written from the tree, never typed. */
#ifndef NIC_TREE_H
#define NIC_TREE_H

#include <stddef.h>
#include <stdint.h>

#include "nic_proto.h"

#define NIC_TREE_MAX 256u

typedef struct {
    uint8_t bus, addr;          /* TYPE|NUM as the table reports it */
    uint8_t card;               /* the card's address on the trunk */
    uint8_t argus;              /* a sonde: the NUMBER whose table holds it; 0 for a card's unit */
    uint8_t slot;               /* a card's unit: 1..8; a sonde: its row 0..7 */
    uint8_t port, rung, status; /* the row's bytes: a spur port or an Argus's segment */
    uint8_t tagged;
    uint16_t tag, tag2;
} nic_seen;

typedef struct {
    nic_seen u[NIC_TREE_MAX];
    uint16_t n;
} nic_tree;

void nic_tree_init(nic_tree *t);
/* A card's table: its rows, and the sondes under them, replace what it sent before. -1 full. */
int nic_tree_card(nic_tree *t, uint8_t card, const uint8_t table[NIC_PAYLOAD]);
/* An Argus's table, sent from its first NUMBER `argus` on `card`. */
int nic_tree_argus(nic_tree *t, uint8_t card, uint8_t argus, const uint8_t table[NIC_PAYLOAD]);
/* The tags a `GET` per slot read; `argus` 0 for a card's slot. -1 no such row. */
int nic_tree_tags(nic_tree *t, uint8_t card, uint8_t argus, uint8_t slot, uint16_t tag, uint16_t tag2);
/* The sonde whose 8 B ride position `pos` of `number`'s frames on `card`, or NULL. */
const nic_seen *nic_tree_sonde(const nic_tree *t, uint8_t card, uint8_t number, unsigned pos);

/* ------------------------------------------------------------------ the numbers */

typedef struct {
    uint8_t bus, addr;          /* the NUMBER the mirror holds */
    uint16_t tag, tag2;
} nic_mirror_row;

enum nic_verdict {
    NIC_V_CONFIRMED = 0,        /* a tag it knows on the number the mirror holds: nothing sent */
    NIC_V_MOVED,                /* a tag it knows on another number: back to the mirror's */
    NIC_V_RENUMBERED,           /* an unknown tag sharing its number with another: the lowest free */
    NIC_V_STRANGER,             /* an unknown tag on a number not the default: the lowest free */
    NIC_V_NEW,                  /* an unknown tag on the default: the lowest free */
    NIC_V_REPLACED,             /* a fresh unit, a missing row of its type: the human asked, one each */
    NIC_V_FULL,                 /* no number of its type left */
    NIC_V_UNTAGGED              /* its tags not read yet: nothing decided */
};

typedef struct {
    uint16_t seen;              /* the tree's row */
    uint8_t verdict, to;        /* the address it is to stand on */
    uint8_t ask;                /* 1: sent only on the human's word */
} nic_decision;

/* Every row of the tree against the mirror. `service` 0 is the initial sweep on an empty mirror —
 * every row new, numbered from the bottom; 1 the service sweep. `missing[i]` set for each mirror
 * row nobody answered. One decision per tree row, in the tree's order; the count returned. */
size_t nic_tree_number(const nic_tree *t, const nic_mirror_row *m, size_t nm, int service,
                       nic_decision *out, uint8_t *missing);
/* The ASSIGN_ADDR a decision sends through the card: by the row's tags, its own slot. */
void nic_tree_assign(const nic_tree *t, const nic_decision *d, uint8_t out[NIC_CTRL_LEN]);
/* The card confirmed it: the tree's row stands on its new number. */
void nic_tree_apply(nic_tree *t, const nic_decision *d);
/* The roster from the tree: the cards on the trunk, their units under them, each sonde under the
 * NUMBER that carries its position (port its segment, slot its position). */
void nic_tree_roster(const nic_tree *t, nic_roster *r);

#endif

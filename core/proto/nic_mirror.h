/* The head's mirror, as ../../mayak/FIRMWARE.md §7 and §14 fix it: the station's configuration as
 * rows and cells. A row is everything enrolled — a card, a unit, an Argus NUMBER, a sonde, a
 * ModBus sensor — 512 of them. A cell is `key 4 B · value 10 B · CRC-16 2 B`, the key the row
 * (10 bits), a page (4 bits), a read-only flag, a deletion flag and the register (16 bits). The
 * row's own fields are the registers 0xFE00–0xFE03; entry i of a table register r (r < 0x80,
 * i < 128) — the Palatine roster, Marconi's sounders — is the register 0x8000 | r«7 | i.
 *
 * A value takes pages: page 0 carries its length, the CRC-16 of the whole value and its first 7 B,
 * every other page 10 B more, up to 157 B; the pages are written last first, page 0 closing them,
 * so a value cut short by a lost supply is never taken — the one before it stands.
 *
 * The cells are appended to one half of the flash area; when it is full the live set is written
 * into the other half under the next generation, closed by a marker, and the old half erased.
 * At boot the half with the newer closed generation is replayed. Only a confirmed value reaches
 * the flash: an edited or a pending one is held in RAM, and a reboot drops it. Nothing is
 * allocated; the flash is the firmware's calls.
 *
 * The identity, 0xFE00, is 11 B: bus · TYPE|NUM · tag 2 B · second tag 2 B · card · port · slot or
 * position · parent (the Argus NUMBER of a sonde, the Palatine NUMBER of a ModBus sensor) · arm. */
#ifndef NIC_MIRROR_H
#define NIC_MIRROR_H

#include <stddef.h>
#include <stdint.h>

#include "nic_tree.h"

#define NIC_MIR_ROWS     512u
#define NIC_MIR_SECTOR   4096u
#define NIC_MIR_HALF     (128u * 1024u)                 /* bytes a half; the area is two */
#define NIC_MIR_CELLS    (NIC_MIR_HALF / 16u)           /* cells a half holds */
#define NIC_MIR_MAX      157u                           /* a value's bytes */
#define NIC_MIR_PENDING  64u                            /* values edited or sent, not confirmed */

enum { NIC_REG_IDENTITY = 0xFE00, NIC_REG_RULES, NIC_REG_SEND, NIC_REG_MEASURED };
#define NIC_REG_TABLE(r, i) ((uint16_t)(0x8000u | ((unsigned)(r) << 7) | (unsigned)(i)))
#define NIC_REG_ROW      0xFFFFu                        /* a deletion of the whole row */

enum nic_mir_state { NIC_MIR_NONE = 0, NIC_MIR_CONFIRMED, NIC_MIR_EDITED, NIC_MIR_SENT };

typedef struct {
    int (*read)(void *ctx, uint32_t addr, uint8_t *buf, uint32_t len);
    int (*program)(void *ctx, uint32_t addr, const uint8_t buf[16]);   /* one cell */
    int (*erase)(void *ctx, uint32_t addr);                            /* one sector */
    void *ctx;
} nic_flash;

typedef struct {
    uint32_t key;                  /* row | page«10 | ro«14 | reg«16 */
    uint8_t value[10];
} nic_mir_cell;

typedef struct {
    uint16_t row, reg;
    uint8_t state, len;
    uint8_t value[NIC_MIR_MAX];
} nic_mir_pending;

typedef struct {
    nic_flash flash;
    uint8_t half;                  /* the half being appended to */
    uint32_t generation, next;     /* the cell index of the next append */
    nic_mir_cell live[NIC_MIR_CELLS];
    uint32_t n;
    nic_mir_pending pend[NIC_MIR_PENDING];
    uint32_t compactions, torn;    /* torn: values or cells passed over at boot */
} nic_mirror;

/* At boot: the newer closed half replayed; an empty area made a fresh one. */
int nic_mir_load(nic_mirror *m, const nic_flash *flash);

/* A confirmed value written: a row's own field, a measured row, a value read back, or what a
 * unit confirmed. ro marks what the station only reads. */
int nic_mir_put(nic_mirror *m, uint16_t row, uint16_t reg, int ro, const uint8_t *v, uint8_t len);
/* A register's confirmed value: its length, -1 none; *ro its flag. */
int nic_mir_value(const nic_mirror *m, uint16_t row, uint16_t reg, uint8_t *v, int *ro);
/* A register gone, or with NIC_REG_ROW the whole row. */
int nic_mir_remove(nic_mirror *m, uint16_t row, uint16_t reg);
/* The lowest row with nothing in it, -1 none left. */
int nic_mir_free_row(const nic_mirror *m);

/* The three states: edited on the phone, sent to the unit, confirmed by it — written then. */
int nic_mir_edit(nic_mirror *m, uint16_t row, uint16_t reg, const uint8_t *v, uint8_t len);
int nic_mir_sent(nic_mirror *m, uint16_t row, uint16_t reg);
int nic_mir_confirm(nic_mirror *m, uint16_t row, uint16_t reg);
void nic_mir_refused(nic_mirror *m, uint16_t row, uint16_t reg);
/* What the phone shows: the edited or sent value over the confirmed one, and which it is. */
enum nic_mir_state nic_mir_get(const nic_mirror *m, uint16_t row, uint16_t reg, uint8_t *v, uint8_t *len);

/* The rows the station tree numbers against (nic_tree.c): bus, NUMBER and tags from each row's
 * identity; `rows[i]` the mirror row of out[i]. Their count. */
size_t nic_mir_tree(const nic_mirror *m, nic_mirror_row *out, uint16_t *rows, size_t max);

#endif

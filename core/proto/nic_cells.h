/* The configuration cells, as core/PROTOCOL.md §7 fixes them: in the MCU's own flash, one 8 KB
 * sector of 512 cells of 16 B — the H5's quad-word, one program operation each — appended, the
 * last good cell per id read, the live set rewritten when the sector is full and once a year. A
 * cell is id 2 B · value 12 B · CRC-16 2 B; erased flash, all 0xFF, is a free cell. The persisted
 * set carries one check over the whole of it, in a cell of its own. The flash is the firmware's,
 * reached through callbacks. */
#ifndef NIC_CELLS_H
#define NIC_CELLS_H

#include <stddef.h>
#include <stdint.h>

#define NIC_CELL        16u
#define NIC_CELL_VALUE  12u
#define NIC_CELLS       512u
#define NIC_CELL_SET    0xFFFEu      /* the set's check: count 1 · reserved 3 · CRC-32 4 · reserved 4 */

typedef struct {
    int (*read)(void *ctx, uint32_t offset, uint8_t *b, size_t n);
    int (*program)(void *ctx, uint32_t offset, const uint8_t cell[NIC_CELL]);
    int (*erase)(void *ctx);
    void *ctx;
} nic_cells_io;

typedef struct {
    nic_cells_io io;
    uint16_t next;                      /* the first cell free to append to */
    uint16_t live_n;
    uint8_t live[NIC_CELLS][NIC_CELL];  /* scratch for a rewrite */
} nic_cells;

/* The sector read: the append point found past every cell written, good or torn. */
int nic_cells_open(nic_cells *c, const nic_cells_io *io);
/* The last good value of an id: 0 found, -1 none. */
int nic_cells_get(nic_cells *c, uint16_t id, uint8_t value[NIC_CELL_VALUE]);
/* Append; a full sector is erased and its live set rewritten first. */
int nic_cells_put(nic_cells *c, uint16_t id, const uint8_t value[NIC_CELL_VALUE]);
/* The live set rewritten — the yearly refresh, and what a full sector does. */
int nic_cells_rewrite(nic_cells *c);
/* The persisted set: each id's cell, then the check over them all, in the order given. */
int nic_cells_put_set(nic_cells *c, const uint16_t *ids, const uint8_t (*values)[NIC_CELL_VALUE],
                      size_t n);
/* 0 when every id is there and the check over their last values holds; -1 is no set: boot fresh. */
int nic_cells_get_set(nic_cells *c, const uint16_t *ids, uint8_t (*values)[NIC_CELL_VALUE], size_t n);

#endif

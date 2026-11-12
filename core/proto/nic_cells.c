/* The configuration cells; see nic_cells.h. */
#include "nic_cells.h"

#include <string.h>

#include "nic_load.h"
#include "nic_proto.h"

static int erased(const uint8_t *b)
{
    for (unsigned i = 0; i < NIC_CELL; i++)
        if (b[i] != 0xFF)
            return 0;
    return 1;
}

static int good(const uint8_t *b, uint16_t *id)
{
    uint16_t crc = nic_crc16(b, 14);
    if (erased(b) || (uint16_t)(b[14] | b[15] << 8) != crc)
        return 0;
    *id = (uint16_t)(b[0] | b[1] << 8);
    return 1;
}

int nic_cells_open(nic_cells *c, const nic_cells_io *io)
{
    uint8_t b[NIC_CELL];
    c->io = *io;
    c->next = 0;
    for (uint16_t i = 0; i < NIC_CELLS; i++) {
        if (c->io.read(c->io.ctx, (uint32_t)i * NIC_CELL, b, NIC_CELL))
            return -1;
        if (!erased(b))
            c->next = (uint16_t)(i + 1);       /* past every cell written, a torn one too */
    }
    return 0;
}

int nic_cells_get(nic_cells *c, uint16_t id, uint8_t value[NIC_CELL_VALUE])
{
    uint8_t b[NIC_CELL];
    uint16_t k;
    for (uint16_t i = c->next; i-- > 0;) {
        if (c->io.read(c->io.ctx, (uint32_t)i * NIC_CELL, b, NIC_CELL))
            return -1;
        if (good(b, &k) && k == id) {
            memcpy(value, b + 2, NIC_CELL_VALUE);
            return 0;
        }
    }
    return -1;
}

static int program(nic_cells *c, uint16_t id, const uint8_t value[NIC_CELL_VALUE])
{
    uint8_t b[NIC_CELL];
    b[0] = (uint8_t)id;
    b[1] = (uint8_t)(id >> 8);
    memcpy(b + 2, value, NIC_CELL_VALUE);
    uint16_t crc = nic_crc16(b, 14);
    b[14] = (uint8_t)crc;
    b[15] = (uint8_t)(crc >> 8);
    if (c->io.program(c->io.ctx, (uint32_t)c->next * NIC_CELL, b))
        return -1;
    c->next++;
    return 0;
}

int nic_cells_rewrite(nic_cells *c)
{
    uint8_t b[NIC_CELL];
    uint16_t k;
    c->live_n = 0;
    for (uint16_t i = c->next; i-- > 0;) {     /* newest first: the first seen of an id is its last */
        if (c->io.read(c->io.ctx, (uint32_t)i * NIC_CELL, b, NIC_CELL))
            return -1;
        if (!good(b, &k))
            continue;
        int seen = 0;
        for (uint16_t j = 0; j < c->live_n && !seen; j++)
            seen = c->live[j][0] == b[0] && c->live[j][1] == b[1];
        if (!seen)
            memcpy(c->live[c->live_n++], b, NIC_CELL);
    }
    if (c->io.erase(c->io.ctx))
        return -1;
    c->next = 0;
    for (uint16_t j = c->live_n; j-- > 0;) {   /* written back oldest first, the order kept */
        if (c->io.program(c->io.ctx, (uint32_t)c->next * NIC_CELL, c->live[j]))
            return -1;
        c->next++;
    }
    return 0;
}

int nic_cells_put(nic_cells *c, uint16_t id, const uint8_t value[NIC_CELL_VALUE])
{
    if (id == 0xFFFF)
        return -1;
    if (c->next >= NIC_CELLS && (nic_cells_rewrite(c) || c->next >= NIC_CELLS))
        return -1;
    return program(c, id, value);
}

static uint32_t set_crc(const uint16_t *ids, const uint8_t (*values)[NIC_CELL_VALUE], size_t n)
{
    uint32_t crc = 0;
    for (size_t i = 0; i < n; i++) {
        uint8_t idb[2] = {(uint8_t)ids[i], (uint8_t)(ids[i] >> 8)};
        crc = nic_crc32(crc, idb, 2);
        crc = nic_crc32(crc, values[i], NIC_CELL_VALUE);
    }
    return crc;
}

int nic_cells_put_set(nic_cells *c, const uint16_t *ids, const uint8_t (*values)[NIC_CELL_VALUE],
                      size_t n)
{
    uint8_t check[NIC_CELL_VALUE] = {0};
    uint32_t crc = set_crc(ids, values, n);
    if (n > 255 || c->next + n + 1 > NIC_CELLS) {   /* the set and its check in one sector */
        if (nic_cells_rewrite(c) || c->next + n + 1 > NIC_CELLS)
            return -1;
    }
    for (size_t i = 0; i < n; i++)
        if (nic_cells_put(c, ids[i], values[i]))
            return -1;
    check[0] = (uint8_t)n;
    check[4] = (uint8_t)crc;
    check[5] = (uint8_t)(crc >> 8);
    check[6] = (uint8_t)(crc >> 16);
    check[7] = (uint8_t)(crc >> 24);
    return nic_cells_put(c, NIC_CELL_SET, check);
}

int nic_cells_get_set(nic_cells *c, const uint16_t *ids, uint8_t (*values)[NIC_CELL_VALUE], size_t n)
{
    uint8_t check[NIC_CELL_VALUE];
    if (nic_cells_get(c, NIC_CELL_SET, check) || check[0] != n)
        return -1;
    for (size_t i = 0; i < n; i++)
        if (nic_cells_get(c, ids[i], values[i]))
            return -1;
    uint32_t crc = (uint32_t)check[4] | (uint32_t)check[5] << 8 | (uint32_t)check[6] << 16
                   | (uint32_t)check[7] << 24;
    return set_crc(ids, (const uint8_t (*)[NIC_CELL_VALUE])values, n) == crc ? 0 : -1;
}

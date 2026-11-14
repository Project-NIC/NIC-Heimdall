/* The head's mirror — see nic_mirror.h. */
#include "nic_mirror.h"

#include <string.h>

#define ROW_META   1023u                 /* the half's own cells: its header and its closing marker */
#define PAGE_HEAD  15u
#define PAGE_CLOSE 14u
#define DELETED    0x8000u

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

static uint32_t key_of(uint16_t row, uint8_t page, int ro, uint16_t reg)
{
    return (uint32_t)row | (uint32_t)page << 10 | (uint32_t)(ro ? 1 : 0) << 14 | (uint32_t)reg << 16;
}
static uint16_t k_row(uint32_t k) { return (uint16_t)(k & 0x3FF); }
static uint8_t k_page(uint32_t k) { return (uint8_t)(k >> 10 & 0xF); }
static int k_ro(uint32_t k) { return (int)(k >> 14 & 1); }
static int k_del(uint32_t k) { return (int)(k >> 15 & 1); }
static uint16_t k_reg(uint32_t k) { return (uint16_t)(k >> 16); }
static uint32_t order(uint16_t row, uint16_t reg, uint8_t page)
{
    return (uint32_t)row << 20 | (uint32_t)reg << 4 | page;
}
static uint32_t order_of(uint32_t k) { return order(k_row(k), k_reg(k), k_page(k)); }

static void cell_bytes(uint32_t key, const uint8_t value[10], uint8_t out[16])
{
    for (int i = 0; i < 4; i++)
        out[i] = (uint8_t)(key >> (8 * i));
    memcpy(out + 4, value, 10);
    uint16_t c = crc16(out, 14);
    out[14] = (uint8_t)c;
    out[15] = (uint8_t)(c >> 8);
}

static int erased(const uint8_t c[16])
{
    for (int i = 0; i < 16; i++)
        if (c[i] != 0xFF)
            return 0;
    return 1;
}

/* A cell read: 1 whole, 0 erased, -1 torn. */
static int cell_read(nic_mirror *m, uint8_t half, uint32_t i, uint32_t *key, uint8_t value[10])
{
    uint8_t c[16];
    if (m->flash.read(m->flash.ctx, half * NIC_MIR_HALF + 16 * i, c, 16))
        return -1;
    if (erased(c))
        return 0;
    if (crc16(c, 14) != (uint16_t)(c[14] | c[15] << 8))
        return -1;
    *key = (uint32_t)c[0] | (uint32_t)c[1] << 8 | (uint32_t)c[2] << 16 | (uint32_t)c[3] << 24;
    memcpy(value, c + 4, 10);
    return 1;
}

static int cell_write(nic_mirror *m, uint32_t key, const uint8_t value[10])
{
    uint8_t c[16];
    cell_bytes(key, value, c);
    if (m->flash.program(m->flash.ctx, m->half * NIC_MIR_HALF + 16 * m->next, c))
        return -1;
    m->next++;
    return 0;
}

/* ------------------------------------------------------------------ the live set in RAM */

/* The first live cell at or after (row, reg, page). */
static uint32_t lower(const nic_mirror *m, uint32_t o)
{
    uint32_t a = 0, b = m->n;
    while (a < b) {
        uint32_t mid = a + (b - a) / 2;
        if (order_of(m->live[mid].key) < o)
            a = mid + 1;
        else
            b = mid;
    }
    return a;
}

static void drop(nic_mirror *m, uint16_t row, uint16_t reg)
{
    uint32_t a = lower(m, order(row, reg, 0)), b = a;
    while (b < m->n && k_row(m->live[b].key) == row && k_reg(m->live[b].key) == reg)
        b++;
    memmove(m->live + a, m->live + b, (m->n - b) * sizeof m->live[0]);
    m->n -= b - a;
}

static void drop_row(nic_mirror *m, uint16_t row)
{
    uint32_t a = lower(m, order(row, 0, 0)), b = a;
    while (b < m->n && k_row(m->live[b].key) == row)
        b++;
    memmove(m->live + a, m->live + b, (m->n - b) * sizeof m->live[0]);
    m->n -= b - a;
}

/* A whole value's pages taken in place of the old ones. */
static int take(nic_mirror *m, const nic_mir_cell *pages, uint8_t np)
{
    uint16_t row = k_row(pages[0].key), reg = k_reg(pages[0].key);
    drop(m, row, reg);
    if (m->n + np > NIC_MIR_CELLS)
        return -1;
    uint32_t at = lower(m, order(row, reg, 0));
    memmove(m->live + at + np, m->live + at, (m->n - at) * sizeof m->live[0]);
    memcpy(m->live + at, pages, np * sizeof pages[0]);
    m->n += np;
    return 0;
}

static uint8_t pages_for(uint8_t len) { return (uint8_t)(len <= 7 ? 1 : 1 + (len - 7 + 9) / 10); }

/* A value cut into its pages, page 0 first in the array. */
static uint8_t cut(uint16_t row, uint16_t reg, int ro, const uint8_t *v, uint8_t len, nic_mir_cell *out)
{
    uint8_t np = pages_for(len);
    uint16_t c = crc16(v, len);
    memset(out, 0, np * sizeof out[0]);
    out[0].key = key_of(row, 0, ro, reg);
    out[0].value[0] = len;
    out[0].value[1] = (uint8_t)c;
    out[0].value[2] = (uint8_t)(c >> 8);
    memcpy(out[0].value + 3, v, len < 7 ? len : 7);
    for (uint8_t p = 1; p < np; p++) {
        size_t from = 7u + 10u * (p - 1u), n = len - from < 10 ? len - from : 10;
        out[p].key = key_of(row, p, ro, reg);
        memcpy(out[p].value, v + from, n);
    }
    return np;
}

/* ------------------------------------------------------------------ the halves */

static int header(nic_mirror *m, uint8_t page)
{
    uint8_t v[10] = {0};
    for (int i = 0; i < 4; i++)
        v[i] = (uint8_t)(m->generation >> (8 * i));
    memcpy(v + 4, "NICM", 4);
    return cell_write(m, key_of(ROW_META, page, 0, 0xFFFF), v);
}

static int erase_half(nic_mirror *m, uint8_t half)
{
    for (uint32_t s = 0; s < NIC_MIR_HALF; s += NIC_MIR_SECTOR)
        if (m->flash.erase(m->flash.ctx, half * NIC_MIR_HALF + s))
            return -1;
    return 0;
}

/* The live set into the other half under the next generation, closed, the old half erased. */
static int compact(nic_mirror *m)
{
    uint8_t old = m->half;
    if (m->n + 2 > NIC_MIR_CELLS)
        return -1;
    m->half = (uint8_t)(1 - old);
    m->next = 0;
    m->generation++;
    if (erase_half(m, m->half) || header(m, PAGE_HEAD))
        return -1;
    for (uint32_t a = 0; a < m->n;) {                   /* each value's pages last first */
        uint32_t b = a + 1;
        while (b < m->n && k_page(m->live[b].key) != 0)
            b++;
        for (uint32_t i = b; i > a; i--)
            if (cell_write(m, m->live[i - 1].key, m->live[i - 1].value))
                return -1;
        a = b;
    }
    if (header(m, PAGE_CLOSE) || erase_half(m, old))
        return -1;
    m->compactions++;
    return 0;
}

static int room(nic_mirror *m, uint32_t cells)
{
    if (m->next + cells <= NIC_MIR_CELLS)
        return 0;
    if (compact(m) || m->next + cells > NIC_MIR_CELLS)
        return -1;
    return 0;
}

/* A half's closed generation, -1 when it has none. */
static int64_t closed(nic_mirror *m, uint8_t half, uint32_t *end)
{
    uint32_t key, gen = 0;
    uint8_t v[10];
    int64_t found = -1;
    *end = 0;
    if (cell_read(m, half, 0, &key, v) != 1 || key != key_of(ROW_META, PAGE_HEAD, 0, 0xFFFF)
        || memcmp(v + 4, "NICM", 4))
        return -1;
    gen = (uint32_t)v[0] | (uint32_t)v[1] << 8 | (uint32_t)v[2] << 16 | (uint32_t)v[3] << 24;
    for (uint32_t i = 1; i < NIC_MIR_CELLS; i++) {
        int r = cell_read(m, half, i, &key, v);
        if (r == 0)
            break;
        *end = i + 1;
        if (r == 1 && key == key_of(ROW_META, PAGE_CLOSE, 0, 0xFFFF)
            && !memcmp(v, (uint8_t[4]){(uint8_t)gen, (uint8_t)(gen >> 8), (uint8_t)(gen >> 16),
                                       (uint8_t)(gen >> 24)}, 4))
            found = gen;
    }
    return found;
}

static void replay(nic_mirror *m, uint32_t end)
{
    nic_mir_cell stage[16];
    uint8_t staged = 0;
    for (uint32_t i = 1; i < end; i++) {
        nic_mir_cell c;
        int r = cell_read(m, m->half, i, &c.key, c.value);
        if (r != 1) {
            if (r < 0) {
                m->torn++;
                staged = 0;
            }
            continue;
        }
        if (k_row(c.key) == ROW_META)
            continue;
        if (k_del(c.key)) {
            staged = 0;
            if (k_reg(c.key) == NIC_REG_ROW)
                drop_row(m, k_row(c.key));
            else
                drop(m, k_row(c.key), k_reg(c.key));
            continue;
        }
        if (k_page(c.key)) {                            /* a page waiting for its page 0 */
            if (staged && (k_row(stage[staged - 1].key) != k_row(c.key)
                           || k_reg(stage[staged - 1].key) != k_reg(c.key)
                           || k_page(c.key) >= k_page(stage[staged - 1].key))) {   /* a new write */
                m->torn++;
                staged = 0;
            }
            if (staged < 15)
                stage[staged++] = c;
            continue;
        }
        /* page 0 closes the value: its pages all there, its CRC whole; pages of another value
         * left by a write cut short are passed over */
        if (staged && (k_row(stage[staged - 1].key) != k_row(c.key)
                       || k_reg(stage[staged - 1].key) != k_reg(c.key))) {
            m->torn++;
            staged = 0;
        }
        uint8_t len = c.value[0], np = pages_for(len);
        nic_mir_cell pages[16];
        uint8_t v[NIC_MIR_MAX];
        int ok = len <= NIC_MIR_MAX && staged >= np - 1;
        uint8_t base = ok ? (uint8_t)(staged - (np - 1)) : 0;   /* older pages: a write cut short */
        if (ok && base)
            m->torn++;
        pages[0] = c;
        for (uint8_t p = 1; ok && p < np; p++) {
            const nic_mir_cell *s = &stage[base + np - 1 - p];   /* written last first */
            ok = k_page(s->key) == p && k_row(s->key) == k_row(c.key) && k_reg(s->key) == k_reg(c.key);
            pages[p] = *s;
        }
        if (ok) {
            memcpy(v, c.value + 3, len < 7 ? len : 7);
            for (uint8_t p = 1; p < np; p++) {
                size_t from = 7u + 10u * (p - 1u), n = len - from < 10 ? len - from : 10;
                memcpy(v + from, pages[p].value, n);
            }
            ok = crc16(v, len) == (uint16_t)(c.value[1] | c.value[2] << 8);
        }
        if (ok)
            take(m, pages, np);
        else
            m->torn++;
        staged = 0;
    }
    m->next = end;
}

int nic_mir_load(nic_mirror *m, const nic_flash *flash)
{
    memset(m, 0, sizeof *m);
    m->flash = *flash;
    uint32_t end[2];
    int64_t g0 = closed(m, 0, &end[0]), g1 = closed(m, 1, &end[1]);
    if (g0 < 0 && g1 < 0) {                             /* a fresh area */
        m->half = 0;
        m->generation = 1;
        m->next = 0;
        if (erase_half(m, 0) || header(m, PAGE_HEAD) || header(m, PAGE_CLOSE))
            return -1;
        return 0;
    }
    m->half = g1 > g0 ? 1 : 0;
    m->generation = (uint32_t)(g1 > g0 ? g1 : g0);
    replay(m, end[m->half]);
    return 0;
}

/* ------------------------------------------------------------------ values */

int nic_mir_put(nic_mirror *m, uint16_t row, uint16_t reg, int ro, const uint8_t *v, uint8_t len)
{
    if (row >= NIC_MIR_ROWS || len > NIC_MIR_MAX || reg == NIC_REG_ROW)
        return -1;
    nic_mir_cell pages[16];
    uint8_t np = cut(row, reg, ro, v, len, pages);
    if (room(m, np))
        return -1;
    for (uint8_t p = np; p > 0; p--)                    /* last first, page 0 closing */
        if (cell_write(m, pages[p - 1].key, pages[p - 1].value))
            return -1;
    return take(m, pages, np);
}

int nic_mir_value(const nic_mirror *m, uint16_t row, uint16_t reg, uint8_t *v, int *ro)
{
    uint32_t a = lower(m, order(row, reg, 0));
    if (a >= m->n || k_row(m->live[a].key) != row || k_reg(m->live[a].key) != reg)
        return -1;
    const nic_mir_cell *c = &m->live[a];
    uint8_t len = c->value[0];
    memcpy(v, c->value + 3, len < 7 ? len : 7);
    for (uint8_t p = 1; p < pages_for(len); p++) {
        size_t from = 7u + 10u * (p - 1u), n = len - from < 10 ? len - from : 10;
        memcpy(v + from, m->live[a + p].value, n);
    }
    if (ro)
        *ro = k_ro(c->key);
    return len;
}

int nic_mir_remove(nic_mirror *m, uint16_t row, uint16_t reg)
{
    uint8_t zero[10] = {0};
    if (room(m, 1) || cell_write(m, key_of(row, 0, 0, reg) | DELETED, zero))
        return -1;
    if (reg == NIC_REG_ROW)
        drop_row(m, row);
    else
        drop(m, row, reg);
    return 0;
}

int nic_mir_free_row(const nic_mirror *m)
{
    uint16_t want = 0;
    for (uint32_t i = 0; i < m->n && want < NIC_MIR_ROWS; i++) {
        uint16_t r = k_row(m->live[i].key);
        if (r > want)
            return want;
        if (r == want)
            want++;
    }
    return want < NIC_MIR_ROWS ? want : -1;
}

/* ------------------------------------------------------------------ the three states */

static nic_mir_pending *pending(const nic_mirror *m, uint16_t row, uint16_t reg)
{
    for (unsigned i = 0; i < NIC_MIR_PENDING; i++)
        if (m->pend[i].state && m->pend[i].row == row && m->pend[i].reg == reg)
            return (nic_mir_pending *)&m->pend[i];
    return 0;
}

int nic_mir_edit(nic_mirror *m, uint16_t row, uint16_t reg, const uint8_t *v, uint8_t len)
{
    if (len > NIC_MIR_MAX)
        return -1;
    nic_mir_pending *p = pending(m, row, reg);
    for (unsigned i = 0; !p && i < NIC_MIR_PENDING; i++)
        if (!m->pend[i].state)
            p = &m->pend[i];
    if (!p)
        return -1;
    p->row = row;
    p->reg = reg;
    p->state = NIC_MIR_EDITED;
    p->len = len;
    memcpy(p->value, v, len);
    return 0;
}

int nic_mir_sent(nic_mirror *m, uint16_t row, uint16_t reg)
{
    nic_mir_pending *p = pending(m, row, reg);
    if (!p)
        return -1;
    p->state = NIC_MIR_SENT;
    return 0;
}

int nic_mir_confirm(nic_mirror *m, uint16_t row, uint16_t reg)
{
    nic_mir_pending *p = pending(m, row, reg);
    if (!p || p->state != NIC_MIR_SENT || nic_mir_put(m, row, reg, 0, p->value, p->len))
        return -1;
    p->state = NIC_MIR_NONE;
    return 0;
}

void nic_mir_refused(nic_mirror *m, uint16_t row, uint16_t reg)
{
    nic_mir_pending *p = pending(m, row, reg);
    if (p)
        p->state = NIC_MIR_NONE;
}

enum nic_mir_state nic_mir_get(const nic_mirror *m, uint16_t row, uint16_t reg, uint8_t *v, uint8_t *len)
{
    const nic_mir_pending *p = pending(m, row, reg);
    if (p) {
        memcpy(v, p->value, p->len);
        *len = p->len;
        return (enum nic_mir_state)p->state;
    }
    int n = nic_mir_value(m, row, reg, v, 0);
    if (n < 0)
        return NIC_MIR_NONE;
    *len = (uint8_t)n;
    return NIC_MIR_CONFIRMED;
}

size_t nic_mir_tree(const nic_mirror *m, nic_mirror_row *out, uint16_t *rows, size_t max)
{
    size_t k = 0;
    uint8_t v[NIC_MIR_MAX];
    for (uint16_t row = 0; row < NIC_MIR_ROWS && k < max; row++) {
        if (nic_mir_value(m, row, NIC_REG_IDENTITY, v, 0) != 11)
            continue;
        out[k].bus = v[0];
        out[k].addr = v[1];
        out[k].tag = (uint16_t)(v[2] | v[3] << 8);
        out[k].tag2 = (uint16_t)(v[4] | v[5] << 8);
        if (rows)
            rows[k] = row;
        k++;
    }
    return k;
}

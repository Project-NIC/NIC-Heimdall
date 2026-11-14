/* The head's mirror on a PC, on a simulated NOR flash of two 128 kB halves: a full station — 16
 * cards with their per-port registers, 128 units, 32 Argus NUMBERs, 247 ModBus sensors, four
 * Palatine rosters of 64, Marconi's curve and sounders — written and read back, and again from the
 * flash after a reboot; a base station's size; thirty thousand changes turning the halves over;
 * the supply cut at twelve hundred points of a run that compacts — every value the one before or the
 * one after the write it cut, nothing else moved; the three states, a reboot dropping what was not
 * confirmed; a row removed and its number free again. */
#include "../nic_mirror.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int fails;
#define CHECK(x) do { if (!(x)) { fails++; fprintf(stderr, "%s:%d: %s\n", __FILE__, __LINE__, #x); } } while (0)

/* ------------------------------------------------------------------ the flash */

#define AREA (2u * NIC_MIR_HALF)
static uint8_t flash[AREA], snap[AREA];
static long budget = -1;                 /* operations before the supply goes, -1 never */
static int cut_bytes, dead;

static int fl_read(void *ctx, uint32_t a, uint8_t *b, uint32_t n)
{
    (void)ctx;
    if ((size_t)a + n > AREA) return -1;
    memcpy(b, flash + a, n);
    return 0;
}

static int spend(void)
{
    if (dead) return -1;
    if (budget >= 0 && budget-- == 0) { dead = 1; return 1; }
    return 0;
}

static int fl_program(void *ctx, uint32_t a, const uint8_t b[16])
{
    (void)ctx;
    int s = spend();
    if (s < 0) return -1;
    int n = s ? cut_bytes : 16;          /* a cell cut part way */
    for (int i = 0; i < n; i++) flash[a + i] &= b[i];
    return s ? -1 : 0;
}

static int fl_erase(void *ctx, uint32_t a)
{
    (void)ctx;
    int s = spend();
    if (s < 0) return -1;
    if (s) {                             /* an erase cut: part of the sector */
        memset(flash + a, 0xFF, NIC_MIR_SECTOR / 3);
        return -1;
    }
    memset(flash + a, 0xFF, NIC_MIR_SECTOR);
    return 0;
}

static const nic_flash FL = {fl_read, fl_program, fl_erase, 0};

/* ------------------------------------------------------------------ the model */

typedef struct { uint16_t row, reg; uint8_t used, ro, len, v[NIC_MIR_MAX]; } val;
static val model[8192], saved[8192];
static int nkeys;
static uint32_t rnd = 12345;
static uint32_t next_rnd(void) { rnd = rnd * 1103515245u + 12345u; return rnd >> 8; }

static int key(uint16_t row, uint16_t reg, uint8_t ro, uint8_t len)
{
    val *x = &model[nkeys];
    x->row = row; x->reg = reg; x->ro = ro; x->len = len; x->used = 0;
    return nkeys++;
}

static void fill(val *x)
{
    for (int i = 0; i < x->len; i++) x->v[i] = (uint8_t)next_rnd();
}

static int put(nic_mirror *m, int k)
{
    val *x = &model[k];
    fill(x);
    int r = nic_mir_put(m, x->row, x->reg, x->ro, x->v, x->len);
    if (!r) x->used = 1;
    return r;
}

/* every key as the model has it, in RAM and nothing more */
static int quiet;
static int matches(const nic_mirror *m, const val *mod)
{
    int ok = 1;
    uint8_t v[NIC_MIR_MAX];
    for (int k = 0; k < nkeys; k++) {
        int ro, n = nic_mir_value(m, mod[k].row, mod[k].reg, v, &ro);
        if (!mod[k].used) {
            if (n >= 0) { ok = 0; if (!quiet) fprintf(stderr, "  %u/%04X should be gone\n", mod[k].row, mod[k].reg); }
            continue;
        }
        if (n != mod[k].len || memcmp(v, mod[k].v, (size_t)n) || ro != mod[k].ro) {
            ok = 0;
            if (!quiet) fprintf(stderr, "  %u/%04X differs\n", mod[k].row, mod[k].reg);
        }
    }
    return ok;
}

static nic_mirror m, r;

/* the station's keys: rows of cards, units, Argus NUMBERs, ModBus sensors */
static void station(int cards, int units, int arguses, int modbus, int palatines, int roster)
{
    int row = 0;
    for (int c = 0; c < cards; c++, row++) {
        key((uint16_t)row, NIC_REG_IDENTITY, 0, 11); key((uint16_t)row, NIC_REG_RULES, 0, 6);
        key((uint16_t)row, NIC_REG_SEND, 0, 12); key((uint16_t)row, NIC_REG_MEASURED, 1, 12);
        for (int p = 0; p < 8; p++)
            for (int g = 0; g < 5; g++) key((uint16_t)row, (uint16_t)((0xFF20 + 0x10 * g + 0x10 * (g > 2)) + p), 0, 2);
    }
    for (int u = 0; u < units; u++, row++) {
        key((uint16_t)row, NIC_REG_IDENTITY, 0, 11); key((uint16_t)row, NIC_REG_RULES, 0, 6);
        key((uint16_t)row, NIC_REG_SEND, 0, 12); key((uint16_t)row, NIC_REG_MEASURED, 1, 12);
        key((uint16_t)row, 0xFF0A, 0, 6); key((uint16_t)row, 0xFF0B, 0, 2); key((uint16_t)row, 0xFF13, 0, 2);
        if (!(u >= 4 && u < 4 + palatines))
            for (int g = 0; g < 5; g++) key((uint16_t)row, (uint16_t)(0x0010 + g), 0, 2);
        if (u % 8 == 1) { key((uint16_t)row, 0x0017, 0, 18); key((uint16_t)row, 0x0018, 0, 16); }   /* a Tesla */
        if (u == 3) {                                                                             /* a Marconi */
            key((uint16_t)row, 0x0016, 1, 128);
            for (int i = 0; i < 16; i++) key((uint16_t)row, NIC_REG_TABLE(0x14, i), 0, 12);
        }
        if (u >= 4 && u < 4 + palatines) {                                                        /* a Palatine */
            for (int i = 0; i < roster; i++) key((uint16_t)row, NIC_REG_TABLE(0x14, i), 0, 10);
            for (int a = 0; a < 4; a++) key((uint16_t)row, (uint16_t)(0x0010 + a), 0, 6);
        }
    }
    for (int a = 0; a < arguses; a++, row++) {
        key((uint16_t)row, NIC_REG_IDENTITY, 0, 11); key((uint16_t)row, NIC_REG_MEASURED, 1, 12);
        key((uint16_t)row, 0xFF0B, 0, 2);
    }
    for (int s = 0; s < modbus; s++, row++) {
        key((uint16_t)row, NIC_REG_IDENTITY, 0, 11); key((uint16_t)row, NIC_REG_RULES, 0, 6);
        key((uint16_t)row, NIC_REG_SEND, 0, 12);
        for (int g = 0; g < 3; g++) key((uint16_t)row, (uint16_t)(0x0010 + g), 0, 2);
    }
    CHECK(row <= (int)NIC_MIR_ROWS);
}

static void fresh(void)
{
    memset(flash, 0xFF, AREA);
    budget = -1; dead = 0;
    CHECK(!nic_mir_load(&m, &FL));
}

int main(void)
{
    /* a base station: four cards, 32 units, one Palatine of 20 sensors on ModBus */
    fresh();
    nkeys = 0;
    station(4, 32, 0, 20, 1, 20);
    for (int k = 0; k < nkeys; k++) CHECK(!put(&m, k));
    CHECK(matches(&m, model));
    printf("a base station: %d values, %u cells, %u kB\n", nkeys, m.n, m.n * 16 / 1024);

    /* a full station */
    fresh();
    nkeys = 0;
    station(16, 128, 32, 247, 4, 64);
    for (int k = 0; k < nkeys; k++) CHECK(!put(&m, k));
    CHECK(matches(&m, model));
    printf("a full station: %d values, %u cells, %u kB of a %u kB half\n", nkeys, m.n, m.n * 16 / 1024,
           NIC_MIR_HALF / 1024);
    CHECK(!nic_mir_load(&r, &FL) && matches(&r, model) && r.torn == 0);

    /* thirty thousand changes, a removal now and then: the halves turn over */
    for (int i = 0; i < 30000; i++) {
        int k = (int)(next_rnd() % (uint32_t)nkeys);
        if (i % 97 == 0 && model[k].used) {
            CHECK(!nic_mir_remove(&m, model[k].row, model[k].reg));
            model[k].used = 0;
        } else
            CHECK(!put(&m, k));
    }
    CHECK(m.compactions > 3);
    CHECK(matches(&m, model));
    CHECK(!nic_mir_load(&r, &FL) && matches(&r, model) && r.torn == 0);
    printf("thirty thousand changes: %u compactions\n", m.compactions);

    /* the supply cut at twelve hundred points of a run that crosses a compaction */
    memcpy(snap, flash, AREA);
    memcpy(saved, model, sizeof model);
    uint32_t seed = rnd;
    int cuts_in_compaction = 0, trials = 0;
    for (long t = 0; t < 1200; t++) {
        memcpy(flash, snap, AREA);
        memcpy(model, saved, sizeof model);
        rnd = seed;
        budget = -1; dead = 0;
        CHECK(!nic_mir_load(&m, &FL));
        uint32_t comp = m.compactions;
        budget = t * 10 + (t % 7);
        cut_bytes = (int)(t % 17);
        int k = -1, failed = 0;
        val before;
        for (int i = 0; i < 5000 && !failed; i++) {
            k = (int)(next_rnd() % (uint32_t)nkeys);
            before = model[k];
            if (put(&m, k)) failed = 1;
        }
        if (!failed) continue;
        trials++;
        val after = model[k];
        after.used = 1;                      /* the write may have landed whole */
        model[k] = before;
        budget = -1; dead = 0;
        CHECK(!nic_mir_load(&r, &FL));
        quiet = 1;
        int as_before = matches(&r, model);
        model[k] = after;
        int as_after = !as_before && matches(&r, model);
        quiet = 0;
        if (!as_before && !as_after) {
            fprintf(stderr, "  cut %ld at key %d (%u/%04X len %u): neither; comp %u->%u half %u->%u gen %u/%u torn %u next %u\n",
                    t, k, model[k].row, model[k].reg, model[k].len, comp, m.compactions, m.half, r.half, m.generation, r.generation, r.torn, r.next);
            CHECK(0);
        }
        if (m.half != r.half || m.generation != r.generation) cuts_in_compaction++;
    }
    CHECK(trials > 1000);
    printf("the supply cut %d times, %d of them about a compaction\n", trials, cuts_in_compaction);

    /* the three states */
    fresh();
    nkeys = 0;
    station(1, 1, 0, 0, 0, 0);
    for (int k = 0; k < nkeys; k++) CHECK(!put(&m, k));
    uint8_t v[NIC_MIR_MAX], len, nv[2] = {0xAB, 0xCD};
    uint16_t row = 1, reg = 0xFF0B;
    uint8_t old[2];
    CHECK(nic_mir_value(&m, row, reg, old, 0) == 2);
    CHECK(!nic_mir_edit(&m, row, reg, nv, 2));
    CHECK(nic_mir_get(&m, row, reg, v, &len) == NIC_MIR_EDITED && len == 2 && v[0] == 0xAB);
    CHECK(nic_mir_confirm(&m, row, reg) == -1);                 /* not sent */
    CHECK(!nic_mir_sent(&m, row, reg));
    CHECK(nic_mir_get(&m, row, reg, v, &len) == NIC_MIR_SENT);
    CHECK(!nic_mir_load(&r, &FL) && nic_mir_get(&r, row, reg, v, &len) == NIC_MIR_CONFIRMED
          && !memcmp(v, old, 2));                                /* a reboot: the confirmed one */
    CHECK(!nic_mir_confirm(&m, row, reg));
    CHECK(nic_mir_get(&m, row, reg, v, &len) == NIC_MIR_CONFIRMED && v[1] == 0xCD);
    CHECK(!nic_mir_load(&r, &FL) && nic_mir_value(&r, row, reg, v, 0) == 2 && v[1] == 0xCD);
    CHECK(!nic_mir_edit(&m, row, reg, old, 2) && !nic_mir_sent(&m, row, reg));
    nic_mir_refused(&m, row, reg);
    CHECK(nic_mir_get(&m, row, reg, v, &len) == NIC_MIR_CONFIRMED && v[1] == 0xCD);
    for (unsigned i = 0; i < NIC_MIR_PENDING; i++)
        CHECK(!nic_mir_edit(&m, row, (uint16_t)(0x100 + i), nv, 2));
    CHECK(nic_mir_edit(&m, row, 0x0200, nv, 2) == -1);          /* the table full */

    /* a row removed, its number free again, after a reboot too */
    CHECK(nic_mir_free_row(&m) == 2);
    CHECK(!nic_mir_remove(&m, 0, NIC_REG_ROW));
    CHECK(nic_mir_free_row(&m) == 0);
    CHECK(!nic_mir_load(&r, &FL) && nic_mir_free_row(&r) == 0 && nic_mir_value(&r, 0, NIC_REG_IDENTITY, v, 0) < 0
          && nic_mir_value(&r, 1, NIC_REG_IDENTITY, v, 0) == 11);

    /* the tree's rows from the identities */
    fresh();
    for (uint16_t i = 0; i < 5; i++) {
        uint8_t id[11] = {(uint8_t)(i == 4 ? 1 : 0), (uint8_t)(0x51 + i), (uint8_t)(0x10 + i), 0x20, 0x30, (uint8_t)i, 1, 2, 3, 0, 0};
        CHECK(!nic_mir_put(&m, (uint16_t)(i * 3), NIC_REG_IDENTITY, 0, id, 11));
        CHECK(!nic_mir_put(&m, (uint16_t)(i * 3), 0xFF0B, 0, nv, 2));
    }
    nic_mirror_row tr[8];
    uint16_t trows[8];
    CHECK(nic_mir_tree(&m, tr, trows, 8) == 5);
    CHECK(tr[2].addr == 0x53 && tr[2].bus == 0 && tr[2].tag == 0x2012 && tr[2].tag2 == 0x0230 && trows[2] == 6);
    CHECK(tr[4].bus == 1 && tr[4].addr == 0x55);

    if (fails) {
        printf("test_mirror: %d FAILED\n", fails);
        return 1;
    }
    printf("test_mirror: all passed\n");
    return 0;
}

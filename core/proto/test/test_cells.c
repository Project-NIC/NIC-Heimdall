/* The configuration cells on a simulated H5 sector: a quad-word programmed once, erased flash all
 * 0xFF. The last good value per id, a torn cell skipped, the full sector rewritten with its live
 * set, the persisted set's check failing when one member moves alone, the yearly refresh, and an
 * erase with no rewrite behind it reading as no set. */
#include "../nic_cells.h"

#include <stdio.h>
#include <string.h>

static int fails;
#define CHECK(x) do { if (!(x)) { fails++; fprintf(stderr, "%s:%d: %s\n", __FILE__, __LINE__, #x); } } while (0)

static uint8_t flash[NIC_CELLS * NIC_CELL];
static int erases, torn_at = -1, programs_left = -1;

static int rd(void *ctx, uint32_t off, uint8_t *b, size_t n) { (void)ctx; memcpy(b, flash + off, n); return 0; }

static int prog(void *ctx, uint32_t off, const uint8_t cell[NIC_CELL])
{
    (void)ctx;
    for (unsigned i = 0; i < NIC_CELL; i++)
        if (flash[off + i] != 0xFF)
            return -1;                         /* the H5 programs a quad-word once */
    if (programs_left == 0)
        return -1;
    if (programs_left > 0)
        programs_left--;
    unsigned n = (int)(off / NIC_CELL) == torn_at ? NIC_CELL / 2 : NIC_CELL;   /* power lost mid-write */
    for (unsigned i = 0; i < n; i++)
        flash[off + i] &= cell[i];
    return 0;
}

static int er(void *ctx) { (void)ctx; memset(flash, 0xFF, sizeof flash); erases++; return 0; }

static void val(uint8_t v[12], unsigned x) { memset(v, 0, 12); v[0] = (uint8_t)x; v[1] = (uint8_t)(x >> 8); v[11] = 0x5A; }

int main(void)
{
    static nic_cells c;
    nic_cells_io io = {rd, prog, er, NULL};
    uint8_t v[12], w[12];
    memset(flash, 0xFF, sizeof flash);
    CHECK(!nic_cells_open(&c, &io) && c.next == 0);
    CHECK(nic_cells_get(&c, 7, v) == -1);
    val(v, 1); CHECK(!nic_cells_put(&c, 7, v));
    val(v, 2); CHECK(!nic_cells_put(&c, 8, v));
    val(v, 3); CHECK(!nic_cells_put(&c, 7, v));
    CHECK(!nic_cells_get(&c, 7, w) && w[0] == 3 && !nic_cells_get(&c, 8, w) && w[0] == 2);
    /* a torn cell: skipped, and the next append goes past it */
    torn_at = c.next;
    val(v, 4); nic_cells_put(&c, 7, v);
    torn_at = -1;
    static nic_cells c2;
    CHECK(!nic_cells_open(&c2, &io) && c2.next == 4);
    CHECK(!nic_cells_get(&c2, 7, w) && w[0] == 3);                   /* the last good one */
    val(v, 5); CHECK(!nic_cells_put(&c2, 7, v) && !nic_cells_get(&c2, 7, w) && w[0] == 5);
    /* the persisted set: number, slot, BUSCFG */
    uint16_t ids[3] = {1, 2, 3};
    uint8_t set[3][12], got[3][12];
    for (int i = 0; i < 3; i++) val(set[i], 0x100 + (unsigned)i);
    CHECK(!nic_cells_put_set(&c2, ids, (const uint8_t (*)[12])set, 3));
    CHECK(!nic_cells_get_set(&c2, ids, got, 3) && !memcmp(got, set, sizeof set));
    val(v, 0x999); CHECK(!nic_cells_put(&c2, 2, v));                 /* one member moved alone */
    CHECK(nic_cells_get_set(&c2, ids, got, 3) == -1);                 /* no set: boot fresh */
    CHECK(!nic_cells_put_set(&c2, ids, (const uint8_t (*)[12])set, 3) && !nic_cells_get_set(&c2, ids, got, 3));
    /* fill the sector: it is erased once and the live set rewritten, every last value kept */
    int e0 = erases;
    for (unsigned k = 0; k < 2000; k++) {
        val(v, k); CHECK(!nic_cells_put(&c2, (uint16_t)(100 + k % 40), v));
    }
    CHECK(erases > e0);
    for (unsigned id = 100; id < 140; id++)
        CHECK(!nic_cells_get(&c2, (uint16_t)id, w) && w[0] == (uint8_t)(1960 + (id - 100)) && w[1] == (uint8_t)((1960 + (id - 100)) >> 8));
    CHECK(!nic_cells_get(&c2, 7, w) && w[0] == 5);
    CHECK(!nic_cells_get_set(&c2, ids, got, 3) && !memcmp(got, set, sizeof set));   /* the set survives */
    /* the yearly refresh: erase and write, nothing lost, the sector compact */
    uint16_t before = c2.next;
    CHECK(!nic_cells_rewrite(&c2) && c2.next <= before && c2.next == 3 + 1 + 1 + 1 + 40);
    CHECK(!nic_cells_get_set(&c2, ids, got, 3) && !nic_cells_get(&c2, 8, w) && w[0] == 2);
    /* an erase with no rewrite behind it (power lost): no set, the unit boots fresh */
    programs_left = 0;
    CHECK(nic_cells_rewrite(&c2) == -1);
    programs_left = -1;
    static nic_cells c3;
    CHECK(!nic_cells_open(&c3, &io) && c3.next == 0 && nic_cells_get_set(&c3, ids, got, 3) == -1);
    CHECK(nic_cells_put(&c3, 0xFFFF, v) == -1);                       /* the erased id is no id */
    if (fails) {
        printf("%d checks failed\n", fails);
        return 1;
    }
    printf("the configuration cells: every check passed\n");
    return 0;
}

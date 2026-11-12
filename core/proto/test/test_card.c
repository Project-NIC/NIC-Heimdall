/* The card's data path on a PC: the slot window, a unit's circular buffer — the frame a second
 * late and stamped, every filler reason, one RESEND and no second, the ladder — the segment table,
 * the PORTS frame, and an Argus's tiling. */
#include "../nic_card.h"

#include <stdio.h>
#include <string.h>

static int fails;
#define CHECK(x) do { if (!(x)) { fails++; fprintf(stderr, "%s:%d: %s\n", __FILE__, __LINE__, #x); } } while (0)

enum { ROWS = 128 };
static const uint32_t SEC = 0x6AE12B00u;

static size_t frame_for(uint32_t period, uint8_t addr, uint8_t slot, uint8_t fill, uint8_t *f)
{
    uint8_t pl[32];
    memset(pl, fill, 32);
    nic_header h = {(uint8_t)(period / ROWS), (uint8_t)(period % ROWS), addr, slot, NIC_DATA, 0};
    return nic_data_build(&h, pl, 32, f);
}

static int released(nic_unitbuf *u, uint32_t now, int labelled, nic_header *h, const uint8_t **p, uint32_t *sec)
{
    static uint8_t out[48];
    nic_unitbuf_release(u, now, labelled, out);
    return nic_trunk_parse(out, sec, h, p);
}

int main(void)
{
    /* the window: ± two byte times about round start + (slot − 1) × width */
    CHECK(nic_slot_inside(1000 + 2 * 300, 1000, 3, 300, 10));
    CHECK(nic_slot_inside(1000 + 600 - 20, 1000, 3, 300, 10) && nic_slot_inside(1600 + 20, 1000, 3, 300, 10));
    CHECK(!nic_slot_inside(1600 + 21, 1000, 3, 300, 10) && !nic_slot_inside(1600 - 21, 1000, 3, 300, 10));
    CHECK(!nic_slot_inside(1000, 1000, 0, 300, 10));

    static nic_cell cells[NIC_BIFROST_DELAY];
    nic_unitbuf u;
    nic_unitbuf_init(&u, cells, NIC_BIFROST_DELAY, NIC_BIFROST_DELAY, ROWS, 0x52, 3);
    uint8_t f[48];
    nic_header h;
    const uint8_t *p;
    uint32_t sec, p0 = SEC * ROWS + 100;
    /* periods p0 … p0+9: 0 whole, 1 nothing, 2 heard only, 3 one miss then its RESEND, 4 two misses,
       5 slipped, 6 the wrong slot, 7..9 whole */
    for (uint32_t k = 0; k < 10; k++) {
        uint32_t per = p0 + k;
        if (k == 1)
            continue;
        if (k == 2) {
            nic_unitbuf_heard(&u, per);
            continue;
        }
        frame_for(per, 0x52, 3, (uint8_t)k, f);
        if (k == 3 || k == 4) {
            f[12] ^= 1;
            CHECK(nic_unitbuf_arrive(&u, per, f, 40, 1) == NIC_ASK_RESEND);
            if (k == 3) {
                f[12] ^= 1;                                  /* the repeat, whole */
                CHECK(nic_unitbuf_arrive(&u, per, f, 40, 1) == NIC_STORED);
            } else {
                CHECK(nic_unitbuf_arrive(&u, per, f, 40, 1) == NIC_GIVEN_UP);
            }
            continue;
        }
        if (k == 5)
            frame_for(per + 1, 0x52, 3, 5, f);              /* a frame from the wrong period */
        if (k == 6)
            frame_for(per, 0x52, 4, 6, f);
        CHECK(nic_unitbuf_arrive(&u, per, f, 40, 1) == (k == 5 || k == 6 ? NIC_WRONG : NIC_STORED));
    }
    static const uint8_t want[10] = {0, NIC_FILL_MISSING, NIC_FILL_EMPTY, 0, NIC_FILL_CORRUPT,
                                     NIC_FILL_SLIPPED, NIC_FILL_EMPTY, 0, 0, 0};
    for (uint32_t k = 0; k < 10; k++) {
        CHECK(!released(&u, p0 + k + NIC_BIFROST_DELAY, 1, &h, &p, &sec));
        CHECK(sec == (p0 + k) / ROWS && h.frame == (p0 + k) % ROWS && h.addr == 0x52 && h.slot == 3);
        if (want[k]) {
            CHECK(h.kind == NIC_FILLER && p[0] == want[k] && p[1] == 0 && p[31] == 0);
        } else {
            CHECK(h.kind == NIC_DATA && p[0] == k && p[31] == k);
        }
    }
    /* across the second and around the ring, release before the period's arrivals */
    for (uint32_t k = 10; k < 400; k++) {
        CHECK(!released(&u, p0 + k, 1, &h, &p, &sec));
        frame_for(p0 + k, 0x52, 3, (uint8_t)k, f);
        CHECK(nic_unitbuf_arrive(&u, p0 + k, f, 40, 1) == NIC_STORED);
        if (k >= NIC_BIFROST_DELAY + 10)
            CHECK(h.kind == NIC_DATA && p[0] == (uint8_t)(k - NIC_BIFROST_DELAY) && sec == (p0 + k - NIC_BIFROST_DELAY) / ROWS);
    }
    CHECK(!released(&u, p0 + 400, 0, &h, &p, &sec) && h.kind == NIC_FILLER && p[0] == NIC_FILL_UNLABELLED);
    /* the wrong place three times in a row: the ladder; a good frame resets the count */
    frame_for(p0 + 500, 0x52, 3, 1, f);
    CHECK(nic_unitbuf_arrive(&u, p0 + 500, f, 40, 0) == NIC_WRONG);
    CHECK(nic_unitbuf_arrive(&u, p0 + 500, f, 40, 0) == NIC_WRONG);
    CHECK(nic_unitbuf_arrive(&u, p0 + 500, f, 40, 0) == NIC_LADDER);
    CHECK(nic_unitbuf_arrive(&u, p0 + 500, f, 40, 1) == NIC_STORED && u.wrong == 0);
    CHECK(nic_unitbuf_arrive(&u, p0 + 501, f, 12, 1) == NIC_WRONG);   /* a CONTROL length: heard */
    CHECK(!released(&u, p0 + 501 + NIC_BIFROST_DELAY, 1, &h, &p, &sec) && p[0] == NIC_FILL_EMPTY);

    /* the segment table and the PORTS frame */
    nic_seg_row rows[8] = {{0x52, 1, 2, 0}, {0}, {0x31, 3, 1, 2}};
    uint8_t t[32];
    nic_segment_table(rows, t);
    CHECK(t[0] == 0x52 && t[1] == 1 && t[2] == 2 && t[3] == 0 && t[4] == 0 && t[8] == 0x31 && t[11] == 2 && t[31] == 0);
    uint16_t vb[5] = {1, 2, 3, 4, 5}, cu[5] = {0x1011, 0x2022, 0x3033, 0x4044, 0x5055}, lk[4] = {7, 8, 9, 0xABCD};
    nic_ports_payload(vb, cu, lk, -1234, t);
    CHECK(t[0] == 1 && t[2] == 0x11 && t[3] == 0x10 && t[16] == 5 && t[18] == 0x55 && t[19] == 0x50);
    CHECK(t[20] == 7 && t[26] == 0xCD && t[27] == 0xAB && (int16_t)(t[28] | t[29] << 8) == -1234 && t[30] == 0 && t[31] == 0);

    /* an Argus's tiling: payloads by position, the flags OR'd, the worst code, a seated sonde absent */
    nic_tile tl;
    uint8_t m[8] = {1, 2, 3, 4, 5, 6, 7, 8};
    nic_tile_init(&tl);
    nic_tile_place(&tl, 0, NIC_OK | NIC_ALARM, m);
    nic_tile_place(&tl, 3, NIC_CLIPPED | NIC_FAULT, m);
    CHECK(!memcmp(tl.payload, m, 8) && !memcmp(tl.payload + 24, m, 8) && tl.payload[8] == 0 && tl.payload[16] == 0);
    CHECK(nic_tile_status(&tl, 0x09) == (NIC_ALARM | NIC_FAULT | NIC_CLIPPED));
    nic_tile_init(&tl);
    nic_tile_place(&tl, 1, NIC_OK, m);
    CHECK(nic_tile_status(&tl, 0x03) == NIC_SENSOR);                   /* position 0 seated, silent */
    CHECK(nic_tile_status(&tl, 0x02) == NIC_OK);
    if (fails) {
        printf("%d checks failed\n", fails);
        return 1;
    }
    printf("the card's data path: every check passed\n");
    return 0;
}

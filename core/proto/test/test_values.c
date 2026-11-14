/* The head's values frame on a PC: the 32 B laid out byte by byte and read back, a bit flipped
 * refused; the heartbeat an hour after boot and an hour after every frame; a marker at once,
 * restarting the hour, a second within the minute a weak event in the next frame's count; the count
 * held at 65 535; a day of markers every 30 s under a LoRa duty cap of 36 s an hour — every hour's
 * airtime under it, the frames held and the heartbeat stretched; the last 16 frames resent, the
 * seventeenth gone. */
#include "../nic_values.h"

#include <stdio.h>
#include <string.h>

static int fails;
#define CHECK(x) do { if (!(x)) { fails++; fprintf(stderr, "%s:%d: %s\n", __FILE__, __LINE__, #x); } } while (0)

int main(void)
{
    /* the layout */
    nic_val_frame f = {0x123456, 1790856000u, 13250, 0, -7, {0}, NIC_CLK_LOCKED, 1, NIC_WHY_SUPPLY, NIC_PWR_CELL, 37};
    f.events = 513;
    f.alive[0] = 0xFF; f.alive[15] = 0x80;
    uint8_t b[NIC_VAL_LEN], out[NIC_VAL_LEN];
    nic_val_build(&f, b);
    CHECK(b[0] == 0x56 && b[1] == 0x34 && b[2] == 0x12);
    CHECK(b[3] == 0x40 && b[4] == 0x4B && b[5] == 0xBE && b[6] == 0x6A);            /* 1790856000 */
    CHECK(b[7] == 0xC2 && b[8] == 0x33 && b[9] == 0xF9 && b[10] == 0x01 && b[11] == 0x02);
    CHECK(b[12] == 0xFF && b[27] == 0x80);
    CHECK(b[28] == (2 | 4 | 3 << 3 | 3 << 6) && b[29] == 37);
    nic_val_frame g;
    CHECK(!nic_val_parse(b, &g) && g.station == f.station && g.second == f.second && g.pack_mv == 13250
          && g.temperature == -7 && g.events == 513 && g.alive[15] == 0x80 && g.clock == NIC_CLK_LOCKED
          && g.marker == 1 && g.why == NIC_WHY_SUPPLY && g.power == NIC_PWR_CELL && g.peak == 37);
    b[20] ^= 4;
    CHECK(nic_val_parse(b, &g) == -1);

    /* the heartbeat and the marker, no cap */
    nic_val_state st = {13100, 21, {0}, NIC_CLK_LOCKED, NIC_PWR_NORMAL};
    nic_values v;
    uint32_t t0 = 1790856000u;
    nic_val_init(&v, 0x123456, t0, 1810, 0);
    CHECK(!nic_val_tick(&v, t0 + 3599, &st, out));
    CHECK(nic_val_tick(&v, t0 + 3600, &st, out) && !nic_val_parse(out, &g) && !g.marker && g.second == t0 + 3600);
    nic_val_weak(&v); nic_val_weak(&v);
    CHECK(nic_val_marker(&v, t0 + 4000, NIC_WHY_EVENT, 80) == 1);
    CHECK(nic_val_tick(&v, t0 + 4000, &st, out) && !nic_val_parse(out, &g) && g.marker && g.why == NIC_WHY_EVENT
          && g.peak == 80 && g.events == 2);
    CHECK(nic_val_marker(&v, t0 + 4030, NIC_WHY_EVENT, 60) == 0);   /* an aftershock in the minute */
    CHECK(!nic_val_tick(&v, t0 + 4030, &st, out));
    CHECK(!nic_val_tick(&v, t0 + 4000 + 3599, &st, out));            /* the hour restarted by the marker */
    CHECK(nic_val_tick(&v, t0 + 4000 + 3600, &st, out) && !nic_val_parse(out, &g) && !g.marker && g.events == 1);
    CHECK(nic_val_marker(&v, t0 + 8000, NIC_WHY_FAULT, 0) == 1);     /* a minute on: at once */
    CHECK(nic_val_tick(&v, t0 + 8000, &st, out) && !nic_val_parse(out, &g) && g.why == NIC_WHY_FAULT);

    /* the count held at its ceiling */
    for (int i = 0; i < 70000; i++) nic_val_weak(&v);
    CHECK(nic_val_tick(&v, t0 + 8000 + 3600, &st, out) && !nic_val_parse(out, &g) && g.events == 65535);

    /* a day of markers every 30 s under 36 s of airtime an hour */
    nic_val_init(&v, 0x123456, t0, 1810, 36000);
    uint32_t sent[4096];
    int nsent = 0;
    for (uint32_t s = 0; s < 86400; s++) {
        if (s % 30 == 0) nic_val_marker(&v, t0 + s, NIC_WHY_EVENT, 10);
        if (nic_val_tick(&v, t0 + s, &st, out)) sent[nsent++] = t0 + s;
    }
    for (int i = 0; i < nsent; i++) {                                 /* every hour's airtime under the cap */
        int in_hour = 0;
        for (int j = 0; j < nsent; j++)
            if (sent[j] <= sent[i] && sent[i] - sent[j] < 3600) in_hour++;
        CHECK((uint32_t)in_hour * 1810 <= 36000);
    }
    CHECK(nsent >= 24 * 19 - 19 && nsent <= 24 * 19 + 1 && v.held > 0);
    printf("a day of markers every 30 s under the cap: %d frames, %u ticks held\n", nsent, v.held);

    /* the last 16 kept for a resend; the seventeenth gone */
    nic_val_init(&v, 0x123456, t0, 1810, 0);
    for (int i = 1; i <= 20; i++) CHECK(nic_val_tick(&v, t0 + 3600u * (uint32_t)i, &st, out));
    CHECK(nic_val_resend(&v, t0 + 3600 * 20 + 10, t0 + 3600 * 20, out) && !nic_val_parse(out, &g) && g.second == t0 + 3600 * 20);
    CHECK(nic_val_resend(&v, t0 + 3600 * 20 + 10, t0 + 3600 * 5, out));
    CHECK(!nic_val_resend(&v, t0 + 3600 * 20 + 10, t0 + 3600 * 4, out));
    CHECK(!nic_val_resend(&v, t0 + 3600 * 20 + 10, t0 + 17, out));

    if (fails) {
        printf("test_values: %d FAILED\n", fails);
        return 1;
    }
    printf("test_values: all passed\n");
    return 0;
}

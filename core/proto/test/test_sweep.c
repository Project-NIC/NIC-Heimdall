/* The sweep on a simulated shared segment at 38 400: replies of 12 B (3 125 µs), a collision when
 * two start within the 3 µs detection window, listen-before-talk otherwise. Every unit found once
 * by its tag — two units with one tag included — the enrolled one silent, the round closed by two
 * silent windows, ASSIGN_ADDR taken by its tag alone; the round's length measured. */
#include "../nic_sweep.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int fails;
#define CHECK(x) do { if (!(x)) { fails++; fprintf(stderr, "%s:%d: %s\n", __FILE__, __LINE__, #x); } } while (0)

#define FRAME_US 3125u      /* 12 B × 10 bits at 38 400 */
#define DETECT_US 3u

typedef struct { int unit; uint32_t start, end; int bad; uint8_t f[12]; } tx_t;

static uint32_t rnd_state = 1;
static uint32_t rnd(void) { rnd_state = rnd_state * 1103515245u + 12345u; return rnd_state >> 8; }

/* run one round; returns its length in µs, 0 when it did not end within 30 s */
static uint32_t round_on_bus(nic_sweep_unit *u, int n, nic_sweep_host *h)
{
    uint8_t f[12];
    tx_t tx[64];
    int ntx = 0;
    nic_sweep_host_start(h, 0, f);
    for (int i = 0; i < n; i++)
        nic_sweep_unit_discover(&u[i], 0);
    for (uint32_t t = 1; t < 30000000u; t++) {
        /* frames ending now */
        for (int k = 0; k < ntx; k++) {
            if (tx[k].end != t)
                continue;
            nic_sweep_host_rx(h, tx[k].f, 12, t);
            if (tx[k].bad)
                h->found[0].tag = h->found[0].tag;   /* a garbled frame: activity, no reply */
            nic_sweep_unit_echo(&u[tx[k].unit], !tx[k].bad, t);
            for (int i = 0; i < n; i++)
                if (i != tx[k].unit)
                    nic_sweep_unit_heard(&u[i], t);
            tx[k] = tx[--ntx];
            k--;
        }
        /* units whose wait is over */
        for (int i = 0; i < n; i++) {
            int busy = 0, near = -1;
            for (int k = 0; k < ntx; k++) {
                if (t - tx[k].start > DETECT_US)
                    busy = 1;
                else
                    near = k;
            }
            if (nic_sweep_unit_due(&u[i], t, busy, f)) {
                tx_t x = {i, t, t + FRAME_US, 0, {0}};
                memcpy(x.f, f, 12);
                if (near >= 0) {                       /* inside the detection window: a collision */
                    x.bad = 1;
                    for (int k = 0; k < ntx; k++)
                        if (t - tx[k].start <= DETECT_US)
                            tx[k].bad = 1;
                }
                if (x.bad)
                    x.f[5] ^= 0xFF;
                tx[ntx++] = x;
            }
        }
        for (int k = 0; k < ntx; k++)
            if (tx[k].bad)
                tx[k].f[5] |= 0;
        if (!ntx && nic_sweep_host_tick(h, t, f)) {
            for (int i = 0; i < n; i++)
                nic_sweep_unit_discover(&u[i], t);
        }
        if (h->done)
            return t;
    }
    return 0;
}

static void uid_of(uint8_t uid[12])
{
    for (int k = 0; k < 12; k++)
        uid[k] = (uint8_t)rnd();
}

int main(void)
{
    static nic_sweep_unit u[16];
    nic_sweep_host h;
    uint8_t uid[12], f[12], ack[12];
    /* the tag is CRC-16 over the whole UID */
    uint8_t one[12] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12};
    CHECK(nic_tag(one) == nic_crc16(one, 12));
    /* eight fresh Quakes, all on the default byte 0x5F, and one enrolled #1 */
    uint32_t worst = 0, sum = 0;
    int runs = 40;
    for (int r = 0; r < runs; r++) {
        for (int i = 0; i < 8; i++) {
            uid_of(uid);
            nic_sweep_unit_init(&u[i], uid, 0x5F, 0);
        }
        uid_of(uid);
        nic_sweep_unit_init(&u[8], uid, 0x51, 1);
        uint32_t t = round_on_bus(u, 9, &h);
        CHECK(t && h.n == 8);
        for (int i = 0; i < 8; i++) {
            int seen = 0;
            for (int k = 0; k < h.n; k++)
                seen += h.found[k].tag == u[i].tag && h.found[k].addr == 0x5F;
            CHECK(seen == 1 && u[i].answered);
        }
        CHECK(!u[8].answered);
        sum += t;
        if (t > worst)
            worst = t;
    }
    printf("eight units: %.2f s on average, %.2f s at worst of %d rounds\n", sum / 1e6 / runs, worst / 1e6, runs);
    /* nobody there: two silent windows */
    uint32_t t0 = round_on_bus(u, 0, &h);
    CHECK(h.n == 0 && t0 >= 2 * NIC_SWEEP_SILENCE_US && t0 < 2 * NIC_SWEEP_SILENCE_US + 10);
    /* ASSIGN_ADDR by tag: only the unit whose tag it is */
    uid_of(uid); nic_sweep_unit_init(&u[0], uid, 0x5F, 0);
    uid_of(uid); nic_sweep_unit_init(&u[1], uid, 0x5F, 0);
    nic_assign(u[1].tag, 0x5F, 0x52, 3, f);
    CHECK(!nic_sweep_unit_assign(&u[0], f, ack) && u[0].addr == 0x5F && !u[0].enrolled);
    CHECK(nic_sweep_unit_assign(&u[1], f, ack) && u[1].addr == 0x52 && u[1].slot == 3 && u[1].enrolled);
    nic_ctrl c;
    CHECK(!nic_ctrl_parse(ack, &c) && c.op == NIC_ACK && c.arg0 == NIC_ASSIGN_ADDR && c.addr == 0x52);
    nic_sweep_unit_discover(&u[1], 0);
    CHECK(!u[1].waiting);                              /* enrolled: silent */
    if (fails) {
        printf("%d checks failed\n", fails);
        return 1;
    }
    printf("the sweep: every check passed\n");
    return 0;
}

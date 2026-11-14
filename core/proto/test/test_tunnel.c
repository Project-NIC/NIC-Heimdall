/* The head's tunnel on a PC: the CRC against ModBus's own check value and a textbook frame; a
 * simulated Palatine passing exactly the package's length onto an arm, a ModBus slave on it; a
 * read of 13 registers and a write of 11, the largest that fit, and one over refused; FC06 and
 * FC16 echoed and confirmed; an exception told apart; a reply with a bit flipped, a node's ERROR
 * and silence each sent again once and then given up; a second package to a busy bridge refused,
 * two bridges at once; a stray reply; a reply from the wrong slave. */
#include "../nic_tunnel.h"

#include <stdio.h>
#include <string.h>

static int fails;
#define CHECK(x) do { if (!(x)) { fails++; fprintf(stderr, "%s:%d: %s\n", __FILE__, __LINE__, #x); } } while (0)

/* ------------------------------------------------------------------ a slave behind a bridge */

static uint16_t regs[2][64];          /* two slaves: 20 and 21 */
static int on_wire;                    /* bytes the bridge put on the arm */

static size_t slave(const uint8_t *q, size_t n, uint8_t *r)
{
    if (n < 4 || nic_mb_crc(q, n - 2) != (uint16_t)(q[n - 2] | q[n - 1] << 8) || q[0] < 20 || q[0] > 21)
        return 0;                                        /* nobody answers */
    uint16_t *h = regs[q[0] - 20];
    uint16_t start = (uint16_t)(q[2] << 8 | q[3]), count = (uint16_t)(q[4] << 8 | q[5]);
    size_t len;
    r[0] = q[0];
    r[1] = q[1];
    if (q[1] == 0x03 && start + count <= 64) {
        r[2] = (uint8_t)(2 * count);
        for (int i = 0; i < count; i++) { r[3 + 2 * i] = (uint8_t)(h[start + i] >> 8); r[4 + 2 * i] = (uint8_t)h[start + i]; }
        len = 3u + 2u * count;
    } else if (q[1] == 0x06 && start < 64) {
        h[start] = count;
        memcpy(r + 2, q + 2, 4);
        len = 6;
    } else if (q[1] == 0x10 && start + count <= 64) {
        for (int i = 0; i < count; i++) h[start + i] = (uint16_t)(q[7 + 2 * i] << 8 | q[8 + 2 * i]);
        memcpy(r + 2, q + 2, 4);
        len = 6;
    } else {
        r[1] = (uint8_t)(q[1] | 0x80);
        r[2] = 2;                                        /* illegal data address */
        len = 3;
    }
    uint16_t c = nic_mb_crc(r, len);
    r[len] = (uint8_t)c;
    r[len + 1] = (uint8_t)(c >> 8);
    return len + 2;
}

/* Palatine: exactly the length's bytes onto the arm, the reply back the same way */
static int bridge(const uint8_t down[32], uint8_t up[32])
{
    uint8_t r[64];
    on_wire = down[0];
    size_t n = slave(down + 1, down[0], r);
    if (!n || n > NIC_TUN_MAX) return -1;                /* a timeout, or too long: ERROR */
    memset(up, 0, 32);
    up[0] = (uint8_t)n;
    memcpy(up + 1, r, n);
    return 0;
}

int main(void)
{
    /* the CRC: ModBus's check value, and the textbook read */
    CHECK(nic_mb_crc((const uint8_t *)"123456789", 9) == 0x4B37);
    uint8_t pkg[NIC_TUN_MAX], down[32], up[32], again[32];
    size_t n = nic_mb_read(0x01, 0x0000, 10, pkg);
    CHECK(n == 8 && pkg[6] == 0xC5 && pkg[7] == 0xCD);

    nic_tunnel t;
    nic_tun_init(&t);
    nic_tun_answer a;
    for (int i = 0; i < 64; i++) regs[0][i] = (uint16_t)(0x1100 + i);

    /* a read of 13, the most a reply carries; a read of 14 refused */
    n = nic_mb_read(20, 5, 13, pkg);
    CHECK(n == 8 && !nic_tun_send(&t, 0x61, pkg, n, 0, down));
    CHECK(down[0] == 8 && !memcmp(down + 1, pkg, 8) && down[9] == 0);
    CHECK(!bridge(down, up) && on_wire == 8 && up[0] == 31);
    CHECK(nic_tun_reply(&t, 0x61, up, 10, &a, again) == NIC_TUN_OK);
    uint16_t v[16];
    CHECK(nic_mb_registers(&a, 13, v) == 13 && v[0] == 0x1105 && v[12] == 0x1111);
    CHECK(nic_mb_read(20, 0, 14, pkg) == 0);

    /* a write of 11, the most a package carries, confirmed by its echo; 12 refused */
    uint16_t w[12];
    for (int i = 0; i < 12; i++) w[i] = (uint16_t)(0xA000 + i);
    n = nic_mb_writen(20, 30, w, 11, pkg);
    CHECK(n == 31 && !nic_tun_send(&t, 0x61, pkg, n, 20, down) && down[0] == 31);
    CHECK(!bridge(down, up) && on_wire == 31);
    CHECK(nic_tun_reply(&t, 0x61, up, 30, &a, again) == NIC_TUN_OK && nic_mb_confirms(&a, pkg, n));
    CHECK(regs[0][30] == 0xA000 && regs[0][40] == 0xA00A && regs[0][41] == 0x1129);
    CHECK(nic_mb_writen(20, 30, w, 12, pkg) == 0);

    /* FC06: Pluvius's threshold, echoed */
    n = nic_mb_write1(20, 0x12, 900, pkg);
    CHECK(!nic_tun_send(&t, 0x61, pkg, n, 40, down) && !bridge(down, up));
    CHECK(nic_tun_reply(&t, 0x61, up, 50, &a, again) == NIC_TUN_OK && nic_mb_confirms(&a, pkg, n)
          && regs[0][0x12] == 900);

    /* an exception: past the slave's registers */
    n = nic_mb_read(20, 60, 10, pkg);
    CHECK(!nic_tun_send(&t, 0x61, pkg, n, 60, down) && !bridge(down, up));
    CHECK(nic_tun_reply(&t, 0x61, up, 70, &a, again) == NIC_TUN_EXCEPTION && a.exception == 2 && a.function == 3);

    /* a bit flipped on the way up: sent again, answered */
    n = nic_mb_read(20, 0, 2, pkg);
    CHECK(!nic_tun_send(&t, 0x61, pkg, n, 100, down) && !bridge(down, up));
    up[4] ^= 0x10;
    CHECK(nic_tun_reply(&t, 0x61, up, 110, &a, again) == NIC_TUN_WAIT && !memcmp(again, down, 32));
    CHECK(!bridge(again, up) && nic_tun_reply(&t, 0x61, up, 120, &a, again) == NIC_TUN_OK && t.bad == 1);

    /* twice bad: given up */
    CHECK(!nic_tun_send(&t, 0x61, pkg, n, 200, down) && !bridge(down, up));
    up[3] ^= 1;
    CHECK(nic_tun_reply(&t, 0x61, up, 210, &a, again) == NIC_TUN_WAIT);
    CHECK(nic_tun_reply(&t, 0x61, up, 220, &a, again) == NIC_TUN_FAILED && t.failures == 1);

    /* the node's ERROR, then silence: sent again, then given up */
    n = nic_mb_read(25, 0, 1, pkg);                      /* nobody at 25 */
    CHECK(!nic_tun_send(&t, 0x61, pkg, n, 300, down) && bridge(down, up) == -1);
    CHECK(nic_tun_error(&t, 0x61, 310, again) == NIC_TUN_WAIT && !memcmp(again, down, 32));
    uint8_t which = 0;
    CHECK(nic_tun_tick(&t, 400, &which, again) == NIC_TUN_WAIT);          /* 90 ms: not yet */
    CHECK(nic_tun_tick(&t, 510, &which, again) == NIC_TUN_FAILED && which == 0x61 && t.failures == 2);

    /* silence alone: once again after 200 ms, given up after 200 more */
    CHECK(!nic_tun_send(&t, 0x61, pkg, n, 1000, down));
    CHECK(nic_tun_tick(&t, 1199, &which, again) == NIC_TUN_WAIT);
    CHECK(nic_tun_tick(&t, 1200, &which, again) == NIC_TUN_WAIT && !memcmp(again, down, 32));
    CHECK(nic_tun_tick(&t, 1400, &which, again) == NIC_TUN_FAILED);

    /* one open a bridge; two bridges at once; a stray; the wrong slave */
    n = nic_mb_read(20, 0, 1, pkg);
    uint8_t pkg2[NIC_TUN_MAX], down2[32];
    size_t n2 = nic_mb_read(21, 0, 1, pkg2);
    CHECK(!nic_tun_send(&t, 0x61, pkg, n, 2000, down));
    CHECK(nic_tun_send(&t, 0x61, pkg2, n2, 2000, down2) == -1);
    CHECK(!nic_tun_send(&t, 0x62, pkg2, n2, 2000, down2));
    regs[1][0] = 0xBEEF;
    CHECK(!bridge(down2, up) && nic_tun_reply(&t, 0x62, up, 2010, &a, again) == NIC_TUN_OK
          && nic_mb_registers(&a, 1, v) == 1 && v[0] == 0xBEEF);
    CHECK(nic_tun_reply(&t, 0x62, up, 2020, &a, again) == NIC_TUN_STRAY && t.strays == 1);
    CHECK(nic_tun_reply(&t, 0x61, up, 2030, &a, again) == NIC_TUN_WAIT);  /* slave 21's answer to 20's question */
    CHECK(!bridge(again, up) && nic_tun_reply(&t, 0x61, up, 2040, &a, again) == NIC_TUN_OK && v[0] == 0xBEEF);
    CHECK(nic_mb_registers(&a, 1, v) == 1 && v[0] == 0x1100);
    pkg[3] ^= 1;                                         /* a package whose CRC is wrong is no package */
    CHECK(nic_tun_send(&t, 0x61, pkg, n, 3000, down) == -1);

    if (fails) {
        printf("test_tunnel: %d FAILED\n", fails);
        return 1;
    }
    printf("test_tunnel: all passed\n");
    return 0;
}

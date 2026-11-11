/* The protocol core on a PC: every rule of PROTOCOL.md §1, §2 and §9 this library carries. */
#include "../nic_proto.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int fails;
#define CHECK(x) do { if (!(x)) { fails++; fprintf(stderr, "%s:%d: %s\n", __FILE__, __LINE__, #x); } } while (0)

static void addresses(void)
{
    uint8_t a;
    unsigned t, n;
    /* the document's own examples */
    CHECK(!nic_addr_pack(NIC_NODBUS, 5, 2, &a) && a == 0x52);       /* Quake #2 */
    CHECK(!nic_addr_pack(NIC_MINI, 1, 2, &a) && a == 0x12);         /* Gauss #2 */
    CHECK(!nic_addr_pack(NIC_MODBUS, 5, 2, &a) && a == 0x16);       /* Pluvius #2 */
    CHECK(!nic_addr_pack(NIC_MODBUS, 4, 0, &a) && a == 16);         /* Babel, bare, #0 */
    CHECK(!nic_addr_pack(NIC_MODBUS, 7, 0, &a) && a == 28);         /* Sakura #0 */
    CHECK(!nic_addr_pack(NIC_MODBUS, 61, 3, &a) && a == 247);       /* Modbus's ceiling */
    CHECK(!nic_addr_pack(NIC_MINI, 3, 1, &a) && a == 0x31);         /* Pascal 1 … */
    CHECK(!nic_addr_pack(NIC_NODBUS, 3, 1, &a) && a == 0x31);       /* … and Argus 1, one byte */
    /* the ranges */
    CHECK(nic_addr_pack(NIC_NODBUS, 0, 1, &a) && nic_addr_pack(NIC_NODBUS, 15, 1, &a));
    CHECK(nic_addr_pack(NIC_NODBUS, 1, 0, &a) && nic_addr_pack(NIC_MINI, 14, 16, &a));
    CHECK(nic_addr_pack(NIC_MODBUS, 3, 3, &a) && nic_addr_pack(NIC_MODBUS, 62, 0, &a));
    CHECK(nic_addr_pack(NIC_MODBUS, 4, 4, &a));
    for (unsigned x = 0; x < 256; x++) {        /* NodBus lands 0x11..0xEF, never 0x01..0x0F */
        int ok = !nic_addr_unpack(NIC_NODBUS, (uint8_t)x, &t, &n);
        CHECK(ok == (x >= 0x11 && x <= 0xEF && (x & 15) != 0));
        ok = !nic_addr_unpack(NIC_MODBUS, (uint8_t)x, &t, &n);
        CHECK(ok == (x >= 16 && x <= 247));
    }
    CHECK(!nic_addr_unpack(NIC_NODBUS, 0x52, &t, &n) && t == 5 && n == 2);
    CHECK(nic_addr_unpack(NIC_NODBUS, NIC_BCAST, &t, &n));         /* 0xFF is the broadcast */
}

static void data(void)
{
    uint8_t pl[32], f[40], m[16];
    const uint8_t *p;
    size_t plen;
    nic_header h = {0x34, 17, 0x52, 3, NIC_DATA, NIC_CLIPPED | NIC_ALARM}, g;
    for (int i = 0; i < 32; i++)
        pl[i] = (uint8_t)(i * 7);
    CHECK(nic_data_build(&h, pl, 32, f) == 40 && nic_classify(40) == NIC_F_SPUR);
    CHECK(!nic_data_parse(f, 40, &g, &p, &plen) && plen == 32 && !memcmp(p, pl, 32));
    CHECK(!memcmp(&g, &h, sizeof h) && (g.status & 15) == NIC_CLIPPED && (g.status & NIC_ALARM));
    CHECK(nic_crc16((const uint8_t *)"123456789", 9) == 0x29B1);   /* CCITT-FALSE's check value */
    CHECK(f[38] == (uint8_t)nic_crc16(f, 38) && f[39] == nic_crc16(f, 38) >> 8);   /* little-endian */
    for (int i = 0; i < 40; i++) {                                /* every single bit is caught */
        for (int b = 0; b < 8; b++) {
            f[i] ^= (uint8_t)(1 << b);
            CHECK(nic_data_parse(f, 40, &g, &p, &plen) == -1);
            f[i] ^= (uint8_t)(1 << b);
        }
    }
    CHECK(nic_data_build(&h, pl, 8, m) == 16 && nic_classify(16) == NIC_F_MINI);
    CHECK(!nic_data_parse(m, 16, &g, &p, &plen) && plen == 8 && !memcmp(p, pl, 8));
    CHECK(nic_data_build(&h, pl, 16, m) == 0 && nic_data_parse(f, 39, &g, &p, &plen) == -1);
    CHECK(nic_classify(12) == NIC_F_CTRL && nic_classify(48) == NIC_F_TRUNK && nic_classify(41) == NIC_BAD);
}

static void trunk(void)
{
    uint8_t buf[48], pl[32];
    const uint8_t *p;
    nic_header h = {0x05, 99, 0x52, 2, NIC_DATA, 0}, g;
    uint32_t sec;
    memset(pl, 0xA5, 32);
    memset(buf, 0xEE, sizeof buf);
    nic_data_build(&h, pl, 32, buf + 3);                          /* the spur frame lands 3 in */
    uint8_t before[40];
    memcpy(before, buf + 3, 38);
    uint32_t card = (1790856000u & ~0xFFu) | 0x04;                  /* the card at …04, the node at …05 */
    CHECK(!nic_trunk_complete(buf, card));
    CHECK(!memcmp(buf + 3, before, 38));                           /* never moved */
    CHECK(buf[41] == 0 && buf[42] == 0 && buf[43] == 0 && buf[44] == 0 && buf[45] == 0);
    CHECK(!nic_trunk_parse(buf, &sec, &g, &p) && !memcmp(p, pl, 32) && g.addr == 0x52 && g.frame == 99);
    CHECK(sec == card + 1);
    /* across a wrap of the low byte, both ways */
    h.unix0 = 0xFF;
    nic_data_build(&h, pl, 32, buf + 3);
    CHECK(!nic_trunk_complete(buf, 0x12345601u) && !nic_trunk_parse(buf, &sec, &g, &p) && sec == 0x123455FFu);
    h.unix0 = 0x01;
    nic_data_build(&h, pl, 32, buf + 3);
    CHECK(!nic_trunk_complete(buf, 0x123455FEu) && !nic_trunk_parse(buf, &sec, &g, &p) && sec == 0x12345601u);
    /* the time most significant first */
    CHECK(buf[0] == 0x12 && buf[1] == 0x34 && buf[2] == 0x56 && buf[3] == 0x01);
    buf[10] ^= 1;
    CHECK(nic_trunk_parse(buf, &sec, &g, &p) == -1);
    nic_data_build(&h, pl, 32, buf + 3);
    buf[20] ^= 1;
    CHECK(nic_trunk_complete(buf, 0x12345600u) == -1);            /* a bad node frame is refused */
}

static void control(void)
{
    uint8_t f[12];
    nic_ctrl c = {0x34, 0, 0x31, 0x12, NIC_SET, 0x04, 0x34, 0x12}, d;
    nic_ctrl_build(&c, f);
    CHECK(!nic_ctrl_parse(f, &d) && !memcmp(&c, &d, sizeof c) && nic_ctrl_value(&d) == 0x1234);
    CHECK(f[8] == 0 && f[9] == 0);
    /* down through an Argus: the sonde sees its own address and SUB 0 */
    CHECK(!nic_argus_down(f) && !nic_ctrl_parse(f, &d) && d.addr == 0x12 && d.sub == 0 && d.op == NIC_SET);
    CHECK(nic_argus_down(f) == -1);                                /* SUB 0: the frame is the Argus's */
    /* the sonde's ACK up: the Argus's NUMBER in front, the sonde in SUB */
    nic_ctrl ack = {0x34, 0, 0x12, 0, NIC_ACK, NIC_SET, 0, 0};
    nic_ctrl_build(&ack, f);
    nic_argus_up(f, 0x31);
    CHECK(!nic_ctrl_parse(f, &d) && d.addr == 0x31 && d.sub == 0x12 && d.op == NIC_ACK && d.arg0 == NIC_SET);
    f[4] ^= 0x80;
    CHECK(nic_ctrl_parse(f, &d) == -1);
    /* the opcode numbers are the table's */
    CHECK(NIC_DISCOVER == 1 && NIC_GET == 9 && NIC_SET == 10 && NIC_ERROR == 12 && NIC_EVENT == 14 &&
          NIC_FLOOR == 17 && NIC_LOAD == 18 && NIC_APPLY == 19);
    CHECK(NIC_STREAM == 9 && NIC_HEALTH == 8 && NIC_REGISTER == 5);
    uint8_t pl[32], v[6] = {1, 2, 3, 4, 5, 6};
    nic_register_payload(0xFF01, v, 6, pl);
    CHECK(nic_register_of(pl) == 0xFF01 && !memcmp(pl + 2, v, 6) && pl[8] == 0 && pl[31] == 0);
}

static void roster(void)
{
    static nic_roster r;
    nic_roster_init(&r);
    nic_unit card = {NIC_NODBUS, 0x21, NIC_NONE, 1, 0};
    uint16_t ic = nic_roster_put(&r, &card);
    nic_unit quake = {NIC_NODBUS, 0x52, ic, 2, 3}, argus = {NIC_NODBUS, 0x31, ic, 3, 1};
    uint16_t iq = nic_roster_put(&r, &quake), ia = nic_roster_put(&r, &argus);
    nic_unit gauss = {NIC_MINI, 0x12, ia, 1, 2}, pascal = {NIC_MINI, 0x31, ia, 2, 3};
    nic_roster_put(&r, &gauss);
    nic_roster_put(&r, &pascal);
    nic_unit pal = {NIC_NODBUS, 0x61, ic, 4, 2}, pluv = {NIC_MODBUS, 0x16, NIC_NONE, 1, 0};
    uint16_t ip = nic_roster_put(&r, &pal);
    pluv.parent = ip;
    nic_roster_put(&r, &pluv);
    /* one load each; the colliding byte 0x31 kept apart by the bus */
    CHECK(nic_roster_find(&r, NIC_NODBUS, 0x52) == iq && nic_roster_find(&r, NIC_NODBUS, 0x31) == ia);
    CHECK(r.rows[nic_roster_find(&r, NIC_MINI, 0x31)].parent == ia);
    CHECK(nic_roster_find(&r, NIC_MODBUS, 0x16) != NIC_NONE && nic_roster_find(&r, NIC_NODBUS, 0x16) == NIC_NONE);
    /* a re-sweep replaces, never duplicates */
    quake.slot = 5;
    CHECK(nic_roster_put(&r, &quake) == iq && r.rows[iq].slot == 5 && r.n == 7);
    /* the Argus off: its sondes with it, the rest re-indexed and still found */
    CHECK(nic_roster_drop(&r, NIC_NODBUS, 0x31) == 3 && r.n == 4);
    CHECK(nic_roster_find(&r, NIC_MINI, 0x12) == NIC_NONE && nic_roster_find(&r, NIC_MINI, 0x31) == NIC_NONE);
    uint16_t q = nic_roster_find(&r, NIC_NODBUS, 0x52), p = nic_roster_find(&r, NIC_MODBUS, 0x16);
    CHECK(q != NIC_NONE && r.rows[q].addr == 0x52 && r.rows[r.rows[q].parent].addr == 0x21);
    CHECK(p != NIC_NONE && r.rows[r.rows[p].parent].addr == 0x61);
    CHECK(nic_roster_drop(&r, NIC_NODBUS, 0x21) == 4 && r.n == 0);
}

int main(void)
{
    addresses();
    data();
    trunk();
    control();
    roster();
    if (fails) {
        printf("%d checks failed\n", fails);
        return 1;
    }
    printf("the protocol core: every check passed\n");
    return 0;
}

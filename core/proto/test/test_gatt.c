/* The head's GATT side on a PC: the handset contract's own status example written byte for byte,
 * with the counter and the corrections table; a name with a quote escaped; a document too long for
 * its buffer refused; the document cut into notifications of 20 B and of 244 B and joined again;
 * every command of the contract signed as the phone signs it, COBS-framed, written in pieces of
 * 20 B and read; `refresh` under counter 0 with a zero tag; a changed byte, a replayed counter, a
 * write under counter 0 that is not a read, a SET_K1 from the phone, unknown and broken JSON —
 * each refused; two commands in one write; the mirror's `set` and `mirror`, a bad value refused;
 * the pending values in the status document; one unit's whole mirror as its line. */
#include "../nic_gatt.h"

#include <stdio.h>
#include <string.h>

static int fails;
#define CHECK(x) do { if (!(x)) { fails++; fprintf(stderr, "%s:%d: %s\n", __FILE__, __LINE__, #x); } } while (0)

static size_t cobs(const uint8_t *in, size_t n, uint8_t *out)
{
    size_t o = 1, code_at = 0;
    uint8_t code = 1;
    for (size_t i = 0; i < n; i++) {
        if (in[i] == 0) {
            out[code_at] = code; code_at = o++; code = 1;
        } else {
            out[o++] = in[i];
            if (++code == 0xFF) { out[code_at] = code; code_at = o++; code = 1; }
        }
    }
    out[code_at] = code;
    out[o++] = 0;
    return o;
}

static nic_cmd_keys phone;           /* the phone's copy: K1 from the server, K0 never — signs with both */

/* a COMMAND as the phone writes it: framed, closed by a zero */
static size_t command(uint32_t counter, const char *json, int zero_tag, uint8_t type, uint8_t *out)
{
    uint8_t m[1100];
    size_t j = strlen(json);
    m[0] = type;
    for (int i = 0; i < 4; i++) m[1 + i] = (uint8_t)(counter >> (8 * i));
    memcpy(m + 5, json, j);
    if (zero_tag) memset(m + 5 + j, 0, 8);
    else nic_cmd_sign(&phone, m, 5 + j, 0);
    return cobs(m, 5 + j + 8, out);
}

static nic_cmd_keys head;
static nic_gatt_rx rx;

/* a framed write in pieces of 20 B, as BLE gives them */
static enum nic_gatt_result write_all(const uint8_t *b, size_t n, nic_gatt_cmd *c, enum nic_cmd_result *why)
{
    enum nic_gatt_result r = NIC_GW_MORE;
    for (size_t at = 0; at < n;) {
        size_t piece = n - at < 20 ? n - at : 20, used;
        r = nic_gatt_write(&rx, &head, b + at, piece, &used, c, why);
        at += used;
        if (r != NIC_GW_MORE) break;
    }
    return r;
}

int main(void)
{
    /* the contract's example */
    static const nic_st_sensor quake[4] = {{0, "ICM", 0}, {1, "ADXL355", 2}, {2, "SCL3300", 0}, {3, "RM3100", 0}};
    static const nic_st_sensor pal[2] = {{1, "AIR1", 0}, {9, "SOIL1", 0}};
    static const nic_st_node nodes[2] = {{1, 5, NIC_BUS_NODBUS, 0, "Quake", quake, 4},
                                         {2, 6, NIC_BUS_NODBUS, 1, "Palatine", pal, 2}};
    static const nic_st_term terms[3] = {{"cab", "antenna cable", 20, 5, 100, 0},
                                         {"int", "antenna + receiver", -1, 0, 50, 0},
                                         {"route", "remoted receiver, clock run", -1, 0, 13000, 1}};
    nic_status st = {0, 41, nodes, 2, terms, 3, 13150, 0, 0};
    static char doc[2048];
    size_t len = nic_status_write(&st, doc, sizeof doc);
    const char *want =
        "{\"go\":false,\"counter\":41,\"nodes\":["
        "{\"n\":1,\"bus\":\"nodbus\",\"type\":5,\"label\":\"Quake\",\"ok\":false,\"sensors\":["
        "{\"id\":0,\"name\":\"ICM\",\"h\":0},{\"id\":1,\"name\":\"ADXL355\",\"h\":2},"
        "{\"id\":2,\"name\":\"SCL3300\",\"h\":0},{\"id\":3,\"name\":\"RM3100\",\"h\":0}]},"
        "{\"n\":2,\"bus\":\"nodbus\",\"type\":6,\"label\":\"Palatine\",\"ok\":true,\"sensors\":["
        "{\"id\":1,\"name\":\"AIR1\",\"h\":0},{\"id\":9,\"name\":\"SOIL1\",\"h\":0}]}],"
        "\"corr\":{\"total_ns\":13150,\"terms\":["
        "{\"id\":\"cab\",\"label\":\"antenna cable\",\"m\":20,\"ns_per_m\":5,\"ns\":100,\"src\":\"typed\"},"
        "{\"id\":\"int\",\"label\":\"antenna + receiver\",\"ns\":50,\"src\":\"typed\"},"
        "{\"id\":\"route\",\"label\":\"remoted receiver, clock run\",\"ns\":13000,\"src\":\"ranged\"}]}}\n";
    CHECK(len == strlen(want) && !memcmp(doc, want, len));
    CHECK(nic_status_write(&st, doc, len - 1) == 0);

    static const nic_st_sensor odd[1] = {{7, "say \"hi\"\\", 3}};
    static const nic_st_node one[1] = {{3, 18, NIC_BUS_MODBUS, 0, 0, odd, 1}};
    nic_status st2 = {1, 0, one, 1, 0, 0, 0, 0, 0};
    len = nic_status_write(&st2, doc, sizeof doc);
    doc[len] = 0;
    CHECK(len && strstr(doc, "\"name\":\"say \\\"hi\\\"\\\\\"") && strstr(doc, "\"bus\":\"modbus\"")
          && !strstr(doc, "label") && !strstr(doc, "corr") && doc[len - 1] == '\n');

    /* cut into notifications and joined */
    len = nic_status_write(&st, doc, sizeof doc);
    size_t mtus[2] = {20, 244};
    for (int k = 0; k < 2; k++) {
        nic_notify nf;
        char joined[2048];
        size_t at = 0, got, pieces = 0;
        uint8_t chunk[256];
        nic_notify_start(&nf, doc, len);
        while ((got = nic_notify_next(&nf, mtus[k], chunk)) != 0) {
            CHECK(got <= mtus[k]);
            memcpy(joined + at, chunk, got);
            at += got;
            pieces++;
        }
        CHECK(at == len && !memcmp(joined, doc, len) && pieces == (len + mtus[k] - 1) / mtus[k]);
    }

    /* the commands */
    memcpy(head.k0, "K0K0K0K0", 8); memcpy(head.k1, "k1k1k1k1", 8);
    head.station = 0x123456; head.counter = 41;
    phone = head;
    nic_gatt_rx_init(&rx);
    uint8_t w[1200];
    nic_gatt_cmd c;
    enum nic_cmd_result why;

    size_t n = command(0, "{\"cmd\":\"refresh\"}", 1, NIC_MSG_COMMAND, w);
    CHECK(write_all(w, n, &c, &why) == NIC_GW_TAKEN && c.kind == NIC_GC_REFRESH && why == NIC_CMD_READ && head.counter == 41);

    n = command(42, "{\"cmd\":\"recommission\"}", 0, NIC_MSG_COMMAND, w);
    CHECK(write_all(w, n, &c, &why) == NIC_GW_TAKEN && c.kind == NIC_GC_RECOMMISSION && head.counter == 42);
    CHECK(write_all(w, n, &c, &why) == NIC_GW_REFUSED && why == NIC_CMD_STALE);   /* replayed */

    n = command(43, "{\"cmd\":\"provision\",\"node\":1}", 0, NIC_MSG_COMMAND, w);
    CHECK(write_all(w, n, &c, &why) == NIC_GW_TAKEN && c.kind == NIC_GC_PROVISION && c.node == 1);

    n = command(44, "{ \"cmd\" : \"corr\", \"set\" : [ {\"id\":\"cab\",\"m\":25}, {\"m\":3,\"id\":\"int\",\"x\":[1,{\"y\":2}]} ] }", 0, NIC_MSG_COMMAND, w);
    CHECK(write_all(w, n, &c, &why) == NIC_GW_TAKEN && c.kind == NIC_GC_CORR && c.nterms == 2
          && !strcmp(c.terms[0].id, "cab") && c.terms[0].m == 25 && !strcmp(c.terms[1].id, "int") && c.terms[1].m == 3);

    n = command(45, "{\"cmd\":\"card\",\"out\":1}", 0, NIC_MSG_COMMAND, w);
    CHECK(write_all(w, n, &c, &why) == NIC_GW_TAKEN && c.kind == NIC_GC_CARD && c.slot == 1);

    n = command(46, "{\"cmd\":\"card\",\"out\":1}", 0, NIC_MSG_COMMAND, w);
    w[9] ^= 0x20;                                      /* a byte changed on the way */
    CHECK(write_all(w, n, &c, &why) == NIC_GW_REFUSED && why == NIC_CMD_FORGED);

    n = command(0, "{\"cmd\":\"recommission\"}", 1, NIC_MSG_COMMAND, w);
    CHECK(write_all(w, n, &c, &why) == NIC_GW_REFUSED);                          /* under 0, not a read */

    n = command(47, "{\"cmd\":\"selfdestruct\"}", 0, NIC_MSG_COMMAND, w);
    CHECK(write_all(w, n, &c, &why) == NIC_GW_BAD);
    n = command(48, "{\"cmd\":\"card\",\"out\":1", 0, NIC_MSG_COMMAND, w);
    CHECK(write_all(w, n, &c, &why) == NIC_GW_BAD);
    n = command(49, "{\"cmd\":\"card\",\"out\":7}", 0, NIC_MSG_COMMAND, w);
    CHECK(write_all(w, n, &c, &why) == NIC_GW_BAD);
    n = command(50, "{\"cmd\":\"x\"}", 0, NIC_MSG_SET_K1, w);
    CHECK(write_all(w, n, &c, &why) == NIC_GW_BAD);                              /* never the phone's */

    /* two commands in one write */
    size_t n1 = command(51, "{\"cmd\":\"provision\",\"node\":3}", 0, NIC_MSG_COMMAND, w);
    size_t n2 = command(52, "{\"cmd\":\"card\",\"out\":0}", 0, NIC_MSG_COMMAND, w + n1);
    size_t used;
    CHECK(nic_gatt_write(&rx, &head, w, n1 + n2, &used, &c, &why) == NIC_GW_TAKEN && used == n1 && c.node == 3);
    CHECK(nic_gatt_write(&rx, &head, w + used, n1 + n2 - used, &used, &c, &why) == NIC_GW_TAKEN
          && c.kind == NIC_GC_CARD && c.slot == 0 && head.counter == 52);

    /* the mirror: set, its value read; mirror; a value that is not hex, odd or too long refused */
    n = command(53, "{\"cmd\":\"set\",\"bus\":\"modbus\",\"type\":5,\"n\":0,\"reg\":18,\"v\":\"0384\"}", 0, NIC_MSG_COMMAND, w);
    CHECK(write_all(w, n, &c, &why) == NIC_GW_TAKEN && c.kind == NIC_GC_SET && c.bus == NIC_BUS_MODBUS && c.type == 5
          && c.n == 0 && c.reg == 18 && c.vlen == 2 && c.v[0] == 0x03 && c.v[1] == 0x84);
    n = command(54, "{\"cmd\":\"mirror\",\"bus\":\"nodbus\",\"type\":5,\"n\":1}", 0, NIC_MSG_COMMAND, w);
    CHECK(write_all(w, n, &c, &why) == NIC_GW_TAKEN && c.kind == NIC_GC_MIRROR && c.bus == NIC_BUS_NODBUS && c.n == 1);
    static const char *bad[] = {
        "{\"cmd\":\"set\",\"bus\":\"nodbus\",\"type\":5,\"n\":1,\"reg\":19,\"v\":\"03G4\"}",
        "{\"cmd\":\"set\",\"bus\":\"nodbus\",\"type\":5,\"n\":1,\"reg\":19,\"v\":\"038\"}",
        "{\"cmd\":\"set\",\"bus\":\"can\",\"type\":5,\"n\":1,\"reg\":19,\"v\":\"0384\"}",
        "{\"cmd\":\"set\",\"bus\":\"nodbus\",\"type\":5,\"n\":1,\"v\":\"0384\"}",
        "{\"cmd\":\"mirror\",\"bus\":\"nodbus\",\"type\":5}"};
    for (int i = 0; i < 5; i++) {
        n = command((uint32_t)(55 + i), bad[i], 0, NIC_MSG_COMMAND, w);
        CHECK(write_all(w, n, &c, &why) == NIC_GW_BAD);
    }
    char longv[400] = "{\"cmd\":\"set\",\"bus\":\"nodbus\",\"type\":5,\"n\":1,\"reg\":19,\"v\":\"";
    for (int i = 0; i < 2 * 157; i++) strcat(longv, "a");
    strcat(longv, "\"}");
    n = command(60, longv, 0, NIC_MSG_COMMAND, w);
    CHECK(write_all(w, n, &c, &why) == NIC_GW_TAKEN && c.vlen == 157 && c.v[156] == 0xAA);
    longv[strlen(longv) - 2] = 0;
    strcat(longv, "aa\"}");
    n = command(61, longv, 0, NIC_MSG_COMMAND, w);
    CHECK(write_all(w, n, &c, &why) == NIC_GW_BAD);                              /* 158 B */

    /* the pending values, and a unit's line */
    static const uint8_t v19[2] = {0x03, 0x84}, v16[3] = {0xAB, 0xCD, 0xEF};
    static const nic_st_pending pend[2] = {{NIC_BUS_NODBUS, 5, 1, 19, 2, 3, v19}, {NIC_BUS_MODBUS, 5, 0, 18, 2, 2, v19}};
    nic_status st3 = {1, 7, 0, 0, 0, 0, 0, pend, 2};
    len = nic_status_write(&st3, doc, sizeof doc);
    doc[len] = 0;
    CHECK(!strcmp(doc, "{\"go\":true,\"counter\":7,\"nodes\":[],\"mirror\":["
                       "{\"bus\":\"nodbus\",\"type\":5,\"n\":1,\"reg\":19,\"v\":\"0384\",\"state\":\"sent\"},"
                       "{\"bus\":\"modbus\",\"type\":5,\"n\":0,\"reg\":18,\"v\":\"0384\",\"state\":\"edited\"}]}\n"));
    static const nic_st_reg regs[2] = {{19, 2, 0, v19}, {0xFE03, 3, 1, v16}};
    len = nic_mirror_line(NIC_BUS_NODBUS, 5, 1, regs, 2, doc, sizeof doc);
    doc[len] = 0;
    CHECK(!strcmp(doc, "{\"unit\":{\"bus\":\"nodbus\",\"type\":5,\"n\":1},\"regs\":["
                       "{\"reg\":19,\"v\":\"0384\",\"ro\":false},{\"reg\":65027,\"v\":\"abcdef\",\"ro\":true}]}\n"));
    CHECK(nic_mirror_line(NIC_BUS_NODBUS, 5, 1, regs, 2, doc, len - 1) == 0);

    if (fails) {
        printf("test_gatt: %d FAILED\n", fails);
        return 1;
    }
    printf("test_gatt: all passed\n");
    return 0;
}

/* The head's command check against the session's own WINDOW (../archive/ref/c/session.c). */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../nic_cmd.h"
#include "session.h"

static int fails;
#define CHECK(c) do { if (!(c)) { printf("FAIL %s:%d %s\n", __FILE__, __LINE__, #c); fails++; } } while (0)

static size_t unhex(const char *h, uint8_t *out)
{
    size_t n = 0;
    for (; h[0] && h[1]; h += 2) {
        unsigned b;
        sscanf(h, "%2x", &b);
        out[n++] = (uint8_t)b;
    }
    return n;
}

int main(void)
{
    nic_cmd_keys s = {{1, 2, 3, 4, 5, 6, 7, 8}, {9, 10, 11, 12, 13, 14, 15, 16}, 0x123456, 41, 0, 0};
    uint8_t key[16], *msg, *raw;
    size_t len, rawlen;
    memcpy(key, s.k0, 8);
    memcpy(key + 8, s.k1, 8);
    /* a WINDOW from the session's code is a command the check takes */
    CHECK(ses_window(key, s.station, 42, 0xFF, 1700000000u, 600, &msg, &len) == HMC_OK);
    CHECK(ses_unframe(msg, len - 1, &raw, &rawlen) == HMC_OK && rawlen == 22);
    CHECK(nic_cmd_check(&s, raw, rawlen, 0) == NIC_CMD_OK && s.counter == 42);
    CHECK(nic_cmd_check(&s, raw, rawlen, 0) == NIC_CMD_STALE && s.stale == 1);      /* a replay */
    /* signed here, checked by the session's parser */
    raw[1] = 43;
    nic_cmd_sign(&s, raw, 14, 0);
    ses_window_t w;
    CHECK(ses_window_parse(key, s.station, 42, raw + 1, 21, &w) == HMC_OK && w.counter == 43);
    /* a byte changed, another station, K0 alone where K0‖K1 signed: forged, the counter kept */
    raw[6] ^= 1;
    CHECK(nic_cmd_check(&s, raw, rawlen, 0) == NIC_CMD_FORGED && s.counter == 42);
    raw[6] ^= 1;
    nic_cmd_keys other = s;
    other.station = 0x123457;
    CHECK(nic_cmd_check(&other, raw, rawlen, 0) == NIC_CMD_FORGED);
    CHECK(nic_cmd_check(&s, raw, rawlen, 1) == NIC_CMD_FORGED && s.forged == 2);
    /* a stale counter is dropped before its tag is looked at */
    raw[1] = 40;
    CHECK(nic_cmd_check(&s, raw, rawlen, 0) == NIC_CMD_STALE);
    /* SET K1 under K0 alone; after it the old K1 signs nothing */
    uint8_t set[1 + 4 + 8 + 8] = {9, 50, 0, 0, 0, 0xA1, 0xA2, 0xA3, 0xA4, 0xA5, 0xA6, 0xA7, 0xA8};
    nic_cmd_sign(&s, set, 13, 1);
    CHECK(nic_cmd_check(&s, set, 21, 0) == NIC_CMD_FORGED);
    CHECK(nic_cmd_check(&s, set, 21, 1) == NIC_CMD_OK && s.counter == 50);
    nic_cmd_rekey(&s, set + 5);
    raw[1] = 51;
    CHECK(nic_cmd_check(&s, raw, rawlen, 0) == NIC_CMD_FORGED);
    nic_cmd_sign(&s, raw, 14, 0);
    CHECK(nic_cmd_check(&s, raw, rawlen, 0) == NIC_CMD_OK && s.counter == 51);
    /* the Python reference's bytes (../archive/ref/session.py): a COMMAND under K0‖K1 and a
     * SET_K1 under K0, its new key unwrapped */
    static const char *const cmd_hex = "082b0000007b22636d64223a227265636f6d6d697373696f6e227d"
                                       "bac25fff569a5421";
    static const char *const set_hex = "0932000000fad346f5fbf21835636febd97c85a32a";
    uint8_t v[64];
    size_t vn = unhex(cmd_hex, v);
    nic_cmd_keys f = {{1, 2, 3, 4, 5, 6, 7, 8}, {9, 10, 11, 12, 13, 14, 15, 16}, 0x123456, 42, 0, 0};
    CHECK(nic_cmd_take(&f, v, vn) == NIC_CMD_OK && f.counter == 43);
    CHECK(nic_cmd_take(&f, v, vn) == NIC_CMD_STALE);
    vn = unhex(set_hex, v);
    CHECK(nic_cmd_take(&f, v, vn) == NIC_CMD_OK && f.counter == 50);
    CHECK(f.k1[0] == 0xA1 && f.k1[7] == 0xA8);
    CHECK(nic_cmd_take(&f, v, vn) == NIC_CMD_STALE);
    /* a read under counter 0 moves nothing; an unknown type is refused */
    uint8_t rd[13] = {8, 0, 0, 0, 0};
    CHECK(nic_cmd_take(&f, rd, 13) == NIC_CMD_READ && f.counter == 50);
    rd[0] = 5;
    CHECK(nic_cmd_take(&f, rd, 13) == NIC_CMD_TYPE);
    /* sizes */
    CHECK(nic_cmd_check(&s, raw, 12, 0) == NIC_CMD_SIZE);
    free(msg);
    free(raw);
    if (fails) {
        printf("%d checks failed\n", fails);
        return 1;
    }
    printf("the head's command check: every check passed\n");
    return 0;
}

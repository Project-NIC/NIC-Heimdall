/* The head's command check against the session's own WINDOW (../archive/ref/c/session.c). */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../nic_cmd.h"
#include "session.h"

static int fails;
#define CHECK(c) do { if (!(c)) { printf("FAIL %s:%d %s\n", __FILE__, __LINE__, #c); fails++; } } while (0)

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

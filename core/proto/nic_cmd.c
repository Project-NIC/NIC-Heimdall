/* The head's command check; see nic_cmd.h. */
#include "nic_cmd.h"

#include <string.h>

#include "session.h"

static void tag_of(const nic_cmd_keys *s, const uint8_t *msg, size_t n, int k0_alone, uint8_t t[8])
{
    uint8_t key[16], mac[32], buf[3 + NIC_CMD_MAX];
    memcpy(key, s->k0, 8);
    memcpy(key + 8, s->k1, 8);
    buf[0] = (uint8_t)s->station;
    buf[1] = (uint8_t)(s->station >> 8);
    buf[2] = (uint8_t)(s->station >> 16);
    memcpy(buf + 3, msg, n);
    ses_hmac_sha256(key, k0_alone ? 8 : 16, buf, 3 + n, mac);
    memcpy(t, mac, 8);
}

enum nic_cmd_result nic_cmd_check(nic_cmd_keys *s, const uint8_t *msg, size_t n, int k0_alone)
{
    if (n < 1 + 4 + 8 || n > NIC_CMD_MAX + 8)
        return NIC_CMD_SIZE;
    uint32_t c = (uint32_t)msg[1] | (uint32_t)msg[2] << 8 | (uint32_t)msg[3] << 16
                 | (uint32_t)msg[4] << 24;
    if (c <= s->counter) {
        s->stale++;
        return NIC_CMD_STALE;
    }
    uint8_t t[8], d = 0;
    tag_of(s, msg, n - 8, k0_alone, t);
    for (int i = 0; i < 8; i++)          /* compared in constant time */
        d |= (uint8_t)(t[i] ^ msg[n - 8 + i]);
    if (d) {
        s->forged++;
        return NIC_CMD_FORGED;
    }
    s->counter = c;
    return NIC_CMD_OK;
}

void nic_cmd_sign(const nic_cmd_keys *s, uint8_t *msg, size_t n, int k0_alone)
{
    tag_of(s, msg, n, k0_alone, msg + n);
}

void nic_cmd_rekey(nic_cmd_keys *s, const uint8_t k1[8])
{
    memcpy(s->k1, k1, 8);
}

void nic_cmd_pad(const nic_cmd_keys *s, uint32_t counter, uint8_t pad[8])
{
    uint8_t d[7] = {(uint8_t)s->station, (uint8_t)(s->station >> 8), (uint8_t)(s->station >> 16),
                    (uint8_t)counter, (uint8_t)(counter >> 8), (uint8_t)(counter >> 16),
                    (uint8_t)(counter >> 24)}, mac[32];
    ses_hmac_sha256(s->k0, 8, d, 7, mac);
    memcpy(pad, mac, 8);
}

enum nic_cmd_result nic_cmd_take(nic_cmd_keys *s, const uint8_t *msg, size_t n)
{
    if (n < 1 + 4 + 8)
        return NIC_CMD_SIZE;
    switch (msg[0]) {
    case NIC_MSG_COMMAND:
        if ((msg[1] | msg[2] | msg[3] | msg[4]) == 0)
            return NIC_CMD_READ;
        return nic_cmd_check(s, msg, n, 0);
    case NIC_MSG_WINDOW:
        return n == 22 ? nic_cmd_check(s, msg, n, 0) : NIC_CMD_SIZE;
    case NIC_MSG_SET_K1: {
        if (n != 21)
            return NIC_CMD_SIZE;
        enum nic_cmd_result r = nic_cmd_check(s, msg, n, 1);
        if (r != NIC_CMD_OK)
            return r;
        uint8_t pad[8], k1[8];
        nic_cmd_pad(s, s->counter, pad);
        for (int i = 0; i < 8; i++)
            k1[i] = (uint8_t)(msg[5 + i] ^ pad[i]);
        nic_cmd_rekey(s, k1);
        return NIC_CMD_OK;
    }
    default:
        return NIC_CMD_TYPE;
    }
}

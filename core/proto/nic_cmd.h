/* The head's check of a command from outside, as ../PROTOCOL.md §6 and ../../mayak/FIRMWARE.md §8
 * fix it. A command lies as `WINDOW` does: `type 1 B · counter 4 B · body · tag 8 B`. Its counter
 * must be above the stored one — checked first, so a replay costs no MAC — and its tag is
 * HMAC-SHA-256 over the station's 3 B and every byte of the message before the tag, under
 * K0‖K1, or under K0 alone for `SET K1`, its first 8 B. The SHA is the head's engine; here it is
 * the archive's reference (../archive/ref/c/session.c). */
#ifndef NIC_CMD_H
#define NIC_CMD_H

#include <stddef.h>
#include <stdint.h>

typedef struct {
    uint8_t k0[8], k1[8];
    uint32_t station;          /* the 3 B identity */
    uint32_t counter;          /* the stored one: the last command accepted */
    uint32_t stale, forged;    /* dropped, for the event log */
} nic_cmd_keys;

#define NIC_CMD_MAX 1024u      /* a command's bytes before its tag */

enum nic_cmd_result { NIC_CMD_OK = 0, NIC_CMD_SIZE, NIC_CMD_STALE, NIC_CMD_FORGED };

/* The message without its COBS framing. On OK the counter is taken: the caller persists it
 * before it acts on the command, so a reboot cannot open a replay. */
enum nic_cmd_result nic_cmd_check(nic_cmd_keys *s, const uint8_t *msg, size_t n, int k0_alone);
/* The server's or the handset's side: the tag written behind the message's first n bytes,
 * n ≤ NIC_CMD_MAX. */
void nic_cmd_sign(const nic_cmd_keys *s, uint8_t *msg, size_t n, int k0_alone);
/* SET K1 accepted: the new working key. */
void nic_cmd_rekey(nic_cmd_keys *s, const uint8_t k1[8]);

#endif

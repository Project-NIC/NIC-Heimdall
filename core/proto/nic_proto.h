/* The protocol's core — the frames, the addresses, the CONTROL plane and the master's tables —
 * as core/PROTOCOL.md §1, §2 and §9 fix them. Portable C99: no allocation, no hardware, nothing
 * but the bytes; every unit, card and the head build on it, and it is unit-tested on a PC
 * (test/test_proto.c). */
#ifndef NIC_PROTO_H
#define NIC_PROTO_H

#include <stddef.h>
#include <stdint.h>

#define NIC_PAYLOAD       32u
#define NIC_MINI_PAYLOAD  8u
#define NIC_SPUR_LEN      40u    /* a node emits: header 6 · payload 32 · CRC 2 */
#define NIC_MINI_LEN      16u    /* header 6 · payload 8 · CRC 2 */
#define NIC_TRUNK_LEN     48u    /* unix.3..1 · the node's 40 with its CRC zeroed into 5 reserve · CRC */
#define NIC_CTRL_LEN      12u
#define NIC_BCAST         0xFFu

enum nic_bus { NIC_NODBUS = 0, NIC_MINI = 1, NIC_MODBUS = 2 };

enum nic_kind {
    NIC_DATA = 0, NIC_TUNNEL, NIC_TUNNEL_REPLY, NIC_FILLER, NIC_TABLE, NIC_REGISTER, NIC_REPORT,
    NIC_PORTS, NIC_HEALTH, NIC_STREAM
};

/* status: the low nibble one state code, the high nibble four flags */
enum nic_state {
    NIC_OK = 0, NIC_REJOINED, NIC_SENSOR, NIC_RC, NIC_WARMUP, NIC_CHANGED, NIC_CLIPPED, NIC_DISTURBED
};
#define NIC_ALARM   0x10u
#define NIC_SUPPLY  0x20u
#define NIC_FAULT   0x40u

enum nic_op {
    NIC_DISCOVER = 1, NIC_ASSIGN_ADDR, NIC_BUSCFG, NIC_SYNC, NIC_TICK, NIC_END, NIC_HARD_RESET,
    NIC_RESEND, NIC_GET, NIC_SET, NIC_ACK, NIC_ERROR, NIC_DIAG_RETIRED, NIC_EVENT, NIC_PORT_PWR,
    NIC_PORT_CLK, NIC_FLOOR, NIC_LOAD, NIC_APPLY
};

enum nic_frame_type { NIC_BAD = 0, NIC_F_SPUR, NIC_F_MINI, NIC_F_TRUNK, NIC_F_CTRL };

/* CRC-16-CCITT, the station's: polynomial 0x1021 from 0xFFFF, unreflected, no final XOR. A frame
 * carries it little-endian, over every byte before it. */
uint16_t nic_crc16(const uint8_t *b, size_t n);

/* A frame is told by its length and nothing else. */
enum nic_frame_type nic_classify(size_t len);

/* ------------------------------------------------------------------ addresses (§2) */

/* NodBus and mini: TYPE«4 | NUMBER, TYPE 1..14, NUMBER 1..15. ModBus: TYPE«2 | NUMBER, TYPE 4..61,
 * NUMBER 0..3. 0 packed, -1 out of the bus's range. */
int nic_addr_pack(enum nic_bus bus, unsigned type, unsigned number, uint8_t *addr);
int nic_addr_unpack(enum nic_bus bus, uint8_t addr, unsigned *type, unsigned *number);

/* ------------------------------------------------------------------ DATA (§1) */

typedef struct {
    uint8_t unix0;     /* the second mod 256 */
    uint8_t frame;     /* the sample in the second */
    uint8_t addr;      /* TYPE|NUM */
    uint8_t slot;
    uint8_t kind;
    uint8_t status;
} nic_header;

/* A node's frame: 40 B with a 32 B payload, 16 B with an 8 B one (mini). */
size_t nic_data_build(const nic_header *h, const uint8_t *payload, size_t plen, uint8_t *out);
/* A spur or mini frame: 0 whole, -1 its length or CRC wrong. *payload points into f. */
int nic_data_parse(const uint8_t *f, size_t len, nic_header *h, const uint8_t **payload,
                   size_t *plen);

/* The card completes a spur frame for the trunk in place: the node's 40 B already lie at buf + 3.
 * Their CRC is checked, unix.3..1 written in front — the second nearest `card_second` whose low
 * byte is the frame's unix.0 — the five reserve bytes over the node's CRC zeroed, the one CRC
 * recomputed over the 46. -1 when the node's CRC fails; nothing is moved. */
int nic_trunk_complete(uint8_t buf[NIC_TRUNK_LEN], uint32_t card_second);
int nic_trunk_parse(const uint8_t f[NIC_TRUNK_LEN], uint32_t *second, nic_header *h,
                    const uint8_t **payload);

/* ------------------------------------------------------------------ CONTROL (§1, §9) */

typedef struct {
    uint8_t unix0, frame, addr, sub, op, arg0, arg1, arg2;
} nic_ctrl;

void nic_ctrl_build(const nic_ctrl *c, uint8_t out[NIC_CTRL_LEN]);
int nic_ctrl_parse(const uint8_t f[NIC_CTRL_LEN], nic_ctrl *c);
static inline uint16_t nic_ctrl_value(const nic_ctrl *c) { return (uint16_t)(c->arg1 | c->arg2 << 8); }

/* An Argus swaps the last hop. Down: TYPE|NUM ← SUB, SUB ← 0 — -1 when SUB is 0, the frame being
 * the Argus's own. Up: SUB ← the sonde's TYPE|NUM, TYPE|NUM ← the Argus's NUMBER. The CRC is
 * recomputed; a unit always sees its own address and SUB 0. */
int nic_argus_down(uint8_t f[NIC_CTRL_LEN]);
void nic_argus_up(uint8_t f[NIC_CTRL_LEN], uint8_t argus_addr);

/* A register too wide for CONTROL rides a kind 5 payload: its number first, the value after it,
 * zeros to 32 B. */
void nic_register_payload(uint16_t reg, const uint8_t *value, size_t len, uint8_t out[NIC_PAYLOAD]);
uint16_t nic_register_of(const uint8_t payload[NIC_PAYLOAD]);

/* ------------------------------------------------------------------ the master's tables (§2) */

/* A table is a 256-entry array indexed by the address itself: one load, no search. */
#define NIC_NONE 0xFFFFu

typedef struct {
    uint8_t bus, addr;       /* the key, with the bus */
    uint16_t parent;         /* the host's row, NIC_NONE for the head's own trunk */
    uint8_t port, slot;      /* where under the parent: the trunk, spur or segment port, the slot */
} nic_unit;

#define NIC_ROSTER_MAX 256u

typedef struct {
    uint16_t by_addr[3][256];      /* bus · address → row */
    nic_unit rows[NIC_ROSTER_MAX];
    uint16_t n;
} nic_roster;

void nic_roster_init(nic_roster *r);
/* The row, added or replaced: the key is (bus, TYPE, NUMBER), which the address is. */
uint16_t nic_roster_put(nic_roster *r, const nic_unit *u);
static inline uint16_t nic_roster_find(const nic_roster *r, enum nic_bus bus, uint8_t addr)
{
    return r->by_addr[bus][addr];
}
/* A row and everything under it out; the table keeps no hole. Returns the rows removed. */
uint16_t nic_roster_drop(nic_roster *r, enum nic_bus bus, uint8_t addr);

#endif

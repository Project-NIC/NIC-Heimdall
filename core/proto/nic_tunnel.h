/* The head's side of the ModBus tunnel, as ../PROTOCOL.md §1 and §9 and ../../mayak/FIRMWARE.md §4
 * and §8 fix it: the head composes the whole RTU package — address, function, data, its CRC — and
 * sends it to a bridge, a Palatine, as a `kind` 1 TUNNEL payload, its length first, zeros to 32 B;
 * the sensor's reply comes back verbatim as `kind` 2 TUNNEL_REPLY the same way. One package open
 * per bridge, the reply paired by that alone; the head validates the RTU CRC and that the reply
 * answers the question — the sensor's address and function, an exception told apart — and retries
 * once on a bad reply, a node's `ERROR` or 200 ms of silence. A package is at most 31 B: a write of
 * 11 registers, a read of 13. Nothing is allocated. */
#ifndef NIC_TUNNEL_H
#define NIC_TUNNEL_H

#include <stddef.h>
#include <stdint.h>

#define NIC_TUN_BRIDGES   16u
#define NIC_TUN_MAX       31u        /* a package's bytes */
#define NIC_TUN_READ_MAX  13u        /* registers one FC03 reply carries */
#define NIC_TUN_WRITE_MAX 11u        /* registers one FC16 carries */
#define NIC_TUN_TIMEOUT   200u       /* ms, the head's for every command */

enum { NIC_MB_READ = 0x03, NIC_MB_WRITE1 = 0x06, NIC_MB_WRITEN = 0x10 };

uint16_t nic_mb_crc(const uint8_t *b, size_t n);          /* CRC-16/MODBUS */

/* The packages, the CRC appended: their length, 0 when it would not fit. */
size_t nic_mb_read(uint8_t slave, uint16_t start, uint8_t count, uint8_t out[NIC_TUN_MAX]);
size_t nic_mb_write1(uint8_t slave, uint16_t reg, uint16_t value, uint8_t out[NIC_TUN_MAX]);
size_t nic_mb_writen(uint8_t slave, uint16_t start, const uint16_t *values, uint8_t count,
                     uint8_t out[NIC_TUN_MAX]);

enum nic_tun_result {
    NIC_TUN_WAIT = 0,                /* nothing yet, or the package sent again */
    NIC_TUN_OK,                      /* the answer, whole */
    NIC_TUN_EXCEPTION,               /* the sensor's exception code in `exception` */
    NIC_TUN_FAILED,                  /* twice without a good answer: the event log, the row's health */
    NIC_TUN_STRAY                    /* a reply nothing was asked for */
};

typedef struct {
    uint8_t bridge, open, tries;
    uint32_t sent;                   /* ms */
    uint8_t len, pkg[NIC_TUN_MAX];
} nic_tun_slot;

typedef struct {
    nic_tun_slot s[NIC_TUN_BRIDGES];
    uint32_t bad, strays, failures;  /* for the event log */
} nic_tunnel;

typedef struct {
    uint8_t slave, function, exception;
    uint8_t len;                     /* the reply's data, after address and function, before CRC */
    uint8_t data[NIC_TUN_MAX];
} nic_tun_answer;

void nic_tun_init(nic_tunnel *t);
/* A package for a bridge (TYPE|NUM of a Palatine): its TUNNEL payload into out. -1 when one is
 * open for that bridge, or the package is no package. */
int nic_tun_send(nic_tunnel *t, uint8_t bridge, const uint8_t *pkg, size_t len, uint32_t now,
                 uint8_t out[32]);
/* A TUNNEL_REPLY payload from a bridge. On WAIT the package has been put again into `again`. */
enum nic_tun_result nic_tun_reply(nic_tunnel *t, uint8_t bridge, const uint8_t payload[32], uint32_t now,
                                  nic_tun_answer *a, uint8_t again[32]);
/* The node's ERROR for the open package. */
enum nic_tun_result nic_tun_error(nic_tunnel *t, uint8_t bridge, uint32_t now, uint8_t again[32]);
/* The clock: a package 200 ms unanswered sent again, then given up. `*bridge` the one it was. */
enum nic_tun_result nic_tun_tick(nic_tunnel *t, uint32_t now, uint8_t *bridge, uint8_t again[32]);

/* An FC03 answer's registers; their count, -1 when it is not one of `count`. */
int nic_mb_registers(const nic_tun_answer *a, uint8_t count, uint16_t *values);
/* An FC06 or FC16 answer that echoes the write: 1 confirmed — what the mirror commits on. */
int nic_mb_confirms(const nic_tun_answer *a, const uint8_t *pkg, size_t len);

#endif

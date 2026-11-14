/* The head's side of the handset's GATT contract (../../mayak/handset/README.md, ../../mayak/FIRMWARE.md
 * §10), above the BLE stack: the status document written as one line of JSON — `go`, `counter`, the
 * nodes and their sensors' health, the corrections table — and cut into notifications of the
 * link's size; the command characteristic's writes gathered to each closing zero, COBS undone, the
 * `COMMAND` checked by nic_cmd.c — its counter, its tag, a read under counter 0 — and its JSON
 * read: `refresh`, `recommission`, `provision`, `corr` with its typed terms, `card`, and the mirror's
 * `set` and `mirror`; the values not yet confirmed in the status document, and one unit's whole
 * mirror as a line of its own. A value is hex, as the unit takes its bytes. Nothing is allocated. */
#ifndef NIC_GATT_H
#define NIC_GATT_H

#include <stddef.h>
#include <stdint.h>

#include "nic_cmd.h"

/* ------------------------------------------------------------------ the status document */

enum { NIC_BUS_NODBUS = 0, NIC_BUS_MINI, NIC_BUS_MODBUS };
enum { NIC_H_OK = 0, NIC_H_SELFTEST_FAIL, NIC_H_NO_RESPONSE, NIC_H_DEGRADED };

typedef struct {
    uint8_t id;
    const char *name;
    uint8_t h;
} nic_st_sensor;

typedef struct {
    uint8_t n, type, bus, ok;
    const char *label;             /* a courtesy, NULL none */
    const nic_st_sensor *sensors;
    uint8_t nsensors;
} nic_st_node;

typedef struct {
    const char *id, *label;
    int32_t m, ns_per_m;           /* m < 0: the term has no metres */
    int32_t ns;
    uint8_t ranged;                /* `src`: ranged, read-only; else typed */
} nic_st_term;

typedef struct {
    uint8_t bus, type, n;          /* the unit as the status names it */
    uint16_t reg;
    uint8_t len, state;            /* state: 2 edited, 3 sent (nic_mirror.h's) */
    const uint8_t *v;
} nic_st_pending;

typedef struct {
    uint16_t reg;
    uint8_t len, ro;
    const uint8_t *v;
} nic_st_reg;

typedef struct {
    uint8_t go;
    uint32_t counter;
    const nic_st_node *nodes;
    uint16_t nnodes;
    const nic_st_term *terms;      /* NULL: no corrections table */
    uint8_t nterms;
    int32_t total_ns;
    const nic_st_pending *pending; /* the mirror's values not yet confirmed */
    uint8_t npending;
} nic_status;

/* The document, closed by its newline: its length, 0 when it does not fit `max`. */
size_t nic_status_write(const nic_status *s, char *out, size_t max);

/* One unit's whole mirror, its own line: its length, 0 when it does not fit. */
size_t nic_mirror_line(uint8_t bus, uint8_t type, uint8_t n, const nic_st_reg *regs, uint16_t nregs,
                       char *out, size_t max);

/* A document cut into notifications of at most `mtu` bytes. */
typedef struct {
    const char *doc;
    size_t len, at;
} nic_notify;

void nic_notify_start(nic_notify *n, const char *doc, size_t len);
/* The next notification's bytes into out: their count, 0 when the document has gone. */
size_t nic_notify_next(nic_notify *n, size_t mtu, uint8_t *out);

/* ------------------------------------------------------------------ the command characteristic */

#define NIC_GATT_TERMS 4u
#define NIC_GATT_VALUE 157u        /* a mirror value's bytes */

enum nic_gatt_kind { NIC_GC_NONE = 0, NIC_GC_REFRESH, NIC_GC_RECOMMISSION, NIC_GC_PROVISION, NIC_GC_CORR,
                     NIC_GC_CARD, NIC_GC_SET, NIC_GC_MIRROR };

typedef struct {
    enum nic_gatt_kind kind;
    int node;                      /* provision */
    int slot;                      /* card */
    uint8_t nterms;                /* corr */
    struct { char id[8]; int32_t m; } terms[NIC_GATT_TERMS];
    uint8_t bus, type, n;          /* set, mirror: the unit */
    uint16_t reg;                  /* set */
    uint8_t vlen, v[NIC_GATT_VALUE];
} nic_gatt_cmd;

enum nic_gatt_result {
    NIC_GW_MORE = 0,               /* no closing zero yet */
    NIC_GW_TAKEN,                  /* a command, checked and read */
    NIC_GW_REFUSED,                /* its check failed: nic_cmd.c's result in `why` */
    NIC_GW_BAD                     /* not a COMMAND, its framing broken or its JSON not one we know */
};

typedef struct {
    uint8_t buf[2 + 1024 + 16 + 16];   /* a framed COMMAND: COBS's overhead on 1 KB of JSON */
    size_t n;
    uint8_t overflow;
} nic_gatt_rx;

void nic_gatt_rx_init(nic_gatt_rx *r);
/* A write's bytes. On TAKEN the command is in *c; *why carries nic_cmd.c's verdict either way.
 * Bytes after the closing zero stay for the next call with n = 0. */
enum nic_gatt_result nic_gatt_write(nic_gatt_rx *r, nic_cmd_keys *k, const uint8_t *b, size_t n,
                                    size_t *used, nic_gatt_cmd *c, enum nic_cmd_result *why);
/* A command's JSON read on its own: 0 when it is one we know. */
int nic_gatt_parse(const char *json, size_t n, nic_gatt_cmd *c);

#endif

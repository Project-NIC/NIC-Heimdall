/* The head's live packet, as ../PROTOCOL.md §6, ../archive/HMC.md (*The uplink*) and
 * ../../mayak/FIRMWARE.md §9 fix it: what each address's send rule for a link selects from what was
 * recorded since the link's last live packet, one segment per address coded as in a file, in a
 * `LIVE` message — `second 4 B · count 1 B`, then per address `address 1 B · length 4 B · segment`.
 * One nic_live per link, each with its own rules and cadence.
 *
 * The records come as the archive writes them — the segment the spool hands the writer — so the
 * live packet is at most a segment's 8 s behind. A repaired record, older than the newest the
 * address has given, never goes live. An address holds NIC_LIVE_BYTES of records between packets;
 * past them the oldest give way to the newest, counted — the live view wants the newest. A packet
 * longer than the link takes carries the addresses that fit, in order, and the rest wait for the
 * next. The segment is built by the archive's library, the same code the writer uses. */
#ifndef NIC_LIVE_H
#define NIC_LIVE_H

#include <stddef.h>
#include <stdint.h>

#include "hcc.h"
#include "hmc.h"

#define NIC_LIVE_ADDRS 40u         /* 32 units, a Palatine's four tables, spare */
#define NIC_LIVE_BYTES 8192u       /* an address's records between packets, 9 B each beside its data */

typedef struct {
    uint8_t addr;
    hmc_selector sel;
    hcc_field layout[HCC_MAX_LAYOUT];
    size_t nl;
    uint8_t bytes[NIC_LIVE_BYTES];
    hcc_record recs[NIC_LIVE_BYTES / 9];
    size_t n, used;                /* records held, and their bytes as counted against the bound */
    uint32_t newest;
    uint8_t have_newest;
} nic_live_addr;

typedef struct {
    nic_live_addr a[NIC_LIVE_ADDRS];
    uint8_t n;
    uint32_t given_way, repairs;   /* for the event log */
} nic_live;

void nic_live_init(nic_live *l);
void nic_live_free(nic_live *l);
/* An address and its send rule on this link; its layout, from the sections in force, as the
 * writer codes its segments. Called again when the rule or the layout changes. */
int nic_live_add(nic_live *l, uint8_t addr, const hmc_rule *send, int blocks, const hcc_field *layout,
                 size_t nl);
/* A segment's records as the archive writes them. */
void nic_live_records(nic_live *l, uint8_t addr, const hcc_record *recs, size_t n);
/* The LIVE message at `second`, `type · body` unframed, into out of `max` bytes: its length, 0
 * when no address has anything or none fits. What went is forgotten; what did not fit waits. */
size_t nic_live_build(nic_live *l, uint32_t second, uint8_t *out, size_t max);

#endif

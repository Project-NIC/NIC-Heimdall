/* The head's ingest above its pins, as mayak/FIRMWARE.md §6 describes it: every good trunk frame
 * into its second's bucket, two buckets open; a second closed one second after its end, a slot no
 * frame arrived for made a FILLER, *no data arrived*; each address's frames through its recording
 * rule into its buffer; a buffer a segment at 16 kB or 8 s after its first record, handed to the
 * writer the firmware gives (the HMC writer, on both cards). A frame repaired after its second
 * closed goes into its unit's buffer as a record with an older second, kept. */
#ifndef NIC_INGEST_H
#define NIC_INGEST_H

#include <stddef.h>
#include <stdint.h>

#include "nic_proto.h"
#include "hcc.h"
#include "hmc.h"

#define NIC_INGEST_UNITS   32u      /* four trunks of eight */
#define NIC_INGEST_ROWS    256u
#define NIC_SPOOL_BYTES    16384u
#define NIC_SPOOL_SECONDS  8u

typedef struct {
    uint8_t addr, rows, blocks;     /* blocks: a ModBus payload, kept per block */
    hmc_rule rule;
} nic_ingest_unit;

typedef struct {
    uint32_t second;
    uint8_t open;
    uint8_t have[NIC_INGEST_UNITS][NIC_INGEST_ROWS / 8];
    uint8_t head[NIC_INGEST_UNITS][NIC_INGEST_ROWS][2];      /* kind, status */
    uint8_t data[NIC_INGEST_UNITS][NIC_INGEST_ROWS][NIC_PAYLOAD];
} nic_bucket;

typedef struct {
    uint8_t bytes[NIC_SPOOL_BYTES + 512];
    hcc_record recs[NIC_SPOOL_BYTES / 9 + 16];
    size_t n, used;
    uint32_t first_second;
} nic_spool;

typedef int (*nic_segment_fn)(void *ctx, uint8_t addr, const hcc_record *recs, size_t n);

typedef struct {
    uint8_t idx[256];                /* address → unit, 0xFF none */
    nic_ingest_unit units[NIC_INGEST_UNITS];
    uint8_t nunits;
    nic_bucket b[2];
    hmc_selector sel[NIC_INGEST_UNITS];
    nic_spool spool[NIC_INGEST_UNITS];
    nic_segment_fn segment;
    void *ctx;
    uint32_t dropped, repaired, holes, segments;
} nic_ingest;

void nic_ingest_init(nic_ingest *g, nic_segment_fn segment, void *ctx);
int nic_ingest_add(nic_ingest *g, const nic_ingest_unit *u);

enum { NIC_IN_STORED = 0, NIC_IN_REPAIRED, NIC_IN_DROPPED };
/* A trunk frame as it arrived: its CRC, its address, its second. */
int nic_ingest_frame(nic_ingest *g, const uint8_t trunk[NIC_TRUNK_LEN]);
/* At one second past the end of `second`: its holes filled, its frames selected and spooled, the
 * buffers whose 8 s are up written. */
void nic_ingest_close(nic_ingest *g, uint32_t second);
/* The 12 V lost: every buffer written now. */
void nic_ingest_flush(nic_ingest *g);

#endif

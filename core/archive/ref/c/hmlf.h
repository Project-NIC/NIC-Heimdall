/* MLF — Heimdall's profile, the C reference.
 *
 * One address's file as core/archive/MLF.md fixes it and as ../hmlf.py writes it, on MLF's core
 * (NIC-SDK, mlf/): the source — the station in bytes 16–18, the address in 19 —, Heimdall's
 * sections, the records and the event log's lines, raw and NRC, the path, the rules. C99.
 */
#ifndef HMLF_H
#define HMLF_H

#include <stddef.h>
#include <stdint.h>

#include "mlf.h"
#include "hnrc.h"

#define HMLF_MAX_BLOCK   253u     /* a record's length is one byte: module · length · data */

enum {
    HMLF_STATION = 1, HMLF_UNIT, HMLF_LAYOUT, HMLF_PROFILE, HMLF_SETTINGS, HMLF_RULES,
    HMLF_RESPONSE, HMLF_TIME = 9  /* 8 not used: a codec's table is the core's MLF_CODEC */
};

/* The layout NRC codes a segment with, from the sections in force; `out` holds HNRC_MAX_LAYOUT. */
int hmlf_nrc_layout(const mlf_sections *s, hnrc_field *out, size_t *n);

typedef struct {
    uint8_t address;
    uint16_t sequence;
    uint32_t first_second;
    uint32_t station;    /* 24 bits */
    uint8_t size;        /* 1 MB × 2^size */
    uint32_t length;     /* parse: the whole header */
    const uint8_t *sections;
    size_t sections_len;
} hmlf_header;

void hmlf_to_core(const hmlf_header *h, mlf_header *c);
void hmlf_from_core(const mlf_header *c, hmlf_header *h);
size_t hmlf_header_build(const hmlf_header *h, uint8_t *out);  /* out holds 24 + sections_len */
int hmlf_header_parse(const uint8_t *b, size_t len, hmlf_header *h);

typedef struct {
    uint32_t second;
    uint8_t source, code;
    uint16_t detail;
    uint32_t value;
} hmlf_event;

/* Unframed objects, malloc'd: a segment coded raw and by NRC, the shorter, raw on a tie. */
int hmlf_segment_build(const hnrc_record *recs, size_t n, const hnrc_field *layout, size_t nl,
                       uint8_t **out, size_t *len);
int hmlf_event_segment_build(const hmlf_event *lines, size_t n, uint8_t **out, size_t *len);

/* A segment's records: raw ones point into its body, NRC's are one hnrc_decode block; free
 * *recs with hnrc_free either way. */
int hmlf_segment_records(const mlf_object *o, const hnrc_field *layout, size_t nl,
                        hnrc_record **recs, size_t *n);
int hmlf_events_parse(const mlf_object *o, hmlf_event **lines, size_t *n);

/* A file in memory: MLF's writer opened with Heimdall's header. */
int hmlf_writer_open(mlf_writer *w, const hmlf_header *h);
int hmlf_writer_segment(mlf_writer *w, const hnrc_record *recs, size_t n);
int hmlf_writer_events(mlf_writer *w, const hmlf_event *lines, size_t n);

/* ------------------------------------------------------------------ the rules */

enum { HMLF_ALL = 0, HMLF_CHANGE, HMLF_DECIMATE, HMLF_INTERVAL, HMLF_NONZERO, HMLF_NONE };
#define HMLF_MAX_BLOCK_KEYS 256u

typedef struct {
    uint8_t mode, n;
    uint16_t s;            /* INTERVAL's seconds · CHANGE's and NONZERO's at least every, 0 none */
    uint8_t nmodules;      /* a send rule's ModBus modules, 0 all */
    uint8_t modules[255];
} hmlf_rule;

typedef struct {
    uint8_t bytes[2 + HMLF_MAX_BLOCK];   /* the block as it rode */
    uint32_t second;
} hmlf_block_key;

/* One address's rule, as the head walks the second's frames or a link's send rule walks what
 * was recorded. `blocks` for a ModBus payload, kept per block. */
typedef struct {
    hmlf_rule rule;
    int blocks;
    int have_state, have_frame, have_frame_iv, have_nzero;
    uint8_t kind, status, frame[HNRC_PAYLOAD];
    uint32_t frame_second, frame_iv, nzero;
    hmlf_block_key *keys;
    size_t nkeys;
    uint8_t have_mod_iv[256];
    uint32_t mod_iv[256];
} hmlf_selector;

void hmlf_selector_init(hmlf_selector *s, const hmlf_rule *r, int blocks);
void hmlf_selector_free(hmlf_selector *s);
/* A frame's kept records into out (16 at most); their data point into `payload`. */
size_t hmlf_select_frame(hmlf_selector *s, uint32_t second, uint8_t frame, uint8_t kind,
                        uint8_t status, const uint8_t *payload, hnrc_record out[16]);
/* A recorded record under a send rule: 1 kept, 0 not. */
int hmlf_select_record(hmlf_selector *s, const hnrc_record *r);
/* A ModBus payload's whole blocks up to a zero address: their count, -1 when one runs past. */
int hmlf_split_blocks(const uint8_t *payload, uint8_t offsets[16]);

/* `52/20261001/120000.mlf`; out holds 32 bytes. */
void hmlf_path(uint8_t address, uint32_t first_second, char out[32]);

#endif

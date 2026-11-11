/* HMC — the Heimdall Matryoshka Container, the C reference.
 *
 * One address's file as core/archive/HMC.md fixes it and as ../hmc.py writes it: the two write
 * the same bytes, and ../tests/test_c.py compares them. The writer holds the file in memory and
 * appends as the head does — an object's framed bytes, then its entry; a head on a card does the
 * same with the same builders. C99, on hcc.c.
 */
#ifndef HMC_H
#define HMC_H

#include <stddef.h>
#include <stdint.h>

#include "hcc.h"

#define HMC_MB          (1u << 20)
#define HMC_VERSION     2u
#define HMC_MAX_BLOCK   253u     /* a record's length is one byte: module · length · data */
#define HMC_MAX_SECTIONS 64u

enum { HMC_SEG = 1, HMC_CFG = 2 };
enum { HMC_RAW = 0, HMC_HCC = 1 };
enum {
    HMC_STATION = 1, HMC_UNIT, HMC_LAYOUT, HMC_PROFILE, HMC_SETTINGS, HMC_RULES, HMC_RESPONSE,
    HMC_CODEC, HMC_TIME
};
enum {
    HMC_OK = 0,
    HMC_DAMAGED = 1,     /* not whole: a CRC, a length, a sync, the framing */
    HMC_EINVAL = 2,      /* what the format does not hold */
    HMC_ENOMEM = 3,
    HMC_FULL = 4,        /* the object and its entry would cross */
    HMC_UNKNOWN = 5      /* a codec this reader does not know */
};

uint32_t hmc_crc32(const uint8_t *b, size_t n);     /* IEEE 802.3, zlib's */
uint16_t hmc_crc16(const uint8_t *b, size_t n);     /* CCITT-FALSE: 0x1021, from 0xFFFF */

/* COBS: `out` holds n + n / 254 + 1 bytes; decode's `out` holds n. */
size_t hmc_cobs_encode(const uint8_t *in, size_t n, uint8_t *out);
int hmc_cobs_decode(const uint8_t *in, size_t n, uint8_t *out, size_t *outlen);

typedef struct {
    uint8_t id;
    uint32_t length;
    const uint8_t *body;
} hmc_section;

/* The sections in force: the header's, each config's ids replacing all of theirs. The bodies
 * are copied. */
typedef struct {
    size_t n;
    hmc_section s[HMC_MAX_SECTIONS];
} hmc_sections;

int hmc_sections_parse(const uint8_t *b, size_t len, hmc_sections *out);   /* bodies point in b */
int hmc_sections_apply(hmc_sections *in_force, const hmc_sections *config);
void hmc_sections_free(hmc_sections *s);                                   /* owned copies only */

/* The layout HCC codes a segment with, from the sections in force; `out` holds HCC_MAX_LAYOUT. */
int hmc_hcc_layout(const hmc_sections *s, hcc_field *out, size_t *n);

typedef struct {
    uint8_t address;
    uint16_t sequence;
    uint32_t first_second;
    uint32_t station;    /* 24 bits */
    uint8_t size;        /* 1 MB × 2^size */
    uint32_t length;     /* parse: the whole header */
    const uint8_t *sections;
    size_t sections_len;
} hmc_header;

size_t hmc_header_build(const hmc_header *h, uint8_t *out);  /* out holds 24 + sections_len */
int hmc_header_parse(const uint8_t *b, size_t len, hmc_header *h);

typedef struct {
    uint32_t second;
    uint8_t source, code;
    uint16_t detail;
    uint32_t value;
} hmc_event;

/* Unframed objects, malloc'd: a segment coded raw and by HCC, the shorter, raw on a tie. */
int hmc_segment_build(const hcc_record *recs, size_t n, const hcc_field *layout, size_t nl,
                      uint8_t **out, size_t *len);
int hmc_event_segment_build(const hmc_event *lines, size_t n, uint8_t **out, size_t *len);
int hmc_config_build(uint32_t from_second, const uint8_t *sections, size_t slen, uint8_t **out,
                     size_t *len);

typedef struct {
    uint32_t second, offset, length;
    uint8_t type;
} hmc_entry;

void hmc_entry_build(const hmc_entry *e, uint8_t out[16]);
int hmc_entry_parse(const uint8_t b[16], hmc_entry *e);    /* HMC_OK or HMC_DAMAGED (free too) */

typedef struct {
    int type;
    uint32_t first, last, from;   /* a segment's seconds · a config's */
    uint8_t codec;
    const uint8_t *body;          /* a segment's body · a config's sections */
    size_t body_len;
} hmc_object;

int hmc_object_parse(const uint8_t *b, size_t len, hmc_object *o);

/* A segment's records: raw ones point into its body, HCC's are one hcc_decode block; free
 * *recs with hcc_free either way. */
int hmc_segment_records(const hmc_object *o, const hcc_field *layout, size_t nl,
                        hcc_record **recs, size_t *n);
int hmc_events_parse(const hmc_object *o, hmc_event **lines, size_t *n);

/* ------------------------------------------------------------------ a file in memory */

typedef struct {
    uint8_t *image;
    size_t size, end, entries;
    uint32_t last;
    hmc_sections in_force;
    uint8_t address;
} hmc_writer;

int hmc_writer_open(hmc_writer *w, const hmc_header *h);
int hmc_writer_append(hmc_writer *w, const uint8_t *obj, size_t len, uint8_t type, uint32_t second);
int hmc_writer_segment(hmc_writer *w, const hcc_record *recs, size_t n);
int hmc_writer_events(hmc_writer *w, const hmc_event *lines, size_t n);
int hmc_writer_config(hmc_writer *w, uint32_t from_second, const uint8_t *sections, size_t slen);
void hmc_writer_close(hmc_writer *w);

typedef struct {
    const uint8_t *image;
    size_t size, entries;
    hmc_header h;
} hmc_reader;

int hmc_reader_open(hmc_reader *r, const uint8_t *image, size_t len);
int hmc_reader_entry(const hmc_reader *r, size_t i, hmc_entry *e);
size_t hmc_reader_find(const hmc_reader *r, uint32_t second);
/* The object entry i points at; *buf the decoded bytes it lies in, malloc'd. */
int hmc_reader_object(const hmc_reader *r, size_t i, uint8_t **buf, hmc_object *o);

/* ------------------------------------------------------------------ the rules */

enum { HMC_ALL = 0, HMC_CHANGE, HMC_DECIMATE, HMC_INTERVAL, HMC_NONZERO, HMC_NONE };
#define HMC_MAX_BLOCK_KEYS 256u

typedef struct {
    uint8_t mode, n;
    uint16_t s;            /* INTERVAL's seconds · CHANGE's and NONZERO's at least every, 0 none */
    uint8_t nmodules;      /* a send rule's ModBus modules, 0 all */
    uint8_t modules[255];
} hmc_rule;

typedef struct {
    uint8_t bytes[2 + HMC_MAX_BLOCK];   /* the block as it rode */
    uint32_t second;
} hmc_block_key;

/* One address's rule, as the head walks the second's frames or a link's send rule walks what
 * was recorded. `blocks` for a ModBus payload, kept per block. */
typedef struct {
    hmc_rule rule;
    int blocks;
    int have_state, have_frame, have_frame_iv, have_nzero;
    uint8_t kind, status, frame[HCC_PAYLOAD];
    uint32_t frame_second, frame_iv, nzero;
    hmc_block_key *keys;
    size_t nkeys;
    uint8_t have_mod_iv[256];
    uint32_t mod_iv[256];
} hmc_selector;

void hmc_selector_init(hmc_selector *s, const hmc_rule *r, int blocks);
void hmc_selector_free(hmc_selector *s);
/* A frame's kept records into out (16 at most); their data point into `payload`. */
size_t hmc_select_frame(hmc_selector *s, uint32_t second, uint8_t frame, uint8_t kind,
                        uint8_t status, const uint8_t *payload, hcc_record out[16]);
/* A recorded record under a send rule: 1 kept, 0 not. */
int hmc_select_record(hmc_selector *s, const hcc_record *r);
/* A ModBus payload's whole blocks up to a zero address: their count, -1 when one runs past. */
int hmc_split_blocks(const uint8_t *payload, uint8_t offsets[16]);

/* `52/20261001/120000.hmc`; out holds 32 bytes. */
void hmc_path(uint8_t address, uint32_t first_second, char out[32]);

#endif

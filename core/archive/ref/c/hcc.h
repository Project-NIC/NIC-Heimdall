/* HCC — the Heimdall Compression Codec, the C reference.
 *
 * Codes one address's kept records into one bit stream and back, losslessly, exactly as
 * core/archive/HCC.md fixes it and as ../hcc.py writes it: the two write the same bytes, and
 * ../tests/test_c.py compares them. C99, no state, no allocation but malloc and free.
 */
#ifndef HCC_H
#define HCC_H

#include <stddef.h>
#include <stdint.h>

#define HCC_PAYLOAD      32u      /* a frame's payload */
#define HCC_MAX_RECORDS  65536u   /* HCC.md, *Decoding*: a count above it is damaged */
#define HCC_MAX_GROUPS   256u     /* a block's group index is one byte */
#define HCC_MAX_LAYOUT   (2u * HCC_PAYLOAD)

enum {
    HCC_OK      = 0,
    HCC_DAMAGED = 1,   /* decode: the stream says what no coder writes, or ends wrong */
    HCC_EINVAL  = 2,   /* the records or the layout are not what HCC codes */
    HCC_ENOMEM  = 3
};

typedef struct {
    uint8_t offset;        /* byte offset in the 32 B payload */
    uint8_t width;         /* 1..4 bytes */
    uint8_t is_signed;
    uint8_t big_endian;
    uint8_t channel;
    uint8_t ref;           /* the reference channel + 1, 0 none */
} hcc_field;

typedef struct {
    uint32_t second;
    uint8_t  frame, kind, status;
    uint8_t  form;         /* 0 a frame: 32 B · 1 a block: module · length · bytes */
    uint16_t length;       /* of data */
    const uint8_t *data;
} hcc_record;

/* The declared fields plus a one-byte channel for every byte none covers, after the declared
 * channels, in offset order. `out` holds n + 32 fields; returns the count. */
size_t hcc_compose_layout(const hcc_field *fields, size_t n, hcc_field *out);

/* An Argus NUMBER's layout from the mini layout at each 8 B position; a position with count 0
 * is empty. `out` holds the sum of the counts + 32 fields; returns the count. */
size_t hcc_argus_layout(const hcc_field *const *positions, const size_t *counts, size_t npos,
                        hcc_field *out);

/* The records of one address, in the order kept, to one stream; *out is malloc'd. */
int hcc_encode(const hcc_record *recs, size_t n, const hcc_field *layout, size_t nlayout,
               uint8_t **out, size_t *outlen);

/* One stream back to its records; *recs is one malloc'd block, the payloads inside it. */
int hcc_decode(const uint8_t *stream, size_t len, const hcc_field *layout, size_t nlayout,
               hcc_record **recs, size_t *n);

void hcc_free(void *p);

/* Codec 0's length of the same records: 9 B of record head and the bytes. */
size_t hcc_raw_length(const hcc_record *recs, size_t n);

#endif

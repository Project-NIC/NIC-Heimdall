/* NRC — Heimdall's profile, the C reference: its records as series on NRC's core (NIC-SDK, nrc/).
 *
 * Codes one address's kept records into one bit stream and back, losslessly, exactly as
 * core/archive/NRC.md fixes it and as ../hnrc.py writes it: the two write the same bytes, and
 * ../tests/test_c.py compares them. C99, no state, no allocation but malloc and free.
 */
#ifndef HNRC_H
#define HNRC_H

#include <stddef.h>
#include <stdint.h>

#define HNRC_PAYLOAD      32u      /* a frame's payload */
#define HNRC_MAX_RECORDS  65536u   /* NRC.md, *Decoding*: a count above it is damaged */
#define HNRC_MAX_GROUPS   256u     /* a block's group index is one byte */
#define HNRC_MAX_LAYOUT   (2u * HNRC_PAYLOAD)

enum {
    HNRC_OK      = 0,
    HNRC_DAMAGED = 1,   /* decode: the stream says what no coder writes, or ends wrong */
    HNRC_EINVAL  = 2,   /* the records or the layout are not what NRC codes */
    HNRC_ENOMEM  = 3
};

typedef struct {
    uint8_t offset;        /* byte offset in the 32 B payload */
    uint8_t width;         /* 1..4 bytes */
    uint8_t is_signed;
    uint8_t big_endian;
    uint8_t channel;
    uint8_t ref;           /* the reference channel + 1, 0 none */
} hnrc_field;

typedef struct {
    uint32_t second;
    uint8_t  frame, kind, status;
    uint8_t  form;         /* 0 a frame: 32 B · 1 a block: module · length · bytes */
    uint16_t length;       /* of data */
    const uint8_t *data;
} hnrc_record;

/* The declared fields plus a one-byte channel for every byte none covers, after the declared
 * channels, in offset order. `out` holds n + 32 fields; returns the count. */
size_t hnrc_compose_layout(const hnrc_field *fields, size_t n, hnrc_field *out);

/* An Argus NUMBER's layout from the mini layout at each 8 B position; a position with count 0
 * is empty. `out` holds the sum of the counts + 32 fields; returns the count. */
size_t hnrc_argus_layout(const hnrc_field *const *positions, const size_t *counts, size_t npos,
                        hnrc_field *out);

/* The records of one address, in the order kept, to one stream; *out is malloc'd. */
int hnrc_encode(const hnrc_record *recs, size_t n, const hnrc_field *layout, size_t nlayout,
               uint8_t **out, size_t *outlen);

/* One stream back to its records; *recs is one malloc'd block, the payloads inside it. */
int hnrc_decode(const uint8_t *stream, size_t len, const hnrc_field *layout, size_t nlayout,
               hnrc_record **recs, size_t *n);

void hnrc_free(void *p);

/* Codec 0's length of the same records: 9 B of record head and the bytes. */
size_t hnrc_raw_length(const hnrc_record *recs, size_t n);

#endif

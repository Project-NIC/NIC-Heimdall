/* The uplink session's messages, the C reference, as core/PROTOCOL.md §6 fixes them and as
 * ../session.py writes them; ../tests/test_c.py compares the two. SHA-256 and HMAC are here so
 * the library stands alone; the head computes them on its SHA engine. */
#ifndef SESSION_H
#define SESSION_H

#include <stddef.h>
#include <stdint.h>

#include "hmc.h"

#define SES_TAG 8u
#define SES_NONE_ENTRY 0xFFFFFFFFu

enum { SES_HELLO = 1, SES_CURSOR, SES_ARCHIVE, SES_ACK, SES_LIVE, SES_WINDOW, SES_BOARD };

void ses_sha256(const uint8_t *d, size_t n, uint8_t out[32]);
void ses_hmac_sha256(const uint8_t *key, size_t klen, const uint8_t *d, size_t n, uint8_t out[32]);
/* A tag: HMAC-SHA-256 under the 16 B K0‖K1 over the parts in order, its first 8 B. */
void ses_tag(const uint8_t key[16], const uint8_t *const *parts, const size_t *lens, size_t nparts,
             uint8_t out[8]);

/* A message is type · body, COBS-framed and followed by a zero; *msg malloc'd. unframe takes
 * the bytes without the zero and gives the raw message, malloc'd. */
int ses_frame(const uint8_t *raw, size_t n, uint8_t **msg, size_t *len);
int ses_unframe(const uint8_t *framed, size_t n, uint8_t **raw, size_t *rawlen);

typedef struct {
    uint32_t station, counter;
    uint8_t nonce[8], board, tag[8];
} ses_hello_t;

int ses_hello(const uint8_t key[16], uint32_t station, uint32_t counter, const uint8_t nonce[8],
              int board, uint8_t tag[8], uint8_t **msg, size_t *len);
int ses_hello_parse(const uint8_t key[16], const uint8_t *body, size_t n, ses_hello_t *h);

typedef struct {
    uint8_t address;
    uint16_t sequence;
    uint32_t entry;
} ses_mark;

/* CURSOR or ACK. parse: *marks malloc'd. */
int ses_marks(int type, const ses_mark *m, size_t n, uint8_t **msg, size_t *len);
int ses_marks_parse(const uint8_t *body, size_t n, ses_mark **marks, size_t *count);

typedef struct {
    uint32_t index;
    const uint8_t *entry;      /* 16 B */
    const uint8_t *cobs;
    size_t cobs_len;
} ses_object;

typedef struct {
    uint8_t key[16], nonce[8], last[8];
    uint32_t packet;
} ses_chain;

typedef struct {
    uint8_t address;
    uint16_t sequence;
    uint32_t packet;
    const uint8_t *header;
    size_t header_len, count;
    ses_object *objects;       /* malloc'd; the bytes point into the body */
} ses_archive_t;

void ses_chain_init(ses_chain *c, const uint8_t key[16], const uint8_t nonce[8],
                    const uint8_t hello_tag[8]);
int ses_archive(ses_chain *c, uint8_t address, uint16_t sequence, const uint8_t *header,
                size_t hlen, const ses_object *objs, size_t n, uint8_t **msg, size_t *len);
/* The server's side: the packet's tag checked against the chain, then the chain moved on. */
int ses_archive_verify(ses_chain *c, const uint8_t *body, size_t n, ses_archive_t *a);

typedef struct {
    uint8_t address;
    const uint8_t *segment;
    size_t len;
} ses_seg;

int ses_live_body(uint32_t second, const ses_seg *segs, size_t n, uint8_t **body, size_t *len);
int ses_live(uint32_t second, const ses_seg *segs, size_t n, uint8_t **msg, size_t *len);
int ses_live_parse(const uint8_t *body, size_t n, uint32_t *second, ses_seg **segs, size_t *count);

typedef struct {
    uint32_t counter, second, window;
    uint8_t address;
} ses_window_t;

int ses_window(const uint8_t key[16], uint32_t station, uint32_t counter, uint8_t address,
               uint32_t second, uint32_t span, uint8_t **msg, size_t *len);
int ses_window_parse(const uint8_t key[16], uint32_t station, uint32_t stored, const uint8_t *body,
                     size_t n, ses_window_t *w);
int ses_board(uint32_t station, const uint8_t *live_body, size_t n, uint8_t **msg, size_t *len);

/* The head's cuts of a file. forward: the good entries after `held`, packed into packets of up
 * to `budget` bytes of objects — packet[k] the packet entry idx[k] rides in. window: the segments
 * whose first to last second touches [second − span, second + span] and the configs before the
 * last of them. idx and packet hold r->entries. */
size_t ses_forward(const hmc_reader *r, uint32_t held, size_t budget, uint32_t *idx,
                   uint32_t *packet);
size_t ses_window_cut(const hmc_reader *r, uint32_t second, uint32_t span, uint32_t *idx);
/* The object an entry points at, as an ARCHIVE packet carries it. */
int ses_object_of(const hmc_reader *r, uint32_t index, ses_object *o);

#endif

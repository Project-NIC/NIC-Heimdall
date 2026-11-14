/* The head's side of the uplink session, as ../PROTOCOL.md §6 and ../../mayak/FIRMWARE.md §9 fix
 * it: `HELLO` with the command counter and a fresh nonce, the chain's start; the server's `CURSOR`
 * taken per address — the file it names sent forward from the entry after the one held, then every
 * later file of that address in order, each opening with its header; an address the cursor does
 * not name from its oldest file on the cards; a sequence the cards cannot place rewound to what
 * they hold, a rewind being free. One `ARCHIVE` packet at a time: the event log, address 0, first
 * whenever it has anything, then the other addresses in turn, a packet each. Caught up, the live
 * file's new segments go as they are written. `ACK` is kept to say how far behind the server is;
 * nothing is forgotten by it. Sequences compare in a circle of 2^16.
 *
 * A `WINDOW` the head has accepted (nic_cmd.c) is answered before anything else, one address or
 * every one: each file it touches as ARCHIVE packets of its header, its configs before the last
 * segment sent, and the segments whose first to last second touches [second − span, second +
 * span]. An entry's second is its segment's last and they never go back, so a file is searched
 * from the first entry at or after the window's start; the search stops a minute past its end
 * (NIC_UP_LATE), beyond which no segment can hold a record of the window — a segment spans 8 s and
 * a repaired record arrives within seconds of its own.
 *
 * The cards are read through the store the firmware gives — both cards behind it, the primary
 * read — and nothing is allocated. The messages come out unframed, `type · body`; COBS and the
 * closing zero are the transport's. The SHA is the head's engine; here it is the archive's
 * reference (../archive/ref/c/session.c). */
#ifndef NIC_UPLINK_H
#define NIC_UPLINK_H

#include <stddef.h>
#include <stdint.h>

#include "session.h"

#define NIC_UP_ADDRS      64u        /* the addresses with files on the cards, the event log one */
#define NIC_UP_BUDGET     16384u     /* a packet's objects, each 20 B of index and entry beside it */
#define NIC_UP_OBJECT_MAX 17408u     /* one object's COBS bytes: a 16 kB segment and its overhead */
#define NIC_UP_HEADER_MAX 4096u      /* a file's header with its sections */
#define NIC_UP_WINDOWS    8u         /* windows waiting their turn */
#define NIC_UP_LATE       60u        /* s past a window's end that a file is still searched */
#define NIC_UP_PACKET     (14u + NIC_UP_HEADER_MAX + 2u + 20u + NIC_UP_OBJECT_MAX + 8u)

/* The cards, read. header: the file's header into buf, its length, 0 when there is none or it is
 * longer than max. entries: the description entries written to the file so far. entry: entry i's
 * 16 B, 0 when it is read. read: len bytes at offset, 0 when read. */
typedef struct {
    size_t (*header)(void *ctx, uint8_t addr, uint16_t seq, uint8_t *buf, size_t max);
    uint32_t (*entries)(void *ctx, uint8_t addr, uint16_t seq);
    int (*entry)(void *ctx, uint8_t addr, uint16_t seq, uint32_t i, uint8_t out[16]);
    int (*read)(void *ctx, uint8_t addr, uint16_t seq, uint32_t offset, uint32_t len, uint8_t *buf);
    void *ctx;
} nic_store;

typedef struct {
    uint8_t addr;
    uint16_t first, live;          /* the oldest file the cards hold, and the one being written */
    uint16_t seq;                  /* the file being sent */
    uint32_t next;                 /* its next entry */
    uint8_t header_due;
    uint8_t acked;                 /* an ACK has named this address */
    uint16_t ack_seq;
    uint32_t ack_entry;
} nic_up_addr;

enum { NIC_UP_IDLE = 0, NIC_UP_HELLO, NIC_UP_SENDING };

typedef struct {
    uint8_t addr;                  /* 0xFF every address */
    uint32_t lo, hi;
} nic_up_window;

typedef struct {
    nic_up_window w;
    uint8_t on, k, ready, header_due;
    uint16_t seq;
    uint32_t i, last;              /* the next entry, and the last segment that touches */
} nic_up_answer;

typedef struct {
    nic_store store;
    nic_up_addr a[NIC_UP_ADDRS];
    uint8_t n, turn, state;
    uint8_t key[16];
    ses_chain chain;
    uint32_t skipped;              /* damaged entries passed over, for the event log */
    nic_up_window queue[NIC_UP_WINDOWS];
    uint8_t queued;
    nic_up_answer ans;
    uint8_t pkt[NIC_UP_PACKET];
} nic_uplink;

void nic_up_init(nic_uplink *u, const nic_store *store, const uint8_t key[16]);
/* An address with files on the cards, first to live; called again when a new file opens. */
int nic_up_files(nic_uplink *u, uint8_t addr, uint16_t first, uint16_t live);

/* A session opens: HELLO into out (26 B), the chain started on its tag. */
size_t nic_up_hello(nic_uplink *u, uint32_t station, uint32_t counter, const uint8_t nonce[8],
                    int board, uint8_t out[26]);
/* The server's CURSOR body (after the type): -1 when it is cut short. */
int nic_up_cursor(nic_uplink *u, const uint8_t *body, size_t n);
/* The next ARCHIVE message, in u->pkt; its length, 0 when everything the cards hold has gone. */
size_t nic_up_next(nic_uplink *u);
/* A WINDOW the head has accepted: its answer queued. -1 when the queue is full or the address
 * has no files. */
int nic_up_window_ask(nic_uplink *u, uint8_t addr, uint32_t second, uint32_t span);
/* The server's ACK body: -1 when it is cut short. */
int nic_up_ack(nic_uplink *u, const uint8_t *body, size_t n);
/* The entries of an address the server has not acknowledged. */
uint32_t nic_up_behind(const nic_uplink *u, uint8_t addr);

/* a before b, in a circle of 2^16 */
static inline int nic_up_before(uint16_t a, uint16_t b) { return ((uint16_t)(a - b) & 0x8000u) != 0; }

#endif

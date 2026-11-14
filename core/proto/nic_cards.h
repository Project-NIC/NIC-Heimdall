/* The head's two cards, as ../archive/HMC.md (*The files*) and ../../mayak/FIRMWARE.md §6 fix them:
 * the same files on both; the circle — while either card has less than the reserve free, the
 * oldest file by its first second, whatever the address and never an open one, goes from every
 * card that is in, the firmware told whether the server held it whole; a card let out by the
 * `card` command; a card put back brought level — the files open now created on it where missing
 * before the writer turns to it, their gaps copied first, then every other file of the address
 * newest first, a missing file whole and a short one from its last good entry, a file the circle
 * took meanwhile deleted from it — in steps the firmware runs below every write.
 *
 * The catalogue is per address — its files are one run of sequences, first to live, as the uplink
 * has them. A file's last good entry is found by binary search, its data copied before its
 * entries, as the writer writes them, so a copy cut short is a file cut short and nothing worse.
 * The cards are read and written through the firmware's calls (FatFs on the head); nothing is
 * allocated. */
#ifndef NIC_CARDS_H
#define NIC_CARDS_H

#include <stddef.h>
#include <stdint.h>

#define NIC_CARDS_ADDRS   64u
#define NIC_CARDS_RESERVE (512ull << 20)    /* kept free on each card */
#define NIC_CARDS_CHUNK   16384u            /* one copy's bytes */

typedef struct {
    uint8_t addr;
    uint16_t seq;
    uint32_t first_second;
    uint8_t size;                  /* 1 MB × 2^size */
} nic_card_file;

typedef struct {
    uint64_t (*free_bytes)(void *ctx, int card);
    /* a file's bytes, 0 when read; a file not on the card is not read */
    int (*read)(void *ctx, int card, uint8_t addr, uint16_t seq, uint32_t off, uint32_t len, uint8_t *buf);
    int (*write)(void *ctx, int card, uint8_t addr, uint16_t seq, uint32_t off, uint32_t len,
                 const uint8_t *buf);
    int (*create)(void *ctx, int card, const nic_card_file *f);       /* allocated whole */
    int (*remove)(void *ctx, int card, uint8_t addr, uint16_t seq);
    /* an address's oldest and newest file on a card, as its directory gave them; -1 none */
    int (*span)(void *ctx, int card, uint8_t addr, uint16_t *first, uint16_t *live);
    /* the server has acknowledged the file whole */
    int (*held)(void *ctx, uint8_t addr, uint16_t seq);
    /* the circle took a file: the event log's line when it was not held, the uplink told */
    void (*gone)(void *ctx, uint8_t addr, uint16_t seq, int held);
    void (*level)(void *ctx, int card);                               /* the cards are level */
    void *ctx;
} nic_card_io;

typedef struct {
    uint8_t addr;
    uint16_t first, live;
    uint32_t first_second;         /* the oldest file's, read from its header */
    uint16_t cursor;               /* the sync: the next closed file, newest first */
    uint8_t cursor_on;
    uint16_t live_then;            /* the file open when the sync began */
    uint32_t gap_from, gap_to;     /* its entries the card put back lacked */
    uint8_t gap_due;
} nic_card_addr;

enum { NIC_SYNC_OFF = 0, NIC_SYNC_GAPS, NIC_SYNC_FILES };

typedef struct {
    nic_card_io io;
    uint64_t reserve;
    nic_card_addr a[NIC_CARDS_ADDRS];
    uint8_t n;
    uint8_t in[2];
    uint8_t sync_state;
    int8_t sync_card;
    uint8_t sync_k;                /* the address being worked */
    uint8_t copying;               /* a copy in progress: its file and ranges */
    uint8_t c_addr;
    uint16_t c_seq;
    uint32_t c_off, c_end;         /* the data, then the entries */
    uint32_t e_off, e_end;
    uint32_t gone_unheld, copied;  /* for the event log */
    uint8_t buf[NIC_CARDS_CHUNK];
} nic_cards;

void nic_cards_init(nic_cards *c, const nic_card_io *io, int in0, int in1);
/* An address's files first to live; called again when a new file opens. */
int nic_cards_files(nic_cards *c, uint8_t addr, uint16_t first, uint16_t live);
/* The circle: files taken until every card in has the reserve free. Its count, -1 when only open
 * files are left. */
int nic_cards_circle(nic_cards *c);
/* The `card` command: the firmware has written every buffer to it; it is written no more. */
void nic_cards_out(nic_cards *c, int card);
/* A card answered in an empty slot: the open files made ready on it. From the return on, the
 * writer writes it. */
int nic_cards_back(nic_cards *c, int card);
/* The background copy, at most `budget` bytes: 1 while there is more, 0 when the cards are level. */
int nic_cards_step(nic_cards *c, uint32_t budget);
/* A file's good entries on a card: the first that fails, by binary search. */
uint32_t nic_cards_good(nic_cards *c, int card, uint8_t addr, uint16_t seq, uint8_t size);

#endif

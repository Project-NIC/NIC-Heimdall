/* A stream down and a trial boot, as core/PROTOCOL.md §7 fixes them: LOAD, the kind 9 frames —
 * the image header first, the body after, the last frame padded — and APPLY, the new image booted
 * untried and confirmed by its first answered GET VERSION. The flash and the persisted cells are
 * the firmware's, reached through callbacks; this is the logic, on a PC as on the MCU. */
#ifndef NIC_LOAD_H
#define NIC_LOAD_H

#include <stddef.h>
#include <stdint.h>

#include "nic_proto.h"

/* ERROR's codes (§9): arg0 the code, arg1 the op it answers */
enum nic_error {
    NIC_E_REGISTER = 1,     /* no such register */
    NIC_E_ADDRESS = 2,      /* no such address — a card's */
    NIC_E_TUNNEL = 3,       /* the tunnel timed out */
    NIC_E_NOISY = 4,        /* a calibration too noisy */
    NIC_E_VALUE = 5,        /* a value the register does not take */
    NIC_E_GAP = 16,         /* a stream frame out of its count, or one that failed its CRC */
    NIC_E_TYPE = 17,        /* an image for another TYPE, or a header for another slot */
    NIC_E_LENGTH = 18,      /* the length and the frames LOAD announced disagree, or past the slot */
    NIC_E_CRC32 = 19,       /* the body's CRC-32 is not the header's */
    NIC_E_FLASH = 20,       /* the slot could not be erased or written */
    NIC_E_NO_IMAGE = 21     /* APPLY with no whole image loaded */
};

/* The image header, the stream's first 32 B: TYPE 1 · slot 1 · reserved 2 · length 4 · CRC-32 4 ·
 * version 2 · reserved 18, zeros. */
typedef struct {
    uint8_t type, slot;
    uint32_t length, crc;
    uint16_t version;
} nic_image_header;

void nic_image_header_build(const nic_image_header *h, uint8_t out[NIC_PAYLOAD]);
int nic_image_header_parse(const uint8_t in[NIC_PAYLOAD], nic_image_header *h);
uint32_t nic_crc32(uint32_t crc, const uint8_t *b, size_t n);   /* IEEE 802.3; start from 0 */

typedef struct {
    int (*erase)(void *ctx, uint8_t slot, uint32_t length);
    int (*write)(void *ctx, uint8_t slot, uint32_t offset, const uint8_t *b, size_t n);
    void *ctx;
    uint32_t slot_size[2];   /* 0 the image slot, 1 the table slot */
} nic_flash;

typedef struct {
    nic_flash flash;
    uint8_t type;            /* the unit's own TYPE */
    uint8_t open, slot, payload, have_header;
    uint32_t frames, got, written, crc;
    uint8_t head[NIC_PAYLOAD];
    nic_image_header h;
} nic_loader;

/* What the unit answers: nothing yet, ACK, or ERROR with its code. */
typedef struct {
    uint8_t op;      /* 0 nothing · NIC_ACK · NIC_ERROR */
    uint8_t code;
} nic_answer;

void nic_loader_init(nic_loader *l, const nic_flash *f, uint8_t type);
/* LOAD: the frames that follow and the slot; `payload` 32 on a spur, 8 behind an Argus. */
nic_answer nic_loader_load(nic_loader *l, const nic_ctrl *load, uint8_t payload);
/* A stream frame, already through its CRC; a frame that failed it goes to nic_loader_miss. */
nic_answer nic_loader_frame(nic_loader *l, const nic_header *h, const uint8_t *payload);
nic_answer nic_loader_miss(nic_loader *l);
/* A whole image waits for APPLY: 1 when the last LOAD ended in ACK on slot 0. */
int nic_loader_ready(const nic_loader *l);

/* The master's side: the stream's frames for a LOAD, each through `emit` — the first the header,
 * the body after it, the last padded; `payload` 32, or 8 when a card cuts for an Argus. */
uint32_t nic_stream_frames(uint32_t length, uint8_t payload);
void nic_stream(const nic_image_header *h, const uint8_t *body, uint8_t addr, uint8_t unix0,
                uint8_t payload, void (*emit)(void *ctx, const uint8_t *frame, size_t len),
                void *ctx);

/* ------------------------------------------------------------------ the trial boot */

/* Persisted in the unit's configuration cells: the slot that boots, and a trial's state. */
typedef struct {
    uint8_t active;          /* 0 or 1 */
    uint8_t trial;           /* 0 none · 1 untried, to start · 2 started, not confirmed */
} nic_boot;

/* APPLY: the other slot booted next, untried. */
void nic_boot_apply(nic_boot *b);
/* The bootloader: the slot to start. An untried image starts once; one that reset before it
 * confirmed hands the boot back to the old slot. The cells are written when *b changes. */
uint8_t nic_boot_select(nic_boot *b);
/* The image's first answered GET VERSION: 1 when that confirmed a trial — the cells to write. */
int nic_boot_confirm(nic_boot *b, uint8_t running);

#endif

/* The head's USB console and its first boot, as ../../mayak/FIRMWARE.md §2, §10 and §14 and
 * ../PROTOCOL.md §6 fix them. The console is a line-oriented command set on USB Serial/JTAG, in every
 * state: `mirror` · `tree` · `log` · `tail` · `hermes` read, answered by the firmware's handlers;
 * `card out N` lets a card out as the handset's `card` does; `id` gives the station's identity;
 * `k0` gives K0 once — the provisioning tool's one read into the operator's key store — and is
 * refused for ever after, until the board is flashed again over BOOT0 and makes a new one. At the
 * first boot, with no K0 in the key store, the TRNG makes it. Lines end in CR, LF or both, 128 B at
 * most; an answer is one line or several, closed by `ok` or `error <why>`. Nothing is allocated. */
#ifndef NIC_CONSOLE_H
#define NIC_CONSOLE_H

#include <stddef.h>
#include <stdint.h>

#define NIC_CON_LINE 128u

typedef struct {
    /* the key store in the module's encrypted flash: 0 read, -1 none */
    int (*k0_load)(void *ctx, uint8_t k0[8], uint8_t *read_out);
    int (*k0_store)(void *ctx, const uint8_t k0[8], uint8_t read_out);
    int (*trng)(void *ctx, uint8_t *b, size_t n);
    /* an answer's line, without its end */
    void (*say)(void *ctx, const char *line);
    /* the readers: each says its lines; 0 done */
    int (*mirror)(void *ctx, const char *arg);
    int (*tree)(void *ctx);
    int (*log)(void *ctx, const char *arg);
    int (*tail)(void *ctx, const char *arg);
    int (*hermes)(void *ctx);
    int (*card_out)(void *ctx, int slot);
    void *ctx;
} nic_con_io;

typedef struct {
    nic_con_io io;
    uint32_t station;
    char line[NIC_CON_LINE + 1];
    size_t n;
    uint8_t overflow, last_cr;
} nic_console;

/* The first boot's part: K0 made if the store has none. 1 made now, 0 there already, -1 failed. */
int nic_con_first_boot(const nic_con_io *io);
void nic_con_init(nic_console *c, const nic_con_io *io, uint32_t station);
/* Bytes from the USB: each whole line run. */
void nic_con_bytes(nic_console *c, const uint8_t *b, size_t n);

#endif

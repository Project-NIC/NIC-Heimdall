/* The head's USB console and its first boot — see nic_console.h. */
#include "nic_console.h"

#include <string.h>

int nic_con_first_boot(const nic_con_io *io)
{
    uint8_t k0[8], read_out;
    if (!io->k0_load(io->ctx, k0, &read_out))
        return 0;
    if (io->trng(io->ctx, k0, sizeof k0) || io->k0_store(io->ctx, k0, 0))
        return -1;
    memset(k0, 0, sizeof k0);
    return 1;
}

void nic_con_init(nic_console *c, const nic_con_io *io, uint32_t station)
{
    memset(c, 0, sizeof *c);
    c->io = *io;
    c->station = station;
}

static void hex(char *o, const uint8_t *b, size_t n)
{
    for (size_t i = 0; i < n; i++) {
        o[2 * i] = "0123456789abcdef"[b[i] >> 4];
        o[2 * i + 1] = "0123456789abcdef"[b[i] & 15];
    }
    o[2 * n] = 0;
}

static void done(nic_console *c, int r, const char *why)
{
    if (!r) {
        c->io.say(c->io.ctx, "ok");
    } else {
        char e[96] = "error ";
        size_t n = strlen(why);
        if (n > sizeof e - 7)
            n = sizeof e - 7;
        memcpy(e + 6, why, n);
        e[6 + n] = 0;
        c->io.say(c->io.ctx, e);
    }
}

/* The command word and the rest of the line after its spaces. */
static const char *word(const char *line, char *w, size_t max)
{
    size_t k = 0;
    while (*line == ' ')
        line++;
    while (*line && *line != ' ' && k + 1 < max)
        w[k++] = *line++;
    w[k] = 0;
    while (*line && *line != ' ')                       /* a word too long: cut, its tail skipped */
        line++;
    while (*line == ' ')
        line++;
    return line;
}

static void run(nic_console *c, const char *line)
{
    char w[16];
    const char *arg = word(line, w, sizeof w);
    if (!w[0])
        return;
    if (!strcmp(w, "mirror")) {
        done(c, c->io.mirror(c->io.ctx, arg), "mirror");
    } else if (!strcmp(w, "tree")) {
        done(c, c->io.tree(c->io.ctx), "tree");
    } else if (!strcmp(w, "log")) {
        done(c, c->io.log(c->io.ctx, arg), "log");
    } else if (!strcmp(w, "tail")) {
        done(c, c->io.tail(c->io.ctx, arg), "tail");
    } else if (!strcmp(w, "hermes")) {
        done(c, c->io.hermes(c->io.ctx), "hermes");
    } else if (!strcmp(w, "id")) {
        char o[16] = "id ";
        uint8_t b[3] = {(uint8_t)(c->station >> 16), (uint8_t)(c->station >> 8), (uint8_t)c->station};
        hex(o + 3, b, 3);
        c->io.say(c->io.ctx, o);
        done(c, 0, "");
    } else if (!strcmp(w, "card")) {
        char sub[8];
        const char *n = word(arg, sub, sizeof sub);
        if (strcmp(sub, "out") || (strcmp(n, "0") && strcmp(n, "1")))
            done(c, -1, "usage: card out 0|1");
        else
            done(c, c->io.card_out(c->io.ctx, n[0] - '0'), "card");
    } else if (!strcmp(w, "k0")) {
        uint8_t k0[8], read_out;
        if (c->io.k0_load(c->io.ctx, k0, &read_out)) {
            done(c, -1, "no K0");
        } else if (read_out) {
            done(c, -1, "K0 has left the board once; flash it again over BOOT0 for a new one");
        } else if (c->io.k0_store(c->io.ctx, k0, 1)) {  /* marked read before it is shown */
            done(c, -1, "the key store refused");
        } else {
            char o[24] = "k0 ";
            hex(o + 3, k0, 8);
            c->io.say(c->io.ctx, o);
            memset(o, 0, sizeof o);
            done(c, 0, "");
        }
        memset(k0, 0, sizeof k0);
    } else {
        done(c, -1, "unknown command");
    }
}

void nic_con_bytes(nic_console *c, const uint8_t *b, size_t n)
{
    for (size_t i = 0; i < n; i++) {
        char ch = (char)b[i];
        if (ch == '\r' || ch == '\n') {
            if (ch == '\n' && c->last_cr && !c->n && !c->overflow) {
                c->last_cr = 0;                         /* the LF of a CR LF */
                continue;
            }
            c->last_cr = ch == '\r';
            c->line[c->n] = 0;
            if (c->overflow)
                done(c, -1, "line too long");
            else
                run(c, c->line);
            c->n = 0;
            c->overflow = 0;
            continue;
        }
        c->last_cr = 0;
        if (c->n < NIC_CON_LINE && ch >= 0x20 && ch < 0x7F)
            c->line[c->n++] = ch;
        else if (c->n >= NIC_CON_LINE)
            c->overflow = 1;
    }
}

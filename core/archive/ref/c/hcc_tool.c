/* The C reference driven from ../tests/test_c.py: requests on stdin, answers on stdout, all
 * little-endian.
 *   'E' layout records   → 'E' status u32-length stream
 *   'D' layout u32-length stream → 'D' status [records]
 *   'L' fields           → 'L' layout            (hcc_compose_layout)
 *   'A' u8-positions, each fields → 'L' layout   (hcc_argus_layout; 0 fields an empty position)
 * fields and layout: u8 count, then offset · width · signed · big_endian · channel · ref;
 * records: u32 count, then u32 second · frame · kind · status · form · u16 length · the bytes. */
#include "hcc.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static uint8_t *in;
static size_t in_len, at;

static int need(size_t n) { return at + n <= in_len; }
static uint8_t u8(void) { return in[at++]; }
static uint16_t u16(void) { uint16_t v = (uint16_t)(in[at] | in[at + 1] << 8); at += 2; return v; }
static uint32_t u32(void)
{
    uint32_t v = (uint32_t)in[at] | (uint32_t)in[at + 1] << 8 | (uint32_t)in[at + 2] << 16
                 | (uint32_t)in[at + 3] << 24;
    at += 4;
    return v;
}

static void put8(uint8_t v) { fputc(v, stdout); }
static void put16(uint16_t v) { put8((uint8_t)v); put8((uint8_t)(v >> 8)); }
static void put32(uint32_t v) { put16((uint16_t)v); put16((uint16_t)(v >> 16)); }

static size_t fields(hcc_field *f)
{
    size_t n = u8();
    for (size_t i = 0; i < n; i++) {
        f[i].offset = u8();
        f[i].width = u8();
        f[i].is_signed = u8();
        f[i].big_endian = u8();
        f[i].channel = u8();
        f[i].ref = u8();
    }
    return n;
}

static void put_fields(const hcc_field *f, size_t n)
{
    put8((uint8_t)n);
    for (size_t i = 0; i < n; i++) {
        put8(f[i].offset); put8(f[i].width); put8(f[i].is_signed);
        put8(f[i].big_endian); put8(f[i].channel); put8(f[i].ref);
    }
}

int main(void)
{
    size_t cap = 1 << 20;
    in = malloc(cap);
    for (size_t r; (r = fread(in + in_len, 1, cap - in_len, stdin)) > 0;) {
        in_len += r;
        if (in_len == cap)
            in = realloc(in, cap *= 2);
    }
    hcc_field lay[HCC_MAX_LAYOUT * 4], pos[8][HCC_MAX_LAYOUT];
    while (need(1)) {
        uint8_t cmd = u8();
        if (cmd == 'E') {
            size_t nl = fields(lay), n = u32();
            hcc_record *recs = malloc(sizeof(hcc_record) * (n ? n : 1));
            for (size_t i = 0; i < n; i++) {
                recs[i].second = u32();
                recs[i].frame = u8();
                recs[i].kind = u8();
                recs[i].status = u8();
                recs[i].form = u8();
                recs[i].length = u16();
                recs[i].data = in + at;
                at += recs[i].length;
            }
            uint8_t *s;
            size_t len;
            int st = hcc_encode(recs, n, lay, nl, &s, &len);
            put8('E'); put8((uint8_t)st); put32((uint32_t)len);
            fwrite(s, 1, len, stdout);
            hcc_free(s);
            free(recs);
        } else if (cmd == 'D') {
            size_t nl = fields(lay), len = u32();
            hcc_record *recs;
            size_t n;
            int st = hcc_decode(in + at, len, lay, nl, &recs, &n);
            at += len;
            put8('D'); put8((uint8_t)st);
            if (st == HCC_OK) {
                put32((uint32_t)n);
                for (size_t i = 0; i < n; i++) {
                    put32(recs[i].second); put8(recs[i].frame); put8(recs[i].kind);
                    put8(recs[i].status); put8(recs[i].form); put16(recs[i].length);
                    fwrite(recs[i].data, 1, recs[i].length, stdout);
                }
                hcc_free(recs);
            }
        } else if (cmd == 'L') {
            size_t n = fields(lay);
            hcc_field out[HCC_MAX_LAYOUT * 2];
            put8('L');
            put_fields(out, hcc_compose_layout(lay, n, out));
        } else if (cmd == 'A') {
            size_t np = u8(), counts[8];
            const hcc_field *ptr[8];
            for (size_t p = 0; p < np; p++) {
                counts[p] = fields(pos[p]);
                ptr[p] = pos[p];
            }
            put8('L');
            put_fields(lay, hcc_argus_layout(ptr, counts, np, lay));
        } else {
            fprintf(stderr, "hcc_tool: unknown request %d at %zu\n", cmd, at - 1);
            return 2;
        }
    }
    free(in);
    return 0;
}

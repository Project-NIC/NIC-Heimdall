/* The C reference driven from ../tests/test_c.py: requests on stdin, answers on stdout, all
 * little-endian.
 *   'E' layout records   → 'E' status u32-length stream
 *   'D' layout u32-length stream → 'D' status [records]
 *   'L' fields           → 'L' layout            (hcc_compose_layout)
 *   'A' u8-positions, each fields → 'L' layout   (hcc_argus_layout; 0 fields an empty position)
 * fields and layout: u8 count, then offset · width · signed · big_endian · channel · ref;
 * records: u32 count, then u32 second · frame · kind · status · form · u16 length · the bytes.
 *   'W' address u8 · sequence u16 · first u32 · station u32 · size u8 · u32-length sections,
 *       then ops to 'Z': 'S' records · 'C' from u32 · u32-length sections · 'V' u32 count,
 *       12 B lines → 'W' status · kept u32 · u32-length image          (hmc_writer)
 *   'R' u32-length image → 'R' status · entries u32 · per object: 0 damaged · 1 segment first ·
 *       last · codec · status · records or lines · 2 config from · u32-length sections
 *   'P' address · second u32 → 'P' u8-length path
 *   'K' rule (mode · n · s u16 · u8-count modules) · blocks · 0 and u32-count frames (second u32 ·
 *       frame · kind · status · 32 B) or 1 and records → 'K' the records kept */
#include "hcc.h"
#include "hmc.h"
#include "session.h"

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

static hcc_record *records(size_t *n)
{
    *n = u32();
    hcc_record *recs = malloc(sizeof(hcc_record) * (*n ? *n : 1));
    for (size_t i = 0; i < *n; i++) {
        recs[i].second = u32();
        recs[i].frame = u8();
        recs[i].kind = u8();
        recs[i].status = u8();
        recs[i].form = u8();
        recs[i].length = u16();
        recs[i].data = in + at;
        at += recs[i].length;
    }
    return recs;
}

static void put_records(const hcc_record *r, size_t n)
{
    put32((uint32_t)n);
    for (size_t i = 0; i < n; i++) {
        put32(r[i].second); put8(r[i].frame); put8(r[i].kind);
        put8(r[i].status); put8(r[i].form); put16(r[i].length);
        fwrite(r[i].data, 1, r[i].length, stdout);
    }
}

static hmc_event *lines(size_t *n)
{
    *n = u32();
    hmc_event *l = malloc(sizeof(hmc_event) * (*n ? *n : 1));
    for (size_t i = 0; i < *n; i++) {
        l[i].second = u32();
        l[i].source = u8();
        l[i].code = u8();
        l[i].detail = u16();
        l[i].value = u32();
    }
    return l;
}

static void write_file(void)
{
    hmc_header h;
    hmc_writer w;
    h.address = u8();
    h.sequence = u16();
    h.first_second = u32();
    h.station = u32();
    h.size = u8();
    h.sections_len = u32();
    h.sections = in + at;
    at += h.sections_len;
    int st = hmc_writer_open(&w, &h);
    uint32_t kept = 0;
    for (uint8_t op; (op = u8()) != 'Z';) {
        size_t n;
        int e = HMC_OK;
        if (op == 'S') {
            hcc_record *r = records(&n);
            if (!st) e = hmc_writer_segment(&w, r, n);
            free(r);
        } else if (op == 'V') {
            hmc_event *l = lines(&n);
            if (!st) e = hmc_writer_events(&w, l, n);
            free(l);
        } else {
            uint32_t from = u32();
            size_t slen = u32();
            if (!st) e = hmc_writer_config(&w, from, in + at, slen);
            at += slen;
        }
        if (!st && e)
            st = e == HMC_FULL ? -1 : e;
        else if (!st)
            kept++;
    }
    put8('W'); put8((uint8_t)(st < 0 ? 0 : st)); put32(kept);
    put32((uint32_t)(st > 0 ? 0 : w.size));
    if (st <= 0)
        fwrite(w.image, 1, w.size, stdout);
    if (st <= 0)
        hmc_writer_close(&w);
}

static void read_file(void)
{
    size_t len = u32();
    hmc_reader r;
    int st = hmc_reader_open(&r, in + at, len);
    at += len;
    put8('R'); put8((uint8_t)st);
    if (st)
        return;
    put32((uint32_t)r.entries);
    hmc_sections force = {0}, hs;
    hmc_sections_parse(r.h.sections, r.h.sections_len, &hs);
    hmc_sections_apply(&force, &hs);
    for (size_t i = 0; i < r.entries; i++) {
        uint8_t *buf;
        hmc_object o;
        if (hmc_reader_object(&r, i, &buf, &o)) {
            put8(0);
            free(buf);
            continue;
        }
        if (o.type == HMC_CFG) {
            hmc_sections cs;
            hmc_sections_parse(o.body, o.body_len, &cs);
            hmc_sections_apply(&force, &cs);
            put8(2); put32(o.from); put32((uint32_t)o.body_len);
            fwrite(o.body, 1, o.body_len, stdout);
            free(buf);
            continue;
        }
        put8(1); put32(o.first); put32(o.last); put8(o.codec);
        if (r.h.address == 0) {
            hmc_event *l;
            size_t n;
            int e = hmc_events_parse(&o, &l, &n);
            put8((uint8_t)e);
            if (!e) {
                put32((uint32_t)n);
                for (size_t k = 0; k < n; k++) {
                    put32(l[k].second); put8(l[k].source); put8(l[k].code);
                    put16(l[k].detail); put32(l[k].value);
                }
            }
            free(l);
        } else {
            hcc_field lay[HCC_MAX_LAYOUT];
            size_t nl, n = 0;
            hcc_record *recs = NULL;
            int e = hmc_hcc_layout(&force, lay, &nl);
            if (!e)
                e = hmc_segment_records(&o, lay, nl, &recs, &n);
            put8((uint8_t)e);
            if (!e)
                put_records(recs, n);
            hcc_free(recs);
        }
        free(buf);
    }
    hmc_sections_free(&force);
}

static void select_(void)
{
    hmc_rule rule;
    hmc_selector sel;
    rule.mode = u8();
    rule.n = u8();
    rule.s = u16();
    rule.nmodules = u8();
    for (unsigned i = 0; i < rule.nmodules; i++)
        rule.modules[i] = u8();
    int blocks = u8(), how = u8();
    hmc_selector_init(&sel, &rule, blocks);
    size_t n, m = 0;
    hcc_record *kept;
    if (how == 0) {
        n = u32();
        kept = malloc(sizeof(hcc_record) * 16 * (n ? n : 1));
        for (size_t i = 0; i < n; i++) {
            uint32_t sec = u32();
            uint8_t fr = u8(), kind = u8(), st = u8();
            m += hmc_select_frame(&sel, sec, fr, kind, st, in + at, kept + m);
            at += HCC_PAYLOAD;
        }
    } else {
        hcc_record *r = records(&n);
        kept = malloc(sizeof(hcc_record) * (n ? n : 1));
        for (size_t i = 0; i < n; i++)
            if (hmc_select_record(&sel, &r[i]))
                kept[m++] = r[i];
        free(r);
    }
    put8('K');
    put_records(kept, m);
    free(kept);
    hmc_selector_free(&sel);
}

static void put_bytes(const uint8_t *b, size_t n) { put32((uint32_t)n); fwrite(b, 1, n, stdout); }

static void chain_in(ses_chain *c)
{
    memcpy(c->key, in + at, 16);
    memcpy(c->nonce, in + at + 16, 8);
    memcpy(c->last, in + at + 24, 8);
    at += 32;
    c->packet = u32();
}

static void session_req(uint8_t cmd)
{
    uint8_t *msg = NULL, key[16];
    size_t len = 0;
    if (cmd == 'I') {
        size_t kl = u8();
        const uint8_t *k = in + at;
        at += kl;
        size_t n = u32();
        uint8_t out[32];
        ses_hmac_sha256(k, kl, in + at, n, out);
        at += n;
        put8('I');
        fwrite(out, 1, 32, stdout);
        return;
    }
    if (cmd == 'H') {
        uint8_t tag[8];
        memcpy(key, in + at, 16); at += 16;
        uint32_t st = u32(), ctr = u32();
        const uint8_t *nonce = in + at; at += 8;
        int board = u8();
        ses_hello(key, st, ctr, nonce, board, tag, &msg, &len);
        put8('H'); fwrite(tag, 1, 8, stdout); put_bytes(msg, len);
    } else if (cmd == 'h') {
        ses_hello_t h;
        memcpy(key, in + at, 16); at += 16;
        size_t n = u32();
        int e = ses_hello_parse(key, in + at, n, &h);
        at += n;
        put8('h'); put8((uint8_t)e);
        if (!e) {
            put32(h.station); put32(h.counter); fwrite(h.nonce, 1, 8, stdout);
            put8(h.board); fwrite(h.tag, 1, 8, stdout);
        }
    } else if (cmd == 'M') {
        int type = u8();
        size_t n = u16();
        ses_mark *m = malloc(sizeof(ses_mark) * (n ? n : 1));
        for (size_t i = 0; i < n; i++) {
            m[i].address = u8();
            m[i].sequence = u16();
            m[i].entry = u32();
        }
        ses_marks(type, m, n, &msg, &len);
        free(m);
        put8('M'); put_bytes(msg, len);
    } else if (cmd == 'X') {
        ses_chain c;
        chain_in(&c);
        uint8_t a = u8();
        uint16_t sq = u16();
        size_t hl = u32();
        const uint8_t *h = in + at;
        at += hl;
        size_t n = u16();
        ses_object *o = malloc(sizeof(ses_object) * (n ? n : 1));
        for (size_t i = 0; i < n; i++) {
            o[i].index = u32();
            o[i].entry = in + at; at += 16;
            o[i].cobs_len = u32();
            o[i].cobs = in + at; at += o[i].cobs_len;
        }
        ses_archive(&c, a, sq, h, hl, o, n, &msg, &len);
        free(o);
        put8('X'); put_bytes(msg, len); fwrite(c.last, 1, 8, stdout);
    } else if (cmd == 'Y') {
        ses_chain c;
        ses_archive_t ar;
        chain_in(&c);
        size_t n = u32();
        int e = ses_archive_verify(&c, in + at, n, &ar);
        at += n;
        put8('Y'); put8((uint8_t)e);
        if (!e) {
            put8(ar.address); put16(ar.sequence); put32(ar.packet);
            put_bytes(ar.header, ar.header_len); put32((uint32_t)ar.count);
            for (size_t i = 0; i < ar.count; i++) {
                put32(ar.objects[i].index); fwrite(ar.objects[i].entry, 1, 16, stdout);
                put_bytes(ar.objects[i].cobs, ar.objects[i].cobs_len);
            }
            fwrite(c.last, 1, 8, stdout); put32(c.packet);
            free(ar.objects);
        }
    } else if (cmd == 'G') {
        uint32_t sec = u32();
        size_t n = u8();
        ses_seg *sg = malloc(sizeof(ses_seg) * (n ? n : 1));
        for (size_t i = 0; i < n; i++) {
            sg[i].address = u8();
            sg[i].len = u32();
            sg[i].segment = in + at; at += sg[i].len;
        }
        ses_live(sec, sg, n, &msg, &len);
        free(sg);
        put8('G'); put_bytes(msg, len);
    } else if (cmd == 'g') {
        size_t n = u32(), cnt;
        uint32_t sec;
        ses_seg *sg;
        int e = ses_live_parse(in + at, n, &sec, &sg, &cnt);
        at += n;
        put8('g'); put8((uint8_t)e);
        if (!e) {
            put32(sec); put32((uint32_t)cnt);
            for (size_t i = 0; i < cnt; i++) {
                put8(sg[i].address); put_bytes(sg[i].segment, sg[i].len);
            }
            free(sg);
        }
    } else if (cmd == 'N') {
        memcpy(key, in + at, 16); at += 16;
        uint32_t st = u32(), ctr = u32();
        uint8_t a = u8();
        uint32_t sec = u32(), span = u32();
        ses_window(key, st, ctr, a, sec, span, &msg, &len);
        put8('N'); put_bytes(msg, len);
    } else if (cmd == 'n') {
        ses_window_t w;
        memcpy(key, in + at, 16); at += 16;
        uint32_t st = u32(), stored = u32();
        size_t n = u32();
        int e = ses_window_parse(key, st, stored, in + at, n, &w);
        at += n;
        put8('n'); put8((uint8_t)e);
        if (!e) {
            put32(w.counter); put8(w.address); put32(w.second); put32(w.window);
        }
    } else if (cmd == 'B') {
        uint32_t st = u32();
        size_t n = u32();
        ses_board(st, in + at, n, &msg, &len);
        at += n;
        put8('B'); put_bytes(msg, len);
    } else if (cmd == 'F' || cmd == 'C') {
        size_t n = u32();
        hmc_reader r;
        int e = hmc_reader_open(&r, in + at, n);
        at += n;
        uint32_t a1 = u32(), a2 = u32();
        uint32_t *idx = malloc(sizeof(uint32_t) * (r.entries + 1)),
                 *pk = malloc(sizeof(uint32_t) * (r.entries + 1));
        size_t m = e ? 0 : cmd == 'F' ? ses_forward(&r, a1, a2, idx, pk) : ses_window_cut(&r, a1, a2, idx);
        put8(cmd); put32((uint32_t)m);
        for (size_t i = 0; i < m; i++) {
            put32(idx[i]);
            if (cmd == 'F')
                put32(pk[i]);
        }
        free(idx);
        free(pk);
    }
    free(msg);
}

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
        } else if (cmd == 'W') {
            write_file();
        } else if (cmd == 'R') {
            read_file();
        } else if (strchr("IHhMXYGgNnBFC", cmd)) {
            session_req(cmd);
        } else if (cmd == 'K') {
            select_();
        } else if (cmd == 'P') {
            uint8_t a = u8();
            char path[32];
            hmc_path(a, u32(), path);
            put8('P'); put8((uint8_t)strlen(path));
            fwrite(path, 1, strlen(path), stdout);
        } else {
            fprintf(stderr, "hcc_tool: unknown request %d at %zu\n", cmd, at - 1);
            return 2;
        }
    }
    free(in);
    return 0;
}

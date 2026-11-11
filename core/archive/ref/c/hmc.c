/* HMC — the C reference; see hmc.h. */
#include "hmc.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* ------------------------------------------------------------------ little-endian */

static void put16(uint8_t *p, uint32_t v) { p[0] = (uint8_t)v; p[1] = (uint8_t)(v >> 8); }
static void put32(uint8_t *p, uint32_t v) { put16(p, v & 0xFFFF); put16(p + 2, v >> 16); }
static uint32_t get16(const uint8_t *p) { return (uint32_t)p[0] | (uint32_t)p[1] << 8; }
static uint32_t get32(const uint8_t *p) { return get16(p) | get16(p + 2) << 16; }

/* ------------------------------------------------------------------ checks and framing */

uint32_t hmc_crc32(const uint8_t *b, size_t n)
{
    static uint32_t table[256];
    static int ready;
    if (!ready) {
        for (uint32_t i = 0; i < 256; i++) {
            uint32_t c = i;
            for (int k = 0; k < 8; k++)
                c = (c & 1) ? 0xEDB88320u ^ (c >> 1) : c >> 1;
            table[i] = c;
        }
        ready = 1;
    }
    uint32_t c = 0xFFFFFFFFu;
    for (size_t i = 0; i < n; i++)
        c = table[(c ^ b[i]) & 0xFF] ^ (c >> 8);
    return c ^ 0xFFFFFFFFu;
}

uint16_t hmc_crc16(const uint8_t *b, size_t n)
{
    uint16_t c = 0xFFFF;
    for (size_t i = 0; i < n; i++) {
        c ^= (uint16_t)(b[i] << 8);
        for (int k = 0; k < 8; k++)
            c = (c & 0x8000) ? (uint16_t)((c << 1) ^ 0x1021) : (uint16_t)(c << 1);
    }
    return c;
}

size_t hmc_cobs_encode(const uint8_t *in, size_t n, uint8_t *out)
{
    size_t at = 0, o = 1;
    uint8_t code = 1;
    for (size_t i = 0; i < n; i++) {
        if (in[i] == 0) {
            out[at] = code;
            at = o++;
            code = 1;
        } else {
            out[o++] = in[i];
            if (++code == 0xFF) {
                out[at] = code;
                at = o++;
                code = 1;
            }
        }
    }
    out[at] = code;
    return o;
}

int hmc_cobs_decode(const uint8_t *in, size_t n, uint8_t *out, size_t *outlen)
{
    size_t i = 0, o = 0;
    while (i < n) {
        uint8_t c = in[i];
        if (c == 0 || i + c > n)
            return HMC_DAMAGED;
        for (size_t k = 1; k < c; k++) {
            if (in[i + k] == 0)
                return HMC_DAMAGED;
            out[o++] = in[i + k];
        }
        i += c;
        if (c < 0xFF && i < n)
            out[o++] = 0;
    }
    *outlen = o;
    return HMC_OK;
}

/* ------------------------------------------------------------------ sections */

int hmc_sections_parse(const uint8_t *b, size_t len, hmc_sections *out)
{
    size_t i = 0;
    out->n = 0;
    while (i < len) {
        if (len - i < 8)
            return HMC_DAMAGED;
        uint32_t ln = get32(b + i + 4);
        if (ln > len - i - 8)
            return HMC_DAMAGED;
        if (out->n == HMC_MAX_SECTIONS)
            return HMC_EINVAL;
        out->s[out->n].id = b[i];
        out->s[out->n].length = ln;
        out->s[out->n].body = b + i + 8;
        out->n++;
        i += 8 + ln;
    }
    return HMC_OK;
}

int hmc_sections_apply(hmc_sections *f, const hmc_sections *c)
{
    size_t m = 0;
    for (size_t i = 0; i < f->n; i++) {
        int replaced = 0;
        for (size_t j = 0; j < c->n; j++)
            replaced |= f->s[i].id == c->s[j].id;
        if (replaced)
            free((void *)f->s[i].body);
        else
            f->s[m++] = f->s[i];
    }
    f->n = m;
    for (size_t j = 0; j < c->n; j++) {
        if (f->n == HMC_MAX_SECTIONS)
            return HMC_EINVAL;
        uint8_t *b = malloc(c->s[j].length ? c->s[j].length : 1);
        if (!b)
            return HMC_ENOMEM;
        memcpy(b, c->s[j].body, c->s[j].length);
        f->s[f->n] = c->s[j];
        f->s[f->n].body = b;
        f->n++;
    }
    return HMC_OK;
}

void hmc_sections_free(hmc_sections *s)
{
    for (size_t i = 0; i < s->n; i++)
        free((void *)s->s[i].body);
    s->n = 0;
}

static int layout_fields(const hmc_section *s, hcc_field *out, size_t *n)
{
    if (s->length < 8)
        return HMC_DAMAGED;
    size_t count = s->body[4];
    if (s->length < 8 + 32 * count)
        return HMC_DAMAGED;
    if (count > HCC_PAYLOAD)
        return HMC_EINVAL;
    for (size_t i = 0; i < count; i++) {
        const uint8_t *f = s->body + 8 + 32 * i;
        hcc_field h = {f[0], f[1], (uint8_t)(f[2] & 1), (uint8_t)((f[2] >> 1) & 1), f[3], f[5]};
        out[i] = h;
    }
    *n = count;
    return HMC_OK;
}

int hmc_hcc_layout(const hmc_sections *s, hcc_field *out, size_t *n)
{
    const hmc_section *unit = NULL, *lay[HMC_MAX_SECTIONS];
    size_t nlay = 0;
    for (size_t i = 0; i < s->n; i++) {
        const hmc_section *x = &s->s[i];
        if (x->id == HMC_LAYOUT) {
            if (x->length < 8)
                return HMC_DAMAGED;
            int seen = 0;
            for (size_t j = 0; j < nlay; j++)
                seen |= lay[j]->body[0] == x->body[0] && lay[j]->body[1] == x->body[1];
            int e;
            hcc_field tmp[HCC_PAYLOAD];
            size_t k;
            if ((e = layout_fields(x, tmp, &k)))    /* a damaged one is damaged wherever it is */
                return e;
            if (!seen)
                lay[nlay++] = x;
        } else if (x->id == HMC_UNIT && !unit) {
            if (x->length < 11)
                return HMC_DAMAGED;
            unit = x;
        }
    }
    hcc_field fields[4][HCC_PAYLOAD];
    size_t counts[4] = {0};
    if (!unit) {
        *n = hcc_compose_layout(NULL, 0, out);
        return HMC_OK;
    }
    const uint8_t *pos = unit->body + 7;
    if (pos[0] | pos[1] | pos[2] | pos[3]) {
        const hcc_field *ptr[4] = {NULL, NULL, NULL, NULL};
        for (int p = 0; p < 4; p++) {
            if (!pos[p])
                continue;
            for (size_t j = 0; j < nlay; j++)
                if (lay[j]->body[0] == 2 && lay[j]->body[1] == pos[p] >> 4) {
                    layout_fields(lay[j], fields[p], &counts[p]);
                    ptr[p] = fields[p];
                }
        }
        *n = hcc_argus_layout(ptr, counts, 4, out);
        return HMC_OK;
    }
    for (size_t j = 0; j < nlay; j++)
        if (lay[j]->body[0] == unit->body[0] && lay[j]->body[1] == unit->body[1])
            layout_fields(lay[j], fields[0], &counts[0]);
    *n = hcc_compose_layout(fields[0], counts[0], out);
    return HMC_OK;
}

/* ------------------------------------------------------------------ the header */

size_t hmc_header_build(const hmc_header *h, uint8_t *out)
{
    size_t length = 20 + h->sections_len + 4;
    memcpy(out, "HMC", 4);
    out[4] = HMC_VERSION;
    out[5] = h->address;
    put16(out + 6, h->sequence);
    put32(out + 8, (uint32_t)length);
    put32(out + 12, h->first_second);
    out[16] = (uint8_t)h->station;
    put16(out + 17, h->station >> 8);
    out[19] = h->size;
    memcpy(out + 20, h->sections, h->sections_len);
    put32(out + 20 + h->sections_len, hmc_crc32(out, 20 + h->sections_len));
    return length;
}

int hmc_header_parse(const uint8_t *b, size_t len, hmc_header *h)
{
    if (len < 24 || memcmp(b, "HMC", 4) || b[4] != HMC_VERSION)
        return HMC_DAMAGED;
    uint32_t length = get32(b + 8);
    if (length < 24 || length > len || get32(b + length - 4) != hmc_crc32(b, length - 4))
        return HMC_DAMAGED;
    if (b[19] > 7)
        return HMC_DAMAGED;
    hmc_sections tmp;
    if (hmc_sections_parse(b + 20, length - 24, &tmp))
        return HMC_DAMAGED;
    h->address = b[5];
    h->sequence = (uint16_t)get16(b + 6);
    h->length = length;
    h->first_second = get32(b + 12);
    h->station = b[16] | get16(b + 17) << 8;
    h->size = b[19];
    h->sections = b + 20;
    h->sections_len = length - 24;
    return HMC_OK;
}

/* ------------------------------------------------------------------ segments and configs */

static int segment(uint32_t first, uint32_t last, uint8_t codec, const uint8_t *body, size_t blen,
                   uint8_t **out, size_t *len)
{
    size_t n = 20 + blen + 4;
    uint8_t *b = malloc(n);
    if (!b)
        return HMC_ENOMEM;
    memcpy(b, "HSEG", 4);
    put32(b + 4, (uint32_t)n);
    put32(b + 8, first);
    put32(b + 12, last);
    b[16] = codec;
    b[17] = b[18] = b[19] = 0;
    memcpy(b + 20, body, blen);
    put32(b + 20 + blen, hmc_crc32(b, 20 + blen));
    *out = b;
    *len = n;
    return HMC_OK;
}

int hmc_segment_build(const hcc_record *recs, size_t n, const hcc_field *layout, size_t nl,
                      uint8_t **out, size_t *len)
{
    if (n == 0)
        return HMC_EINVAL;
    size_t rlen = 0;
    uint32_t first = recs[0].second, last = recs[0].second;
    for (size_t i = 0; i < n; i++) {
        const hcc_record *r = &recs[i];
        if (r->form > 1 || (r->form == 0 && r->length != HCC_PAYLOAD)
            || (r->form == 1 && (r->length < 2 || r->length > 2 + HMC_MAX_BLOCK
                                 || r->length != 2u + r->data[1])))
            return HMC_EINVAL;
        rlen += 9 + r->length;
        if (r->second < first) first = r->second;
        if (r->second > last) last = r->second;
    }
    uint8_t *raw = malloc(rlen), *p = raw, *coded = NULL;
    size_t clen = 0;
    if (!raw)
        return HMC_ENOMEM;
    for (size_t i = 0; i < n; i++) {
        put32(p, recs[i].second);
        p[4] = recs[i].frame;
        p[5] = recs[i].kind;
        p[6] = recs[i].status;
        p[7] = recs[i].form;
        p[8] = (uint8_t)recs[i].length;
        memcpy(p + 9, recs[i].data, recs[i].length);
        p += 9 + recs[i].length;
    }
    int e = hcc_encode(recs, n, layout, nl, &coded, &clen);
    if (e == HCC_ENOMEM) {
        free(raw);
        return HMC_ENOMEM;
    }
    if (e == HCC_OK && clen < rlen)
        e = segment(first, last, HMC_HCC, coded, clen, out, len);
    else
        e = segment(first, last, HMC_RAW, raw, rlen, out, len);
    free(raw);
    hcc_free(coded);
    return e;
}

int hmc_event_segment_build(const hmc_event *lines, size_t n, uint8_t **out, size_t *len)
{
    if (n == 0)
        return HMC_EINVAL;
    uint8_t *body = malloc(12 * n);
    uint32_t first = lines[0].second, last = lines[0].second;
    if (!body)
        return HMC_ENOMEM;
    for (size_t i = 0; i < n; i++) {
        uint8_t *p = body + 12 * i;
        put32(p, lines[i].second);
        p[4] = lines[i].source;
        p[5] = lines[i].code;
        put16(p + 6, lines[i].detail);
        put32(p + 8, lines[i].value);
        if (lines[i].second < first) first = lines[i].second;
        if (lines[i].second > last) last = lines[i].second;
    }
    int e = segment(first, last, HMC_RAW, body, 12 * n, out, len);
    free(body);
    return e;
}

int hmc_config_build(uint32_t from_second, const uint8_t *sections, size_t slen, uint8_t **out,
                     size_t *len)
{
    size_t n = 16 + slen + 4;
    uint8_t *b = malloc(n);
    if (!b)
        return HMC_ENOMEM;
    memcpy(b, "HCFG", 4);
    put32(b + 4, (uint32_t)n);
    put32(b + 8, from_second);
    put32(b + 12, 0);
    memcpy(b + 16, sections, slen);
    put32(b + 16 + slen, hmc_crc32(b, 16 + slen));
    *out = b;
    *len = n;
    return HMC_OK;
}

void hmc_entry_build(const hmc_entry *e, uint8_t out[16])
{
    put32(out, e->second);
    put32(out + 4, e->offset);
    put32(out + 8, e->length);
    out[12] = e->type;
    out[13] = 0;
    put16(out + 14, hmc_crc16(out, 14));
}

int hmc_entry_parse(const uint8_t b[16], hmc_entry *e)
{
    if (get16(b + 14) != hmc_crc16(b, 14) || (b[12] != HMC_SEG && b[12] != HMC_CFG))
        return HMC_DAMAGED;
    e->second = get32(b);
    e->offset = get32(b + 4);
    e->length = get32(b + 8);
    e->type = b[12];
    return HMC_OK;
}

int hmc_object_parse(const uint8_t *b, size_t len, hmc_object *o)
{
    if (len < 20)
        return HMC_DAMAGED;
    int seg = !memcmp(b, "HSEG", 4), cfg = !memcmp(b, "HCFG", 4);
    if (!seg && !cfg)
        return HMC_DAMAGED;
    uint32_t length = get32(b + 4);
    if (length != len || get32(b + length - 4) != hmc_crc32(b, length - 4))
        return HMC_DAMAGED;
    memset(o, 0, sizeof *o);
    if (seg) {
        if (length < 24)
            return HMC_DAMAGED;
        o->type = HMC_SEG;
        o->first = get32(b + 8);
        o->last = get32(b + 12);
        o->codec = b[16];
        o->body = b + 20;
        o->body_len = length - 24;
        return HMC_OK;
    }
    hmc_sections tmp;
    o->type = HMC_CFG;
    o->from = get32(b + 8);
    o->body = b + 16;
    o->body_len = length - 20;
    return hmc_sections_parse(o->body, o->body_len, &tmp) ? HMC_DAMAGED : HMC_OK;
}

int hmc_segment_records(const hmc_object *o, const hcc_field *layout, size_t nl,
                        hcc_record **recs, size_t *n)
{
    *recs = NULL;
    *n = 0;
    if (o->codec == HMC_HCC) {
        int e = hcc_decode(o->body, o->body_len, layout, nl, recs, n);
        return e == HCC_OK ? HMC_OK : e == HCC_ENOMEM ? HMC_ENOMEM : e == HCC_EINVAL ? HMC_EINVAL
                                                                                    : HMC_DAMAGED;
    }
    if (o->codec != HMC_RAW)
        return HMC_UNKNOWN;
    size_t count = 0;
    for (size_t i = 0; i < o->body_len; count++) {
        if (o->body_len - i < 9 || o->body[i + 8] > o->body_len - i - 9)
            return HMC_DAMAGED;
        i += 9 + o->body[i + 8];
    }
    hcc_record *r = malloc(sizeof(hcc_record) * (count ? count : 1));
    if (!r)
        return HMC_ENOMEM;
    for (size_t i = 0, k = 0; k < count; k++) {
        const uint8_t *p = o->body + i;
        r[k].second = get32(p);
        r[k].frame = p[4];
        r[k].kind = p[5];
        r[k].status = p[6];
        r[k].form = p[7];
        r[k].length = p[8];
        r[k].data = p + 9;
        i += 9 + p[8];
    }
    *recs = r;
    *n = count;
    return HMC_OK;
}

int hmc_events_parse(const hmc_object *o, hmc_event **lines, size_t *n)
{
    *lines = NULL;
    *n = 0;
    if (o->codec != HMC_RAW)
        return HMC_UNKNOWN;
    if (o->body_len % 12)
        return HMC_DAMAGED;
    size_t count = o->body_len / 12;
    hmc_event *l = malloc(sizeof(hmc_event) * (count ? count : 1));
    if (!l)
        return HMC_ENOMEM;
    for (size_t i = 0; i < count; i++) {
        const uint8_t *p = o->body + 12 * i;
        l[i].second = get32(p);
        l[i].source = p[4];
        l[i].code = p[5];
        l[i].detail = (uint16_t)get16(p + 6);
        l[i].value = get32(p + 8);
    }
    *lines = l;
    *n = count;
    return HMC_OK;
}

/* ------------------------------------------------------------------ a file in memory */

int hmc_writer_open(hmc_writer *w, const hmc_header *h)
{
    hmc_sections hs;
    memset(w, 0, sizeof *w);
    if (h->size > 7 || 24 + h->sections_len > ((size_t)HMC_MB << h->size)
        || hmc_sections_parse(h->sections, h->sections_len, &hs))
        return HMC_EINVAL;
    w->size = (size_t)HMC_MB << h->size;
    w->image = calloc(1, w->size);
    if (!w->image)
        return HMC_ENOMEM;
    w->end = hmc_header_build(h, w->image);
    w->address = h->address;
    return hmc_sections_apply(&w->in_force, &hs);
}

int hmc_writer_append(hmc_writer *w, const uint8_t *obj, size_t len, uint8_t type, uint32_t second)
{
    if (second < w->last)
        return HMC_EINVAL;
    size_t desc = w->size - 16 * (w->entries + 1);
    uint8_t *f = malloc(len + len / 254 + 2);
    if (!f)
        return HMC_ENOMEM;
    size_t flen = hmc_cobs_encode(obj, len, f);
    f[flen++] = 0;
    if (w->end + flen > desc) {
        free(f);
        return HMC_FULL;
    }
    memcpy(w->image + w->end, f, flen);
    hmc_entry e = {second, (uint32_t)w->end, (uint32_t)(flen - 1), type};
    hmc_entry_build(&e, w->image + desc);
    w->end += flen;
    w->entries++;
    w->last = second;
    free(f);
    return HMC_OK;
}

int hmc_writer_segment(hmc_writer *w, const hcc_record *recs, size_t n)
{
    hcc_field layout[HCC_MAX_LAYOUT];
    size_t nl;
    uint8_t *seg;
    size_t len;
    int e = hmc_hcc_layout(&w->in_force, layout, &nl);
    if (e)
        return e;
    if ((e = hmc_segment_build(recs, n, layout, nl, &seg, &len)))
        return e;
    uint32_t last = recs[0].second;
    for (size_t i = 1; i < n; i++)
        if (recs[i].second > last)
            last = recs[i].second;
    e = hmc_writer_append(w, seg, len, HMC_SEG, last);
    free(seg);
    return e;
}

int hmc_writer_events(hmc_writer *w, const hmc_event *lines, size_t n)
{
    uint8_t *seg;
    size_t len;
    int e = hmc_event_segment_build(lines, n, &seg, &len);
    if (e)
        return e;
    uint32_t last = lines[0].second;
    for (size_t i = 1; i < n; i++)
        if (lines[i].second > last)
            last = lines[i].second;
    e = hmc_writer_append(w, seg, len, HMC_SEG, last);
    free(seg);
    return e;
}

int hmc_writer_config(hmc_writer *w, uint32_t from_second, const uint8_t *sections, size_t slen)
{
    hmc_sections cs;
    uint8_t *cfg;
    size_t len;
    int e = hmc_sections_parse(sections, slen, &cs);
    if (e)
        return HMC_EINVAL;
    if ((e = hmc_config_build(from_second, sections, slen, &cfg, &len)))
        return e;
    e = hmc_writer_append(w, cfg, len, HMC_CFG, from_second);
    free(cfg);
    return e ? e : hmc_sections_apply(&w->in_force, &cs);
}

void hmc_writer_close(hmc_writer *w)
{
    free(w->image);
    hmc_sections_free(&w->in_force);
    memset(w, 0, sizeof *w);
}

int hmc_reader_open(hmc_reader *r, const uint8_t *image, size_t len)
{
    memset(r, 0, sizeof *r);
    if (hmc_header_parse(image, len, &r->h) || len != ((size_t)HMC_MB << r->h.size))
        return HMC_DAMAGED;
    r->image = image;
    r->size = len;
    size_t lo = 0, hi = (len - r->h.length) / 16;
    while (lo < hi) {           /* a binary search for the last good entry */
        size_t mid = lo + (hi - lo) / 2;
        hmc_entry e;
        if (hmc_entry_parse(image + len - 16 * (mid + 1), &e))
            hi = mid;
        else
            lo = mid + 1;
    }
    r->entries = lo;
    return HMC_OK;
}

int hmc_reader_entry(const hmc_reader *r, size_t i, hmc_entry *e)
{
    if (i >= r->entries)
        return HMC_EINVAL;
    return hmc_entry_parse(r->image + r->size - 16 * (i + 1), e);
}

static uint32_t second_at(const hmc_reader *r, size_t i)
{
    hmc_entry e;
    for (i++; i-- > 0;)         /* a damaged entry takes the last good one's second, 0 with none */
        if (!hmc_reader_entry(r, i, &e))
            return e.second;
    return 0;
}

size_t hmc_reader_find(const hmc_reader *r, uint32_t second)
{
    size_t lo = 0, hi = r->entries;
    while (lo < hi) {
        size_t mid = lo + (hi - lo) / 2;
        if (second_at(r, mid) < second)
            lo = mid + 1;
        else
            hi = mid;
    }
    return lo;
}

int hmc_reader_object(const hmc_reader *r, size_t i, uint8_t **buf, hmc_object *o)
{
    hmc_entry e;
    size_t len;
    *buf = NULL;
    if (hmc_reader_entry(r, i, &e))
        return HMC_DAMAGED;
    if ((uint64_t)e.offset + e.length >= r->size || r->image[e.offset + e.length] != 0)
        return HMC_DAMAGED;
    *buf = malloc(e.length ? e.length : 1);
    if (!*buf)
        return HMC_ENOMEM;
    if (hmc_cobs_decode(r->image + e.offset, e.length, *buf, &len)
        || hmc_object_parse(*buf, len, o) || o->type != e.type)
        return HMC_DAMAGED;
    return HMC_OK;
}

void hmc_path(uint8_t address, uint32_t first_second, char out[32])
{
    int64_t days = first_second / 86400, s = first_second % 86400;
    int64_t z = days + 719468, era = z / 146097, doe = z - era * 146097;
    int64_t yoe = (doe - doe / 1460 + doe / 36524 - doe / 146096) / 365;
    int64_t doy = doe - (365 * yoe + yoe / 4 - yoe / 100), mp = (5 * doy + 2) / 153;
    int64_t d = doy - (153 * mp + 2) / 5 + 1, m = mp < 10 ? mp + 3 : mp - 9;
    int64_t y = yoe + era * 400 + (m <= 2);
    snprintf(out, 32, "%02X/%04d%02d%02d/%02d%02d%02d.hmc", address, (int)y, (int)m, (int)d,
             (int)(s / 3600), (int)(s / 60 % 60), (int)(s % 60));
}

/* ------------------------------------------------------------------ the rules */

void hmc_selector_init(hmc_selector *s, const hmc_rule *r, int blocks)
{
    memset(s, 0, sizeof *s);
    s->rule = *r;
    s->blocks = blocks;
}

void hmc_selector_free(hmc_selector *s)
{
    free(s->keys);
    s->keys = NULL;
    s->nkeys = 0;
}

int hmc_split_blocks(const uint8_t *payload, uint8_t offsets[16])
{
    unsigned i = 0;
    int n = 0;
    while (i < HCC_PAYLOAD && payload[i] != 0) {
        if (i + 2 > HCC_PAYLOAD || i + 2 + payload[i + 1] > HCC_PAYLOAD)
            return -1;
        offsets[n++] = (uint8_t)i;
        i += 2u + payload[i + 1];
    }
    return n;
}

static int keep(hmc_selector *s, const hcc_record *r)
{
    const hmc_rule *rule = &s->rule;
    if (rule->mode == HMC_NONE)
        return 0;
    int block = r->form == 1, k, store = 1;
    hmc_block_key *key = NULL;
    if (block) {
        for (size_t i = 0; i < s->nkeys; i++)
            if (s->keys[i].bytes[0] == r->data[0] && s->keys[i].bytes[1] == r->data[1])
                key = &s->keys[i];
        if (!key && s->nkeys >= HMC_MAX_BLOCK_KEYS)
            store = 0;
    }
    if (!store) {
        k = 1;
    } else if (!s->have_state || r->kind != s->kind || r->status != s->status
               || (r->kind >= 5 && r->kind <= 8) || rule->mode == HMC_ALL) {
        k = 1;
    } else if (rule->mode == HMC_CHANGE) {
        int have = block ? key != NULL : s->have_frame;
        const uint8_t *prev = block ? (key ? key->bytes : NULL) : s->frame;
        uint32_t prev_sec = block ? (key ? key->second : 0) : s->frame_second;
        k = !have || memcmp(prev, r->data, r->length)
            || (rule->s && (int64_t)r->second - prev_sec >= rule->s);
    } else if (rule->mode == HMC_NONZERO) {
        int nz = 0;
        for (unsigned i = 0; i < r->length; i++)
            nz |= r->data[i];
        k = nz || (rule->s && (!s->have_nzero || (int64_t)r->second - s->nzero >= rule->s));
    } else if (rule->mode == HMC_DECIMATE) {
        k = rule->n <= 1 || r->frame % rule->n == 0;
    } else if (rule->mode == HMC_INTERVAL) {
        uint32_t iv = rule->s ? r->second / rule->s : 0;
        k = rule->s == 0 || (block ? !s->have_mod_iv[r->data[0]] || s->mod_iv[r->data[0]] != iv
                                   : !s->have_frame_iv || s->frame_iv != iv);
    } else {
        k = 0;
    }
    if (!k)
        return 0;
    s->have_state = 1;
    s->kind = r->kind;
    s->status = r->status;
    s->have_nzero = 1;
    s->nzero = r->second;
    if (store) {
        if (block) {
            if (!key) {
                if (!s->keys)
                    s->keys = malloc(sizeof(hmc_block_key) * HMC_MAX_BLOCK_KEYS);
                if (s->keys)
                    key = &s->keys[s->nkeys++];
            }
            if (key) {
                memcpy(key->bytes, r->data, r->length);
                key->second = r->second;
            }
        } else {
            s->have_frame = 1;
            memcpy(s->frame, r->data, HCC_PAYLOAD);
            s->frame_second = r->second;
        }
    }
    if (rule->s) {
        if (block) {
            s->have_mod_iv[r->data[0]] = 1;
            s->mod_iv[r->data[0]] = r->second / rule->s;
        } else {
            s->have_frame_iv = 1;
            s->frame_iv = r->second / rule->s;
        }
    }
    return 1;
}

size_t hmc_select_frame(hmc_selector *s, uint32_t second, uint8_t frame, uint8_t kind,
                        uint8_t status, const uint8_t *payload, hcc_record out[16])
{
    hcc_record r = {second, frame, kind, status, 0, HCC_PAYLOAD, payload};
    size_t m = 0;
    if (s->blocks && kind == 0) {
        uint8_t off[16];
        int n = hmc_split_blocks(payload, off);
        if (n > 0) {
            for (int i = 0; i < n; i++) {
                hcc_record b = {second, frame, kind, status, 1,
                                (uint16_t)(2u + payload[off[i] + 1]), payload + off[i]};
                if (keep(s, &b))
                    out[m++] = b;
            }
            return m;
        }
    }
    if (keep(s, &r))
        out[m++] = r;
    return m;
}

int hmc_select_record(hmc_selector *s, const hcc_record *r)
{
    if (r->form == 1 && s->rule.nmodules) {
        int named = 0;
        for (unsigned i = 0; i < s->rule.nmodules; i++)
            named |= s->rule.modules[i] == r->data[0];
        if (!named)
            return 0;
    }
    return keep(s, r);
}

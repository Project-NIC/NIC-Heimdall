/* MLF — Heimdall's profile, the C reference; see hmlf.h. */

#include "hmlf.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* ------------------------------------------------------------------ little-endian */

static void put16(uint8_t *p, uint32_t v) { p[0] = (uint8_t)v; p[1] = (uint8_t)(v >> 8); }
static void put32(uint8_t *p, uint32_t v) { put16(p, v & 0xFFFF); put16(p + 2, v >> 16); }
static uint32_t get16(const uint8_t *p) { return (uint32_t)p[0] | (uint32_t)p[1] << 8; }
static uint32_t get32(const uint8_t *p) { return get16(p) | get16(p + 2) << 16; }

static int layout_fields(const mlf_section *s, hnrc_field *out, size_t *n)
{
    if (s->length < 8)
        return MLF_DAMAGED;
    size_t count = s->body[4];
    if (s->length < 8 + 32 * count)
        return MLF_DAMAGED;
    if (count > HNRC_PAYLOAD)
        return MLF_EINVAL;
    for (size_t i = 0; i < count; i++) {
        const uint8_t *f = s->body + 8 + 32 * i;
        hnrc_field h = {f[0], f[1], (uint8_t)(f[2] & 1), (uint8_t)((f[2] >> 1) & 1), f[3], f[5]};
        out[i] = h;
    }
    *n = count;
    return MLF_OK;
}

int hmlf_nrc_layout(const mlf_sections *s, hnrc_field *out, size_t *n)
{
    const mlf_section *unit = NULL, *lay[MLF_MAX_SECTIONS];
    size_t nlay = 0;
    for (size_t i = 0; i < s->n; i++) {
        const mlf_section *x = &s->s[i];
        if (x->id == HMLF_LAYOUT) {
            if (x->length < 8)
                return MLF_DAMAGED;
            int seen = 0;
            for (size_t j = 0; j < nlay; j++)
                seen |= lay[j]->body[0] == x->body[0] && lay[j]->body[1] == x->body[1];
            int e;
            hnrc_field tmp[HNRC_PAYLOAD];
            size_t k;
            if ((e = layout_fields(x, tmp, &k)))    /* a damaged one is damaged wherever it is */
                return e;
            if (!seen)
                lay[nlay++] = x;
        } else if (x->id == HMLF_UNIT && !unit) {
            if (x->length < 11)
                return MLF_DAMAGED;
            unit = x;
        }
    }
    hnrc_field fields[4][HNRC_PAYLOAD];
    size_t counts[4] = {0};
    if (!unit) {
        *n = hnrc_compose_layout(NULL, 0, out);
        return MLF_OK;
    }
    const uint8_t *pos = unit->body + 7;
    if (pos[0] | pos[1] | pos[2] | pos[3]) {
        const hnrc_field *ptr[4] = {NULL, NULL, NULL, NULL};
        for (int p = 0; p < 4; p++) {
            if (!pos[p])
                continue;
            for (size_t j = 0; j < nlay; j++)
                if (lay[j]->body[0] == 2 && lay[j]->body[1] == pos[p] >> 4) {
                    layout_fields(lay[j], fields[p], &counts[p]);
                    ptr[p] = fields[p];
                }
        }
        *n = hnrc_argus_layout(ptr, counts, 4, out);
        return MLF_OK;
    }
    for (size_t j = 0; j < nlay; j++)
        if (lay[j]->body[0] == unit->body[0] && lay[j]->body[1] == unit->body[1])
            layout_fields(lay[j], fields[0], &counts[0]);
    *n = hnrc_compose_layout(fields[0], counts[0], out);
    return MLF_OK;
}

/* ------------------------------------------------------------------ the header */

void hmlf_to_core(const hmlf_header *h, mlf_header *c)
{
    c->sequence = h->sequence;
    c->first_second = h->first_second;
    c->source = (h->station & 0xFFFFFFu) | (uint32_t)h->address << 24;
    c->size = h->size;
    c->length = h->length;
    c->sections = h->sections;
    c->sections_len = h->sections_len;
    c->letter = HMLF_LETTER;
}

void hmlf_from_core(const mlf_header *c, hmlf_header *h)
{
    h->address = (uint8_t)(c->source >> 24);
    h->sequence = c->sequence;
    h->first_second = c->first_second;
    h->station = c->source & 0xFFFFFFu;
    h->size = c->size;
    h->length = c->length;
    h->sections = c->sections;
    h->sections_len = c->sections_len;
}

size_t hmlf_header_build(const hmlf_header *h, uint8_t *out)
{
    mlf_header c;
    hmlf_to_core(h, &c);
    return mlf_header_build(&c, out);
}

int hmlf_header_parse(const uint8_t *b, size_t len, hmlf_header *h)
{
    mlf_header c;
    int e = mlf_header_parse(b, len, &c);
    if (!e)
        hmlf_from_core(&c, h);
    return e;
}

int hmlf_writer_open(mlf_writer *w, const hmlf_header *h)
{
    mlf_header c;
    hmlf_to_core(h, &c);
    return mlf_writer_open(w, &c);
}

/* ------------------------------------------------------------------ segments */

int hmlf_segment_build(const hnrc_record *recs, size_t n, const hnrc_field *layout, size_t nl,
                      uint8_t **out, size_t *len)
{
    if (n == 0)
        return MLF_EINVAL;
    size_t rlen = 0;
    uint32_t first = recs[0].second, last = recs[0].second;
    for (size_t i = 0; i < n; i++) {
        const hnrc_record *r = &recs[i];
        if (r->form > 1 || (r->form == 0 && r->length != HNRC_PAYLOAD)
            || (r->form == 1 && (r->length < 2 || r->length > 2 + HMLF_MAX_BLOCK
                                 || r->length != 2u + r->data[1])))
            return MLF_EINVAL;
        rlen += 9 + r->length;
        if (r->second < first) first = r->second;
        if (r->second > last) last = r->second;
    }
    uint8_t *raw = malloc(rlen), *p = raw, *coded = NULL;
    size_t clen = 0;
    if (!raw)
        return MLF_ENOMEM;
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
    int e = hnrc_encode(recs, n, layout, nl, &coded, &clen);
    if (e == HNRC_ENOMEM) {
        free(raw);
        return MLF_ENOMEM;
    }
    if (e == HNRC_OK && clen < rlen)
        e = mlf_segment_build(first, last, MLF_NRC, coded, clen, out, len);
    else
        e = mlf_segment_build(first, last, MLF_RAW, raw, rlen, out, len);
    free(raw);
    hnrc_free(coded);
    return e;
}

int hmlf_event_segment_build(const hmlf_event *lines, size_t n, uint8_t **out, size_t *len)
{
    if (n == 0)
        return MLF_EINVAL;
    uint8_t *body = malloc(12 * n);
    uint32_t first = lines[0].second, last = lines[0].second;
    if (!body)
        return MLF_ENOMEM;
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
    int e = mlf_segment_build(first, last, MLF_RAW, body, 12 * n, out, len);
    free(body);
    return e;
}

int hmlf_segment_records(const mlf_object *o, const hnrc_field *layout, size_t nl,
                        hnrc_record **recs, size_t *n)
{
    *recs = NULL;
    *n = 0;
    if (o->codec == MLF_NRC) {
        int e = hnrc_decode(o->body, o->body_len, layout, nl, recs, n);
        return e == HNRC_OK ? MLF_OK : e == HNRC_ENOMEM ? MLF_ENOMEM : e == HNRC_EINVAL ? MLF_EINVAL
                                                                                    : MLF_DAMAGED;
    }
    if (o->codec != MLF_RAW)
        return MLF_UNKNOWN;
    size_t count = 0;
    for (size_t i = 0; i < o->body_len; count++) {
        if (o->body_len - i < 9 || o->body[i + 8] > o->body_len - i - 9)
            return MLF_DAMAGED;
        i += 9 + o->body[i + 8];
    }
    hnrc_record *r = malloc(sizeof(hnrc_record) * (count ? count : 1));
    if (!r)
        return MLF_ENOMEM;
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
    return MLF_OK;
}

int hmlf_events_parse(const mlf_object *o, hmlf_event **lines, size_t *n)
{
    *lines = NULL;
    *n = 0;
    if (o->codec != MLF_RAW)
        return MLF_UNKNOWN;
    if (o->body_len % 12)
        return MLF_DAMAGED;
    size_t count = o->body_len / 12;
    hmlf_event *l = malloc(sizeof(hmlf_event) * (count ? count : 1));
    if (!l)
        return MLF_ENOMEM;
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
    return MLF_OK;
}

int hmlf_writer_segment(mlf_writer *w, const hnrc_record *recs, size_t n)
{
    hnrc_field layout[HNRC_MAX_LAYOUT];
    size_t nl;
    uint8_t *seg;
    size_t len;
    int e = hmlf_nrc_layout(&w->in_force, layout, &nl);
    if (e)
        return e;
    if ((e = hmlf_segment_build(recs, n, layout, nl, &seg, &len)))
        return e;
    uint32_t last = recs[0].second;
    for (size_t i = 1; i < n; i++)
        if (recs[i].second > last)
            last = recs[i].second;
    e = mlf_writer_append(w, seg, len, MLF_SEG, last);
    free(seg);
    return e;
}

int hmlf_writer_events(mlf_writer *w, const hmlf_event *lines, size_t n)
{
    uint8_t *seg;
    size_t len;
    int e = hmlf_event_segment_build(lines, n, &seg, &len);
    if (e)
        return e;
    uint32_t last = lines[0].second;
    for (size_t i = 1; i < n; i++)
        if (lines[i].second > last)
            last = lines[i].second;
    e = mlf_writer_append(w, seg, len, MLF_SEG, last);
    free(seg);
    return e;
}

void hmlf_path(uint8_t address, uint32_t first_second, char out[32])
{
    int64_t days = first_second / 86400, s = first_second % 86400;
    int64_t z = days + 719468, era = z / 146097, doe = z - era * 146097;
    int64_t yoe = (doe - doe / 1460 + doe / 36524 - doe / 146096) / 365;
    int64_t doy = doe - (365 * yoe + yoe / 4 - yoe / 100), mp = (5 * doy + 2) / 153;
    int64_t d = doy - (153 * mp + 2) / 5 + 1, m = mp < 10 ? mp + 3 : mp - 9;
    int64_t y = yoe + era * 400 + (m <= 2);
    snprintf(out, 32, "%02X/%04d%02d%02d/%02d%02d%02d.mlf", address, (int)y, (int)m, (int)d,
             (int)(s / 3600), (int)(s / 60 % 60), (int)(s % 60));
}

/* ------------------------------------------------------------------ the rules */

void hmlf_selector_init(hmlf_selector *s, const hmlf_rule *r, int blocks)
{
    memset(s, 0, sizeof *s);
    s->rule = *r;
    s->blocks = blocks;
}

void hmlf_selector_free(hmlf_selector *s)
{
    free(s->keys);
    s->keys = NULL;
    s->nkeys = 0;
}

int hmlf_split_blocks(const uint8_t *payload, uint8_t offsets[16])
{
    unsigned i = 0;
    int n = 0;
    while (i < HNRC_PAYLOAD && payload[i] != 0) {
        if (i + 2 > HNRC_PAYLOAD || i + 2 + payload[i + 1] > HNRC_PAYLOAD)
            return -1;
        offsets[n++] = (uint8_t)i;
        i += 2u + payload[i + 1];
    }
    return n;
}

static int keep(hmlf_selector *s, const hnrc_record *r)
{
    const hmlf_rule *rule = &s->rule;
    if (rule->mode == HMLF_NONE)
        return 0;
    int block = r->form == 1, k, store = 1;
    hmlf_block_key *key = NULL;
    if (block) {
        for (size_t i = 0; i < s->nkeys; i++)
            if (s->keys[i].bytes[0] == r->data[0] && s->keys[i].bytes[1] == r->data[1])
                key = &s->keys[i];
        if (!key && s->nkeys >= HMLF_MAX_BLOCK_KEYS)
            store = 0;
    }
    if (!store) {
        k = 1;
    } else if (!s->have_state || r->kind != s->kind || r->status != s->status
               || (r->kind >= 5 && r->kind <= 8) || rule->mode == HMLF_ALL) {
        k = 1;
    } else if (rule->mode == HMLF_CHANGE) {
        int have = block ? key != NULL : s->have_frame;
        const uint8_t *prev = block ? (key ? key->bytes : NULL) : s->frame;
        uint32_t prev_sec = block ? (key ? key->second : 0) : s->frame_second;
        k = !have || memcmp(prev, r->data, r->length)
            || (rule->s && (int64_t)r->second - prev_sec >= rule->s);
    } else if (rule->mode == HMLF_NONZERO) {
        int nz = 0;
        for (unsigned i = 0; i < r->length; i++)
            nz |= r->data[i];
        k = nz || (rule->s && (!s->have_nzero || (int64_t)r->second - s->nzero >= rule->s));
    } else if (rule->mode == HMLF_DECIMATE) {
        k = rule->n <= 1 || r->frame % rule->n == 0;
    } else if (rule->mode == HMLF_INTERVAL) {
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
                    s->keys = malloc(sizeof(hmlf_block_key) * HMLF_MAX_BLOCK_KEYS);
                if (s->keys)
                    key = &s->keys[s->nkeys++];
            }
            if (key) {
                memcpy(key->bytes, r->data, r->length);
                key->second = r->second;
            }
        } else {
            s->have_frame = 1;
            memcpy(s->frame, r->data, HNRC_PAYLOAD);
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

size_t hmlf_select_frame(hmlf_selector *s, uint32_t second, uint8_t frame, uint8_t kind,
                        uint8_t status, const uint8_t *payload, hnrc_record out[16])
{
    hnrc_record r = {second, frame, kind, status, 0, HNRC_PAYLOAD, payload};
    size_t m = 0;
    if (s->blocks && kind == 0) {
        uint8_t off[16];
        int n = hmlf_split_blocks(payload, off);
        if (n > 0) {
            for (int i = 0; i < n; i++) {
                hnrc_record b = {second, frame, kind, status, 1,
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

int hmlf_select_record(hmlf_selector *s, const hnrc_record *r)
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

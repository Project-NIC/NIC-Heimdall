/* The uplink session — the C reference; see session.h. */
#include "session.h"

#include <stdlib.h>
#include <string.h>

/* ------------------------------------------------------------------ SHA-256 (FIPS 180-4) */

typedef struct {
    uint32_t h[8];
    uint64_t len;
    uint8_t buf[64];
    size_t n;
} sha_t;

static const uint32_t K[64] = {
    0x428a2f98, 0x71374491, 0xb5c0fbcf, 0xe9b5dba5, 0x3956c25b, 0x59f111f1, 0x923f82a4, 0xab1c5ed5,
    0xd807aa98, 0x12835b01, 0x243185be, 0x550c7dc3, 0x72be5d74, 0x80deb1fe, 0x9bdc06a7, 0xc19bf174,
    0xe49b69c1, 0xefbe4786, 0x0fc19dc6, 0x240ca1cc, 0x2de92c6f, 0x4a7484aa, 0x5cb0a9dc, 0x76f988da,
    0x983e5152, 0xa831c66d, 0xb00327c8, 0xbf597fc7, 0xc6e00bf3, 0xd5a79147, 0x06ca6351, 0x14292967,
    0x27b70a85, 0x2e1b2138, 0x4d2c6dfc, 0x53380d13, 0x650a7354, 0x766a0abb, 0x81c2c92e, 0x92722c85,
    0xa2bfe8a1, 0xa81a664b, 0xc24b8b70, 0xc76c51a3, 0xd192e819, 0xd6990624, 0xf40e3585, 0x106aa070,
    0x19a4c116, 0x1e376c08, 0x2748774c, 0x34b0bcb5, 0x391c0cb3, 0x4ed8aa4a, 0x5b9cca4f, 0x682e6ff3,
    0x748f82ee, 0x78a5636f, 0x84c87814, 0x8cc70208, 0x90befffa, 0xa4506ceb, 0xbef9a3f7, 0xc67178f2};

#define ROR(x, n) (((x) >> (n)) | ((x) << (32 - (n))))

static void sha_block(sha_t *s, const uint8_t *p)
{
    uint32_t w[64], a, b, c, d, e, f, g, h;
    for (int i = 0; i < 16; i++)
        w[i] = (uint32_t)p[4 * i] << 24 | (uint32_t)p[4 * i + 1] << 16 | (uint32_t)p[4 * i + 2] << 8
               | p[4 * i + 3];
    for (int i = 16; i < 64; i++) {
        uint32_t s0 = ROR(w[i - 15], 7) ^ ROR(w[i - 15], 18) ^ (w[i - 15] >> 3);
        uint32_t s1 = ROR(w[i - 2], 17) ^ ROR(w[i - 2], 19) ^ (w[i - 2] >> 10);
        w[i] = w[i - 16] + s0 + w[i - 7] + s1;
    }
    a = s->h[0]; b = s->h[1]; c = s->h[2]; d = s->h[3];
    e = s->h[4]; f = s->h[5]; g = s->h[6]; h = s->h[7];
    for (int i = 0; i < 64; i++) {
        uint32_t t1 = h + (ROR(e, 6) ^ ROR(e, 11) ^ ROR(e, 25)) + ((e & f) ^ (~e & g)) + K[i] + w[i];
        uint32_t t2 = (ROR(a, 2) ^ ROR(a, 13) ^ ROR(a, 22)) + ((a & b) ^ (a & c) ^ (b & c));
        h = g; g = f; f = e; e = d + t1; d = c; c = b; b = a; a = t1 + t2;
    }
    s->h[0] += a; s->h[1] += b; s->h[2] += c; s->h[3] += d;
    s->h[4] += e; s->h[5] += f; s->h[6] += g; s->h[7] += h;
}

static void sha_init(sha_t *s)
{
    static const uint32_t h0[8] = {0x6a09e667, 0xbb67ae85, 0x3c6ef372, 0xa54ff53a,
                                   0x510e527f, 0x9b05688c, 0x1f83d9ab, 0x5be0cd19};
    memcpy(s->h, h0, sizeof h0);
    s->len = 0;
    s->n = 0;
}

static void sha_update(sha_t *s, const uint8_t *d, size_t n)
{
    s->len += n;
    while (n) {
        size_t k = 64 - s->n < n ? 64 - s->n : n;
        memcpy(s->buf + s->n, d, k);
        s->n += k;
        d += k;
        n -= k;
        if (s->n == 64) {
            sha_block(s, s->buf);
            s->n = 0;
        }
    }
}

static void sha_final(sha_t *s, uint8_t out[32])
{
    uint64_t bits = s->len * 8;
    uint8_t pad = 0x80, zero = 0, l[8];
    sha_update(s, &pad, 1);
    while (s->n != 56)
        sha_update(s, &zero, 1);
    for (int i = 0; i < 8; i++)
        l[i] = (uint8_t)(bits >> (56 - 8 * i));
    sha_update(s, l, 8);
    for (int i = 0; i < 8; i++) {
        out[4 * i] = (uint8_t)(s->h[i] >> 24);
        out[4 * i + 1] = (uint8_t)(s->h[i] >> 16);
        out[4 * i + 2] = (uint8_t)(s->h[i] >> 8);
        out[4 * i + 3] = (uint8_t)s->h[i];
    }
}

void ses_sha256(const uint8_t *d, size_t n, uint8_t out[32])
{
    sha_t s;
    sha_init(&s);
    sha_update(&s, d, n);
    sha_final(&s, out);
}

static void hmac_parts(const uint8_t *key, size_t klen, const uint8_t *const *parts,
                       const size_t *lens, size_t np, uint8_t out[32])
{
    uint8_t k[64] = {0}, pad[64], inner[32];
    sha_t s;
    if (klen > 64)
        ses_sha256(key, klen, k);
    else
        memcpy(k, key, klen);
    for (int i = 0; i < 64; i++)
        pad[i] = k[i] ^ 0x36;
    sha_init(&s);
    sha_update(&s, pad, 64);
    for (size_t i = 0; i < np; i++)
        sha_update(&s, parts[i], lens[i]);
    sha_final(&s, inner);
    for (int i = 0; i < 64; i++)
        pad[i] = k[i] ^ 0x5c;
    sha_init(&s);
    sha_update(&s, pad, 64);
    sha_update(&s, inner, 32);
    sha_final(&s, out);
}

void ses_hmac_sha256(const uint8_t *key, size_t klen, const uint8_t *d, size_t n, uint8_t out[32])
{
    hmac_parts(key, klen, &d, &n, 1, out);
}

void ses_tag(const uint8_t key[16], const uint8_t *const *parts, const size_t *lens, size_t np,
             uint8_t out[8])
{
    uint8_t full[32];
    hmac_parts(key, 16, parts, lens, np, full);
    memcpy(out, full, 8);
}

static int same_tag(const uint8_t *a, const uint8_t *b)   /* in constant time */
{
    uint8_t d = 0;
    for (int i = 0; i < 8; i++)
        d |= a[i] ^ b[i];
    return d == 0;
}

/* ------------------------------------------------------------------ framing and bytes */

static void put16(uint8_t *p, uint32_t v) { p[0] = (uint8_t)v; p[1] = (uint8_t)(v >> 8); }
static void put32(uint8_t *p, uint32_t v) { put16(p, v & 0xFFFF); put16(p + 2, v >> 16); }
static uint32_t get16(const uint8_t *p) { return (uint32_t)p[0] | (uint32_t)p[1] << 8; }
static uint32_t get32(const uint8_t *p) { return get16(p) | get16(p + 2) << 16; }

int ses_frame(const uint8_t *raw, size_t n, uint8_t **msg, size_t *len)
{
    uint8_t *m = malloc(n + n / 254 + 2);
    if (!m)
        return HMC_ENOMEM;
    size_t k = hmc_cobs_encode(raw, n, m);
    m[k++] = 0;
    *msg = m;
    *len = k;
    return HMC_OK;
}

int ses_unframe(const uint8_t *framed, size_t n, uint8_t **raw, size_t *rawlen)
{
    uint8_t *r = malloc(n ? n : 1);
    if (!r)
        return HMC_ENOMEM;
    if (hmc_cobs_decode(framed, n, r, rawlen) || *rawlen == 0) {
        free(r);
        return HMC_DAMAGED;
    }
    *raw = r;
    return HMC_OK;
}

static int finish(uint8_t *raw, size_t n, uint8_t **msg, size_t *len)
{
    int e = ses_frame(raw, n, msg, len);
    free(raw);
    return e;
}

/* ------------------------------------------------------------------ the messages */

int ses_hello(const uint8_t key[16], uint32_t station, uint32_t counter, const uint8_t nonce[8],
              int board, uint8_t tag[8], uint8_t **msg, size_t *len)
{
    uint8_t raw[26];
    raw[0] = SES_HELLO;
    raw[1] = 1;
    raw[2] = (uint8_t)station;
    put16(raw + 3, station >> 8);
    put32(raw + 5, counter);
    memcpy(raw + 9, nonce, 8);
    raw[17] = (uint8_t)(board != 0);
    const uint8_t *p = raw;
    size_t l = 18;
    ses_tag(key, &p, &l, 1, raw + 18);
    memcpy(tag, raw + 18, 8);
    return ses_frame(raw, 26, msg, len);
}

int ses_hello_parse(const uint8_t key[16], const uint8_t *body, size_t n, ses_hello_t *h)
{
    uint8_t t[8], type = SES_HELLO;
    if (n != 25 || body[0] != 1)
        return HMC_DAMAGED;
    const uint8_t *parts[2] = {&type, body};
    size_t lens[2] = {1, 17};
    ses_tag(key, parts, lens, 2, t);
    if (!same_tag(t, body + 17))
        return HMC_DAMAGED;
    h->station = body[1] | get16(body + 2) << 8;
    h->counter = get32(body + 4);
    memcpy(h->nonce, body + 8, 8);
    h->board = body[16] != 0;
    memcpy(h->tag, body + 17, 8);
    return HMC_OK;
}

int ses_marks(int type, const ses_mark *m, size_t n, uint8_t **msg, size_t *len)
{
    uint8_t *raw = malloc(3 + 7 * n);
    if (!raw)
        return HMC_ENOMEM;
    raw[0] = (uint8_t)type;
    put16(raw + 1, (uint32_t)n);
    for (size_t i = 0; i < n; i++) {
        raw[3 + 7 * i] = m[i].address;
        put16(raw + 4 + 7 * i, m[i].sequence);
        put32(raw + 6 + 7 * i, m[i].entry);
    }
    return finish(raw, 3 + 7 * n, msg, len);
}

int ses_marks_parse(const uint8_t *body, size_t n, ses_mark **marks, size_t *count)
{
    if (n < 2 || n != 2 + 7 * (size_t)get16(body))
        return HMC_DAMAGED;
    *count = get16(body);
    ses_mark *m = malloc(sizeof(ses_mark) * (*count ? *count : 1));
    if (!m)
        return HMC_ENOMEM;
    for (size_t i = 0; i < *count; i++) {
        m[i].address = body[2 + 7 * i];
        m[i].sequence = (uint16_t)get16(body + 3 + 7 * i);
        m[i].entry = get32(body + 5 + 7 * i);
    }
    *marks = m;
    return HMC_OK;
}

void ses_chain_init(ses_chain *c, const uint8_t key[16], const uint8_t nonce[8],
                    const uint8_t hello_tag[8])
{
    memcpy(c->key, key, 16);
    memcpy(c->nonce, nonce, 8);
    memcpy(c->last, hello_tag, 8);
    c->packet = 0;
}

int ses_archive(ses_chain *c, uint8_t address, uint16_t sequence, const uint8_t *header,
                size_t hlen, const ses_object *objs, size_t n, uint8_t **msg, size_t *len)
{
    size_t total = 1 + 11 + hlen + 2 + 8;
    for (size_t i = 0; i < n; i++)
        total += 20 + objs[i].cobs_len;
    uint8_t *raw = malloc(total), *p;
    if (!raw)
        return HMC_ENOMEM;
    c->packet++;
    raw[0] = SES_ARCHIVE;
    raw[1] = address;
    put16(raw + 2, sequence);
    put32(raw + 4, c->packet);
    put32(raw + 8, (uint32_t)hlen);
    if (hlen)
        memcpy(raw + 12, header, hlen);
    p = raw + 12 + hlen;
    put16(p, (uint32_t)n);
    p += 2;
    for (size_t i = 0; i < n; i++) {
        put32(p, objs[i].index);
        memcpy(p + 4, objs[i].entry, 16);
        memcpy(p + 20, objs[i].cobs, objs[i].cobs_len);
        p += 20 + objs[i].cobs_len;
    }
    const uint8_t *parts[3] = {c->nonce, c->last, raw};
    size_t lens[3] = {8, 8, total - 8};
    ses_tag(c->key, parts, lens, 3, c->last);
    memcpy(p, c->last, 8);
    return finish(raw, total, msg, len);
}

int ses_archive_verify(ses_chain *c, const uint8_t *body, size_t n, ses_archive_t *a)
{
    uint8_t t[8], type = SES_ARCHIVE;
    memset(a, 0, sizeof *a);
    if (n < 8)
        return HMC_DAMAGED;
    const uint8_t *parts[4] = {c->nonce, c->last, &type, body};
    size_t lens[4] = {8, 8, 1, n - 8};
    ses_tag(c->key, parts, lens, 4, t);
    if (!same_tag(t, body + n - 8))
        return HMC_DAMAGED;
    size_t end = n - 8, i = 11;
    if (end < 13)
        return HMC_DAMAGED;
    a->address = body[0];
    a->sequence = (uint16_t)get16(body + 1);
    a->packet = get32(body + 3);
    a->header_len = get32(body + 7);
    if (a->header_len > end - i || end - i - a->header_len < 2)
        return HMC_DAMAGED;
    a->header = body + i;
    i += a->header_len;
    a->count = get16(body + i);
    i += 2;
    a->objects = malloc(sizeof(ses_object) * (a->count ? a->count : 1));
    if (!a->objects)
        return HMC_ENOMEM;
    for (size_t k = 0; k < a->count; k++) {
        hmc_entry e;
        if (end - i < 20 || hmc_entry_parse(body + i + 4, &e) || e.length > end - i - 20) {
            free(a->objects);
            a->objects = NULL;
            return HMC_DAMAGED;
        }
        a->objects[k].index = get32(body + i);
        a->objects[k].entry = body + i + 4;
        a->objects[k].cobs = body + i + 20;
        a->objects[k].cobs_len = e.length;
        i += 20 + e.length;
    }
    if (i != end || a->packet != c->packet + 1) {
        free(a->objects);
        a->objects = NULL;
        return HMC_DAMAGED;
    }
    c->packet = a->packet;
    memcpy(c->last, body + n - 8, 8);
    return HMC_OK;
}

int ses_live_body(uint32_t second, const ses_seg *segs, size_t n, uint8_t **body, size_t *len)
{
    size_t total = 5;
    for (size_t i = 0; i < n; i++)
        total += 5 + segs[i].len;
    uint8_t *b = malloc(total), *p = b + 5;
    if (!b)
        return HMC_ENOMEM;
    put32(b, second);
    b[4] = (uint8_t)n;
    for (size_t i = 0; i < n; i++) {
        p[0] = segs[i].address;
        put32(p + 1, (uint32_t)segs[i].len);
        memcpy(p + 5, segs[i].segment, segs[i].len);
        p += 5 + segs[i].len;
    }
    *body = b;
    *len = total;
    return HMC_OK;
}

int ses_live(uint32_t second, const ses_seg *segs, size_t n, uint8_t **msg, size_t *len)
{
    uint8_t *body;
    size_t bl;
    int e = ses_live_body(second, segs, n, &body, &bl);
    if (e)
        return e;
    uint8_t *raw = malloc(bl + 1);
    if (!raw) {
        free(body);
        return HMC_ENOMEM;
    }
    raw[0] = SES_LIVE;
    memcpy(raw + 1, body, bl);
    free(body);
    return finish(raw, bl + 1, msg, len);
}

int ses_live_parse(const uint8_t *body, size_t n, uint32_t *second, ses_seg **segs, size_t *count)
{
    if (n < 5)
        return HMC_DAMAGED;
    *second = get32(body);
    *count = body[4];
    ses_seg *s = malloc(sizeof(ses_seg) * (*count ? *count : 1));
    size_t i = 5;
    if (!s)
        return HMC_ENOMEM;
    for (size_t k = 0; k < *count; k++) {
        if (n - i < 5 || get32(body + i + 1) > n - i - 5) {
            free(s);
            return HMC_DAMAGED;
        }
        s[k].address = body[i];
        s[k].len = get32(body + i + 1);
        s[k].segment = body + i + 5;
        i += 5 + s[k].len;
    }
    if (i != n) {
        free(s);
        return HMC_DAMAGED;
    }
    *segs = s;
    return HMC_OK;
}

int ses_window(const uint8_t key[16], uint32_t station, uint32_t counter, uint8_t address,
               uint32_t second, uint32_t span, uint8_t **msg, size_t *len)
{
    uint8_t raw[22], st[3] = {(uint8_t)station, (uint8_t)(station >> 8), (uint8_t)(station >> 16)};
    raw[0] = SES_WINDOW;
    put32(raw + 1, counter);
    raw[5] = address;
    put32(raw + 6, second);
    put32(raw + 10, span);
    const uint8_t *parts[2] = {st, raw};
    size_t lens[2] = {3, 14};
    ses_tag(key, parts, lens, 2, raw + 14);
    return ses_frame(raw, 22, msg, len);
}

int ses_window_parse(const uint8_t key[16], uint32_t station, uint32_t stored, const uint8_t *body,
                     size_t n, ses_window_t *w)
{
    uint8_t t[8], type = SES_WINDOW,
                  st[3] = {(uint8_t)station, (uint8_t)(station >> 8), (uint8_t)(station >> 16)};
    if (n != 21)
        return HMC_DAMAGED;
    const uint8_t *parts[3] = {st, &type, body};
    size_t lens[3] = {3, 1, 13};
    ses_tag(key, parts, lens, 3, t);
    if (!same_tag(t, body + 13) || get32(body) <= stored)
        return HMC_DAMAGED;
    w->counter = get32(body);
    w->address = body[4];
    w->second = get32(body + 5);
    w->window = get32(body + 9);
    return HMC_OK;
}

int ses_board(uint32_t station, const uint8_t *live_body, size_t n, uint8_t **msg, size_t *len)
{
    uint8_t *raw = malloc(4 + n);
    if (!raw)
        return HMC_ENOMEM;
    raw[0] = SES_BOARD;
    raw[1] = (uint8_t)station;
    put16(raw + 2, station >> 8);
    memcpy(raw + 4, live_body, n);
    return finish(raw, 4 + n, msg, len);
}

/* ------------------------------------------------------------------ the head's cuts */

int ses_object_of(const hmc_reader *r, uint32_t index, ses_object *o)
{
    hmc_entry e;
    if (hmc_reader_entry(r, index, &e))
        return HMC_DAMAGED;
    o->index = index;
    o->entry = r->image + r->size - 16 * ((size_t)index + 1);
    o->cobs = r->image + (e.offset < r->size ? e.offset : r->size);
    o->cobs_len = e.offset < r->size ? (e.length < r->size - e.offset ? e.length : r->size - e.offset) : 0;
    return HMC_OK;
}

size_t ses_forward(const hmc_reader *r, uint32_t held, size_t budget, uint32_t *idx,
                   uint32_t *packet)
{
    size_t m = 0, size = 0, cur = 0;
    uint32_t pk = 0;
    for (uint32_t i = 0; i < r->entries; i++) {
        ses_object o;
        if (ses_object_of(r, i, &o) || (held != SES_NONE_ENTRY && i <= held))
            continue;
        if (cur && size + 20 + o.cobs_len > budget) {
            pk++;
            cur = 0;
            size = 0;
        }
        idx[m] = i;
        packet[m++] = pk;
        cur++;
        size += 20 + o.cobs_len;
    }
    return m;
}

size_t ses_window_cut(const hmc_reader *r, uint32_t second, uint32_t span, uint32_t *idx)
{
    int64_t lo = (int64_t)second - span, hi = (int64_t)second + span;
    size_t m = 0;
    int64_t last = -1;
    uint8_t *touch = calloc(r->entries ? r->entries : 1, 1);
    if (!touch)
        return 0;
    for (uint32_t i = 0; i < r->entries; i++) {
        ses_object o;
        hmc_entry e;
        if (ses_object_of(r, i, &o) || hmc_reader_entry(r, i, &e) || e.type != HMC_SEG)
            continue;
        uint8_t *buf = malloc(o.cobs_len ? o.cobs_len : 1);
        size_t len;
        hmc_object ob;
        if (buf && !hmc_cobs_decode(o.cobs, o.cobs_len, buf, &len) && !hmc_object_parse(buf, len, &ob)
            && ob.type == HMC_SEG && ob.first <= hi && ob.last >= lo) {
            touch[i] = 1;
            last = i;
        }
        free(buf);
    }
    for (uint32_t i = 0; last >= 0 && i < r->entries; i++) {
        hmc_entry e;
        if (hmc_reader_entry(r, i, &e))
            continue;
        if (touch[i] || (e.type == HMC_CFG && (int64_t)i < last))
            idx[m++] = i;
    }
    free(touch);
    return m;
}

/* HCC — the C reference; see hcc.h. Built with -std=c99 -ffp-contract=off: the predictor's
 * double arithmetic must round every operation as ../hcc.py does, with no fused multiply-add. */
#if defined(__clang__)
#pragma STDC FP_CONTRACT OFF
#endif

#include "hcc.h"

#include <math.h>
#include <stdlib.h>
#include <string.h>

#define MAX_ORDER 8

/* ------------------------------------------------------------------ bit I/O */

typedef struct {
    uint8_t *buf;
    size_t len, cap;
    uint64_t acc;
    unsigned nbits;
    int nomem;
} bw_t;

static void bw_put(bw_t *b, uint8_t byte)
{
    if (b->len == b->cap) {
        size_t cap = b->cap ? 2 * b->cap : 256;
        uint8_t *p = realloc(b->buf, cap);
        if (!p) {
            b->nomem = 1;
            return;
        }
        b->buf = p;
        b->cap = cap;
    }
    b->buf[b->len++] = byte;
}

static void bw_write(bw_t *b, uint64_t v, unsigned width)
{
    if (width == 0 || b->nomem)
        return;
    v &= (1ull << width) - 1;                  /* width ≤ 33 */
    b->acc = (b->acc << width) | v;
    b->nbits += width;
    while (b->nbits >= 8) {
        b->nbits -= 8;
        bw_put(b, (uint8_t)(b->acc >> b->nbits));
    }
    b->acc &= (1ull << b->nbits) - 1;
}

static void bw_unary(bw_t *b, uint64_t q)
{
    while (q > 0) {
        unsigned n = q < 32 ? (unsigned)q : 32;
        bw_write(b, (1ull << n) - 1, n);
        q -= n;
    }
    bw_write(b, 0, 1);
}

static unsigned bitlen(uint64_t m)
{
    unsigned l = 0;
    while (m) {
        l++;
        m >>= 1;
    }
    return l;
}

static void bw_gamma(bw_t *b, uint64_t m)
{
    unsigned l = bitlen(m) - 1;
    bw_write(b, 0, l);
    bw_write(b, m, l + 1);
}

static void bw_flush(bw_t *b)
{
    if (b->nbits) {
        bw_put(b, (uint8_t)(b->acc << (8 - b->nbits)));
        b->acc = 0;
        b->nbits = 0;
    }
}

typedef struct {
    const uint8_t *d;
    uint64_t pos, end;
    int bad;
} br_t;

static uint64_t br_read(br_t *r, unsigned width)
{
    uint64_t v = 0;
    if (width == 0 || r->bad)
        return 0;
    if (r->pos + width > r->end) {
        r->bad = 1;
        return 0;
    }
    for (unsigned i = 0; i < width; i++, r->pos++)
        v = (v << 1) | ((r->d[r->pos >> 3] >> (7 - (r->pos & 7))) & 1u);
    return v;
}

static uint64_t br_unary(br_t *r)
{
    uint64_t q = 0;
    while (!r->bad && br_read(r, 1))
        q++;
    return q;
}

static uint64_t br_gamma(br_t *r)
{
    unsigned l = 0;
    while (!r->bad && br_read(r, 1) == 0) {
        if (r->bad)
            return 0;
        if (++l > 32) {
            r->bad = 1;
            return 0;
        }
    }
    if (r->bad)
        return 0;
    return (1ull << l) | br_read(r, l);
}

/* ------------------------------------------------------------------ small arithmetic */

static uint64_t mask_of(unsigned w) { return (w >= 64) ? ~0ull : (1ull << w) - 1; }

static int64_t to_signed(uint64_t v, unsigned w)
{
    return (v & (1ull << (w - 1))) ? (int64_t)v - (int64_t)(1ull << w) : (int64_t)v;
}

static int64_t value(uint64_t x, unsigned w, int sgn) { return sgn ? to_signed(x, w) : (int64_t)x; }

static int64_t floor_shift(int64_t a, unsigned s)  /* a >> s, arithmetic, in portable C */
{
    return a >= 0 ? (int64_t)((uint64_t)a >> s) : (int64_t) ~(((uint64_t) ~a) >> s);
}

static uint64_t fold(uint64_t r, unsigned w)
{
    uint64_t v = r & mask_of(w);
    if (v & (1ull << (w - 1)))
        return 2 * ((mask_of(w) - v) + 1) - 1;
    return 2 * v;
}

static uint64_t unfold(uint64_t u) { return (u & 1) ? ~(u >> 1) : (u >> 1); }

/* An exact sum of products, as two magnitudes — the autocorrelation needs up to ~2^80. */
typedef struct {
    uint64_t hi, lo;
} u128;

static void u128_add(u128 *a, uint64_t m)
{
    a->lo += m;
    if (a->lo < m)
        a->hi++;
}

static double u128_to_double(u128 m)       /* round to nearest, ties to even, as Python's float() */
{
    if (m.hi == 0 && m.lo < (1ull << 53))
        return (double)m.lo;
    unsigned l = m.hi ? 64 + bitlen(m.hi) : bitlen(m.lo);
    unsigned sh = l - 53;                    /* 1 .. 75 */
    uint64_t mant, rem_hi, rem_lo, half_hi, half_lo;
    if (sh >= 64) {
        mant = m.hi >> (sh - 64);
        rem_hi = m.hi & mask_of(sh - 64);
        rem_lo = m.lo;
        half_hi = (sh - 64) ? 1ull << (sh - 65) : 0;
        half_lo = (sh - 64) ? 0 : 1ull << 63;
    } else {
        mant = (m.lo >> sh) | (m.hi << (64 - sh));
        rem_hi = 0;
        rem_lo = m.lo & mask_of(sh);
        half_hi = 0;
        half_lo = 1ull << (sh - 1);
    }
    int up;
    if (rem_hi != half_hi)
        up = rem_hi > half_hi;
    else if (rem_lo != half_lo)
        up = rem_lo > half_lo;
    else
        up = (int)(mant & 1);
    if (up && ++mant == (1ull << 53)) {
        mant >>= 1;
        sh++;
    }
    return ldexp((double)mant, (int)sh);
}

/* ------------------------------------------------------------------ residuals */

static uint64_t part_bits(const uint64_t *us, unsigned m, unsigned k, unsigned w)
{
    uint64_t t = 5;
    if (k == 31)
        return t + (uint64_t)w * m;
    for (unsigned i = 0; i < m; i++)
        t += (us[i] >> k) + 1 + k;
    return t;
}

static unsigned best_k(const uint64_t *us, unsigned m, unsigned w, uint64_t *bits)
{
    unsigned kmax = w < 30 ? w : 30, bk = 0;
    uint64_t best = UINT64_MAX;
    for (unsigned k = 0; k <= kmax; k++) {
        uint64_t b = part_bits(us, m, k, w);
        if (b < best) {
            best = b;
            bk = k;
        }
    }
    uint64_t esc = part_bits(us, m, 31, w);
    if (esc <= best) {
        *bits = esc;
        return 31;
    }
    *bits = best;
    return bk;
}

static uint64_t residual_bits(const uint32_t *rs, size_t n, unsigned w)
{
    uint64_t total = 0, b;
    uint64_t us[32];
    for (size_t i = 0; i < n; i += 32) {
        unsigned m = (unsigned)(n - i < 32 ? n - i : 32);
        for (unsigned j = 0; j < m; j++)
            us[j] = fold(rs[i + j], w);
        best_k(us, m, w, &b);
        total += b;
    }
    return total;
}

static void write_residuals(bw_t *bw, const uint32_t *rs, size_t n, unsigned w)
{
    uint64_t us[32], b;
    for (size_t i = 0; i < n; i += 32) {
        unsigned m = (unsigned)(n - i < 32 ? n - i : 32);
        for (unsigned j = 0; j < m; j++)
            us[j] = fold(rs[i + j], w);
        unsigned k = best_k(us, m, w, &b);
        bw_write(bw, k, 5);
        for (unsigned j = 0; j < m; j++) {
            if (k == 31) {
                bw_write(bw, us[j], w);
            } else {
                bw_unary(bw, us[j] >> k);
                bw_write(bw, us[j], k);
            }
        }
    }
}

static int read_residual(br_t *r, unsigned *k, size_t j, unsigned w, uint64_t *res)
{
    if (j % 32 == 0)
        *k = (unsigned)br_read(r, 5);
    uint64_t u;
    if (*k == 31) {
        u = br_read(r, w);
    } else {
        uint64_t q = br_unary(r);
        u = (q << *k) | br_read(r, *k);
    }
    *res = unfold(u);
    return r->bad;
}

/* ------------------------------------------------------------------ the predictor */

static int levinson(const double *r, int max_order, double sets[MAX_ORDER][MAX_ORDER])
{
    double a[MAX_ORDER], nw[MAX_ORDER], err;
    int nsets = 0;
    if (r[0] <= 0.0)
        return 0;
    err = r[0];
    for (int p = 1; p <= max_order; p++) {
        double acc = r[p];
        for (int j = 1; j < p; j++)
            acc = acc - a[j - 1] * r[p - j];
        if (err <= 0.0)
            break;
        double k = acc / err;
        if (!(-1.0 < k && k < 1.0))
            break;
        for (int j = 1; j < p; j++)
            nw[j - 1] = a[j - 1] - k * a[p - j - 1];
        nw[p - 1] = k;
        memcpy(a, nw, sizeof(double) * (size_t)p);
        err = err * (1.0 - k * k);
        memcpy(sets[nsets++], a, sizeof(double) * (size_t)p);
    }
    return nsets;
}

static int quantise(const double *a, int p, int *s_out, int32_t *q)
{
    double amax = 0.0;
    for (int j = 0; j < p; j++)
        if (fabs(a[j]) > amax)
            amax = fabs(a[j]);
    if (amax == 0.0) {
        *s_out = 15;
        for (int j = 0; j < p; j++)
            q[j] = 0;
        return 1;
    }
    int s = 15;
    while (s >= 0 && amax * (double)(1 << s) >= (double)(1 << 14))
        s--;
    if (s < 0)
        return 0;
    for (int j = 0; j < p; j++) {
        double v = a[j] * (double)(1 << s);
        int64_t qi = (int64_t)(fabs(v) + 0.5);
        if (qi >= (1 << 14))
            return 0;
        q[j] = (int32_t)(v >= 0 ? qi : -qi);
    }
    *s_out = s;
    return 1;
}

static void lpc_residuals(const uint32_t *xs, size_t n, unsigned w, int sgn, int p, int s,
                          const int32_t *c, uint32_t *rs)
{
    uint64_t mask = mask_of(w);
    for (size_t i = (size_t)p; i < n; i++) {
        int64_t acc = 0;
        for (int j = 1; j <= p; j++)
            acc += (int64_t)c[j - 1] * value(xs[i - (size_t)j], w, sgn);
        rs[i - (size_t)p] = (uint32_t)(((uint64_t)value(xs[i], w, sgn)
                                        - (uint64_t)floor_shift(acc, (unsigned)s)) & mask);
    }
}

/* ------------------------------------------------------------------ a series */

typedef struct {
    uint64_t bits;
    int use_ref, mode, order, s;
    int32_t c[MAX_ORDER];
    int set;
} choice_t;

static int better(const choice_t *b, uint64_t bits, int use_ref, int mode, int order)
{
    if (!b->set) return 1;
    if (bits != b->bits) return bits < b->bits;
    if (use_ref != b->use_ref) return use_ref < b->use_ref;
    if (mode != b->mode) return mode < b->mode;
    return order < b->order;
}

static void take(choice_t *b, uint64_t bits, int use_ref, int mode, int order, int s,
                 const int32_t *c)
{
    b->bits = bits;
    b->use_ref = use_ref;
    b->mode = mode;
    b->order = order;
    b->s = s;
    if (c)
        memcpy(b->c, c, sizeof(int32_t) * (size_t)order);
    b->set = 1;
}

static void diff1(const uint32_t *xs, size_t n, uint64_t mask, uint32_t *rs)
{
    for (size_t i = 1; i < n; i++)
        rs[i - 1] = (uint32_t)(((uint64_t)xs[i] - xs[i - 1]) & mask);
}

static void diff2(const uint32_t *xs, size_t n, uint64_t mask, uint32_t *rs)
{
    for (size_t i = 2; i < n; i++)
        rs[i - 2] = (uint32_t)(((uint64_t)xs[i] - 2 * (uint64_t)xs[i - 1] + xs[i - 2]) & mask);
}

static void consider(const uint32_t *xs, size_t n, unsigned w, int sgn, int use_ref,
                     uint32_t *rs, choice_t *best)
{
    uint64_t mask = mask_of(w), bits;
    size_t i;
    for (i = 1; i < n && xs[i] == xs[0]; i++)
        ;
    if (i == n && better(best, 3 + w, use_ref, 0, 0))
        take(best, 3 + w, use_ref, 0, 0, 0, NULL);
    diff1(xs, n, mask, rs);
    bits = 3 + w + residual_bits(rs, n - 1, w);
    if (better(best, bits, use_ref, 1, 0))
        take(best, bits, use_ref, 1, 0, 0, NULL);
    if (n >= 2) {
        diff2(xs, n, mask, rs);
        bits = 3 + 2 * (uint64_t)w + residual_bits(rs, n - 2, w);
        if (better(best, bits, use_ref, 2, 0))
            take(best, bits, use_ref, 2, 0, 0, NULL);
    }
    /* LPC: the autocorrelation exact, then Levinson–Durbin in double */
    double r[MAX_ORDER + 1];
    int nr = 0;
    for (size_t lag = 0; lag <= MAX_ORDER && lag < n; lag++, nr++) {
        u128 pos = {0, 0}, neg = {0, 0};
        for (i = lag; i < n; i++) {
            int64_t a = value(xs[i], w, sgn), b = value(xs[i - lag], w, sgn);
            uint64_t ma = (uint64_t)(a < 0 ? -a : a), mb = (uint64_t)(b < 0 ? -b : b);
            u128_add(((a < 0) != (b < 0)) ? &neg : &pos, ma * mb);
        }
        int negative = pos.hi < neg.hi || (pos.hi == neg.hi && pos.lo < neg.lo);
        u128 big = negative ? neg : pos, small = negative ? pos : neg, d;
        d.lo = big.lo - small.lo;
        d.hi = big.hi - small.hi - (big.lo < small.lo);
        r[lag] = negative ? -u128_to_double(d) : u128_to_double(d);
    }
    if (nr >= 2) {
        double sets[MAX_ORDER][MAX_ORDER];
        int ns = levinson(r, nr - 1 < MAX_ORDER ? nr - 1 : MAX_ORDER, sets);
        for (int k = 0; k < ns; k++) {
            int p = k + 1, s;
            int32_t c[MAX_ORDER];
            if (!quantise(sets[k], p, &s, c) || (size_t)p >= n)
                continue;
            lpc_residuals(xs, n, w, sgn, p, s, c, rs);
            bits = 3 + 3 + 4 + 15 * (uint64_t)p + (uint64_t)p * w + residual_bits(rs, n - (size_t)p, w);
            if (better(best, bits, use_ref, 3, p))
                take(best, bits, use_ref, 3, p, s, c);
        }
    }
    bits = 3 + (uint64_t)n * w;
    if (better(best, bits, use_ref, 4, 0))
        take(best, bits, use_ref, 4, 0, 0, NULL);
}

static int write_series(bw_t *bw, const uint32_t *xs, size_t n, unsigned w, int sgn,
                        const uint32_t *ref)
{
    if (n == 0)
        return HCC_OK;
    uint64_t mask = mask_of(w);
    uint32_t *rs = malloc(sizeof(uint32_t) * n), *dv = NULL;
    if (!rs)
        return HCC_ENOMEM;
    choice_t best;
    memset(&best, 0, sizeof best);
    consider(xs, n, w, sgn, 0, rs, &best);
    if (ref) {
        dv = malloc(sizeof(uint32_t) * n);
        if (!dv) {
            free(rs);
            return HCC_ENOMEM;
        }
        for (size_t i = 0; i < n; i++)
            dv[i] = (uint32_t)(((uint64_t)xs[i] - ref[i]) & mask);
        consider(dv, n, w, sgn, 1, rs, &best);
    }
    const uint32_t *v = best.use_ref ? dv : xs;
    if (ref)
        bw_write(bw, (uint64_t)best.use_ref, 1);
    bw_write(bw, (uint64_t)best.mode, 3);
    switch (best.mode) {
    case 0:
        bw_write(bw, v[0], w);
        break;
    case 1:
        bw_write(bw, v[0], w);
        diff1(v, n, mask, rs);
        write_residuals(bw, rs, n - 1, w);
        break;
    case 2:
        bw_write(bw, v[0], w);
        bw_write(bw, v[1], w);
        diff2(v, n, mask, rs);
        write_residuals(bw, rs, n - 2, w);
        break;
    case 3:
        bw_write(bw, (uint64_t)(best.order - 1), 3);
        bw_write(bw, (uint64_t)best.s, 4);
        for (int j = 0; j < best.order; j++)
            bw_write(bw, (uint64_t)best.c[j] & 0x7FFF, 15);
        for (int j = 0; j < best.order; j++)
            bw_write(bw, v[j], w);
        lpc_residuals(v, n, w, sgn, best.order, best.s, best.c, rs);
        write_residuals(bw, rs, n - (size_t)best.order, w);
        break;
    default:
        for (size_t i = 0; i < n; i++)
            bw_write(bw, v[i], w);
    }
    free(rs);
    free(dv);
    return bw->nomem ? HCC_ENOMEM : HCC_OK;
}

static int read_series(br_t *r, size_t n, unsigned w, int sgn, const uint32_t *ref, uint32_t *xs)
{
    if (n == 0)
        return HCC_OK;
    uint64_t mask = mask_of(w), res;
    unsigned k = 0;
    int use_ref = ref ? (int)br_read(r, 1) : 0;
    int mode = (int)br_read(r, 3);
    if (r->bad)
        return HCC_DAMAGED;
    switch (mode) {
    case 0: {
        uint32_t v = (uint32_t)br_read(r, w);
        for (size_t i = 0; i < n; i++)
            xs[i] = v;
        break;
    }
    case 1:
        xs[0] = (uint32_t)br_read(r, w);
        for (size_t i = 1; i < n; i++) {
            if (read_residual(r, &k, i - 1, w, &res))
                return HCC_DAMAGED;
            xs[i] = (uint32_t)((xs[i - 1] + res) & mask);
        }
        break;
    case 2:
        if (n < 2)
            return HCC_DAMAGED;
        xs[0] = (uint32_t)br_read(r, w);
        xs[1] = (uint32_t)br_read(r, w);
        for (size_t i = 2; i < n; i++) {
            if (read_residual(r, &k, i - 2, w, &res))
                return HCC_DAMAGED;
            xs[i] = (uint32_t)((res + 2 * (uint64_t)xs[i - 1] - xs[i - 2]) & mask);
        }
        break;
    case 3: {
        size_t p = (size_t)br_read(r, 3) + 1;
        if (r->bad || p >= n)
            return HCC_DAMAGED;
        unsigned s = (unsigned)br_read(r, 4);
        int64_t c[MAX_ORDER];
        for (size_t j = 0; j < p; j++)
            c[j] = to_signed(br_read(r, 15), 15);
        for (size_t j = 0; j < p; j++)
            xs[j] = (uint32_t)br_read(r, w);
        for (size_t i = p; i < n; i++) {
            if (read_residual(r, &k, i - p, w, &res))
                return HCC_DAMAGED;
            int64_t acc = 0;
            for (size_t j = 1; j <= p; j++)
                acc += c[j - 1] * value(xs[i - j], w, sgn);
            xs[i] = (uint32_t)((res + (uint64_t)floor_shift(acc, s)) & mask);
        }
        break;
    }
    case 4:
        for (size_t i = 0; i < n && !r->bad; i++)
            xs[i] = (uint32_t)br_read(r, w);
        break;
    default:
        return HCC_DAMAGED;
    }
    if (r->bad)
        return HCC_DAMAGED;
    if (use_ref)
        for (size_t i = 0; i < n; i++)
            xs[i] = (uint32_t)(((uint64_t)xs[i] + ref[i]) & mask);
    return HCC_OK;
}

/* ------------------------------------------------------------------ the layout */

size_t hcc_compose_layout(const hcc_field *fields, size_t n, hcc_field *out)
{
    uint8_t covered[HCC_PAYLOAD] = {0};
    int next = -1;
    for (size_t i = 0; i < n; i++) {
        out[i] = fields[i];
        for (unsigned b = fields[i].offset; b < (unsigned)fields[i].offset + fields[i].width
                                            && b < HCC_PAYLOAD; b++)
            covered[b] = 1;
        if (fields[i].channel > next)
            next = fields[i].channel;
    }
    next++;
    size_t m = n;
    for (unsigned b = 0; b < HCC_PAYLOAD; b++)
        if (!covered[b]) {
            hcc_field f = {(uint8_t)b, 1, 0, 0, (uint8_t)next++, 0};
            out[m++] = f;
        }
    return m;
}

size_t hcc_argus_layout(const hcc_field *const *positions, const size_t *counts, size_t npos,
                        hcc_field *out)
{
    hcc_field tmp[HCC_MAX_LAYOUT * 4];
    size_t m = 0;
    int next = 0;
    for (size_t pos = 0; pos < npos; pos++) {
        if (!positions[pos] || !counts[pos])
            continue;
        int base = next;
        for (size_t i = 0; i < counts[pos]; i++) {
            hcc_field f = positions[pos][i];
            f.offset = (uint8_t)(f.offset + 8 * pos);
            f.channel = (uint8_t)(base + f.channel);
            f.ref = f.ref ? (uint8_t)(base + f.ref) : 0;
            tmp[m++] = f;
        }
        for (size_t i = 0; i < m; i++)
            if (tmp[i].channel + 1 > next)
                next = tmp[i].channel + 1;
    }
    return hcc_compose_layout(tmp, m, out);
}

typedef struct {
    int channel;
    size_t first, count;      /* into the sorted field order */
} chan_t;

/* The layout's channels ascending, each with its fields in offset order. */
static int channels(const hcc_field *layout, size_t nl, hcc_field *sorted, chan_t *ch, size_t *nch)
{
    if (nl > HCC_MAX_LAYOUT)
        return HCC_EINVAL;
    memcpy(sorted, layout, sizeof(hcc_field) * nl);
    for (size_t i = 1; i < nl; i++) {           /* stable insertion sort by (channel, offset) */
        hcc_field f = sorted[i];
        size_t j = i;
        while (j > 0 && (sorted[j - 1].channel > f.channel
                         || (sorted[j - 1].channel == f.channel && sorted[j - 1].offset > f.offset))) {
            sorted[j] = sorted[j - 1];
            j--;
        }
        sorted[j] = f;
    }
    *nch = 0;
    for (size_t i = 0; i < nl; i++) {
        const hcc_field *f = &sorted[i];
        if (f->width < 1 || f->width > 4 || f->offset + f->width > (int)HCC_PAYLOAD)
            return HCC_EINVAL;
        if (*nch && ch[*nch - 1].channel == f->channel) {
            if (f->width != sorted[ch[*nch - 1].first].width)
                return HCC_EINVAL;
            ch[*nch - 1].count++;
        } else {
            ch[*nch].channel = f->channel;
            ch[*nch].first = i;
            ch[*nch].count = 1;
            (*nch)++;
        }
    }
    return HCC_OK;
}

static uint32_t read_field(const uint8_t *pl, const hcc_field *f)
{
    uint32_t v = 0;
    for (unsigned i = 0; i < f->width; i++)
        v |= (uint32_t)pl[f->offset + i] << (8 * (f->big_endian ? f->width - 1 - i : i));
    return v;
}

static void write_field(uint8_t *pl, const hcc_field *f, uint32_t v)
{
    for (unsigned i = 0; i < f->width; i++)
        pl[f->offset + i] = (uint8_t)(v >> (8 * (f->big_endian ? f->width - 1 - i : i)));
}

/* ------------------------------------------------------------------ the stream */

#define TRY(x) do { int e_ = (x); if (e_) { err = e_; goto done; } } while (0)

int hcc_encode(const hcc_record *recs, size_t n, const hcc_field *layout, size_t nl,
               uint8_t **out, size_t *outlen)
{
    hcc_field sorted[HCC_MAX_LAYOUT];
    chan_t ch[HCC_MAX_LAYOUT];
    size_t nch, nd = 0, no = 0, nb = 0, ng = 0;
    uint16_t groups[HCC_MAX_GROUPS];
    uint32_t *col = NULL, *done_xs[256] = {0};
    size_t done_n[256] = {0};
    uint8_t *gi = NULL;
    bw_t bw;
    int err = HCC_OK;

    memset(&bw, 0, sizeof bw);
    *out = NULL;
    *outlen = 0;
    if (n == 0 || n > HCC_MAX_RECORDS)
        return HCC_EINVAL;
    if (channels(layout, nl, sorted, ch, &nch))
        return HCC_EINVAL;
    for (size_t i = 0; i < n; i++) {
        const hcc_record *r = &recs[i];
        if (r->form == 0) {
            if (r->length != HCC_PAYLOAD)
                return HCC_EINVAL;
            if (r->kind == 0) nd++; else no++;
        } else if (r->form == 1) {
            if (r->length < 2 || r->length != 2u + r->data[1])
                return HCC_EINVAL;
            nb++;
        } else {
            return HCC_EINVAL;
        }
    }
    col = malloc(sizeof(uint32_t) * n);   /* a header, an OTHER byte, a group's byte: ≤ n values */
    gi = malloc(nb ? nb : 1);
    if (!col || !gi)
        TRY(HCC_ENOMEM);

    bw_gamma(&bw, n);
    /* 2 HEADERS */
    for (size_t i = 0; i < n; i++) col[i] = recs[i].second;
    TRY(write_series(&bw, col, n, 32, 0, NULL));
    for (size_t i = 0; i < n; i++) col[i] = recs[i].frame;
    TRY(write_series(&bw, col, n, 8, 0, NULL));
    for (size_t i = 0; i < n; i++) col[i] = recs[i].kind;
    TRY(write_series(&bw, col, n, 8, 0, NULL));
    for (size_t i = 0; i < n; i++) col[i] = recs[i].status;
    TRY(write_series(&bw, col, n, 8, 0, NULL));
    for (size_t i = 0; i < n; i++) col[i] = recs[i].form;
    TRY(write_series(&bw, col, n, 8, 0, NULL));
    /* 3 DATA — frame records of kind 0, one series per channel */
    for (size_t c = 0; c < nch; c++) {
        const hcc_field *fs = &sorted[ch[c].first];
        size_t nf = ch[c].count, m = 0;
        uint32_t *xs = malloc(sizeof(uint32_t) * (nd * nf > 0 ? nd * nf : 1));
        if (!xs)
            TRY(HCC_ENOMEM);
        for (size_t i = 0; i < n; i++)
            if (recs[i].form == 0 && recs[i].kind == 0)
                for (size_t f = 0; f < nf; f++)
                    xs[m++] = read_field(recs[i].data, &fs[f]);
        const uint32_t *refv = NULL;
        if (fs[0].ref && done_xs[fs[0].ref - 1] && done_n[fs[0].ref - 1] == m)
            refv = done_xs[fs[0].ref - 1];
        free(done_xs[ch[c].channel]);
        done_xs[ch[c].channel] = xs;
        done_n[ch[c].channel] = m;
        TRY(write_series(&bw, xs, m, 8u * fs[0].width, fs[0].is_signed, refv));
    }
    /* 4 OTHER — frame records of any other kind, 32 one-byte series */
    for (unsigned b = 0; b < HCC_PAYLOAD; b++) {
        size_t m = 0;
        for (size_t i = 0; i < n; i++)
            if (recs[i].form == 0 && recs[i].kind != 0)
                col[m++] = recs[i].data[b];
        TRY(write_series(&bw, col, m, 8, 0, NULL));
    }
    /* 5 BLOCKS */
    for (size_t i = 0, k = 0; i < n; i++) {
        if (recs[i].form != 1)
            continue;
        uint16_t g = (uint16_t)(recs[i].data[0] << 8 | recs[i].data[1]);
        size_t j;
        for (j = 0; j < ng && groups[j] != g; j++)
            ;
        if (j == ng) {
            if (ng == HCC_MAX_GROUPS)
                TRY(HCC_EINVAL);
            groups[ng++] = g;
        }
        gi[k++] = (uint8_t)j;
    }
    bw_gamma(&bw, ng + 1);
    for (size_t j = 0; j < ng; j++) {
        bw_write(&bw, groups[j] >> 8, 8);
        bw_write(&bw, groups[j] & 0xFF, 8);
    }
    if (nb) {
        for (size_t k = 0; k < nb; k++) col[k] = gi[k];
        TRY(write_series(&bw, col, nb, 8, 0, NULL));
        for (size_t j = 0; j < ng; j++) {
            unsigned length = groups[j] & 0xFF;
            for (unsigned b = 0; b < length; b++) {
                size_t m = 0;
                for (size_t i = 0, k = 0; i < n; i++)
                    if (recs[i].form == 1 && gi[k++] == j)
                        col[m++] = recs[i].data[2 + b];
                TRY(write_series(&bw, col, m, 8, 0, NULL));
            }
        }
    }
    bw_flush(&bw);
    if (bw.nomem)
        TRY(HCC_ENOMEM);
    *out = bw.buf;
    *outlen = bw.len;
    bw.buf = NULL;
done:
    free(bw.buf);
    free(col);
    free(gi);
    for (int c = 0; c < 256; c++)
        free(done_xs[c]);
    return err;
}

int hcc_decode(const uint8_t *stream, size_t len, const hcc_field *layout, size_t nl,
               hcc_record **recs_out, size_t *n_out)
{
    hcc_field sorted[HCC_MAX_LAYOUT];
    chan_t ch[HCC_MAX_LAYOUT];
    size_t nch, n, nd = 0, no = 0, nb = 0, g = 0;
    uint32_t *hd[5] = {0}, *xs = NULL, *done_xs[256] = {0}, *gidx = NULL;
    size_t done_n[256] = {0};
    uint8_t *data = NULL, *other = NULL, *bdata = NULL;
    uint16_t *groups = NULL;
    size_t *gcount = NULL, *goff = NULL, *gseen = NULL;
    br_t r = {stream, 0, (uint64_t)len * 8, 0};
    int err = HCC_OK;

    *recs_out = NULL;
    *n_out = 0;
    if (channels(layout, nl, sorted, ch, &nch))
        return HCC_EINVAL;
    uint64_t count = br_gamma(&r);
    if (r.bad || count > HCC_MAX_RECORDS)
        return HCC_DAMAGED;
    n = (size_t)count;
    static const unsigned hw[5] = {32, 8, 8, 8, 8};
    for (int h = 0; h < 5; h++) {
        hd[h] = malloc(sizeof(uint32_t) * n);
        if (!hd[h])
            TRY(HCC_ENOMEM);
        TRY(read_series(&r, n, hw[h], 0, NULL, hd[h]));
    }
    for (size_t i = 0; i < n; i++) {
        if (hd[4][i] > 1)
            TRY(HCC_DAMAGED);
        if (hd[4][i] == 1) nb++;
        else if (hd[2][i] == 0) nd++;
        else no++;
    }
    data = calloc(nd ? nd : 1, HCC_PAYLOAD);
    other = calloc(no ? no : 1, HCC_PAYLOAD);
    if (!data || !other)
        TRY(HCC_ENOMEM);
    for (size_t c = 0; c < nch; c++) {
        const hcc_field *fs = &sorted[ch[c].first];
        size_t nf = ch[c].count, m = nd * nf;
        xs = malloc(sizeof(uint32_t) * (m ? m : 1));
        if (!xs)
            TRY(HCC_ENOMEM);
        const uint32_t *refv = NULL;
        if (fs[0].ref && done_xs[fs[0].ref - 1] && done_n[fs[0].ref - 1] == m)
            refv = done_xs[fs[0].ref - 1];
        TRY(read_series(&r, m, 8u * fs[0].width, fs[0].is_signed, refv, xs));
        for (size_t i = 0, k = 0; i < nd; i++)
            for (size_t f = 0; f < nf; f++)
                write_field(data + HCC_PAYLOAD * i, &fs[f], xs[k++]);
        free(done_xs[ch[c].channel]);
        done_xs[ch[c].channel] = xs;
        done_n[ch[c].channel] = m;
        xs = NULL;
    }
    xs = malloc(sizeof(uint32_t) * (no > nb ? (no ? no : 1) : (nb ? nb : 1)));
    if (!xs)
        TRY(HCC_ENOMEM);
    for (unsigned b = 0; b < HCC_PAYLOAD; b++) {
        TRY(read_series(&r, no, 8, 0, NULL, xs));
        for (size_t i = 0; i < no; i++)
            other[HCC_PAYLOAD * i + b] = (uint8_t)xs[i];
    }
    uint64_t gg = br_gamma(&r);
    if (r.bad || gg - 1 > (r.end - r.pos) / 16)
        TRY(HCC_DAMAGED);
    g = (size_t)(gg - 1);
    groups = malloc(sizeof(uint16_t) * (g ? g : 1));
    gcount = calloc(g ? g : 1, sizeof(size_t));
    goff = calloc(g ? g : 1, sizeof(size_t));
    gseen = calloc(g ? g : 1, sizeof(size_t));
    if (!groups || !gcount || !goff || !gseen)
        TRY(HCC_ENOMEM);
    for (size_t j = 0; j < g; j++) {
        uint16_t module = (uint16_t)br_read(&r, 8);
        groups[j] = (uint16_t)(module << 8 | br_read(&r, 8));
    }
    if (r.bad)
        TRY(HCC_DAMAGED);
    size_t bytes = 0;
    if (nb) {
        gidx = malloc(sizeof(uint32_t) * nb);
        if (!gidx)
            TRY(HCC_ENOMEM);
        TRY(read_series(&r, nb, 8, 0, NULL, gidx));
        for (size_t k = 0; k < nb; k++) {
            if (gidx[k] >= g)
                TRY(HCC_DAMAGED);
            gcount[gidx[k]]++;
        }
        for (size_t j = 0; j < g; j++) {
            goff[j] = bytes;
            bytes += gcount[j] * (2u + (groups[j] & 0xFF));
        }
        bdata = malloc(bytes ? bytes : 1);
        if (!bdata)
            TRY(HCC_ENOMEM);
        for (size_t j = 0; j < g; j++) {
            unsigned length = groups[j] & 0xFF;
            size_t cnt = gcount[j];
            for (size_t i = 0; i < cnt; i++) {
                bdata[goff[j] + i * (2u + length)] = (uint8_t)(groups[j] >> 8);
                bdata[goff[j] + i * (2u + length) + 1] = (uint8_t)length;
            }
            for (unsigned b = 0; b < length; b++) {
                TRY(read_series(&r, cnt, 8, 0, NULL, xs));
                for (size_t i = 0; i < cnt; i++)
                    bdata[goff[j] + i * (2u + length) + 2 + b] = (uint8_t)xs[i];
            }
        }
    }
    if (r.end - r.pos >= 8)
        TRY(HCC_DAMAGED);
    {
        size_t total = sizeof(hcc_record) * (n ? n : 1) + HCC_PAYLOAD * (nd + no) + bytes;
        hcc_record *out = malloc(total);
        if (!out)
            TRY(HCC_ENOMEM);
        uint8_t *pool = (uint8_t *)(out + (n ? n : 1));
        size_t di = 0, oi = 0, bi = 0;
        for (size_t i = 0; i < n; i++) {
            hcc_record *o = &out[i];
            o->second = hd[0][i];
            o->frame = (uint8_t)hd[1][i];
            o->kind = (uint8_t)hd[2][i];
            o->status = (uint8_t)hd[3][i];
            o->form = (uint8_t)hd[4][i];
            if (o->form == 1) {
                size_t j = gidx[bi++];
                size_t len_b = 2u + (groups[j] & 0xFF);
                memcpy(pool, bdata + goff[j] + gseen[j]++ * len_b, len_b);
                o->length = (uint16_t)len_b;
            } else {
                memcpy(pool, (o->kind == 0 ? data + HCC_PAYLOAD * di++ : other + HCC_PAYLOAD * oi++),
                       HCC_PAYLOAD);
                o->length = HCC_PAYLOAD;
            }
            o->data = pool;
            pool += o->length;
        }
        *recs_out = out;
        *n_out = n;
    }
done:
    for (int h = 0; h < 5; h++)
        free(hd[h]);
    for (int c = 0; c < 256; c++)
        free(done_xs[c]);
    free(xs);
    free(gidx);
    free(data);
    free(other);
    free(bdata);
    free(groups);
    free(gcount);
    free(goff);
    free(gseen);
    return err;
}

void hcc_free(void *p) { free(p); }

size_t hcc_raw_length(const hcc_record *recs, size_t n)
{
    size_t t = 0;
    for (size_t i = 0; i < n; i++)
        t += 9 + recs[i].length;
    return t;
}

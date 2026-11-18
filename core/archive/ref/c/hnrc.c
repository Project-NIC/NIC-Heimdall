/* NRC — Heimdall's profile, the C reference; see hnrc.h. */

#include "hnrc.h"
#include "nrc.h"

#include <stdlib.h>
#include <string.h>

/* ------------------------------------------------------------------ the layout */

size_t hnrc_compose_layout(const hnrc_field *fields, size_t n, hnrc_field *out)
{
    uint8_t covered[HNRC_PAYLOAD] = {0};
    int next = -1;
    for (size_t i = 0; i < n; i++) {
        out[i] = fields[i];
        for (unsigned b = fields[i].offset; b < (unsigned)fields[i].offset + fields[i].width
                                            && b < HNRC_PAYLOAD; b++)
            covered[b] = 1;
        if (fields[i].channel > next)
            next = fields[i].channel;
    }
    next++;
    size_t m = n;
    for (unsigned b = 0; b < HNRC_PAYLOAD; b++)
        if (!covered[b]) {
            hnrc_field f = {(uint8_t)b, 1, 0, 0, (uint8_t)next++, 0};
            out[m++] = f;
        }
    return m;
}

size_t hnrc_argus_layout(const hnrc_field *const *positions, const size_t *counts, size_t npos,
                        hnrc_field *out)
{
    hnrc_field tmp[HNRC_MAX_LAYOUT * 4];
    size_t m = 0;
    int next = 0;
    for (size_t pos = 0; pos < npos; pos++) {
        if (!positions[pos] || !counts[pos])
            continue;
        int base = next;
        for (size_t i = 0; i < counts[pos]; i++) {
            hnrc_field f = positions[pos][i];
            f.offset = (uint8_t)(f.offset + 8 * pos);
            f.channel = (uint8_t)(base + f.channel);
            f.ref = f.ref ? (uint8_t)(base + f.ref) : 0;
            tmp[m++] = f;
        }
        for (size_t i = 0; i < m; i++)
            if (tmp[i].channel + 1 > next)
                next = tmp[i].channel + 1;
    }
    return hnrc_compose_layout(tmp, m, out);
}

typedef struct {
    int channel;
    size_t first, count;      /* into the sorted field order */
} chan_t;

/* The layout's channels ascending, each with its fields in offset order. */
static int channels(const hnrc_field *layout, size_t nl, hnrc_field *sorted, chan_t *ch, size_t *nch)
{
    if (nl > HNRC_MAX_LAYOUT)
        return HNRC_EINVAL;
    memcpy(sorted, layout, sizeof(hnrc_field) * nl);
    for (size_t i = 1; i < nl; i++) {           /* stable insertion sort by (channel, offset) */
        hnrc_field f = sorted[i];
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
        const hnrc_field *f = &sorted[i];
        if (f->width < 1 || f->width > 4 || f->offset + f->width > (int)HNRC_PAYLOAD)
            return HNRC_EINVAL;
        if (*nch && ch[*nch - 1].channel == f->channel) {
            if (f->width != sorted[ch[*nch - 1].first].width)
                return HNRC_EINVAL;
            ch[*nch - 1].count++;
        } else {
            ch[*nch].channel = f->channel;
            ch[*nch].first = i;
            ch[*nch].count = 1;
            (*nch)++;
        }
    }
    return HNRC_OK;
}

static uint32_t read_field(const uint8_t *pl, const hnrc_field *f)
{
    uint32_t v = 0;
    for (unsigned i = 0; i < f->width; i++)
        v |= (uint32_t)pl[f->offset + i] << (8 * (f->big_endian ? f->width - 1 - i : i));
    return v;
}

static void write_field(uint8_t *pl, const hnrc_field *f, uint32_t v)
{
    for (unsigned i = 0; i < f->width; i++)
        pl[f->offset + i] = (uint8_t)(v >> (8 * (f->big_endian ? f->width - 1 - i : i)));
}

/* ------------------------------------------------------------------ the stream */

#define TRY(x) do { int e_ = (x); if (e_) { err = e_; goto done; } } while (0)

int hnrc_encode(const hnrc_record *recs, size_t n, const hnrc_field *layout, size_t nl,
               uint8_t **out, size_t *outlen)
{
    hnrc_field sorted[HNRC_MAX_LAYOUT];
    chan_t ch[HNRC_MAX_LAYOUT];
    size_t nch, nd = 0, nb = 0, ng = 0;
    uint16_t groups[HNRC_MAX_GROUPS];
    uint32_t *col = NULL, *done_xs[256] = {0};
    size_t done_n[256] = {0};
    uint8_t *gi = NULL;
    nrc_writer bw;
    int err = HNRC_OK;

    memset(&bw, 0, sizeof bw);
    *out = NULL;
    *outlen = 0;
    if (n == 0 || n > HNRC_MAX_RECORDS)
        return HNRC_EINVAL;
    if (channels(layout, nl, sorted, ch, &nch))
        return HNRC_EINVAL;
    for (size_t i = 0; i < n; i++) {
        const hnrc_record *r = &recs[i];
        if (r->form == 0) {
            if (r->length != HNRC_PAYLOAD)
                return HNRC_EINVAL;
            if (r->kind == 0)
                nd++;
        } else if (r->form == 1) {
            if (r->length < 2 || r->length != 2u + r->data[1])
                return HNRC_EINVAL;
            nb++;
        } else {
            return HNRC_EINVAL;
        }
    }
    col = malloc(sizeof(uint32_t) * n);   /* a header, an OTHER byte, a group's byte: ≤ n values */
    gi = malloc(nb ? nb : 1);
    if (!col || !gi)
        TRY(HNRC_ENOMEM);

    nrc_gamma(&bw, n);
    /* 2 HEADERS */
    for (size_t i = 0; i < n; i++) col[i] = recs[i].second;
    TRY(nrc_write_series(&bw, col, n, 32, 0, NULL));
    for (size_t i = 0; i < n; i++) col[i] = recs[i].frame;
    TRY(nrc_write_series(&bw, col, n, 8, 0, NULL));
    for (size_t i = 0; i < n; i++) col[i] = recs[i].kind;
    TRY(nrc_write_series(&bw, col, n, 8, 0, NULL));
    for (size_t i = 0; i < n; i++) col[i] = recs[i].status;
    TRY(nrc_write_series(&bw, col, n, 8, 0, NULL));
    for (size_t i = 0; i < n; i++) col[i] = recs[i].form;
    TRY(nrc_write_series(&bw, col, n, 8, 0, NULL));
    /* 3 DATA — frame records of kind 0, one series per channel */
    for (size_t c = 0; c < nch; c++) {
        const hnrc_field *fs = &sorted[ch[c].first];
        size_t nf = ch[c].count, m = 0;
        uint32_t *xs = malloc(sizeof(uint32_t) * (nd * nf > 0 ? nd * nf : 1));
        if (!xs)
            TRY(HNRC_ENOMEM);
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
        TRY(nrc_write_series(&bw, xs, m, 8u * fs[0].width, fs[0].is_signed, refv));
    }
    /* 4 OTHER — frame records of any other kind, 32 one-byte series */
    for (unsigned b = 0; b < HNRC_PAYLOAD; b++) {
        size_t m = 0;
        for (size_t i = 0; i < n; i++)
            if (recs[i].form == 0 && recs[i].kind != 0)
                col[m++] = recs[i].data[b];
        TRY(nrc_write_series(&bw, col, m, 8, 0, NULL));
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
            if (ng == HNRC_MAX_GROUPS)
                TRY(HNRC_EINVAL);
            groups[ng++] = g;
        }
        gi[k++] = (uint8_t)j;
    }
    nrc_gamma(&bw, ng + 1);
    for (size_t j = 0; j < ng; j++) {
        nrc_write(&bw, groups[j] >> 8, 8);
        nrc_write(&bw, groups[j] & 0xFF, 8);
    }
    if (nb) {
        for (size_t k = 0; k < nb; k++) col[k] = gi[k];
        TRY(nrc_write_series(&bw, col, nb, 8, 0, NULL));
        for (size_t j = 0; j < ng; j++) {
            unsigned length = groups[j] & 0xFF;
            for (unsigned b = 0; b < length; b++) {
                size_t m = 0;
                for (size_t i = 0, k = 0; i < n; i++)
                    if (recs[i].form == 1 && gi[k++] == j)
                        col[m++] = recs[i].data[2 + b];
                TRY(nrc_write_series(&bw, col, m, 8, 0, NULL));
            }
        }
    }
    nrc_flush(&bw);
    if (bw.nomem)
        TRY(HNRC_ENOMEM);
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

int hnrc_decode(const uint8_t *stream, size_t len, const hnrc_field *layout, size_t nl,
               hnrc_record **recs_out, size_t *n_out)
{
    hnrc_field sorted[HNRC_MAX_LAYOUT];
    chan_t ch[HNRC_MAX_LAYOUT];
    size_t nch, n, nd = 0, no = 0, nb = 0, g = 0;
    uint32_t *hd[5] = {0}, *xs = NULL, *done_xs[256] = {0}, *gidx = NULL;
    size_t done_n[256] = {0};
    uint8_t *data = NULL, *other = NULL, *bdata = NULL;
    uint16_t *groups = NULL;
    size_t *gcount = NULL, *goff = NULL, *gseen = NULL;
    nrc_reader r = {stream, 0, (uint64_t)len * 8, 0};
    int err = HNRC_OK;

    *recs_out = NULL;
    *n_out = 0;
    if (channels(layout, nl, sorted, ch, &nch))
        return HNRC_EINVAL;
    uint64_t count = nrc_read_gamma(&r);
    if (r.bad || count > HNRC_MAX_RECORDS)
        return HNRC_DAMAGED;
    n = (size_t)count;
    static const unsigned hw[5] = {32, 8, 8, 8, 8};
    for (int h = 0; h < 5; h++) {
        hd[h] = malloc(sizeof(uint32_t) * n);
        if (!hd[h])
            TRY(HNRC_ENOMEM);
        TRY(nrc_read_series(&r, n, hw[h], 0, NULL, hd[h]));
    }
    for (size_t i = 0; i < n; i++) {
        if (hd[4][i] > 1)
            TRY(HNRC_DAMAGED);
        if (hd[4][i] == 1) nb++;
        else if (hd[2][i] == 0) nd++;
        else no++;
    }
    data = calloc(nd ? nd : 1, HNRC_PAYLOAD);
    other = calloc(no ? no : 1, HNRC_PAYLOAD);
    if (!data || !other)
        TRY(HNRC_ENOMEM);
    for (size_t c = 0; c < nch; c++) {
        const hnrc_field *fs = &sorted[ch[c].first];
        size_t nf = ch[c].count, m = nd * nf;
        xs = malloc(sizeof(uint32_t) * (m ? m : 1));
        if (!xs)
            TRY(HNRC_ENOMEM);
        const uint32_t *refv = NULL;
        if (fs[0].ref && done_xs[fs[0].ref - 1] && done_n[fs[0].ref - 1] == m)
            refv = done_xs[fs[0].ref - 1];
        TRY(nrc_read_series(&r, m, 8u * fs[0].width, fs[0].is_signed, refv, xs));
        for (size_t i = 0, k = 0; i < nd; i++)
            for (size_t f = 0; f < nf; f++)
                write_field(data + HNRC_PAYLOAD * i, &fs[f], xs[k++]);
        free(done_xs[ch[c].channel]);
        done_xs[ch[c].channel] = xs;
        done_n[ch[c].channel] = m;
        xs = NULL;
    }
    xs = malloc(sizeof(uint32_t) * (no > nb ? (no ? no : 1) : (nb ? nb : 1)));
    if (!xs)
        TRY(HNRC_ENOMEM);
    for (unsigned b = 0; b < HNRC_PAYLOAD; b++) {
        TRY(nrc_read_series(&r, no, 8, 0, NULL, xs));
        for (size_t i = 0; i < no; i++)
            other[HNRC_PAYLOAD * i + b] = (uint8_t)xs[i];
    }
    uint64_t gg = nrc_read_gamma(&r);
    if (r.bad || gg - 1 > (r.end - r.pos) / 16)
        TRY(HNRC_DAMAGED);
    g = (size_t)(gg - 1);
    groups = malloc(sizeof(uint16_t) * (g ? g : 1));
    gcount = calloc(g ? g : 1, sizeof(size_t));
    goff = calloc(g ? g : 1, sizeof(size_t));
    gseen = calloc(g ? g : 1, sizeof(size_t));
    if (!groups || !gcount || !goff || !gseen)
        TRY(HNRC_ENOMEM);
    for (size_t j = 0; j < g; j++) {
        uint16_t module = (uint16_t)nrc_read(&r, 8);
        groups[j] = (uint16_t)(module << 8 | nrc_read(&r, 8));
    }
    if (r.bad)
        TRY(HNRC_DAMAGED);
    size_t bytes = 0;
    if (nb) {
        gidx = malloc(sizeof(uint32_t) * nb);
        if (!gidx)
            TRY(HNRC_ENOMEM);
        TRY(nrc_read_series(&r, nb, 8, 0, NULL, gidx));
        for (size_t k = 0; k < nb; k++) {
            if (gidx[k] >= g)
                TRY(HNRC_DAMAGED);
            gcount[gidx[k]]++;
        }
        for (size_t j = 0; j < g; j++) {
            goff[j] = bytes;
            bytes += gcount[j] * (2u + (groups[j] & 0xFF));
        }
        bdata = malloc(bytes ? bytes : 1);
        if (!bdata)
            TRY(HNRC_ENOMEM);
        for (size_t j = 0; j < g; j++) {
            unsigned length = groups[j] & 0xFF;
            size_t cnt = gcount[j];
            for (size_t i = 0; i < cnt; i++) {
                bdata[goff[j] + i * (2u + length)] = (uint8_t)(groups[j] >> 8);
                bdata[goff[j] + i * (2u + length) + 1] = (uint8_t)length;
            }
            for (unsigned b = 0; b < length; b++) {
                TRY(nrc_read_series(&r, cnt, 8, 0, NULL, xs));
                for (size_t i = 0; i < cnt; i++)
                    bdata[goff[j] + i * (2u + length) + 2 + b] = (uint8_t)xs[i];
            }
        }
    }
    if (r.end - r.pos >= 8)
        TRY(HNRC_DAMAGED);
    {
        size_t total = sizeof(hnrc_record) * (n ? n : 1) + HNRC_PAYLOAD * (nd + no) + bytes;
        hnrc_record *out = malloc(total);
        if (!out)
            TRY(HNRC_ENOMEM);
        uint8_t *pool = (uint8_t *)(out + (n ? n : 1));
        size_t di = 0, oi = 0, bi = 0;
        for (size_t i = 0; i < n; i++) {
            hnrc_record *o = &out[i];
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
                memcpy(pool, (o->kind == 0 ? data + HNRC_PAYLOAD * di++ : other + HNRC_PAYLOAD * oi++),
                       HNRC_PAYLOAD);
                o->length = HNRC_PAYLOAD;
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

void hnrc_free(void *p) { free(p); }

size_t hnrc_raw_length(const hnrc_record *recs, size_t n)
{
    size_t t = 0;
    for (size_t i = 0; i < n; i++)
        t += 9 + recs[i].length;
    return t;
}

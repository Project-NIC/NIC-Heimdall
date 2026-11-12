/* The head's ingest; see nic_ingest.h. */
#include "nic_ingest.h"

#include <string.h>

void nic_ingest_init(nic_ingest *g, nic_segment_fn segment, void *ctx)
{
    memset(g, 0, sizeof *g);
    memset(g->idx, 0xFF, sizeof g->idx);
    g->segment = segment;
    g->ctx = ctx;
}

int nic_ingest_add(nic_ingest *g, const nic_ingest_unit *u)
{
    if (g->nunits == NIC_INGEST_UNITS || g->idx[u->addr] != 0xFF || u->rows == 0)
        return -1;
    uint8_t i = g->nunits++;
    g->units[i] = *u;
    g->idx[u->addr] = i;
    hmc_selector_init(&g->sel[i], &u->rule, u->blocks);
    return 0;
}

static void spool_out(nic_ingest *g, uint8_t i)
{
    nic_spool *s = &g->spool[i];
    if (!s->n)
        return;
    g->segment(g->ctx, g->units[i].addr, s->recs, s->n);
    g->segments++;
    s->n = s->used = 0;
}

static void spool_add(nic_ingest *g, uint8_t i, const hcc_record *r)
{
    nic_spool *s = &g->spool[i];
    if (s->n && s->used + 9 + r->length > NIC_SPOOL_BYTES)
        spool_out(g, i);
    if (!s->n)
        s->first_second = r->second;
    memcpy(s->bytes + s->used, r->data, r->length);
    s->recs[s->n] = *r;
    s->recs[s->n].data = s->bytes + s->used;
    s->used += 9 + r->length;       /* codec 0's size of the record: what 16 kB counts */
    s->n++;
}

static nic_bucket *bucket_for(nic_ingest *g, uint32_t second)
{
    for (int k = 0; k < 2; k++)
        if (g->b[k].open && g->b[k].second == second)
            return &g->b[k];
    return NULL;
}

int nic_ingest_frame(nic_ingest *g, const uint8_t trunk[NIC_TRUNK_LEN])
{
    uint32_t second;
    nic_header h;
    const uint8_t *p;
    if (nic_trunk_parse(trunk, &second, &h, &p) || g->idx[h.addr] == 0xFF) {
        g->dropped++;
        return NIC_IN_DROPPED;
    }
    uint8_t i = g->idx[h.addr];
    if (h.frame >= g->units[i].rows) {
        g->dropped++;
        return NIC_IN_DROPPED;
    }
    nic_bucket *b = bucket_for(g, second);
    if (!b) {
        uint32_t newest = 0;
        int any = 0;
        for (int k = 0; k < 2; k++)
            if (g->b[k].open) {
                any = 1;
                if (g->b[k].second > newest)
                    newest = g->b[k].second;
            }
        if (any && second < newest) {       /* its second is closed: a repair, kept */
            hcc_record r = {second, h.frame, h.kind, h.status, 0, NIC_PAYLOAD, p};
            spool_add(g, i, &r);
            g->repaired++;
            return NIC_IN_REPAIRED;
        }
        /* a new second: the older bucket must already be closed, else it is closed now */
        int k = !g->b[0].open ? 0 : !g->b[1].open ? 1 : (g->b[0].second < g->b[1].second ? 0 : 1);
        if (g->b[k].open)
            nic_ingest_close(g, g->b[k].second);
        b = &g->b[k];
        memset(b->have, 0, sizeof b->have);
        b->second = second;
        b->open = 1;
    }
    b->have[i][h.frame / 8] |= (uint8_t)(1u << (h.frame % 8));
    b->head[i][h.frame][0] = h.kind;
    b->head[i][h.frame][1] = h.status;
    memcpy(b->data[i][h.frame], p, NIC_PAYLOAD);
    return NIC_IN_STORED;
}

void nic_ingest_close(nic_ingest *g, uint32_t second)
{
    nic_bucket *b = bucket_for(g, second);
    if (b) {
        for (uint8_t i = 0; i < g->nunits; i++) {
            for (unsigned f = 0; f < g->units[i].rows; f++) {
                if (!(b->have[i][f / 8] & (1u << (f % 8)))) {   /* a hole: the head's FILLER */
                    memset(b->data[i][f], 0, NIC_PAYLOAD);
                    b->data[i][f][0] = 2;                         /* no data arrived */
                    b->head[i][f][0] = NIC_FILLER;
                    b->head[i][f][1] = 0;
                    g->holes++;
                }
                hcc_record kept[16];
                size_t n = hmc_select_frame(&g->sel[i], second, (uint8_t)f, b->head[i][f][0],
                                            b->head[i][f][1], b->data[i][f], kept);
                for (size_t k = 0; k < n; k++)
                    spool_add(g, i, &kept[k]);
            }
        }
        b->open = 0;
    }
    for (uint8_t i = 0; i < g->nunits; i++)       /* 8 s after a buffer's first record */
        if (g->spool[i].n && second >= g->spool[i].first_second + NIC_SPOOL_SECONDS - 1)
            spool_out(g, i);
}

void nic_ingest_flush(nic_ingest *g)
{
    for (uint8_t i = 0; i < g->nunits; i++)
        spool_out(g, i);
}

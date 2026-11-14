/* The head's live packet — see nic_live.h. */
#include "nic_live.h"

#include <stdlib.h>
#include <string.h>

#include "session.h"

static void put32(uint8_t *p, uint32_t v)
{
    p[0] = (uint8_t)v; p[1] = (uint8_t)(v >> 8); p[2] = (uint8_t)(v >> 16); p[3] = (uint8_t)(v >> 24);
}

void nic_live_init(nic_live *l)
{
    memset(l, 0, sizeof *l);
}

void nic_live_free(nic_live *l)
{
    for (uint8_t i = 0; i < l->n; i++)
        hmc_selector_free(&l->a[i].sel);
    l->n = 0;
}

static nic_live_addr *find(nic_live *l, uint8_t addr)
{
    for (uint8_t i = 0; i < l->n; i++)
        if (l->a[i].addr == addr)
            return &l->a[i];
    return 0;
}

int nic_live_add(nic_live *l, uint8_t addr, const hmc_rule *send, int blocks, const hcc_field *layout,
                 size_t nl)
{
    if (nl > HCC_MAX_LAYOUT)
        return -1;
    nic_live_addr *a = find(l, addr);
    if (a) {
        hmc_selector_free(&a->sel);
    } else {
        if (l->n == NIC_LIVE_ADDRS)
            return -1;
        a = &l->a[l->n++];
        memset(a, 0, sizeof *a);
        a->addr = addr;
    }
    hmc_selector_init(&a->sel, send, blocks);
    memcpy(a->layout, layout, nl * sizeof *layout);
    a->nl = nl;
    return 0;
}

/* The oldest records given up until `need` more bytes fit. */
static void give_way(nic_live *l, nic_live_addr *a, size_t need)
{
    size_t k = 0, freed = 0, data = 0;
    while (k < a->n && a->used - freed + need > NIC_LIVE_BYTES) {
        freed += 9 + a->recs[k].length;
        data += a->recs[k].length;
        k++;
    }
    if (!k)
        return;
    memmove(a->bytes, a->bytes + data, a->used - freed - 9 * (a->n - k));
    memmove(a->recs, a->recs + k, (a->n - k) * sizeof a->recs[0]);
    a->n -= k;
    a->used -= freed;
    size_t at = 0;
    for (size_t i = 0; i < a->n; i++) {
        a->recs[i].data = a->bytes + at;
        at += a->recs[i].length;
    }
    l->given_way += (uint32_t)k;
}

void nic_live_records(nic_live *l, uint8_t addr, const hcc_record *recs, size_t n)
{
    nic_live_addr *a = find(l, addr);
    if (!a || a->sel.rule.mode == HMC_NONE)
        return;
    for (size_t i = 0; i < n; i++) {
        const hcc_record *r = &recs[i];
        if (a->have_newest && r->second < a->newest) {
            l->repairs++;
            continue;
        }
        a->newest = r->second;
        a->have_newest = 1;
        if (!hmc_select_record(&a->sel, r) || 9u + r->length > NIC_LIVE_BYTES)
            continue;
        give_way(l, a, 9u + r->length);
        size_t data = a->used - 9 * a->n;          /* the data bytes held */
        memcpy(a->bytes + data, r->data, r->length);
        a->recs[a->n] = *r;
        a->recs[a->n].data = a->bytes + data;
        a->n++;
        a->used += 9u + r->length;
    }
}

size_t nic_live_build(nic_live *l, uint32_t second, uint8_t *out, size_t max)
{
    if (max < 6)
        return 0;
    size_t at = 6;
    uint8_t count = 0;
    out[0] = SES_LIVE;
    put32(out + 1, second);
    for (uint8_t i = 0; i < l->n && count < 255; i++) {
        nic_live_addr *a = &l->a[i];
        if (!a->n)
            continue;
        uint8_t *seg;
        size_t len;
        if (hmc_segment_build(a->recs, a->n, a->layout, a->nl, &seg, &len))
            continue;
        if (at + 5 + len <= max) {
            out[at] = a->addr;
            put32(out + at + 1, (uint32_t)len);
            memcpy(out + at + 5, seg, len);
            at += 5 + len;
            count++;
            a->n = 0;
            a->used = 0;
        }
        free(seg);
    }
    out[5] = count;
    return count ? at : 0;
}

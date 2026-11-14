/* The head's live packet on a PC: three addresses on one link — `ALL`, `INTERVAL 10`, `NONE` — fed
 * 40 s of records as the writer's segments, a repaired record among them; the LIVE message parsed
 * by the archive's session and each segment decoded to exactly the records the send rule keeps,
 * the repair left out, `NONE` absent; nothing twice; a link too short for both addresses carrying
 * the first and the second waiting; an address past its bound keeping the newest. */
#include "../nic_live.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "session.h"

static int fails;
#define CHECK(x) do { if (!(x)) { fails++; fprintf(stderr, "%s:%d: %s\n", __FILE__, __LINE__, #x); } } while (0)

static hcc_field layout[HCC_MAX_LAYOUT];
static size_t nl;
static uint8_t pool[4096][32];
static hcc_record stream[4096];
static size_t nstream;

static const hcc_record *make(uint32_t second, uint8_t frame, uint8_t v)
{
    uint8_t *d = pool[nstream];
    for (int i = 0; i < 32; i++)
        d[i] = (uint8_t)(v + (i == 0 ? frame : 0));
    stream[nstream] = (hcc_record){second, frame, 1, 0, 0, 32, d};
    return &stream[nstream++];
}

/* the records a LIVE message carries for an address, decoded */
static size_t decoded(const uint8_t *msg, size_t n, uint8_t addr, hcc_record **out)
{
    uint32_t second;
    ses_seg *segs;
    size_t count, got = 0;
    *out = 0;
    CHECK(msg[0] == SES_LIVE);
    if (ses_live_parse(msg + 1, n - 1, &second, &segs, &count)) {
        CHECK(!"LIVE refused");
        return 0;
    }
    for (size_t i = 0; i < count; i++) {
        if (segs[i].address != addr)
            continue;
        hmc_object o;
        CHECK(!hmc_object_parse(segs[i].segment, segs[i].len, &o) && o.type == HMC_SEG);
        CHECK(!hmc_segment_records(&o, layout, nl, out, &got));
    }
    free(segs);
    return got;
}

static int same(const hcc_record *a, const hcc_record *b)
{
    return a->second == b->second && a->frame == b->frame && a->kind == b->kind && a->status == b->status
           && a->length == b->length && !memcmp(a->data, b->data, a->length);
}

static int has(const uint8_t *msg, size_t n, uint8_t addr)
{
    uint32_t second;
    ses_seg *segs;
    size_t count;
    int found = 0;
    if (ses_live_parse(msg + 1, n - 1, &second, &segs, &count))
        return 0;
    for (size_t i = 0; i < count; i++)
        found |= segs[i].address == addr;
    free(segs);
    return found;
}

static nic_live l;
static uint8_t msg[65536];

int main(void)
{
    hcc_field f = {0, 2, 0, 0, 0, 0};
    nl = hcc_compose_layout(&f, 1, layout);
    hmc_rule all = {HMC_ALL, 0, 0, 0, {0}}, every10 = {HMC_INTERVAL, 0, 10, 0, {0}},
             none = {HMC_NONE, 0, 0, 0, {0}};
    nic_live_init(&l);
    CHECK(!nic_live_add(&l, 0x52, &all, 0, layout, nl));
    CHECK(!nic_live_add(&l, 0x16, &every10, 0, layout, nl));
    CHECK(!nic_live_add(&l, 0x31, &none, 0, layout, nl));

    /* 40 s, four frames a second, handed over in 8 s segments; a repair in the third */
    hcc_record want52[512], want16[512];
    size_t n52 = 0, n16 = 0;
    hmc_selector ref;
    hmc_selector_init(&ref, &every10, 0);
    uint32_t t0 = 1790856000u;
    for (int seg = 0; seg < 5; seg++) {
        size_t from = nstream;
        for (uint32_t s = 0; s < 8; s++)
            for (uint8_t fr = 0; fr < 4; fr++) {
                const hcc_record *r = make(t0 + 8u * (uint32_t)seg + s, fr, (uint8_t)(s * 3 + seg));
                want52[n52++] = *r;
                if (hmc_select_record(&ref, r))
                    want16[n16++] = *r;
            }
        if (seg == 2)
            make(t0 + 3, 2, 0xEE);                   /* a frame of second 3 repaired late */
        nic_live_records(&l, 0x52, stream + from, nstream - from);
        nic_live_records(&l, 0x16, stream + from, nstream - from);
        nic_live_records(&l, 0x31, stream + from, nstream - from);
    }
    hmc_selector_free(&ref);
    CHECK(l.repairs == 2);                           /* once per address that took the segment */

    size_t len = nic_live_build(&l, t0 + 40, msg, sizeof msg);
    CHECK(len > 0);
    hcc_record *got;
    size_t n = decoded(msg, len, 0x52, &got);
    CHECK(n == n52);
    for (size_t i = 0; i < n && i < n52; i++)
        CHECK(same(&got[i], &want52[i]));
    hcc_free(got);
    n = decoded(msg, len, 0x16, &got);
    CHECK(n == n16 && n16 == 4);                     /* one every 10 s */
    for (size_t i = 0; i < n && i < n16; i++)
        CHECK(same(&got[i], &want16[i]));
    hcc_free(got);
    CHECK(!has(msg, len, 0x31));
    CHECK(nic_live_build(&l, t0 + 41, msg, sizeof msg) == 0);        /* nothing twice */

    /* a link that takes one address: the first goes, the second waits for the next */
    size_t from = nstream;
    for (uint32_t s = 40; s < 48; s++)
        for (uint8_t fr = 0; fr < 4; fr++)
            make(t0 + s, fr, (uint8_t)s);
    nic_live_records(&l, 0x52, stream + from, nstream - from);
    nic_live_records(&l, 0x16, stream + from, nstream - from);
    size_t whole = nic_live_build(&l, t0 + 48, msg, sizeof msg);
    CHECK(whole > 0 && has(msg, whole, 0x52) && has(msg, whole, 0x16));
    for (uint32_t s = 48; s < 56; s++)
        for (uint8_t fr = 0; fr < 4; fr++)
            make(t0 + s, fr, (uint8_t)s);
    from = nstream - 32;
    nic_live_records(&l, 0x52, stream + from, 32);
    nic_live_records(&l, 0x16, stream + from, 32);
    size_t short_len = nic_live_build(&l, t0 + 56, msg, whole - 40);
    CHECK(short_len > 0 && has(msg, short_len, 0x52) && !has(msg, short_len, 0x16));
    len = nic_live_build(&l, t0 + 57, msg, sizeof msg);
    CHECK(len > 0 && !has(msg, len, 0x52) && has(msg, len, 0x16));

    /* past the bound: the oldest give way, the newest kept */
    from = nstream;
    for (uint32_t s = 56; s < 56 + 100; s++)
        for (uint8_t fr = 0; fr < 4; fr++)
            make(t0 + s, fr, (uint8_t)s);
    nic_live_records(&l, 0x52, stream + from, nstream - from);
    size_t keep = NIC_LIVE_BYTES / (9 + 32);
    CHECK(l.given_way == 400 - keep);
    len = nic_live_build(&l, t0 + 156, msg, sizeof msg);
    n = decoded(msg, len, 0x52, &got);
    CHECK(n == keep);
    for (size_t i = 0; i < n && i < keep; i++)
        CHECK(same(&got[i], &stream[nstream - keep + i]));
    hcc_free(got);

    nic_live_free(&l);
    if (fails) {
        printf("test_live: %d FAILED\n", fails);
        return 1;
    }
    printf("test_live: all passed\n");
    return 0;
}

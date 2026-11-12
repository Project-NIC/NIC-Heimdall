/* The head's ingest on a PC: three units a second for twenty seconds through the buckets, the
 * close, the rules and the spool into HMC files, read back — holes filled, a late frame repaired
 * into a later segment, every segment within 16 kB and 8 s, each rule keeping what it says. */
#include "../nic_ingest.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int fails;
#define CHECK(x) do { if (!(x)) { fails++; fprintf(stderr, "%s:%d: %s\n", __FILE__, __LINE__, #x); } } while (0)

static hmc_writer w[256];
static int maxspan, maxbytes;

static int segment(void *ctx, uint8_t addr, const hcc_record *recs, size_t n)
{
    (void)ctx;
    uint32_t lo = recs[0].second, hi = recs[0].second;
    int bytes = 0;
    for (size_t i = 0; i < n; i++) {
        if (recs[i].second < lo) lo = recs[i].second;
        if (recs[i].second > hi) hi = recs[i].second;
        bytes += 9 + recs[i].length;
    }
    if ((int)(hi - lo) > maxspan && recs[0].kind != NIC_STREAM) maxspan = (int)(hi - lo);
    if (bytes > maxbytes) maxbytes = bytes;
    return hmc_writer_segment(&w[addr], recs, n);
}

static void open_file(uint8_t addr, uint8_t type)
{
    uint8_t sec[8 + 11] = {HMC_UNIT, 0, 0, 0, 11, 0, 0, 0, 1, type, 1, 1, 1, 128, 0, 0, 0, 0, 0};
    hmc_header h = {addr, 0, 1790856000u, 0x123456, 0, 0, sec, sizeof sec};
    CHECK(!hmc_writer_open(&w[addr], &h));
}

static void send(nic_ingest *g, uint32_t sec, uint8_t frame, uint8_t addr, const uint8_t *pl)
{
    uint8_t t[48];
    nic_header h = {(uint8_t)sec, frame, addr, 1, NIC_DATA, 0};
    nic_data_build(&h, pl, 32, t + 3);
    CHECK(!nic_trunk_complete(t, sec));
    nic_ingest_frame(g, t);
}

/* every record an address's file holds, in the order written */
static size_t read_back(uint8_t addr, hcc_record *out, uint8_t *pool)
{
    hmc_reader r;
    size_t n = 0, used = 0;
    CHECK(!hmc_reader_open(&r, w[addr].image, w[addr].size));
    hmc_sections force = {0}, hs;
    hmc_sections_parse(r.h.sections, r.h.sections_len, &hs);
    hmc_sections_apply(&force, &hs);
    for (size_t i = 0; i < r.entries; i++) {
        uint8_t *buf;
        hmc_object o;
        hcc_field lay[HCC_MAX_LAYOUT];
        size_t nl, k;
        hcc_record *recs;
        CHECK(!hmc_reader_object(&r, i, &buf, &o));
        CHECK(!hmc_hcc_layout(&force, lay, &nl));
        CHECK(!hmc_segment_records(&o, lay, nl, &recs, &k));
        for (size_t j = 0; j < k; j++) {
            out[n] = recs[j];
            memcpy(pool + used, recs[j].data, recs[j].length);
            out[n++].data = pool + used;
            used += recs[j].length;
        }
        hcc_free(recs);
        free(buf);
    }
    hmc_sections_free(&force);
    return n;
}

int main(void)
{
    static nic_ingest g;
    static hcc_record recs[8192];
    static uint8_t pool[8192 * 32];
    const uint32_t T0 = 1790856000u;
    nic_ingest_init(&g, segment, NULL);
    nic_ingest_unit quake = {0x52, 128, 0, {HMC_ALL, 0, 0, 0, {0}}};
    nic_ingest_unit pal = {0x61, 128, 1, {HMC_CHANGE, 0, 3600, 0, {0}}};
    nic_ingest_unit tesla = {0x71, 128, 0, {HMC_NONZERO, 0, 60, 0, {0}}};
    CHECK(!nic_ingest_add(&g, &quake) && !nic_ingest_add(&g, &pal) && !nic_ingest_add(&g, &tesla));
    CHECK(nic_ingest_add(&g, &quake) == -1);
    open_file(0x52, 5);
    open_file(0x61, 6);
    open_file(0x71, 7);
    uint8_t late[32];
    for (uint32_t s = 0; s < 20; s++) {
        for (unsigned f = 0; f < 128; f++) {
            uint8_t q[32], p[32] = {0}, z[32] = {0};
            for (int k = 0; k < 32; k++)
                q[k] = (uint8_t)(s * 7 + f * 3 + k);
            if (s == 3 && f >= 10 && f <= 12) {             /* three frames never arrive in time */
                if (f == 11)
                    memcpy(late, q, 32);
            } else {
                send(&g, T0 + s, (uint8_t)f, 0x52, q);
            }
            uint8_t blk[] = {3, 4, 1, 2, 3, 4, 5, 2, (uint8_t)(s / 5), 9, 0};
            memcpy(p, blk, sizeof blk);
            send(&g, T0 + s, (uint8_t)f, 0x61, p);
            if (s == 7 && f <= 20)
                z[0] = (uint8_t)(f + 1);
            send(&g, T0 + s, (uint8_t)f, 0x71, z);
        }
        if (s >= 1)
            nic_ingest_close(&g, T0 + s - 1);               /* one second after its end */
        if (s == 5)
            send(&g, T0 + 3, 11, 0x52, late);               /* a repair after its second closed */
    }
    nic_ingest_close(&g, T0 + 19);
    nic_ingest_flush(&g);
    CHECK(g.holes == 3 && g.repaired == 1 && g.dropped == 0);
    /* Quake: every frame of twenty seconds, two holes as FILLERs, the repair in a later segment */
    size_t n = read_back(0x52, recs, pool);
    CHECK(n == 20 * 128 + 1);
    int fillers = 0, repair = -1;
    for (size_t i = 0; i < n; i++) {
        if (recs[i].kind == NIC_FILLER) {
            fillers++;
            CHECK(recs[i].data[0] == 2 && recs[i].second == T0 + 3);
        }
        if (recs[i].second == T0 + 3 && recs[i].frame == 11 && recs[i].kind == NIC_DATA) {
            repair = (int)i;
            CHECK(!memcmp(recs[i].data, late, 32));
        }
    }
    CHECK(fillers == 3 && repair == 5 * 128);             /* right behind second 4, its own second 3 */
    CHECK(maxbytes <= (int)NIC_SPOOL_BYTES && maxspan <= (int)NIC_SPOOL_SECONDS);
    /* Palatine under CHANGE per block: the two blocks once, then module 5 at each change */
    n = read_back(0x61, recs, pool);
    CHECK(n == 2 + 3);
    CHECK(recs[0].form == 1 && recs[0].data[0] == 3 && recs[1].data[0] == 5);
    for (size_t i = 2; i < n; i++)
        CHECK(recs[i].data[0] == 5 && recs[i].second == T0 + 5 * i - 5);
    /* Tesla under NONZERO: the first frame, then the burst */
    n = read_back(0x71, recs, pool);
    CHECK(n == 1 + 21 && recs[0].second == T0 && recs[1].second == T0 + 7 && recs[21].frame == 20);
    if (fails) {
        printf("%d checks failed\n", fails);
        return 1;
    }
    printf("the head's ingest: every check passed (%u segments)\n", g.segments);
    return 0;
}

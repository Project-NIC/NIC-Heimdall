/* A stream down and a trial boot on a PC: an image through LOAD and its frames into a slot, on a
 * spur and cut for an Argus; every way a stream fails; the trial confirmed, and the trial that
 * resets before it confirms handing the boot back. */
#include "../nic_load.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int fails;
#define CHECK(x) do { if (!(x)) { fails++; fprintf(stderr, "%s:%d: %s\n", __FILE__, __LINE__, #x); } } while (0)

static uint8_t slots[2][65536];
static int erase_fails;

static int erase(void *ctx, uint8_t slot, uint32_t length)
{
    (void)ctx; (void)length;
    if (erase_fails)
        return -1;
    memset(slots[slot], 0xFF, sizeof slots[slot]);
    return 0;
}

static int write_(void *ctx, uint8_t slot, uint32_t off, const uint8_t *b, size_t n)
{
    (void)ctx;
    memcpy(slots[slot] + off, b, n);
    return 0;
}

typedef struct {
    uint8_t f[4096][NIC_SPUR_LEN];
    size_t len[4096], n;
} wire;

static void emit(void *ctx, const uint8_t *frame, size_t len)
{
    wire *w = ctx;
    memcpy(w->f[w->n], frame, len);
    w->len[w->n++] = len;
}

/* the unit's side of a whole stream: the LOAD, then each frame through its CRC */
static nic_answer run(nic_loader *l, wire *w, uint32_t frames, uint8_t slot, uint8_t payload, int skip,
                      int flip)
{
    nic_ctrl load = {0, 0, 0x52, 0, NIC_LOAD, (uint8_t)frames, (uint8_t)(frames >> 8), slot};
    nic_answer a = nic_loader_load(l, &load, payload);
    if (a.op != NIC_ACK)
        return a;
    for (size_t i = 0; i < w->n; i++) {
        nic_header h;
        const uint8_t *p;
        size_t plen;
        if ((int)i == skip)
            continue;
        if ((int)i == flip)
            w->f[i][10] ^= 1;
        if (nic_data_parse(w->f[i], w->len[i], &h, &p, &plen))
            a = nic_loader_miss(l);
        else
            a = nic_loader_frame(l, &h, p);
        if (a.op)
            return a;
    }
    return a;
}

int main(void)
{
    static uint8_t body[3000];
    static wire w;
    nic_loader l;
    nic_flash fl = {erase, write_, NULL, {sizeof slots[0], 1024}};
    for (size_t i = 0; i < sizeof body; i++)
        body[i] = (uint8_t)(i * 31 + 7);
    CHECK(nic_crc32(0, (const uint8_t *)"123456789", 9) == 0xCBF43926u);
    nic_image_header h = {5, 0, sizeof body, nic_crc32(0, body, sizeof body), 0x0102};
    uint8_t hb[32];
    nic_image_header g;
    nic_image_header_build(&h, hb);
    CHECK(!nic_image_header_parse(hb, &g) && g.type == 5 && g.length == 3000 && g.crc == h.crc && g.version == 0x0102);
    for (int i = 14; i < 32; i++)
        CHECK(hb[i] == 0);
    for (int payload = 8; payload <= 32; payload += 24) {
        uint32_t frames = nic_stream_frames(h.length, (uint8_t)payload);
        CHECK(frames == (1 + 94) * (32u / (unsigned)payload));
        /* whole: ACK, the slot holds the body, the image waits for APPLY */
        w.n = 0;
        nic_stream(&h, body, 0x52, 9, (uint8_t)payload, emit, &w);
        CHECK(w.n == frames && w.len[0] == (payload == 32 ? 40u : 16u));
        nic_loader_init(&l, &fl, 5);
        nic_answer a = run(&l, &w, frames, 0, (uint8_t)payload, -1, -1);
        CHECK(a.op == NIC_ACK && !memcmp(slots[0], body, sizeof body) && nic_loader_ready(&l));
        /* a frame left out, a frame failing its CRC: ERROR, the gap */
        nic_loader_init(&l, &fl, 5);
        a = run(&l, &w, frames, 0, (uint8_t)payload, 7, -1);
        CHECK(a.op == NIC_ERROR && a.code == NIC_E_GAP && !nic_loader_ready(&l));
        w.n = 0;
        nic_stream(&h, body, 0x52, 9, (uint8_t)payload, emit, &w);
        a = run(&l, &w, frames, 0, (uint8_t)payload, -1, 20);
        CHECK(a.op == NIC_ERROR && a.code == NIC_E_GAP);
        /* another TYPE's image; the count LOAD announced wrong */
        w.n = 0;
        nic_stream(&h, body, 0x52, 9, (uint8_t)payload, emit, &w);
        nic_loader_init(&l, &fl, 7);
        a = run(&l, &w, frames, 0, (uint8_t)payload, -1, -1);
        CHECK(a.op == NIC_ERROR && a.code == NIC_E_TYPE);
        nic_loader_init(&l, &fl, 5);
        a = run(&l, &w, frames + 1, 0, (uint8_t)payload, -1, -1);
        CHECK(a.op == NIC_ERROR && a.code == NIC_E_LENGTH);
        /* a header for the other slot */
        a = run(&l, &w, frames, 1, (uint8_t)payload, -1, -1);
        CHECK(a.op == NIC_ERROR && a.code == NIC_E_TYPE);
    }
    /* a body whose bytes are not the header's CRC-32 */
    nic_image_header bad = h;
    bad.crc ^= 1;
    w.n = 0;
    nic_stream(&bad, body, 0x52, 9, 32, emit, &w);
    nic_loader_init(&l, &fl, 5);
    nic_answer a = run(&l, &w, nic_stream_frames(h.length, 32), 0, 32, -1, -1);
    CHECK(a.op == NIC_ERROR && a.code == NIC_E_CRC32 && !nic_loader_ready(&l));
    /* a table past its slot; an erase that fails; a slot that does not exist */
    nic_image_header tab = {5, 1, 2000, nic_crc32(0, body, 2000), 1};
    w.n = 0;
    nic_stream(&tab, body, 0x52, 9, 32, emit, &w);
    a = run(&l, &w, nic_stream_frames(2000, 32), 1, 32, -1, -1);
    CHECK(a.op == NIC_ERROR && a.code == NIC_E_LENGTH);
    erase_fails = 1;
    a = run(&l, &w, 10, 0, 32, -1, -1);
    CHECK(a.op == NIC_ERROR && a.code == NIC_E_FLASH);
    erase_fails = 0;
    a = run(&l, &w, 10, 2, 32, -1, -1);
    CHECK(a.op == NIC_ERROR && a.code == NIC_E_VALUE);
    /* the trial boot */
    nic_boot b = {0, 0};
    CHECK(nic_boot_select(&b) == 0 && b.trial == 0);
    nic_boot_apply(&b);
    CHECK(nic_boot_select(&b) == 1 && b.trial == 2);           /* the new image starts, once */
    CHECK(nic_boot_confirm(&b, 1) && b.active == 1 && b.trial == 0);
    CHECK(nic_boot_select(&b) == 1);                           /* and stays */
    CHECK(!nic_boot_confirm(&b, 1));                           /* nothing to confirm twice */
    nic_boot_apply(&b);
    CHECK(nic_boot_select(&b) == 0 && b.trial == 2);           /* the next trial starts … */
    CHECK(nic_boot_select(&b) == 1 && b.trial == 0);           /* … resets unconfirmed: the old slot */
    CHECK(!nic_boot_confirm(&b, 1) && b.active == 1);
    if (fails) {
        printf("%d checks failed\n", fails);
        return 1;
    }
    printf("the stream and the trial boot: every check passed\n");
    return 0;
}

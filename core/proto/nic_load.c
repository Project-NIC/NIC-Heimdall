/* A stream down and a trial boot; see nic_load.h. */
#include "nic_load.h"

#include <string.h>

static void put32(uint8_t *p, uint32_t v)
{
    p[0] = (uint8_t)v; p[1] = (uint8_t)(v >> 8); p[2] = (uint8_t)(v >> 16); p[3] = (uint8_t)(v >> 24);
}

static uint32_t get32(const uint8_t *p)
{
    return (uint32_t)p[0] | (uint32_t)p[1] << 8 | (uint32_t)p[2] << 16 | (uint32_t)p[3] << 24;
}

uint32_t nic_crc32(uint32_t crc, const uint8_t *b, size_t n)
{
    crc = ~crc;
    for (size_t i = 0; i < n; i++) {
        crc ^= b[i];
        for (int k = 0; k < 8; k++)
            crc = (crc & 1) ? 0xEDB88320u ^ (crc >> 1) : crc >> 1;
    }
    return ~crc;
}

void nic_image_header_build(const nic_image_header *h, uint8_t out[NIC_PAYLOAD])
{
    memset(out, 0, NIC_PAYLOAD);
    out[0] = h->type;
    out[1] = h->slot;
    put32(out + 4, h->length);
    put32(out + 8, h->crc);
    out[12] = (uint8_t)h->version;
    out[13] = (uint8_t)(h->version >> 8);
}

int nic_image_header_parse(const uint8_t in[NIC_PAYLOAD], nic_image_header *h)
{
    h->type = in[0];
    h->slot = in[1];
    h->length = get32(in + 4);
    h->crc = get32(in + 8);
    h->version = (uint16_t)(in[12] | in[13] << 8);
    return h->slot <= 1 ? 0 : -1;
}

uint32_t nic_stream_frames(uint32_t length, uint8_t payload)
{
    return (1 + (length + NIC_PAYLOAD - 1) / NIC_PAYLOAD) * (NIC_PAYLOAD / payload);
}

/* ------------------------------------------------------------------ the unit's side */

void nic_loader_init(nic_loader *l, const nic_flash *f, uint8_t type)
{
    memset(l, 0, sizeof *l);
    l->flash = *f;
    l->type = type;
    l->slot = 0xFF;
}

static nic_answer answer(uint8_t op, uint8_t code)
{
    nic_answer a = {op, code};
    return a;
}

static nic_answer fail(nic_loader *l, uint8_t code)
{
    l->open = 0;
    l->slot = 0xFF;
    return answer(NIC_ERROR, code);
}

nic_answer nic_loader_load(nic_loader *l, const nic_ctrl *load, uint8_t payload)
{
    uint32_t frames = (uint32_t)load->arg0 | (uint32_t)load->arg1 << 8;
    uint8_t slot = load->arg2;
    l->open = 0;
    l->slot = 0xFF;
    if (slot > 1)
        return answer(NIC_ERROR, NIC_E_VALUE);
    if ((payload != NIC_PAYLOAD && payload != NIC_MINI_PAYLOAD) || frames == 0)
        return answer(NIC_ERROR, NIC_E_LENGTH);
    if (l->flash.erase(l->flash.ctx, slot, (uint32_t)frames * payload))
        return answer(NIC_ERROR, NIC_E_FLASH);
    l->open = 1;
    l->slot = slot;
    l->payload = payload;
    l->frames = frames;
    l->got = l->written = l->crc = 0;
    l->have_header = 0;
    return answer(NIC_ACK, 0);
}

nic_answer nic_loader_miss(nic_loader *l)
{
    if (!l->open)
        return answer(0, 0);
    return fail(l, NIC_E_GAP);
}

nic_answer nic_loader_frame(nic_loader *l, const nic_header *h, const uint8_t *payload)
{
    if (!l->open || h->kind != NIC_STREAM)
        return answer(0, 0);
    if (h->frame != (uint8_t)l->got)
        return fail(l, NIC_E_GAP);
    uint32_t at = l->got * l->payload;      /* the stream's byte offset of this frame */
    l->got++;
    const uint8_t *p = payload;
    size_t n = l->payload;
    if (at < NIC_PAYLOAD) {                 /* the header, in one frame or four */
        memcpy(l->head + at, p, n);
        if (at + n < NIC_PAYLOAD)
            return answer(0, 0);
        nic_image_header_parse(l->head, &l->h);
        if (l->h.type != l->type || l->h.slot != l->slot)
            return fail(l, NIC_E_TYPE);
        if (nic_stream_frames(l->h.length, l->payload) != l->frames
            || l->h.length > l->flash.slot_size[l->slot])
            return fail(l, NIC_E_LENGTH);
        l->have_header = 1;
        if (l->got < l->frames)
            return answer(0, 0);
        n = 0;
    } else {
        uint32_t body = at - NIC_PAYLOAD;
        if (body < l->h.length) {
            size_t take = l->h.length - body < n ? l->h.length - body : n;
            if (l->flash.write(l->flash.ctx, l->slot, body, p, take))
                return fail(l, NIC_E_FLASH);
            l->crc = nic_crc32(l->crc, p, take);
            l->written += (uint32_t)take;
        }
    }
    if (l->got < l->frames)
        return answer(0, 0);
    l->open = 0;
    if (l->crc != l->h.crc || l->written != l->h.length)
        return fail(l, NIC_E_CRC32);
    return answer(NIC_ACK, 0);
}

int nic_loader_ready(const nic_loader *l)
{
    return !l->open && l->slot == 0 && l->have_header && l->got == l->frames
           && l->crc == l->h.crc && l->written == l->h.length;
}

/* ------------------------------------------------------------------ the master's side */

void nic_stream(const nic_image_header *h, const uint8_t *body, uint8_t addr, uint8_t unix0,
                uint8_t payload, void (*emit)(void *ctx, const uint8_t *frame, size_t len), void *ctx)
{
    uint8_t chunk[NIC_PAYLOAD], f[NIC_SPUR_LEN];
    uint32_t total = nic_stream_frames(h->length, payload), k = 0;
    nic_header hd = {unix0, 0, addr, 0, NIC_STREAM, 0};
    for (uint32_t block = 0; k < total; block++) {
        if (block == 0) {
            nic_image_header_build(h, chunk);
        } else {
            uint32_t off = (block - 1) * NIC_PAYLOAD;
            uint32_t take = h->length - off < NIC_PAYLOAD ? h->length - off : NIC_PAYLOAD;
            memset(chunk, 0, NIC_PAYLOAD);
            memcpy(chunk, body + off, take);
        }
        for (uint32_t part = 0; part < NIC_PAYLOAD / payload; part++, k++) {
            hd.frame = (uint8_t)k;
            size_t len = nic_data_build(&hd, chunk + part * payload, payload, f);
            emit(ctx, f, len);
        }
    }
}

/* ------------------------------------------------------------------ the trial boot */

void nic_boot_apply(nic_boot *b)
{
    b->trial = 1;
}

uint8_t nic_boot_select(nic_boot *b)
{
    if (b->trial == 1) {            /* untried: start it once */
        b->trial = 2;
        return (uint8_t)(1 - b->active);
    }
    if (b->trial == 2)              /* started and never confirmed: the old slot */
        b->trial = 0;
    return b->active;
}

int nic_boot_confirm(nic_boot *b, uint8_t running)
{
    if (b->trial != 2 || running == b->active)
        return 0;
    b->active = running;
    b->trial = 0;
    return 1;
}

/* The head's side of the handset's GATT contract — see nic_gatt.h. */
#include "nic_gatt.h"

#include <string.h>

/* ------------------------------------------------------------------ writing JSON */

typedef struct {
    char *o;
    size_t max, n;
    int over;
} out_t;

static void raw(out_t *w, const char *s, size_t n)
{
    if (w->n + n > w->max) {
        w->over = 1;
        return;
    }
    memcpy(w->o + w->n, s, n);
    w->n += n;
}

static void lit(out_t *w, const char *s) { raw(w, s, strlen(s)); }

static void str(out_t *w, const char *s)
{
    raw(w, "\"", 1);
    for (; *s; s++) {
        unsigned char c = (unsigned char)*s;
        if (c == '"' || c == '\\') {
            char e[2] = {'\\', (char)c};
            raw(w, e, 2);
        } else if (c < 0x20) {
            char e[6] = {'\\', 'u', '0', '0', "0123456789abcdef"[c >> 4], "0123456789abcdef"[c & 15]};
            raw(w, e, 6);
        } else {
            raw(w, (const char *)&c, 1);
        }
    }
    raw(w, "\"", 1);
}

static void num(out_t *w, int64_t v)
{
    char b[24];
    int i = sizeof b;
    int neg = v < 0;
    uint64_t u = neg ? (uint64_t)(-(v + 1)) + 1 : (uint64_t)v;
    do {
        b[--i] = (char)('0' + u % 10);
        u /= 10;
    } while (u);
    if (neg)
        b[--i] = '-';
    raw(w, b + i, sizeof b - (size_t)i);
}

static void hex(out_t *w, const uint8_t *v, uint8_t n)
{
    raw(w, "\"", 1);
    for (uint8_t i = 0; i < n; i++) {
        char e[2] = {"0123456789abcdef"[v[i] >> 4], "0123456789abcdef"[v[i] & 15]};
        raw(w, e, 2);
    }
    raw(w, "\"", 1);
}

static const char *const BUS[3] = {"nodbus", "mini", "modbus"};

static void unit(out_t *w, uint8_t bus, uint8_t type, uint8_t n)
{
    lit(w, "\"bus\":");
    str(w, BUS[bus < 3 ? bus : 0]);
    lit(w, ",\"type\":");
    num(w, type);
    lit(w, ",\"n\":");
    num(w, n);
}

static void key(out_t *w, const char *k)
{
    str(w, k);
    raw(w, ":", 1);
}

size_t nic_status_write(const nic_status *s, char *o, size_t max)
{
    out_t w = {o, max, 0, 0};
    lit(&w, "{\"go\":");
    lit(&w, s->go ? "true" : "false");
    lit(&w, ",\"counter\":");
    num(&w, s->counter);
    lit(&w, ",\"nodes\":[");
    for (uint16_t i = 0; i < s->nnodes; i++) {
        const nic_st_node *d = &s->nodes[i];
        lit(&w, i ? ",{\"n\":" : "{\"n\":");
        num(&w, d->n);
        lit(&w, ",\"bus\":");
        str(&w, BUS[d->bus < 3 ? d->bus : 0]);
        lit(&w, ",\"type\":");
        num(&w, d->type);
        if (d->label) {
            lit(&w, ",\"label\":");
            str(&w, d->label);
        }
        lit(&w, ",\"ok\":");
        lit(&w, d->ok ? "true" : "false");
        lit(&w, ",\"sensors\":[");
        for (uint8_t j = 0; j < d->nsensors; j++) {
            lit(&w, j ? ",{\"id\":" : "{\"id\":");
            num(&w, d->sensors[j].id);
            lit(&w, ",\"name\":");
            str(&w, d->sensors[j].name);
            lit(&w, ",\"h\":");
            num(&w, d->sensors[j].h);
            lit(&w, "}");
        }
        lit(&w, "]}");
    }
    lit(&w, "]");
    if (s->terms) {
        lit(&w, ",\"corr\":{\"total_ns\":");
        num(&w, s->total_ns);
        lit(&w, ",\"terms\":[");
        for (uint8_t i = 0; i < s->nterms; i++) {
            const nic_st_term *t = &s->terms[i];
            lit(&w, i ? ",{" : "{");
            key(&w, "id");
            str(&w, t->id);
            lit(&w, ",");
            key(&w, "label");
            str(&w, t->label);
            if (t->m >= 0) {
                lit(&w, ",\"m\":");
                num(&w, t->m);
                lit(&w, ",\"ns_per_m\":");
                num(&w, t->ns_per_m);
            }
            lit(&w, ",\"ns\":");
            num(&w, t->ns);
            lit(&w, t->ranged ? ",\"src\":\"ranged\"}" : ",\"src\":\"typed\"}");
        }
        lit(&w, "]}");
    }
    if (s->npending) {
        lit(&w, ",\"mirror\":[");
        for (uint8_t i = 0; i < s->npending; i++) {
            const nic_st_pending *p = &s->pending[i];
            lit(&w, i ? ",{" : "{");
            unit(&w, p->bus, p->type, p->n);
            lit(&w, ",\"reg\":");
            num(&w, p->reg);
            lit(&w, ",\"v\":");
            hex(&w, p->v, p->len);
            lit(&w, p->state == 3 ? ",\"state\":\"sent\"}" : ",\"state\":\"edited\"}");
        }
        lit(&w, "]");
    }
    lit(&w, "}\n");
    return w.over ? 0 : w.n;
}

size_t nic_mirror_line(uint8_t bus, uint8_t type, uint8_t n, const nic_st_reg *regs, uint16_t nregs,
                       char *o, size_t max)
{
    out_t w = {o, max, 0, 0};
    lit(&w, "{\"unit\":{");
    unit(&w, bus, type, n);
    lit(&w, "},\"regs\":[");
    for (uint16_t i = 0; i < nregs; i++) {
        lit(&w, i ? ",{\"reg\":" : "{\"reg\":");
        num(&w, regs[i].reg);
        lit(&w, ",\"v\":");
        hex(&w, regs[i].v, regs[i].len);
        lit(&w, regs[i].ro ? ",\"ro\":true}" : ",\"ro\":false}");
    }
    lit(&w, "]}\n");
    return w.over ? 0 : w.n;
}

void nic_notify_start(nic_notify *n, const char *doc, size_t len)
{
    n->doc = doc;
    n->len = len;
    n->at = 0;
}

size_t nic_notify_next(nic_notify *n, size_t mtu, uint8_t *out)
{
    size_t k = n->len - n->at;
    if (k > mtu)
        k = mtu;
    memcpy(out, n->doc + n->at, k);
    n->at += k;
    return k;
}

/* ------------------------------------------------------------------ reading a command's JSON */

typedef struct {
    const char *p, *end;
} in_t;

static void ws(in_t *r)
{
    while (r->p < r->end && (*r->p == ' ' || *r->p == '\t' || *r->p == '\n' || *r->p == '\r'))
        r->p++;
}

static int eat(in_t *r, char c)
{
    ws(r);
    if (r->p < r->end && *r->p == c) {
        r->p++;
        return 1;
    }
    return 0;
}

/* A string into out (cut to max − 1, NUL-closed), escapes kept to the simple ones. */
static int rstr(in_t *r, char *out, size_t max)
{
    size_t k = 0;
    if (!eat(r, '"'))
        return -1;
    while (r->p < r->end && *r->p != '"') {
        char c = *r->p++;
        if (c == '\\') {
            if (r->p >= r->end)
                return -1;
            c = *r->p++;
            if (c != '"' && c != '\\' && c != '/')
                return -1;
        }
        if (k + 1 < max)
            out[k++] = c;
    }
    if (r->p >= r->end)
        return -1;
    r->p++;
    if (max)
        out[k] = 0;
    return 0;
}

static int rnum(in_t *r, int32_t *v)
{
    ws(r);
    int neg = 0, digits = 0;
    int64_t x = 0;
    if (r->p < r->end && *r->p == '-') {
        neg = 1;
        r->p++;
    }
    while (r->p < r->end && *r->p >= '0' && *r->p <= '9' && digits < 10) {
        x = x * 10 + (*r->p++ - '0');
        digits++;
    }
    if (!digits || (r->p < r->end && *r->p >= '0' && *r->p <= '9'))
        return -1;
    *v = (int32_t)(neg ? -x : x);
    return 0;
}

static int skip(in_t *r, int depth)
{
    ws(r);
    if (r->p >= r->end || depth > 8)
        return -1;
    char c = *r->p;
    if (c == '"') {
        char d[1];
        return rstr(r, d, 0);
    }
    if (c == '{' || c == '[') {
        char close = c == '{' ? '}' : ']';
        r->p++;
        if (eat(r, close))
            return 0;
        do {
            if (c == '{') {
                char d[1];
                if (rstr(r, d, 0) || !eat(r, ':'))
                    return -1;
            }
            if (skip(r, depth + 1))
                return -1;
        } while (eat(r, ','));
        return eat(r, close) ? 0 : -1;
    }
    while (r->p < r->end && *r->p != ',' && *r->p != '}' && *r->p != ']' && *r->p != ' ')
        r->p++;
    return 0;
}

static int terms(in_t *r, nic_gatt_cmd *c)
{
    if (!eat(r, '['))
        return -1;
    if (eat(r, ']'))
        return 0;
    do {
        if (c->nterms == NIC_GATT_TERMS || !eat(r, '{'))
            return -1;
        int have_id = 0, have_m = 0;
        do {
            char k[8];
            if (rstr(r, k, sizeof k) || !eat(r, ':'))
                return -1;
            if (!strcmp(k, "id")) {
                if (rstr(r, c->terms[c->nterms].id, sizeof c->terms[0].id))
                    return -1;
                have_id = 1;
            } else if (!strcmp(k, "m")) {
                if (rnum(r, &c->terms[c->nterms].m))
                    return -1;
                have_m = 1;
            } else if (skip(r, 1)) {
                return -1;
            }
        } while (eat(r, ','));
        if (!eat(r, '}') || !have_id || !have_m)
            return -1;
        c->nterms++;
    } while (eat(r, ','));
    return eat(r, ']') ? 0 : -1;
}

int nic_gatt_parse(const char *json, size_t n, nic_gatt_cmd *c)
{
    in_t r = {json, json + n};
    char cmd[16] = "", bus[8] = "";
    char v[2 * NIC_GATT_VALUE + 2];
    int32_t node = -1, out = -1, type = -1, n_ = -1, reg = -1;
    int have_set = 0, have_v = 0;
    memset(c, 0, sizeof *c);
    if (!eat(&r, '{'))
        return -1;
    if (!eat(&r, '}'))
        do {
            char k[16];
            if (rstr(&r, k, sizeof k) || !eat(&r, ':'))
                return -1;
            if (!strcmp(k, "cmd")) {
                if (rstr(&r, cmd, sizeof cmd))
                    return -1;
            } else if (!strcmp(k, "node")) {
                if (rnum(&r, &node))
                    return -1;
            } else if (!strcmp(k, "out")) {
                if (rnum(&r, &out))
                    return -1;
            } else if (!strcmp(k, "set")) {
                if (terms(&r, c))
                    return -1;
                have_set = 1;
            } else if (!strcmp(k, "bus")) {
                if (rstr(&r, bus, sizeof bus))
                    return -1;
            } else if (!strcmp(k, "type")) {
                if (rnum(&r, &type))
                    return -1;
            } else if (!strcmp(k, "n")) {
                if (rnum(&r, &n_))
                    return -1;
            } else if (!strcmp(k, "reg")) {
                if (rnum(&r, &reg))
                    return -1;
            } else if (!strcmp(k, "v")) {
                v[sizeof v - 1] = 0;
                if (rstr(&r, v, sizeof v) || strlen(v) >= sizeof v - 1)
                    return -1;
                have_v = 1;
            } else if (skip(&r, 0)) {
                return -1;
            }
        } while (eat(&r, ','));
    if (!eat(&r, '}'))
        return -1;
    ws(&r);
    if (r.p != r.end)
        return -1;
    if (!strcmp(cmd, "refresh"))
        c->kind = NIC_GC_REFRESH;
    else if (!strcmp(cmd, "recommission"))
        c->kind = NIC_GC_RECOMMISSION;
    else if (!strcmp(cmd, "provision") && node >= 1 && node <= 255)
        c->kind = NIC_GC_PROVISION, c->node = node;
    else if (!strcmp(cmd, "corr") && have_set && c->nterms)
        c->kind = NIC_GC_CORR;
    else if (!strcmp(cmd, "card") && (out == 0 || out == 1))
        c->kind = NIC_GC_CARD, c->slot = out;
    else if (!strcmp(cmd, "set") || !strcmp(cmd, "mirror")) {
        int b = !strcmp(bus, "nodbus") ? 0 : !strcmp(bus, "mini") ? 1 : !strcmp(bus, "modbus") ? 2 : -1;
        if (b < 0 || type < 1 || type > 61 || n_ < 0 || n_ > 15)
            return -1;
        c->bus = (uint8_t)b;
        c->type = (uint8_t)type;
        c->n = (uint8_t)n_;
        c->kind = NIC_GC_MIRROR;
        if (!strcmp(cmd, "set")) {
            size_t h = have_v ? strlen(v) : 1;
            if (reg < 0 || reg > 0xFFFF || h % 2 || h / 2 > NIC_GATT_VALUE)
                return -1;
            for (size_t i = 0; i < h; i++) {
                char x = v[i];
                int d = x >= '0' && x <= '9' ? x - '0' : x >= 'a' && x <= 'f' ? x - 'a' + 10
                      : x >= 'A' && x <= 'F' ? x - 'A' + 10 : -1;
                if (d < 0)
                    return -1;
                c->v[i / 2] = (uint8_t)(i % 2 ? c->v[i / 2] | d : d << 4);
            }
            c->vlen = (uint8_t)(h / 2);
            c->reg = (uint16_t)reg;
            c->kind = NIC_GC_SET;
        }
    } else
        return -1;
    return 0;
}

/* ------------------------------------------------------------------ the command characteristic */

void nic_gatt_rx_init(nic_gatt_rx *r)
{
    memset(r, 0, sizeof *r);
}

static int cobs_decode(const uint8_t *in, size_t n, uint8_t *out, size_t *len)
{
    size_t i = 0, o = 0;
    while (i < n) {
        uint8_t code = in[i++];
        if (!code || i + code - 1 > n)
            return -1;
        for (uint8_t k = 1; k < code; k++)
            out[o++] = in[i++];
        if (code < 0xFF && i < n)
            out[o++] = 0;
    }
    *len = o;
    return 0;
}

enum nic_gatt_result nic_gatt_write(nic_gatt_rx *r, nic_cmd_keys *k, const uint8_t *b, size_t n,
                                    size_t *used, nic_gatt_cmd *c, enum nic_cmd_result *why)
{
    size_t i = 0;
    *why = NIC_CMD_OK;
    while (i < n && b[i] != 0) {
        if (r->n < sizeof r->buf)
            r->buf[r->n++] = b[i];
        else
            r->overflow = 1;
        i++;
    }
    if (i == n) {
        *used = n;
        return NIC_GW_MORE;
    }
    *used = i + 1;                                      /* the closing zero */
    uint8_t msg[sizeof r->buf];
    size_t len = 0;
    int broken = r->overflow || cobs_decode(r->buf, r->n, msg, &len);
    r->n = 0;
    r->overflow = 0;
    if (broken || len < 13 || msg[0] != NIC_MSG_COMMAND)
        return NIC_GW_BAD;                              /* the phone writes COMMAND alone */
    *why = nic_cmd_take(k, msg, len);
    if (*why != NIC_CMD_OK && *why != NIC_CMD_READ)
        return NIC_GW_REFUSED;
    if (nic_gatt_parse((const char *)msg + 5, len - 13, c))
        return NIC_GW_BAD;
    if (*why == NIC_CMD_READ && c->kind != NIC_GC_REFRESH)
        return NIC_GW_REFUSED;                          /* under counter 0 only a read */
    return NIC_GW_TAKEN;
}

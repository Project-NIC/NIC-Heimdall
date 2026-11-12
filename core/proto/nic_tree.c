/* The head's station tree; see nic_tree.h. */
#include "nic_tree.h"

#include <string.h>

#include "nic_sweep.h"

#define ARGUS_TYPE 3u

void nic_tree_init(nic_tree *t)
{
    t->n = 0;
}

static void drop_where(nic_tree *t, uint8_t card, int argus)  /* argus −1: the card and all under it */
{
    uint16_t k = 0;
    for (uint16_t i = 0; i < t->n; i++) {
        const nic_seen *s = &t->u[i];
        int gone = s->card == card && (argus < 0 || s->argus == (uint8_t)argus);
        if (!gone)
            t->u[k++] = *s;
    }
    t->n = k;
}

static int put_rows(nic_tree *t, uint8_t card, uint8_t argus, const uint8_t table[NIC_PAYLOAD])
{
    for (int i = 0; i < 8; i++) {
        const uint8_t *r = table + 4 * i;
        if (!r[0])
            continue;
        if (t->n >= NIC_TREE_MAX)
            return -1;
        nic_seen *s = &t->u[t->n++];
        memset(s, 0, sizeof *s);
        s->bus = argus ? NIC_MINI : NIC_NODBUS;
        s->addr = r[0];
        s->card = card;
        s->argus = argus;
        s->slot = (uint8_t)(argus ? i : i + 1);
        s->port = r[1];
        s->rung = r[2];
        s->status = r[3];
    }
    return 0;
}

int nic_tree_card(nic_tree *t, uint8_t card, const uint8_t table[NIC_PAYLOAD])
{
    drop_where(t, card, -1);
    return put_rows(t, card, 0, table);
}

int nic_tree_argus(nic_tree *t, uint8_t card, uint8_t argus, const uint8_t table[NIC_PAYLOAD])
{
    drop_where(t, card, argus);
    return put_rows(t, card, argus, table);
}

static nic_seen *row_at(nic_tree *t, uint8_t card, uint8_t argus, uint8_t slot)
{
    for (uint16_t i = 0; i < t->n; i++)
        if (t->u[i].card == card && t->u[i].argus == argus && t->u[i].slot == slot)
            return &t->u[i];
    return NULL;
}

int nic_tree_tags(nic_tree *t, uint8_t card, uint8_t argus, uint8_t slot, uint16_t tag, uint16_t tag2)
{
    nic_seen *s = row_at(t, card, argus, slot);
    if (!s)
        return -1;
    s->tag = tag;
    s->tag2 = tag2;
    s->tagged = 1;
    return 0;
}

static const nic_seen *unit(const nic_tree *t, uint8_t card, uint8_t addr)
{
    for (uint16_t i = 0; i < t->n; i++)
        if (t->u[i].card == card && !t->u[i].argus && t->u[i].addr == addr)
            return &t->u[i];
    return NULL;
}

/* The NUMBER whose table carries `number`'s positions, and the row offset: its own at 0, or the
 * first NUMBER of the same Argus — the same tag on the same card — at 4. */
static int owner(const nic_tree *t, uint8_t card, uint8_t number, uint8_t *first)
{
    const nic_seen *a = unit(t, card, number);
    if (!a || a->addr >> 4 != ARGUS_TYPE)
        return -1;
    for (uint16_t i = 0; i < t->n; i++)
        if (t->u[i].card == card && t->u[i].argus == number) {
            *first = number;
            return 0;
        }
    if (!a->tagged)
        return -1;
    for (uint16_t i = 0; i < t->n; i++) {
        const nic_seen *b = &t->u[i];
        if (b->card == card && !b->argus && b != a && b->tagged && b->tag == a->tag && b->tag2 == a->tag2
            && b->slot < a->slot) {
            *first = b->addr;
            return 4;
        }
    }
    return -1;
}

const nic_seen *nic_tree_sonde(const nic_tree *t, uint8_t card, uint8_t number, unsigned pos)
{
    uint8_t first;
    int off = pos < 4 ? owner(t, card, number, &first) : -1;
    if (off < 0)
        return NULL;
    for (uint16_t i = 0; i < t->n; i++)
        if (t->u[i].card == card && t->u[i].argus == first && t->u[i].slot == off + pos)
            return &t->u[i];
    return NULL;
}

/* ------------------------------------------------------------------ the numbers */

static unsigned type_of(uint8_t bus, uint8_t addr) { return bus == NIC_MODBUS ? addr >> 2 : addr >> 4; }
static unsigned num_of(uint8_t bus, uint8_t addr) { return bus == NIC_MODBUS ? addr & 3u : addr & 15u; }
static unsigned dflt(uint8_t bus) { return bus == NIC_MODBUS ? 3u : 15u; }

/* the lowest number of the type that no mirror row and no decision so far holds */
static int lowest_free(uint8_t bus, unsigned type, const nic_mirror_row *m, size_t nm,
                       uint8_t taken[3][256], uint8_t *addr)
{
    for (unsigned n = bus == NIC_MODBUS ? 0 : 1; n < dflt(bus); n++) {
        uint8_t a;
        if (nic_addr_pack((enum nic_bus)bus, type, n, &a))
            return -1;
        int held = taken[bus][a];
        for (size_t i = 0; i < nm && !held; i++)
            held = m[i].bus == bus && m[i].addr == a;
        if (!held) {
            *addr = a;
            return 0;
        }
    }
    return -1;
}

size_t nic_tree_number(const nic_tree *t, const nic_mirror_row *m, size_t nm, int service,
                       nic_decision *out, uint8_t *missing)
{
    uint8_t taken[3][256], used[NIC_TREE_MAX];   /* numbers handed out; mirror rows matched */
    memset(taken, 0, sizeof taken);
    memset(used, 0, sizeof used);
    if (!service)
        nm = 0;
    if (nm > NIC_TREE_MAX)
        nm = NIC_TREE_MAX;
    for (uint16_t i = 0; i < t->n; i++) {
        out[i].seen = i;
        out[i].verdict = 0xFF;
        out[i].to = t->u[i].addr;
        out[i].ask = 0;
        if (!t->u[i].tagged)
            out[i].verdict = NIC_V_UNTAGGED;
    }
    /* a tag it knows, on the number the mirror holds */
    for (uint16_t i = 0; i < t->n; i++) {
        const nic_seen *s = &t->u[i];
        for (size_t j = 0; j < nm && out[i].verdict == 0xFF; j++)
            if (!used[j] && m[j].bus == s->bus && m[j].tag == s->tag && m[j].tag2 == s->tag2
                && m[j].addr == s->addr) {
                used[j] = 1;
                out[i].verdict = NIC_V_CONFIRMED;
            }
    }
    /* a tag it knows, on another number: back to the mirror's, the lowest left first */
    for (uint16_t i = 0; i < t->n; i++) {
        const nic_seen *s = &t->u[i];
        for (size_t j = 0; j < nm && out[i].verdict == 0xFF; j++)
            if (!used[j] && m[j].bus == s->bus && m[j].tag == s->tag && m[j].tag2 == s->tag2) {
                used[j] = 1;
                out[i].verdict = NIC_V_MOVED;
                out[i].to = m[j].addr;
            }
    }
    for (size_t j = 0; j < nm; j++)
        missing[j] = !used[j];
    uint8_t *offered = used;                /* now: a missing row already put to the human */
    /* the rest: unknown tags, and a known box grown by a slot */
    for (uint16_t i = 0; i < t->n; i++) {
        const nic_seen *s = &t->u[i];
        if (out[i].verdict != 0xFF)
            continue;
        unsigned type = type_of(s->bus, s->addr);
        int fresh = num_of(s->bus, s->addr) == dflt(s->bus), shared = 0;
        for (uint16_t k = 0; k < t->n; k++)
            shared |= k != i && t->u[k].bus == s->bus && t->u[k].addr == s->addr;
        if (service && fresh) {
            size_t j;
            for (j = 0; j < nm; j++)
                if (missing[j] && !offered[j] && m[j].bus == s->bus && type_of(m[j].bus, m[j].addr) == type)
                    break;
            if (j < nm) {                   /* the human answers: the old number, the series on */
                offered[j] = 1;
                out[i].verdict = NIC_V_REPLACED;
                out[i].to = m[j].addr;
                out[i].ask = 1;
                continue;
            }
        }
        uint8_t a;
        if (lowest_free(s->bus, type, m, nm, taken, &a)) {
            out[i].verdict = NIC_V_FULL;
            continue;
        }
        taken[s->bus][a] = 1;
        out[i].to = a;
        out[i].verdict = !service || fresh ? NIC_V_NEW : shared ? NIC_V_RENUMBERED : NIC_V_STRANGER;
        out[i].ask = (uint8_t)(service && fresh);  /* in service nothing fresh is numbered unasked */
    }
    return t->n;
}

void nic_tree_assign(const nic_tree *t, const nic_decision *d, uint8_t out[NIC_CTRL_LEN])
{
    const nic_seen *s = &t->u[d->seen];
    nic_found f = {s->tag, s->tag2, s->addr};
    nic_assign(&f, d->to, s->slot, out);
}

void nic_tree_apply(nic_tree *t, const nic_decision *d)
{
    nic_seen *s = &t->u[d->seen];
    uint8_t was = s->addr;
    s->addr = d->to;
    if (s->argus || was == d->to)
        return;
    for (uint16_t i = 0; i < t->n; i++)     /* an Argus's table follows its first NUMBER */
        if (t->u[i].card == s->card && t->u[i].argus == was)
            t->u[i].argus = d->to;
}

void nic_tree_roster(const nic_tree *t, nic_roster *r)
{
    nic_roster_init(r);
    for (uint16_t i = 0; i < t->n; i++) {   /* the cards first, so a unit finds its parent */
        const nic_seen *s = &t->u[i];
        if (s->argus || nic_roster_find(r, NIC_NODBUS, s->card) != NIC_NONE)
            continue;
        nic_unit c = {NIC_NODBUS, s->card, NIC_NONE, 0, 0};
        nic_roster_put(r, &c);
    }
    for (uint16_t i = 0; i < t->n; i++) {
        const nic_seen *s = &t->u[i];
        if (s->argus)
            continue;
        nic_unit u = {NIC_NODBUS, s->addr, nic_roster_find(r, NIC_NODBUS, s->card), s->port, s->slot};
        nic_roster_put(r, &u);
    }
    for (uint16_t i = 0; i < t->n; i++) {
        const nic_seen *s = &t->u[i];
        if (!s->argus)
            continue;
        uint8_t number = s->argus;          /* rows 4–7 under the Argus's second NUMBER */
        if (s->slot >= 4) {
            const nic_seen *a = unit(t, s->card, s->argus);
            for (uint16_t k = 0; a && k < t->n; k++) {
                const nic_seen *b = &t->u[k];
                if (b->card == s->card && !b->argus && b != a && b->tagged && a->tagged
                    && b->tag == a->tag && b->tag2 == a->tag2)
                    number = b->addr;
            }
        }
        nic_unit u = {NIC_MINI, s->addr, nic_roster_find(r, NIC_NODBUS, number), s->port,
                      (uint8_t)(s->slot & 3u)};
        nic_roster_put(r, &u);
    }
}

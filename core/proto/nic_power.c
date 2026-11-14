/* The head's power — see nic_power.h. */
#include "nic_power.h"

#include <string.h>

static int64_t full_mas(const nic_power *p) { return (int64_t)p->cfg.capacity_mah * 3600; }

void nic_pwr_init(nic_power *p, const nic_pwr_cfg *cfg, uint8_t bms_pct)
{
    memset(p, 0, sizeof *p);
    p->cfg = *cfg;
    p->charge_mas = full_mas(p) * (bms_pct > 100 ? 100 : bms_pct) / 100;
}

uint8_t nic_pwr_pct(const nic_power *p)
{
    return (uint8_t)(p->charge_mas * 100 / full_mas(p));
}

/* The count moved on by a reading: integrated over the time since the last, anchored on float. */
static void count(nic_power *p, const nic_pwr_reading *r)
{
    if (p->have_last && r->second > p->last && r->second - p->last <= 3600)
        p->charge_mas += (int64_t)r->current_ma * (r->second - p->last);
    p->last = r->second;
    p->have_last = 1;
    if (r->floating)
        p->charge_mas = full_mas(p) * p->cfg.anchor_pct / 100;
    if (p->charge_mas < 0)
        p->charge_mas = 0;
    if (p->charge_mas > full_mas(p))
        p->charge_mas = full_mas(p);
}

enum nic_pwr_verdict nic_pwr_running(nic_power *p, const nic_pwr_reading *r)
{
    count(p, r);
    if (p->charge_mas < full_mas(p) * p->cfg.shed_pct / 100)
        return NIC_PV_SHED_DEPLETED;
    if (r->pack_mv < p->cfg.low_mv)
        return NIC_PV_SHED_LOW_VOLTAGE;
    return NIC_PV_RUN;
}

enum nic_pwr_verdict nic_pwr_survival(nic_power *p, const nic_pwr_reading *r)
{
    count(p, r);
    if (r->pack_mv >= p->cfg.recover_mv) {
        if (!p->recovering) {
            p->recovering = 1;
            p->recovered_since = r->second;
        }
    } else {
        p->recovering = 0;
    }
    if (p->charge_mas >= full_mas(p) * p->cfg.wake_pct / 100 && p->recovering
        && r->second - p->recovered_since >= NIC_PWR_RECOVER_S)
        return NIC_PV_WAKE;
    return NIC_PV_RUN;
}

/* ------------------------------------------------------------------ the ordered shutdown */

enum { PH_MARKER = 0, PH_FLOOR_END, PH_FLOOR_WAIT, PH_CLK, PH_PWR, PH_FLUSH, PH_CARD_END, PH_GATE, PH_DOWN,
       PH_SURVIVAL, PH_DONE };

void nic_shut_start(nic_shutdown *s, const uint8_t *cards, const uint32_t *load_mw, const uint8_t *units,
                    uint8_t n)
{
    uint32_t load[NIC_PWR_FLOORS];
    memset(s, 0, sizeof *s);
    s->n = n > NIC_PWR_FLOORS ? NIC_PWR_FLOORS : n;
    for (uint8_t i = 0; i < s->n; i++) {            /* highest-drawing first, a stable insertion */
        uint8_t j = i;
        while (j > 0 && load[j - 1] < load_mw[i]) {
            load[j] = load[j - 1];
            s->card[j] = s->card[j - 1];
            s->units[j] = s->units[j - 1];
            j--;
        }
        load[j] = load_mw[i];
        s->card[j] = cards[i];
        s->units[j] = units[i];
    }
}

void nic_shut_ack(nic_shutdown *s)
{
    if (s->phase == PH_FLOOR_WAIT)
        s->acks++;
}

nic_shut_step nic_shut_next(nic_shutdown *s, uint32_t now)
{
    nic_shut_step st = {NIC_SA_WAIT, 0};
    switch (s->phase) {
    case PH_MARKER:
        st.act = NIC_SA_MARKER;
        s->phase = s->n ? PH_FLOOR_END : PH_FLUSH;
        s->i = 0;
        break;
    case PH_FLOOR_END:
        st.act = NIC_SA_END_FLOOR;
        st.card = s->card[s->i];
        s->acks = 0;
        s->since = now;
        s->phase = PH_FLOOR_WAIT;
        break;
    case PH_FLOOR_WAIT:
        if (s->acks < s->units[s->i] && now - s->since < NIC_PWR_FLOOR_WAIT)
            break;
        if (++s->i < s->n) {
            s->phase = PH_FLOOR_END;
            return nic_shut_next(s, now);
        }
        s->i = 0;
        s->phase = PH_CLK;
        return nic_shut_next(s, now);
    case PH_CLK:                                     /* the clock first, the feed second, card by card */
        st.act = NIC_SA_CLK_OFF;
        st.card = s->card[s->i];
        s->phase = PH_PWR;
        break;
    case PH_PWR:
        st.act = NIC_SA_PWR_OFF;
        st.card = s->card[s->i];
        if (++s->i < s->n) {
            s->phase = PH_CLK;
        } else {
            s->phase = PH_FLUSH;
            s->i = 0;
        }
        break;
    case PH_FLUSH:
        st.act = NIC_SA_FLUSH;
        s->phase = s->n ? PH_CARD_END : PH_GATE;
        s->i = 0;
        break;
    case PH_CARD_END:
        st.act = NIC_SA_END_CARD;
        st.card = s->card[s->i];
        if (++s->i >= s->n)
            s->phase = PH_GATE;
        break;
    case PH_GATE:
        st.act = NIC_SA_KRONOS_GATE0;
        s->phase = PH_DOWN;
        break;
    case PH_DOWN:
        st.act = NIC_SA_KRONOS_DOWN;
        s->phase = PH_SURVIVAL;
        break;
    case PH_SURVIVAL:
        st.act = NIC_SA_SURVIVAL;
        s->phase = PH_DONE;
        break;
    default:
        break;
    }
    return st;
}

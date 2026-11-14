/* The head's power on a PC: the reference pack of 628 Ah counted over a day of minute readings
 * to the mA·s; float re-anchoring the count at the charge stop; a gap of hours not integrated; the
 * shed at the threshold to the minute, and on a low pack voltage; survival waking at 50 % only after
 * five unbroken minutes over the recovery voltage; the ordered shutdown in its order — the marker,
 * the floors highest-drawing first, each floor's END closed by its ACKs or by 10 s, the clock before
 * the feed card by card, the flush, the cards' END, Kronos gated and down, survival; a head with no
 * cards. */
#include "../nic_power.h"

#include <stdio.h>

static int fails;
#define CHECK(x) do { if (!(x)) { fails++; fprintf(stderr, "%s:%d: %s\n", __FILE__, __LINE__, #x); } } while (0)

int main(void)
{
    nic_pwr_cfg cfg = {628000, 80, 20, 50, 12000, 13200};
    nic_power p;
    nic_pwr_init(&p, &cfg, 60);
    CHECK(nic_pwr_pct(&p) == 60);
    int64_t start = p.charge_mas;

    /* a day at −2 A, minute by minute: 48 Ah gone */
    uint32_t t = 1790856000u;
    nic_pwr_reading r = {t, 13100, -2000, 0};
    CHECK(nic_pwr_running(&p, &r) == NIC_PV_RUN);
    for (int m = 1; m <= 1440; m++) {
        r.second = t + 60u * (uint32_t)m;
        CHECK(nic_pwr_running(&p, &r) == NIC_PV_RUN);
    }
    CHECK(start - p.charge_mas == 2000LL * 86400);
    CHECK(nic_pwr_pct(&p) == 52);

    /* a reboot's gap of three hours is not integrated; float re-anchors at 80 % */
    r.second += 3 * 3600;
    int64_t before = p.charge_mas;
    nic_pwr_running(&p, &r);
    CHECK(p.charge_mas == before);
    r.second += 60; r.current_ma = 5000; r.floating = 1;
    nic_pwr_running(&p, &r);
    CHECK(nic_pwr_pct(&p) == 80);

    /* down to the shed: at −40 A it is to the minute */
    r.floating = 0; r.current_ma = -40000;
    int64_t to_shed = p.charge_mas - (int64_t)628000 * 3600 * 20 / 100;
    int expect = (int)(to_shed / (40000LL * 60)) + 1, m;
    for (m = 1; m < 10000; m++) {
        r.second += 60;
        if (nic_pwr_running(&p, &r) != NIC_PV_RUN) break;
    }
    CHECK(m == expect && nic_pwr_running(&p, &r) == NIC_PV_SHED_DEPLETED);

    nic_pwr_init(&p, &cfg, 70);
    nic_pwr_reading lv = {t, 11900, 0, 0};
    CHECK(nic_pwr_running(&p, &lv) == NIC_PV_SHED_LOW_VOLTAGE);

    /* survival: charged past 50 %, waking only after five unbroken minutes over 13,2 V */
    nic_pwr_init(&p, &cfg, 40);
    nic_pwr_reading s = {t, 13000, 30000, 0};
    nic_pwr_survival(&p, &s);
    int woke = -1;
    for (int k = 1; k < 2000 && woke < 0; k++) {
        s.second = t + 60u * (uint32_t)k;
        s.pack_mv = (k == 124) ? 13150 : 13300;          /* one dip under, at the 124th minute */
        if (nic_pwr_survival(&p, &s) == NIC_PV_WAKE) woke = k;
    }
    /* 30 A gives the 62,8 Ah to 50 % in 126 minutes; the dip at 124 restarts the five minutes */
    CHECK(woke == 130);
    CHECK(nic_pwr_pct(&p) >= 50);

    /* the ordered shutdown */
    nic_shutdown sd;
    uint8_t cards[3] = {0x21, 0x22, 0x23}, units[3] = {2, 3, 1};
    uint32_t loads[3] = {3000, 9000, 5000};
    nic_shut_start(&sd, cards, loads, units, 3);
    nic_shut_step st = nic_shut_next(&sd, 0);
    CHECK(st.act == NIC_SA_MARKER);
    st = nic_shut_next(&sd, 0);
    CHECK(st.act == NIC_SA_END_FLOOR && st.card == 0x22);                      /* 9 W first */
    CHECK(nic_shut_next(&sd, 100).act == NIC_SA_WAIT);
    nic_shut_ack(&sd); nic_shut_ack(&sd); nic_shut_ack(&sd);                   /* its three units */
    st = nic_shut_next(&sd, 200);
    CHECK(st.act == NIC_SA_END_FLOOR && st.card == 0x23);
    nic_shut_ack(&sd);
    st = nic_shut_next(&sd, 300);
    CHECK(st.act == NIC_SA_END_FLOOR && st.card == 0x21);
    nic_shut_ack(&sd);                                                          /* one of two */
    CHECK(nic_shut_next(&sd, 300 + 9999).act == NIC_SA_WAIT);
    st = nic_shut_next(&sd, 300 + 10000);                                       /* 10 s: on */
    CHECK(st.act == NIC_SA_CLK_OFF && st.card == 0x22);
    st = nic_shut_next(&sd, 0); CHECK(st.act == NIC_SA_PWR_OFF && st.card == 0x22);
    st = nic_shut_next(&sd, 0); CHECK(st.act == NIC_SA_CLK_OFF && st.card == 0x23);
    st = nic_shut_next(&sd, 0); CHECK(st.act == NIC_SA_PWR_OFF && st.card == 0x23);
    st = nic_shut_next(&sd, 0); CHECK(st.act == NIC_SA_CLK_OFF && st.card == 0x21);
    st = nic_shut_next(&sd, 0); CHECK(st.act == NIC_SA_PWR_OFF && st.card == 0x21);
    CHECK(nic_shut_next(&sd, 0).act == NIC_SA_FLUSH);
    st = nic_shut_next(&sd, 0); CHECK(st.act == NIC_SA_END_CARD && st.card == 0x22);
    st = nic_shut_next(&sd, 0); CHECK(st.act == NIC_SA_END_CARD && st.card == 0x23);
    st = nic_shut_next(&sd, 0); CHECK(st.act == NIC_SA_END_CARD && st.card == 0x21);
    CHECK(nic_shut_next(&sd, 0).act == NIC_SA_KRONOS_GATE0);
    CHECK(nic_shut_next(&sd, 0).act == NIC_SA_KRONOS_DOWN);
    CHECK(nic_shut_next(&sd, 0).act == NIC_SA_SURVIVAL);
    CHECK(nic_shut_next(&sd, 0).act == NIC_SA_WAIT);

    nic_shut_start(&sd, 0, 0, 0, 0);                                            /* no cards */
    CHECK(nic_shut_next(&sd, 0).act == NIC_SA_MARKER);
    CHECK(nic_shut_next(&sd, 0).act == NIC_SA_FLUSH);
    CHECK(nic_shut_next(&sd, 0).act == NIC_SA_KRONOS_GATE0);
    CHECK(nic_shut_next(&sd, 0).act == NIC_SA_KRONOS_DOWN);
    CHECK(nic_shut_next(&sd, 0).act == NIC_SA_SURVIVAL);

    if (fails) {
        printf("test_power: %d FAILED\n", fails);
        return 1;
    }
    printf("test_power: all passed\n");
    return 0;
}

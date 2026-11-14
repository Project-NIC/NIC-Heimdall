/* The head's power, as ../../mayak/FIRMWARE.md §11 and ../POWER.md §3 fix it: the head's own
 * coulomb count over Hermes's minute readings — the pack's current integrated, re-anchored at the
 * charge stop when the MPPT floats; the shed when the count falls under its threshold or the pack
 * under its low voltage; the ordered shutdown, step by step — the marker, `END` floor by floor,
 * the highest-drawing first, 10 s each for the last frames and the `ACK`s, each card's clock then
 * feed, the buffers and the session, `END` to each card, Kronos gated and down, survival; and the
 * survival loop of the LP core, the same count kept, waking the head when the count is over its
 * threshold and the pack over its recovery voltage for 5 minutes. Nothing is allocated; what is
 * switched is the firmware's, told by the steps. */
#ifndef NIC_POWER_H
#define NIC_POWER_H

#include <stddef.h>
#include <stdint.h>

#define NIC_PWR_FLOORS     16u
#define NIC_PWR_FLOOR_WAIT 10000u       /* ms a floor is given for its last frames and ACKs */
#define NIC_PWR_RECOVER_S  300u

typedef struct {
    uint32_t capacity_mah;            /* the pack's nameplate: 628 000 for the reference 2P4S */
    uint8_t anchor_pct;               /* the charge stop, where float re-anchors the count: 80 */
    uint8_t shed_pct, wake_pct;       /* 20, 50 */
    uint16_t low_mv, recover_mv;      /* the pack's low and recovery voltages */
} nic_pwr_cfg;

typedef struct {
    uint32_t second;
    uint16_t pack_mv;
    int32_t current_ma;               /* into the pack positive */
    uint8_t floating;                 /* Hermes's CHG_STATE 1 */
} nic_pwr_reading;

enum nic_pwr_verdict { NIC_PV_RUN = 0, NIC_PV_SHED_DEPLETED, NIC_PV_SHED_LOW_VOLTAGE, NIC_PV_WAKE };

typedef struct {
    nic_pwr_cfg cfg;
    int64_t charge_mas;               /* the count, mA·s */
    uint32_t last;
    uint8_t have_last;
    uint32_t recovered_since;         /* survival: the pack over its recovery voltage since */
    uint8_t recovering;
} nic_power;

/* At boot: the count from what the BMS said, the only figure there is before the first float. */
void nic_pwr_init(nic_power *p, const nic_pwr_cfg *cfg, uint8_t bms_pct);
uint8_t nic_pwr_pct(const nic_power *p);
/* A minute's reading, running: SHED and why, or RUN. */
enum nic_pwr_verdict nic_pwr_running(nic_power *p, const nic_pwr_reading *r);
/* A minute's reading on the LP core: WAKE, or RUN to wait on. */
enum nic_pwr_verdict nic_pwr_survival(nic_power *p, const nic_pwr_reading *r);

/* ------------------------------------------------------------------ the ordered shutdown */

enum nic_shut_act {
    NIC_SA_MARKER = 1,                /* the reason on the modem, before anything is switched */
    NIC_SA_END_FLOOR,                 /* `END` to the units of floor `card` */
    NIC_SA_CLK_OFF, NIC_SA_PWR_OFF,   /* `PORT_CLK 0`, then `PORT_PWR 0`, on card `card` */
    NIC_SA_FLUSH,                     /* every buffer to both cards, the session closed, Wi-Fi off */
    NIC_SA_END_CARD,                  /* `END` to card `card`: it flushes and sleeps */
    NIC_SA_KRONOS_GATE0, NIC_SA_KRONOS_DOWN,
    NIC_SA_SURVIVAL,                  /* the head into SURVIVAL: the LP core and Hermes alone */
    NIC_SA_WAIT                       /* nothing now */
};

typedef struct {
    enum nic_shut_act act;
    uint8_t card;
} nic_shut_step;

typedef struct {
    uint8_t card[NIC_PWR_FLOORS];     /* the floors, highest-drawing first */
    uint8_t units[NIC_PWR_FLOORS];    /* the ACKs a floor's END waits for */
    uint8_t n, phase, i, acks;
    uint32_t since;
} nic_shutdown;

/* The floors by the cards' addresses and their measured loads (mW), and their units' counts. */
void nic_shut_start(nic_shutdown *s, const uint8_t *cards, const uint32_t *load_mw, const uint8_t *units,
                    uint8_t n);
/* A unit's `ACK` to the floor's END. */
void nic_shut_ack(nic_shutdown *s);
/* The next step at `now` (ms). */
nic_shut_step nic_shut_next(nic_shutdown *s, uint32_t now);

#endif

/* The head's values frame on the modem, as ../../mayak/FIRMWARE.md §9 and ../PROTOCOL.md §6 fix it:
 * 32 B, little-endian — station 3 B · second 4 B · pack 2 B (mV) · temperature 1 B (°C, signed) ·
 * events 2 B (weak events since the last frame, held at 65 535) · alive 16 B (a bit a unit,
 * card × 8 + slot) · flags 1 B · peak 1 B (0,5 dB) · CRC-16 2 B, the station's.
 *
 * Sent once an hour as the heartbeat and at once on a marker, every frame restarting the hour; a
 * marker within a minute of the last is a weak event instead, so aftershocks do not flood; the
 * airtime of the last hour held under the modem's cap, a frame that would pass it waiting — the
 * cadence stretches; the last 16 frames kept for a resend request. Nothing is allocated. */
#ifndef NIC_VALUES_H
#define NIC_VALUES_H

#include <stddef.h>
#include <stdint.h>

#define NIC_VAL_LEN     32u
#define NIC_VAL_HOUR    3600u
#define NIC_VAL_MINUTE  60u
#define NIC_VAL_KEPT    16u
#define NIC_VAL_SENDS   128u         /* sends remembered for the cap */

enum { NIC_CLK_UNSYNCED = 0, NIC_CLK_HOLDOVER, NIC_CLK_LOCKED, NIC_CLK_SEEDED };
enum { NIC_WHY_HEARTBEAT = 0, NIC_WHY_EVENT, NIC_WHY_FAULT, NIC_WHY_SUPPLY, NIC_WHY_NO_CLOCK,
       NIC_WHY_NO_STORE, NIC_WHY_NO_BMS, NIC_WHY_SHED };
enum { NIC_PWR_NORMAL = 0, NIC_PWR_SHED, NIC_PWR_SURVIVAL, NIC_PWR_CELL };

/* What the head knows at the moment a frame goes. */
typedef struct {
    uint16_t pack_mv;
    int8_t temperature;
    uint8_t alive[16];
    uint8_t clock, power;            /* NIC_CLK_*, NIC_PWR_* */
} nic_val_state;

typedef struct {
    uint32_t station, second;
    uint16_t pack_mv, events;
    int8_t temperature;
    uint8_t alive[16];
    uint8_t clock, marker, why, power, peak;
} nic_val_frame;

void nic_val_build(const nic_val_frame *f, uint8_t out[NIC_VAL_LEN]);
int nic_val_parse(const uint8_t in[NIC_VAL_LEN], nic_val_frame *f);   /* -1 its CRC */

typedef struct {
    uint32_t station;
    uint32_t airtime_ms, cap_ms;     /* a frame's airtime on this modem; the hour's cap, 0 none */
    uint32_t last_sent, last_marker;
    uint8_t marked;                  /* a marker has gone: the minute counts from it */
    uint8_t due, due_why, due_peak;  /* a marker waiting for the cap */
    uint32_t events;
    uint8_t kept[NIC_VAL_KEPT][NIC_VAL_LEN];
    uint8_t nkept, kept_at;
    uint32_t sends[NIC_VAL_SENDS];   /* the seconds of the hour's sends, a ring */
    uint8_t nsends, sends_at;
    uint32_t held;                   /* frames that waited on the cap */
} nic_values;

/* At boot: the hour counts from `now`. */
void nic_val_init(nic_values *v, uint32_t station, uint32_t now, uint32_t airtime_ms, uint32_t cap_ms);
void nic_val_weak(nic_values *v);
/* A line of the marker set: due at once, or within the minute a weak event. 1 when it is due. */
int nic_val_marker(nic_values *v, uint32_t now, uint8_t why, uint8_t peak);
/* Every second: 1 and the frame in out when one goes now. */
int nic_val_tick(nic_values *v, uint32_t now, const nic_val_state *st, uint8_t out[NIC_VAL_LEN]);
/* The gateway asks for the frame of `second` again: 1 and the frame, 0 when it is not kept or
 * the cap is spent. */
int nic_val_resend(nic_values *v, uint32_t now, uint32_t second, uint8_t out[NIC_VAL_LEN]);

#endif

/* Kronos's discipline loop, as ../../kronos/FIRMWARE.md §5 and §6 fix it, above its pins: once a
 * second, on a usable source's paired capture of TIM5 (2²⁷ ticks a second), the interval, the
 * gate, the phase against PPS-K's place K, a PI — the free-running frequency averaged over 64 s,
 * the phase error over 256 s, clipped to ±4 FRACN codes about the estimate — the one step on the
 * 8th usable second of a board never locked, the lock, the holdover on the temperature table,
 * and the code split for the 2 ms dither. All in fixed point: a frequency in 2⁻¹⁶ tick a second
 * (q); one FRACN code, 3,81 ppm, is 512 ticks a second, 2²⁵ q. The firmware gives it the
 * captures, the source's checks and the temperature, and writes the code and ARR. */
#ifndef NIC_KRONOS_H
#define NIC_KRONOS_H

#include <stdint.h>

#define NIC_K_SECOND   134217728        /* 2²⁷ */
#define NIC_K_CODE_Q   ((int64_t)1 << 25)
#define NIC_K_WINDOW   64u
#define NIC_K_TAU      256              /* the phase term's time constant, s */
#define NIC_K_BINS     250u             /* 0,5 °C from −40 to +85 */

enum nic_k_state { NIC_K_CLOCKED = 0, NIC_K_LOCKED, NIC_K_HOLDOVER, NIC_K_SEEDED };

typedef struct {
    uint8_t state, been_locked, have_prev, usable, stepped;
    uint32_t prev, usable_seconds;
    int32_t K;                           /* PPS-K's place on TIM5's count, 0 … 2²⁷ − 1 */
    int32_t e;                           /* the last phase error, ticks */
    int64_t win[NIC_K_WINDOW];           /* the free-running frequency a second, q */
    uint8_t wn, wi;
    int64_t est;                         /* its average, the estimate, q */
    int64_t last_good;                   /* the estimate when the source was last usable */
    int64_t hist[8];                     /* the estimate over the last 8 s, for the lock */
    uint8_t hn;
    int64_t applied;                     /* what the code put on the second now running, q */
    int64_t carry;                       /* the split's remainder, into the next second */
    int32_t code;                        /* this second's code and its 2 ms slots at code + 1 */
    uint16_t hi;
    /* the temperature table */
    int16_t table[NIC_K_BINS];           /* 2⁻¹⁰ ppm */
    uint8_t filled[NIC_K_BINS];
    int16_t temp;                        /* 0,01 °C */
    uint8_t have_temp, tmin, tmax;       /* the bins seen since the last filing */
    uint32_t since_file, since_persist;
    uint32_t glitches, steps, filings;
} nic_kronos;

/* At boot: the persisted code (`c`, `d` as its slots of 500), the state CLOCKED. */
void nic_kronos_init(nic_kronos *k, int32_t code, uint16_t hi);
/* The head's seed: SEEDED, the persisted code, no steering. */
void nic_kronos_seed(nic_kronos *k);

enum nic_k_action { NIC_K_NONE = 0, NIC_K_STEP = 1, NIC_K_PERSIST = 2 };

/* A usable source's edge, paired: its capture and the delay table's sum `corr`, ticks. The
 * action's bits: STEP — the running period stretched by *step ticks (ARR = 2²⁷ − 1 + step),
 * K advanced by it; PERSIST — the code written to its cell, the hour's. */
int nic_kronos_edge(nic_kronos *k, uint32_t capture, int32_t corr, int32_t *step);
/* The source failed a check, or no edge came: a locked board goes to HOLDOVER. */
void nic_kronos_lost(nic_kronos *k);
/* The TMP117's reading, every 10 s. */
void nic_kronos_temp(nic_kronos *k, int16_t centi);
/* The code for the second now starting, from the state: the loop's, the table's or the
 * persisted one. Called once a second, after the edge or in its place. */
void nic_kronos_second(nic_kronos *k, int32_t *code, uint16_t *hi);
/* The table's frequency for a temperature, q; 0 and the last good estimate's when it has none. */
int64_t nic_kronos_table_q(const nic_kronos *k, int16_t centi, int *found);

#endif

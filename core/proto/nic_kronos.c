/* Kronos's discipline loop; see nic_kronos.h. */
#include "nic_kronos.h"

#include <math.h>
#include <string.h>

#define PPM_Q(x)   ((int64_t)((x) * 134.217728 * 65536.0))   /* ppm → q, for the constants */
#define GATE_FLOOR ((int64_t)8 << 16)   /* the gate never tighter than ±32 ticks */
#define LOCK_E     134                  /* ±1 µs */

static int64_t q_of_entry(int16_t v) { return (int64_t)v * 8589934592LL / 1000000; }
static int16_t entry_of_q(int64_t q) { return (int16_t)(q * 1000000 / 8589934592LL); }

static int bin_of(int16_t centi)
{
    int b = ((int)centi + 4000) / 50;
    return b < 0 ? 0 : b >= (int)NIC_K_BINS ? (int)NIC_K_BINS - 1 : b;
}

static void split(int64_t w, int32_t *code, uint16_t *hi)
{
    int64_t c = w / NIC_K_CODE_Q;                            /* floor, without a shift */
    if (c * NIC_K_CODE_Q > w)
        c--;
    int64_t slots = ((w - c * NIC_K_CODE_Q) * 500 + NIC_K_CODE_Q / 2) / NIC_K_CODE_Q;
    if (slots >= 500) {
        c++;
        slots = 0;
    }
    *code = (int32_t)c;
    *hi = (uint16_t)slots;
}

static int64_t applied_of(int32_t code, uint16_t hi)
{
    return (int64_t)code * NIC_K_CODE_Q + ((int64_t)hi * NIC_K_CODE_Q + 250) / 500;
}

void nic_kronos_init(nic_kronos *k, int32_t code, uint16_t hi)
{
    memset(k, 0, sizeof *k);
    k->state = NIC_K_CLOCKED;
    k->code = code;
    k->hi = hi;
    k->applied = applied_of(code, hi);
    k->est = k->last_good = -k->applied;
}

void nic_kronos_seed(nic_kronos *k)
{
    if (k->state == NIC_K_CLOCKED)
        k->state = NIC_K_SEEDED;
}

static void restart(nic_kronos *k)
{
    k->have_prev = 0;
    k->wn = k->wi = 0;
    k->hn = 0;
    k->usable_seconds = 0;
    k->stepped = 0;
}

void nic_kronos_lost(nic_kronos *k)
{
    if (k->usable)
        k->last_good = k->est;
    k->usable = 0;
    restart(k);
    if (k->state == NIC_K_LOCKED)
        k->state = NIC_K_HOLDOVER;
}

static int64_t mean(const nic_kronos *k)
{
    int64_t s = 0;
    for (uint8_t i = 0; i < k->wn; i++)
        s += k->win[i];
    return k->wn ? s / k->wn : k->est;
}

static int64_t rms(const nic_kronos *k, int64_t m)
{
    double s = 0;
    for (uint8_t i = 0; i < k->wn; i++) {
        double d = (double)(k->win[i] - m);
        s += d * d;
    }
    return k->wn ? (int64_t)sqrt(s / k->wn) : 0;
}

static void file_temperature(nic_kronos *k)
{
    if (++k->since_file < NIC_K_WINDOW)
        return;
    if (k->have_temp && k->tmin == k->tmax && k->wn == NIC_K_WINDOW) {  /* one bin across 64 s */
        int16_t v = entry_of_q(k->est);
        int16_t *t = &k->table[k->tmin];
        *t = k->filled[k->tmin] ? (int16_t)(*t + (v - *t) / 8) : v;  /* a slow average */
        k->filled[k->tmin] = 1;
        k->filings++;
    }
    k->since_file = 0;
    k->tmin = k->tmax = (uint8_t)bin_of(k->temp);
}

int nic_kronos_edge(nic_kronos *k, uint32_t capture, int32_t corr, int32_t *step)
{
    int action = NIC_K_NONE;
    if (!k->usable) {
        k->usable = 1;
        restart(k);
    }
    if (!k->have_prev) {
        k->prev = capture;
        k->have_prev = 1;
        k->usable_seconds = 1;
        return action;
    }
    /* 1. the raw interval, over however many seconds since the last good edge */
    uint32_t delta = capture - k->prev;
    uint32_t secs = (delta + (NIC_K_SECOND >> 1)) >> 27;
    if (secs == 0)
        secs = 1;
    int64_t err = (int64_t)delta - (int64_t)secs * NIC_K_SECOND;
    int64_t s = err * 65536 / (int64_t)secs - k->applied;   /* the free-running frequency */
    /* 2. the gate */
    int64_t m = mean(k), r = rms(k, m);
    if (r < GATE_FLOOR)
        r = GATE_FLOOR;
    if (k->wn >= 2 && (s - m > 4 * r || m - s > 4 * r)) {
        k->glitches++;
        return action;                    /* dropped: the last good edge kept */
    }
    k->prev = capture;
    k->usable_seconds++;
    k->win[k->wi] = s;
    k->wi = (uint8_t)((k->wi + 1) % NIC_K_WINDOW);
    if (k->wn < NIC_K_WINDOW)
        k->wn++;
    k->est = mean(k);
    k->hist[k->hn % 8] = k->est;
    k->hn++;
    /* 3. the phase */
    int32_t phi = (int32_t)((capture - (uint32_t)corr) & (NIC_K_SECOND - 1));
    int32_t e = (phi - k->K) & (NIC_K_SECOND - 1);
    if (e >= NIC_K_SECOND / 2)
        e -= NIC_K_SECOND;
    k->e = e;
    /* 7. the step, once, on the 8th usable second of a board never locked */
    if ((k->state == NIC_K_CLOCKED || k->state == NIC_K_SEEDED) && k->usable_seconds >= 8) {
        if (!k->been_locked && !k->stepped) {
            k->stepped = 1;
            *step = (int32_t)((uint32_t)e & (NIC_K_SECOND - 1));
            k->K = (k->K + *step) & (NIC_K_SECOND - 1);
            k->e = 0;
            k->steps++;
            action |= NIC_K_STEP;
        }
        int64_t lo = k->hist[0], hi = k->hist[0];
        for (int i = 1; i < 8 && i < k->hn; i++) {
            lo = k->hist[i] < lo ? k->hist[i] : lo;
            hi = k->hist[i] > hi ? k->hist[i] : hi;
        }
        if (k->hn >= 8 && (k->e < LOCK_E && k->e > -LOCK_E) && hi - lo < PPM_Q(0.05))
            k->state = NIC_K_LOCKED, k->been_locked = 1;
    } else if (k->state == NIC_K_HOLDOVER && k->e < LOCK_E && k->e > -LOCK_E) {
        k->state = NIC_K_LOCKED;          /* slewed in */
    }
    if (k->state == NIC_K_LOCKED) {
        k->last_good = k->est;
        file_temperature(k);
        if (++k->since_persist >= 3600) {
            k->since_persist = 0;
            action |= NIC_K_PERSIST;
        }
    }
    return action;
}

void nic_kronos_temp(nic_kronos *k, int16_t centi)
{
    uint8_t b = (uint8_t)bin_of(centi);
    k->temp = centi;
    if (!k->have_temp) {
        k->have_temp = 1;
        k->tmin = k->tmax = b;
    }
    k->tmin = b < k->tmin ? b : k->tmin;
    k->tmax = b > k->tmax ? b : k->tmax;
}

int64_t nic_kronos_table_q(const nic_kronos *k, int16_t centi, int *found)
{
    double x = ((double)centi + 4000.0) / 50.0 - 0.5;     /* in bin centres */
    int lo = -1, hi = -1;
    for (int b = (int)floor(x); b >= 0; b--)
        if (b < (int)NIC_K_BINS && k->filled[b]) { lo = b; break; }
    for (int b = (int)ceil(x); b < (int)NIC_K_BINS; b++)
        if (b >= 0 && k->filled[b]) { hi = b; break; }
    *found = lo >= 0 && hi >= 0;
    if (!*found)
        return k->last_good;
    if (lo == hi)
        return q_of_entry(k->table[lo]);
    double f = (x - lo) / (hi - lo), v = k->table[lo] + f * (k->table[hi] - k->table[lo]);
    return (int64_t)(v * 8589934592.0 / 1000000.0);
}

void nic_kronos_second(nic_kronos *k, int32_t *code, uint16_t *hi)
{
    int64_t w;
    if (k->state == NIC_K_SEEDED && !k->usable) {
        *code = k->code;                  /* the persisted code, no steering */
        *hi = k->hi;
        return;
    }
    if (!k->usable) {
        int found;                        /* holdover: the table, or the last good estimate */
        w = k->state == NIC_K_HOLDOVER && k->have_temp ? -nic_kronos_table_q(k, k->temp, &found)
                                                       : -k->last_good;
    } else if ((k->state == NIC_K_CLOCKED || k->state == NIC_K_SEEDED) && !k->stepped
               && !k->been_locked) {
        w = -k->est;                      /* before the step: the frequency alone */
    } else {
        int64_t p = -(int64_t)k->e * 65536 / NIC_K_TAU, cap = 4 * NIC_K_CODE_Q;
        p = p > cap ? cap : p < -cap ? -cap : p;   /* 5. the slew ceiling */
        w = -k->est + p;
    }
    w += k->carry;                        /* 6. what the last split could not give, given now */
    split(w, &k->code, &k->hi);
    k->applied = applied_of(k->code, k->hi);
    k->carry = w - k->applied;
    *code = k->code;
    *hi = k->hi;
}

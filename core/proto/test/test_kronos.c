/* Kronos's loop against a simulated TCXO: ±1 ppm with the temperature, 20 ns of pulse jitter.
 * The lock within 20 s of the first usable second and one step, |e| under 134 ticks for 24 h, a
 * 3 µs glitch dropped by the gate, the table filed; a day of holdover on it, and a day on a TCXO
 * that jumped, slewed back at the ceiling with no step. */
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#include "../nic_kronos.h"

static int fails;
#define CHECK(c) do { if (!(c)) { printf("FAIL %s:%d %s\n", __FILE__, __LINE__, #c); fails++; } } while (0)

/* the world: true seconds n; the counter runs at 2²⁷ (1 + y + u) */
static double count, jump;
static uint64_t rng = 88172645463325252ULL;

static double gauss(void)
{
    double a = 0, b;
    for (int i = 0; i < 12; i++) {
        rng ^= rng << 13; rng ^= rng >> 7; rng ^= rng << 17;
        b = (double)(rng >> 11) / 9007199254740992.0;
        a += b;
    }
    return a - 6;
}

static double temp_at(double t) { return 25 + 5 * sin(2 * M_PI * t / 86400); }
static double y_at(double t) { return 0.6e-6 + 0.06e-6 * (temp_at(t) - 25) + jump; }

static nic_kronos k;
static long t;                        /* true seconds */
static int32_t code;
static uint16_t hi;

/* one second: the counter runs under the code, the edge captured with jitter, the loop fed */
static int second(int source, double glitch, int32_t *step, int *action)
{
    double u = ((double)code + hi / 500.0) * 512.0 / 134217728.0;
    count += 134217728.0 * (1 + y_at((double)t) + u);
    t++;
    if (t % 10 == 0)
        nic_kronos_temp(&k, (int16_t)lround(temp_at((double)t) * 100));
    *action = 0;
    if (source) {
        double j = gauss() * 20e-9 + glitch;
        uint32_t cap = (uint32_t)(uint64_t)llround(count + j * 134217728.0);
        *action = nic_kronos_edge(&k, cap, 0, step);
    } else {
        nic_kronos_lost(&k);
    }
    nic_kronos_second(&k, &code, &hi);
    return k.state;
}

/* the true phase of PPS-K against the source, ticks: K against the count's place in the second */
static double true_e(void)
{
    double phi = fmod(count, 134217728.0), e = phi - k.K;
    if (e >= 67108864.0) e -= 134217728.0;
    if (e < -67108864.0) e += 134217728.0;
    return e;
}

int main(void)
{
    int32_t step = 0;
    int a;
    count = 12345.0;
    nic_kronos_init(&k, 0, 0);
    nic_kronos_second(&k, &code, &hi);
    for (int i = 0; i < 30; i++)                           /* no source yet: the persisted code */
        second(0, 0, &step, &a);
    CHECK(k.state == NIC_K_CLOCKED && code == 0 && hi == 0);
    long first = t + 1, locked_at = 0;
    for (int i = 0; i < 120 && !locked_at; i++) {
        second(1, 0, &step, &a);
        if (k.state == NIC_K_LOCKED)
            locked_at = t;
    }
    CHECK(locked_at && locked_at - first < 20 && k.steps == 1);
    printf("locked %ld s after the first usable second, one step of %d ticks\n", locked_at - first + 1,
           (int)step);
    /* a day locked: |e| under 134 ticks, a 3 µs glitch at noon dropped */
    double worst = 0, worst_true = 0;
    uint32_t persists = 0;
    for (long i = 0; i < 86400; i++) {
        second(1, i == 43200 ? 3e-6 : 0, &step, &a);
        persists += (a & NIC_K_PERSIST) != 0;
        if (fabs((double)k.e) > worst) worst = fabs((double)k.e);
        if (i > 600 && fabs(true_e()) > worst_true) worst_true = fabs(true_e());
        if (k.state != NIC_K_LOCKED || (a & NIC_K_STEP)) {
            CHECK(0);
            break;
        }
    }
    CHECK(worst < 40 && k.glitches == 1 && persists == 24 && k.steps == 1);
    printf("a day locked: |e| at most %.0f ticks, %.0f true; %u glitch, %u filings\n", worst,
           worst_true, (unsigned)k.glitches, (unsigned)k.filings);
    int filled = 0;
    for (unsigned b = 0; b < NIC_K_BINS; b++)
        filled += k.filled[b];
    CHECK(filled >= 18);                                   /* 20–30 °C in 0,5 °C bins */
    /* a day of holdover on the table: the phase walks by much less than 86 ms */
    long K0 = k.K;
    double e0 = true_e();
    for (long i = 0; i < 86400; i++)
        second(0, 0, &step, &a);
    double walked = fabs(true_e() - e0) / 134217728.0;
    printf("a day of holdover on the table: %.2f ms\n", walked * 1e3);
    CHECK(k.state == NIC_K_HOLDOVER && walked < 5e-3);
    /* back: slewed in, no step */
    long back = 0;
    for (long i = 0; i < 4 * 3600 && !back; i++) {
        second(1, 0, &step, &a);
        CHECK(!(a & NIC_K_STEP));
        if (k.state == NIC_K_LOCKED)
            back = i + 1;
    }
    CHECK(back && k.K == K0);
    /* a TCXO jumped by 1 ppm at the start of a day's holdover: ~86 ms, pulled in at 15 ppm */
    jump = 1e-6;
    for (long i = 0; i < 86400; i++)
        second(0, 0, &step, &a);
    double off = fabs(true_e()) / 134217728.0;
    long pulled = 0;
    double fastest = 0;
    for (long i = 0; i < 4 * 3600 && !pulled; i++) {
        double before = true_e();
        second(1, 0, &step, &a);
        double rate = fabs(true_e() - before) / 134217728.0;
        if (i > 70 && rate > fastest) fastest = rate;
        CHECK(!(a & NIC_K_STEP));
        if (k.state == NIC_K_LOCKED)
            pulled = i + 1;
    }
    printf("a jumped TCXO: %.1f ms off after a day, slewed in %.2f h at %.1f ppm at most\n",
           off * 1e3, pulled / 3600.0, fastest * 1e6);
    CHECK(off > 0.08 && pulled > 1.9 * 3600 && pulled < 2.3 * 3600 && fastest < 15.4e-6);
    CHECK(k.K == K0 && k.steps == 1 && fabs((double)k.e) < 134);
    if (fails) {
        printf("%d checks failed\n", fails);
        return 1;
    }
    printf("Kronos's loop: every check passed\n");
    return 0;
}

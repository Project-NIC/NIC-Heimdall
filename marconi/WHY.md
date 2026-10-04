★ N.I.C. ★

# Marconi — the graveyard

Rejected alternatives and superseded states, with the reason. Nothing returns from here and
nothing is deleted from it.

## A faster MCU — the H725, the F7 — not taken

The H725: 1177 DMIPS against 599, 564 KB against 1380. **A burst is written once by DMA and read
once by the FFT** — nothing for a cache to hold; the core sits under 5 % either way. **Buffer
decides.** The STM32F7 was never in it: 532 KB and no PSSI or equivalent.

## Antenna — the ones that stay held, and what would bring each one back

The antenna is one vertical loop; its two azimuth nulls are a siting condition (`CONSTRUCTION.md`,
*One loop, standing, and its two nulls are a siting condition*). Nothing below is wrong — each is out on one named condition, so a build without it can
pick the antenna up again. **This section is not a graveyard and is not to be trimmed to one
line when the graveyard is optimised.**

### The condition that rules out every electric antenna: the station does not know where it will stand

A short vertical — whip, cross, ring, top hat, any height — has its null at the zenith, where a
near transmitter arrives. One hop off F2 at a 300 km virtual height:

| distance | elevation | short vertical |
|---|---|---|
| 8 000 km | 4,3° | −0,0 dB |
| 2 000 km | 16,7° | −0,4 dB |
| 1 000 km | 31,0° | −1,3 dB |
| 600 km | 45,0° | −3,0 dB |
| 200 km | 71,6° | −10,0 dB |
| overhead | 90° | **−∞** |

A vertical loop measures the horizontal `B`, independent of elevation, so it is flat in elevation
and sees the zenith; its nulls are two fixed azimuths, occupied once from known coordinates. A
vertical's zenith null moves only by standing further from the transmitter — backwards, since a
near-vertical path measures `foF2` directly, not through the secant law (`FIRMWARE.md` §6).

### Held, with the condition

| antenna | out because | comes back when |
|---|---|---|
| **vertical whip** | the zenith null; an E-field antenna — local electric noise a loop rejects, a counterpoise on a site deliberately not earthed | a fixed position known far from every transmitter used, **and** a site electrically quiet against a loop |
| **horizontal cross of half-dipoles, or a horizontal ring, as a capacitive top hat** | the same zenith null — the downlead is the antenna, not the ring; at 3–5 m it resonates **inside** 0,5–16 MHz, and `S1` is compensated for an inductive source | the same, **and** its resonance above 16 MHz, **and** a front end compensated for a capacitive source |
| **horizontal loop** | `B_z` = 0 for a vertically polarised wave — blind at every elevation and to NAVTEX, NDB, DGPS and MW | **never** — geometry, not a condition |
| **Adcock array** | a measured bearing, now computed from two coordinate pairs; four rods on a roof are four lightning finials | a build needing a measured bearing |
| **two crossed loops, one per converter channel** | a second converter channel; two channels on the PSSI cost half rate, which loses the band (*The port*) | a second converter, or a part carrying two channels at full rate on one port |
| **two crossed loops into one channel, switched between bursts** | the switch alone carries the isolation — an FDA's `PD` does not (the resistor network keeps the path, TI, `THS4551` §9.4; the input diodes conduct on a large signal) — and no analogue switch publishes distortion at 5–16 MHz, `TMUX121` included | that distortion is measured, **and** a site's bearings spread enough that the worst target's −6 dB costs something |
| **two crossed loops into one channel, quadrature hybrid** | two front ends on the power budget; a 90° network holding phase over five octaves | the budget carries it **and** the network holds phase over temperature and part spread |
| **a ferrite rod** | ~14 turns usable to 16 MHz against Tesla's ~150: 15,7 dB down where the loop has 11,2 dB over its own amplifier | never at this band top; Tesla's rod wins at 512 kHz for the same reason |
| **splitting the band across two channels** | day/night marker pairs are ~2:1 in frequency — the ratio **is** the measurement — so any boundary cuts all of them | never |

The crossed pair bought no nulls, a bearing (now computed) and the polarisation ellipse — O/X
separation, which needs two orthogonal components sampled at once — gone, not deferred.

### Rejected on the loop itself

**Series R or series C on the loop.** A series element puts a corner that follows the coil — the
antenna-dependent response the virtual short exists to avoid: 36,5 Ω costs 16 dB at 300 kHz; the C
is second-order and inverts phase across its resonance. The unpopulated series-C footprint stays
for a site under a live MW transmitter.

**More turns, spread turns, a shunt resistor across the loop.** `L ∝ N²` tightens
`i_n < e_n/ωL` faster than it buys signal; a shunt across a virtual short carries almost no current
and steals frequency-dependently. The amplifier upgrade bought 17,5 dB instead.

## Front end

**`THS4541` on S1 — the first choice there.** At 4,7 kΩ on 2,9 µH, a loop gain of 3,3: 12,7 dB of
distortion correction and a transimpedance set 23 % by the ±30 % GBW, no longer following the
resistor. The `LMH5401` took S1; the `THS4541` stays on S2.

**`LTC6409`.** 1,1 nV/√Hz, but `i_n·Rf` = 41 nV swamps the band top — 1,4 dB behind the
`THS4541`.

**`LMH3401`.** Fixed internal feedback — a few hundred ohms against the 4,7 kΩ needed.

**A Tayloe QSD front end — the road not taken**, the analysis finished:

| | **A — Tayloe QSD** | **B — direct sampling** *(the pick)* |
|---|---|---|
| front | FST3253 dual 4:1 switch + 74AC74 ÷4, LO from the MCU's PLL/MCO | pipeline ADC on a parallel bus |
| converter | **ADS127L14, the Tesla part** — I/Q on 2 channels at 512 kSPS | a 16-bit pipeline at 2²⁶ — the **AD9265-80** (`LTC2143` at the time) |
| seen at once | one **512 kHz** window, hopped | **the whole band, continuously** |
| dynamic range | `IIP3` ≈ +30 dBm; the sampling caps are a **filter that tunes with the LO**, rejecting outside the window *before* the converter | the whole band hits the converter; S2 rejects outside it |
| chirpsounder | a ~100 kHz/s chirp crosses a window in ~5 s — a slice, not a sweep | **the entire ionogram** |
| new to the project | **nothing** | a pipeline-ADC family and a parallel interface |

A wins everywhere but the deciding point — no new part family, better instantaneous dynamic range
amid the station's own switching noise, the per-carrier log-level record directly. **B was taken
because the passive ionogram is the goal, not a bonus**, and chirps are where A is weaker. Left
open for A: its LO wants 4× the tuned frequency, no whole division of 2²³, so a fractional PLL —
whose phase noise reciprocally mixes a strong neighbour into the wanted bin, past any filter. If
the ionogram is ever a bonus again, A is the better board.

**Splitting the band "for dynamic range" (an early revision).** It forgot processing gain: a
carrier is read from one 256 Hz bin, 51 dB under the broadband floor, so one 80 dB below the
loudest still sits 34 dB above the bin floor.

**"How far down `Rf1` goes on a given site is one measurement."** `Rf1` is fixed at 2,35 kΩ for
every site; the clamp pair across `Rf2` is the one field provision.

## Filters

**LC ladder (6-pole Chebyshev) between the stages — superseded by the all-RC chain.** It held
−73 dB at the first fold and −128 dB at 144 MHz, and left with all board inductors: no wound part
beside a magnetic antenna, no SMD self-resonance in band, one filter principle on all three units.
The RC chain's shortfall (80 dB at 2 m, 40 dB at the first fold) is carried by the sparse HF
spectrum and digital notches. If a site drowns in 2 m/FM intermodulation, the ladder waits here.

**The switched scale divider — removed with Tesla's.** Fixed gain sized to the realistic worst
case; the plug-in attenuation position across `Rf2` replaces every switched mechanism.

## The port — one channel, because half rate loses the band

The FMC reads in bursts, and a burst boundary loses samples from a converter that does not wait;
the 16-bit PSSI carries one channel at this rate. Two would need the interleaved output at 2²⁵
each, and **half rate is what Marconi cannot pay**:

| | |
|---|---|
| Nyquist at 2²⁵ | **16,777 MHz**, the 16 MHz band top at 95,4 % of it |
| the first fold onto the band top | **17,55 MHz** — 0,13 octaves above the band, against 1,68 today |
| what S2 gives there | a few dB; no RC chain or LC ladder stops 1,55 MHz above a 16 MHz passband |
| what folds in | CB 26,965–27,405 → **6,15–6,59 MHz**, the marker region; ISM 27,12 → 6,43; 10 m 28,0–29,7 → **3,85–5,55**, onto RWM 4996 and WWV 5000; 11 m 25,67–26,10 → **7,45–7,88**, onto CHU 7850 |

Cutting the band to keep the skirt is worse: 1,68 octaves at 2²⁵ puts the top at 8,0 MHz, losing
WWV/WWVH/BPM 15000 and 10000, CHU 14670 and RWM 9996 — the 5000/10000/15000 triplet becomes one
rung and daytime `foF2` at 8–13 MHz leaves the range. One channel at 2²⁶; the cost is the second
antenna input.

## Converter

**`LTC2143-14` — was the pick, now the recorded alternate.** The `AD9251`'s class (73,1 against
74,3 dBFS SNR), twice the price, zero stock at LCSC, where the boards are assembled. Swapping back
redraws the converter corner; it is not a part swap.

**`AD9268` and `AD9269`** (16-bit, dual): on a two-channel board, +5,1 and +4,5 dB of converter
for 750 and 200 mW against the `AD9251`'s 113 mW bought 0,27 dB of system — rejected. One channel
inverts it: the dropped `LMH5401` and `THS4541` free ~214 mW, and the 16-bit single `AD9265-80`
costs ~254 mW against the 14-bit `AD9649-80`'s 87 mW. **The channel pays for the bits:
`AD9265-80`.** The system gains 0,26 dB (the sky is 24,9 dB above the converter); the 4,7 dB goes
to headroom, where a hot site spends it.

**`AD9649-80`** (14-bit, single): 87 mW, 74.x dBFS; the fallback if the budget tightens.

**`AD6655`**: the DDC window is 22 % of the sample rate against a band 15,5 MHz wide.

**`AD9866`**: 12 bits, one channel, and half the die a transmitter.

**A $2000 converter.** Chain floor: sky 468 → electronics 86 → converter 27 nV/√Hz. A perfect
converter saves 0,40 dB, a perfect everything 0,55 dB; below 30 MHz the receiver fights the
planet, not its silicon.

## Converter, the sample rate — 3 × 2²⁴ weighed, and the package

**3 × 2²⁴ = 50,3 MSPS** against 2²⁶: an integer PLL2 (2²² × 96 ÷ 8), inside the converter's
20–80 MSPS, a quarter fewer samples. Not taken: ×6 to the 2²³ timebase instead of a shift; FFT bins
and the carrier record's step no longer powers of two; kT/C density per bin 1,25 dB worse; the
scintillation boards on the same converter lose a quarter of their samples per pulse. A short core
is relieved by decimation after the first half-band.

**A faster core to raise the rate — closed, not a compute question.** `PSSI_PDCK` is 100 MHz
absolute, the lower of its two limits (the 0,4 ratio allows 107 at a 2²⁸ kernel), so
2²⁷ = 134,2 MHz is out at the pin, as octal DDR to the PSRAM is at 120 MHz. **The pins hold the
rate**; no processor is weighed for it.

**The package.** LQFP176 was taken for a 32-bit parallel bus now gone; the PSSI's 17 pins fit
LQFP144. The package stays on the grounds in `../core/HARDWARE.md`.

## MCU — the substitutions weighed

**STM32N657 — deferred, not rejected.** Its M55 at 800 MHz with Helium, 4,2 MB of SRAM and int8
NPU sit behind the converter; what binds is in front: `fHSE_ext` 16–48 MHz against the 2²² wire
(an input multiplier); the same PSSI 100 MHz and `PDCK`/`f_HCLK` ≤ 0,4, so no rate gain, and data
setup worse, 2 ns to 3,5; timers capped at 240 MHz, halving the ranging capture's resolution; no
internal flash; BGA only. The NPU has no job: the transform is no convolution of its shape, and
int8 cannot carry a chain holding 101 dB.

**STM32V863, LQFP176 with 2 MB of eNVM — a footnote until its datasheet exists.** The tier's
package, its own non-volatile memory, `fHSE_ext` 4–50 MHz (no multiplier), Helium, up to 512 kB of
TCM with ECC, `T_J` 140 °C, latch-up-immune 18 nm FD-SOI. Two unpublished numbers decide it: the
16-bit parallel interface's clock ceiling (it must take 2²⁶) and the timers' against the 2²⁸
kernels; then the order code, a GPDMA hardware ring for the one gapless 1 MB burst, the ePCM's
1000-write endurance, programming after reflow.

## The `TMP117`/`STS35` on `I2C1` — superseded by an NTC on `ADC1`

The board's temperature is a covariate on `GET` and corrects nothing, so 0,1 °C bought nothing.
Replaced by the station's one method — an `NCP18XH103F03RB` on a ratiometric divider, powered only
to convert (`../tesla/HARDWARE.md` §7); `I2C1` carries the `INA238` alone.

## The station interface as a translator class and an I²C shifter choice — superseded

Once "fixed-direction translators of the `SN74AXC4T245`/`8T245` class", the I²C on "`TXS0102` or
`PCA9306`", `LINE_EN` "tied on", the bodies untabled, and the rails row placing the 4,0 V where
"`ENABLE` does not reach the 12 V". The clock line had already refused the `TXS0102`'s
auto-direction one-shots, so the I²C is a `PCA9306`; two `SN74AXC8T245`, `DIR` strapped per
package, the bodies tabled (`HARDWARE.md`, *The station interface*). `ENABLE` at the source power
board removes the whole feed, every rail with it.

## The long record — how it is captured, and why the second memory

**A capture at fs 2²⁵ behind a half-band ÷2 — infeasible, and the number is the filter's.** One
OCTOSPI at 2²⁶ DDR is 134 MB/s, the raw stream to the byte, so the record was to be halved on the
way in. Passing 16 MHz and holding the fold down past 17,5 under a 16,78 MHz Nyquist is a
transition of 2,3 % of the rate — a half-band of some 60–70 non-zero taps. At 33,5 million outputs
a second, **over two billion multiply-accumulates a second against the M7's half billion**, and
unskippable: without it the 11 m broadcast band and the 10 m amateurs fold onto CHU 7850, RWM, the
Buzzer and the Pip around 4–5,5 MHz.

**The `DFSDM` as that decimator — dropped.** It does take a parallel stream from memory (`DATMPX`
on the internal register, `CHDATINR` written by CPU or DMA), but its filters are sinc, `sinc¹` to
`sinc⁵`, and a sinc ÷2 is no half-band: droop across the passband, a few decibels at the fold. A
DMA word per sample into an APB register at 67 MHz is past the bus, and the words sit in memory
first anyway — memory → `DFSDM` → memory, triple the traffic for the wrong filter.

**A narrower long band, 0,5–12 MHz — dropped.** A 12 MHz top gives a 12 → 21,5 MHz transition,
14 % of the rate: a half-band of six non-zero taps at about three quarters of the core for the
31 ms. It loses CHU 14 670 and WWV 15 000 — the two carriers the band reaches 16 MHz for
(`BAND.md`) — in the one mode built for weak carriers.

**A larger PSRAM — no.** Wires, not bytes: 32 MB is eight times the record, and 64 or 128 MB on the
same eight data lines at 2²⁶ carries the same 134 MB/s. A 16-bit PSRAM would do it on one bus, but
the H7A3's OCTOSPI is 8-bit (16-bit XSPI is the H7R/S family's), and the pad caps a faster bus.

**Why an option and not the build.** The long record is a second `APS25608N` on OCTOSPI2, the
stream split raw, no filter, a four-step FFT across both memories (`FIRMWARE.md` §6). It buys 6 dB
on the weakest carriers and only that — 32 Hz bins against the burst's 128 Hz, the noise in a bin
four times narrower, a coherent carrier whole — returning to a target on the loop's worst azimuth
what the pattern took; timing, the revisit, the ionogram and the sweep are unchanged. **The price
is Marconi's most complex firmware and a second memory**, for what the burst already measures —
the day/night ratio on the strong markers, `foF2` from the sounders' chirps. A site wanting the
weak carriers populates the second memory and sets `LONG`.

## The product test by frequency alone — superseded by the level test

A candidate on `2f`, `3f`, `2f₁ − f₂` or `2f₂ − f₁` of the strong list was rejected outright. **That
rejects WWV itself**: 10000 and 15000 are exactly the second and third harmonic of 5000, and with
5000 on the strong list the triplet the band reaches 16 MHz for could never be promoted, while the
test plan kept 15000. The frequency now only makes a suspect; a suspect is rejected when its level
follows its parents' over the hour, which a product does by construction and a transmitter on its
own path does not.

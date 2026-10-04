★ N.I.C. ★

# Tesla — the graveyard

Rejected alternatives and superseded states, with the reason. Nothing returns from here and
nothing is deleted from it.

## Amplifier

**`THS4551` in the 2 × 2 mm `RUN` — not taken.** Its lower parasitics pay at hundreds of
megahertz; the loop closes at 27 MHz. The reference's `DGK` VSSOP-8 has no bottom pad.

**`THS4541` on S1 — rejected.** 2,2 against 3,3 nV/√Hz, but an `i_n` of 1,9 against 0,5 pA/√Hz
through Tesla's large `Rf1`: **~4 dB worse at 250 kHz**, where the timing lives, for ~7× the supply
current. Marconi takes it on the same equation reversed; it stays the drop-in escape should an
in-band carrier saturate S1.

**`NE5532` (the first build).** Single-ended, ±15 V, and at a noise gain of ~140 its 10 MHz GBW
left no loop gain at the old band top.

## Gain and trim

**The switched scale divider (`Rs`/`Rb` arms bridged by `TMUX1511` pairs) — removed.** Replaced by
fixed gain sized to a 490 nT clip (`HARDWARE.md` §0.4): **a clip set by physics is the same
everywhere**, so a per-site trim has nothing to trim. Eight switch channels went with it.

**`Rf1` near 1 kΩ (an early revision).** GBW mistyped as 150 kHz for 150 MHz. Bandwidth in S1 is
cheap; headroom is not.

**`Rf1` = 22 kΩ (the sensitive build), and its noise table.** The knee of the noise curve, blind
circle 27 km. The 490 nT clip turned ~28 dB of excess sensitivity into headroom, the noise five-fold
lower; **the floor stays under outdoor QRN**.

## Converter and rates

**2²⁰ via OSR 12 at 3 × 2²³ — rejected, then superseded.** 0,8 dB of converter noise and a doubled
capture buffer, for shape alone, on a misread 32,768 MHz ceiling: **OSR 16 at `f_CLK` = 2²⁵ reaches
2²⁰ with 101 dB and a 32-bit frame.**

**The wideband filter.** Its long symmetric response smears a µs edge and rings before it, and its
lowest OSR misses the operating rate. The sinc4 is short, linear-phase, its droop a calibration
constant.

**A faster or better converter.** Timing gains nothing — the band is limited and the CFD
interpolates; amplitude gains 4 dB over the full band, 1,4 dB in the amplitude product, **under
outdoor QRN either way**.

**A simultaneous SAR instead of the ΔΣ — dropped.** A 2 MSPS part is a 2²⁰ part without the sinc4
before the fold. The next rungs:

| class | example | gives | lacks |
|---|---|---|---|
| quad 4–5 MSPS | `AD7380-4`, `LTC2325-16` | four channels at 2²² | **16 bits**, 11 dB of floor below the ΔΣ |
| single 18-bit 5 MSPS | `AD7960`, `LTC2387-18` | ~99 dB, 2²² with margin | **one channel a package**, four on one `CNV`; the `AD7960` is LVDS |
| 20-bit 40 MSPS | `AD4080` | 20 bits, 2²⁵ | one channel, LVDS, its own price and power class |

Four `LTC2387-18` would bring the floor to ~4,5 pT from 6,4, under QRN either way. **The decimation
decides it:** the sinc4 decimates in hardware; four SARs would hand the H7A3 64 MB/s at its serial
ports' limit and 16,8 million samples a second to filter — ~17 cycles a sample. It cannot, and four
tens-of-dollars parts replace one.

**The converter's full-band noise as 25,1 µV — superseded.** That is the sinc4 over its own 240 kHz
noise bandwidth; the FIR that flattens the −15 dB droop lifts the noise with the signal. Over
5–512 kHz: converter 36,5 µV, chain 28,9 µV, **floor 6,4 pT, not 4,4** (`HARDWARE.md` §5).

**The converter's "internal 4,096 V reference" — a misreading.** The `ADS127L14`'s `REFP`/`REFN`
are required inputs (SBASAM0B §7.3.2); the `REF6041` took the position. The same read fixed the
channels (`AIN0`–`AIN3`, not `AIN1`–`AIN4`), the frame (`[8 b status][24 b data]`) and the mode
(`MODE` high, SPI). **The pin-strap mode and the `HDR` strap went:** strapping loses `STATUS`,
`REG_CRC_EN`, `CLK_CNT` and `DEV_ID` with the configuration port; the header is `DP_STAT_EN`, and
float position plus CRC, 40 bits, overflows a 32 bit-time frame.

**`AVDD2` on its own `TPS7A2018` off the 3,3 V interface rail — superseded.** The range was read as
1,8–5 V; at the sheet's 1,74–5,5 V the processor's 1,8 V through the 2,2 µH, 10 µF and bead arrives
at **1,774 V at the worst corner**. The LDO is gone.

**`STATUS` polled once a capture block — superseded by the `ERROR` pin.** 200 polls a second burst
beside a running modulator for an event that almost never comes. `ERROR`, the OR of the seven flags,
interrupts on PD2; `STATUS` is read on its edge only.

## MCU (the DSP tier — shared with Marconi)

**"In a tight SIMD MAC loop the gap narrows to the clock ratio" — wrong twice.** The clocks are 2²⁸
and 2²⁷, a ratio of two, and the dual-issue M7 out of TCM sustains twice the M33's MACs a cycle:
**~4× on that loop, 2,9× on Dhrystone** (`HARDWARE.md`, *Why the H7A3*). The memory and the sixth SPI remain
the reasons.

**STM32H725 — rejected; the compute does not decide it.** 1177 DMIPS against 599, but **the clock
was never the shortage**: everything on is
~17 % (`pip` up to 5 %), the classifier 5,1 % at 1024 events/s, and a streaming ladder leaves the
cache nothing to hold. **Its 564 KB against 1,4 MB trades away what an upgrade needs** — classifier
weights, context, raw-waveform recording — and Marconi's capture burst is
exactly the 1 MB of AXI SRAM, so the tier would split. **Upgrade trigger:** a classifier that
outgrows the H7A3 on real events; `DFSDM`'s nine hardware sinc filters are read first.

**GD32H757 — rejected, the recorded backup.** A 600 MHz M7 on paper; in the errata TCM wait states
above 350 MHz, ITCM ECC alarms at 600 MHz, a broken USART FIFO and OSPI DMA, and D-bus access that
erodes the clock advantage. **Backup trigger:** H7A3 price or availability failing at the
assembler.

**STM32H7R/S — rejected.** 64 KB of bootflash forces external XiP flash: **a continuously clocked
wide bus beside the antenna**, against the EMI doctrine.

**STM32N657 — deferred, not rejected.** The M55, 4,2 MB of SRAM and an int8 NPU sit behind the
converter; **what binds the board sits in front of it**: `fHSE_ext` 16–48 MHz against the 2²² wire,
timers capped at 240 MHz (half the ranging resolution), no internal flash, BGA only. The upgrade
trigger's classifier would decide it.

**STM32V863, LQFP176 with 2 MB of eNVM — a footnote until its datasheet exists.** The tier's
package, its own non-volatile memory, `fHSE_ext` 4–50 MHz (the 2²² wire straight onto `OSC_IN`),
`T_J` 140 °C. **Two unpublished numbers decide it:** the 16-bit parallel
interface's clock ceiling (it must take 2²⁶) and the timers' against the 2²⁸ kernels; then the order
code, a GPDMA ring in hardware, the ePCM's 1000-write endurance, and programming after reflow.

## Filters

**LC ladders on the board — rejected.** **The antenna is the receive chain's only wound part:** no
SMD inductor beside it, no self-resonance in the band. The soft knee of RC poles is paid
deliberately; pole stagger, the FIR and the calibration straighten it.

**Chebyshev, or any high-Q family.** Skirt bought with Q is paid in group-delay ripple, and Tesla
times edges: **flat delay outranks stopband**.

**A third analogue pole against the alias fold — 120 pF at the `AIN` pins.** 2,3 dB of stop band for
2,0 dB at the band edge; the footprint stays, the part does not. The one coherent in-band fold gets
a digital notch. **The datasheet's 2 × 22 Ω + 2,2 nF** answers a kickback the precharge buffers
already remove; unpopulated.

**Droop at the band top traded for fold rejection — closed.** 6 dB of droop at 512 kHz buys 3,2 dB at
the first fold for 2,9 dB of high-strip noise, the converter's noise rising with the signal after
the FIR; 10 dB buys 7,4 for 6,7. **The rise-time strip pays, for MW carriers the notches remove
anyway.** The chain stays flat (`HARDWARE.md` §2).

## Antenna

**Four rods at 90° azimuth — superseded by three at 120°.** α and α+180° are the same figure-8, so
**four axes at 90° give two distinct responses**; three at 120° give three, and a better worst-case
dip, on one channel fewer. A fourth at 45° would buy ~0,6 dB.

**300 turns (+6 dB).** Already under atmospheric QRN, the 6 dB buys no reception and doubles every
in-band carrier at S1. Once under the sky, stop.

## Temperature and the reference

**`REF3030` on `VREF+` — removed.** A 3,0 V reference from the H523 days whose 75 ppm/°C is ~1,7 °C
worst case across −40/+60, **no better than `VREF+` on `VDDA` corrected against `VREFINT`**: a part
and a `VREF+ ≤ VDDA` argument for nothing.

**`LMT86` ×5 — superseded by NTC dividers.** Volts out, so only as good as `VREF+`, at the cost of a
reference, a 5 V run and a third wire per arm. Its 2522 mV at −40 °C clips a 1,8 V `VREF+` below
~+26 °C and the 2,5 V `VREFBUF` below −38 °C. **A divider outputs a fraction, which is what the
converter measures.**

**A reference plus a divider, for a narrow window under 1,8 V — dropped.** The 10 kΩ / NTC divider
spans 0,95 → 0,21 of full scale across −40/+60 (~9 LSB/°C worst at 12 bit); resolution is not the
limit (§7). `VREF−` belongs on `VSSA`, so only the top could narrow — with the part just removed.

## Records and protocol

**EMP as record type 2 — refuted.** No lab EMP exists to calibrate against, and **the rod cannot
see one**: the Mn-Zn core absorbs the E1 edge above ~1 MHz and saturates at ~400 mT, so any
amplitude is an artefact, indistinguishable from a sferic 200 m away. Survivability is input
protection — series R, TVS clamps, a common-mode choke — not the core. Its slot went to the SID
task, now Pip's (below).

**The supplement inside the event record — rejected twice.** *Azimuth in the 4 B record:* five bits
cost three of amplitude and two of reach for a field 2–3 times finer than a
single stroke's 10–15°. *A zero magnitude in a `type 1` record as a free codepoint:* 19 bits for
`slot | rate | band | lock | max−mean | count` in 3, 3, 3, 2, 5, 3 — **against the rule that nothing
is packed into a half-byte**; cheap in `UBFX`/`BFI`, implemented wrong once and read wrong forever.
The supplement is four whole bytes in a NOD of its own (`BUS.md`, *The supplement record*).

**The 18-bit offset and the 11-bit amplitude — withdrawn.** 18 bits at 2⁻²² s and 11 at 2⁻⁴ dB cost
two magnitude bits and **broke the station's shared rule** — word A 16 bits of context, word B a
14-bit quantity << 2 | type. Now 16 bits of 2⁻²⁰ s ticks, sign + 13 bits at 2⁻⁶ dB and 2 type bits:
nothing straddles a byte, and the rounding is the budget's largest term (`../core/WHY.md`).
**Linear amplitude stays rejected** — 13 bits over 490 nT is a 59,8 pT LSB against a 6,4 pT floor;
98 dB linear needs 17 bits. **The sign bit stays** — the stroke's polarity, which the TOA-derived
azimuth cannot recover and the ±CG correlation with Quark's TGFs needs.

**`LAT`'s converter term as the datasheet's latency time — superseded.** 4,79 µs (22 ticks with the
front end) is the whole impulse response, `4 · OSR − 3` modulator cycles, plus a ~39-clock pipeline.
An edge sees half the response, `2 · OSR − 2` cycles (1,79 µs), plus the pipeline (1,16 µs):
2,95 µs, 14 ticks. **Every record was 1,9 µs early**, past the ±1 µs contract.

**Strike-capture timers as a deployment knob — superseded by the stream detector.** A few to eight
timer capture channels, by network density, for near-simultaneous strikes. An event's time is now a
sample index (`FIRMWARE.md` §5–6), **bounded by the event queue and the eight slots a frame**, and
nothing is configured per deployment.

## The SID channel and the Pip task — moved off Tesla's image

The SID channel (a 2¹⁷-stage band scan) was a default-off task in
Tesla's image; Pip was a second task (`TIME OUT` ranged by TIM15), then a unit in
`tesla/FIRMWARE.md`. Now Tesla and Pip are one board under two images, the SID channel is Pip's
(NodBus type 12), and Tesla keeps the floor record as `type = 2`. **A carrier tracker and a lightning
detector want the core at the same moment, the storm, and the tracker lost every time**; a
default-off task goes untested; and Pip needs the carriers' phase, which the level task never
computed. `TASKS` shrank to two bits, `CARRIERS` became `NOTCHES`, and USART2, TIM15 and two DMA
streams left the budget.

## 16,7 Hz and 25 Hz AC traction as tracked sources — dropped

The tracker's envelope floor is 100 Hz; the 33 Hz and 50 Hz envelopes of 16,7 Hz and 25 Hz catenary
ship as UFO. **A fault there is mechanical and clears in a few cycles** — nothing to trend — and
0,3 s at 33 Hz is 10 firings, too few for a circular-variance lock. 50/60 Hz catenary and DC
traction's ripple track like a power line; `SRC_FMIN` at 33 admits the legacy systems
(`FIRMWARE.md` §8).

## The station interface as a translator class and an I²C shifter choice — superseded

Once a translator class (`SN74AXC4T245`/`8T245`), the I²C on `TXS0102` or `PCA9306`, `LINE_EN`
tied on, `B_DIR` a strap. **The `TXS0102`'s auto-direction one-shots are what the clock lines
already refused**: the I²C is a `PCA9306`, beside two `SN74AXC8T245` with `DIR` strapped per package
and 100 kΩ on every inbound line (`HARDWARE.md` §8).

## `TIME OUT` on a crossed in-box cable, and the strap read-back — superseded

`TIME OUT` once took a crossed in-box cable, set by the role-strap rule and checked by the strap
read-back. **A measuring unit is never in the box**, so it always carries a Galvani board; `ID_RET`
is not connected, and the socket's 10 kΩ sets `B_DIR`. Withdrawn too: a 1,8 V rail beyond the
port's `ENABLE`, which at the source power board removes the whole feed.

## Earlier states of the board

- **The chain's poles at 362–531 kHz** — 24 dB of alias rejection for −14,7 dB at the band top;
  with the first fold at 536,6 kHz no pole placement filters, so the poles went an octave up and
  the notches carry the folds.
- **Band tops of 70 kHz (~5 µs rise) and 250 kHz (~1,4 µs)** — superseded by 512 kHz.
- **The core free at 280 MHz with `CLKIN` from a second PLL** — now tied to 2²⁸: one PLL, less
  jitter.
- **2²⁴ at OSR 16, 2¹⁹ SPS** — Nyquist below the band top.
- **Four slaved SPIs for the data port** — eleven pins in the 1,8 V `VDDIO2` domain against a
  published ten; SAI shares clock and frame sync.
- **`VDD` 1,8 V with `VDDA` 3,3 V** — sequenced, and pointless once the thermometers went
  ratiometric.
- **The winding a full-length single layer at 1,33 mm pitch** — it links the average flux, losing
  `µ_eff`; now the centre third.
- **`R_w` 10 Ω** — an earlier build's series resistor; the winding is ~1,0 Ω.
- **150 turns of 0,5 mm on the centre third** — 0,55 mm over the enamel is 82 mm of single layer,
  and the centre third of the 190 mm reference rod is 63 mm. Now 0,315 mm, ≤ 0,352 mm over the
  enamel: 53 mm, the section gaps inside the third; `R_w` rises to ~1,0 Ω and S1's lower corner
  from 1,9 to 2,0 kHz, which nothing else feels.
- **µ_rod from the prolate spheroid of the rod's `l/d`, and µ_rod ≈ 140 for the Amidon rod** — two
  models that disagreed: the spheroid read 83 for the 12,7 × 190 mm rod and the text 140, and
  `l/d` 15 → 30 was written 142 → 328. Solved for the cylinder itself (`design/rod_field.py`), the
  centre third the winding sits on reads **94** for the Amidon rod, 140 for the 200 × 10 mm
  prototype, 105 for one H7 and 250 for the pair; `rod_calc.py` takes the spheroid at
  `l/d` × 1,088, which matches the solve within 3 %.
- **The clip at 490 nT and the floor at 6,4 pT** — computed on an antenna of `N·A·µ_eff` ≈ 2,34 m²,
  µ_eff ≈ 200 on the 200 × 10 mm prototype. On the solved 140 (1,65 m²) the same `Rf1` and the same
  2,5 mH clip at ≈ 690 nT and float at ≈ 9,1 pT — every field-referred figure × 1,41, the µV figures
  and the 98 dB unchanged — and the blind circle closes to 2,9–4,3 km. `design/chain.py` computes
  both from the parts; run on the old antenna it returns the old 490 nT and 6,4 pT within 2 %.
- **`Rf1` re-sized to ~6,34 kΩ to keep the clip at 490 nT** — weighed and not taken: `Cf1` would
  follow to ~22 pF and the floor fall to ~7,3 pT, a cascade through the chain for a clip nothing
  needs lower. `Rf1` stays 4,42 kΩ and the clip is ≈ 690 nT; the 9,1 pT floor is under QRN.
- **The `ADS117L14`**, the 16-bit sibling — three bits below the `ADS127L14`, under this chain.
- **A pipeline converter** — no oversampling; ~12,5 ENOB at 80–125 MSPS, 11–14 dB lost.
- **The NTC read over 20 ms** — one 50 Hz period; 100 ms spans whole periods of 50 and 60 Hz.

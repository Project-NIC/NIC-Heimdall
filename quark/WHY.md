★ N.I.C. ★

# Quark — the graveyard

Rejected alternatives and superseded states, with the reason. Nothing returns from here and
nothing is deleted from it.

## `Quark` as the tube unit's name, and the LV units loose under `quark/` — superseded

The tube unit was bare `Quark`, like the folder and the part, and the scintillation units stood
beside `tubes/` as `photon/` and `positron/` (`neutron/` inside `positron/`). **They share no
part**: HV tubes and counts against LV scintillators, counts and energy. `Quark` now names the part
and folder only; the units are **Quark-Tubes** and **Quark-Scintillation**, `scintillation/`
(`photon/`, `positron/`, `neutron/` side by side). Photon keeps its name in both: one quantity,
measured both ways.

## The 12 V distribution — superseded

A 12 V rail in the BOM: tube channels at ~15 mW, an MCU board on 3,3 V at ~50 mW (~110 mW in all),
three-wire cables up to 2 m, a "NOD bus" home run. Superseded: power is the battery rail as-is, no
12 V or 24 V bus (`../core/POWER.md`), and Quark-Tubes sits on an Argus mini segment.

## The neutron channel's slot and the fourth GM tube — K4, then K5, then K4 again

The neutron count first sat in K4 of a four-byte radiation block in the weather node's payload,
Quark a ModBus slave to Palatine on an arm carved from a fifth port, until NodBus mini. It then
moved to K5 so K4 could take a fourth GM tube — unshielded, mica window, ~20 cm over the deposition
plate, β + γ + X as the beta channel against a 1 m reference, first Photon's, then Positron's.

**It went when the 1 m × 1 m plastic deposition plate came into every build**: the three tubes sit
at one height, 0,5 m to at most 1 m, and beta reaches the bare tube through its thin stainless wall.
K4 is the neutron detector again; K5 existed only for the fourth tube. Gone with it: a second
height, snow burial, the mica window, a fifth input.

**One tube type per build.** The `SBM-20` / `SBM-19` / `SI-22G` mix, with its ×1 : ×3 : ×10 ladder
and per-tube normalisation, is gone: one of the three everywhere, calibrated per type.

## "Variant B" — the scintillation build as a mini-NOD on variant A's contract — superseded

Variant B was the same mini-NOD, 8 B frame and five `uint8` counts. **An 8 B frame carries four
values**: counts and energy for three quantities do not fit, and three quantities are three units.
They became NodBus units of their own — Photon (type 9), Positron (11), for a time Helion (10;
*Helion as a NodBus type of its own*, below); Quark-Tubes stays counts only, four `uint16` channels.
Hard/soft gamma banding stays there: a tube has no energy, so bands are its only spectrometry.

## Photodetectors that cannot do the job

**Bare silicon for MeV gamma spectroscopy.** Thickness, not price. Probability that a photon
interacts at all in 300 µm:

| | 10 keV | 20 keV | 30 keV | 60 keV | 100 keV | 662 keV |
|---|---|---|---|---|---|---|
| bare Si, 300 µm | 91 % | 19 % | 7 % | 2,1 % | 1,3 % | 0,54 % |

At MeV the photon Comptons once and the recoil electron leaves the die: the deposit is a random
fraction (~80 keV most probably, of 662), **a continuum, not a spectrum**. The scintillator absorbs;
the diode reads out. The same figure killed **the bare dose diode**: a Compton smear is not a dose,
which now rides the crystal PIN in current mode.

**A photodiode as a radiation detector.** Microns of depletion over undepleted bulk: charge arrives
late by diffusion, partly recombined, and pulse height means nothing.

**A solar cell.** The same bulk, and a µA-class dark current drowns what charge arrives.

**Perovskite (`CsPbBr₃`, `MAPbI₃`).** Good absorption, room temperature, real photopeaks. **Killed
by ion migration**: under bias the response drifts over hours, beyond the calibration chain.
Watched, not built on.

**The bought CsI + PIN module** — a survey meter's inside. **Without gain there is no 1 p.e. peak**,
so no calibration chain. A fallback if the two-window assembly cannot be had.

## Scintillators not taken

**`GS20` bulk `⁶Li` glass** — the original neutron proposal. A solid of appreciable Z sees far more
gamma than a thin gas, and in a strong field small gamma pulses stack into a fake capture. The thin
`⁶LiF/ZnS(Ag)` screen lacks the fault.

**`CLYC`** — gamma and neutron by pulse shape. Dear, hygroscopic, pointless once separate heads
exist.

**A phoswich** for beta-in-gamma — software separation where two mutually blind detectors give it by
geometry.

**`GAGG:Ce`** — denser (6,63), faster (90–150 ns), better resolution, not hygroscopic. Parked, not
rejected; no neutron replacement despite Gd-157 (~254 000 b): the 8 MeV cascade partly escapes a
small crystal.

**`LYSO`** — Lu-176 gives it hundreds of counts/s/cm³ of its own.

## Front ends not taken

**A load resistor and an op-amp on the SiPM.** The resistor's signal voltage subtracts from the
overvoltage: at 300 Ω a 14 MeV muon consumes 107 % of it and the chip switches off mid-event. **The
readout must be a virtual ground.**

**The discrete-JFET summing front end and the module A/B split.** Module B, a segmented PIN on N
JFETs into one summing node (ENC 364 against 1034 e⁻, a secondary gain), beside module A, a SiPM
into a `THS4551`. **Killed by JFET spread**: `I_DSS`, pinch-off and `gm` vary part to part, so every
build has its own operating point and every substitution is an engineering change. Now op-amps only,
an `LTC6268` section per segment. Gone with it: a MOSFET input (1/f noise on a charge node), a slow
`TLV9061`-class op-amp plus driver (three stages where two serve), an op-amp per segment (sixteen
price out the front end).

**A bipolar common-base input on the energy channel.** Right for a fast channel, but its emitter
shot noise integrates over the shaping time: the `C/√τ` term falls, the `i·√τ` term rises, they
cross near 50 ns, and at CsI's microsecond it loses by 20–80×.

**Charge-amp op-amps not taken:** `OPA4863` — bipolar, 0,4 pA/√Hz (~2 500 e⁻ over 2 µs), up to 1,2
µA bias, 12 V across a 10 MΩ `Rf` · `OPA810` — no quad; `V_S(min)` 4,75 V forces a fifth rail ·
`OPA4354` — ~6 nV/√Hz, twice the noise · `OPA607`/`OPA2607` — decompensated G ≥ 6, unusable on a
low-noise-gain node · `OPA859` — its 3,3 nV buys ~10 keV of threshold, under the channel's
resolution, for a line item beside the `LTC6268`.

**`THS4541` as the converter driver.** The 150 pF at the pins absorbs the kickback, so the driver
serves the ≤ 6,5 MHz signal, not the encode rate; the 1,35 mA `THS4551` covers it.

**Three-stage chains, and a separate charge stage on the SiPM channels.** Two op-amps per channel is
the ceiling and the SiPM needs one, the `THS4551`; the extra stage was only uniformity with module
B.

**`Cf` 1 pF, and `Rf` chosen for tail length.** 1 pF is 20–50 % humidity-dependent parasitics; the
C0G part must dominate. `Rf` is a noise element (`√(4kT/R)`: 1 MΩ ~800 e⁻, 4,7 MΩ ~440), and
pole-zero removes the tail digitally.

**RF / MMIC gain blocks.** Specified in dB, GHz and dBm into 50 Ω, not in pulse height, and AC
coupling leaves no baseline.

**A shaping amplifier.** Shaping is arithmetic on the H7A3; an analogue stage undoes the reason for
that processor.

## The FMC — not used

It reads in bursts, and a burst boundary costs samples on a converter that does not wait. The port
is the PSSI.

## Converters not taken

**The MCU's own ADC** at 3,6 MSPS: 3,6 samples across a 1 µs CsI pulse, none across a 54 ns
microcell. Its 16 bits did not matter: **a histogram cares about DNL**, and 14 bits into 1024
channels average four codes each. Superseded by the `AD9251-80`.

**`ADS4245`** — 11,5 bits ENOB against the `AD9251-80`'s 74,3 dBFS, and a second part number.

**A single converter with a multiplexed front end.** The gamma crystal's SiPM and PIN must be
simultaneous.

`LTC2299` was never here; `LTC2143` was rejected on Marconi, on price and stock
(`../marconi/WHY.md`), which does not transfer.

## One scintillation PCB, populated twice

One board built twice per build — a SiPM into a `THS4551` on channel 1, a second SiPM or the
1–4-segment `LTC6268` PIN input on channel 2 — as cheaper than two. The PIN then shared a layout
with a front end it lacks: empty footprints are stubs and leakage paths beside a 0,42 fC node (200
keV), and the guard ring routes around absent parts. **Two builds differing by a part type and a
layout feature are more than a population.** Now `Quark-Photon` and `Quark-Neutron/Positron`, at a
second fabrication order and BOM.

## GM tube families weighed

Some fifteen types; the Soviet stainless-wall `SI-22G`, `SBM-20`, `SBM-19` stayed. American parts
lost on sensitivity or price, Chinese on sensitivity, Japanese and Western on price. Surplus Soviet
tubes are sensitive, cheap and plentiful, and Gadolin/Rhodion needs thirteen of one.

## A push-pull HV supply instead of the flyback and its multiplier

A catalogue transformer making the ~400 V base directly would have dropped the multiplier ladder,
its area, film capacitors and leakage. **No catalogue winding reaching 400 V was found**, and the
alternative ~20-stage ladder is worse than what it replaces. The flyback and multiplier stay.

## The scintillation record before the accumulators — superseded

Five `uint16` a second (count · minimum · maximum · mean · median energy), a `uint32` dose
accumulator and the event count every frame, and a 256-bin log histogram on request. Now one 32 B
layout, every field an accumulator reset on the second. **Only what adds may ride a frame**: a
second is the sum of its 128 frames; a mean or a median cannot combine, and both are exact
downstream (sum ÷ count; interpolation in the band crossing 50 %). The minimum restated the
threshold; dose is the energy sum times a constant; the on-request histogram was missing in every
burst. Per-frame values lost because a lost frame loses its particles, where an accumulator's next
frame still carries the total.

## Photon on TWO NUMBERs, one channel each — dropped

SiPM and PIN as two records, so the archive held their ratio at frame resolution and showed the SiPM
saturating. **Two instruments with overlapping windows are one measurement**: the handover joins
them per event (SiPM below 500 keV, PIN above 700, a blend between). The watch survives as a
verdict: their area ratio against a persisted constant, over 10 % out `DIAG DEGRADED`. The second
NUMBER would have bought the evidence at a permanent slot of the eight on a card.

## The burst channel as counts per time slice, or as per-particle timestamps — dropped

Sixteen or thirty-two slices of particle counts, or per-particle arrivals to the microsecond.
**Counts do not survive the event**: the ~1,5 µs shaping is the pulse-pair resolution, and at 10⁷/s
everything piles up and the SiPM exhausts its cells. The current-mode PIN resolves microseconds but
has no particles to stamp. The slices carry PIN current-mode energy: sixteen fixed slices of 488,28
µs every frame, no trigger.

## Helion as a NodBus type of its own — superseded

Helion held type 10, `Quark-Neutron/Positron` answering as two NODs under *one quantity per NOD*, a
rule from when the neutron channel was to carry energy. A `⁶LiF/ZnS(Ag)` screen and a discriminator
give a two-byte count: **its own address bought a 32 B payload for two bytes** and a card slot — and
`Quark-Tubes` already put four channels in one 8 B payload. The count is now the last word of
`Positron`'s record; type 10 is reserved for `Neutron`.

## Flags in the payload — superseded by the frame header

A flag word: pile-up, enormous event, burst, handover out of tolerance. The header's `status` byte
holds `6 CLIPPED`, what pile-up makes of a count (`../core/PROTOCOL.md` §1); the enormous event is
the largest-event field, a burst shows in the counts, the handover alarm is a `HEALTH` item. The two
bytes gave Photon a twelfth band.

## 28 bands as the base build — kept as an option, not taken

Finer bins merge downstream and coarse ones cannot split; **the obstacle is counting statistics**.
At ~500 events/s over 2,7 decades, a second puts ~42 events in each of twelve bands (±15 % Poisson)
and ~18 in each of twenty-eight (±23 %). Finer binning pays only over minutes, which downstream gets
from twelve, and a spent slot stays spent. Narrower fields fail: a peak band holds tens of thousands
at burst rate, so `uint16` is the floor and 24 B holds twelve. The option stands in
`scintillation/photon/BUS.md`: a second NUMBER of 16 further bands.

## A faster converter, four ways — all dead on one number

Opened by pulse-shape discrimination. The PSSI receives at 100 MHz with `PDCK`/`f_HCLK` ≤ 0,4 (107
MHz at a 2²⁸ kernel); two interleaved channels share the pins, so the encode stops at 50 MSPS, and
21 × 2²¹ already runs them at 88,08 MHz, 12 % margin. **A faster converter has nowhere to put the
samples.**

① Interleaving the `AD9251`'s two channels — impossible: one `CLK` pair and divider per chip;
`SYNC` only aligns chips. ② Two `AD9251` in antiphase, `CLK+`/`CLK−` crossed — the pins would run
at 2²⁷. ③ `AD9255`, 125 MSPS on 1,8 V — the same wall (`AD9245`, named earlier, is a 3 V part).
④ `SD9268-80` (Silanna), 2 × 16 bits at 80 MSPS — the same ceiling, ~1,3 dB from the extra bits,
355 mW against 146 mW. The `AD9251-80` stays on 1,8 V; 3,3 V would need a second rail scheme for a
rate the bus caps anyway.

## MCU

**STM32N657** — Cortex-M55 with the Neural-ART NPU, deferred, not rejected: the 2²² wire needs an
input multiplier (`fHSE_ext` 16–48 MHz), sampling is no faster (the same PSSI and `PDCK`/`f_HCLK` ≤
0,4, setup 3,5 ns against 2), BGA only, no internal flash — for a classifier nothing has asked for.

**STM32V863** in LQFP176 with 2 MB of eNVM — a candidate, not a decision: this tier's body, its own
non-volatile memory, `fHSE_ext` 4–50 MHz. **Two unpublished numbers decide it**: the parallel camera
interface's clock ceiling, which must take 2²⁶ = 67,108864 MHz, and the timers', which must take 2²⁸
(at 240 MHz the ranging resolution halves). None of it, `fHSE_ext` included, is from a datasheet;
order code, GPDMA ring, the ePCM's 1000-write endurance and programming after reflow are
unpublished.

## Digital thermometers — a `TMP117` on `Quark-Tubes` and on the H7A3 boards — superseded by NTCs

`TMP117`s on `I2C2`: on `Quark-Tubes` one for the board and one (or an NTC) per head, briefly twelve
NTCs; on `Quark-Photon` and `Quark-Neutron/Positron` three (or `STS35`) at 0x48–0x4A on the 1,8 V
side, read in a blanked window.

**Nothing on any board asks for 0,1 °C.** The tubes' α defaults to 0, and Gadolin/Rhodion's thirteen
tubes are one counting channel, one temperature. CsI(Tl)'s 0,3 %/°C holds the energy scale to 1 % at
±3 °C, the SiPM bias rides the 1 p.e. servo, the photomultiplier's reading is a rating check. An NTC
on a ratiometric divider puts no clock or edges beside a front end; a `TMP117` per head would have
put a bus without CRC on the pulse cable, one bad head able to hold it. `I2C2` is free on all three
boards.

## A second bias converter for the PIN, and the `S3590` as the part

A second `LT3571` at 50–70 V for full depletion buys a 165 keV threshold where the PIN's work starts
at 500 keV, for a switching converter beside the quietest node. The PIN takes the SiPM's supply
through an RC, at ~260 keV. The noise figures used the Hamamatsu `S3590`, rated −20 to +60 °C like
the other 10 × 10 mm PINs: the reference, not the part.

## A crystal on `Quark-Tubes` — superseded

A binary HSE crystal (2²², 2²³ or 2²⁴) for the core and the UART. It went so the mini tier has **one
clock tree** — the rung disciplining the HSI through `FRACN`, as on Gauss and Pascal, where the body
forbids a crystal; the board gained PH0 and PH1. No holdover table, unlike the sondes: a lost rung
mutes it anyway.

## The converter's encode from `TIM3_CH2`, and PWM-input on the `FO` timer

The `AD9251`'s encode, 21 × 2²¹, as `TIM3_CH2` on PA7 off PLL2 (M 1 · N 42 · P 4, VCO 176 MHz), the
`FO` width in PWM-input mode on CH1. **A timer divides by an integer**, and 2²⁸ / (21 × 2²¹) is
128/21: the encode leaves on `MCO2` (PC9) from PLL2's `P`, N 84 · P 8 at 352 MHz (a 4,19 MHz input
drives only the wide VCO, 128–560 MHz on this part). PWM-input puts one input on both capture channels, so it shares
`TIM3` with no output and serves one tap; the `FO` widths are plain both-edge captures.

## The scintillation Helion — the `⁶LiF/ZnS(Ag)` screen read by a SiPM

The screen on a 6 × 6 mm SiPM through acrylic, on channel 1 of `Quark-Neutron/Positron`, with energy
and a shape match. **The screen answered to everything** — light, gamma, whatever reached the body —
and the light budget hung on self-absorption through the acrylic, which nobody could derive. A
photomultiplier on the kV supply reads it now (`scintillation/neutron/HARDWARE.md`); its count fits
Positron's frame. Channel 1 is named `Neutron` since.

## The SiPM's fast output into a timer — deleted

A ruler fallback: `FO` through an `LTC6268` and a time-over-threshold converter (peak-hold,
constant-current discharge, comparator) into a `TIM3` capture, routed unstuffed. **A timer capture
wants a settled level**; a peak-hold on nanosecond spikes gives none, and the amplifier and
discriminator that would were never worth drawing for a fallback. The ruler is the anode stream's;
PB4, PB5 and `TIM3` are free.

## The PSSI clocked from the converter's `DCOA` — superseded

`MCO2` at 44,04 MHz into the `AD9251`'s `CLK`, `PDCK` from `DCOA` as "the word rate, 88,08 MHz". Per
the datasheet's Figure 3 the interleaved output is double data rate — `DCOA` at the encode rate, one
channel per level — so **a one-edge PSSI would have read one channel**. Now `MCO2` makes 88,08 MHz
for `CLK+` and `PDCK` through a `74AUC2G34`, the converter divides by two, `SYNC` fixes the A/B
parity; `DCOA` and `DCOB` are unconnected.

## The ladder from clean singles, and `Cf` 10 nF on the gamma head's SiPM — superseded by the cumulant ruler at 1 nF

The 1 p.e. ladder histogrammed clean single pulses under 4 p.e. from a 1 ms stretch a second, `N`
the peak spacing; the charge stage was `Cf` 10 nF ∥ `Rf` 150 Ω, set by 14 MeV at `Q/Cf`.

Two numbers killed it. A 6 × 6 mm SiPM at 25 °C fires ~3·10⁶ cells a second, so on a 1,5 µs tail
four or five always decay under any one (a lone cell: e⁻⁹): **there are no clean singles**. And at
10 nF a cell is 0,37 LSB against 1,29 LSB of converter noise, beyond any filter. Replaced by
Campbell's theorem: a random train of identical pulses has cumulants `λ·Nⁿ·∫hⁿ`, so
`N = (4/3)·κ₄/κ₃` of the raw stream between events, Gaussian noise adding to neither. Variance over mean
lost: they carry the amplifier offset (DC noise gain 14) and the converter floor, both drifting with
temperature, and a bias-off reference waits ten seconds for the reservoir to discharge. The ratio
needs cell variance above the converter's, so `Cf` became 1 nF ∥ 1,5 kΩ (step 3,66 LSB), on Positron
1,5 nF ∥ 402 Ω; the range lost above ~3,3 MeV was already the PIN's.

## An isolation buffer before the converter on the SiPM channel — dropped

A second `THS4551` before the converter's RC, routed unpopulated against a moving kickback pedestal.
The 150 pF absorbs the kickback, the 24,9 Ω arms recharge it, the loop gain divides it: **what is
left is a fixed pedestal the baseline subtracts**, and an empty position is a stub beside the front
end.

## A head connector, and a head board behind it — not made

A cable to the heads wanted a head board and a connector the bus's bodies lack. A SiPM sits on a
board anyway, so the head board was a stub beside the charge node and the connector a second one.
**The heads sit on the main board's underside**; the only cable out is the Galvani data body's.

## The He³ head as drawn on the V1.0 sheet — superseded

A charge preamplifier on 5 V biased at 2,5 V; thresholds floating on a background average "so no
temperature sensor is needed"; RC integrator and CR-RC² shaper footprints; a stretcher per
comparator; the gamma count A − B with nowhere to go; a note about photodiode or silicon fronts.
Briefly, a two-comparator window whose AND was one pulse to `K4`.

`Quark-Tubes` gives the head 3,3 V only; a divider threshold does not float, and drift is the NTC
correction's; the `Rf · Cf` tail is all the shaping a counter needs, and empty footprints are stubs;
a microsecond pulse needs no stretcher; other detectors are not this station's decision. **The
window went because thirteen inputs sit idle** when a tube stands in for the ring: the neutron
threshold on `K4`, the LLD on `K4A`, the gamma count the tube's health in a register
(`tubes/helion/HARDWARE.md`).

## The unit-end sockets before the socket rules — superseded

`Quark-Tubes` drove `LINE_EN` from PE3 "for the bench" and read its rung behind an unnamed "buffer";
the H7A3 boards had `LINE_EN` "tied on" without a value, a "`TXS0102` or `PCA9306`" shifter, no pin
table. All three sit at a run's U end, where **nothing on the Galvani boards is switched by a
processor pin**: `LINE_EN` 10 kΩ to 3,3 V, `ENABLE` and `ID_RET` not connected, `B_DIR` 10 kΩ and
`A_SEL` to ground. PE3 is free; the rung enters `TIM2_ETR` from the data body's pin 1; the shifter
is the `PCA9306`, 4,7 kΩ a side.

## Helion as the name of both neutron builds — superseded

Helion named the He³ / BF₃ tube head on `Quark-Tubes` and the LV photomultiplier channel alike. **A
helion is the He³ nucleus**, so the name stays with the tube (`tubes/helion/`, owner of the kV
source); the other is `Neutron` (`scintillation/neutron/`), and `Quark-Helion/Positron` became
`Quark-Neutron/Positron`.

## The Czech neutron study's data format

The study `detektor-neutronu/01…06` (physics in `NEUTRONS.md`, safety in the README, background in
`tubes/HEADS.md`) used 5 B a sample: 16-bit count, tube bitmask, slot, flags, a GPS second marker.
**The station counts on one shared hardware clock**, so samples align by construction; Quark-Tubes'
mini frame carries the counts.

## Rejected neutron routes

- **Nitrogen as a target** — `¹⁴N(γ,n)` matters only at atmospheric scale (~10⁻⁴ in a block), and
  its ~10,5 MeV threshold is above Gd's 7,9 MeV; `¹⁴N(n,p)` is 1,8 b with a µm-range 0,58 MeV
  proton.
- **Uranium-238** — fast neutrons by (n,2n)/(n,3n), 2,7 b thermal, its own chain a constant
  background; regulated and chemically toxic.
- **Machining beryllium** — berylliosis; graphite reflects almost as well.
- **Boron as a reflector** — boron absorbs (3 840 b); it belongs in a shield.
- **Pancake tubes with a mica window** — the diffuse field wants area, cheaper in cylinders; a thin
  foil captures little, a thick one swallows its product; mica is fragile.
- **A scintillator with a photomultiplier, for the cheap replica** — efficient, but a whole analogue
  chain; taken later on the LV board, which has one: `Neutron`.
- **Silicon detectors** — nonlinear in particle and energy, small area per die, drift and
  preamplifier noise.
- **Soft beta outdoors** — stopped by the stainless wall below ~0,7 MeV, by tens of cm to ~1 m of
  air, and by the weather cover; where it matters, gamma is there and better seen.
- **Spectroscopy on He³** — `³He(n,p)³H` releases a fixed ~764 keV whatever the neutron: one peak,
  for a discriminator.
- **A converter on the tube's outside for a charged product** (Gd conversion electrons, Li alpha and
  triton) — the wall stops them; only an activation foil's hard beta crosses.

## A structural filter in place of the lead — not taken

The Pb↔metal equivalence is energy-dependent: below Pb's 88 keV K-edge ~1 mm Pb ≈ ~5,5 mm steel,
above it ~10–20×; tungsten (K-edge 69 keV, ~0,4 mm ≈ 0,5 mm Pb) matches the spectrum, steel does
not. GRP over lead keeps the clean Pb spectrum without recalibration.

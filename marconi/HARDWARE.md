★ N.I.C. ★

# Marconi — the board

> **Design-stage concept.** The figures are verified against the parts' sheets and the parts
> before a board is made.

**One board in one box at the loop's feed**: the loop's front end, the band-defining RC chain, the
`AD9265-80` converter sampling the whole 0,5–16 MHz band at 2²⁶, the STM32H7A3IIT6 reading it on
the PSSI, the PSRAM, the rails and the station interface. Why the band and the targets are what
they are is `BAND.md`; the loop's build and mount are `CONSTRUCTION.md`.

```
     the loop, 1 m, one turn, 2,9 µH
          │
        S1 - S2                               LMH5401 then THS4541, transimpedance into a virtual short
          │
         ch1                                  AD9265-80 — 16 bit, one channel, 2²⁶ SPS
          │
      PSSI 16 b                               one word a clock
          │
      H7A3IIT6  ── OCTOSPI ── APS25608N PSRAM
```

## 1. The loop — an air-core field sensor, and its size is not set by the wavelength

**A shielded loop into a transimpedance virtual short is a field sensor, not a radiator.** Its
short-circuit current is

```
I = N · A · B / L
```

— frequency-independent, because ω cancels. `L` grows with `N²` while area grows with `N`, so
**one turn is the optimum** and more turns lower the output. The current then scales linearly
with radius and only logarithmically with conductor thickness. λ at 5 MHz is 60 m; the loop is
λ/60.

| **one turn, 1 m across, 10 mm tube** | |
|---|---|
| inductance | ≈ 2,9 µH |
| \|Z\| at 5 MHz | 92 Ω |
| self-resonance | ≈ 35 MHz — **above** the band; it lifts the response **+2,1 dB at 16 MHz**, which calibrates out |
| flat at 0,5 MHz | ωL 9,2 Ω against 18 mΩ of copper |

**Not a ferrite rod.** Mn-Zn is lossy above ~1 MHz, and a wound rod's self-resonance (≈285 kHz on
Tesla's ~150-turn rod) sits far **below** 0,5–16 MHz, where the air loop's sits above it. That is
also why Marconi carries no VLF channel: **the D-region is Pip's, on Tesla's rod.** The loop's
build, its orientation and its mount are `CONSTRUCTION.md`.

### Nothing goes in series with the loop

**The loop is bare: no series resistor beyond the clamp's two 2 Ω (§2), no series capacitor.** Both were worked out and both are
rejected, for one reason that outranks their own numbers.

**A virtual short makes the response antenna-independent in *shape*.** `I = N·A·B/L` is flat, in
phase with `B`, from the winding-resistance corner up to where the amplifier stops holding zero.
Change the antenna and the *scale* moves — one number in the calibration — while the band edges do
not, because they live in the electronics. **A series element is the only thing that would break
that**, since its corner `R/2πL` or `1/2π√(LC)` follows the coil.

That is also the answer to why there is no impedance matching anywhere on this board. Fifty ohms
is a convention for **transmission lines**, and there is no line here — the amplifier is
centimetres from the loop. And it could not be done regardless: the loop swings **9,2 Ω at
0,5 MHz to 296 Ω at 16 MHz, 32:1**, which no passive network matches across a band. Every active
loop ever built takes the short-circuit current or the open-circuit voltage for the same reason.

**And the damping argument does not apply either.** The loop self-resonates at 34,8 MHz with an
undamped Q in the thousands, but with both terminals held at the virtual short its capacitance has
no voltage across it and the tank never forms — the short holds to 58,9 MHz here, well past it.

**A footprint for a series `C` stays, unpopulated** — for a site that turns out to sit under a live
medium-wave transmitter.

## 2. Surge — a clamp, not a transformer

**There is no transformer anywhere in this chain, and the reason is that there is nothing to
isolate.** A transformer separates two grounds at different potentials. The loop is a closed
conductor with both ends in the FDA's inputs, floating at `VOCM`; nothing arrives along a cable,
because the surge is **induced in the loop itself**, and a transformer would pass that straight
through. The isolation this board needs is one step further out and already exists: **everything
leaving the enclosure crosses a Galvani board** (`../galvani/README.md`).

**What arrives, in numbers** — a 30 kA stroke, differential EMF over a 1 µs front:

| distance | `B` | EMF | loop short-circuit current |
|---|---|---|---|
| 100 m | 26,7 µT | **21 V** | 7,1 A |
| 30 m | 88,9 µT | 70 V | — |
| 10 m | 267 µT | 210 V | — |

**Tens to hundreds of volts differential, and amps into a virtual short.** The kilovolts are
**common mode on the whole island**, which floats and is therefore not stressed.

**A transimpedance input makes the clamp free.** Its inputs sit at zero volts differential in
normal operation — that is what a virtual short is — so they can be clamped at **±0,9 V**, a diode's drop, without
touching the signal. A voltage-mode front end has no such margin.

```
loop ──[ 2 Ω ]──┬── IN−
                ⎓  BAV199 — its two diodes anti-parallel across the pair, ±0,9 V
loop ──[ 2 Ω ]──┴── IN+
```

**The clamp is one `BAV199`, Tesla's part** — pins 1 and 2 tied make one node and pin 3 the
other, so the series pair inside becomes an anti-parallel pair across the amplifier's inputs. The
sheet is what picks it: **`I_FSM` 4,5 A at 1 µs**, leakage 5 nA at 25 °C and 80 nA at 150 °C
(its shot noise is fA/√Hz against the `LMH5401`'s 3,5 pA), 2 pF, 0,9 V at 1 mA. **The pulse is
the loop's own**: 21 V of EMF over 2,9 µH rises at (21 − 1,5) / 2,9 = 6,7 A/µs, so the peak at
the end of the 1 µs front is 5–7 A and it decays in L/R ≈ 0,6 µs — a microsecond-class pulse
of tens of µJ, which is the rating the part carries. With the clamp at 1,2 V about `VOCM` 1,65 V
the pins sit at 1,05 / 2,25 V, inside the `LMH5401`'s (VS−) − 0,7 … (VS+) + 0,7 V, and the
current runs through the diodes, not the 10 mA the pins allow. **A `BAS416` is the same diode in
two packages, and the `PESD5V0X1BL` class is an ESD part at 1,3 A (8/20 µs) — weaker than the
`BAV199` in this pulse, and 9 kV ESD is nothing a closed ring in PPR meets.** One part, one tier.

**The 2 Ω do not limit the current** — the clamp does. Their job is to decouple the clamp's
capacitance from the summing node and give the amplifier's own input structure something to work
into; **pulse-rated 0805 thick film**, since 7 A across 2 Ω is 100 W for a microsecond. Both sit in series with the loop — 4 Ω, a corner at 220 kHz under the band — and cost
**0,77 dB at 0,5 MHz** and 0,05 dB at 2 MHz, inside the one calibrated curve.

**An air loop does not self-limit.** A ferrite rod **saturates** and acts
as a soft limiter that partly protects the front end by itself (`../tesla/WHY.md`, *Records and protocol*). An air loop
has no core and does not self-limit, so **this clamp is more necessary here than on Tesla, not
less.**

## 3. Direct sampling

**Direct sampling, because the passive ionogram is the goal** — a quadrature-sampling front end
sees one 512 kHz window at a time and a chirp crosses it in ~5 s, a slice and not a sweep. Direct
sampling sees the whole band at once and returns the entire ionogram.

### What it costs — the band-defining filter

**The band-defining filter is not optional**, because the whole band reaches the converter together
and out-of-band energy has nowhere to be rejected downstream. **It is S2** — there is no separate
box and no *preselector* in the switched sense, because with one band there is nothing to select
between. One fixed band-pass, and it does three jobs at once:

| | |
|---|---|
| **defines the band** | 0,5–16 MHz |
| **is the anti-alias filter** | aliases onto 16 MHz arrive from **51,1 MHz**, 1,68 octaves above the band |
| **protects the converter's range** | what lies between 16 MHz and Nyquist at 33,55 MHz does not alias, but it does **land** on the converter and eat its range |

**Nothing ahead of the filter helps.** The loop is flat at 51 MHz, because the virtual short holds
to 58,9 MHz and therefore past the loop's own 34,8 MHz resonance. **The filter does all of it.**

### The band-defining filter is an RC chain between S1 and S2

**All-RC, no inductor anywhere** — the only wound part in the receive chain is the loop itself,
and no SMD inductor sits next to a magnetic antenna with its own self-resonance waiting inside
the band (the shunt-inductor high-pass died on exactly that). The chain, per leg: **three
sections between S1 and S2 — `R1` 39 Ω → `C1` 120 pF across the pair → `R2` 51 Ω → `C2` 82 pF →
`R3` 56 Ω → `C3` 110 pF — into `Cin` 3,3 nF and `Rg` 51 Ω**; `Cf2` **47 pF** across `Rf2` 200 Ω
closes the chain at 16,9 MHz. Effective low-pass poles **16,9 · 19,1 · 19,9 · 21,2 · 21,2 MHz**
(the last is the pin network's — it counts), each section different values. `Cin` is the one
high-pass (pole 249 kHz — also the dc block the stage wants anyway); **S2 is the clean driver of
the converter and nothing sits on its output but the coupling, the `VCM` bias pair and the pin
network.** The series sum `R1`+`R2`+`R3`+`Rg` = **197 Ω** against `Rf2` = 200 Ω is the gain:
**×1,015 — the ladder's resistors are part of the gain resistance, so the chain has no insertion
loss to make back and no stage amplifies anything it did not before.**

What the soft RC knee holds at the folds is honest and bounded: **the passband droop and the
knee are part of the one measured curve the calibration corrects**, the HF spectrum is sparse,
and the survivors of the analog rejection are coherent carriers the sweep's product test and the
notches remove. Computed at synthesis: **80 dB at 144 MHz** (2 m — the one real, strong,
unpredictable interferer; 76–84 dB over ±10 % parts), **40 dB at the 51,1 MHz first fold** —
the best effort the doctrine asked for; two sections would have given only ~66 dB at 2 m and
~32 dB at the fold, which is why there are three.

**The computed response** (0 dB = the 1–8 MHz plateau):

| | |
|---|---|
| 100 kHz | −7,3 dB |
| 250 kHz | −1,8 dB |
| **500 kHz — bottom of the band** | **+0,1 dB** |
| 1 MHz | +0,7 dB |
| 8 MHz | −2,8 dB |
| **16 MHz — top of the band** | **−10,3 dB** — the knee, calibrated |
| 33,55 MHz — Nyquist | −26,7 dB |
| **51,1 MHz — the first fold into the band** | **−40 dB** |
| 144 MHz — 2 m | **−80 dB** |

Group-delay ripple across 1–16 MHz is 56 ns — this unit times no edges, so it is only listed.
The knee starting inside the band is the price of real poles, paid knowingly: a first-order RC
gives −3 dB at its own corner, so five poles close enough to defend the folds droop the band top
by ~10 dB whatever their stagger, and the measured-curve calibration takes it back out.

**Behind S2, two 200 Ω resistors to `VCM`** — S2's load and the dc bias in one pair of parts,
the standard differential-amplifier-into-pipeline-ADC front end, and the datasheet asks for
exactly this: *"the analog inputs are not internally dc-biased; in ac-coupled applications the
user must provide a dc bias externally,"* with `VCM` = AVDD/2 recommended for optimum
performance:

```
      +---- 0,1 uF ---+--- 200 R ---+                +-- 24,9 R --+--> AIN+
  S2 -|               |             +---- VCM -------|            = 150 pF
      +---- 0,1 uF ---+--- 200 R ---+                +-- 24,9 R --+--> AIN-
```

**The two 200 Ω resistors are S2's load — 400 Ω differential — and they set the dc level
behind the coupling capacitors at the same time.** One pair of parts does both jobs, and neither
is a special measure invented here. *(The RC ladder itself terminates upstream, into S2's `Rg` —
this network is behind the driver.)* **`VCM` is decoupled 0,1 µF to ground**, per the datasheet:
with the signal balanced the two arms' currents cancel at that node, so the capacitor carries
only the imbalance residue and the converter's own switching kick-back. The 200 Ω must stay
stiff — the unbuffered switched-capacitor input draws a real average current through the bias
path, and a high-value bias (kΩ-class) would let that current drag the pins off `VCM`.

**The overdrive case — checked against the absolute maximums.** The pin window is
**−0,3 V to AVDD + 0,2 V = 2,0 V**. In range, the pins swing 0,4–1,4 V; but S2 on 3,3 V can
swing ~±1,45 V a leg at full clip, which through the coupling lands the pin at −0,55…+2,35 V —
**~0,3 V past the window at both ends**. What limits it is the 24,9 Ω series arm: ~12 mA class
into the converter's internal clamps, transient, and only when the range is already exhausted.
**A `BAV199` pair per pin to AVDD/AGND is fitted** — its ~1,5 pF vanishes against the 150 pF
reservoir, the part is already on Tesla's BOM, and a few cents is the price of never running the
converter past its absolute maximum.

**The `Rs` + shunt network stays as drawn, and the datasheet sanctions it.** Its own figure uses
12 pF a leg, and it says in as many words that *"the RC component values should be chosen based on
the application's input frequency"*. **Ours is deliberately larger because it is a charge
reservoir**: the equivalent input is `C_SAMPLE` 5 pF behind `R_ON` 15 Ω, so the sample-and-hold
settles against the reservoir and the series arm only recharges it across the whole sample period,
which is what removes the settling constraint on the arm.

**And the coupling capacitors free `VOCM`.** Dc-coupled, `VOCM` would have to be the
converter's own 0,9 V and the swing would be tight. Behind a coupling
capacitor the dc level is set by the 200 Ω pair, not by the amplifier:

| | `VOCM` | headroom down against the 0,50 V the 2 V<sub>PP</sub> range needs |
|---|---|---|
| dc-coupled | 0,9 V | 0,68 V — **36 %** |
| **ac-coupled** | **1,65 V, mid-rail** | 1,43 V — **186 %** |

0,1 µF into 200 Ω a leg corners at **8 kHz**, near two decades below the band. It costs nothing.

**No FPGA and no CPLD.** The capture is a parallel bus into DMA and the processing is the MCU's.

## 4. The front end — `LMH5401` on S1, `THS4541` on S2

**The topology is Tesla's, the parts are not.** Differential transimpedance into a virtual short,
the loop bridged across the FDA inputs — that transfers; the `THS4551` does not, and on the first
stage the `THS4541` does not either.

### The number that decides it

A transimpedance stage's loop gain on an inductive source is

```
T = A(f) · β = (GBW/f) · ωL/(ωL + Rf) ≈ 2π · GBW · L / Rf
```

**The `f` cancels.** As frequency rises the coil gains impedance at exactly the rate the amplifier
loses gain, so `T` is one number per (`L`, `Rf`) pair and holds across the whole band. It also sets
a **ceiling on the transimpedance that `Rf` cannot beat**:

```
Z_t = K / (1 + K/Rf)        K = 2π · GBW · L        Z_t → K  as  Rf → ∞
```

| | `L` | `GBW` | `K` = the ceiling | `T` at `Rf` = 4,7 kΩ, the sensitive value |
|---|---|---|---|---|
| Tesla, ferrite rod | 2,5 mH | 135 MHz | 2,12 MΩ — never reached; at Tesla's `Rf1` 4,42 kΩ, `T` = 480 | — |
| Marconi, `THS4551` | 2,9 µH | 135 MHz | 2,46 kΩ | 0,52 |
| Marconi, `THS4541` | 2,9 µH | 850 MHz | 15,5 kΩ | 3,3 |
| **Marconi, `LMH5401`** | 2,9 µH | **8 GHz** | **148 kΩ** | **31,4** |

**862 times less inductance is the whole story.** On the `THS4551` the loop gain at a useful `Rf`
falls below 2, the transimpedance stops following `Rf` and starts following `GBW` — which carries a
±30 % part tolerance — and there is no loop gain left to correct anything.

**And a voltage-mode stage does not escape it.** Removing the coil from the feedback replaces `β`
with `1/G`, but `A(f)` stays: **`THS4551` has an open-loop gain of 8,4 at 16 MHz** against **264**
at Tesla's 512 kHz band top — both read off the same 135 MHz GBW. The band is **31× higher, so
there is 31× less gain to spend**, whatever is built around it. Voltage mode would also add a **+30,1 dB tilt** across the band, because the loop's
EMF rises with ω — a transimpedance stage is flat and needs no equaliser.

### Intermodulation in S1 — the fix is `Rf1` and the exchange rate is 17 dB for 1

**One number stated the whole problem, and the amplifier has already taken most of it back:**

```
1 + T  =   4,3   →   12,7 dB of distortion correction    Marconi on THS4541 at 4,7 kΩ
1 + T  =  32,4   →   30,2 dB                             Marconi on LMH5401 at 4,7 kΩ
1 + T  =  64     →   36,1 dB                             Marconi on LMH5401 at the fitted 2,35 kΩ
1 + T  = 481     →   53,6 dB                             Tesla at its 4,42 kΩ on 2,5 mH
```

**The cause is the inductance**: 2,9 µH against Tesla's 2,5 mH is 862× less, and
`T = 2π·GBW·L/Rf`. Nothing removes that, but gain-bandwidth is the one term that answers it
directly, and `LMH5401` buys **17,5 dB** of it for no signal at all, and the fitted `Rf1` another 6.
**What is left is a 17,5 dB gap to Tesla**, whose rod carries 862× the inductance, and `Rf1` below
is the one lever that moves it when a site needs it.

**The offender is medium wave.** 0,5–1,6 MHz is inside the band and it is the strongest thing this
loop will ever see. Its second and third harmonics land at **1,0–4,8 MHz** — on NAVTEX 4209,5, on
the Buzzer 4625, on RWM 4996. **A harmonic of a broadcast carrier is indistinguishable from a target
carrier**, and the sweep would promote it as a new beacon. **No filter reaches it**, because it is
made inside the band out of something else inside the band.

**And there is no lever on the antenna side at all.** Every route was walked and each ends the same
way:

| tried | why it fails |
|---|---|
| move the band bottom above MW | S1 still sees MW — the filter is behind it. And the dangerous harmonics land *above* 1,6 MHz anyway |
| a high-pass ahead of S1 | a series capacitor and the loop's `L` make a series resonance whose corner follows the coil — the antenna-dependent response this design gave up on purpose |
| a resistor across the loop | S1's input is `Rf/(1+A(f))`, which **rises** with frequency — 5,5 Ω at 1 MHz, 87 Ω at 16 MHz on the `THS4541` at 4,7 kΩ. A shunt resistor therefore steals more current at the top than at the bottom: it is a low-pass, the opposite of what is wanted |
| use the loop's own inductance | already spent. `ωL` rises at exactly the rate the EMF rises, which is why the current is flat — so the loop has no preference to exploit |
| more turns on the loop | `L ∝ N²` so `T ∝ N²`, and two stacked turns would give +10,4 dB. But the criterion `i_n < e_n/ωL` does not contain `Rf`, so raising `ωL` tightens it: at two turns the bar falls to 1,06 pA and the `LMH5401` fails it by 3,3×. **The amplifier and the turns are alternatives, not additions**, and the amplifier is worth more — 17,5 dB against 10,4, with the antenna untouched. Spread turns raise the self-resonance at the cost of `L`: at 200 mm pitch it is 18 MHz for +7,9 dB; a coplanar spiral is worse still, because the inner turn has less area. **The loop stays one turn** |

**The last row is worth stating on its own, because it also explains why the loop gain does not
help where it is needed.** `Z_in` rises with frequency at the same rate as `ωL`, so
`T = ωL/Z_in = 2π·GBW·L/Rf` and the `f` cancels: **the same 30,2 dB of correction at 1 MHz as at
16 MHz, and none extra where the strong signals are.**

> The property that makes the response flat makes the distortion correction flat too. One does not
> come without the other.

**`Rf1` is the one lever, and it is fixed at build — the board is damped to a value one part
serves on every site, and nothing is trimmed in the field.** What was missing is the rate. The
rate is the same whichever part is fitted, because both terms scale together.

| `Rf1` | intermodulation | signal-to-noise |
|---|---|---|
| **−6 dB** — 4,7 → 2,35 kΩ | **17 dB better** | 1,0 dB worse |
| −10 dB — 4,7 → 1,5 kΩ | 28 dB better | 2,5 dB worse |

**It pays twice, which is why the rate is so good.** The output swing falls, and closed-loop
intermodulation goes with its square — 12 dB for the first 6. And `T = K/Rf` rises as `Rf` falls,
which corrects another 5.

**One value, one channel.** `Rf1` is fixed at build and there is nothing to match it against; if a
second input ever returns with a crossed pair, it is **one trim for both, not two
trims** — separately trimmed inputs stop being a pair.

**And the second half is firmware:** the sweep rejects candidates on `2f`, `3f`, `2f₁−f₂` and
`2f₂−f₁` of the strong list whose level follows their parents' (`BAND.md`). What gets past the resistor is caught in software and never
becomes a beacon.

### The differential convention — read this before picking a resistor

**`Rf1` is the DIFFERENTIAL value. Each physical resistor is half of it, and there are two.**
An FDA transimpedance stage feeds back on both legs — `OUT+` → `IN−` and `OUT−` → `IN+` — with the
loop bridged across the inputs, so for a signal current `I_s`:

```
V(OUT+) = V_cm + I_s·R      V(OUT−) = V_cm − I_s·R      V_diff = 2·I_s·R
```

The transimpedance is `2R`. Everything here — `Z_t`, the loop gain `K/Rf1`, the noise gain
`1 + Rf1/ωL` — is written with the differential value so the equations stay single-ended in form,
and **the factor of two lands in the BOM**: the fitted `Rf1` = 2,35 kΩ differential is **1,18 kΩ ×2** on the
board. Any `Cf` would be the reverse — twice the stated value, twice — because the pole `R·C` per
leg is what has to stay put. **`Rg`, `Rf2` and `Rs` are already per leg** and do not carry it.
Tesla states the same convention; **Helion does not need it**, its `LTC6268` is single-ended
against ground.

### The parts and the values — `LMH5401`, then `THS4541`

**`LMH5401`** — 8 GHz GBP, 1,25 nV/√Hz, 3,5 pA/√Hz, external feedback resistors, its own 3,3 V
electrical table, adjustable `VOCM`, HD3 −96 dBc at 100 MHz.

**It is not a better part on the same curve — it is the same curve at a different place.** `e_n` and
`i_n` trade against each other in a bipolar input stage and their product is nearly a constant:

| | `e_n` | `i_n` | product | `e_n`/`i_n` |
|---|---|---|---|---|
| `THS4541` | 2,2 nV | 1,9 pA | **4,2** | 1158 Ω |
| **`LMH5401`** | **1,25 nV** | **3,5 pA** | **4,4** | **357 Ω** |
| `LTC6409` | 1,1 nV | 8,8 pA | 9,7 | 125 Ω |

`LMH5401` and `THS4541` sit on the same hyperbola; the move is along it, toward the end this board
needs. **`LTC6409` sits on a worse one and was rejected on it** — 1,1 nV looks better until `i_n·Rf`
= 41 nV swamps the top of the band, where it comes out 1,4 dB *behind* the `THS4541`. `LMH3401` was
rejected on topology: it is a fixed-gain part with internal feedback resistors, so its
transimpedance would be a few hundred ohms against the 4,7 kΩ this chain needs, and the converter
would become the limit.

**What the change buys, with nothing else touched** — same `Rf1`, same loop, same filter:

| | |
|---|---|
| distortion and intermodulation | **17,5 dB better** |
| noise at 1 MHz | 4,9 dB better |
| noise at 5 MHz | 4,6 dB better |
| noise at 16 MHz | 2,9 dB better |
| `Z_t` against `Rf1` | **3,1 % below, instead of 23 %** |
| signal | unchanged |

**The last row is a separate win.** At `T` = 3,3 the transimpedance was still partly set by `GBW`,
which carries a ±30 % part tolerance — the same objection this section raises against the
`THS4551`, only smaller. At `T` = 31 the gain is a resistor value again.

**The `LMH5401` is decompensated: stable for a noise gain above 2 V/V** (SBOS710D, *Parameter
Measurement Information*). This board's is 130 at 1 MHz and 9,1 at 16 MHz at the fitted 2,35 kΩ, and past the loop's
35 MHz self-resonance the source turns capacitive and the noise gain rises again, so the loop
crosses unity near 330 MHz at a noise gain of ~25 — twelve times the floor. **On +3,3 V it is inside
its sheet**: supply 3,15–5,25 V; the inputs, which sit at `VOCM` 1,65 V with the loop bridged across
them, against an input common-mode ceiling of `VS+` − 1,41 V = 1,89 V worst case, 0,24 V of room;
quiescent current 54 mA typical, 62 maximum — 178–205 mW.

**S2 does not change and does not need to.** It is a ×1 voltage stage, so its noise gain is 2 rather
than 130, and its loop gain is `A(f)/2`: **52,6 dB of correction at 1 MHz and 28,8 dB at 16 MHz**,
against S1's 36 dB. It sees the same swing and, across medium wave, where the strong signals
are, corrects it 12–22 dB better, and its noise sits behind S1's gain. **It was never the limit.**

**`THS4541`** — 850 MHz GBP, 2,2 nV/√Hz, 1,9 pA/√Hz, 10,1 mA, NRI/RRO, output common-mode control,
2,7–5,4 V. Same design language as the `THS4551`, six times the gain-bandwidth.

| | value | |
|---|---|---|
| supply | **+3,3 V**, `VOCM` = **1,65 V, mid-rail** | the path to the converter is ac-coupled, so the dc level at `AIN±` is set by the 200 Ω pair and not by the amplifier. That leaves `VOCM` free to sit mid-rail, where the headroom against the 0,50 V the 2 V<sub>PP</sub> range needs is 1,43 V instead of the 0,68 V a dc-coupled 0,9 V would give |
| **`Rf1`** | **2,35 kΩ** differential → **1,18 kΩ ×2** | the −6 dB default: 6 dB of headroom and 17 dB of intermodulation for 1,0 dB of SNR, fixed at build — there is no field trim (the clamp pair across `Rf2` is the only provision). On the `LMH5401` the loop gain stays high enough that the gain is the resistor, not the process |
| **`Cf1`** | **not fitted** | minimum stable is **0,63 pF** against the loop's ~10 pF — below any part worth placing, even before the convention's factor of two |
| **S2** | **gain ×1,015** | `Rf2` **200 Ω** against the chain's series sum of 197 Ω (`R1`+`R2`+`R3`+`Rg` — the ladder resistors are part of the gain resistance); `Cf2` **47 pF** across `Rf2` closes the chain at 16,9 MHz. There is no voltage gain anywhere on this board; the RC chain it terminates is above, in *Direct sampling* |
| `Rs` at the converter | **24,9 Ω** ×2 + **150 pF** differential | 21,2 MHz, above the band and below Nyquist. The `AD9265` input is an unbuffered switched-capacitor S/H, so this is not optional. **Re-anchored against the datasheet ✅**: the driver figure uses a 33 Ω / 10 pF-class network and says the values are chosen per application; ours is deliberately the larger charge reservoir — the sample-and-hold settles against the 150 pF and the series arm only recharges it across the whole sample period |
| termination at the converter | **200 Ω ×2 to `VCM`**, behind **0,1 µF ×2** | 400 Ω differential — S2's load and the dc bias in one pair of parts, straight from the datasheet's differential-amplifier front end (the RC ladder terminates upstream, into `Rg`) |
| power | **~0,8–1,2 W board, ~0,95–1,4 W from 12 V** | `LMH5401` ~180 mW + `THS4541` 34 mW + `AD9265-80` ~250 mW (its sheet at 80 MSPS: `AVDD` 126 mA typical, 131 maximum, and `DRVDD` 14 mA in CMOS, 254 mW with a sine input — ~110 mA on `AVDD` at the board's 67 MSPS) + MCU ~200 mW + PSRAM ≤ 36 mW + the LDOs' drops and the communication board, 60 mW on copper to ~400 mW on glass, through the buck at ~87 %. Inside the measuring unit's 3 W |

### What the chain delivers

| at the converter's input | | |
|---|---|---|
| atmospheric floor | 468 nV/√Hz | **+30,7 dB** over the converter |
| the amplifiers | 86 nV/√Hz | **+16,0 dB** |
| `AD9265-80` | ~13,7 nV/√Hz | 79,0 dBFS SNR on 707 mV<sub>RMS</sub> |
| **total in band** | 1,65 mV<sub>RMS</sub> | **52,6 dB** of headroom to full scale, per band path |

**Sky above the electronics, electronics above the converter** — the same ordering Tesla holds, and
the reason the design closes. The 1,5 µV/m atmospheric figure is **a 5 MHz value**; the floor rises
steeply going down, so the bottom of the band closes with more margin, not less.

**`Rf1` sets the gain, fixed at build**, exactly as on Tesla: 52,6 dB of headroom against a band that can hold a
megawatt broadcaster is the margin a real site moves, and the exchange rate for moving it is above —
17 dB of intermodulation for 1 dB of signal-to-noise.

### One layout rule this creates

**There is no feedback capacitor, so the feedback stray *is* the compensation.** At 4,7 kΩ
differential — the fitted 2,35 kΩ doubles every pole below:

| stray across `Rf1` | pole |
|---|---|
| 0,5 pF | 67,7 MHz |
| 1,0 pF | 33,9 MHz |
| **2,0 pF** | **16,9 MHz** |
| 3,0 pF | **11,3 MHz — inside the band** |

**Keep it under 1 pF**: short feedback traces, no ground plane under them, no via stitching beside
them. This is the one place on the board where a routing habit changes a measurement.

## 5. The clock chain — every rung a power of two

```
HCLK      = 2²⁸ = 268,435456 MHz      the core
ADC ENC   = HCLK / 4 = 2²⁶ = 67,108864 MHz    the encode, from a timer output
PSSI_PDCK = ADC DCO  = 2²⁶            the read clock, the encode retimed by the converter
```

Nothing divides by anything but a power of two, all the way down to the network's 2²³. **The converter does
not get an oscillator of its own** — it takes the encode the MCU hands it, and that comes off the
core. An integer PLL from the 2²² line clock reaches 2²⁸ directly, so **no
fractional divider exists anywhere on the board**, and neither do its spurs. **Verified against
DS13195 Table 55**: PLL input 4,194304 MHz sits in the 2–16 MHz window, VCO ×128 = 536,9 MHz in
the 128–560 MHz range, and the cycle-to-cycle jitter at that VCO is **±15–20 ps** — the number
the clock buffer's paragraph and the cleaner-PLL contingency are priced against.

Nyquist is **33,55 MHz** against a 16 MHz band edge.

**`ENC` needs no multiplier and nothing runs faster than the bus.** A pipeline converter clocks at
its *sample rate* — every stage of the pipeline runs off that one edge — so `ENC` is 67,108864 MHz
and the "80 Msps" in the part number is that pin's ceiling, not an internal rate. The MCU already
produces the clock; there is nothing to build.

**One gate re-squares the edge at the converter — a single-gate buffer on the 1,8 V rail,
placed tight against the `CLK` pin** — the **`74AUC1G34`**: the AUC family
is characterised for 1,8 V and is the steeper edge there; **no Schmitt input** — hysteresis converts threshold noise into time. It fixes the one jitter component a part can fix: amplitude
noise picked up along the trace divides by the edge's slew rate at the receiver's threshold, so
a slow edge arriving from across the board costs picoseconds a sharp local edge does not. The
PLL's own jitter it does not touch — no divider or gate can, an edge's position passes through
logic unchanged — and the recorded escalation for that, if a site's strongest carrier ever
demands it, is a narrow-loop cleaner PLL between MCO and `ENC`. The gate and the converter sit
close together, the gate with its own decoupling on the clean 1,8 V.

**And running it slower would make it worse, not better.** The sample-and-hold's noise is `kT/C`,
which is fixed by the capacitor and **independent of both the switch resistance and the acquisition
time** — the switch's `√(4kTR)` and the `1/(4RC)` bandwidth cancel exactly. Holding the switch
closed longer improves *settling*, which is accuracy and distortion, not noise. Halving `f_S`
therefore folds the same noise power into half the Nyquist width and **doubles the density in a
fixed bin**:

| `f_S` | gain into a 128 Hz bin |
|---|---|
| 134 MSPS | 57,2 dB |
| **67,109 MSPS** | **54,2 dB** |
| 33,554 MSPS | 51,2 dB |

Faster is better, and the ceiling is the port: **DS13195 Table 111, PSSI receive — `PSSI_PDCK`
100 MHz, and `PSSI_PDCK`/`f_HCLK` ≤ 0,4, which at a 2²⁸ kernel allows 107 MHz.** **The lower of
the two binds, so the ceiling is the pin and not the kernel**: 67,1 MHz passes with a third of
margin and 2²⁷ = 134 MHz is out against the absolute 100, not merely against the ratio — **a faster
core does not move it**. The same table gives the pin its
numbers: **data setup 2 ns / hold 1 ns at the PSSI pin** against the 14,9 ns period.
*(Table 110 is PSSI **transmit** and caps `PSSI_PDCK` at 50 MHz. It does not apply: this port
receives.)*
**Buying a faster speed grade and clocking it here changes nothing** — the grades differ in
maximum clock, not in `kT/C`.

**Nor is a more expensive converter worth buying.** The front end sits **16,0 dB above** this one,
so the converter costs the system **0,11 dB**. A perfect converter would save exactly that.

**If more per-bin sensitivity is ever wanted, it is bought with record length, not silicon:**

| samples | length | bin | gain | buffer |
|---|---|---|---|---|
| **524 288** | 7,81 ms | 128 Hz | **54,2 dB** | 1,0 MB |
| 1 048 576 | 15,62 ms | 64 Hz | 57,2 dB (+0,5 bit) | 2,0 MB |
| 2 097 152 | 31,25 ms | 32 Hz | **60,2 dB (+1 bit)** | 4,0 MB |

Coherent integration allows it — ionospheric Doppler of 1 Hz moves the phase by 0,016 cycles in
15,6 ms, and even 100 ms is inside a tenth of a cycle. What it costs is memory: the record does
not fit the 1 MB of AXI SRAM, which is what the external RAM below is for — and one OCTOSPI does
not carry it: the pad caps the bus at 2²⁶, 134 MB/s, which is the converter's stream to the byte
with nothing over for commands and page turns, and a half-band ÷2 ahead of the memory does not fit
the core in real time (`WHY.md`, *The long record*). **So the long record is an OPTION, and the
option is a second PSRAM**: the same part on OCTOSPI2 (§10), the raw stream split between the two
buses at half load each, **2 097 152 samples at fs 2²⁶ — the third row, 32 Hz bins, 60,2 dB** —
and the transform a blocked four-step FFT out of the two memories (`FIRMWARE.md` §6, *The long
record*). A board without the second memory runs the burst and refuses `LONG`.

**And note what it does not do:** a longer record lowers the sky, the front end and the converter
by the same amount, so it does not reorder them. It lowers the absolute floor per bin, which means
weaker carriers become readable. That is the real gain and it is not a fix for a noisy converter.

## 6. The converter's bus — the PSSI

**16 bits, one channel, one word a clock** — `PSSI_D0`–`D15` at 2²⁶ = 67,108864 MHz, **134 MB/s**
into AXI SRAM through DMA, against the 64-bit AXI's ~2,15 GB/s. The converter is 16-bit and the
port is 16-bit, so nothing is pulled down and nothing is wasted.

**The FMC is not used.** It reads only in bursts, and a burst boundary costs samples on a converter that does not wait.

**The PSSI is a slave and `PSSI_PDCK` is an input**, so the read clock arrives from the converter's
`DCO` — which is the encode the MCU sent, retimed. The read therefore closes on the part's own
data-to-clock skew rather than on a round-trip trace budget; route `DCO` with the data lines.

**Only the data lines are routed** — `D0`–`D15` and `PDCK`. `DE` and `RDY` are not used: the stream
never stops and there is nothing to flow-control.

**DCMI and PSSI are one block** — same pads, one AHB3 master port, and their two `ENABLE` bits must
not both be set. Only one runs. This board carries no camera.

**The read is gapless by construction.** The PSSI has no address phase — it latches a word on
every `PDCK` edge and the DMA drains its FIFO — so a gap can only be a DMA underrun, and that is
counted on every burst (word count and completion time, `FIRMWARE.md` §9) and proven once at the
bench with the converter's ramp pattern through all 524 288 words.

## 7. Parts and rails

| | | |
|---|---|---|
| **converter** | **AD9265-80** | **single-channel 16-bit**, 80 Msps, entirely 1,8 V (`DRVDD` 1,8–3,3), **79,0 dBFS SNR / 93 dBc SFDR**, SPI configuration, **48-lead LFCSP 7×7 mm**, 1,8 V CMOS or LVDS output, internal clock divider 1–8. **Input 2 V<sub>PP</sub> differential about `VCM` from its own pin; the S/H is switched-capacitor with no buffer, so the RC at the pins is not optional.** **80 Msps is the stopping point** — 2²⁶ is the last rung the clock tree reaches |
| **MCU** | **STM32H7A3IIT6** | Cortex-M7 at 280 MHz, **1414 CoreMark / 599 DMIPS**, **1380 KB SRAM** — 1024 KB of it AXI in three contiguous blocks from `0x2400 0000`, plus 128 KB DTCM and 64 KB ITCM. **6 SPI**, PSSI, two OCTOSPI |
| **front end** | **`LMH5401` on S1, `THS4541` on S2** | **Tesla's topology, not Tesla's part.** Differential transimpedance into a virtual short, then a band-defining stage — S1 on an 8 GHz GBP part, because a 2,9 µH loop caps the transimpedance at `2π·GBW·L`: 148 kΩ on the `LMH5401` against 2,46 kΩ on the `THS4551`, so the loop gain is 63 at the fitted `Rf1` and the gain is the resistor. `LMH5401` 1,25 nV/√Hz, 3,5 pA/√Hz; `THS4541` on S2, 850 MHz, 2,2 nV/√Hz, 1,9 pA/√Hz, 10,1 mA. Both on **+3,3 V** with `VOCM` = **1,65 V mid-rail** — the path to the converter is ac-coupled and the dc level at the pins is set by the 200 Ω pair to `VCM` (see *The front end* above) |
| **slow-data RAM** | **APS25608N-OBR-BD — 32 MB octal PSRAM, DDR, on OCTOSPI at 2²⁶**; a second one on OCTOSPI2 is the `LONG` option (§10) | the capture burst is 524 288 × 2 B = exactly the 1 MB of AXI SRAM, so logs, ionogram parameters and, where the option is built, the long records go outside and the whole megabyte stays with the capture. 1,8 V part, mini-BGA 24 at 1,0 mm, self-refresh, Halfsleep 40 µA with retention. **The clock is settled by the pad**: DS13195 Table 91 caps octal DDR at **120 MHz** (15 pF, VOS0, DHQC), so 2²⁷ = 134,2 MHz is out and the bus runs **2²⁶ — 134 MB/s peak**. What that changes is the full-rate path: a one-channel full-rate stream (134 MB/s) would sit exactly at one bus's peak, so **the long record is the `LONG` option, split over two memories at half load each** (§5) — 2 097 152 samples at fs 2²⁶, 32 Hz bins (Doppler at 1 Hz moves 0,03 cycles across the 31 ms record — still coherent). **Tesla carries the same position** |
| **station interface** | **3,3 V through two direction-set `SN74AXC8T245` and a `PCA9306` — Tesla's, one for one** | the Galvani body is 3,3 V logic on every board in the station; a board whose processor runs 1,8 V crosses on translators and the connector does not change. Which lines cross and which need nothing is in §10, *The station interface* |
| **rails** | 3,3 V analog + 1,8 V digital — **no 5 V on this board**; the unit power board hands it 12 V and nothing lower | **two `TPS629206` make 4,0 V and 1,8 V straight from the 12 V**, both in forced PWM — the 4,0 V at 2,5 MHz, the 1,8 V at 1 MHz, where 12 → 1,8 V is 150 ns of on-time against the part's 40 ns (`../galvani/HARDWARE.md`, *The buck cell*) — and never switched from a pin: off is `ENABLE` at the source power board, which takes the whole feed and every rail here with it; the FDAs and the converter's analog side run on a **clean 3,3 V LDO off the 4,0 V**, each amplifier's supply pin behind the bead `GZ2012D301TF` into 100 nF, the station interface and the communication board on a second, ordinary 3,3 V LDO off the same 4,0 V (0,7 V × 125 mA at most on an optical end, × 18 mA on copper); the **MCU and the PSRAM ride the 1,8 V**, and the converter's `DRVDD` takes that 1,8 V through the shielded 2,2 µH into 10 µF + 100 nF, then the bead `GZ2012D301TF` into 100 nF at the pin, no regulator of its own; the converter's analog `VDD` takes a quiet 1,8 V from a **TPS7A2018** off the clean 3,3 V — ~110 mA at 67 MSPS, 131 mA at the sheet's maximum: 44 % of the part and ~200 mW in it, the board's largest analogue load; its `AVDD` pins stand behind the bead `GZ2012D301TF` into 100 nF at each, 131 mA through 0,20 Ω being 26 mV inside `AVDD`'s 1,7–1,9 V. **Spread spectrum is what disqualifies the alternative here**: this board's band is 0,5–16 MHz, so no buck frequency is out of it and the comb's only defence is being discrete and known (`../galvani/HARDWARE.md`, *The buck cell*); every buck on the board is in forced PWM and never enters power save. The station interface stays 3,3 V through fixed-direction translators of the `SN74AXC` class, as on Tesla (`../tesla/HARDWARE.md` §6). Ranging: `RXD` and `TXD` on channels of one timer, `RXD` on CH1/CH2 (`../bifrost/HARDWARE.md`, *Ranging*) |

## 8. The burst — what the memory holds

**2 B per sample, one channel**, so the 1 MB AXI SRAM holds **524 288 samples = 7,8 ms** and the
transform resolves **128 Hz per bin**.

| job | burst | duty |
|---|---|---|
| carrier levels | one burst per revisit, all monitored carriers read from the one transform | ≈ 0,04 % |
| ionogram | 64 K samples every 100 ms — the chirp moves under 100 Hz in that window and is stationary | ≈ 1 % |

**The ionogram is scaled on the node and ships as parameters** (`BUS.md`), so the part's hardware
JPEG codec has no work here.

## 9. The converter's input, and the gain

**`AD9265-80`, from the datasheet:**

| | |
|---|---|
| input range | **1 to 2 V<sub>PP</sub> differential**, selectable → at 2 V<sub>PP</sub> each pin swings ±0,5 V |
| common mode | **`V_CM` = `VDD`/2 = 0,9 V**, from the part's own `VCM1`/`VCM2` pins; allowed 0,7–1,25 V |
| sample-and-hold | `C_SAMPLE` **5 pF**, `R_ON` **15 Ω**, and **no precharge buffer** |
| common-mode input current | 100 µA per pin at 80 Msps |
| CMRR · full-power bandwidth | 80 dB · 750 MHz |

**The FDA runs on +3,3 V with `VOCM` = 1,65 V, mid-rail, and the ac coupling is what allows it** —
the dc level at the converter pins is set by the 200 Ω pair to `VCM` = 0,9 V, not by the
amplifier. This is the exact opposite of Tesla, where the converter's 2,5 V common mode forces a
5 V rail; here nothing on the board needs 5 V at all.

**And unlike Tesla's converter, this one has no input precharge buffer** — the datasheet asks for
an RC filter right at the pins to isolate the drive from the sample-and-hold switching. What Tesla
could delete, this board must keep — `Rs` 24,9 Ω ×2 and 150 pF, above.

### The gain is fixed, one step down, and the only field provision is a clamp pair

**`Rf1` defaults one −6 dB step below the sensitive value — 2,35 kΩ differential = 1,18 kΩ ×2** —
buying 6 dB of headroom and 17 dB of intermodulation for 1,0 dB of SNR, so the realistic worst
site is covered from the factory and every board ships identical. Nothing switches: the boards
are assembled in one BOM and a soldered trim does not exist in the field. The one provision is
**two unpopulated push-in clamp positions across the `Rf2` legs of S2** — a pressed-in metal-film
resistor per leg lowers the driver gain without touching any filter corner, for the site that
turns out to sit under a live transmitter.

## 10. The processor — pins, timers, clock tree

*The record to draw the H7A3 from and to set CubeMX by. Package LQFP176, `STM32H7A3IIT6` — the LDO-supply part, no SMPS pins; pin
numbers and alternate functions from DS13195 Rev 8, the LQFP176 column. Every pin below is either
assigned or listed free — nothing is left to guess.*

### Peripherals

| peripheral | instance | pins | job | mode |
|---|---|---|---|---|
| the NodBus link | **USART1** + **TIM4** | PB6 `TXD` · PB7 `RXD` | the spur to its card, 8N1 at the rung `BUSCFG` hands down — 2²⁰ alone, 2²¹ chained, through the fixed-direction translators | the pins switch USART → TIM4 for the ranging instant |
| the frame anchor | **TIM2** (32-bit) | PB3 ← `RXD` on CH2 | the card's frame start on the grid | never reset, read as differences |
| echo receiver | **USART3**, RX only | PB11 `RXD_ECHO` | hears the board's own frames | |
| the converter's data | **PSSI**, 16-bit receive | `D0`–`D15` on PH9–PH12 · PH14 · PH15 · PI0–PI4 · PI6 · PI7 · PF11 · PH4 · PI11 | the `AD9265`'s one 16-bit channel, one word a clock, into AXI SRAM through DMA | 2²⁶ words/s, 134 MB/s |
| the converter's clock | `ENC` on **PA7 `TIM3_CH2`**; `PSSI_PDCK` on **PA6** from the converter's `DCO` | PA7 · PA6 | 2²⁶ into a `74AUC1G34` at the converter's `CLK` — no MCO, no oscillator of its own; `DCO` comes back as the read clock | HCLK ÷ 4 |
| the converter's registers | **SPI3**, half-duplex + `CSB` PD2 | PC10 `SCLK` · PC12 `SDIO` | the AD9265's three-wire port, written at boot | ≤ 10 MHz; `PDWN` PG2, `SYNC` PG3, `OR` PG4 on EXTI, `OEB` tied low, `DCO` to PA6 as the PSSI's read clock |
| PSRAM | **OCTOSPI1**, port 2 | PF0–PF5 · PG0 · PG1 · PG10–PG12 · PF12 | the APS25608N, 32 MB octal DDR — the trace and the ionogram parameters | 2²⁶, DQS on PF12 |
| the second PSRAM — **the `LONG` option** | **OCTOSPI2**, port 1 | PD11 · PD12 · PF7 · PD13 · PD4–PD7 · PF10 · PC5 · PG6 | a second APS25608N, **unpopulated unless the site takes `LONG`**; the raw 2²⁶ record split over the two buses | 2²⁶, DQS on PC5 |
| ID reads | **ADC1** | PC0 · PC1 | one resistor per body, against 10 kΩ 1 % to `VREF+`, the 1,8 V | read once at bring-up |
| power body | **I2C1** through the `PCA9306` | PB8 · PB9 | the unit power board's `INA238` | 100 kHz; `ALERT` on PC8, EXTI |
| the thermometer | **ADC1** | `NTC` PA0 · `NTC_EN` PF6 | one 0603 `NCP18XH103F03RB` on the board — the band's temperature covariate in the `REPORT` frame; corrects nothing (`FIRMWARE.md`). the station's divider: 10 kΩ from `NTC_EN` to the pin, the NTC from the pin to `VSSA`, **3 kΩ + 100 nF at the ADC pin**, read against `VREF+` so the rail drops out (`../tesla/HARDWARE.md` §7) | read every 10 s, `NTC_EN` high for the conversion only |
| clock in | **HSE bypass** | PH0 | `NB IN`'s `CLK`, 2²², through the inbound translator | PH1 unconnected |
| debug | **SWD** | PA13 · PA14 | | no JTAG, no trace |

No regulator is enabled from a pin: every rail runs whenever the 12 V is there, and the parts
that sleep sleep by their own pins — the converter's `PDWN`, the amplifiers' `PD`.

**The converter is a slave and the processor owns the grid.** The `AD9265` carries no oscillator:
`CLK+`/`CLK−` is an encode **input**, so PA7 `TIM3_CH2` goes through the `74AUC1G34` and **is** the
encode at 2²⁶. **The PSSI is a slave too and `PSSI_PDCK` is an input**, so the read clock comes back
from the converter's `DCO` — the same encode, retimed — and the read closes on the part's own
data-to-clock skew rather than on a round-trip trace budget. Route `DCO` with the data lines. The
pipeline latency is a constant offset of the sample index, handled in firmware as `LAT`.

**The numbers, DS13195 Table 111 (PSSI receive):** `PSSI_PDCK` 100 MHz and `PSSI_PDCK`/`f_HCLK`
≤ 0,4 — 107 MHz at a 2²⁸ kernel — against 67,108864 MHz here, a third of margin; data setup 2 ns and
hold 1 ns at the pin against a 14,9 ns period. Aperture jitter is 0,1 ps rms, parallel CMOS is the
part's only 1,8 V output, and the `-80` grade's floor is a 12,5 ns period, so 2²⁶ sits 19 % inside
it. The `DCO` phase and polarity are a converter register, written at boot (`FIRMWARE.md` §2).

### Pins

| pin | port | signal on the board | AF / function | dir | note |
|---|---|---|---|---|---|
| 1 | PE2 | `DE` | GPIO | out | through the outbound translator — `NB IN`'s driver enable; 100 kΩ to ground |
| 2 | PE3 | — | | | free |
| 3 | PE4 | `SD` | GPIO | in | through the inbound translator — the optical no-light |
| 4 | PE5 | — | | | free |
| 5 | PE6 | — | | | free |
| 6 | VBAT | — | | | tied to `VDD`; no RTC anywhere |
| 7 | PI8 | — | | | free |
| 8 | PC13 | — | | | free (tamper/RTC pin, unused) |
| 9 | PC14 | — | | | free — no 32 kHz crystal |
| 10 | PC15 | — | | | free |
| 11 | PI9 | — | | | free |
| 12 | PI10 | — | | | free |
| 13 | PI11 | `D15` | `PSSI_D15` | in | the top bit |
| 14 | VSS | | | | |
| 15 | VDD | 1,8 V | | | |
| 16 | PF0 | `PS_IO0` | `OCTOSPIM_P2_IO0` | i/o | the APS25608N, octal DDR — OCTOSPI1 on port 2 |
| 17 | PF1 | `PS_IO1` | `OCTOSPIM_P2_IO1` | i/o | |
| 18 | PF2 | `PS_IO2` | `OCTOSPIM_P2_IO2` | i/o | |
| 19 | PF3 | `PS_IO3` | `OCTOSPIM_P2_IO3` | i/o | |
| 20 | PF4 | `PS_CLK` | `OCTOSPIM_P2_CLK` | out | 2²⁶ at most — the pad table's 120 MHz octal-DDR ceiling |
| 21 | PF5 | `PS_NCLK` | `OCTOSPIM_P2_NCLK` | out | unused — single-ended clock; free if the drawing wants it |
| 22 | VSS | | | | |
| 23 | VDD | 1,8 V | | | |
| 24 | PF6 | `NTC_EN` | GPIO | out | the NTC divider's top, high for the conversion only |
| 25 | PF7 | `IO2_B` | `OCTOSPIM_P1_IO2` | i/o | the `LONG` option's second PSRAM — free on a board without it |
| 26 | PF8 | — | | | free |
| 27 | PF9 | — | | | free |
| 28 | PF10 | `CLK_B` | `OCTOSPIM_P1_CLK` | out | the `LONG` option's second PSRAM — free on a board without it |
| 29 | PH0 | `CLK_IN` | `OSC_IN`, bypass | in | `NB IN`'s `CLK`, 2²², through the inbound translator — `V_IH` ≥ 1,26 V at `VDD` 1,8 V |
| 30 | PH1 | — | | | unconnected in bypass mode |
| 31 | NRST | reset | | | 100 nF, no pull beyond the internal |
| 32 | PC0 | `ID_D` | `ADC12_INP10` | in | `NB IN`'s `ID` — 10 kΩ 1 % to `VREF+`, the 1,8 V, so the reading is a ratio |
| 33 | PC1 | `ID_P` | `ADC12_INP11` | in | `PWR IN`'s `ID`, the same way |
| 34 | PC2 | — | | | free (ADC) |
| 35 | PC3 | — | | | free (ADC) |
| 36 | VDD | 1,8 V | | | |
| 37 | VSSA | | | | |
| 38 | VREF+ | the `VDDA` node — the ADC reference, strapped to `VDDA` | | | |
| 39 | VDDA | 1,8 V through the shielded 2,2 µH `SWPA252012S2R2MT`, 10 µF + 100 nF at the pins, `VREF+` on the same node — the rail is a buck's own, so the filter is the buck-noise inductor of `../core/POWER.md`, not a ferrite | | | |
| 40 | PA0 | `NTC` | ADC | in | the board's NTC, on its divider, 3 kΩ + 100 nF at the pin |
| 41 | PA1 | — | | | free (ADC) |
| 42 | PA2 | — | | | free (ADC) |
| 43 | PH2 | `PGOOD` | GPIO | in | the 1,8 V `TPS629206`'s `PG` |
| 44 | PH3 | `LED` | GPIO | out | the status LED, fitted — 1 kΩ from the 3,3 V, one 50 ms blink a minute while healthy, two on a fault (`../core/HARDWARE.md`) |
| 45 | PH4 | `D14` | `PSSI_D14` | in |  |
| 46 | PH5 | — | | | free |
| 47 | PA3 | — | | | free (ADC) |
| 48 | VSS | | | | |
| 49 | VDD | 1,8 V | | | |
| 50 | PA4 | — | | | free (ADC) |
| 51 | PA5 | — | | | free (ADC) |
| 52 | PA6 | `PDCK` | `PSSI_PDCK` | in | **2²⁶** — the read clock, from the converter's `DCO`; route it with the data lines |
| 53 | PA7 | `ENC` | `TIM3_CH2` | out | **2²⁶** — the converter's encode, through a `74AUC1G34` tight against its `CLK`; the grid |
| 54 | PC4 | — | | | free (ADC) |
| 55 | PC5 | `DQS_B` | `OCTOSPIM_P1_DQS` | i/o | the `LONG` option's second PSRAM — free on a board without it |
| 56 | PB0 | — | | | free (ADC) |
| 57 | PB1 | — | | | free (ADC) |
| 58 | PB2 | — | | | free |
| 59 | PF11 | `D12` | `PSSI_D12` | in |  |
| 60 | PF12 | `PS_DQS` | `OCTOSPIM_P2_DQS` | i/o | |
| 61 | VSS | | | | |
| 62 | VDD | 1,8 V | | | |
| 63 | PF13 | — | | | free (ADC) |
| 64 | PF14 | — | | | free (ADC) |
| 65 | PF15 | — | | | free |
| 66 | PG0 | `PS_IO4` | `OCTOSPIM_P2_IO4` | i/o | |
| 67 | PG1 | `PS_IO5` | `OCTOSPIM_P2_IO5` | i/o | |
| 68 | PE7 | — | | | free |
| 69 | PE8 | — | | | free |
| 70 | PE9 | — | | | free |
| 71 | VSS | | | | |
| 72 | VDD | 1,8 V | | | |
| 73 | PE10 | — | | | free |
| 74 | PE11 | — | | | free |
| 75 | PE12 | — | | | free |
| 76 | PE13 | — | | | free |
| 77 | PE14 | — | | | free |
| 78 | PE15 | — | | | free |
| 79 | PB10 | — | | | free |
| 80 | PB11 | `RXD_ECHO` | `USART3_RX` | in | through the inbound translator — the echo check |
| 81 | VCAP | 2× 1 µF 50 V 0805 + 2× 100 nF 50 V 0603 — 2,2 µF, the sheet's 2,2 µF ±20 %, the internal LDO | | | |
| 82 | VDD | 1,8 V | | | |
| 83 | PH6 | — | | | free |
| 84 | PH7 | — | | | free |
| 85 | PH8 | — | | | free |
| 86 | PH9 | `D0` | `PSSI_D0` | in | the converter's data — one 16-bit channel, one word a clock |
| 87 | PH10 | `D1` | `PSSI_D1` | in |  |
| 88 | PH11 | `D2` | `PSSI_D2` | in |  |
| 89 | PH12 | `D3` | `PSSI_D3` | in |  |
| 90 | VSS | | | | |
| 91 | VDD | 1,8 V | | | |
| 92 | PB12 | — | | | free |
| 93 | PB13 | — | | | free |
| 94 | PB14 | — | | | free |
| 95 | PB15 | — | | | free |
| 96 | PD8 | — | | | free |
| 97 | PD9 | — | | | free |
| 98 | PD10 | — | | | free |
| 99 | PD11 | `IO0_B` | `OCTOSPIM_P1_IO0` | i/o | the `LONG` option's second PSRAM — free on a board without it |
| 100 | PD12 | `IO1_B` | `OCTOSPIM_P1_IO1` | i/o | the `LONG` option's second PSRAM — free on a board without it |
| 101 | PD13 | `IO3_B` | `OCTOSPIM_P1_IO3` | i/o | the `LONG` option's second PSRAM — free on a board without it |
| 102 | VSS | | | | |
| 103 | VDD | 1,8 V | | | |
| 104 | PD14 | — | | | free |
| 105 | PD15 | — | | | free |
| 106 | PG2 | `PDWN` | GPIO | out | the AD9265's `PDWN` — the converter asleep |
| 107 | PG3 | `SYNC` | GPIO | out | the AD9265's `SYNC` — the clock divider's phase, pulsed once at start |
| 108 | PG4 | `OR` | GPIO, EXTI | in | the converter's over-range — one encode clock long, so it is latched by EXTI during the burst, not polled |
| 109 | PG5 | — | | | free |
| 110 | PG6 | `NCS_B` | `OCTOSPIM_P1_NCS` | out | the `LONG` option's second PSRAM; 10 kΩ to the 1,8 V as the first's — free on a board without it |
| 111 | PG7 | `PD_AMP` | GPIO | out | the `LMH5401`'s and the `THS4541`'s `PD` — the amplifiers asleep (`FIRMWARE.md` §2) |
| 112 | PG8 | — | | | free |
| 113 | VSS | | | | |
| 114 | VDD33USB | tied to `VDD` | | | USB unused |
| 115 | PC6 | — | | | free |
| 116 | PC7 | — | | | free |
| 117 | PC8 | `ALERT` | GPIO, EXTI | in | through the inbound translator — the power body's `INA238`, high = alarm |
| 118 | PC9 | — | | | free |
| 119 | PA8 | — | | | free |
| 120 | PA9 | — | | | free |
| 121 | PA10 | — | | | free |
| 122 | PA11 | — | | | free (USB, unused) |
| 123 | PA12 | — | | | free (USB, unused) |
| 124 | PA13 | `SWDIO` | SWD | i/o | |
| 125 | VCAP | 2× 1 µF 50 V 0805 + 2× 100 nF 50 V 0603 — 2,2 µF, the sheet's 2,2 µF ±20 %, the internal LDO | | | |
| 126 | VSS | | | | |
| 127 | VDD | 1,8 V | | | |
| 128 | PH13 | — | | | free |
| 129 | PH14 | `D4` | `PSSI_D4` | in |  |
| 130 | PH15 | `D11` | `PSSI_D11` | in |  |
| 131 | PI0 | `D13` | `PSSI_D13` | in |  |
| 132 | PI1 | `D8` | `PSSI_D8` | in |  |
| 133 | PI2 | `D9` | `PSSI_D9` | in |  |
| 134 | PI3 | `D10` | `PSSI_D10` | in |  |
| 135 | VSS | | | | |
| 136 | VDD | 1,8 V | | | |
| 137 | PA14 | `SWCLK` | SWD | in | |
| 138 | PA15 | — | | | free |
| 139 | PC10 | `SCLK_C` | `SPI3_SCK` | out | the AD9265's `SCLK` — the configuration port |
| 140 | PC11 | — | | | free |
| 141 | PC12 | `SDIO_C` | `SPI3_MOSI`, half-duplex | i/o | the AD9265's `SDIO`, bidirectional |
| 142 | PD0 | — | | | free |
| 143 | PD1 | — | | | free |
| 144 | PD2 | `CSB_C` | GPIO | out | the AD9265's `CSB` |
| 145 | PD3 | — | | | free |
| 146 | PD4 | `IO4_B` | `OCTOSPIM_P1_IO4` | i/o | the `LONG` option's second PSRAM — free on a board without it |
| 147 | PD5 | `IO5_B` | `OCTOSPIM_P1_IO5` | i/o | the `LONG` option's second PSRAM — free on a board without it |
| 148 | VSS | | | | |
| 149 | VDDMMC | 1,8 V | | | |
| 150 | PD6 | `IO6_B` | `OCTOSPIM_P1_IO6` | i/o | the `LONG` option's second PSRAM — free on a board without it |
| 151 | PD7 | `IO7_B` | `OCTOSPIM_P1_IO7` | i/o | the `LONG` option's second PSRAM — free on a board without it |
| 152 | PG9 | — | | | free |
| 153 | PG10 | `PS_IO6` | `OCTOSPIM_P2_IO6` | i/o | |
| 154 | PG11 | `PS_IO7` | `OCTOSPIM_P2_IO7` | i/o | |
| 155 | PG12 | `PS_NCS` | `OCTOSPIM_P2_NCS` | out | |
| 156 | PG13 | — | | | free |
| 157 | PG14 | — | | | free |
| 158 | VSS | | | | |
| 159 | VDD | 1,8 V | | | |
| 160 | PG15 | — | | | free |
| 161 | PB3 | `RXD` | `TIM2_CH2` capture | in | the same net as PB7 — the timebase captures the start edge of the card's frame |
| 162 | PB4 | — | | | free |
| 163 | PB5 | — | | | free |
| 164 | PB6 | `TXD` | `USART1_TX` / `TIM4_CH1` | out | the NodBus link — through the outbound translator to `NB IN`; 100 kΩ to 1,8 V |
| 165 | PB7 | `RXD` | `USART1_RX` / `TIM4_CH2` | in | through the inbound translator; the pins switch USART → TIM4 for the ranging instant |
| 166 | BOOT0 | 10 kΩ to ground | | | |
| 167 | PB8 | `SCL` | `I2C1_SCL` | i/o | through the `PCA9306` to `PWR IN` — the `INA238`, alone on it; 4,7 kΩ to 1,8 V on this side |
| 168 | PB9 | `SDA` | `I2C1_SDA` | i/o | the same; 4,7 kΩ to 1,8 V |
| 169 | PE0 | — | | | free |
| 170 | PE1 | — | | | free |
| 171 | PDR_ON | tied to `VDD` | | | the power-down reset stays armed |
| 172 | VDD | 1,8 V | | | |
| 173 | PI4 | `D5` | `PSSI_D5` | in |  |
| 174 | PI5 | — | | | free |
| 175 | PI6 | `D6` | `PSSI_D6` | in |  |
| 176 | PI7 | `D7` | `PSSI_D7` | in |  |

**55 GPIO used, 84 free — the `LONG` option's eleven among them — `PH1` unconnected** (PA1–PA5 · PA8–PA12 · PA15 · PB0–PB2 · PB4 · PB5 · PB10 · PB12–PB15 · PC2–PC7 · PC9 · PC11 · PC13–PC15 · PD0 · PD1 · PD3–PD15 · PE0 · PE1 · PE3 · PE5–PE15 · PF7–PF10 · PF13–PF15 · PG5 · PG6 · PG8 · PG9 · PG13–PG15 · PH5–PH8 · PH13 · PI5 · PI8–PI10).

### Timers

| timer | bits | kernel | pins | job |
|---|---|---|---|---|
| **TIM4** | 16 | 2²⁸ | CH1 → PB6, CH2 ← PB7 | the ranging turnaround: one-pulse, triggered by the capture on CH2, the return raised on CH1 after `CCR` ticks; PWM-input mode on CH2 measures the incoming pulse's width. 244 µs of range at 2²⁸ — a port past 24 km runs it at ÷4 |
| **TIM2** | 32 | 2²⁸ | CH2 capture ← PB3 · the slot and burst compares, internal | the free-running timebase; the capture of the card's frame start places the round on the grid, a compare starts each burst |
| **TIM3** | 16 | 2²⁸ | CH2 → PA7 | the converter's `ENC`: PWM at ÷4, 2²⁶, 50 % duty, into the `74AUC1G34` |
| **TIM6** | 16 | | | the housekeeping second (`FIRMWARE.md` §12) |
| TIM1 · TIM5 · TIM8 · TIM12–TIM17 · LPTIM1–LPTIM5 | | | | free |
| TIM7 | 16 | | | free — the other basic timer |

### The station interface — `NB IN`, `PWR IN` and the 12 V terminals

**Connectors: `NB IN` · `PWR IN` · `LOOP` · `12V`** (`../galvani/README.md`, *Connector names*).

**Marconi is a measuring unit and stands outside the enclosure, always behind a unit power board,
so both bodies are unit-end sockets**: what sits on the Galvani boards runs from the moment the
feed arrives, held there by resistors, and no processor pin switches anything on them. **`NB IN`** is
the data body, **`PWR IN`** the power body, both `BX2.54-2xNA` shrouded headers, 2×6 and 2×4; **`12V`**
the 12 V terminals.

**Every one-way line crosses a direction-set translator; the I²C crosses a `PCA9306`.** The
processor is 1,8 V and the bodies are 3,3 V logic. Two `SN74AXC8T245` — 1,8 V on `VCCA` from the
processor's rail, 3,3 V on `VCCB` from the interface rail, `OE#` to ground, **`DIR` strapped and
no processor pin on it** — one package a direction, Tesla's pair. Both supplies are isolated
inside the part, so either rail rising first leaves the outputs high-impedance and nothing is
sequenced.

| part | `DIR` | lines | unused |
|---|---|---|---|
| **inbound** `SN74AXC8T245`, B → A | to ground | `NB IN`: `CLK`, `RXD`, `RXD_ECHO`, `SD` · `PWR IN`: `ALERT` — five | three; their B inputs to ground |
| **outbound** `SN74AXC8T245`, A → B | to the 1,8 V | `NB IN`: `TXD`, `DE` — two | six; their A inputs to ground |
| **`PCA9306`** | — | `PWR IN`: `SDA`, `SCL` — `VREF1` on the 1,8 V, `VREF2` and `EN` joined and 200 kΩ to the 3,3 V; **4,7 kΩ to 1,8 V on the processor side and 4,7 kΩ to 3,3 V on the socket side**, on each line | — |

**No translator input floats.** Every inbound line carries **100 kΩ to ground on the 3,3 V side**,
so an empty socket reads a defined low: `SD` quiet, `ALERT` quiet, `RXD` and `CLK` dead. The two
outbound lines carry 100 kΩ on the 1,8 V side for the processor's reset: **`DE` to ground**, the
driver off, **`TXD` to the 1,8 V**, a UART's idle. **An auto-direction translator is not used on a
clock line**: its one-shot accelerators and pass-gate pull-ups round the edges into series
resistance and cable capacitance.

**`NB IN` — the data body.**

| pin | signal | on this board |
|---|---|---|
| 1 | `CLK/PPS` | **in** — the 2²² wire clock, through the inbound translator onto PH0 `OSC_IN`, HSE bypass; 100 kΩ to ground on the 3,3 V side |
| 2 | `GND` | the ground plane |
| 3 | `TXD` | PB6 `USART1_TX` / `TIM4_CH1`, through the outbound translator; 100 kΩ to 1,8 V at the pin |
| 4 | `RXD` | through the inbound translator onto PB7 `USART1_RX` / `TIM4_CH2` and PB3 `TIM2_CH2`; 100 kΩ to ground on the 3,3 V side |
| 5 | `ID` | PC0 `ID_D`, `ADC12_INP10`; **10 kΩ 1 % to `VREF+`**, the 1,8 V — no translator, the reading is a ratio |
| 6 | `ID_RET` | **not connected** — a measuring unit never meets a crossed cable |
| 7 | `RXD_ECHO` | through the inbound translator onto PB11 `USART3_RX`; 100 kΩ to ground on the 3,3 V side |
| 8 | `DE` | PE2, through the outbound translator; 100 kΩ to ground at the pin |
| 9 | `B_DIR` | **tied to ground** — this end listens on channel B, the clock arrives; no processor pin |
| 10 | `LINE_EN` | **10 kΩ to 3,3 V** — the line side runs whenever the unit does; no processor pin |
| 11 | `SD` | through the inbound translator onto PE4; 100 kΩ to ground on the 3,3 V side — a copper board leaves it unpopulated and it reads quiet |
| 12 | `3,3 V` | the ordinary 3,3 V, the interface LDO off the 4,0 V (§7) |

**`PWR IN` — the power body.** One power socket on the board, so its `INA238` is 0x40, alone on I2C1.

| pin | signal | on this board |
|---|---|---|
| 1 | `ENABLE` | **not connected** — the unit power board pulls its own `ENABLE` to its input |
| 2 | `GND` | the ground plane |
| 3 | `ID` | PC1 `ID_P`, `ADC12_INP11`; **10 kΩ 1 % to `VREF+`** |
| 4 | `SDA` | PB9 `I2C1_SDA` through the `PCA9306`; 4,7 kΩ to 3,3 V on this side of it |
| 5 | `A_SEL` | **straight to ground** — 0x40 |
| 6 | `SCL` | PB8 `I2C1_SCL` through the `PCA9306`; 4,7 kΩ to 3,3 V on this side of it |
| 7 | `ALERT` | through the inbound translator onto PC8, GPIO on EXTI; **100 kΩ to ground on the 3,3 V side, high = alarm** |
| 8 | `3,3 V` | the same ordinary 3,3 V — the power board's supply |

**`12V` — the 12 V terminals.** Two two-pole Degson **`DGPS2.5R-5.0`**, 5 mm, 0,75–2,5 mm², an
input and a tap on the same node, taking the unit power board's island output off its own
terminals; the board's two `TPS629206` make the 4,0 V and the 1,8 V from it. No 12 V rides a ribbon: pin 12 of
`NB IN` and pin 8 of `PWR IN` are this board's 3,3 V, and nothing else on the bodies is a supply.

**The parts the interface adds:** 2× `SN74AXC8T245` · `PCA9306` · 2× `DGPS2.5R-5.0` · the resistors
— `ID` 10 kΩ 1 % ×2, `LINE_EN` 10 kΩ, 100 kΩ ×7 (five inbound, two outbound), 4,7 kΩ
×4, 200 kΩ ×1.

### Clock tree

| | |
|---|---|
| `OSC_IN` | 2²² = 4,194304 MHz, the data body's `CLK` through the translator — no crystal on this board |
| HSE | bypass |
| PLL1 | M 1 · N 128 · P 2 → VCO 2²⁹ = 536,870912 MHz, `P` **2²⁸** |
| SYSCLK, AXI, AHB | **2²⁸ = 268,435456 MHz** — VOS0, `VDD` ≥ 1,71 V |
| APB1/2 | ÷2; the timer kernels at 2²⁸ |
| the converter's `ENC` | **2²⁶ = 67,108864 MHz** on PA7, HCLK ÷ 4 — the encode and the grid; `PSSI_PDCK` comes back from the converter's `DCO` at the same rate and carries no grid of its own |
| the clock gone | HSI fallback and the degraded flag; the off sequence drops the clock the same way and the returning feed is the wake (`../core/PROTOCOL.md` §7) |

One integer PLL and nothing divides by anything but a power of two, from the 2²² on the wire down
to the converter's encode.

### The PSRAM — the position, from the sheet

The figures are AP Memory's rev. 1.2 sheet.

| | |
|---|---|
| the part | **`APS25608N-OBR-BD`** — 256 Mb (32 M × 8), octal SPI DDR, 1,62–1,98 V; **mini-BGA 24, 6 × 8 × 1,2 mm, pitch 1,0 mm, ball 0,4 mm**, package code `BD`; the standard grade, `T_C` −40…85 °C (`-OBRX-BD` is the 105 °C grade and is not needed). 200 MHz on the sheet, 2²⁶ here |
| the balls | `CE#` A2 · `CLK` B2 · `DQS/DM` C3 · `ADQ0` D3 · `ADQ1` D2 · `ADQ2` C4 · `ADQ3` D4 · `ADQ4` D5 · `ADQ5` E3 · `ADQ6` E2 · `ADQ7` E1 · `VDD` D1, E4 · `VSS` C1, E5 · `RST#` A3 · `RFU` C2 (a second `CE#`, left open) · the rest `NC`. **`VDDQ` is `VDD` inside the part** — two supply balls, one rail |
| `RESET#` | **left open** — a weak pull-up inside; the firmware resets by the Global Reset command (four clocked `CE#` lows) after the 150 µs power-up phase |
| power-up | `CE#` tracks `VDD` within 200 mV and `CLK` stays low through the first 150 µs — the **10 kΩ pull-up on `NCS`** and the MCU's pins at reset do both; Halfsleep and deep power-down are usable only 1 ms and 500 µs after that |
| the bus | `IO0`–`IO7` PF0–PF2 · PF3 · PG0 · PG1 · PG10 · PG11 · `DQS` PF12 · `CLK` PF4 · `NCS` PG12; `NCLK` PF5 not used, single-ended clock. Source-synchronous: no series part on the data lines or `DQS`, traces ≤ 30 mm and matched to ±5 mm over an unbroken ground — **the sheet allows 15 pF of load**, and the pads are 5–6 pF; **33 Ω at the MCU's `CLK` pad**, the house series part on a fast line. Output drive 25 Ω, the default |
| `NCS` | **10 kΩ to the 1,8 V** — deselected while the MCU's pins are high-Z at boot, and the power-up condition above |
| decoupling | **the sheet's two**: a low-ESR **1 µF** at the supply balls (the house 1 µF 50 V) and a **10 µF** beside the part (the house 10 µF 50 V 1206) for the self-refresh bursts — 25 mA peaks of tens of µs in standby, which the regulator does not see. No filter part: a digital load on a digital rail |
| current | **≤ 20 mA while a record is written** (the sheet: 5 mA at 13 MHz, 19 mA at 133 MHz, 2²⁶ between them); standby **≤ 680 µA at 85 °C**, 90 µA typical at 25 °C; Halfsleep 40 µA typical, deep power-down ≤ 20 µA — none of them worth a mode on a board whose rails are sized for full load |
| clock | 2²⁶ = 67,108864 MHz, the OCTOSPI kernel at 2²⁸ ÷ 4; DDR, so 134 MB/s on the bus — a 120 kB record is under a millisecond. **Write latency code 4 (`MR4[7:5]` = 100b, `F_max` 109 MHz)** — the default code 5 also holds, code 3 stops at 66 MHz and is under the clock. **`CE#` low ≤ 4 µs a burst (`t_CEM`)**: the OCTOSPI's `REFRESH` field caps a memory-mapped access at 268 clocks, so the peripheral breaks long DMA transfers by itself |
| when it runs | the trace and the ionogram parameters; the long record where the `LONG` option is built (`FIRMWARE.md` §6); fitted on every Marconi |
| **the second one — the `LONG` option** | the same part on **OCTOSPI2, port 1**: `IO0`–`IO7` PD11 · PD12 · PF7 · PD13 · PD4 · PD5 · PD6 · PD7 · `CLK` PF10 · `DQS` PC5 · `NCS` PG6 with its 10 kΩ to the 1,8 V, the same decoupling, the same trace rules; **unpopulated unless the site takes `LONG`**. What it buys is one bus more, not more bytes: the raw 2²⁶ record is 134 MB/s, one OCTOSPI's whole width, and two buses carry it at half load each. A larger part on the one bus changes nothing (`WHY.md`) |

### Analogue and reference

**Every filter inductor on this board is a π filter: 10 µF 1206 + 100 nF 0603 before it on the rail and
the same pair at the pins behind it** — the house rule (`../core/POWER.md`, *The filter parts*).

**The converter's `DRVDD` takes the 1,8 V through the shielded 2,2 µH into 10 µF + 100 nF, then the bead `GZ2012D301TF` into 100 nF at the pin** — the `AD9265`'s words at 67 MHz put a data line's fundamental at 34 MHz at most, under the inductor's 69 MHz self-resonance, and the edges' harmonics above it, where the bead absorbs them. `VDDA` and `VREF+` are one node on the **1,8 V** through one shielded 2,2 µH `SWPA252012S2R2MT`, 10 µF + 100 nF at the pins — the rail is a buck's own, so the filter is the buck-noise inductor of `../core/POWER.md` and not a ferrite; `VSSA` to
the ground plane at one point — the LQFP176 bonds `VREF−` to `VSSA` inside. The two `ID` inputs are read
single-ended against `VREF+`, and their 10 kΩ 1 % pull-ups hang on the same 1,8 V, so the reading is a
ratio and the rail's tolerance drops out; the bodies' 3,3 V is not used for the `ID` on this
board. The arrived 12 V and the input current are the power body's `INA238`, over I2C1.

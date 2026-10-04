★ N.I.C. ★

# Tesla — the board

> **Design-stage concept.** The figures are verified against the parts' sheets and the parts
> before a board is made.

**One board**: the three-rod magnetic front end, the `ADS127L14` converter with its reference and
clock, the STM32H7A3IIT6 that runs the detection, the rails, the thermometers and the Galvani
sockets. **It is Tesla's board and Pip's alike** — one board, two images, and not a part on it
differs between them: `TIME OUT` is populated on every board, served by Pip's image and left idle
by Tesla's. §0 is the whole board at value level; the sections after it carry the calculation behind
each value.

---

## 0. The starting build — what to draw

**Build exactly this.** What is trimmed afterwards is one resistor and rows in the station
database (§0.5).

### 0.1 The antenna — wound once, then left alone

| | value | why |
|---|---|---|
| rod | Mn-Zn ferrite, µ ≈ 800, **200 × 10 mm** on the prototype | Amidon `R33-050-750` (12,7 × 190 mm) or a Stormwise VLF rod for the network build (`CONSTRUCTION.md`) |
| turns | **150**, 0,315 mm enamelled — ≤ 0,352 mm over the enamel, **53 mm of single layer** | `SNR ∝ N · µ_eff`; halving N costs 6 dB. The wire is what fits 150 turns and the section gaps into the centre third of a 190 mm rod, 63 mm; 0,5 mm would want 82 mm |
| winding | **single layer, sectioned, on the centre third** | the sections keep the self-capacitance down and the centre keeps the sensitivity up; the pitch and the sectioning are the bench's |
| position | **centre third of the rod** | flux peaks at the centre and falls to zero at the ends; a full-length winding links the average and loses `µ_eff` |
| `L` | **2,5 mH** | the design value; `design/rod_calc.py` gives 2,3–3,3 mH on the prototype for a coupling of 0,5–0,7 — measured on the wound rod |
| `R_w` | **≈ 1,0 Ω** | 150 turns of 0,315 mm on a 10 mm rod is ~4,7 m of wire at 0,221 Ω/m — computed, not measured. It sets nothing: S1's lower corner is `Rs1`'s (§0.2) |
| self-capacitance | **≤ 125 pF** | sets S1's virtual-short bandwidth |
| `SRF` | ≈ 285 kHz — **passivated by the virtual short**: S1 holds the winding at zero volts, its capacitance carries no current and the resonance never develops. The current response stays flat past it (§2) | |
| shield | Faraday, gapped, to AGND, **stood off from the winding** | a shield laid on the winding is most of the 125 pF |

**The band is defended by the filter chain (§0.2), not by the rod.** With the resonance passivated
the rod is live well past the band, so out-of-band carriers reach S1 — the RC chain and `Rf1`'s
headroom are sized for that. **Do not re-wind for more turns**: the electronics already sit under
atmospheric QRN, so turns buy no reception and double every in-band carrier at S1.

### 0.2 One channel — three identical copies, and a fourth converter channel spare

#### The differential convention — read this before picking a resistor

**`Rf1` and `Cf1` below are the DIFFERENTIAL values. Each physical part is half the resistance and
twice the capacitance, and there are two of each.**

An FDA transimpedance stage has feedback on both legs — `OUT+` → `IN−` and `OUT−` → `IN+` — with the
source bridged across the inputs. For a signal current `I_s` through that bridge:

```
V(OUT+) = V_cm + I_s·R          V(OUT−) = V_cm − I_s·R
V_diff  = 2 · I_s · R
```

**so the differential transimpedance is `2R`, not `R`.** Everything in this document — the
transimpedance, the noise gain `1 + Rf1/ωL`, the loop gain `2π·GBW·L/Rf1` and the feedback noise
`√(4kT·Rf1)` — is written in terms of the differential value, which keeps the equations
single-ended in form. The BOM is where the factor of two lands:

| stated | fits the equations | **on the board** |
|---|---|---|
| `Rf1` | differential | **`Rf1`/2, two of them** |
| `Cf1` | differential | **2 × `Cf1`, two of them** — the pole `R·C` per leg is what must stay put |

**`Rg`, `Rf2`, `Rs` are already written per leg** (marked ×2) and do not carry this factor, and the
bridging elements — the ladder's `C1`/`C2` and the unpopulated `Cd` — span the two legs and are
single parts by construction.
S2's gain is `Rf2/Rg` with no factor of two, because it is a voltage stage and not a
transimpedance one.

**This does not apply to Helion**, whose `LTC6268` charge amplifier is single-ended against ground.

| Ref | value | tol / dielectric | what it sets |
|---|---|---|---|
| **S1** `THS4551` | **+5 V**, `VOCM` = 2,5 V | — | transimpedance, virtual short across the loop. **The 5 V is the converter's requirement, not the amplifier's**: `VCM` is 2,5 V and the FSR is ±4,096 V differential, so each pin has to reach **0,452 … 4,548 V**. On +5 V the THS4551 gives 0,20 … 4,80 V, a 0,25 V margin at each end; on +3,3 V it gives 0,21 … 3,09 V and is **1,46 V short at the top** |
| `Rs1` | **15 Ω ×2** | 1 % thin film | **in series with the coil, one per leg — the same value on both legs, so the differential input sees a symmetric source.** With the winding's own 1,0 Ω that is 31,0 Ω, which puts S1's lower corner at **2,0 kHz** — **31,9 dB of mains rejection at 50 Hz**, for **0,6 dB** at the bottom of the band. The 31,0 Ω together are 0,72 nV/√Hz against the amplifier's 3,3 nV. **It is also the current limit for the clamp below, so it goes first** |
| `D1` | **BAV199** ×1, two diodes anti-parallel across the pair | low leakage | **the surge clamp, on the amplifier side of `Rs1`.** Behind the virtual short the coil sees 4,3 mV at full swing, so a 0,5 V threshold is 120× clear; it conducts only once S1 has stopped holding the short. ~3 pF against the winding's 125 pF. Shot noise at 1 nA leakage is 0,018 pA/√Hz against 6,2 pA/√Hz of signal floor — 340× below |
| `Rf1` | **4,42 kΩ** differential → **2,21 kΩ ×2** | 1 % thin film | the transimpedance, and the **build constant — the same on every board, no trim.** It sets the **clipping field, ≈ 690 nT** on the prototype rod (§0.4 for what range that is, and for the recompute against the built rod). The noise floor that comes with it (~9 pT) still sits under outdoor QRN everywhere realistic |
| `Cf1` | **31 pF** differential → **62 pF ×2**, each with **10 Ω** in series | C0G · 0402 thin film | **the first pole of the chain** — corner **1,16 MHz**, an octave above the 512 kHz band top rather than at it (§2); the parts land where E24 lands and the node's FIR calibrates the whole measured curve. Stability: the minimum against the winding's 125 pF is **5,8 pF**, so this keeps 5,3× of margin, the high-frequency noise gain `1 + C_in/Cf1` is ~5,0, and the THS4551 is unity-gain stable. **10 Ω in series with each `Cf1`** — TI's differential-transimpedance procedure, so the capacitor does not resonate with the amplifier's inductive open-loop output impedance; the zero it makes with 62 pF sits at 257 MHz, two decades above the 27 MHz closed loop, so the 1,16 MHz pole does not move |
| **S2** `THS4551` | +5 V, `VOCM` = ADC `VCM` | — | band-pass + ADC drive, gain ×2. Swing **4,60 V** differential at 25 °C, **4,56 V** over −40/+125 (rails 0,20/4,80 and 0,22/4,78) |
| `Cin` | **33 nF** ×2 | X7R | high-pass **4,8 kHz** — the second high-pass pole (the antenna's 2,0 kHz is the first). The corner is set against the whole 990 Ω series sum below, so it did not move when `Rg` shrank |
| **RC ladder** | **2 sections**: `R1` **270 Ω** ×2 → `C1` **330 pF** across the pair → `R2` **390 Ω** ×2 → `C2` **330 pF** across the pair | 1 % / C0G | **the inter-stage chain.** Effective poles **1,23 and 1,10 MHz** — with `Cf1` (1,16 MHz) and `Cf2` (1,17 MHz) four staggered real poles, all of them above the band, each section different values, the high-pass poles turning back part of the phase the low-pass poles cost. The shunt capacitors bridge the pair (one part per node, no ground reference). No inductor anywhere — the only wound part in the chain is the antenna |
| `Rg` | **330 Ω** ×2 | 1 % | the ladder's termination — and the third piece of the gain resistance: **`R1`+`R2`+`Rg` = 990 Ω per leg is what `Rf2` works against**, so the ladder's series resistance is not an insertion loss at all, it is part of the gain resistor. S2's noise gain (×3) is unchanged from the old 1 kΩ `Rg` |
| `Rf2` | **2 kΩ** ×2 | 1 % | **×2 — settled** (2 kΩ / 990 Ω = 2,02): the insertion-loss question dissolved with the series-sum trick above, and ×4 is not needed |
| `Cf2` | **68 pF** ×2 | C0G | the closing low-pass pole, **1,17 MHz** |
| **attenuation terminals** | 2× push-in clamp across the `Rf2` legs, **unpopulated** | — | the only field trim: a metal-film resistor pressed in per leg lowers the driver gain without touching any filter corner. For the site that turns out to sit under a transmitter; every board ships identical |
| `Rs2` | **24,9 Ω ×2** | 1 % thin film | the series arm between S2 and the converter — they form the RC at `AIN±` with the footprint below, and they are what the datasheet's own input network asks for |
| `Cd` | **not populated — settled** | — | the datasheet's 2 × 22 Ω + 2,2 nF is the charge reservoir for a sampling input; with the precharge buffers ON the input is a fixed ±1,5 µA and `Rs2` alone is the network. Fitted, 2,2 nF against 2 × 24,9 Ω is a fifth pole at 1,45 MHz — 0,5 dB more droop at 512 kHz and 1,3 dB more at 1 MHz, inside the one curve the FIR corrects and worth neither. The footprint stays across `AIN±` as the fallback if the buffers are ever turned off |

**Nothing in the chain switches.** The gain is a build constant sized to a clip field that is the
same everywhere (≈ 690 nT on the prototype rod, §0.4), so there is nothing left for a per-site
mechanism to do. The only provision is the pair of unpopulated clamp positions across `Rf2` above —
a pressed-in resistor, not a switch.

### 0.3 The converter

| | value | note |
|---|---|---|
| part | **ADS127L14IRSHT** | quad, 24-bit ΔΣ, RSH QFN-56 |
| filter | **sinc4** | short impulse response, linear phase — the CFD edge needs flat group delay |
| `OSR` | **16** | |
| `CLKIN` | **33,554432 MHz = 2²⁵** | H7A3 `MCO1` — **an eighth of the core clock, taken as PLL1's Q output, VCO 2²⁹ ÷ 16** (§5); SYSCLK itself is not an MCO1 source |
| `f_DATA` | **1048576 SPS = 2²⁰** | **`f_DATA = f_CLK / (2 · OSR)`** at `CLK_DIV` = 1 — the factor of two is a divider in the part's clock tree and it is easy to miss (§4) |
| speed mode | **max speed** | `AVDD1` must then be 4,5–5,5 V (§5.3 of the datasheet), which is why the 3,3 V rail is not an option for this part |
| noise · dynamic range | **25,1 µV · 101,2 dB** | 18,3 bits of effective resolution |
| −3 dB | **238,3 kHz** | 0,2272 × `f_DATA`; the band runs to **512 kHz** and the sinc4 droop across it (−14,8 dB at the top) is part of the one measured curve the node's FIR corrects |
| reference | **external, `REF6041`, 4,096 V on `REFP`, `REFN` on `AVSS`** | the part has no reference of its own; FSR ±4,096 V differential, `REF_RNG` the high-reference range |
| input range | **1×** | 2× is only offered with the input buffers off |
| **input precharge buffers** | **ON** | turns a 95 µA/V signal-dependent load into a fixed ±1,5 µA. This is what allows a resistive divider on the pin at all |
| **reference precharge buffer** | **ON** | ±3 µA a channel on `REFP` instead of 225 µA/V a channel — with the `REF6041`'s own output stage it is why the reference needs no op-amp beside it |
| `DCLK` | **33,554432 MHz = 2²⁵** | 32 bit-times per frame — `CLKIN` itself. **Under the SAI's 50 MHz slave bit-clock ceiling with 1,5× in hand** (§8) |
| data path | frame-sync data port, `DOUT0..2` into **SAI**, not SPI | §8 |
| **supply note** | thin in distribution (2026) | a young part, not EOL — **TI direct first**, and buy a small stash when it surfaces. Fallbacks stay in-family, no redesign: **`ADS127L18`** (populate 3 of 8 channels) or **3× `ADS127L11`** (synchronised, single-channel — the stocked one) |

### 0.4 What the board should measure

| | value | where it comes from |
|---|---|---|
| converter noise | **25,1 µV** RMS | sinc4, OSR 16, max speed — datasheet |
| **noise floor, field-referred** | **≈ 9,1 pT** RMS over 5–512 kHz | computed at `Rf1` 4,42 kΩ through the full chain after the FIR has flattened it, on the prototype rod's `N·A·µ_eff` of 1,65 m² (`design/chain.py`) — the chain 5,7 pT ⊕ the converter 7,1 pT (§5); per strip 3,3 pT low, 1,35 pT middle, 1,7 pT high. Still under outdoor QRN, which is the only bound that matters |
| **clipping field** | **≈ 690 nT** | the converter's ±4,096 V full scale through S2's ×2,02 and `Rf1` — S2's 4,60 V rail sits above it and is the margin, not the clip — **this is the sized number and the one the bench can check** |
| **blind circle**, 30 kA stroke | **≈ 3–4 km** | a model output, not a specification — below. Given away by design; the network sees what the nearest station cannot |
| sinc4 flatness | **−3 dB at 238 kHz**, ~−15 dB at the 512 kHz band top | corrected by the node's FIR — the phase is linear, so the timing is untouched |
| alias exposure | folds arrive from **536,6 – 1560,6 kHz — the MW band**, and the transition band is gone: the first folding frequency sits 4,8 % above the band top | **the decimator carries it, not the chain** — sinc4 holds 16,5 dB at 536,6 kHz, 41 at 784,6 and 95 at 984,6, and the analog chain adds 3–10 dB across that span. The survivors are coherent carriers on a computed 9 kHz raster, notched digitally (§5). **A strip moved down buys protection back fast** (§2, the strip table) |
| **recovery from the rail** | the bench's number: a pulse past the clip into a rod's input, the time until the chain is back inside its linear range | it is how long a clipped event mutes the stream — the detector's dead time on a near stroke, and the blanker's under Pip's image |

**The blind circle is intentional.** Blitzortung's stations clip on near strokes too and the
network works, because time-of-arrival needs four stations and never the nearest one. A stroke
inside 5 km is seen cleanly by every other station in the country, and nothing man-made at these
frequencies comes near 690 nT — DCF77 at one kilometre is 5 nT, over forty decibels under the clip.

**The blind circle is a model output, not a specification.** The clip field is the hard number —
**≈ 690 nT** — and it falls out of the built chain: `Rf1` 4,42 kΩ, `L` 2,5 mH, the rod's effective
area and the converter's 4,096 V full scale through S2's ×2,02 — S2's own 4,60 V rail is the headroom over it, not the clip. What *range* that corresponds to needs a
return-stroke model the chain does not contain:

```
B = µ0 · I · v / (2π · c · d)        transmission-line radiation field
```

`I` the peak current, `v` the return-stroke velocity, `d` the range. **`v` is quoted between c/3
and c/2**, so a 30 kA stroke reaches 690 nT somewhere between **2,9 and 4,3 km**. One figure to two
digits is precision this model does not have, which is why the row above gives a range. *(The
induction term `µ0·I/2πd` is the wrong one here: at 5 km it is already the radiation field that
carries a sferic into this band.)*

**Both numbers follow the antenna actually built.** The chain's clip field
scales with the rod's **`N·A_eff`** — turns × core area × `µ_eff` — and `µ_eff` is a
demagnetisation figure set by the rod's length-to-diameter ratio and by the ferrite's *delivered*
permeability, not by the material figure on the datasheet. It is measured once on the wound rod at
characterisation, and the clip field and the blind circle follow from it; a 20 % `µ_eff` error
moves the clip field by the same 20 %.

#### 0.4.1 An arc has no blind circle, and its horizon is the site

**What the band hears from an arc is the re-ignition, not the fault current.** A 400 kV arcing
fault carries **30–54 kA** — the arc's own voltage is a kilovolt or two against 231 kV
phase-to-earth, so the current is set by source impedance and it is lightning class. It is also
at 50 Hz: `dI/dt` is ~1,3·10⁷ A/s against a return stroke's ~10¹¹, and `Cin`'s 4,8 kHz
high-pass plus `Rs1`'s 31,9 dB remove what is left. **What lands in band is the gap re-striking**
— the recovery voltage collapses across it twice a mains cycle and launches a travelling wave
with a sub-µs front. Its amplitude is the step voltage over the line's surge impedance, and the
fault current does not enter:

| source | step across the gap | `I = V/Z`, `Z` ≈ 400 Ω aerial mode | clips at 690 nT |
|---|---|---|---|
| **400 kV** phase-to-earth flashover | 327 kV peak | **820 A** | **225 m** |
| 110 kV | 90 kV | 225 A | 62 m |
| 22 kV | 18 kV | 45 A | 12 m |
| dry-band arcing insulator | 1–20 kV across the band only | 2–50 A | **under 15 m** |

Same model as §0.4, `β` 0,95 for an aerial-mode wave. **So no arc has a blind circle at any
distance a station is allowed to stand** — the siting rules keep Tesla kilometres off a line for
hum and rain static, and the nearest of these is 225 m. **The blind circle is a lightning
phenomenon and nothing else.**

**The horizon is the site's QRN and nothing else.** The chain's own 9,1 pT never binds outdoors,
so the reach follows the running floor:

```
d = 0,19 · I · (1 nT / B_det)        km, I in amps
```

`B_det` is the detection threshold — the running floor times the arming multiple. A site
detecting at **10 nT** reaches **16 km** on a 400 kV flashover, **4,3 km** on 110 kV, **850 m** on
22 kV and **50–950 m** on a tracking insulator; a quiet site an order lower reaches ten times
further. **The floor is measured on site at commissioning, and the horizon with it.**

**Two horizons, not one.** Detecting an arc as a *condition* gets the repetition gain — 100 to
300 firings a second, **30–35 dB** over a ten-second episode — so it reaches ~30× further than
the table. **Timing one edge to ±1 µs does not**: cross-correlation alignment needs per-pulse SNR
near unity, so the timing horizon is the single-pulse number, and localisation runs on the
shorter one.

**A single station cannot range an arc.** Amplitude is not range here — the step voltage is
unknown, the coupling to the rods depends on the line's geometry and the site's, and the line
re-radiates along its length. Range comes from TOA across stations or not at all, which is the
rule the board already lives under for lightning.

**The line guides the band, so the whole line radiates — but the first arrival is still the
fault.** Power-line carrier works at **30–500 kHz**, Tesla's band exactly, over sections of 25 km
and links past 1000 km; the conductor is a low-loss waveguide at these frequencies and the
transient runs both ways at 0,95–0,98 c, re-radiating everywhere it passes. Nothing re-radiated
can arrive first — it must travel to its point of re-radiation before it leaves the line. **Time
the leading edge, not the peak**, which is what the CFD already does; dispersion moves the peak
further behind the edge the further the wave has run, so the peak biases every station
differently.

### 0.5 Trimmed at commissioning · fixed at build

**Trimmed** — measured on site, recorded in the station database, no soldering:

- **the detection threshold**, learned from the running self-calibrated baseline
- **the placement of the three 16 kHz analysis strips**, moved off local interference
- **the inter-loop crosstalk matrix**, measured once and inverted in software
- **the carrier list** — which the site actually receives, in-band and folded, so the
  node's notches are set once instead of hunted for

**Fixed** — chosen here, changed only by a new build:

- **the antenna**, to §0.1
- **`Rf1` = 4,42 kΩ**, the transimpedance — it sets the clip, ≈ 690 nT on the prototype rod (§0.4)
- **the band, ≈5 kHz to 2¹⁹, the strips to 512 kHz**, and above 238 kHz it is the decimator that shapes it, not the RC chain of §0.2 (§2)

**Where adjustment lives.** Every operating point is reachable by an integer divide and a register
write — the clock tree is binary from the TCXO to the sample — so a mode that turns out badly is a
register, not a redraw. What would sit in the signal path is soldered:

| adjustable in firmware | soldered |
|---|---|
| `CLKIN`, the OSR, the speed mode, the filter | `Rf1` — the sensitivity, and with it the blind circle |
| the decimation ladder and every product it makes | `Cf1`, `Cf2` — the band |
| which strips the classifier compares | the series `Rs1` and the clamp |

**One survey before commissioning settles the site**: a spectrum sweep at the position gives the
QRN floor, the LF carriers received in band and folded (§5), the spectrum at the top of the band
that the clock-jitter table reads (§5), and — where Marconi shares the site — its medium-wave
strength and strong-carrier list. One instrument, one sweep, one record.

---

## 1. Board block diagram

```
                         TESLA — ONE BOARD, ONE plastic IP68 box, rods inside
 ┌──────────────────────────────────────────────────────────────────────────────────────┐
 │  3× IDENTICAL CHANNEL (one per ferrite rod)                                          │
 │                                                                                      │
 │   Rod k ──[Rs1+BAV199]──► [S1 THS4551 transimpedance, Cf1] ──► [RC ladder] ──►       │
 │  (loop)                    [S2 THS4551 ×2, Cf2, VOCM=VCM] ──► ADS127L14 AINkP/AINkN  │
 │                                                                                      │
 │   ADS127L14 (quad, 2²⁰ = 1048576 SPS, sinc4 OSR 16, REF6041 4,096) ── data port ─┐   │
 │        ▲ CLKIN = 2²⁵                                                             │   │
 │        └──────────────── MCO1 (VCO 2²⁹ ÷ 16) ◄────────────── H7A3 (same PCB) ◄───┘   │
 │                                                                                      │
 │   POWER, §6:                                                                         │
 │     12 V → TPS629206 (FPWM, 2,5 MHz) → 5,3 V → 3× TPS7A4701 → 5,0 V analog           │
 │     12 V → TPS629206 (FPWM, 1 MHz) → 1,8 V MCU · 2,2 µH → VDDA/VREF+                 │
 │                                           · 2,2 µH + bead → ADC IOVDD · AVDD2        │
 │     12 V → TPS629206 (FPWM, 2,5 MHz) → 3,3 V interface                               │
 └──────────────────────────────────────────────────────────────────────────────────────┘
   Antenna (3 shielded ferrite loops, 120° azimuth) and 3 of the 4 temp sensors are OFF-board —
  but inside the same single plastic enclosure; the spur's cables are the unit's only
  penetration. Rod-to-board noise inside one volume: the bucks run forced PWM at 1 and 2,5 MHz, above the band,
  and the rods sit at the frame's outer ends, as far from them as the frame allows.
```

---

## 2. One channel — the signal chain

Two **THS4551** FDAs per loop, fully differential end-to-end into the differential ADC.

```
 FERRITE LOOP (Mn-Zn μ≈800, ~150 t → L≈2,5 mH, SRF ≈285 kHz, Rw≈1,0 Ω, shielded/gapped Faraday → AGND)
                                        S1 : TRANSIMPEDANCE (current-mode, flat ∝ B)
        a ●───────────────── IN− ○──┐   ┌──[ Rf1 ]──┬──[ Cf1 ]──┐
                                 THS4551 #1 (FDA)    │           │
        b ●───────────────── IN+ ○──┘   │   OUT+ ○───┴───────────┼──► to S2 (+)
                              VOCM1 = 2,5 V (=VCM)  OUT− ○───┬────┼──► to S2 (−)
                                        └──[ Rf1 ]──┴──[ Cf1 ]──┘
   Loop is bridged ACROSS the FDA inputs (virtual short) → reads CURRENT. The loop's own
   inductance integrates dB/dt → OUT ∝ B (flat), no equaliser. Bias current returns through
   the DC-conductive winding, so an ordinary low-noise FDA is fine (no fA/electrometer part).

                                        S2 : ADC DRIVE (the RC ladder sits ahead of it)
   S1 ──[ RC ladder ]──[ Cin ]─[ Rg ]─ IN∓ ○──┐    ┌──[ Rf2 ]──┬──[ Cf2 ]──┐
                                           THS4551 #2 (FDA)     │           │
                                      VOCM2 = ADC VCM (2,5 V)   │  OUT+ ○─[Rs2]─► AINkP
                                                                │  OUT− ○─[Rs2]─► AINkN
                                           └──[ Rf2 ]──┴──[ Cf2 ]──┘
   (across the Rf2 legs: the two unpopulated clamp positions — the only field trim, §0.2)
```

### Values and the equation that sets each

| Part | Sets | Equation | Start value | Note |
|---|---|---|---|---|
| **Rf1** | S1 transimpedance gain | Z_t(flat) = Rf1·(N·A)/L | **4,42 kΩ** differential = **2,21 kΩ ×2** | the build constant — it sets the clip, **≈ 690 nT** on the prototype rod (§0.4). §0.2 for the factor of two |
| **Cf1** | first anti-alias pole | f = 1/(2π·Rf1·Cf1), **per leg** | **31 pF** differential → **62 pF ×2** (E24), 10 Ω in series with each | corner **1,16 MHz** — per leg, 2,21 kΩ against 62 pF; above the band, not at its edge (below). Also the stability element: against the winding's ~125 pF the minimum is **5,8 pF on the differential R** or **8,2 pF per leg** (`√(C_in/(2π·Rf1·GBW))`, GBW 135 MHz, *The differential convention*) — **31 pF keeps a margin of 3,8× to 5,3× under either convention, so which one the formula wants does not move the part**. The noise gain `1 + C_in/Cf1` = **5,0 on the differential Cf1** and leaves 27 MHz of closed loop |
| (loop Rw/L + Rs1) | S1 lower corner | f = R/(2π·L) | ≈2,0 kHz | the antenna-level high-pass; **the resistor changes with the antenna** |
| **RC ladder** | inter-stage anti-alias | poles ≈ 1/(2π·R_eff·2C) | **270 Ω / 330 pF · 390 Ω / 330 pF** | effective poles **1,23 and 1,10 MHz** — the resistors do not move (they are part of the gain resistance), only the caps. Staggered against `Cf1`'s 1,16 and `Cf2`'s 1,17; the caps bridge the pair |
| **Rg** | S2 input R, ladder termination | gain = Rf2/(R1+R2+Rg) | **330 Ω** | the series sum is **990 Ω** — the ladder resistors are part of the gain resistance, so they cost no gain |
| **Rf2** | S2 gain | **×2,02** = Rf2/(R1+R2+Rg), **per leg** | **2 kΩ ×2** = 4 kΩ differential | the smallest gain that keeps the front end above the converter; ×4 was the fallback and is not needed. Stated both ways for the same reason Rf1 is |
| **Cin** | high-pass (4,8 kHz) | f = 1/(2π·(R1+R2+Rg)·Cin) | **33 nF** | AC-couples the chain; kills mains/sub-kHz |
| **Cf2** | closing low-pass pole | f = 1/(2π·Rf2·Cf2) | **68 pF** | corner **1,17 MHz** |
| **Rs2** | series into the ADC pins | — | **24,9 Ω** ×2 | the datasheet's own input network; the precharge buffers carry the rest |

**The chain's computed response** (0 dB = the 30–100 kHz plateau; analog only, before the sinc4
and the FIR):

| | dB |
|---|---|
| 5 kHz (band bottom) | −3,4 |
| 16 kHz (the low strip) | −0,4 |
| 256 kHz (the middle strip) | −0,8 |
| **512 kHz (band top)** | **−3,0** |
| 524,3 kHz = 2¹⁹ | −3,2 |
| **536,6 kHz (the first folding frequency)** | **−3,3** |
| 728,6 kHz | −5,7 |
| 1 MHz | −9,5 |

**The four poles sit an octave above the band, not at its edge.** With the strips reaching 512
kHz an alias filter is not available at any pole placement: at `f_DATA` = 2²⁰ the first folding
frequency is `2²⁰ − 512 kHz` = **536,6 kHz**, **4,8 % above the band top**, and no analog filter
passes 512 and stops by 536,6. So the chain gives up the job entirely — it is flat across the
band, down 3,3 dB where folding starts, and **what carries the folds is the decimator and the
notches** (*Aliasing*, §5). Group delay runs from 1,6 µs at 32 kHz down to 0,44 µs at 512 kHz; below 10 kHz it rises on the high-pass poles, which is the same measured curve.

The values are written as **computed (ideal) targets**; the parts land where E12/E24 lands, the
response floats ±1 dB with tolerance and layout, and none of it matters — **the node measures the
whole curve once and the FIR corrects it entire.** Accuracy lives in the digital correction;
the analog chain only has to be stable. Steeper skirts, if ever wanted, go to that linear-phase
FIR, **not** more analog poles.

### The band is 5 kHz – 512 kHz — timing is bought with bandwidth

The onset a CFD can resolve is **rise ≈ 0,35 / BW**, so the band *is* the timing floor:

| band top | rise | for comparison |
|---|---|---|
| 70 kHz | ~5 µs | |
| 250 kHz | ~1,4 µs | |
| **512 kHz** | **~0,68 µs** | Blitzortung's System Blue stops at 300 kHz |

**The band runs to Nyquist, 2¹⁹; the strips stop at 512 kHz.** The sample rate is 2²⁰ and the
band ends where the arithmetic says, at 2¹⁹, in powers of two like everything else; what has to
settle below the filter's knee is the measuring strip, so the high strip sits at 496–512 kHz, and
512 is the round number the strips are laid out against. The strips are recommended positions,
not fixed ones. The 12,3 kHz between them is not a guard band and is not
claimed as one — see below.

**The converter was already paying for the rate.** Running the band to 512 changes no clock, no
sample rate and no frame — it spends headroom that is already sampled and already phase-locked to
network time. **What it costs is the transition band, and the cost is the whole of it:** the
first folding frequency is `2²⁰ − 512 kHz` = **536,6 kHz**, so there is no room for an analog
skirt at all. That is a deliberate trade and *Aliasing* (§5) is where it is paid.

### The three strips — width, default placement, and how far they move

**Everything the classifier computes, it computes from three narrow strips** (`DETECTION.md`, *The
strips*). They are cut digitally out of the 2²⁰ stream by mixing and decimation, so
their placement is firmware, not a component:

| | width | default | edges |
|---|---|---|---|
| **low** | 16 kHz | centre 16 kHz | **8 – 24 kHz** |
| **middle** | 16 kHz | centre 256 kHz | **248 – 264 kHz** |
| **high** | 16 kHz | centre 504 kHz | **496 – 512 kHz** |

**The defaults are a starting point and nothing more.** A site's interference decides where the
strips actually sit: PV inverters switch across 16–100 kHz, MW carriers fold in from above, and
what is clean in Bohemia is occupied in Texas, on Kamchatka or in Australia. **Every strip moves
anywhere inside 5 – 512 kHz**, and at binary centres the mixers cost nothing (±1, ±j) while the
CORDIC makes an off-grid centre nearly free, so the placement is a per-station commissioning
choice rather than a design constant.

**What a strip's height costs it — this is the table to read before moving one.** Interferers
fold onto `|f − 2²⁰|`, so a strip with its upper edge at `f_top` receives its folds from
`2²⁰ − f_top` upward. The analog chain is flat there by construction, so the protection is the
decimator's sinc4 plus what the chain still has left:

| strip upper edge | folds arrive from | sinc4 | analog | **total** |
|---|---|---|---|---|
| 24 kHz | 1024,6 kHz | −130,3 | −9,9 | **−140,2 dB** |
| 64 kHz | 984,6 kHz | −95,0 | −9,3 | **−104,3 dB** |
| 128 kHz | 920,6 kHz | −69,2 | −8,4 | **−77,6 dB** |
| 264 kHz | 784,6 kHz | −41,4 | −6,4 | **−47,8 dB** |
| 320 kHz | 728,6 kHz | −34,0 | −5,7 | **−39,7 dB** |
| 384 kHz | 664,6 kHz | −27,0 | −4,8 | **−31,8 dB** |
| 448 kHz | 600,6 kHz | −21,3 | −4,1 | **−25,4 dB** |
| **512 kHz** | **536,6 kHz** | **−16,5** | **−3,3** | **−19,8 dB** |

**So the high strip at its default is the exposed one, and it is exposed to the bottom of the MW
band.** 531, 540 and 549 kHz are high-power European channels; 540 kHz folds to **508,6 kHz**,
inside the default strip, with those 19,8 dB. **The answer is the notches, not the filter** — a
folded broadcast carrier is a fixed, coherent, permanently-on tone on a computed 9 kHz raster,
and one biquad removes it entirely (§5). **The lever the site has is the strip itself**: move it
down and the table gives back protection fast — 448 kHz is worth 5,6 dB, 384 kHz is 12, 320 kHz is
20. A station under a strong MW transmitter runs its high strip lower and loses a little rise
time; a quiet station keeps it at 512.

### Why the protection is thin — a paid position, not an oversight

**The transition band is 4,8 % and nothing analog crosses it.** Three ways out were weighed and
each is priced:

- **More poles.** They buy nothing here, and the arithmetic is exact rather than approximate. A
  flat passband fixes `n·(B/f_c)²`, so the attenuation at `(1+δ)·B` is `3 dB · (1+δ)²`
  **independent of the order**: 4 poles and 98 poles both give **3,3 dB** at 536,6 kHz. Order
  helps far above the band edge, never just past it.
- **Zeros — an elliptic filter.** They do cross a narrow transition, which is why the decimator's
  own nulls are the mechanism this design ends up using. But an analog elliptic section is not
  linear phase, and a filter that rings *before* the event destroys the CFD edge the unit exists
  for; the family carries no inductors either. And one zero is not enough: the folds arrive from
  a **band**, 536,6 – 1560,6 kHz, not from a frequency.
- **The part's own wideband filter**, which is the sound-card answer — flat to 0,45 · `f_DATA`
  with a deep stopband. Two things exclude it: its output rate stops at **512 kSPS**, capping the
  band near **230 kHz**, and its long symmetric impulse response smears and pre-rings a µs-class
  edge (§4, *Tesla takes the sinc4*).

**And under all three: aliasing is irreversible.** Once folded, an interferer sits at a real
in-band frequency and no filter downstream — an FIR however long — separates it from signal
there. That is why the answer is placement and mobility rather than filtering, and why the
station's own bucks are kept out of the chain by distance and layout rather than filtered away —
forced PWM at 1 and 2,5 MHz, above the band, and the 65 mV bench criterion (§6; the family rule,
`../galvani/README.md`, *Converters and measuring boards*).

**The passband is flat as drawn, and the droop lever is closed.** Droop is corrected by the
node's FIR and a fold is not, so flatness could be traded for rejection by lowering the four
corners together — but the FIR restores the response and not the SNR: the converter's noise is
generated *after* the filter and comes back up with the signal, while the chain's own does not,
and the converter is the larger term over the upper band (§5). The trade, integrated per strip
after the FIR, field-referred:

| droop at 512 kHz | `Cf1` corner | at 536,6 kHz | at 1 MHz | low strip 8–24 | middle 248–264 | high 496–512 | 5–512 kHz |
|---|---|---|---|---|---|---|---|
| **3 dB — as drawn** | 1,16 MHz | −3,3 | −9,6 | 3,25 pT | 1,34 pT | **1,70 pT** | **9,1 pT** |
| 6 dB | 714 kHz | −6,5 | −16,3 | 3,25 pT | 1,47 pT | 2,36 pT (+2,9 dB) | 10,2 pT |
| 10 dB | 496 kHz | −10,7 | −23,3 | 3,25 pT | 1,74 pT | 3,66 pT (+6,7 dB) | 12,7 pT |

**Six decibels of droop buy 3,2 dB against a fold at the first folding frequency and cost the high
strip 2,9 dB; ten buy 7,4 and cost 6,7.** The strip that pays is the one whose only job is rise
time, the folds it would be bought against are the MW carriers, which are coherent tones and go
to the notches entire, and the low strip, where reach lives, does not move either way. The chain
stays flat and the alias protection is the decimator's and the notches' (*Aliasing*, §5).

**The strip table is permanent on this part.** At 2²¹ the
first folding frequency for a 512 kHz strip would move to 1,585 MHz and the geometry would return
to what the 250 kHz build had — flat band, 3× guard, the analog chain doing the work again. But
`ADS127L14` tops out near **1300 kSPS**, so 2²⁰ = 1049 kSPS is the last binary rung it reaches
and 2²¹ = 2097 kSPS is off the part entirely. Buying the guard back means a different converter,
which is a bigger question than this band; **inside this design the strip table is the answer,
not a stopgap.**

**Moving a strip is a protocol act, not a rebuild.** Placement rides the front's registers —
`SET register value`, *"what a register means past the common map is the front glue's"*
(`../core/PROTOCOL.md` §5, §9) — one register per strip carrying its centre. Two consequences follow
from the protocol and are not this board's to choose: a runtime setting change **is a config in
the archive** (the head writes the new settings into the unit's file from that second, and a
reader applies them from there), and the node keeps no history of its own. That is exactly right here, because every
classifier feature is computed *in* the strips: a record is only interpretable against the
placement that produced it.

**The low end does not move.** 5 kHz is where sferic energy peaks and where the long paths live.

**The sinc4 is ~−15 dB at the band top and that is not a defect.** The whole response — sinc4
droop, RC chain, tolerances — is one measured curve, and the node's linear-phase FIR corrects it
entire. The phase is linear at every step, so the CFD edge is untouched.

### The winding resonance is passivated, not avoided

The winding's ~125 pF self-capacitance would resonate the 2,5 mH near 285 kHz — inside the band.
It does not, because **S1's virtual short holds the winding at zero volts: a capacitance with no
voltage across it carries no current, and the tank never forms.** The current response `I = EMF/ωL`
stays flat past the resonance, which is what lets the band run to 512 kHz on the rod.

### Why the load is a virtual short and not a resistor

**Damping removes the peak, not the roll-off.** With any resistive load the flat region is bounded
above by `SRF`: past it the winding's own capacitance shunts the source and the current available
to the load falls as `1/ω²`, however heavily it is damped. *(And a 2 kΩ load would put the lower
corner at `R/2πL` = **127 kHz**, so the bottom of the band would rise with frequency. Flat from
5 kHz wants ≈ **78 Ω**, which is affordable on noise — 1,1 nV/√Hz — and fatal at the top.)*

**A virtual short holds the winding at zero volts, so its self-capacitance has no voltage across
it and draws nothing.** The flat band then runs until the amplifier stops holding that zero.

**Which has a number, and the same 125 pF sets it:**

```
f ≈ √( GBW / (2π · R_f · C_in) )        THS4551: GBW 135 MHz
```

**135 MHz, not the 150 on the front page** — that figure is the small-signal bandwidth at G = 1;
the gain-bandwidth product is 135 MHz. And `C_in` is **126,2 pF**: 125 pF of winding plus the
amplifier's own **1,2 pF** differential input capacitance (the datasheet gives 100 kΩ ∥ 1,2 pF).
The self-capacitance really is all coil.

| `R_f` on S1 | where the virtual short ends |
|---|---|
| 22 kΩ | 2,78 MHz |
| **4,42 kΩ — as built** | **~6,2 MHz** |
| 1 kΩ | 13,0 MHz |

**So the virtual short is not the limit — the winding is.** At the built `Rf1` the amplifier holds
zero more than eleven times past the band top, and what actually bounds S1 is `Rf1 · Cf1` = 1,16 MHz,
a corner put an octave above the band on purpose. Cutting the self-capacitance still helps the coil; it buys the
amplifier nothing it needs.

### The gain belongs in the first stage, and the clip spec is what sizes it

**Current mode costs no signal-to-noise.** Where the amplifier's voltage noise dominates — which
is the whole band — the ratio is `ω·N·A·B/e_n` in either mode. What current mode changes is the
*shape* (flat instead of rising) and the *capacitance* (bypassed instead of shunting).

**A second stage cannot improve what the first sets**, so `Rf1` decides the whole board — and it
is sized by one number that is the same at every site: **the clip field.** `Rf1` **4,42 kΩ** puts it
at ≈ 690 nT on the prototype rod and the noise floor at ~9 pT, still under outdoor QRN. What range a 30 kA stroke
reaches that field at is a model output and is in §0.4, not a requirement.

### S2 is ×2, and the converter is why

**The rule is the smallest gain that keeps the front end's noise above the converter's**, because
a converter that contributes nothing leaves nothing for analogue gain to rescue. **×2**: the
ladder's series resistors are part of the gain resistance (`R1`+`R2`+`Rg` = 990 Ω per leg against
`Rf2` = 2 kΩ, §0.2), so the ladder has no insertion loss to make back. The path's gain budget:
rod → `Rf1` 4,42 kΩ differential → chain ×2,02 → converter, clip at ≈ 690 nT exactly as
§0.2 sizes it.

**And the virtual short is comfortable.** At the built `Rf1` = 4,42 kΩ it holds to ~6 MHz, an
order past the band top.

### Choosing an amplifier for S1 — one inequality

**The front end is the limit on this board, and within it one term is the whole story.** At
`Rf1` = 22 kΩ, where the terms separate most clearly — the shape is the same at 4,42 kΩ, scaled:

| | `e_n` × noise gain | `4kT·Rf` | `i_n` × `Rf` |
|---|---|---|---|
| 5 kHz | **927,7 nV** | 19,1 nV | 11,0 nV |
| 50 kHz | **95,7 nV** | 19,1 nV | 11,0 nV |
| 250 kHz | 21,8 nV | 19,1 nV | 11,0 nV |

The amplifier's voltage noise against the coil's 78 Ω at the bottom of the band is forty times
everything else, and because the density there is thirty times higher, it carries the integral.
Input-referred this is `B_n = e_n / (ω·N·A_eff)` — the two levers are `e_n` and the antenna.

**A candidate part must stay voltage-noise-limited across the band**, which is one inequality:

```
i_n  <  e_n / (2π · L · 2¹⁹)  =  e_n / 8235
```

| `e_n` | `i_n` must be under |
|---|---|
| 3,3 nV/√Hz | 0,40 pA/√Hz |
| 2,2 nV/√Hz | 0,27 pA/√Hz |

The `THS4551` is 3,3 nV / 0,5 pA — at the widened band top it sits at its own bar rather than
under it, which is parity at the one frequency; `e_n` still carries the integral. **The trap is that almost every
FDA below 2 nV is bipolar with 2–11 pA**, and `i_n` × `Rf1` is expensive here — that is exactly
what disqualified the `THS4541`. Low `e_n` *and* low `i_n` together means a FET input, and
those are few.

### Sensitivity is bounded on purpose

**There is nothing to win below the sky and a great deal to lose.** The stop rule already says it —
once the electronics sit under atmospheric QRN, stop — and the reason is not economy. **A receiver
quieter than the sky does not hear more lightning; it hears everything local instead**: mains
switching, motors, fences, drives, the station's own converters. Every one of those becomes an
event the classifier has to reject and a candidate for the frame budget.

The bounds are already in place and they are three: **the analogue gain**, which decides whether
the floor at the converter is the sky or the electronics · **the detection threshold**, learned
from the running self-calibrated baseline rather than set by hand · and **the emission rule**, four
records per source a second. **The gain must not chase a site quieter than the design assumed —
the threshold is what follows a quiet site, not `Rf2`.**

**What the bound costs.**

- **N stays at 150 turns.** More turns buy no reception (the electronics already sit under
  atmospheric QRN) and double every in-band carrier at S1 — the stop rule decides
  (`CONSTRUCTION.md`, *The stop rule*).
- **DCF77 is in-band and harmless.** The ADS127L14 is 24-bit and holds a strong local carrier and
  a weak distant sferic 60–80 dB apart at once; with the clip at 690 nT even a station one
  kilometre from Mainflingen sees the carrier 40 dB under the rail. MSF/WWVB 60 kHz, RBU 66,66,
  BPC 68,5 and the 129–139 kHz telecontrol carriers arrive on the same argument.
- **The classifier learns the band.** The running noise floor, the envelope-lock test and the
  UFO threshold learn from what the site puts in 5–512 kHz; nothing about them is compiled in.

**This same band carries the SID channel, and the SID channel is Pip's image on this board** — no
added parts: the carriers sit inside the band, the identical analogue front feeds the carrier
trackers (Goertzel per carrier, ≤ 8, `pip/FIRMWARE.md`), and the board and the record
contract are the same under either image. Tesla's image tracks no carriers — during the storms
and arcing it exists for, they are the quietest thing in the band.

**Tolerances / dielectrics:** Rf/Rg **1 %** (a *ratio* sets the gain, so even 5 % is fine); `Rs1` and `Rs2` 1 % thin film, as §0.2 has them.
Small caps **C0G/NP0** (Cf1, Cf2 — they hold the corner over temperature and are low-loss).
**Cin: X7R is fine** (it only sets the 4,8 kHz mains-killer HP); make it **C0G only if you want that
corner pinned** — X7R drifts ±15 % over temperature. All 0402/0603 **JLC Basic** jellybeans; only the
two ICs are Extended.

**FDA supply = single +5 V, VOCM = 2,5 V**: the ADS127L14 runs
0–5 V with its common-mode at ≈2,5 V, so a single +5 V rail with VOCM tied to the ADC's VCM is the
standard, lower-parts driver topology — one fewer rail, no negative LDO. The loop simply floats at
the 2,5 V common mode (differential signal unaffected). THS4551 max supply 5,4 V → +5 V is in range.

---

## 3. The channels — three rods, and the fourth converter channel is spare

**Three identical copies of §2, one per rod, into `AIN0..AIN2`. `AIN3` is unpopulated.**

**Why three and not four.** A rod's pattern is a figure-8 and the station detects **per rod**,
because the CFD needs one clean edge and combining channels as RSS destroys the timing. Under
per-rod detection three rods at 120° dip to **−1,25 dB** in the worst direction (three axes 60°
apart, `sin 60°`) and four on the frustum at 90° dip to **−3,0 dB** (`sin 45°`), because for a
horizontally-arriving wave the four axes at 90° azimuth give only **two** distinct responses: the
coupling is `cos θ · |sin(α − φ)|`, and α and α + 180° give the same number. Four rods at 45°
would be four distinct axes and −0,69 dB — 0,6 dB over three, not worth a channel. **The fourth rod is four distinct sensors only for an elevated arrival**, and elevated
arrivals come from inside the blind circle, which is given away anyway.

**And the dip does not cost what it looks like.** −1,25 dB is 1,15× of range in one azimuth, and this
station is not range-limited — it joins an existing network rather than extending one. A dip only
matters when the far edge of coverage is the constraint, and here it is not.

Fixed inter-rod crosstalk is a constant linear mixing → measured once at commissioning and inverted
in software on the node, so the three front-ends need only be **stable and identical to each
other**, not perfectly decoupled. Lay them out as matched as practical (same routing, same part
lots).

---

## 4. ADS127L14 — the ADC

Quad, simultaneous-sampling, 24-bit ΔΣ, differential inputs, dynamic range **101,2 dB at the
board's operating point** (sinc4, OSR 16, max speed — the part reaches 130 dB at OSR 4096 and 4 kSPS),
THD −115 dB. Package **RSH (QFN, 56-pin, 7 × 7 mm)**, part **ADS127L14IRSHT**. The 8-channel
**ADS127L18** is the same package and the same pinout.

**Two digital filters, and the data-rate ceiling is the filter's, not the part's:**

| filter | lowest OSR | ceiling | shape |
|---|---|---|---|
| **wideband, linear phase** | 32 | **512 kSPS** | flat to 0,45 × `f_DATA` |
| **low-latency sinc4** | 12 | **1365 kSPS** max-speed · **1066 kSPS** high-speed · 533 mid · 133 low | short impulse response, −3 dB at 0,228 × `f_DATA` |

**The factor of two is a divider in the clock tree, not a property of the OSR.** `CLKIN` becomes
`f_CLK`; the modulator runs at `f_MOD = f_CLK / (CLK_DIV · 2)` and the filter decimates *that* by
the OSR, so `f_DATA = f_CLK / (CLK_DIV · 2 · OSR)`. **`CLK_DIV` is 1 on this board** — the
divide-by-1 setting is also what removes the synchronisation uncertainty a divided clock carries —
so the working form is `f_DATA = f_CLK / (2 · OSR)`. At the board's `CLKIN` of 2²⁵ the
wideband filter's lowest OSR lands on 2¹⁹, whose Nyquist is below the band top — so the filter
choice is forced as well as preferred (§5).

**Tesla takes the sinc4, and for the signal rather than for the rate.** The wideband filter's
long symmetric impulse response smears a µs-class event across its whole length and rings
*before* it; a sinc4 is short and still linear phase, so the group delay the CFD edge depends on
stays flat. Its passband droop is corrected by the node's FIR, which is where the shaping lives
anyway.

- **Supplies:** `AVDD1 = +5,0 V`, **`AVDD2 = +1,8 V` off the processor's rail
  through the shielded 2,2 µH, 2× 10 µF + 100 nF and the bead into 100 nF** — 20 µF puts the corner at
  ~24 kHz with a Q of ~2, ~65 dB at the buck's 1 MHz (the sheet allows 1,74–5,5 V: the rail's
  1,782 V at −1 % less ~8 mV across the two parts leaves 1,774 V), `AVSS = 0 V = AGND`, **`IOVDD = +1,8 V`** — the digital interface is
  **1,8 V CMOS only**, and IOVDD also feeds the converter's digital core through an internal
  regulator, so the rail carries more than pin drive. Every H7A3 pin facing the ADC therefore lives
  in a 1,8 V domain (§8). Draw: **`AVDD1` ≤ 32 mA** including the part's own buffers, the `REF6041` 1,1 mA beside it on the same LDO (§6), **IOVDD ≤ 5 mA** — 0,8 mA a channel
  at max speed on the sinc4 (SBASAM0B §5.5, the row at OSR 32), 3,2 mA for four, and ~1,2 mA of
  line drive into the SAI pins (`DCLK` at 2²⁵ and four `DOUT` lanes on ~10 pF each); `IOVDD`'s
  window is 1,65–1,95 V (SBASAM0B). **`AVDD2`'s rejection is ≥ 105 dB from dc to 100 kHz and
  ≥ 85 dB to 1 MHz** (SBASAM0B Figure 5-65), so the processor's load steps on the shared 1,8 V
  reach the conversion 100 dB down — 10 mV of ripple is 0,1 µV against the converter's 25,1 µV.
- **Reference: `REF6041`, external, and the part needs one** — the `ADS127L14` carries no reference
  of its own; `REFP`/`REFN` are inputs (SBASAM0B §7.3.2). The `REF6041` is TI's own pairing for the
  part (SBASAM0B Figure 9-1): 4,096 V ±0,05 %, 5 ppm/°C, 5 µV RMS integrated noise, and an output
  stage built to drive a sampled reference pin, stable on 10–47 µF (SBOS708C).

  | | |
  |---|---|
  | `VIN` | the converter's 5,0 V `AVDD1` rail — `VOUT` + 0,25 V to 5,5 V is its window, so 5,0 V has 0,65 V to spare; ≤ 1,1 mA |
  | `EN` | to `VIN` — never switched from a pin |
  | `SS` | open — the characterised condition, 10,5 mA short-circuit limit; the load is 3 µA a channel |
  | `FILT` | **1 µF** X7R to ground — the sheet's minimum for stability |
  | `OUT_F` · `OUT_S` | two traces to **22 µF** X7R at the converter's `REFP` (49, 50), the sense taken at the capacitor |
  | `GND_F` · `GND_S` | one plane, run to the capacitor's ground and to `REFN` (47, 48) — `REFN` is `AVSS` |
  | `VIN` bypass | 100 nF at the pin |
  | package | VSSOP-8, `DGK` |

  **Rejection of its supply is 60–75 dB across the band** (SBOS708C Figure 6-15), and what it
  rejects is an LDO's output, not a buck's. A disturbance on the reference scales the signal, not
  the floor: 5 µV on 4,096 V is 1,2 ppm of the reading.

  **4,096 V and not 2,5 V, for the swing**: ±4,096 V about `VCM` 2,5 V asks 0,45–4,55 V of each
  leg, and the `THS4551`'s 0,20–4,80 V on a single +5 V covers it with 0,25 V to spare (§0.2); a 2,5 V reference clips 4,3 dB earlier for the
  same `Rf1`, and `AVDD1` as the reference leaves the top 0,7 dB out of the amplifier's reach and
  puts the converter's own switching load on it. **Accuracy is not what picks it**: amplitude is
  calibrated per rod against a ferrite whose µ(T) moves by percent, ENOB is front-end-limited
  (~13–14 b) and the classification is relative; a precision reference without an output stage
  would want an op-amp beside it and buy nothing measurable.
- **VCM:** the converter's own **`VCM` output pin** drives both stages' `VOCM` — 2,5 V, the
  reference's midpoint, 0,1 mA in the §6 draw table; no buffer and no divider on the board.
- **Inputs:** `AIN0±`..`AIN2±` from the three S2 outputs through `Rs2` (§0.2); `AIN3±` unpopulated.
  **The input precharge buffers are ON**, which is what makes a resistive source on these pins
  legitimate at all:

  | high-speed mode | differential input current | as an input resistance |
  |---|---|---|
  | buffers **off** | 95 µA/V, drift 3 nA/V/°C | **10,5 kΩ** — a signal-dependent load that would also change with the speed mode |
  | buffers **on** | **±1,5 µA**, drift 5 nA/°C | fixed current, no voltage dependence |

  Through the **50 Ω** source the pins actually see (`2·Rs2`), the fixed ±1,5 µA leaves **75 µV**
  of DC offset and sub-µV/°C of drift, both out of band. The `2×` input range is offered only with the buffers off, which
  is one more reason not to use it.
- **Digital: the frame-sync data port on the H7A3's SAI**, the config SPI beside it, no `DRDY`
  (§8). Mode/OSR set by the node over the config SPI at boot.
- **`DEV_ID` (00h) reads 04h**, and the 8-channel `ADS127L18` reads 06h. Since the two parts share
  the package and the pinout, **that register is the only way the board can tell which one is
  fitted**, so the boot self-check reads it before it configures anything. `REV_ID` (01h) is the
  die revision and TI changes it without notice, so nothing may depend on it.
- Run **flat-out** — no decimation past what the OSR gives. The narrowing into products belongs in
  the node (§5), and the filter that preserves the CFD edge is the **sinc4**, chosen above.

### The pins — RSH, VQFN-56, SBASAM0B Table 4-1 and Table 9-3

| pin | name | on this board |
|---|---|---|
| 43 · 44 | `AINP0` · `AINN0` | rod 1, from S2 through `Rs2` |
| 41 · 42 | `AINP1` · `AINN1` | rod 2 |
| 39 · 40 | `AINP2` · `AINN2` | rod 3 |
| 37 · 38 | `AINP3` · `AINN3` | spare — both to `VCM`, inside the buffered input range; the channel is powered down |
| 49, 50 | `REFP` | the `REF6041`'s `OUT_F`/`OUT_S` node; **2,2 µF** X7R to `REFN` at the pins (the value with the `REFP` buffer on) beside the reference's 22 µF |
| 47, 48 | `REFN` | `AVSS`, run straight to the reference's ground pins |
| 46 | `VCM` | both stages' `VOCM` |
| 23, 24 | `AVDD1` | 5,0 V; **2,2 µF** to `AVSS` pin 22 |
| 25 | `AVDD2` | 1,8 V through the filter of §6; **2,2 µF** to `AVSS` pin 22 at the pin, the 100 nF behind the bead beside it |
| 26, 27 | `CAPA` | the internal analogue regulator; **10 µF** to `AVSS` pin 28, no load |
| 20 | `CAPD` | the internal digital regulator; **2,2 µF** to `DGND` pin 21, no load |
| 18, 19 | `IOVDD` | 1,8 V through the filter of §6; **2,2 µF** to `DGND` pin 17 |
| 22, 28–36, 45, 51 · pad | `AVSS` | the one ground plane; 29–36, 45 and 51 take no capacitor (Table 9-3) |
| 17, 21 | `DGND` | the same plane, joined at the converter |
| 54 | `MODE` | **to `IOVDD`** — SPI programming mode; the pin straps `OSR0`/`OSR1`/`FLTR`/`SPEED`/`TDM`/`HDR` mean nothing in it |
| 55 · 56 · 1 · 2 | `CS` · `SCLK` · `SDI` · `SDO` | the configuration port, SPI1 (§8) |
| 16 | `CLKIN` | MCO1, 2²⁵ |
| 14 · 15 | `DCLK` · `FSYNC` | the data port's clocks, out (§8) |
| 6 · 7 · 8 | `DOUT0` · `DOUT1` · `DOUT2` | the three rods' lanes, out (§8) |
| 9 | `DOUT3` | the spare channel's lane — not connected |
| 10 · 11 · 12 · 13 | `GPIO4` · `GPIO5` · `DIN1` · `DIN0` | **100 kΩ to `DGND` each** — TI's own layout holds these pins so none floats if it is ever programmed as an input |
| 3 · 4 | `GPIO0` · `GPIO1` | not used in SPI mode; 100 kΩ to `DGND` |
| 52 · 53 | `RESET` · `START` | PD1 · PD0 |
| 5 | `ERROR` | open-drain, **to PD2** on EXTI, falling edge; the part's own 100 kΩ pull-up to `IOVDD` holds it high, no part on the board — low is any `STATUS` flag set (below) |

**The supplies need no sequencing** (§9.3): any order, any ramp, as long as no input rises above
its own supply — which the one 5,0 V behind the front ends and the converter guarantees.

### The fault channel — the `ERROR` pin, and `STATUS` read on its edge

**The `STATUS` register (02h) carries the device's faults, every flag in it is `R/W1C`** — the
hardware holds the event until firmware writes a one — **and the `ERROR` pin is their OR**
(SBASAM0B §7.4.9.1): open-drain, pulled up inside to `IOVDD`, driven low while any of the seven
flags is set. **It lands on PD2 on EXTI, falling edge, so in operation the converter is not
addressed at all**: no periodic read, no traffic on the configuration port beside a running
modulator. On the edge the node reads `STATUS` once, clears it, and **marks the capture block the
DMA is writing** — for an event record that is the right granularity: what the record wants to know
is that *this event* was taken under a fault, not which sample it landed on. The pin stays low while
a flag's cause persists, so a flag that cannot be cleared keeps the edge from coming back and is
reported once, not in a loop.

| bit | flag | what it witnesses |
|---|---|---|
| 6 | `ALV_FLAG` | analog supply low voltage — a direct witness on the +5 V rail |
| 5 | `POR_FLAG` | the ADC reset itself: power-on, `IOVDD` brownout, or a commanded reset |
| 4 | `SPI_ERR` | config-port CRC error, enabled by `SPI_CRC_EN` |
| 3 | **`REG_ERR`** | **register-map CRC over 08h–50h**, enabled by `REG_CRC_EN` |
| 2 | `ADC_ERR` | internal ADC error. Read-only, and **only a reset or a power cycle clears it** — the pin stays low until then |
| 1 | `ADDR_ERR` | an invalid register address was used (valid range 00h–50h), enabled by `SPI_ADDR_EN` |
| 0 | `SCLK_ERR` | the config port was clocked a number of cycles that was not a multiple of eight, enabled by `SCLK_CNT_EN` |

**`REG_CRC_EN` is the one to turn on and the reason is the deployment.** The configuration is written
once at boot and the part then runs for months unattended. A flipped bit in the OSR, filter or gain
registers would not stop anything — it would produce plausible, wrong data indefinitely, and nothing
else in the chain can see that. The register-map CRC is the only thing that can.

**Two traps in the boot sequence, and both are silent if missed.**

- **`ALV_FLAG` and `POR_FLAG` reset to 1b**, so `ERROR` is low from power-up. Until boot clears
  them they mean nothing, and the EXTI is armed only after the clear.
- **Three flags block register writes while they are set** — `SPI_ERR`, `ADDR_ERR` and `SCLK_ERR`,
  each once its check is enabled: the SPI CRC (`SPI_CRC_EN`, which this board turns on), the address
  range (`SPI_ADDR_EN`) and the SCLK count (`SCLK_CNT_EN`), which it does not. Only `STATUS` itself
  stays writable. **A single config-port error therefore makes the whole configuration silently
  fail**, so boot must clear `STATUS` *before* it writes anything else, not after.

**The data port's header byte** (`DP_STAT_EN`), one per sample per lane: `PWR_FLAG` (`ALV_FLAG` or
`POR_FLAG`) · `ERR_FLAG` (the `ERROR` pin inverted) · **`MOD_FLAG`, modulator saturation in that
conversion** · `RPT_DATA`, a repeated sample · `PWDN` · `CH_ID[2:0]`. There is no "settled" bit; what
proves a lane's alignment is `CH_ID` matching the lane and `RPT_DATA` clear, and **`MOD_FLAG` is
the hardware word for a clipped sample** (`FIRMWARE.md` §6, §9).

**`CLK_CNT` (03h) is a free check that the ADC is getting the clock the MCU thinks it is sending.**
It counts at `f_CLK / 32`, rolling over, and is enabled by `CLK_CNT_EN`. Here that is 2²⁵/32 = 2²⁰ Hz,
so **in one second it advances 1 048 576 counts — exactly 4096 × 256, so an 8-bit counter read one
second apart returns the identical value.** Two PPS-aligned reads that differ mean the clock is not
what it should be. That exactness is a property of the binary tree: an arbitrary clock would not
come back to itself. **The sheet reads the counter only while the part is converting and with
`SCLK` at `f_CLK / 32` or faster** — SPI1 runs at 2 MHz or more for the read, against 1,05 MHz.

**Three more facts of the sheet that the boot stands on.** The part is reset by `RESET` held low
for **4 `CLKIN` cycles** at least and is ready **104 cycles after the rise** — the interval is counted
in `CLKIN`, so the reset is released and the registers written with `CLKIN` running, never before it
(`FIRMWARE.md` §2). **A write to any register from 08h to 50h restarts every channel and loses the
alignment to `START`**, so configuration happens before `START` and a reconfiguration in service is
a restart, `START` low and high again at the phase edge. **The configuration port is SPI mode 1** —
`CPOL` 0, `CPHA` 1, `SCLK` idle low, data launched on the rising edge and read on the falling.

---

## 5. Clock — phase-locked to the network

**The whole tree is binary, from the TCXO to the sample, and `CLKIN` falls out as a divider.** The
H7A3's core runs at **2²⁸ = 268,435456 MHz** — an integer multiple of the 2²³ network timebase
(×32), and the same 2²⁸ Kronos already runs its own VCO at (`../core/blocks/nodbus.md`). **What
arrives on the pin is the wire's 2²², so the board's own multiplier is ×64** — no 2²³ leaves
Kronos at all (`../kronos/HARDWARE.md`). `CLKIN` is **an eighth of** that core clock, taken off the same PLL's second output:

```
the wire 2²² ──► HSE ──► PLL1 ×128 ──► VCO 2²⁹ ──┬── P ÷2 ──► core 2²⁸
                                                 └── Q ÷16 ──► MCO1 ──► CLKIN = 2²⁵ = 33,554432 MHz
```

**One PLL doing one job.** Tying the core to the timebase costs **4,1 % of the core clock**
against a free 280 MHz and buys a plain integer divider in place of a second synthesiser — which is
also **less jitter on `CLKIN`** (below). Every ADC sample is phase-locked
to network time, so the strike timestamp maps straight onto the global timebase with no resampling.

**The line clock arrives on the HSE bypass input, and what this board needs from it is the
levels.** There is no crystal on this board: the wire carries 2²² = 4,194304 MHz and it enters the
clock-pin switch like every node's does. **DS13195 (Rev 8) Table 47 puts
`fHSE_ext` at 4 to 50 MHz, so 2²² clears the floor by 4,9 %**. What the same table adds, and what
this board needs because **its `VDD` is 1,8 V rather than 3,3 V** (§6), is the receiving end:
`VHSEH` ≥ 0,7 `VDD` = **1,26 V**, `VHSEL` ≤ 0,3 `VDD` = **0,54 V**, and a high or low time above
**7 ns** against the wire's 119 ns half-period. Plain 1,8 V CMOS out of the receiver meets all
three with room; it is the 1,8 V domain that makes them worth writing down.

Pick the multiplier and OSR so `fDATA` is a clean binary submultiple of 2²³ and `fCLK` is inside the
datasheet's window:

**The part divides by two on top of the OSR** — it is a divider in the clock tree ahead of the
modulator (§4), and with `CLK_DIV` = 1 as drawn:

```
f_DATA = f_CLK / (2 · OSR)
```

Read off the datasheet's own noise tables — max speed at 32,768 MHz with OSR 12 gives 1365,3 kSPS,
and OSR 4096 at 3,2 MHz gives 0,39 kSPS. **Both fit only with the factor of two**, and it is easy
to lose.

| CLKIN | from core 2²⁸ | OSR | mode | fDATA | Note |
|---|---|---|---|---|---|
| **33,554432 MHz** (2²⁵) | **÷8** | **16** | **max speed** | **1048576 SPS** (=2²⁰) | **primary** |
| 25,165824 MHz (3 × 2²³) | — | 12 | high speed | 1048576 SPS (=2²⁰) | same rate, 8,4 dB worse |
| 16,777216 MHz (2²⁴) | ÷16 | 16 | high speed | 524288 SPS (=2¹⁹) | half the rate — Nyquist below the band top |
| 16,777216 MHz (2²⁴) | ÷16 | 32 | high speed | 262144 SPS (=2¹⁸) | Nyquist 131 kHz — below the band top, rejected |

**`f_CLK` = 2²⁵ is inside the part's window.** The **32,768 MHz** in the filter tables is the
*nominal*; §5.3
Recommended Operating Conditions gives max-speed mode as **0,5 / 32,768 / 33,66 MHz**, so
2²⁵ = 33,554432 MHz sits **0,31 % under the maximum**. The clock is synthesised from a
crystal-disciplined timebase, so there is no tolerance to eat that margin.

**What the correction is worth, at the same 2²⁰ rate:**

| | OSR 12 at 3 × 2²³ | **OSR 16 at 2²⁵** |
|---|---|---|
| noise · dynamic range · effective resolution | 66,1 µV · 92,8 dB · 16,9 b | **25,1 µV · 101,2 dB · 18,3 b** |
| −3 dB | 238,2 kHz | 238,3 kHz — the same |
| **frame** | 24 bit-times — **no room for the status header** | **32 bit-times = 8 status + 24 data** |
| latency time — sync to the first settled conversion, the datasheet's table scaled to the clock | 5,19 µs | **4,79 µs** |
| the delay of an edge — the sinc4's group delay `2 · OSR − 2` modulator cycles plus the 39-clock pipeline the latency table carries over the impulse response | 3,30 µs | **2,95 µs** — `LAT`'s converter term, `FIRMWARE.md` §5 |
| `DCLK` | 25,166 MHz | 33,554 MHz — under the SAI's 50 MHz slave ceiling (§8) |

**2²⁰ closes two questions at once**: the status header fits because the frame
is 32 bit-times again, and the Nyquist limit moves to 2¹⁹, which is what lets the band run to
**512 kHz** at all. **The wider Nyquist is spent on the band, not on alias margin.**

**So 2²⁰ is the rate** — OSR 16 in max-speed mode off `CLKIN` 2²⁵, primary row of the table above.

**Sinc4 passband, `N` = 16 at `f_DATA` = 2²⁰:** −3 dB at **238,3 kHz** = 0,228 × `f_DATA`,
~−15 dB at the 512 kHz band top. The droop is corrected by the node's FIR; the phase is linear,
so the CFD edge is untouched. **The same curve is the alias filter** — its skirt above `f_DATA`/2
is what the analog chain no longer provides (§2).

**Dynamic range at that point: 101,2 dB** (sinc4, OSR 16, max speed → 25,1 µV RMS on FSR 4,096 V).

### Aliasing — the fold band is medium wave, and the answer is the decimator plus notches

At `f_DATA` = 2²⁰ an interferer at `f` folds onto `|f − 1048576|`, so a band running to 512 kHz
receives its folds from **536,6 – 1560,6 kHz — the whole MW broadcast band** (1035 kHz lands at
~13 kHz, in the low analysis strip; 540 kHz lands at 508,6, in the high one). **The transition
band is gone by construction** (§2): 536,6 kHz is 4,8 % above the band top, so the analog chain
cannot be the answer and is not asked to be. Three things carry it:

- **the decimator's sinc4** — **16,5 dB at 536,6 kHz**, 41,4 at 784,6, 69,2 at 920,6, 95,0 at
  984,6 — above 700 kHz the largest term;
- **the spectrum** — MW is sparse and dying; a realistic site hears a handful of carriers, not a
  wall, and only the ones that fold *into a strip* matter at all;
- **the notches** — a folded broadcast carrier is a fixed, coherent, permanently-on tone on a
  computed 9 kHz raster. One biquad per carrier removes it entirely rather than trading it
  against noise. **This is what the high strip stands on** (§2).

**The `Cd` footprint across `AIN±` stays unpopulated** (§0.2) — fitted, it is a fifth pole at
1,45 MHz and buys 0,6 dB against the first fold, 1,7 dB at 1 MHz; a station under a transmitter takes the attenuation
terminals and the notches instead. **Commissioning records which carriers the site receives**, in
band and folded alike, beside the crosstalk matrix; the notches are set from that list.

### Clock jitter — the datasheet's number, and why it is not this board's

**The datasheet asks for `CLKIN` jitter below 10 ps RMS at a 200 kHz signal frequency**, and `CLKIN`
here is the H7A3's MCO. Aperture jitter becomes noise as `SNR = −20·log₁₀(2π · f · t_j)`, so at the
top of the band:

| at the 512 kHz band top | full-scale sine | the real ceiling — a carrier ≥ 40 dB under clip (§0.4) |
|---|---|---|
| to reach the part's own 101,2 dB | 2,7 ps | — |
| to equal the chain's 28,9 µV floor — a **3 dB** loss | 3,0 ps | **311 ps** |
| to cost the chain 0,5 dB | 1,1 ps | 109 ps |
| to cost the chain 0,1 dB | 0,48 ps | **48 ps** |

**Read against a full-scale sine the requirement is absurd — and no full-scale sine exists.**
Aperture noise is proportional to the amplitude and frequency of what is present, the datasheet's
figure is written for a full-scale tone, and §0.4's own bound says nothing man-made comes within
40 dB of the clip (DCF77 at one kilometre). At −40 dBFS the allowances multiply by 100: **a
PLL-driven MCO in the tens of picoseconds costs the chain under 0,1 dB.** Tesla's spectrum peaks
at the bottom of the band besides, and jitter noise falls with the content's frequency.

**And the source's own number** (DS13195 Table 55): the PLL's cycle-to-cycle
jitter is **±15–20 ps** at our VCO — 4,194304 MHz reference (the 2²² line clock, inside the
2–16 MHz PLL window) × 128 = 536,9 MHz (inside the 128–560 MHz VCO range), ÷2 to the 2²⁸
core. That lands exactly in the "tens of picoseconds" the table above prices at **under
0,1 dB** against the −40 dBFS ceiling.

**The site's spectrum at the top of the band decides it** — the commissioning survey (§0.5): a
site with a strong carrier high in the band reads this table one column to the left, and its
answer is a low-jitter clock buffer fed from the same clock chain.

### One stream, several products — the decimation belongs in firmware, not in the sample rate

**The converter delivers one wide stream and the node makes as many products from it as it has
jobs.** Each job wants a different bandwidth, and narrowing costs nothing but arithmetic:

| product | chain | converter | total | vs the full band | field-referred |
|---|---|---|---|---|---|
| **5–512 kHz** — the CFD edge | 28,9 µV | 36,5 µV | 46,6 µV | 0 dB | **9,1 pT** |
| 5–50 kHz — detection | 22,8 µV | 10,9 µV | 25,3 µV | −5,3 dB | 5,0 pT |
| **10–50 kHz** — amplitude | 17,0 µV | 10,2 µV | **19,9 µV** | **−7,4 dB** | 3,5 pT |
| 19–24 kHz — the SID carriers | 6,4 µV | 3,6 µV | 7,4 µV | **−16,0 dB** | 1,3 pT |

Computed at the built `Rf1` = 4,42 kΩ through the §0.2 chain as drawn — the four corners at 1,1–1,23
MHz, S1's four noise terms (`e_n` × the noise gain against the coil and its 126 pF, `4kT·Rf1`,
`i_n·Rf1`, the 31,0 Ω), S2's own 17 nV/√Hz, and the converter as **51 nV/√Hz**, which is its 25,1 µV
spread over the sinc4's 240 kHz noise bandwidth. **The converter's full-band figure is 36,5 µV and
not 25,1**, because the FIR that flattens the sinc4's −15 dB at the band top lifts the converter's
noise there by the same amount; the field column is likewise taken after the FIR, with the
high-pass corners' loss at the bottom restored. So the converter stands 2 dB above the chain over
the wide band and 4 dB under it in the amplitude product.

**Timing needs the whole band and amplitude does not**, so they do not share a filter: the edge is
timed on stage 0 and the amplitude taken on 10–50 kHz.

**The gain is smaller than the bandwidth ratio, and the reason matters.** 10–50 kHz is a sixth of
the band but only −7,4 dB, because **the chain's own noise sits where the signal sits** — both peak
at the bottom. The converter's contribution does fall with the bandwidth (36,5 → 10,2 µV), because
its noise is white; the front end's does not.

**And none of this wants a faster converter**, for two separate reasons:

- **On timing, a higher rate adds nothing.** The signal is band-limited by the RC chain, so sampling
  above twice the band top carries no further information about it — sinc interpolation reconstructs the
  waveform exactly, and the CFD already interpolates. The timing floor is the rise time (~0,7 µs
  at the 512 kHz band top) and the in-band SNR, not the sample spacing.
- **On amplitude, a higher rate is worth under 3 dB.** Raising `f_DATA` reduces the *converter's*
  noise folded into the band, and at the built gain the converter costs the system **4,1 dB**
  over the full band and **1,4 dB** in the amplitude product (§5's product table) — removing all of it could save at most that, and the floor it lowers is
  under outdoor QRN, which is the bound that matters.

**So the multi-rate architecture is free at the present 2²⁰ and should be built there.** What
follows is a separate argument, and it is about the filter's *shape* rather than about noise.

### How long the frame is — the clock ratio, not `2 · OSR`

**`FSYNC` marks the conversion, so a frame is one conversion period long and the bit-times in it are
the ratio of the two clocks:**

```
bit-times per sample = f_DCLK / f_DATA = f_CLK / (f_DATA · DCLK_DIV) = CLK_DIV · 2 · OSR / DCLK_DIV
```

**Written as `2 · OSR` this is only the `CLK_DIV` = `DCLK_DIV` = 1 case.** It is the case this board
runs, but the general form is the one to reason with, because **`DCLK` is tapped from `f_CLK` ahead
of the ADC clock divider**. That is what the datasheet means by saying `DCLK` may run faster than
the ADC clock: `CLK_DIV` slows the modulator and the frame grows in bit-times while the data rate
falls.

**It buys nothing.** For a fixed `f_CLK` and `f_DATA` the two forms are equal, so the frame length
does not depend on how `CLK_DIV` and the OSR split the work — and raising `CLK_DIV` lowers the OSR
by the same factor, trading bit-times for noise one for one:

| `f_CLK` | `CLK_DIV` | `OSR` | ADC clock | mode | `f_DATA` | bit-times | noise |
|---|---|---|---|---|---|---|---|
| **2²⁵ = 33,554 MHz — AS BUILT** | **1** | **16** | **2²⁴ = 16,777 MHz** | **max speed** | **2²⁰** | **32** | **25,1 µV** |
| 3 × 2²³ — the rejected clock, kept only to show the trade | 1 | 12 | 12,583 MHz | high speed | 2²⁰ | 24 | 66,1 µV |
| " | 2 | 12 | 6,291 MHz | mid speed | 2¹⁹ | 48 | 65,3 µV |
| " | 1 | 24 | 12,583 MHz | high speed | 2¹⁹ | 48 | **10,3 µV** |

**The built row is why the frame is 32 bit-times.** `CLK_DIV · 2 · OSR` = 1 · 2 · 16 = 32 = 8 status
+ 24 data, which is the whole reason OSR 16 was worth the faster clock: at OSR 12 the frame is
**24** bit-times and the status header does not fit (§5). The three rows below it are the OSR 12
route and are here for the trade they illustrate, not as settings this board offers.

`DCLK_DIV` only divides, so it can only shorten the frame. **A 32-bit frame at 2²⁰ needs `f_CLK` = 32 × 2²⁰ = 2²⁵ = 33,554432 MHz — which is
where the board runs (§0.3): 0,31 % under the max-speed ceiling.**

*(TI's "effective resolution" figure is `log₂(FSR_pp / e_n)` and the dynamic range is
`20·log₁₀(FSR_pp/2√2 / e_n)`, so the two differ by a fixed 9,03 dB — 1,5 bits. It is not ENOB from
SINAD and the two must not be compared across a table.)*

**2²⁰ is the built rate, and what it bought is the band.** OSR 16 at `f_CLK` = 2²⁵ reaches 2²⁰
with the full 101,2 dB and the 32-bit frame (status header included), Nyquist lands at 2¹⁹, and
the band runs to **512 kHz** under it — rise ~0,68 µs, the winding resonance passivated by the virtual short (§2).
The sinc4 droop across the band is a calibration constant in the node's FIR; the MW folds are
carried by the decimator's sinc4 and the notches (§0.4).

---

## 6. Rails — what this board needs, downstream of the connector

**The unit power board delivers 12 V and nothing lower** (`../galvani/HARDWARE.md`, *The low
rails*) and this board makes every rail from it. The doctrine: **the processor's rail is never
switched** — a processor that cuts its own supply cannot be woken — so no regulator on this
board has its `EN` on a pin, exactly as 3,3 V is never cut on the other units. **Off is the
source end's**: `ENABLE` at the source power board takes the whole feed away and every rail
here with it.
**Three `TPS629206` straight off the 12 V, no cascade**: one makes **5,3 V** at 2,5 MHz, one the
processor's **1,8 V** at 1 MHz — 12 → 1,8 V is 150 ns of on-time there, 122 ns at the 14,7 V the
transil lets through, against the part's 40 ns minimum — 49 ns at 2,5 MHz, too close — and one the **3,3 V** interface rail at 2,5 MHz; all three in
forced PWM (`../galvani/HARDWARE.md`, *The buck cell*). The three `TPS7A4701` stand on the same 5,3 V (0,3 V of
headroom, enough at their ~100 mA). **The 3,3 V is the interface rail only** — the translators' 3,3 V side,
the `PCA9306`, the communication boards on `NB IN` and `TIME OUT`, the power board's `ISO1642` on `PWR IN` — and it is its own `TPS629206` in
forced PWM, because at 5 mA on a copper end a power-save buck's burst rate would land in the
band. **No rail is switched from a pin**: every regulator runs whenever the 12 V is there, and a
sleeping board puts its parts down by their own means — the converter's `START` and standby
register (`../galvani/README.md`, *Three states*). **The 4,0 V does not exist on this board** —
nothing on it wants feedstock.

| rail on board | made by | from | feeds | switched |
|---|---|---|---|---|
| **5,3 V** | **`TPS629206`**, forced PWM at 2,5 MHz, `MODE/S-CONF` 9,31 kΩ, `R1` 1,07 MΩ / `R2` 137 kΩ → 5,286 V | 12 V | the three 5 V LDOs | never |
| **5,0 V analog** ×3 | TPS7A4701 ×3 | 5,3 V | ADC `AVDD1` and the `REF6041` (their own regulator) · S1+S2 of the channels (two regulators, channels split behind shielded 2,2 µH inductors) | never |
| **1,8 V digital** | **`TPS629206`**, forced PWM at 1 MHz, `MODE/S-CONF` 22,1 kΩ, `R1` 274 kΩ / `R2` 137 kΩ → 1,800 V | 12 V | **the whole MCU — `VDD` and `VDDA` alike**, `VREF+` strapped to it (§7), and **`VDDA`/`VREF+` through the station's shielded 2,2 µH `SWPA252012S2R2MT` into 10 µF + 100 nF** — the buck's own 1 MHz on an ADC reference is the inductor's position — and **ADC `IOVDD` through a second shielded 2,2 µH into 10 µF + 100 nF, then the bead `GZ2012D301TF` into 100 nF at the pin** — a digital supply whose 33 MHz clock is under the inductor's 69 MHz self-resonance and whose edges' harmonics are above it (`../core/POWER.md`, *The filter parts*); ≤ 5 mA, digital interface and core, no regulator of its own | **never** |
| **`AVDD2`** | no regulator — the 1,8 V through the shielded 2,2 µH into 2× 10 µF + 100 nF (~24 kHz, Q ~2), then the bead `GZ2012D301TF` into 100 nF at the pin | the 1,8 V | ADC `AVDD2`, ≤ 21 mA | never |
| **3,3 V interface** | **`TPS629206`**, forced PWM at 2,5 MHz, `MODE/S-CONF` 9,31 kΩ, `R1` 619 kΩ / `R2` 137 kΩ → 3,311 V | 12 V | the 3,3 V side of the two `SN74AXC8T245` · the `PCA9306` · the communication boards on `NB IN` and `TIME OUT` · the power body's pin 8 (**18 mA copper** — its line side is 5 mA, the island ~50 % at that load and three transceivers add their `ICC1`, `../galvani/HARDWARE.md` — ~55 mA on glass at a far end, 125 mA at most). **Nothing on the MCU touches it** | never |

**Ranging:** the return is this board's timer — `RXD` and `TXD` on two channels of one timer,
`RXD` on CH1 or CH2, the pin choice at CubeMX (`../bifrost/HARDWARE.md`, *Ranging*).

- **All three bucks run in forced PWM, at 1 and 2,5 MHz, above the 512 kHz band, and never in
  power save**, whose burst rate would fall with the load into the band; the `MODE/S-CONF`
  resistor fixes the mode and the frequency at start-up. What
  keeps those lines out of the chain is distance and layout, not a frequency trick
  (`../galvani/README.md`, *Converters and measuring boards*).
- **The bench criterion**: the spur coupled into the chain input stays under **65 mV**, so the
  folded product sits under the noise floor of a 16 kHz analysis strip. The layout clears it by
  orders.
- **The whole MCU at `VDD` = 1,8 V** — every pin is a 1,8 V pin and the ADC interface needs no
  translation. **Verified against DS13195
  (Rev 8) §3.5, and the combination is legal with one condition:**
  - **`VDDA` is `VDD`** — one buck, one node, `VREF+` strapped to it. DS13195's power-up
    condition (while `VDD` < 1 V every other supply must stay under `VDD` + 0,3 V) is then
    satisfied by construction and there is **no ordering problem, no `PG` gate and no load switch
    on the analog branch**. **The ADC is legal there** (DS13195 Table 93): `VDDA` 1,62–3,6 V with
    the ADC on, and below `VDDA` = 2 V the sheet requires `VREF+` = `VDDA` — which is the strap.
    Four thermometers at a few readings a minute want nothing of the ADC's full grade.
  - **Fast IO at 1,8 V is the HSLV regime** (§3.8): required for PSSI/OCTOSPI-class speeds below
    2,7 V, enabled by **the `VDDIO_HSLV`/`VDDMMC_HSLV` option bits AND the `SYSCFG` `HSLVx`
    bits** — half of it is an option byte, i.e. a **provisioning step at programming, not a
    runtime call**. The pad ceilings against `DCLK` 2²⁵ are in the I/O AC table, read below.
  - **The BOR option byte stays DISABLED** — its thresholds run 2,1–2,7 V, all above the 1,8 V
    rail; enabled, the part would never leave reset. POR/PDR supervise; PVD optional.
  - **`VCORE` comes from the internal LDO, not the internal SMPS** — the SMPS's 1,8 V output
    demands `VDDSMPS` ≥ 2,3 V, which a 1,8 V `VDD` (= `VDDSMPS`) cannot give. The LDO runs from
    1,62 V up; do not let a CubeMX default select the SMPS. **`VCAP`: 2,2 µF ±20 % on each of the two pins, ESR < 100 mΩ**
    (Table 25) — 2× 1 µF 50 V 0805 a pin.
  - **The core scale is VOS0, and it is the only one that reaches 2²⁸** — VOS1 stops at
    225 MHz. VOS0's conditions: **`VDD` ≥ 1,71 V** and **`TJ` ≤ 105 °C** (Table 24). Both hold,
    the first one tightly: the 1,8 V rail stands **5 % above the floor**, and the POR release
    threshold reaches the same 1,71 V from below — so the buck's tolerance *plus its load-step
    transient* must never dip under 1,71 V. A ±1 % part clears it, and the output is **2× 10 µF
    X7R 0603 with `C_FF` 120 pF** — the sheet's own recommendation (SLVSDV6C §9.2.2.6), whose
    Figure 45 steps 50 mA to 1 A for a dip of ~30 mV: 1,782 V at the feedback's low corner less
    30 mV is **1,752 V, 42 mV over the floor**, and this rail's real step — the MCU's peripherals
    coming on, ~60 mA — is a sixteenth of the sheet's. `TJ` 105 °C stands 20 °C over the enclosure's
    ambient ceiling. `VDD`'s fall rate must stay ≥ 10 µs/V (Table 28 — shapes the bulk
    discharge, not the operation).
  - **Every kernel clock clears its VOS0 ceiling**: CPU/AXI/AHB 280 MHz (ours 268,4 — 4 %
    under), PSSI 100 MHz receive with `PDCK`/`f_HCLK` ≤ 0,4 (Marconi's 2²⁶ passes), OCTOSPI 280
    (Marconi's 2²⁷ passes on the kernel side), SPI 280, **SAI kernel 150 MHz — `DCLK` 2²⁵ has
    4,5× under it**, the slave-mode bit-clock ceiling read below, internal ADC 50 MHz.
  - **The pad side holds too** (Tables 68/69): at 1,62–2,7 V, OSPEEDR 11, a pin drives
    **66 MHz into 30 pF with HSLV off and 90 MHz with it on** (175 MHz into 10 pF) — the MCO's
    2²⁵ = 33,6 MHz CLKIN output clears it either way, and every other output on this board is
    slower. The OCTOSPI table closed Marconi's PSRAM clock at **2²⁶** (octal DDR caps at
    120 MHz — `../marconi/HARDWARE.md` carries the consequence). **And the SAI table closes the
    last word**: slave receiver takes **50 MHz** at any `VDD` in range, so `DCLK` 2²⁵ =
    33,55 MHz passes with a third of margin; its 1 ns setup / 3 ns hold are met
    source-synchronously (data moves on the falling `DCLK` edge, reads on the rising — half a
    period of 14,9 ns on either side), and the APB ≥ 2 × `fCK` footnote holds at 140 vs
    67,1 MHz. **Nothing in the 1,8 V scheme is left unverified against DS13195.**
- **The station interface stays 3,3 V** — the Galvani bodies are 3,3 V logic on every board in
  the station and do not change for a board whose processor runs 1,8 V. **Every one-way line
  crosses a direction-set translator, `SN74AXC8T245`, two packages — one inbound, one outbound —
  `DIR` strapped, no processor pin**; the I²C crosses a **`PCA9306`**, 4,7 kΩ on each side, to
  the power board's `ISO1642`, which does not take 1,8 V. `ID` needs nothing — a ratio against the
  board's own 1,8 V. The lines, the straps and the pulls are §8, *The Galvani sockets*. An
  auto-direction translator is not used on a clock line: its one-shot accelerators and pass-gate
  pull-ups round the edges into series resistance and cable capacitance.
- **NON-isolated.** The data port needs a shared ground, so the analog section cannot be
  galvanically isolated from the digital corner; isolation lives on the spur, where it crosses a
  ground domain. Grounding: one solid analog ground, AGND↔DGND meet at a single star point under
  the ADC.
- **Thermal: nothing is interesting.** The LDOs drop 0,3 V across tens of mA; the bucks run
  ~80–85 % from 12 V. **The board's biggest single consumer is the MCU, and its anchor is DS13195
  Table 34**: at VOS0/280 MHz the part draws 69,5 mA typ with peripherals off and 133,5 mA with
  everything on (106/173 mA max at TJ 85 °C) — expect **~75–130 mA on the 1,8 V rail** at 2²⁸
  with the real peripheral set. **The rail-by-rail table**, at the sheets' maxima (DS13195
  Table 34 at 85 °C; ADS127L14 SBASAM0B §5.5, max-speed, four channels, buffers on; THS4551
  `I_Q` 1,92 mA over temperature):

  | rail | what it carries | worst |
  |---|---|---|
  | **1,8 V digital** | MCU 173 mA · ADC `IOVDD` 5 mA · ADC `AVDD2` 21 mA · PSRAM ≤ 20 mA while it is written | **≤ 219 mA, 394 mW** — 37 % of its `TPS629206` |
  | **5,0 V `AVDD1`** | the converter's core 8,1 mA · eight input buffers 16,8 · four reference buffers 6,8 · `VCM` 0,1 · the `REF6041` 1,1 | **≤ 33 mA, 165 mW** |
  | **5,0 V fronts**, two LDOs | six `THS4551` at 1,92 mA · S1's full swing into the ladder, 4,6 mA a channel | **≤ 26 mA, 130 mW** |
  | **5,3 V** | the three LDOs 59 mA | **≤ 59 mA, 0,31 W** — 10 % of its `TPS629206` |
  | **3,3 V interface** | the communication board — **18 mA on copper, ~55 mA on glass, 125 mA at most** · the translators and the `PCA9306`, ~1 mA | **~20 mA copper, ≤ 126 mA glass** |
  | **the 12 V** | the three `TPS629206` at ~80–85 % | **~1,1 W on copper, ~1,5 W on glass** |

  Inside the measuring-unit class of 3 W either way, and the thermal line above holds: the
  three LDOs drop 0,3 V across 59 mA, 18 mW.

## 7. Temperature — four NTCs, read ratiometrically by the H7A3

μ(T) amplitude correction is **digital**, done on the node's H7A3 — no analog ferrite compensation.
Timing is CFD (amplitude-independent), so μ(T) drift never moves the edge; only amplitude is
corrected; one NTC per rod because the frustum's faces see different sun (`CONSTRUCTION.md`).

**The sensor is passive because the receiver is a wideband magnetic front.** A 1-Wire part signals in
60–120 µs slots — a fundamental of 8–16 kHz with GPIO-fast edges, i.e. an **impulsive** emitter
inside the band, which is the one class the interference discriminator cannot reject (it screens
narrowband/periodic). At ~0,7 mA a single un-twisted wire is ~14 nT at 1 cm and still ~0,9 nT at
15 cm — the amplitude of a real sferic. **An NTC is a resistor**: no clock, no edges, no active
device at the rod at all, so nothing has to be blanked and there is no dead time.

| | |
|---|---|
| part | **10 kΩ NTC, B 3380 K, ±1 %** — the station's two codes: the leaded **`NXFT15XH103FA2B`** on the three rods, the 0603 **`NCP18XH103F03RB`** on the board (`../marconi/HARDWARE.md`, `../quark/scintillation/photon/HARDWARE.md`); in a divider with a **10 kΩ** fixed resistor at the board |
| count | **4** — one per rod (3), plus one on the PCB for the electronics' own temperature |
| the divider | fixed 10 kΩ from `VREF+` to the ADC pin, **NTC from that pin to `VSSA`**. Two wires per rod instead of three: the NTC needs no supply of its own |
| **ratiometric — the reference does not enter** | both legs sit between `VREF+` and `VSSA` and the ADC reads their ratio, so `VREF+`'s value, its tempco and its tolerance all cancel **exactly**. There is no reference part, no `VREFINT` correction and no rail to keep quiet — which is why `VREF+` is simply strapped to `VDDA` = **1,8 V** (§6) and the number is never used |
| what the divider covers | 10 kΩ against an NTC that runs ~196 kΩ → 10 kΩ → 3,0 kΩ across −40 / +25 / +60 °C (the part's own table), so the reading spans **0,95 → 0,50 → 0,23 of full scale** — three quarters of the range. The slope is **~40 LSB/°C at 25 °C and worst at the cold end**, where the divider is compressed: **~12 LSB/°C at −40 °C** at 12 bit before any oversampling. **The series resistor is the window**; nothing is gained by narrowing the converter's |
| self-heating | worst case is the matched point, `R_NTC` = 10 kΩ: 90 µA and **81 µW**. Against a bead's 1–2 mW/°C that is **under 0,1 °C**, so the divider stands permanently — no gating, no settling wait. All four together draw ≤ 0,6 mA |
| supply | **none — it is `VREF+`**, the 1,8 V the processor runs on |
| output | **3 kΩ + 100 nF at the ADC pin** — the cap is the charge reservoir for the SAR, so the sampling transient is served locally and the divider's source impedance never has to: one conversion shares the ADC's 4 pF `C_ADC` (DS13195 Table 93) with 100 nF, 4·10⁻⁵ of the reading, and the pin recovers through ≤ 12,5 kΩ — 3 kΩ plus 10 kΩ ∥ the NTC at −40 °C — in 1,25 ms. **What bounds it is the average draw**: 4 pF × 1,8 V × the conversion rate through ≤ 12,5 kΩ stays under one 12-bit LSB (0,44 mV) below **4,9 kSPS** — the read below runs at 2,56 kSPS, 0,23 mV |
| read | hardware oversampling **256×**, spread evenly over **100 ms** — five periods of 50 Hz and six of 60 Hz, so averaging across whole mains cycles nulls the mains and its harmonics on a cable running beside a magnetic receiver on either grid. At a few readings a minute the resolution is free and is not the limit |

**A ratiometric divider is what deletes the whole reference question**, and with it a rail, a part
and a firmware correction: an analog sensor outputs volts and is only as good as `VREF+`, while a
resistive divider outputs a *fraction*, and the converter measures fractions. An NTC has no supply
pin, so two wires run to each rod.

**Mounting.** The bead is the thermal path entire — glue it to the rod and run both leads along
the rod before they leave, or the reading drags toward air. The bead and its leads are
conformal-coated — class SR to IPC-CC-830 / IEC 61086, neutral cure, flexible at −40 °C — because
condensation over −40/+60 °C shunts a 10 kΩ leg and reads as heat. **The wiring must never form a turn around a
rod** — that is a shorted secondary; it damps the rod, moves L and Q, and its own drift lands in
the calibration. Twisted pair, along the axis, out at the end.

**Absolute accuracy is not a requirement**, which is what makes an NTC's curve tolerance a
non-issue: no temperature rides an event — the record is 4 B with no temperature field, and
the board's NTC leaves only in the minute's `REPORT` frame; a rod's reading only ever enters as a correction curve measured per unit at characterisation, which
absorbs both offset and curve. What is needed is repeatability, and a resistor bonded to a rod
has that. The fixed leg is the one part whose tolerance does *not* wash out in characterisation
if it drifts, so it is the one to buy at grade.

**The package constrains nothing here** — `VREF+` is `VDDA`, so the small packages that tie the
two internally would do as well; the LQFP176 brings the pin out and it is strapped. The 10 µF + 100 nF on
the `VDDA`/`VREF+` node stay: they serve the conversion transients locally.

### Behaviour of the electronics over −40 / +60 °C

Worked from the datasheet **worst-case (Tmax) columns, not the 25 °C "typ"** — the standing rule for
this project: leakage / FET input bias roughly **doubles every ~10 °C**, so a headline typical number
is meaningless at the extreme.

- **Gain (Rf2/Rg):** a resistor *ratio* — the tempcos cancel → drift in ppm → effectively **zero**.
- **Band-pass corners (RC):** C0G ±30 ppm/°C + resistors ~100–150 ppm/°C → the RC product drifts
  ~1–1,5 % over the whole range → the LP corner 1,2 MHz ± ~15 kHz → **negligible**, and doubly so
  with the corner an octave above the band.
- **Cin (HP 4,8 kHz):** X7R drifts ±15 % → the corner wanders 4,3–5,8 kHz. Harmless (it only kills
  mains); use C0G if you want it pinned.
- **Op-amp Vos / I_B:** sub-mV over 100 °C and the band-pass strips DC anyway → **irrelevant** to the
  AC signal (and the loop is low-Z, so bias current never bites).
- **The one live variable is the ferrite, μ(T)** — it moves *amplitude*, corrected digitally
  (the rod sensors + μ(T) on the H7A3); CFD timing is amplitude-independent, so the *edge* (⇒ position) never
  moves.

**Net:** the electronics are temperature-robust across −40/+60 °C; the only temperature-sensitive
element is the antenna core, handled in software.

---

## 8. The ADC → MCU path (on-board routing)

### The data path is serial

The `ADS127L14` has no parallel port — only SPI and the four-lane frame-sync port — so the
converter's words reach the H7A3 over **SAI**, and the PSSI sits idle on this board.

### The PSRAM is a population, and on Tesla it is recommended

The event records, the SID logs and the running baseline are written slowly and read rarely, and
they fit the H7A3's 1,4 MB beside the 120 kiB capture history with room to spare. **What does not
fit is the learning**: the classifier learns its background on the node and the training set for
the rare classes is collected there (`DETECTION.md`), and both want a deep buffer. **So Tesla fits
the OCTOSPI PSRAM as a recommendation** — the same `APS25608N-OBR-BD` as Marconi — and a build
that runs the fixed detector without the learned classifier may leave it open.

**The `APS25608N` is a position and fitting it is the build's choice**: fitted, the learned
classifier has its model and its training buffer; open, the rule set runs and nothing else
changes.

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
| when it runs | on this board while the learned classifier loads and collects — a population, above; Pip's image leaves it idle |

### The DMA budget, and where the buffers are allowed to live

**The counts come from `stm32h7a3xx.h`, so they are the part's, not an estimate.** The H7A3 carries
**16 `DMA1`/`DMA2` streams**, 16 BDMA channels (`BDMA` + `BDMA2`), 16 MDMA channels and both
`DMAMUX1` and `DMAMUX2` — and the mux is why no stream is reserved to a peripheral by wiring. **The BOARD
spends eight to nine of the sixteen and Tesla's own image spends six** — three SAI blocks, two for
the spur UART (transmit and the echo receiver) and one for the internal ADC that reads the four
temperature sensors; **`TIME OUT`'s two are Pip's image and the config SPI's one is spent only if it is not
polled**, which is where eight to nine comes from. **The NOD count adds none of them**: three NODs
are three frames out of the one USART on the one transmit stream. The budget closes with most of a
controller spare.

**The buffers cannot sit in DTCM, and that is a bus-matrix fact rather than a preference.**
`DMA1`/`DMA2` are D2-domain masters and reach neither ITCM nor DTCM; only MDMA reaches them. So the
capture ring lives in **AXI SRAM at `0x2400 0000`** — 1 MB in three contiguous blocks — while DTCM
holds the detector loop and its state, which is the split §5 already describes and the only one the
silicon allows.

**That places one item no estimate above has counted: the AXI SRAM is cached, and DMA writes into it
are not visible to the core until the line is invalidated.** Two ways out, and the cost of each is
arithmetic rather than a measurement. **The ring is a non-cacheable MPU region** (`FIRMWARE.md` §2, step 1):
it costs nothing per transfer and gives up cached reads of the buffer, which the detector's
running state in DTCM does not need. The alternative, **invalidation per half-transfer**, would keep the cache
and cost one pass over the ring's lines — the Cortex-M7 line is 32 B and the real half is **three
rods × 5120 samples × 4 B = 60 kiB, so 1920 lines**: a few thousand cycles, about **15 µs against the
4,9 ms the half lasts**, or 0,3 % of the core. Small, but not free, and it buys nothing here.

**Interrupt latency is a deadline here, not a precision term, and the margin is two and a half
orders.** The event time is the **sample index**, not the moment the handler ran — the converter is
locked to the network clock and the record carries ticks of 2⁻²⁰ s back from the frame's start — so
a late handler cannot move a timestamp. It only has to finish inside the ring, which is **10 ms**
deep against a handler in the tens of microseconds.

**The data path is the `ADS127L14`'s frame-sync data port — one `DOUT` per rod — taken on SAI.**
The port's signal set is `DCLK` + `FSYNC` + n data lines, which is **TDM**, the shape audio codecs
present and what the SAI peripheral exists for. **No per-sample interrupt** — the port streams
continuously into DMA ping-pong and the CPU sees block callbacks — and **no software
de-interleave** — each rod lands contiguous in its own buffer, which the per-rod detector reads
directly. **SAI sub-blocks share the clock and the frame sync**, so the port costs one `SCK`, one
`FS` and three `SD`, doubled on the second peripheral's pins; all six SPIs stay free, one for the
configuration port and five spare.

**The instances, from the vendor's device header** (`stm32h7a3xx.h`, ST's CMSIS): the part carries
**`SAI1` and `SAI2`, each with a block A and a block B — four audio blocks**, so the three rods take
three and one is spare. The slave bit-clock ceiling is **50 MHz** against `DCLK` = 33,554432 MHz, a
third of margin. The three are **`SAI2` block A and `SAI1` blocks A and B** (*Pins*, below); `SYNCIN` ties the two peripherals together in configuration.

**Four lanes is `DP_TDM[1:0]` = 11b, and three things follow from it that the board needs.**
It is also the configuration with the most room per channel: at two lanes each lane carries two
channels and needs twice the bit-times, so fewer lanes never relieves the frame.

- **Daisy-chaining is not offered in the four-lane mode**, so no `DIN` is in use; pins 10–13
  (`GPIO4`, `GPIO5`, `DIN1`, `DIN0`) still carry 100 kΩ to `DGND`, as TI's own layout does, and
  `DOUT3`, the spare channel's lane, is not connected (§4, *The pins*).
- **The channel-ID bits in the status byte check the lane**, because the lane *is* the rod: `CH_ID`
  matching it and `RPT_DATA` clear prove the alignment, and `MOD_FLAG` marks a clipped sample
  (§4) — the header is what frame-sync mode has in place of a `DRDY`.
- **A powered-down channel keeps its slot with frozen data.** The frame does not shorten and the DMA
  geometry does not change, so **Pip — this board under its own image — runs the identical data path
  and the identical buffer layout** (`pip/`).

**Every SAI block is a slave and none of them is the master.** `DCLK` comes from the converter, so
the blocks' `SCK` is tied to it and nothing on the MCU generates a bit clock. The config port is
separate on its own four wires (§4) — it drives nothing that the data port sees.

### Nothing is re-sorted, and no sample is moved in memory

**The CPU touches no sample on the way in.** Each slave has its own circular DMA channel writing
32-bit words into its own contiguous buffer, so the three rods arrive already separated and the CPU
sees block callbacks, never a sample. The traffic is 3 × 2²⁰ × 4 B = **12,6 MB/s** into AXI SRAM,
which that bus does not notice.

**And no sample is re-packed on the way out.** The frame is stored as it arrives, 32 bits —
**`[8 b STATUS_DP][24 b data]`, the header leading** (SBASAM0B §7.4.10.3), two's complement, MSB
first — and the 24 data bits are taken at the point of use by one `SBFX` (bits 23..0,
sign-extended), folded into the load the filter loop already performs. **There is no repack pass and no second buffer.** Packing to three
bytes would save a quarter of the buffer (120 → 90 kiB for the 3-rod ring) and cost a touch of every
sample to do it, which is the trade the wrong way round. **Store 32, read 24, move nothing.**

*(The split also quarters the per-line rate — 2²⁵ ≈ 33,5 Mb/s a lane instead of ≈134 on one, which no SAI takes — but that is a
by-product, not a reason: on one board those edges live on an inner layer between ground
planes. The two software reasons carry the decision on their own.)*

| Signal | Dir (ADC→MCU) | Note |
|---|---|---|
| `DCLK` | out | data-port bit clock **33,554432 MHz = 2²⁵** — `f_CLK / DCLK_DIV` with `DCLK_DIV` = 1, i.e. `CLKIN` itself. The 32 bit-times per frame follow from `f_CLK / f_DATA` (§5), not from the word length. **Data changes on the falling edge and the host latches on the rising** — the SAI clock strobe is set to match. **Closed against the datasheet:** the SAI slave-receiver ceiling is 50 MHz, 1,5× over this rate |
| `FSYNC`| out | frame sync → the SAI blocks' `FS` (common); one `FSYNC` period is one conversion period = 32 bit-times = the 8 b status header, then 24 b data |
| `DOUT0..3` | out | one lane per rod → the SAI blocks' `SD`. **`DP_TDM[1:0]` = 11b**, the four-lane mode |
| `SCLK_C / DIN_C / DOUT_C / CS_C` | in/out | ADC config SPI port — **SPI1** on the H7A3, a real port, boot only (*The processor*, below) |
| ~~`DRDY`~~ | — | **does not exist in frame-sync mode**: DRDY is an SPI-data-mode function; the frame-sync port signals settled-data **in-band** (DOUT shifts zeros until settled + status bits in the lane frame). No wire. |
| `RESET / START` | in | H7A3 GPIO — PD1 · PD0 (*The processor*, below) |
| `CLKIN`| in | H7A3 MCO1 — PLL1_Q, VCO 2²⁹ ÷ 16, an eighth of the core 2²⁸ → **2²⁵ = 33,554432 MHz**, phase-locked (§5) |

### The whole ADC-facing side is 1,8 V — and so is the whole MCU

IOVDD is 1,8 V CMOS (§4), and since the power rework the **H7A3's `VDD` is 1,8 V too** (§6):
every pin is a 1,8 V pin, the ADC interface needs no domain, no VDDIO2 gymnastics and no
translators.

What 1,8 V costs instead is **pin speed** — STM32 IO derates at low `VDD` — and the fast lines
here are `DCLK` = 2²⁵ and the data lanes. **Closed in §6 against DS13195**: a pad at 1,62–2,7 V
takes 66 MHz into 30 pF with HSLV off and the SAI slave receiver 50 MHz, so 2²⁵ = 33,554432 MHz
passes on both counts and `DCLK_DIV` stays at 1.

The 3,3 V station interface crosses the direction-set translators (*The Galvani sockets*, below); `CLKIN` is the MCO pin at 1,8 V
directly.

**33 Ω series at every driver on the fast lines** — house practice, not a special case here.
Route them on an inner layer between ground planes; the whole point of one board is that these
edges never reach a connector.

### Placement — the digital corner and the Galvani socket

One board carries a 5–512 kHz magnetic receiver and a 33,6 MHz digital bus, so the separation
between them is bought with **layout** rather than metalwork: the H7A3, the ADC's digital side
and the fast lanes sit in their own corner, behind a ground moat, as far from the three front-ends as the outline allows.

**The Galvani port — the two data bodies, the power body and the two 12 V terminals** — belongs
at the far edge. The unit power board behind it is a wide-input
switcher, and the power doctrine already names this board
as one of the two where switcher noise has to be paid for.
Putting the socket on the opposite edge from the front-ends keeps that converter, its inductor
field and its cable entry out of the analog section — and because the module plugs in rather
than being built in, it can be moved without touching this board.

### The Galvani sockets — `NB IN`, `PWR IN`, `TIME OUT` and the 12 V terminals

**Connectors: `NB IN` · `PWR IN` · `TIME OUT` · `ROD X` · `ROD Y` · `ROD Z` · `12V`** (`../galvani/README.md`, *Connector names*).

**Tesla is a measuring unit and stands outside the enclosure, always behind a unit power board,
so every socket on this board is a unit-end socket**: what sits on the Galvani board runs from the
moment the feed arrives, held there by resistors, and no processor pin switches anything on it.
**`NB IN`** is the NodBus data body, **`PWR IN`** the power body, **`TIME OUT`** the second data body — Pip's time
port toward Kronos, populated on every board, served by Pip's image and left idle by Tesla's —
and **`12V`** the 12 V terminals. The bodies are `BX2.54-2xNA` shrouded headers, 2×6 for `NB IN` and
`TIME OUT`, 2×4 for `PWR IN`.

**Every one-way line crosses a direction-set translator; the I²C crosses a `PCA9306`.** The
processor is 1,8 V and the bodies are 3,3 V logic. Two `SN74AXC8T245` — 1,8 V on `VCCA` from the
processor's rail, 3,3 V on `VCCB` from the interface rail, `OE#` to ground, **`DIR` strapped and no
processor pin on it** — one package a direction, because an 8-bit part has one `DIR` for all
eight lines. Both supplies are isolated inside the part, so either rail rising first leaves the
outputs high-impedance and nothing is sequenced.

| part | `DIR` | lines | unused |
|---|---|---|---|
| **`U25`**, inbound, B → A | to ground | `NB IN`: `CLK`, `RXD`, `RXD_ECHO`, `SD` · `PWR IN`: `ALERT` · `TIME OUT`: `RXD`, `SD` — seven | one; its B input to ground |
| **`U26`**, outbound, A → B | to the 1,8 V | `NB IN`: `TXD`, `DE` · `TIME OUT`: `TXD`, `DE`, `CLK/PPS` — five | three; their A inputs to ground |
| **`U27`** `PCA9306` | — | `PWR IN`: `SDA`, `SCL` — `VREF1` on the 1,8 V, `VREF2` and `EN` joined and 200 kΩ to the 3,3 V; **4,7 kΩ to 1,8 V on the processor side and 4,7 kΩ to 3,3 V on the socket side**, on each line | — |

**No translator input floats.** Every inbound line carries **100 kΩ to ground on the 3,3 V side**,
so an empty socket — `TIME OUT` on every Tesla, and any socket on the bench — reads a defined low: `SD`
quiet, `ALERT` quiet, `RXD` and `CLK` dead. Every outbound line carries 100 kΩ on the 1,8 V side
for the processor's reset, when its pins are high-impedance: **`DE` and the PPS to ground** — the
driver off and no edge — **`TXD` to the 1,8 V**, a UART's idle.

**`NB IN` — the NodBus data body.**

| pin | signal | on this board |
|---|---|---|
| 1 | `CLK/PPS` | **in** — the 2²² wire clock, through `U25` onto PH0 `OSC_IN`, HSE bypass; 100 kΩ to ground on the 3,3 V side |
| 2 | `GND` | the ground plane of the digital corner |
| 3 | `TXD` | PB6 `USART1_TX` / `TIM4_CH1`, through `U26`; 100 kΩ to 1,8 V at the pin |
| 4 | `RXD` | through `U25` onto PB7 `USART1_RX` / `TIM4_CH2` and PB3 `TIM2_CH2`; 100 kΩ to ground on the 3,3 V side |
| 5 | `ID` | PC0 `ID_D`, `ADC12_INP10`; **10 kΩ 1 % to `VREF+`**, the 1,8 V — no translator, the reading is a ratio |
| 6 | `ID_RET` | **not connected** — a measuring unit never meets a crossed cable |
| 7 | `RXD_ECHO` | through `U25` onto PD9 `USART3_RX`; 100 kΩ to ground on the 3,3 V side |
| 8 | `DE` | PE2, through `U26`; 100 kΩ to ground at the pin |
| 9 | `B_DIR` | **tied to ground** — this end listens on channel B, the clock arrives; no processor pin |
| 10 | `LINE_EN` | **10 kΩ to 3,3 V** — the line side runs whenever the unit does; no processor pin |
| 11 | `SD` | through `U25` onto PD10; 100 kΩ to ground on the 3,3 V side — a copper board leaves it unpopulated and it reads quiet |
| 12 | `3,3 V` | the 3,3 V interface rail, `U24` |

**`PWR IN` — the power body.** One power socket on the board, so its `INA238` is 0x40.

| pin | signal | on this board |
|---|---|---|
| 1 | `ENABLE` | **not connected** — the unit power board pulls its own `ENABLE` to its input |
| 2 | `GND` | the ground plane |
| 3 | `ID` | PC1 `ID_P`, `ADC12_INP11`; **10 kΩ 1 % to `VREF+`** |
| 4 | `SDA` | PB9 `I2C1_SDA` through `U27`; 4,7 kΩ to 3,3 V on this side of it |
| 5 | `A_SEL` | **straight to ground** — 0x40 |
| 6 | `SCL` | PB8 `I2C1_SCL` through `U27`; 4,7 kΩ to 3,3 V on this side of it |
| 7 | `ALERT` | through `U25` onto PC8, GPIO on EXTI; **100 kΩ to ground on the 3,3 V side, high = alarm** |
| 8 | `3,3 V` | the 3,3 V interface rail, `U24` — the power board's supply |

**`TIME OUT` — the second data body, Pip's time port.** Pip stands outside like Tesla, so `TIME OUT` always
carries a Galvani board and never a crossed cable — any communication board with a channel B.
`TIME OUT` drives channel B: the PPS goes out.

| pin | signal | on this board |
|---|---|---|
| 1 | `CLK/PPS` | **out** — PA15 `PPS_PIP`, `TIM2_CH1` compare, through `U26` and **33 Ω** at its output; Pip's second. 100 kΩ to ground at PA15 |
| 2 | `GND` | the ground plane |
| 3 | `TXD` | PA2 `TXD_PIP`, `USART2_TX` / `TIM15_CH1`, through `U26` — NMEA, and the ranging return; 100 kΩ to 1,8 V at the pin |
| 4 | `RXD` | through `U25` onto PA3 `RXD_PIP`, `USART2_RX` / `TIM15_CH2` — Kronos's heartbeat and the ranging edge; 100 kΩ to ground on the 3,3 V side |
| 5 | `ID` | PB0 `ID_PIP`, `ADC12_INP9`; **10 kΩ 1 % to `VREF+`** |
| 6 | `ID_RET` | **not connected** — `TIME OUT` never meets a crossed cable |
| 7 | `RXD_ECHO` | **not connected** — one talker on the pair and nothing echo-checks NMEA; Kronos's parser checks the sentence |
| 8 | `DE` | PE12 `DE_PIP`, through `U26`; 100 kΩ to ground at the pin |
| 9 | `B_DIR` | **tied to 3,3 V** — this end drives channel B; no processor pin |
| 10 | `LINE_EN` | **10 kΩ to 3,3 V** — the line side runs whenever the unit does; it follows no processor pin |
| 11 | `SD` | through `U25` onto PE13 `SD_PIP`; 100 kΩ to ground on the 3,3 V side |
| 12 | `3,3 V` | the 3,3 V interface rail, `U24` |

**Bench, on every board:** `TIME OUT`'s PPS edge within **±1 tick of 2²⁸** of the TIM2 compare
value it was placed at, on a counter, the socket driven into a Kronos; with nothing plugged,
`ID_PIP` reads 1,00 ± 0,025.

**`12V` — the 12 V terminals.** Two two-pole Degson **`DGPS2.5R-5.0`**, 5 mm, 0,75–2,5 mm², an
input and a tap on the same node, taking the unit power board's island output off its own
terminals. No 12 V rides a ribbon: pin 12 of a data body and pin 8 of the power body are this
board's 3,3 V, and nothing else on the bodies is a supply.

---

## The processor — pins, timers, clock tree

*The record to draw the H7A3 from and to set CubeMX by. Package LQFP176, `STM32H7A3IIT6` — the LDO-supply part, no SMPS pins; pin
numbers and alternate functions from DS13195 Rev 8, the LQFP176 column. Every pin below is either
assigned or listed free — nothing is left to guess.*

### Why the H7A3

**The DSP tier's part — Tesla, Pip, Marconi and the two scintillation boards — and no DSP-tier
board goes back to the H523.**

| | H523 | **H7A3IIT6** |
|---|---|---|
| core | Cortex-M33 @ 250 MHz | **Cortex-M7 @ 280 MHz** |
| FPU | single precision | **double precision** |
| Dhrystone | 1,50 DMIPS/MHz → **375** | 2,14 DMIPS/MHz → **599** |
| SRAM | 272 KB | **1,4 MB** — 1 MB of it AXI, plus 128 KB DTCM / 64 KB ITCM and 288 KB of SRAM1/2/4 |
| SPI | **exactly 4** | **6** |

- **The memory and the sixth SPI buy the part.** Four SPIs would leave the converter's
  configuration port bit-banged; six leave it a real port with spares. 1,4 MB is what capture
  depth and a learned classifier's model are made of: DTCM holds the detector loop at zero wait
  states while the AXI SRAM holds the ring.
- **The compute gap is larger than the datasheet ratio.** Both parts are clocked off the timebase,
  **2²⁸ = 268,435456 MHz against 2²⁷**, so the clock ratio in the station is 2×, not 1,12×, and
  Dhrystone at station clocks is **574 against 201 — 2,9×**. On a tight SIMD MAC loop it is
  **~4×**: the M7 is dual-issue out of TCM at zero wait and sustains about two MACs a cycle against
  the M33's one — **~537 MMAC/s against ~134**.
- **Double precision** takes the scaling arithmetic out of the slow maths around the loops —
  baseline tracking, curve fits, the µ(T) correction; the inner loops are fixed-point SIMD either
  way.
- **The budget is headroom, not slack.** Tesla runs at ~18,5 % of the core at its ceiling; what is
  left buys the next analysis, and every one of them is a MAC loop.

### Peripherals

| peripheral | instance | pins | job | mode |
|---|---|---|---|---|
| the NodBus link | **USART1** + **TIM4** | PB6 `TXD` · PB7 `RXD` | the spur to its card, 8N1 at the rung `BUSCFG` hands down — 2²⁰ alone, 2²¹ chained, through the fixed-direction translators | the pins switch USART → TIM4 for the ranging instant |
| the frame anchor | **TIM2** (32-bit) | PB3 ← `RXD` on CH2 · PA15 `PPS_PIP` on CH1 | the card's frame start on the grid; Pip's PPS is a compare on the same counter | never reset, read as differences |
| echo receiver | **USART3**, RX only | PD9 `RXD_ECHO` | hears the board's own frames | |
| the converter's data | **SAI2** block A · **SAI1** blocks A and B, all slaves | PD13/PD12/PD11 · PE5/PE4/PE6 · PE3 | three lanes of the ADS127L14's frame-sync port, `DP_TDM` 11b — `DCLK` 2²⁵ and `FSYNC` reach both peripherals' clock pins, block B rides block A's | slave receiver, 32-bit slots, data on the falling edge, latched on the rising; one DMA stream per block |
| the converter's clock | **MCO1** ← PLL1_Q | PA8 `CLKIN` | 2²⁵, phase-locked to the network | prescaler 1 |
| the converter's registers | **SPI1**, mode 1 (`CPOL` 0, `CPHA` 1), ≥ 2 MHz + `CS` PA4 | PA5 · PA6 · PA7 | the configuration port — written once at boot, `CLK_CNT` read once a second for the free-clock check, `STATUS` read on `ERROR`'s edge | mode 1, 2–10 MHz; `START` PD0, `RESET` PD1, `ERROR` PD2 on EXTI |
| thermometers | **ADC1** | PC2 · PC3 · PC4 · PC5 | four NTCs, ratiometric to `VREF+` | 256× hardware oversampling, one DMA stream |
| ID reads | **ADC1** | PC0 · PC1 · PB0 | `NB IN`'s data body, `PWR IN`'s power body and `TIME OUT`'s data body | read once at bring-up |
| PSRAM | **OCTOSPI1**, port 2 | PF0–PF5 · PG0 · PG1 · PG10–PG12 · PF12 | the APS25608N, 32 MB octal DDR — the classifier's background model; a population | ≤ 2²⁶, DQS on PF12 |
| `TIME OUT`, Pip's socket | **USART2** + **TIM15** · TIM2 CH1 | PA2 · PA3 · PA15 · PE12 · PE13 | NMEA out and the PPS out, channel B outward — Kronos's capture end, this board's compare end. **Populated on every board; Tesla's image leaves it idle, Pip's serves it under Kronos's heartbeat** (`pip/FIRMWARE.md`) | 115 200 8N1; TIM15 the turnaround |
| power body | **I2C1** through the `PCA9306` | PB8 · PB9 | the unit power board's `INA238` | 100 kHz; `ALERT` on PC8, EXTI |
| clock in | **HSE bypass** | PH0 | the data body's `CLK`, 2²², through the translator | PH1 unconnected |
| debug | **SWD** | PA13 · PA14 | | no JTAG, no trace |

The whole part runs at 1,8 V, `VDDIO_HSLV` set, so every line to the converter is 1,8 V direct
and every line to the station crosses a translator (§8). The configuration port is a real SPI —
the H7A3 has six and the rods take none. **No regulator is enabled from a pin**: every rail runs whenever the 12 V is there, and
what the board switches off it switches at the parts — the converter's `START`, its standby
register. **Three SAI blocks, not four**: SAI1 block B is synchronous to block A and needs no
clock pins, so `DCLK` and `FSYNC` are two nets on four pins, and the fourth block stays free.

### Pins

| pin | port | signal on the board | AF / function | dir | note |
|---|---|---|---|---|---|
| 1 | PE2 | `DE` | GPIO | out | through `U26` — `NB IN`'s driver enable; 100 kΩ to ground |
| 2 | PE3 | `DOUT2` | `SAI1_SD_B` | in | rod 3 — block B synchronous to block A, no clock pins of its own |
| 3 | PE4 | `FSYNC` | `SAI1_FS_A` | in | the same net as PD12 |
| 4 | PE5 | `DCLK` | `SAI1_SCK_A` | in | the same net as PD13 |
| 5 | PE6 | `DOUT1` | `SAI1_SD_A` | in | rod 2 |
| 6 | VBAT | — | | | tied to `VDD`; no RTC anywhere |
| 7 | PI8 | — | | | free |
| 8 | PC13 | — | | | free (tamper/RTC pin, unused) |
| 9 | PC14 | — | | | free — no 32 kHz crystal |
| 10 | PC15 | — | | | free |
| 11 | PI9 | — | | | free |
| 12 | PI10 | — | | | free |
| 13 | PI11 | — | | | free |
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
| 24 | PF6 | — | | | free |
| 25 | PF7 | — | | | free |
| 26 | PF8 | — | | | free |
| 27 | PF9 | — | | | free |
| 28 | PF10 | — | | | free |
| 29 | PH0 | `CLK_IN` | `OSC_IN`, bypass | in | `NB IN`'s `CLK`, 2²², through `U25` — `V_IH` ≥ 1,26 V at `VDD` 1,8 V |
| 30 | PH1 | — | | | unconnected in bypass mode |
| 31 | NRST | reset | | | 100 nF, no pull beyond the internal |
| 32 | PC0 | `ID_D` | `ADC12_INP10` | in | the data body's `ID` — the 10 kΩ pull-up on this board's 1,8 V, so the ratio is against `VREF+` |
| 33 | PC1 | `ID_P` | `ADC12_INP11` | in | the power body's `ID`, the same way |
| 34 | PC2 | `NTC_1` | `ADC12_INP12` | in | rod 1's NTC — 10 kΩ from `VREF+`, the NTC to `VSSA`, 3 kΩ + 100 nF at the pin. **On LQFP176 the pin is `PC2_C`**: `INP12` reads it only with the `SYSCFG` analog switch closed; `ADC2_INP0` is the direct channel and the firmware may take either (DS13195 Table 7) |
| 35 | PC3 | `NTC_2` | `ADC12_INP13` | in | rod 2 — the same: the pin is `PC3_C`, `INP13` through the switch or `ADC2_INP1` direct |
| 36 | VDD | 1,8 V | | | |
| 37 | VSSA | | | | |
| 38 | VREF+ | the `VDDA` node — the ADC reference, strapped to `VDDA` | | | |
| 39 | VDDA | 1,8 V through the shielded 2,2 µH `SWPA252012S2R2MT`, 10 µF + 100 nF at the pins, `VREF+` on the same node | | | |
| 40 | PA0 | — | | | free (ADC) |
| 41 | PA1 | — | | | free (ADC) |
| 42 | PA2 | `TXD_PIP` | `USART2_TX` / `TIM15_CH1` | out | `TIME OUT`, the second data body — Pip's NMEA to Kronos, through `U26`; 100 kΩ to 1,8 V |
| 43 | PH2 | — | | | free |
| 44 | PH3 | — | | | free |
| 45 | PH4 | — | | | free |
| 46 | PH5 | — | | | free |
| 47 | PA3 | `RXD_PIP` | `USART2_RX` / `TIM15_CH2` | in | through `U25`; Kronos ranges this run, TIM15 turns the edge round |
| 48 | VSS | | | | |
| 49 | VDD | 1,8 V | | | |
| 50 | PA4 | `CS_C` | GPIO | out | |
| 51 | PA5 | `SCLK_C` | `SPI1_SCK` | out | the converter's configuration port — a real port, written at boot |
| 52 | PA6 | `DOUT_C` | `SPI1_MISO` | in | |
| 53 | PA7 | `DIN_C` | `SPI1_MOSI` | out | |
| 54 | PC4 | `NTC_3` | `ADC12_INP4` | in | rod 3 |
| 55 | PC5 | `NTC_PCB` | `ADC12_INP8` | in | the board |
| 56 | PB0 | `ID_PIP` | `ADC12_INP9` | in | `TIME OUT`'s `ID` |
| 57 | PB1 | — | | | free (ADC) |
| 58 | PB2 | — | | | free |
| 59 | PF11 | — | | | free (ADC) |
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
| 75 | PE12 | `DE_PIP` | GPIO | out | through `U26` — `TIME OUT`'s driver enable; 100 kΩ to ground |
| 76 | PE13 | `SD_PIP` | GPIO | in | through `U25` — `TIME OUT`'s optical no-light |
| 77 | PE14 | — | | | free |
| 78 | PE15 | — | | | free |
| 79 | PB10 | — | | | free |
| 80 | PB11 | — | | | free |
| 81 | VCAP | 2× 1 µF 50 V 0805 + 2× 100 nF 50 V 0603 — 2,2 µF, the sheet's 2,2 µF ±20 %, the internal LDO | | | |
| 82 | VDD | 1,8 V | | | |
| 83 | PH6 | — | | | free |
| 84 | PH7 | — | | | free |
| 85 | PH8 | — | | | free |
| 86 | PH9 | — | | | free |
| 87 | PH10 | — | | | free |
| 88 | PH11 | — | | | free |
| 89 | PH12 | — | | | free |
| 90 | VSS | | | | |
| 91 | VDD | 1,8 V | | | |
| 92 | PB12 | — | | | free |
| 93 | PB13 | — | | | free |
| 94 | PB14 | — | | | free |
| 95 | PB15 | — | | | free |
| 96 | PD8 | — | | | free |
| 97 | PD9 | `RXD_ECHO` | `USART3_RX` | in | through `U25` — the echo check |
| 98 | PD10 | `SD` | GPIO | in | through `U25` — the optical no-light |
| 99 | PD11 | `DOUT0` | `SAI2_SD_A` | in | rod 1 |
| 100 | PD12 | `FSYNC` | `SAI2_FS_A` | in | the frame sync — one net on two pins (PE4 too) |
| 101 | PD13 | `DCLK` | `SAI2_SCK_A` | in | the ADS127L14's bit clock, 2²⁵ — one net on two pins (PE5 too), 33 Ω at the converter |
| 102 | VSS | | | | |
| 103 | VDD | 1,8 V | | | |
| 104 | PD14 | `PGOOD` | GPIO | in | the 1,8 V `TPS629206`'s `PG` |
| 105 | PD15 | — | | | free |
| 106 | PG2 | — | | | free |
| 107 | PG3 | — | | | free |
| 108 | PG4 | — | | | free |
| 109 | PG5 | — | | | free |
| 110 | PG6 | — | | | free |
| 111 | PG7 | — | | | free |
| 112 | PG8 | — | | | free |
| 113 | VSS | | | | |
| 114 | VDD33USB | tied to `VDD` | | | USB unused |
| 115 | PC6 | — | | | free |
| 116 | PC7 | — | | | free |
| 117 | PC8 | `ALERT` | GPIO, EXTI | in | through `U25` — the power body's `INA238`, high = alarm |
| 118 | PC9 | — | | | free |
| 119 | PA8 | `CLKIN` | `MCO1` ← PLL1_Q, prescaler 1 | out | **2²⁵ = 33,554432 MHz** into the ADS127L14's `CLKIN`, at 1,8 V directly |
| 120 | PA9 | — | | | free |
| 121 | PA10 | — | | | free |
| 122 | PA11 | — | | | free (USB, unused) |
| 123 | PA12 | — | | | free (USB, unused) |
| 124 | PA13 | `SWDIO` | SWD | i/o | |
| 125 | VCAP | 2× 1 µF 50 V 0805 + 2× 100 nF 50 V 0603 — 2,2 µF, the sheet's 2,2 µF ±20 %, the internal LDO | | | |
| 126 | VSS | | | | |
| 127 | VDD | 1,8 V | | | |
| 128 | PH13 | — | | | free |
| 129 | PH14 | — | | | free |
| 130 | PH15 | — | | | free |
| 131 | PI0 | — | | | free |
| 132 | PI1 | — | | | free |
| 133 | PI2 | — | | | free |
| 134 | PI3 | — | | | free |
| 135 | VSS | | | | |
| 136 | VDD | 1,8 V | | | |
| 137 | PA14 | `SWCLK` | SWD | in | |
| 138 | PA15 | `PPS_PIP` | `TIM2_CH1` output compare | out | `TIME OUT`'s `CLK/PPS`, channel B outward through `U26` and 33 Ω — Pip's second, a timer edge on the timebase; 100 kΩ to ground |
| 139 | PC10 | — | | | free |
| 140 | PC11 | — | | | free |
| 141 | PC12 | — | | | free |
| 142 | PD0 | `START` | GPIO | out | ADS127L14 `START` |
| 143 | PD1 | `RESET` | GPIO | out | ADS127L14 `RESET` |
| 144 | PD2 | `ERROR` | GPIO, EXTI2 | in | the converter's `ERROR`, falling edge — its own 100 kΩ pull-up to `IOVDD`, the same 1,8 V |
| 145 | PD3 | — | | | free |
| 146 | PD4 | `LED` | GPIO | out | the status LED, fitted — 1 kΩ from the 3,3 V, one 50 ms blink a minute while healthy, two on a fault (`../core/HARDWARE.md`) |
| 147 | PD5 | — | | | free |
| 148 | VSS | | | | |
| 149 | VDDMMC | 1,8 V | | | |
| 150 | PD6 | — | | | free |
| 151 | PD7 | — | | | free |
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
| 164 | PB6 | `TXD` | `USART1_TX` / `TIM4_CH1` | out | the NodBus link — through `U26` to `NB IN`; 100 kΩ to 1,8 V |
| 165 | PB7 | `RXD` | `USART1_RX` / `TIM4_CH2` | in | through `U25`; the pins switch USART → TIM4 for the ranging instant |
| 166 | BOOT0 | 10 kΩ to ground | | | |
| 167 | PB8 | `SCL` | `I2C1_SCL` | i/o | through the `PCA9306` to `PWR IN` — the `INA238`; 4,7 kΩ to 1,8 V on this side |
| 168 | PB9 | `SDA` | `I2C1_SDA` | i/o | the same; 4,7 kΩ to 1,8 V |
| 169 | PE0 | — | | | free |
| 170 | PE1 | — | | | free |
| 171 | PDR_ON | tied to `VDD` | | | the power-down reset stays armed |
| 172 | VDD | 1,8 V | | | |
| 173 | PI4 | — | | | free |
| 174 | PI5 | — | | | free |
| 175 | PI6 | — | | | free |
| 176 | PI7 | — | | | free |

**53 GPIO used, 86 free, `PH1` unconnected** (PA0 · PA1 · PA9–PA12 · PB1 · PB2 · PB4 · PB5 · PB10–PB15 · PC6 · PC7 · PC9–PC15 · PD3 · PD5–PD8 · PD15 · PE0 · PE1 · PE7–PE11 · PE14 · PE15 · PF6–PF11 · PF13–PF15 · PG2–PG9 · PG13–PG15 · PH2–PH15 · PI0–PI11).

### Timers

| timer | bits | kernel | pins | job |
|---|---|---|---|---|
| **TIM4** | 16 | 2²⁸ | CH1 → PB6, CH2 ← PB7 | the ranging turnaround: one-pulse, triggered by the capture on CH2, the return raised on CH1 after `CCR` ticks; PWM-input mode on CH2 measures the incoming pulse's width. 244 µs of range at 2²⁸ — a port past 24 km runs it at ÷4 |
| **TIM2** | 32 | 2²⁸ | CH2 capture ← PB3 · **CH3 the slot compare, internal** · CH1 compare → PA15 | the free-running timebase; the card's frame start places the round on the grid. **CH3 fires the transmit slots and is ONE channel re-armed per slot** — slots never overlap inside a round, so the NOD count costs no channels and three NODs are three compare events on the one channel, not three of them. **CH1 is Pip's PPS** on Pip's image, one edge a second on the same counter, and idle on Tesla's, where `TIME OUT` is closed. **CH4 free.** |
| **TIM15** | 16 | 2²⁸ | CH1 → PA2, CH2 ← PA3 | `TIME OUT`'s turnaround — Kronos fires, this board returns the edge after `CCR` ticks |
| the strike captures | | | none — the CFD is digital on the sampled stream | no timer channel is spent on a strike; how many captures the firmware runs is its own business |
| **TIM6** | 16 | | | the housekeeping second (`FIRMWARE.md` §12) |
| TIM1 · TIM3 · TIM5 · TIM8 · TIM12–TIM14 · TIM16 · TIM17 · LPTIM1–LPTIM5 | | | | free |
| TIM7 | 16 | | | free — the other basic timer |

### Clock tree

| | |
|---|---|
| `OSC_IN` | 2²² = 4,194304 MHz, the data body's `CLK` through the translator — no crystal on this board |
| HSE | bypass |
| PLL1 | M 1 · N 128 · P 2 · Q 16 → VCO 2²⁹ = 536,870912 MHz, `P` **2²⁸**, `Q` **2²⁵** |
| SYSCLK, AXI, AHB | **2²⁸ = 268,435456 MHz** — VOS0, `VDD` ≥ 1,71 V |
| APB1/2 | ÷2; the timer kernels at 2²⁸ |
| MCO1 | PLL1_Q, prescaler 1 → **2²⁵** on PA8 — the converter's `CLKIN` |
| SAI kernels | slaves — the bit clock comes from the converter |
| OCTOSPI kernel | 2²⁸ ÷ 4 = 2²⁶ |
| the clock gone | HSI fallback and the degraded flag; the off sequence drops the clock the same way and the returning feed is the wake (`../core/PROTOCOL.md` §7) |

The one PLL, integer, no fractional divider; `f_DATA` = 2²⁵ / (2 · 16) = 2²⁰ follows from the
converter's own dividers and nothing on this board divides by anything but a power of two.

### Analogue and reference

**Every filter inductor on this board is a π filter: 10 µF 1206 + 100 nF 0603 before it on the rail and
the same pair at the pins behind it** — the house rule (`../core/POWER.md`, *The filter parts*).

`VDDA` and `VREF+` are one node on the **1,8 V** through one shielded 2,2 µH `SWPA252012S2R2MT`, 10 µF + 100 nF at the pins — the rail is a buck's own, so the filter is the buck-noise inductor of `../core/POWER.md` and not a ferrite; `VSSA` to
the ground plane at one point — the LQFP176 bonds `VREF−` to `VSSA` inside. The three `ID` inputs are read
single-ended against `VREF+`, and their 10 kΩ 1 % pull-ups hang on the same 1,8 V, so the reading is a
ratio and the rail's tolerance drops out; the bodies' 3,3 V is not used for the `ID` on this
board. The arrived 12 V and the input current are the power body's `INA238`, over I2C1. The four NTC dividers hang on the same `VREF+`, so the thermometers
read whenever the processor runs.

## 9. Reference BOM (JLCPCB-assemblable)

| Ref | Part | LCSC class | Qty |
|---|---|---|---|
| U1..U6 | **`THS4551IDGKR`** FDA, VSSOP-8 (2 per channel, three channels) — the `DGK`, not the 2 × 2 mm `RUN`: 0,65 mm pitch, no bottom pad, hand-reworkable, the same body as `U10`; the WQFN's lower parasitics matter at hundreds of megahertz, two decades above the 27 MHz loop | Extended, stocked | 6 |
| U9 | **ADS127L14IRSHT** quad ΔΣ ADC | Extended, stocked | 1 |
| U10 | **`REF6041IDGKR`** — the converter's 4,096 V reference, VSSOP-8, on the `AVDD1` LDO; 1 µF on `FILT`, 22 µF on the output, 100 nF at `VIN` (§4) | Extended | 1 |
| U14 | **STM32H7A3IIT6** — the node MCU, same PCB | Extended | 1 |
| U15 | **`APS25608N-OBR-BD`** — the OCTOSPI PSRAM, a position: fitted for the learned classifier, open for the rule set alone; `NCS` 10 kΩ to the 1,8 V, 33 Ω at `CLK`, 1 µF + 10 µF (§8) | Extended | 0–1 |
| U11..U13 | **TPS7A4701** — 5,0 V: `AVDD1` (max-speed mode needs 4,5–5,5 V) and the front-end rails, fed from the 5,3 V (§6) | Basic/Extended | 3 |
| U23 | **`TPS629206`** — 12 V → 5,3 V for the three 5 V LDOs, forced PWM at 2,5 MHz (§6) | | 1 |
| U22 | **`TPS629206`** — 12 V → 1,8 V for the MCU, never switched, forced PWM at 1 MHz (§6) | | 1 |
| U24 | **`TPS629206`** — 12 V → 3,3 V, the interface rail, forced PWM at 2,5 MHz (§6) | | 1 |
| D1..D3 | **BAV199** — the coil clamp, two diodes anti-parallel across each rod's pair, on the amplifier side of `Rs1` (§0.2) | Basic | 3 |
| R/C | Rf/Cf (with the 10 Ω in each `Cf1` leg)/`R1`/`R2`/`C1`/`C2`/Rg/Cin/Rs/decoupling per §0.2, §2, §4 | Basic (jellybean) | — |
| L1 | **`SWPA252012S2R2MT`** — shielded 2,2 µH, the station's buck-noise inductor: the MCU's `VDDA`/`VREF+` off the 1,8 V, into 10 µF + 100 nF (§6, §7) | Basic `C23894` | 1 |
| L2..L5 | **`SWPA252012S2R2MT`** — the station's shielded 2,2 µH: the converter's `IOVDD` and `AVDD2`, and the channel split behind the two 5,0 V front-end LDOs (§6) | Basic `C23894` | 4 |
| FB | **`GZ2012D301TF`** — the 0805 bead, 300 Ω at 100 MHz, behind the inductors of `IOVDD` and `AVDD2`, for what reaches past the inductor's 69 MHz (§6) | | 2 |
| U25, U26 | **`SN74AXC8T245`** — the direction-set translators, 1,8 V ↔ 3,3 V, `OE#` to ground: `U25` inbound, `DIR` to ground; `U26` outbound, `DIR` to the 1,8 V (§8, *The Galvani sockets*) | Extended | 2 |
| U27 | **`PCA9306`** — the I²C to `PWR IN`, `VREF1` on the 1,8 V, `VREF2` and `EN` on 200 kΩ to the 3,3 V (§8) | Extended | 1 |
| R (sockets) | `ID_D`, `ID_P`, `ID_PIP` 10 kΩ 1 % to `VREF+` ×3 · `NB IN` and `TIME OUT` `LINE_EN` 10 kΩ to 3,3 V ×2 · the inbound lines 100 kΩ to ground ×7 · the outbound lines 100 kΩ ×5 · the I²C 4,7 kΩ ×4 · `U27`'s 200 kΩ · 33 Ω on `TIME OUT` pin 1 (§8) | Basic | — |
| `NB IN` | Galvani data body, `BX2.54-2xNA` 2×6 — NodBus (§8, *The Galvani sockets*) | — | 1 |
| `PWR IN` | Galvani power body, `BX2.54-2xNA` 2×4 — `A_SEL` to ground, so its board is 0x40; `ENABLE` not connected (§8) | — | 1 |
| `12V` | **`DGPS2.5R-5.0`**, two-pole — the 12 V terminals, an input and a tap, off the unit power board's terminals (§8) | — | 2 |
| `ROD X` · `ROD Y` · `ROD Z` | antenna rod terminals (3×, differential) | — | 3 |
| `TIME OUT` | Galvani data body, `BX2.54-2xNA` 2×6 — **the second data body, populated on every board**: Pip's time port toward Kronos, PPS out on channel B and NMEA on channel A, through a communication board; `B_DIR` tied to 3,3 V. Tesla's image leaves it idle, Pip's serves it when Kronos's heartbeat answers (§8, `pip/README.md`) | — | 1 |

Off-board / not JLC-assembled: **3× ferrite rod** (Mn-Zn μ≈800, datasheet grade — Amidon 33
`R33-050-750` or Stormwise VLF) + **hand-wound shielded loops**, and **3× `NXFT15XH103FA2B`** on the rods
(§7; the fourth, an `NCP18XH103F03RB`, sits on the board, and every fixed 10 kΩ leg is on the board).

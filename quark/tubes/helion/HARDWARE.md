★ N.I.C. ★

# Helion — the He³ / BF₃ proportional tube and the kV source

> **Design-stage concept — nothing built.** The figures are verified against the parts' sheets and
> the parts before a head is built. What Helion is: [`README.md`](README.md); the board it counts
> on is `Quark-Tubes` ([`../HARDWARE.md`](../HARDWARE.md)); rejected and superseded states are
> [`WHY.md`](WHY.md).

**One quantity, neutron, on the HV variant: the tube, its bias and its charge front end, counted
on `Quark-Tubes`' `K4`.** The bias is Helion's kV source, tapped for the tube, 1300–2000 V (§2). Gadolin's 13-tube
merge fills the same `K4` on a build that carries it instead.

## How it detects

A **³He proportional tube**. A neutron carries no charge, so it can't ionise the gas by itself — but
when it is captured, `³He(n,p)³H`, the reaction fires off a proton and a triton carrying **0,76 MeV**,
and *those* ionise the gas and make the pulse (family **(a)**, a prompt charged product — `../../NEUTRONS.md`).

**Efficiency ~70–96 %** against the Gadolin ring's ~1 %; He³ is scarce since 2008 and the head
needs a charge front end, which is the price of that. The fill is a populate-time choice (§5).

## 1. The chain — a transimpedance stage, a gain stage, two comparators

```
 kV source ──[ R_iso 1 MΩ ]──┬── HV+ ──[ R_bias, the tube's sheet ]── anode
                            100 nF
                             ═╧═
 cathode ──[ 1 MΩ ]──▶ LTC6268, DC-coupled, Rf ‖ Cf ──[ C47 ]──[ 100 k ]──▶ TLV9061 ×5,1 ──┬──▶ LLD, above the noise ──▶ K4A  (γ + n)
            (the tube body insulated from the shield)                                        └──▶ the neutron threshold ──▶ K4   (n)
```

**The pulse is read at the cathode and the HV never reaches the amplifier**, as on the GM heads:
the anode takes the kV through the local RC and its bias resistor, the cathode — the tube body,
insulated from the shield — goes through **1 MΩ** into the summing node of an **`LTC6268`** held at
**`Vcc/2` = 1,65 V** by a 100 k / 100 k divider. **DC-coupled: no blocking capacitor at the input** —
the stage is a transimpedance amplifier and the tube's own gigohms of DC impedance limit the
standing current into the virtual ground to the tube's leakage. The 1 MΩ is the input's
protection; there is no clamp.

**`Rf` ‖ `Cf` is the whole shaping.** A capture's charge lands on `Cf` as a step `Q/Cf` and decays
through `Rf` with τ = `Rf · Cf`; nothing else shapes the pulse — no integrator, no CR-RC² stage, no
footprint for one. The head counts and does not measure a spectrum.

**`LTC6268`, the unity-gain-stable part, I-grade.** What decides it is speed: during the
amplifier's delay the charge piles onto the input capacitance and moves the summing node by
dV/dt × delay, and 500 MHz keeps that excursion small. Its fA input comes with it; the signal is
0,2 pC — 1,25 million electrons — so input current noise never enters. The decompensated `-10`
would want `C_in/Cf` ≥ 9, which the tube's ~8 pF against 1 pF does not give. **`TLV9061`** for the
gain stage: the first stage sets the noise, the second sees a volt from a low-impedance output;
what binds it is slew, `SR ≥ ΔV_out / t_rise`. **A He³ tube collects its charge in 4–8 µs** — the
electrons drift in from along the proton–triton track and the ions moving off the wire induce the
rest — so the 1,02 V step asks 0,13–0,26 V/µs of the part's 6,5 V/µs, 25× over; a BF₃ or a
boron-lined fill is of the same order. Both on the head's **3,3 V**, the board's rail carried on
the head cable; there is no other rail on `Quark-Tubes`.

**The head board carries K4's thermometer**: an `NCP18XH103F03RB` NTC, 0603, two wires on the
head cable to the divider on `Quark-Tubes` (`../HARDWARE.md`, *The thermometers sit on the head
boards*); the count's correction `K × (1 + α × (T − 20 °C))` is the board's, α small.

## 2. The bias — the kV source

**One construction, two instances, two voltages.** The ~400 V module on `Quark-Tubes` feeds every GM
tube of a build (`../HARDWARE.md`, *The one 400 V source*); the kV source is the same
construction as a module of its own for the kV load — the He³ / BF₃ tube on the HV variant, the
photomultiplier on the LV one — because the two loads want different voltages and one source
cannot hold both. The construction: an `LT8331` flyback into a half-wave Cockcroft-Walton ladder,
`FBX` regulation across the real output.

### The ladder — 400 V a stage, populated to the ceiling, tapped to the voltage

The winding delivers 400 V and every ladder stage adds 400 V. **The number of stages fitted sets
the ceiling; the `FBX` ratio sets the voltage under it.**

| stages fitted | ceiling | build |
|---|---|---|
| 2 | 1200 V | — |
| **3** | **1600 V** | **the photomultiplier**, tapped 1200 V, the third stage the headroom for a gain step; **the He³ tube** at its fill's 1300–1600 V |
| 4 | 2000 V | a BF₃ or a high-voltage He³ fill |

One board and one BOM cover the three. **Every ladder part works at 400 V**, five times inside the
2 kV film; the output reservoir sits across the whole output and is a 3 kV part. The stage is
never lengthened to save parts: at a fixed output, halving the stage count doubles the stage
voltage and puts it on the winding, and the winding is the one part that is not to be stressed.

**Half-wave, four positions at most.** The load is microamps, so ripple and droop are nothing and a
full-wave ladder would be parts without a job. What the ladder's parts must do:

| part | value | why this and not the obvious |
|---|---|---|
| **diodes** | HV fast rectifier, **5 kV**, `2CL` class | 5 kV against ~1 kV of PIV; fast because the ladder runs at the converter's frequency. Two figures matter here that do not in a converter with µF behind the diode: **reverse leakage** — at µA of load a leaky diode takes the voltage, and `I_R` doubles every 10 °C — and **junction capacitance**, 50–100 pF, which divides against ladder capacitors of nF. The 47 nF below makes it 0,2 % |
| **ladder capacitors** | **47 nF · 2 kV · polypropylene film** (`MMKP82` / `C82` class), one value at every position | film holds its value at full voltage; an HV ceramic loses most of its marked value under DC bias and the ladder with it. One value because at µA of load nothing tapers; 47 nF because it swamps the diodes' capacitance and holds the output through the quarter-cycle a half-wave ladder on a flyback conducts. Through-hole on a generic footprint by pitch |
| **output reservoir** | **22 nF · 3 kV film** | across the whole output, where a 2 kV part at 2000 V has no margin |
| **output clamp** | a gas discharge tube across the output, **2,5 kV** class | the one protection against an open loop — an open `FBX` string runs the converter to its current limit and the ladder above its film's rating; no standing current, nothing in the measurement |
| **bleeder** | none — the `FBX` string is the bleeder | below |

**Stored energy is under 0,1 J** (½CV² on each 47 nF at its 400 V, and on the 22 nF reservoir at the full 1–2 kV) — a bite, not a hazard, and there
is no crowbar. **The output stays charged for minutes after power-off**: the string drains it with
τ ≈ 30 s, and the output is shorted before it is touched.

### The transformer — `750311681`, and its two secondaries go in series

**`Würth WE-FB`, EP13, order code `750311681`:**

| | | |
|---|---|---|
| `L` primary (N1) | **100 µH** ±10 % | |
| turns ratio N1 : N2 : N3 | **1 : 10 : 10** | **two secondaries** |
| `I_SAT` | **2 A** typ. | against `LT8331`'s 0,5–0,7 A limit — **4×** |
| leakage `L_S` | **3 µH max** | 3 % of the primary; the clamp takes ~10 mW and `BZX55C62` is a 500 mW part — **50×** |
| interwinding `C` | 30 pF max | the grounds are common, so the displacement current returns locally and this is not a common-mode path |
| insulation N1 ⇒ N2,3 | **1500 V AC** | against ~640 V of winding excursion at the 2 kV build — **2,3×** |
| `RDC` | 0,22 Ω primary, 28,5 Ω per secondary | 0,05 V at the divider's current |
| temperature | −40 … +125 °C | |

**Both secondaries in series, 1:20.** On one secondary alone the reflected voltage at the 2 kV build
is 50 V against `BZX55C62`'s 62 V — a 1,24× margin, which would have the clamp conducting every cycle
instead of only on the leakage spike. In series the reflected voltage is 25 V and the margin is 2,5×;
the drain sits at 37 V and is clamped at 74 V against a switch rated 140 V. One transformer with two
secondaries, not two transformers.

**A flyback, and it stays a flyback.** The gapped core stores the energy; a forward or push-pull
stage would want an ungapped core and a reset, and no catalogue winding makes the 400 V base
directly (`../../WHY.md`, *A push-pull HV supply instead of the flyback and its multiplier*). A ladder on a flyback harvests the whole swing, the on-time and the off-time,
which is more volts and not a problem. **The tube draws ~zero current, so the transformer is
voltage-stressed and never current-stressed** — the limit is insulation and creepage, not heat.

### Regulation — `LT8331`, `FBX` across the real output

**`LT8331`**: a non-isolated flyback controller with a **140 V, 0,5 A switch**, 4,5–100 V in,
100–500 kHz, 6 µA quiescent in Burst Mode — this module's part, it travels nowhere else. Its **`FBX` pin
regulates the true output at 1,600 V** (1,568–1,632 V; line regulation 0,005 %/V), and **the `FBX`
divider sits across the ladder's output, so the whole ladder is inside the loop**: the forward and
flyback mix, the diodes' leakage and the battery's droop are all corrected. The He³ tube's gas gain
is ∝ e^V and does not tolerate the sag a primary-side-sensed supply leaves.

- **`FBX` divider: `R_top` 625 MΩ / `R_bottom` 500 kΩ** — ratio 1251, 1,600 V × 1251 = **2 kV**;
  standing current **3,2 µA**, 6 mW at 2 kV; τ of the bleed with 47 nF ≈ 30 s. The string is
  built from 47 MΩ elements and the ratio is a tap on it, one per ladder length.
- **The `FBX` pin's own current is ±10 nA at most** (`LT8331` sheet: `FBX` pin current at 1,6 V,
  −10 … +10 nA over temperature), so across 625 MΩ it is **≤ 6,25 V, 0,3 % of 2 kV**, and the
  reference's own tolerance is 2 %. Both are a static offset: **`R_top` is trimmed on the bench to
  the target**, and what drifts afterwards — the string's tempco on the gain, ∝ e^V on the tube and
  V^5,5 on the photomultiplier — is absorbed by the floating thresholds and the ladder like every
  other slow drift. The full 1250 MΩ string would double the offset and the bleed time for 1,6 µA
  saved; not taken (`WHY.md`).
- **The string is the minimum load, the bleeder and the sense in one.** A converter with no load
  cannot hold its output: the smallest energy it delivers in a cycle has to go somewhere or the
  voltage walks up, and the 3,2 µA of the string is where it goes. That is half of why `LT8331` is
  the part — a controller wanting a stiffer divider would need a dummy load, and on a supply whose
  tube draws ~zero that dummy load would be the power budget.
- **`SYNC/MODE` is set by the build.** The He³ tube draws ~3 µA: **`GND`, Burst Mode** — 6 µA of
  quiescent current on a supply that runs all day, and the sheet's low-ripple light-load mode. The
  photomultiplier's divider draws 102 µA, 0,12 W, where burst's envelope sits in the kHz: that
  build takes **an external clock on `SYNC`**, a fixed frequency inside the part's 100–500 kHz,
  from one of the board's timers. Pulse-skipping (`INTVCC`) is not used. **The switch-node damper
  is fitted on both builds**: `R` 1,8 kΩ / `C` 100 pF across the primary, ~3 mW, against the
  discontinuous ring of `L_PRI` with the switch-node capacitance at ~2,9 MHz — an emission
  position, not a regulation one (`../../../galvani/README.md`, *Converters and measuring boards*).
- **The output is filtered again at the load**: an RC at the tube's cathode, at the
  photomultiplier's base, so what the cable picks up does not reach the anode through the tube's
  own capacitance (§1). Regulation is the loop's; ripple is this filter's.

**The 12 V input** is the unit's 12 V wire — the unit power board's island output — taken on the
board's own two two-pole Degson **`DGPS2.5R-5.0`** terminals, an input and a tap, the same node; the
counting board beside it does not carry the 12 V across itself for this one.

**The input filter**: the house inductor `SWPA252012S2R2MT` on the 12 V input into 2× 10 µF 50 V
1206 + 100 nF (`../../../core/POWER.md`, *The filter parts*) — what does not leave on the
wire does not radiate, and this is the noisiest board in the unit. The input draws tens of
milliamps against the part's 1,15 A.

### Layout — its own board, potted

**The kV source is its own board, cabled to the head** — the converter, the ladder, the reservoir
and the `FBX` string together, away from the charge front end. On the tube head the front end
stays at the tube, where the high-impedance node must be millimetres short (§1), and the cable carries the HV up, the amplifier's supply up and a low-impedance signal down; on
the photomultiplier head the base is at the tube and the cable carries the HV alone.

**Creepage and clearance** by slots and cut-outs between HV nodes; no sharp copper — corona; the
board washed, because flux conducts at kilovolts; the HV section potted. Every part is rated at its
**continuous working DC**, never at a surge or a maximum figure, and everything is rated for the
**2 kV ceiling** so the one BOM covers every ladder length.

## 3. The values, and what each follows from

| | value | follows |
|---|---|---|
| **`Cf`** | **1 pF** — the parasitic plus a fitted part on the trim pad | `V_step = Q/Cf` ≈ **200 mV** a capture at Q ≈ 0,2 pC, independent of `Rf`; 170–190 mV after the ballistic deficit, below |
| **`Rf`** | **22–47 MΩ**, the leakier the tube the lower | τ = `Rf · Cf` = 22–47 µs |
| the offset at 10 nA of leakage | 0,22–0,47 V, **on the transimpedance stage's output only** | `V = I · Rf`; `C47` stops it |
| the second stage | **×5,1** (510 k / 100 k) → **1,02 V** of pulse above 1,65 V | the gain pair |
| **`C47`** | **4,7 nF** | τ_c ≈ 470 µs into the 100 kΩ, below |

**`Rf`'s bounds, on the 3,3 V rail.** Below, τ must outlast the 4–8 µs of charge collection or
the peak under-reads the charge: a step of duration `T` into τ peaks at `(τ/T)·(1 − e^(−T/τ))` of
`Q/Cf`, which is **84–96 % at τ 22–47 µs** — the deficit the thresholds are set under — and would
be 55–75 % at 4,7–10 µs. **`Rf` ≥ 22 MΩ.** Above, the stage's output sits at 1,65 V with ~1,3 V of
usable swing; the 200 mV step is allowed for, so `I_leak · Rf` ≤ ~1,1 V — **50 nA of leakage at
22 MΩ, 23 nA at 47 MΩ**. A proportional tube's sheet gives an insulation resistance above 10¹² Ω
at its bias, which is a nanoampere or less, so a tube that does not fit the window is a bad tube
and not a value of `Rf`; the fitted tube's leakage picks the point within it, a commissioning
datum like the thresholds. Rate never enters: at 0,02–0,5 cps a 47 µs tail meets nothing.

**`C47` is the one owner of the leakage offset.** Without it `I_leak · Rf` would go ×5,1 into the
comparators; with it they see the electronics' few millivolts of offset and nothing of the tube.
Its constant is `C47` into the 100 kΩ against the gain stage's virtual ground — `Rf` is inside
the first loop and does not enter it. A tail pulse of τ_p through a high-pass of τ_c loses about
τ_p/τ_c off its peak and returns the charge as a shallow undershoot through the same 100 kΩ:

| `C47` | τ_c | corner | 50 Hz | recovery, 5τ | peak loss at τ_p 22 / 47 µs |
|---|---|---|---|---|---|
| 10 nF | 1 ms | 159 Hz | −10 dB | 5 ms | 2 % / 5 % |
| **4,7 nF** | **470 µs** | **339 Hz** | **−17 dB** | **2,4 ms** | **5 % / 10 %** |
| 2,2 nF | 220 µs | 723 Hz | −23 dB | 1,1 ms | 10 % / 21 % |

Rate never enters — at 0,02–0,5 cps nothing piles up at any of these; the corner buys hum, 1/f and
the recovery from a tube arc, and 4,7 nF is where the peak loss stays under a tenth across the
`Rf` window. **No pole-zero across `C47`**: the resistor it would want, ~4,7 kΩ
against 100 kΩ, passes DC and puts most of the leakage offset into the gain stage.

## 4. Discrimination — two thresholds, two counts

**Two comparators with hysteresis, two wires to the board.** The **LLD** sits just above the
noise and fires on everything — gamma and neutron — into **`K4A`** (`TIM12_CH1`); the **neutron
threshold** sits above the gamma population — a gamma leaves next to nothing in the thin gas and
a capture dumps its 0,76 MeV, so the two populations are far apart and the threshold sits in the
valley — and fires on captures alone into **`K4`** (`TIM15_CH1`). Both are timer inputs in
external clock mode; a comparator's pulse is microseconds long on the tail, so no stretcher is
needed. A capture rises through the LLD and then the neutron threshold, so `K4` ⊆ `K4A` and the
difference is never negative. Both thresholds are resistor dividers on the head board, **set on
the bench from the fitted tube's pulse-height spectrum** — and re-picked with `Cf`, because a
feedback capacitor moves the pulse and with it the divider. Nothing floats in service; the drift
the tube has with temperature is the count correction's, through the NTC.

**`K4` is the neutron channel; `K4A` is the tube's health.** The gamma count `K4A − K4` is a poor
gamma measurement — the real one is Photon — and the mini frame has no channel for it, so it is a
register and never a word of the frame. What it is for is the threshold: the ratio `K4 / K4A`
is where the neutron threshold stands on the tube's spectrum, and a threshold that has drifted
off the valley — gas gain, HV, a leaking tube — shows there before it shows in the neutron count
(`../FIRMWARE.md` §A6, §A7).

## 5. The fill — He³, BF₃ or boron-lined, set per the tube's sheet

The chain is the same for any proportional fill; **the HV, `R_bias`, `Rf` and the two edges are
set from the fitted tube's sheet** and nothing else changes. He³ gives the highest efficiency and
is scarce; BF₃ is available and toxic; a boron-lined tube is the non-toxic substitute at lower
efficiency. The boron fills give the larger pulse (α + Li, 2,3–2,8 MeV against 0,76) and want the
higher bias, which the ladder's 2000 V ceiling covers. A `⁶Li` glass or a screen is not a fill
for this head — that is the `Neutron` channel's photomultiplier (`../../scintillation/neutron/HARDWARE.md`).

**No lead shield.** The tube is gamma-blind by construction and a proportional tube recovers in
microseconds, locally along the wire, so background gamma never keeps it busy. An intense flash —
a TGF — could load it for the instant the neutrons arrive, and that instant is all it costs.

## 6. The K channels, and where this head's count lands

The counts ride Quark-Tubes' mini frame as **four `uint16` channels — beta · soft γ · hard γ ·
neutron** — and [`../BUS.md`](../BUS.md) owns the layout. **Three GM tubes, K1..K3, feed the gamma and
beta channels and K4 the neutron channel; this head fills `K4`, and `K4A` beside it.** Gadolin/Rhodion's thirteen
tubes, merged, fill the same `K4` on a build that carries the ring instead (`../gadolin/`).

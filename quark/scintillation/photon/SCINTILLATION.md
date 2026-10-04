★ N.I.C. ★

# The scintillation build — the gamma head's physics, and what the three LV heads share

> **Design-stage concept — nothing built.** The figures are verified against the parts' sheets and
> the parts before a head is built. The gamma board is [`HARDWARE.md`](HARDWARE.md), the beta
> block and its board [`../positron/HARDWARE.md`](../positron/HARDWARE.md), the neutron screens
> and their photomultiplier [`../neutron/HARDWARE.md`](../neutron/HARDWARE.md),
> the record [`BUS.md`](BUS.md). What is here besides the gamma head — the calibration chain, the
> reference tables, survivability, the deposition plate, the bill of materials — serves all three.
> Rejected alternatives are [`../../WHY.md`](../../WHY.md).

**Nothing on the two boards exceeds ~30 V.** The one kilovolt source on this side is the
photomultiplier's, Helion's kV module on its own board (`../../tubes/helion/HARDWARE.md` §2). What
replaces the tube side's high voltage on the boards is a crystal, a photosensor and a converter.

## Selectivity is built by blindness

**No detector responds to one radiation only.** A channel is made selective by what sits in front
of it, what it is made of and how its pulse is read — all three, always — and above all by being
unable to see the other radiation; software recovers less than geometry prevents.

| separate | with | why |
|---|---|---|
| neutron from gamma | a thin capture layer | the capture deposits MeV locally; a thin low-Z solid leaves a gamma tens of keV |
| beta from gamma | **two detectors, one blind to beta** | the only robust way |
| alpha from everything | **the window** | it stops every alpha there is (*Alpha emitters*) |
| isotope from isotope | **the spectrum** | photopeaks at named energies |

No channel here is pure and none claims to be: each is a quantity with a quantified admixture,
written beside it.

## The four channels

| channel | material | thickness | why | how it leaks |
|---|---|---|---|---|
| **beta** | plastic scintillator, **`EJ-240`**, the slow plastic, 285 ns | **25 mm block** | low Z, so electrons barely backscatter (~5 % against 40–50 % off a crystal); the 430 nm blue sits on the SiPM's PDE peak; **the thickness contains Y-90's 2,28 MeV** | 19,8 % of 662 keV gammas — subtracted with the gamma head's own rate |
| **gamma** | **CsI(Tl)** | **25 mm cube** | 54 000 photons/MeV, dense and high-Z → clean photopeaks; its 1 µs decay matches the sampling | capture on I and Cs (~7 MeV) and cosmic muons — **both above 2615 keV, separable by energy** |
| **neutron** | **`⁶LiF/ZnS(Ag)` screen · PMMA · a thin second `⁶LiF/ZnS(Ag)`**, read by a photomultiplier — `Neutron` | 0,5 mm the outer, ~0,15–0,2 mm the inner | thin, so nearly transparent to gamma, while the capture products stop in it | a gamma is a few photoelectrons against a capture's hundreds — under the threshold, and the 3 cm lead shield takes a TGF's flood |
| **dose / TGF** | the gamma head's crystal PIN, **in current mode** | — | silicon's 3,6 eV/pair is constant with energy and particle, so dose IS deposited energy read linearly; current mode has no dead time and no pile-up | *The dose duty* |

Gamma and beta run one photosensor family and one bias; the neutron stack is read by the
photomultiplier (`../neutron/HARDWARE.md`).

## The gamma head has two sensors, and they hand over

Both deliver **charge proportional to the photons that reach them**. A SiPM microcell fires and
yields ~2,8·10⁶ electrons; a PIN yields **one** pair per photon. The PIN's better quantum efficiency
(82 % against 25 % at the crystal's green) and its share of the split light leave a **fixed ratio**:

| deposit | SiPM | PIN | ratio |
|---|---|---|---|
| 200 keV | 0,138 nC | 0,42 fC | |
| 500 keV | 0,344 nC | 1,04 fC | **330 000×** |
| 1 MeV | 0,688 nC | 2,09 fC | constant across the range |
| 3,5 MeV | 2,41 nC | 7,30 fC | |

**The constant ratio hands the energy scale over with one number, and a departure from it is a
fault that needs no reference to notice.** There is no range switch: both channels record every
event, two converter inputs in parallel, and which to believe for a pulse is a line of software.

```
   keV    200        500        700              3500              14000
          |----------|----------|-----------------|------------------|
   SiPM   ############ good #####  bending  ###### end
   PIN               ############### good ###############################
                     |<------ overlap: the scale transfers here ------>|
```

The weight shifts near 500 keV — the SiPM bends near 700 keV, the PIN still has S/N ≈ 7 at 500 keV
— and in the overlap both measure the same event and must agree at 330 000. **Both rulers are free
and always present**: the SiPM's own dark counts (the 1 p.e. peak), and for the PIN the **cosmic
muon** at ~14 MeV, about one every ten seconds on a 2,5 × 2,5 cm face.

**The two sensors sit side by side on the crystal's one window** — a sealed can with a second window
opposite is not a part the market offers (`../../WHY.md`). One crystal, one event, both channels
integrating it; the cost is light per sensor, **13,3 MeV instantaneous ceiling side by side against
3,5 MeV for a sensor alone on the face**. **The heads are bought sealed and potted.**

| | bare PIN beside the crystal | PIN on the crystal's window, beside the SiPM |
|---|---|---|
| efficiency | 0,44 % | **46 %** — the crystal's |
| energy | no | **yes, real pulse height** |
| muon ruler | no | **yes** |

**Light collection is assumed at 40 % onto one sensor; two sensors sharing the window get 25–35 %
each**, and every figure below falls with it — the weakest number in this document.

### The charge economy, and why the bias decoupling is a microfarad

**The SiPM's gain is its cell's stored charge**: `Q = C·ΔV` = 179 fF × 2,5 V = **0,447 pC**,
2,79·10⁶ electrons. Each cell sits behind its own ~300 kΩ quench resistor, so cells do not feed each
other on a 54 ns scale; **the reservoir is the capacitor at the chip.** The transient, from a part
drawing ~1,5 µA at rest:

| event | cells | charge | **peak current** |
|---|---|---|---|
| 662 keV | 947 | 0,42 nC | 7,9 mA |
| 2,6 MeV | 3 742 | 1,68 nC | 31 mA |
| **muon, 14 MeV** | 20 005 | 8,95 nC | **167 mA** |

It comes from the capacitor millimetres away, which refills over ~10 µs — **and its sag is a gain
error**, gain being proportional to overvoltage:

| decoupling | 662 keV | **muon 14 MeV** | ceiling at 1 % gain error |
|---|---|---|---|
| 100 nF | 0,17 % | **3,6 %** | 3,9 MeV |
| 470 nF | 0,04 % | 0,76 % | 18 MeV |
| **1 µF** | 0,02 % | **0,36 %** | **39 MeV** |
| 4,7 µF | 0,00 % | 0,08 % | 184 MeV |

**At 100 nF this saturates before the cell count does. Fit 1 µF with 100 nF in parallel** — the
small part carries the edge, the large one the charge.

### The readout impedance is zero

A load resistor's signal voltage **subtracts from the chip's overvoltage**, so cells firing late in a
pulse see less than the early ones:

| load | 662 keV | **muon 14 MeV** |
|---|---|---|
| 300 Ω | 127 mV = 5,1 % | 2683 mV = **107 %** — the chip switches itself off |
| 100 Ω | 42 mV = 1,7 % | 894 mV = 35,8 % |
| 50 Ω | 21 mV = 0,8 % | 447 mV = 17,9 % |
| 10 Ω | 4,2 mV = 0,2 % | 89 mV = 3,6 % |
| **TIA (virtual ground)** | **0** | **0** |

**The readout is a transimpedance stage**: the chip never sees its own signal. The price is noise
gain against the die's 3,4 nF terminal capacitance. The pulse is not stretched at the chip nor
shaped after it — it is **sampled**, and the shaping is arithmetic on the H7A3.

### Cell occupancy, and why a ceiling from the cell count is understated

At 662 keV in the crystal: 35 700 photons, 3 789 reaching the SiPM side by side, **947 firing a
cell** — 5,0 % of 18 980 cells, 0,050 hits a cell, 0,12 % chance of a double. The flux is divided
among independent cells, so each handles a twentieth of a photon per event:

| event | photons per cell | interval on that cell, at peak |
|---|---|---|
| 662 keV | 0,20 | 5 000 ns |
| 2,6 MeV | 0,79 | 1 268 ns |
| 4 MeV | 1,21 | 829 ns |
| **muon 14 MeV** | 4,22 | 237 ns |
| a TGF at 100× | 19,96 | **50 ns** |

Recovery is ~54 ns (179 fF against ~300 kΩ), so faster recharge buys nothing at ordinary energies.
**It cannot be made faster anyway**: carriers trapped on lattice defects release over tens to
hundreds of ns, and a cell re-armed before they clear fires spuriously — the recovery is set to
outlast them. A cell recovering in 54 ns inside a 1 µs decay fires up to ~19 times an event:

| | ceiling |
|---|---|
| instantaneous flash, sensor alone on the face | 3,5 MeV |
| side-by-side split, instantaneous | 13,3 MeV |
| with recycling over a 1 µs decay | upper bound ~246 MeV |

**The ceiling lies between the bounds** — past ~30 % occupancy the response bends, and the
saturation curve below is what reads it.

**A fast scintillator would hit the cell count hard, which is why the beta block is `EJ-240`.** A
2 ns plastic lands all its light before any cell recovers: Y-90 at 2,28 MeV near 18 % and the muon
(~5,1 MeV through 2,56 g/cm²) near 41 %, past the bend. 285 ns spreads the light over five recoveries
— Y-90 near 2 %, the muon 4,5 %, Rh-106 3,1 %, all linear, so the muon is a ruler on the block too.

**A PTFE-wrapped crystal is an integrating sphere**: the light leaves the window nearly uniformly
wherever the event happened, the residue inside the 7 % resolution. **The saturation correction is
one measured curve against total light.**

**The wrap is PTFE thread-seal tape — the plumber's reel, sold everywhere.** White, unsintered,
unpigmented and **with no adhesive**: an adhesive layer absorbs the blue. The denser grade sold for
gas joints reflects more for the same number of turns. It is wound to **at least 0,5 mm in total**
— five turns of a 0,1 mm tape — because a thin PTFE layer passes light and its reflectance climbs
with thickness until it levels off; more turns cost nothing. The window face stays bare, and the
light-tight layer — black tape or the housing — goes over the wrap, never under it.

## The PIN channel's front end

**The PIN exists for the TGF**: a burst drives the SiPM into outright saturation (~20 photons a cell
at 100×), where bare silicon with no gain integrates and stays linear. **It is the one place in the
station where a low-noise front end is worth building** — everywhere else the detector brings its
own gain:

| detector | electrons per event | S/N against ~1000 e⁻ | its own gain |
|---|---|---|---|
| GM tube | 3,1·10⁸ | 312 000 | gas avalanche, ~10⁸ |
| SiPM, gamma @662 keV | 2,8·10⁹ | 2 840 000 | Geiger, 2,8·10⁶ |
| He³ tube (Helion) | 1,25·10⁶ | 1 248 | gas gain ~10⁴ — its floor is the tube's leakage |
| **PIN @662 keV** | **1,2·10⁴** | **12** | **none** |

**The noise is set by the diode's capacitance**, so the specification is a small diode: 100 pF gives
1940 e⁻ and S/N 6 at 662 keV, 50 pF 970 e⁻ and 12.

### A bipolar common-base stage belongs to a fast channel

ENC in electrons, 50 pF detector, 16 segments where segmented:

| shaping | op-amp 1× | op-amp 16× | CB 16× @1 mA | CB 16× @10 µA | CB 16× @1 µA |
|---|---|---|---|---|---|
| 5 ns | 19 421 | 4 855 | 31 609 | 3 223 | **1 184** |
| 20 ns | 9 711 | 2 428 | 63 207 | 6 329 | **2 024** |
| 100 ns | 4 343 | **1 086** | 141 333 | 14 134 | 4 472 |
| **2 µs** (CsI) | 971 | **245** | 632 061 | 63 206 | 19 988 |

The emitter current's shot noise `√(2q·I_E)` integrates over the shaping time: capacitance noise
falls as `C/√τ`, bias-current noise rises as `i·√τ`, and they cross near 50 ns. A common-base stage
wins a fast channel and loses this one by 20–80×; the same arithmetic excludes **every bipolar-input
op-amp** from the charge node. **A FET/CMOS input is the entry condition; among those, voltage noise
decides.**

### The front end — two op-amp types

**Every channel is at most two stages, a charge stage into a `THS4551` that drives the converter.**

- **`LTC6268`** (dual **`LTC6269`**) — the charge amplifier on every PIN segment and nothing else;
  already Helion's part. Unity-gain stable, 500 MHz GBW, 4,3 nV/√Hz, **5,5 fA/√Hz and 3 fA bias**.
- **`THS4551`** — the differential driver on every converter input, and **on a SiPM channel the whole
  front end**: the anode on its summing node, `Cf` ∥ `Rf` in both arms, the outputs into the RC at the
  converter pins, `VOCM` from the converter's `VCM`. 135 MHz GBW, 3,3 nV/√Hz, 1,35 mA. The 150 pF at
  the pins takes the sample-and-hold's kickback, so the driver's bandwidth serves the ≤ 6,5 MHz signal,
  not the encode rate — `THS4541` stays Marconi's.

| channel | stage 1 | stage 2 |
|---|---|---|
| SiPM, the crystal | — | `THS4551` as the charge stage: **`Cf` 1 nF ∥ `Rf` 1,5 kΩ**, tail **1,5 µs** — one microcell a step of **3,66 LSB**. The block's channel on Positron is the same stage at 1,5 nF ∥ 402 Ω, 603 ns, one cell 2,44 LSB; the photomultiplier's at 10 pF ∥ 10 kΩ, τ ≈ 100 ns — their boards' values |
| PIN, 1–4 segments | `LTC6268` a segment, `Cf` 10 pF C0G ∥ `Rf` 4,7 MΩ, then a **CR differentiator with pole-zero, τ = 1,5 µs** | `THS4551` sums the segments at its virtual ground, gain ~50 |
| dose duty | the crystal PIN in current mode | |

**Every channel hands the converter the same pulse — unipolar, ~1,5 µs, back on the baseline within
~5 µs** — so one trigger and one area algorithm serve all. The PIN is a bought part, **the quadrant,
four segments on a 10 × 10 mm die**, or a single diode; the board carries **1–4 inputs by
population**, one `LTC6269` to two segments.

- **`Cf` = 10 pF, not 1 pF.** ENC does not depend on `Cf`; 1 pF cannot survive the 0,2–0,5 pF of
  parasitics across the feedback, which move with humidity. The deliberate capacitor dominates the
  parasitic; its absolute value calibrates out through the rulers.
- **`Rf` in megohms, and the pulse cut after the gain.** `Rf`'s thermal current `√(4kT/R)` at 1,5 µs
  shaping costs ~220 e⁻ at 10 MΩ, ~380 at 4,7 MΩ, ~1 900 at 200 kΩ. So `Rf` stays 4,7 MΩ, its 47 µs
  tail lives only at the `LTC6268`'s output, and the CR with its pole-zero resistor trims what the
  chain sees to 1,5 µs without undershoot. Three passive parts a channel.
- **The gain is stage 2's.** At ×50 the `THS4551` keeps 2,7 MHz against 1 MHz of pulse content; the
  muon's 29 fC lands at 145 mV ≈ 1 200 LSB over a ~6 LSB floor.
- **Speed is `GBW × Cf`.** The `THS4551` with 1 nF holds the SiPM node to `I/(2π·GBW·Cf)`, 1,2 Ω: 9 mV at
  a 662 keV peak (0,4 % of overvoltage), ~20 mV at a real muon, 200 mV = 8 % only under the
  instantaneous-flash bound — which is the PIN's event by design. **`Cf` is set by the ruler, not by
  the range**: the one-cell step `Q/Cf` must stand several LSB above the converter's 1,29 LSB of
  noise for the cumulants below to be read, and 1 nF puts it at 3,66 LSB. At 10 nF it was 0,37 LSB and
  no statistic recovers a step a third of the noise. What 1 nF costs: the noise gain against the die's
  3,4 nF rises to 4,4 and the channel's floor to 1,41 LSB; the node's residual impedance is 1,2 Ω
  against 0,12; and full scale on this channel falls to **~3,3 MeV** at the slow light's peak, above
  the 700 keV where the PIN takes the energy over. The muon rails the amplifier, the charge parks on
  the node and `Rf` sweeps it in ~5 µs — the dose duty asks for 10 ms.

### The noise budget, and what the threshold is

Quadrant channel, 25 °C, 1,5 µs shaping, **biased at ~25 V off the SiPM's supply** — ~42 pF a
segment, against ~25 pF fully depleted at 70 V:

| term | e⁻ | scales as |
|---|---|---|
| `LTC6268` voltage noise × node capacitance | ~1 490 | `e_n·C/√τ` |
| `LTC6268` current noise | ~9 | `i_n·√τ` |
| `Rf` 4,7 MΩ, four | ~380 | `√(τ/R)` |
| diode leakage, ~1,2 nA at 25 V | ~150 | `√(I·τ)` |
| **total** | **~1 550** | **threshold ~260 keV** (3σ against ~18 e⁻/keV on the crystal) |

A single diode lands at ~2 050 e⁻ / ~340 keV; at 70 V the quadrant would be ~990 e⁻ / ~165 keV.
**Nothing below ~500 keV consumes the threshold** — the SiPM owns the spectrum from ~30 keV, the PIN's
work starts at the handover — so 260–340 keV keeps 1,5–2× of margin. The first row is the diode's
capacitance; the ~400 e⁻ under it is the resistor and the leakage, which no op-amp moves.

**Temperature moves the diode**, its leakage ~1,09× per °C — doubling every 8 °C:

| diode at | leakage at 25 V | its ENC | quadrant threshold | DC across `Rf` |
|---|---|---|---|---|
| 25 °C | ~1,2 nA | ~150 e⁻ | ~260 keV | 6 mV |
| +60 °C | ~24 nA | ~700 e⁻ | ~280 keV | 0,11 V |
| +85 °C | ~220 nA | ~2 000 e⁻ | ~420 keV | 1,0 V |

**The diode is rated −40 °C or it is not fitted.** The catalogue 10 × 10 mm PINs stop at −20 °C
(Hamamatsu `S3590`/`S8650`, First Sensor `PS100-6`) or −10 °C (OSI `PIN-10DP`) — the epoxy package,
not the silicon. The diode sits inside the sealed assembly, so **the assembly is bought rated −40 °C
with its PIN**; Excelitas `VTH3020`, a chip-on-board die, is the one to ask about.

### The PIN's bias

**The PIN rides the SiPM's bias, through 1 MΩ and 100 nF C0G 100 V to the common cathode**, taken off
the `LT3571`'s output ahead of the SiPM's 2 × 49R9 filter. The bias sets the capacitance and nothing
else. A 15 mV servo step a minute reaches the node as ~10 pA; the RC divides the switching line by
~10⁶. **A second `LT3571` at 50–70 V** would buy a 165 keV threshold nobody consumes and put a
switcher beside the quietest node on the board.

**The shaping stays 1,5 µs**: at the 10⁵ counts/s the converter keeps separated it is already ~10 %
occupancy, and the CsI window still integrates 2–3 µs of area over it.

### The converter's kickback, and the SiPM's peak current

The `AD9251`'s sample-and-hold kicks charge back 44 million times a second; the 150 pF absorbs it,
the 24,9 Ω arms recharge it over the whole period, and the residue is synchronous — a pedestal the
baseline subtracts. No isolation buffer on the SiPM channel (`../../WHY.md`).

**The output must sink the SiPM's transient through `Cf`**: `THS4551` ±45 mA at 25 °C, ±30 mA over
temperature, against 8–31 mA on real CsI events. The 167 mA instantaneous-flash bound exceeds every
amplifier; past the limit the charge parks on the node for tens of ns and is swept after — conserved,
at the cost of a transient sag on that one event class. A burst past that is the PIN's by design.

### Two boards, and the PIN is what separates them

- **`Quark-Photon`** — channel 1 a SiPM into `THS4551`, channel 2 the 1–4-segment `LTC6268` PIN input,
  **and the guard ring**. The crystal's SiPM and PIN must be simultaneous.
- **`Quark-Neutron/Positron`** — channel 1 the photomultiplier's anode, channel 2 the block's SiPM,
  both into `THS4551`. **No `LTC6268` and no guard ring — absent, not unpopulated.** A charge stage
  does not care what delivers the charge; the quantity is a scale factor in firmware.

**The heads are on the boards' underside, windows down, and there is no head connector.**

**The guard ring** protects the `LTC6268`'s high-impedance node against surface leakage across the
PCB. Twelve thousand electrons do not swallow it the way a tube's million do, least of all in an
outdoor enclosure where water condenses.

### The chain into the converter — no shaping amplifier

```
SiPM -> THS4551 charge stage (tail 1,5 µs) ----------------------+ the RC at the pins -> AD9251-80
PIN segment(s) -> LTC6268 -> CR+pole-zero 1,5 µs -> THS4551 sum+gain --+
```

**Nothing is stretched; the shaping is digital**, on the H7A3. **The converter's front end is
Marconi's entire**: the `AD9251` takes **2 V<sub>PP</sub> differential into an unbuffered
switched-capacitor sample-and-hold about `VCM` = 0,9 V**, so the pins carry **24,9 Ω ×2 in series plus
150 pF differential**, a corner at 21,2 MHz, and **200 Ω ×2 to `VCM` behind 0,1 µF ×2** as termination
and bias. **That network is the anti-alias filter**: Nyquist is 22,02 MHz at 21 × 2²¹, the fastest
feature the ~54 ns single-cell pulse at ~6,5 MHz, and 21,2 MHz passes it whole and folds nothing back.

## The dose duty rides the crystal PIN

**Dose is deposited energy, and silicon reads it linearly**: 3,6 eV a pair, constant with energy and
particle — no quenching, no photon statistics. On an ordinary day dose is the sum of the pulse areas
the channels already measure.

**In a burst the crystal PIN, read in current mode, is a solid-state ion chamber**: no dead time, no
pile-up, a **dose-rate waveform at microsecond resolution, stamped to ±1 µs against the clock Tesla's
sferic stands on**. Current mode is a firmware regime of the same channel; the leakage, doubling per
~8 °C, is subtracted by a running baseline. **A bare diode is not fitted** — 300 µm of silicon stops
0,54 % of 662 keV photons (`../../WHY.md`).

### What a TGF is, and what this channel says about one

**Bremsstrahlung**: a thundercloud's field runs electrons away into an avalanche, and they radiate as
air nuclei deflect them. **A falling continuum with no lines**, ~10 keV to tens of MeV, most of it in
the hundreds of keV, in a burst of tens to hundreds of µs. The crystal counts it; it cannot give the
energy of the high tail — containing 10 MeV takes ~6 radiation lengths, ~10 cm of CsI against a 25 mm
cube. **What the station wants is the time**, stamped against Tesla's sferic; the spectrum is an
array's or a satellite's.

A ground-level microsecond TGF is rare; the minutes-scale glows (TGE) are what a ground station
usually sees, and the counting channels carry those. **The SiPM's own bias current is a flag, not a
measurement** — it saturates by cell exhaustion exactly when it is needed.

### The photonuclear reaction has four signatures and three of them are gamma

A flash driving `¹⁴N(γ,n)¹³N` leaves a sequence this head reads without a neutron sensor:

| signature | when | energy | on this head |
|---|---|---|---|
| the flash | < 1 ms | continuum to tens of MeV | saturating; the burst record |
| **capture on hydrogen**, `¹H(n,γ)²H` | 10–60 ms | **2223 keV** | **a line, inside the ceiling** |
| capture on nitrogen, `¹⁴N(n,γ)¹⁵N` | 10–60 ms | 10 829 / 5269 / 1885 keV | above the ceiling; overflow |
| **annihilation**, `¹³N` β⁺ | ~1 minute | **511 keV** | **a line, inside the ceiling** |

**Hydrogen is the converter nobody buys** — soil, concrete, timber and plastics are 8–11 % hydrogen —
and `¹²C(n,γ)` at 3,5 mb against its 332 mb does not appear. **A triple coincidence reads it** — the
flash as trigger, 10–60 ms behind it, a known line — with nil background. **It gives no fluence**: the
efficiency depends on the hydrogen around the station, so the reading is *an event of this size*,
independent of `Neutron`'s screens.

**The head must be counting again within milliseconds of the flash**: the burst saturates it
sub-millisecond and the capture gammas follow at 10 ms. **511 keV lands inside the handover blend and
is read there** — the coincidence identifies it, not the resolution.

**A muon crossing the PIN's silicon but not the crystal deposits ~24 000 electrons directly**, ~1,7 MeV
on the crystal's scale, near the hydrogen line in an untriggered spectrum; it separates by shape —
nanoseconds against CsI(Tl)'s microsecond decay.

## The calibration chain — no voltage reference

**The one-cell step is the ruler.** A microcell delivers a fixed 0,447 pC, so every dark count is
the same step at the amplifier's output, `N` = `Q/Cf` in LSB, in the same units as every pulse.
**Dividing an event's area by `N` gives its photoelectrons** up to a shape constant, and
`E = (area/N)·k`, with `k` in keV per photoelectron measured once per head on the bench by the same
arithmetic, so the constant is inside it.

**`N` is read as a statistic of the dark stream, and no cell is ever resolved.** A 6 × 6 mm SiPM at
25 °C fires ~3·10⁶ cells a second; on a 1,5 µs tail four or five are always decaying under any one,
so there is no clean single to histogram and no spacing of peaks to take. What there is, is
Campbell's theorem: a train of identical pulses at random times has cumulants `κₙ = λ·Nⁿ·∫hⁿ dt`, so
**the ratio of two consecutive cumulants is `N` times a shape constant, whatever the rate**. The
ruler takes the fourth over the third — `N = (4/3)·κ₄/κ₃` for the exponential tail — because the
converter's and the amplifier's noise are Gaussian and add nothing to either: **no offset, no noise
floor and no reference enters it**; the mean would carry the amplifier's offset at a DC noise gain
of 14 and the variance the converter's floor, and both drift with temperature. The stream is read
between events, in blocks the scan found no threshold crossing in; crosstalk and afterpulsing are the
part's constants and sit in `k` like the shape. What it delivers, by simulation at 44 MSPS with the
converter's 1,29 LSB:

| dark rate | Photon, 1 nF, one second | Positron, 1,5 nF, one second |
|---|---|---|
| 10⁵ /s | 2 % | 2 % |
| 3·10⁵ – 10⁶ /s | 1–2 % | 2–3 % |
| 3·10⁶ /s | 4 % | 2 % |
| 10⁷ /s | ~8 % | 5 % |

The warm end is the weak one — at `λτ` ≫ 1 the stream goes Gaussian and the cumulants shrink — and
the 16-second running mean the firmware carries brings every row under 1–2 %.

**The ratio cancels** the converter reference, the amplifier gain and the SiPM gain. **It does not
cancel** the PDE's dependence on overvoltage (~5 %/V near 2,5 V), removed by holding the overvoltage,
nor the crystal's light-yield tempco (CsI(Tl) ≈ −0,3 %/°C), which is what the thermometer is for. **The
bias servo's setpoint is the 1 p.e. position in raw counts**, its DAC inside the loop, so it needs no
accuracy.

| measurement | what it is for |
|---|---|
| `N`, the one-cell step | the energy scale by ratio, and the bias servo's setpoint |
| temperature | the **crystal's** light-yield tempco, and the DCR correction |
| dark current | diagnostics: light leak, damage, ageing |

**The thermometer sits at the crystal**, and the SiPM and the PIN each have an NTC position beside
them, fitted — cents against a redrawn board. **None of those two is a correction**: the ratio carries
the sensor's temperature, and self-heating through a burst divides out with the ladder measured in
that frame. They keep the conventional route — bias set from temperature — open in firmware alone.
The photomultiplier's own single-photoelectron peak carries its temperature, ageing and HV drift
likewise. **On `Neutron`, which only counts**, temperature can move the peak sort, and there is no correction
to apply.

**Ageing**: the dark current at a fixed `N` fixes the overvoltage, leaving temperature at
a factor of two per 10 K:

`DCR_corrected = I_dark / 2^((T − T_ref)/10)`

Ageing and damage move DCR by factors; ±1 °C of thermometer error moves it ±7 %. **This deletes**
analog NTC bias compensation, the I–V breakdown sweep and any precision reference in the measurement
path; the bias converter's current monitor is telemetry.

**The ruler is read between events, digitally from the anode.** A 25 mm cube sees ~10³ events/s of
~3 µs, busy 0,3 %, against ~3·10⁶ dark counts/s — at 330 ns mean spacing on a 1,5 µs tail no cell
is seen alone, which is why the ruler is a cumulant ratio and not a histogram. The cell's own 54 ns
rise gets 2,38 samples at 21 × 2²¹ and nothing in the chain measures it: an event is an area over
its tail and the ruler is a statistic of the stream.

## Reference — the numbers this head is built against

### What the beta window passes

CSDA ranges against the two window builds (15,5 and 24,5 mg/cm²), `R = 0,412·E^(1,265−0,0954·lnE)`
g/cm², valid 0,01–3 MeV.

| natural | β max | range mg/cm² | through |
|---|---|---|---|
| **Pb-210** | 64 keV | 6,1 | **no — long-lived background kept out** |
| C-14 | 157 keV | 28,4 | marginal, effectively lost |
| Pb-212 (Th) | 574 keV | 198 | yes |
| Pb-214 (U) | 1024 keV | 425 | yes |
| K-40 | 1311 keV | 576 | yes |
| Tl-208 (Th) | 1803 keV | 840 | yes |
| Ac-228 (Th) | 2069 keV | 983 | yes |
| Bi-212 (Th) | 2254 keV | 1082 | yes |
| Bi-214 (U — the rain washout) | 3272 keV | 1614 | yes |

| release | β max | range | half-life | gamma |
|---|---|---|---|---|
| Xe-133 | 346 keV | 97 | 5,2 d | 81 keV — first out of a breach |
| Co-60 | 318 keV | 85 | 5,3 y | 1173 / 1332 keV |
| **Cs-137** | 514 / 1176 keV | 170 | 30,1 y | **662 keV** — the long-term marker |
| **Sr-90 → Y-90** | 546 → **2280 keV** | 185 / 1095 | 28,8 y | **none** |
| I-131 | 606 keV | 214 | 8,0 d | 364 keV — the early marker |
| Cs-134 | 658 keV | 239 | 2,1 y | 605 / 796 keV; its ratio to Cs-137 dates the release |
| Ir-192 | 675 keV | 247 | 74 d | 316 / 468 keV — the orphan source |
| Kr-85 | 687 keV | 253 | 10,8 y | effectively none |
| Ru-106 → Rh-106 | **3541 keV** | 1751 | 372 d | weak |

**Everything worth measuring passes both windows**; tritium (18,6 keV) and Pu-241 (20,8 keV) need
liquid scintillation. **`Sr-90`/`Y-90` is why the beta head exists** — pure beta, no gamma line, a
calcium analogue that goes to bone — and `Kr-85` is the same case in a noble gas: the one class of
event nothing else in the station sees.

### Alpha emitters — the head is alpha-blind for free

`R_air[cm] = 0,325·E^1,5`, the aluminium column anchored on Am-241 (5,486 MeV → 26 µm).

| isotope | MeV | air | aluminium | mg/cm² |
|---|---|---|---|---|
| Th-232 | 4,01 | 2,6 cm | 16 µm | 4,4 |
| U-238 | 4,20 | 2,8 cm | 17 µm | 4,7 |
| U-235 | 4,40 | 3,0 cm | 19 µm | 5,0 |
| Ra-226 | 4,78 | 3,4 cm | 21 µm | 5,7 |
| Pu-239 | 5,16 | 3,8 cm | 24 µm | 6,4 |
| Po-210 | 5,30 | 4,0 cm | 25 µm | 6,7 |
| **Am-241** | 5,49 | 4,2 cm | 26 µm | 7,0 |
| **Rn-222** | 5,49 | 4,2 cm | 26 µm | 7,0 |
| Pu-238 | 5,50 | 4,2 cm | 26 µm | 7,0 |
| Ra-224 | 5,69 | 4,4 cm | 27 µm | 7,4 |
| Cm-244 | 5,81 | 4,6 cm | 28 µm | 7,6 |
| Po-218 | 6,00 | 4,8 cm | 30 µm | 8,0 |
| Bi-212 | 6,05 | 4,8 cm | 30 µm | 8,1 |
| **Rn-220** | 6,29 | 5,1 cm | 32 µm | 8,6 |
| Po-216 | 6,78 | 5,7 cm | 36 µm | 9,6 |
| Po-214 | 7,69 | 6,9 cm | 43 µm | 11,6 |
| **Po-212** | **8,79** | 8,5 cm | 53 µm | **14,2** |

**No alpha reaches the detector**: only Po-212 passes bare 50 µm aluminium (14,2 against 13,5 mg/cm²),
with nearly no energy left, and the coat stops it. **Alpha is never measured through this window** —
it wants under ~2 mg/cm² — so **radon is read through its daughters**, Pb-214 and Bi-214 by gamma.

### The largest single event

CsI(Tl) at 54 000 photons/MeV electron-equivalent; an alpha carries a **quenching factor of 0,55**.

| event | deposit | **looks like** | photons | SiPM cells |
|---|---|---|---|---|
| Tl-208 gamma, full | 2,62 MeV | 2,62 | 141 000 | 20 % |
| Rh-106 beta, full | 3,54 MeV | 3,54 | 191 000 | 27 % |
| Am-241 alpha | 5,49 MeV | **3,02** | 163 000 | 23 % |
| Po-214 alpha | 7,69 MeV | **4,23** | 228 000 | 32 % |
| **Po-212 alpha — the largest from any isotope** | 8,79 MeV | **4,83** | 261 000 | 36 % |
| Bi-212 β + Po-212 α, piled | 11,04 MeV | 7,09 | 383 000 | 53 % |
| **cosmic muon, through** | 13,98 MeV | 13,98 | 755 000 | 105 % |

**No decay beats the muon**, ~3× the strongest isotope event — the ceiling and the ruler. An alpha
is found at half its energy (internal contamination only; the window stops every outside one).
**Bi-212 and Po-212 pile into one ~7,1 MeV pulse**, Po-212's 299 ns half-life inside the 1 µs
window, matching no isotope.

**Nothing natural lies above 2615 keV** (Tl-208), so **3–13 MeV is a free anomaly window**: a muon, a
neutron capture or a TGF. **The CsI is itself a neutron detector** — capture on I and Cs gives a ~7 MeV
cascade in that window, reported in the top band; a cross-check on `Neutron` and a confounder for the
head-against-head contamination check, which downstream accounts for.

**Range: 30 keV – 5 MeV for isotopes, 30 keV – 15 MeV to keep the muon ruler.**

### Three times, and they are not the same number

| | what it is | this station |
|---|---|---|
| **when it arrived** | the leading edge a trigger sits on | **nanoseconds**, at a few hundred photoelectrons |
| **when the next can be taken** | the amplifier's reset, `Rf · Cf` | 1,5 µs on the gamma board, 603 ns on the block, ~100 ns on the screens |
| **when a moderated neutron is captured** | die-away in the moderator | **tens to hundreds of µs** |

**Only the first is a timestamp**, and a neutron timed against a stroke is limited by the third.

## Survivability — the prompt event, not the chronic dose

Chronic dose is not the threat: 10 Gy takes eleven years at 100 µSv/h. A prompt event is:

| | tube | discrete bipolar | STM32 (CMOS) |
|---|---|---|---|
| total dose | ~immune | 100–1000 Gy | **50–300 Gy** |
| neutron displacement | immune | 10¹² n/cm², loses gain | **10¹⁴ n/cm²** |
| **latch-up** | none | none | **one particle can kill it** |
| **energy to destroy, EMP** | ~1 J | ~1 mJ | **~1 µJ** |

**Latch-up and EMP are met outside the chip, on the run's Galvani boards** — a feed that trips and
restarts breaks the parasitic SCR; **a bit flip takes a watchdog and a state reload.**

**The soft part is the detector**: a SiPM's dark rate climbs from ~10⁹–10¹⁰ n/cm², four orders before
the MCU, and the dark-current trend at constant gain already watches it.

## Deployment — the deposition plate

**The head looks down at a plate, not at the ground.** Soil and concrete shine with K-40 and the
uranium series, moving with moisture; a white PE sheet is radiologically dead, so what the head sees
from the plate is deposition — fallout, dust, rain-washed radon daughters — over the constant floor
of the geology around. **The material is non-critical** — PE, PP, polished stainless.

**~1 × 1 m, slightly raised for drainage, no rim, no cleaning — rain is the maintenance.** Each
shower rinses the old load and lays fresh daughters, every rainfall a measurement with the 26,8 +
19,9 min decay tail. **The roof covers the head only; the plate stands in the rain.** The head sits
**0,5–1 m above the centre, beta window down** — half the height is 4× the counts; a small overhang
settles mud splash.

Bi-214's betas carry ~10 m in air and Pb-214's ~3–4 m. After a washout the surface carries hundreds to
thousands of Bq/m² of daughters — **0,1–1 count/s above background**, readable with the decay curve.
**The plate is relative by design**: dry deposition moves with wind and particle size, so it gives a
constant-geometry trend, not a certified Bq/m².

## The bill of materials

**~200 USD of parts for a complete gamma channel** is the one anchor — the Open-Gamma-Detector's
figure — crystal and SiPM ~120–150 of it. The tube build's parts are `../../tubes/`, Gadolin's
materials `../../tubes/gadolin/CONSTRUCTION.md` §8.

| line | part | price |
|---|---|---|
| detector | packaged scintillator assembly | **the dominant line** |
| photosensor | `MICROFC-60035-SMT`, two — the crystal's and the block's | — |
| bias | `LT3571` class — boost with an integrated current monitor | — |
| bias decoupling | **1 µF ∥ 100 nF at the chip** + 2× 49R9 filter | pennies, not optional |
| PIN charge amplifiers | `LTC6268` / dual `LTC6269`, one section a segment, 1–4 by population | ~4 USD a segment |
| segmented PIN | the quadrant (4 segments, 10 × 10 mm) or a single diode, rated −40 °C inside its assembly | — |
| converter drivers | `THS4551` + 24,9 Ω ×2 / 150 pF / 200 Ω ×2 at the pins | ~2,30 USD each |
| converter | **`AD9251-80`** at 21 × 2²¹ a channel | — |
| MCU | **`STM32H7A3IIT6`**, −40…+85 °C | — |
| thermometers | three NTCs on ratiometric dividers, 3 kΩ + 100 nF at the ADC pin — the leaded `NXFT15XH103FA2B` bonded to the crystal, the 0603 `NCP18XH103F03RB` at the SiPM and the PIN | small |

**Against the tube build this deletes** the He³ tube, the GM tubes and their 400 V bus, and
`Quark-Tubes` with its heads; **it keeps one kV module**, the photomultiplier's, with its film,
diodes, creepage and discharge hazard on its own board.

**The crystal assemblies are bought sealed**: no hygroscopic handling, optical coupling, wrapping or
light-tightness, and no delamination over −40/+60 °C. **The beta block is the one that needs hands** —
a sealed window stops beta, so it takes its own PTFE wrap, its own light-tight housing and a window
specified in mg/cm².

**Suppliers**: **Scionix** (custom assemblies, the entrance window specified in mg/cm²) and
**CapeScint** (the price anchor, SiPM-coupled assemblies too). Hamamatsu `S3590`, ~40 pF at 70 V, is
the reference for the noise figures and not the part — rated −20…+60 °C only. **Not a gamma detector**:
an *evaporated* CsI:Tl layer on a photodiode (First Sensor `X100` class) — 200 µm absorbs 16 % at
60 keV and 0,77 % at 662 keV against the cube's 62 %; the 16-channel `S8559` class is a line sensor.

### What to send out for quotation

1. **Sealed CsI(Tl) assembly**, ~25 mm cube, coupled to a 6 × 6 mm SiPM window.
2. **The same assembly with the PIN beside the SiPM on the one window**, **rated −40 °C with its PIN**,
   the PIN at ~25 V reverse.
3. **Two `⁶LiF/ZnS(Ag)` screens** (`EJ-426` class): the 0,5 mm `HD` and a **thin one, ~0,15–0,2 mm on
   the 0,25 mm clear polyester carrier** — coating weights offered and gamma rejection in writing; a
   `¹⁰B` screen as an enquiry behind them.
4. **`MICROFC-60035-SMT`**, two.
5. **Plastic block, 25 × 25 × 25 mm, `EJ-240`** — catalogue stock at this size — with the **entrance
   window in mg/cm², ≤ 30, confirmed in writing.**

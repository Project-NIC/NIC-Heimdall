★ N.I.C. ★

# Gadolin — detector construction (moderator, reflector, converter)

The **mechanical / nuclear** construction of the Gadolin neutron detector: the moderator +
reflector geometry, the tube array, and the neutron converter. The board **electronics** (HV, the
per-tube front-end, the 13 lines to Quark-Tubes) live in [`HARDWARE.md`](HARDWARE.md); the shared neutron **physics**
lives in [`../../NEUTRONS.md`](../../NEUTRONS.md).

> **Design-stage concept — nothing built, and a theoretical model.** The figures are verified
> against the parts' sheets and the parts before a ring is built. **The dimensions are reasoned,
> not simulated**: the project closes here, and whoever builds a ring first runs the Monte-Carlo
> pass (MCNP / Geant4 / OpenMC) on its geometry — star against cylinder, the layer thicknesses,
> the energy response, the converter's loading — and builds to the result. Of the station's
> radiation builds this is the costliest and the least likely to be built; the model stands as a
> model.

---

## 1. Why a neutron detector is this elaborate — the neutron is neutral

A neutron ionises nothing directly, so **every** neutron detector needs a nuclear middleman that turns
the capture into something ionising. The catch: the *cheap* converters emit products too **soft** to
reach the tube gas — conversion electrons and low-energy gammas die in the wall or the moderator, and
their energy is wasted. So the whole construction is built around delivering a **penetrating** product
to a tube. Two routes survive, and **this board's skeleton serves both**:

| Route | Reaction | Detected product | Reach | Timing |
|---|---|---|---|---|
| **Gd** (default) | `¹⁵⁷Gd(n,γ)` | **7,9 MeV gamma** cascade | metres (penetrating) | prompt |
| **Rh** (variant) | `¹⁰³Rh(n,γ)¹⁰⁴Rh` | **2,44 MeV hard beta** | ~1 cm in plastic | delayed (42 s ½-life) |

Gd is the default — **prompt *and* penetrating**. Rh — the **Rhodion** variant — runs the
same skeleton (§4, §6): same tubes, same lines to Quark-Tubes; the **converter swaps and the inner moderator
re-tunes** (§4) — not a bare element swap.

---

## 2. Common skeleton, swappable converter

One mechanical assembly; only the converter changes. Layers **outside → in**:

```
graphite reflector           (encapsulated — conductive dust vs. the 400 V)
 → thin outer moderator       (polyethylene, NO converter) — thermalise + feed neutrons inward
 → 13× SI-22G tubes           (1 central + 12 in a staggered 6+6 double ring — §5)
 → central moderator core
      Gd variant:  ~1 % Gd₂O₃ dispersed IN the core (8-point star cross-section — §5)
      Rh variant:  core stays plain; Rh sits as foil-on-tube OR a Rh-loaded shell
```

**Starting dimensions:**

| Layer | Start value | Sets |
|---|---|---|
| central core Ø | **~95–100 mm** | hosts 1+12 tubes + does the bulk moderation |
| outer moderator | **~25–30 mm** poly | pre-thermalise + feed inward |
| graphite reflector | **~50–70 mm** | albedo (returns leakage) vs. weight |
| whole can | ≈ **Ø250–290 × ~300 mm** | — |

Note: a *thin* outer moderator alone can't thermalise a MeV neutron (needs ~5–10 cm of hydrogen) — the
**bulk of the moderation happens in the central core** the neutron drifts through, so keep the core
generous (≥ ~90 mm), not minimal-to-fit-tubes.

**Shape — a capsule, not a can with flat ends.** The body is a **cylinder closed by two hemispherical
caps** — a *boiler / PWR-vessel* profile — so the **graphite reflector wraps the ends too**, not just the
sides, and closes **axial leakage out the flat ends**: the hemispherical caps turn
escaping neutrons back into the tube volume, so **the tubes need not run the full length** — a neutron
that slips past a tube end is reflected back for another pass. Cables leave through a **single gland in the
bottom cap**. (A capsule beats a bare cylinder or a conical reflector on neutron economy — it is the
closest practical shape to the ideal leakage-minimising sphere, and it is symmetric.)

---

## 3. Tubes — 13× SI-22G, one tube for both variants

**SI-22G** (СИ-22Г, **Ø19 × 215 mm, 40 g**, a **stainless** cathode, 380–480 V, plateau 100 V at
0,125 %/V, 540 counts per µR, −40…+50 °C), **1 central + 12 in a ring**. *(SI-22G is the large
~20 cm tube; **SBM-20** — ~108 × 11 mm — is the small one, a stainless wall of ~40 mg/cm². They differ
in size and in wall thickness, not in wall material — the same steel-wall family as the Photon
energy-band tubes.)*

**The wall is ~0,25 mm, ~200 mg/cm² — not the 50 µm of a thin-wall counter.** The sheet gives no
thickness; the handbook class is 0,05–0,3 mm for a stainless cylinder, and the figure comes from
the family's own masses: the Ø19 gamma counters SI-20G (175 mm, 35 g), SI-22G (215 mm, 40 g) and
SI-21G (260 mm, 45 g) differ by their cylinder alone, 0,11–0,125 g per mm of length, which on
π × 19 mm of circumference at 7,9 g/cm³ is a wall of **0,24–0,27 mm**; the catalogue's 5 g rounding
bounds it at 0,12–0,37 mm; the thin-wall СТС-6, a larger cylinder at Ø22 × 200 mm, weighs 20 g in
the same tables, half of it. It is a gamma counter and built like one. **The wall is read off one
tube before the converter is specified, without cutting it: the tube weighed to 0,1 g (the ends of
this family weigh ~14 g, the cylinder 0,118 g per mm at 0,25 mm), or an ultrasonic thickness gauge
on the cylinder** — one number, and the loss table below reads off it.

**What the wall costs the Rh beta.** `¹⁰⁴Rh`'s 2,44 MeV endpoint is a CSDA range of 1 180 mg/cm²,
and the spectrum's mean is 1,0 MeV; the share of the spectrum that dies in the wall, for a beta
arriving normally and for one born in a converter lying on the wall, which arrives at every angle:

| wall | stops below | lost, normal incidence | lost, converter on the wall |
|---|---|---|---|
| 50 mg/cm² (0,06 mm) | 0,22 MeV | 6 % | 21 % |
| 100 mg/cm² (0,13 mm) | 0,35 MeV | 11 % | 35 % |
| **200 mg/cm² (0,25 mm)** | **0,58 MeV** | **23 %** | **56 %** |
| 300 mg/cm² (0,37 mm) | 0,76 MeV | 34 % | 70 % |

So of the betas a plated or foil-wrapped SI-22G makes, half leave outward and about half of the
rest die in the wall: **roughly a quarter is counted**, against the half a thin-wall tube would give.
The Gd variant does not care — its 7,9 MeV gamma crosses any wall — and the Rh variant stays a
variant at that efficiency; a build that wants more of the beta takes a thin-wall tube for the ring
and keeps the SI-22G in the centre, which the skeleton allows.

**Stainless, not glass** — cost, ruggedness, mounting, and a conductive wall the Rh can be plated
onto (§4); glass is in `WHY.md`.

Why the **same** tube serves Gd and Rh:
- **Rh-beta:** a GM tube fires **~100 %** on any beta that enters the gas → gas **volume barely
  matters**; capture is set by **ring completeness + wall thinness**. What matters is the wall's
  **areal density** (mg/cm²), not glass vs. steel: the SI-22G's ~200 mg/cm² stops the *soft* 29–250 keV
  conversion electrons whole and the **hard 2,44 MeV beta above ~0,6 MeV passes** (its range is
  1 180 mg/cm², the wall a sixth of it); the loss is the table in §3. **A stainless
  wall is conductive**, so for the Rh variant the foil can even be **galvanically plated straight onto
  the tube** (no wrapping) — see §4.
- **Gd-gamma:** the gamma needs an interaction path → **volume matters** → the big tube wins.
- → the big **SI-22G covers both**: keep the platform (skeleton + tube) common, swap only the
  converter. Thin glass tubes would beat it *only* for beta, lose for gamma, and are fragile.

---

## 4. Converter placement + moderator tuning — what differs between variants

The detected product's **reach** dictates where the converter goes:

| | **Gd** | **Rh** |
|---|---|---|
| Product | 7,9 MeV gamma (penetrating) | 2,44 MeV beta (short range) |
| Placement | **dispersed in the MASS** (star tips, core) | **foil ON each tube**, or a **Rh-loaded shell** |
| Constraint | must be **dilute** (self-shielding, below) | **nothing** between converter and tube wall |
| Make | wax+rosin+Gd-stearate cast, or HDPE+Gd₂O₃ | Rh foil, or ~0,5 mm ABS + Rh powder |

- **Gd self-shields brutally** — it is a ~**254 000 barn** absorber (thermal mean free path ~microns),
  so a thick layer captures everything at the surface and **starves the inner tubes**. Dilute to
  **~1 % Gd₂O₃** → mean capture path ~**6 mm** → captures spread **among** the tubes, not on a skin.
- **Rh must be at the tube** — beta reach is only ~1 cm in plastic, so Rh dispersed in the star tips
  (like Gd) would have its beta **die in the moderator** before reaching a tube. Foil-on-tube = highest
  efficiency (beta born at the wall, ~half enters at once). A **Rh-loaded plastic shell** is one part
  doing **converter + structure + waterproofing** — simpler to build; the cost is that **half the beta
  escapes outward** (a high-Z backing behind the Rh layer backscatters ~30–40 % of it inward).
- **Foil self-absorption is a non-issue — go thin, or just plate the tube.** A classic activation foil
  is **~5–10 µm** (0,005–0,01 mm), a few mg/cm² against the wall's ~200, so what a beta loses it loses
  in the wall (§3) and not in the foil. **Plated tubes** (Rh
  electroplated straight onto the conductive stainless — §3) are the cleanest: even thinner, no wrapping.
- Beta travels **~10 m in air**, so the detector's internal **air gap is irrelevant** — a beta that
  misses the ring easily reaches the **central** or **opposite** tube.

**The moderator re-tunes with the converter, too — not just its placement.** The **inner moderator
thickness differs between variants**, for two reasons: (1) **energy** — Gd wants the neutrons **fully
thermalised** (more moderator), while Rh's **1,26 eV resonance** wants them left **epithermal** (less
moderator, §6); and (2) **product reach** — for Rh the **beta must cross to the tube**, so the moderator
**between the Rh foil and the tube wall is kept thin or absent**, whereas for Gd the penetrating gamma is
indifferent to how much moderator sits between capture and tube. So Gd and Rh share the skeleton but run
**different moderator thicknesses**.

---

## 5. The star core — kept, but for the right reason

Moderation is **not** straight-line flight. n–p scattering is **forward-drifted** (mean lab cosine
**μ̄ = 2/3**; ~**18 collisions** to thermalise from ~MeV), so a neutron entering radially **drifts
toward the centre while it slows** — plus there is a real **tail of near-straight fast punch-through**.
→ a **central (13th) tube is justified**: it catches both the drifted-to-thermal neutrons *and* that
fast tail.

The multi-point **star** core does **not** "equalise path length" (there is no single path). Its real
payoff is:
1. **varied moderator thickness with angle → a broader neutron-energy response** — effectively a
   built-in Bonner set. Cosmic / TGF neutrons are broad-spectrum, so some star tip is always "the right
   thickness" for a given energy.
2. **more Gd↔tube interface** → more uniform capture and gamma collection, less blob self-shielding.

A cast star is harder to make than a cylinder, and it is kept for those two gains; **8 points**.

### The tube ring — a staggered 6+6, not a single circle

Twelve tubes in **one** ring leave **gaps between the cylinders** where a fast neutron can **stream
straight through** un-captured (and, for Rh, where a beta finds no tube to enter). Split them into **two
offset rings — 6 at a smaller radius, 6 at a larger, angularly staggered** — and the outer tubes **cover
the inner gaps** (overlapping solid angle, the tube pattern of a PWR fuel array). The cost is a slightly
larger, more open core; the gain is **higher capture and lower fast-streaming**. It matters most for the
**Rh (beta, ~1 cm reach)** variant, where a tube must physically intercept the beta; for **Gd** the
7,9 MeV gamma is long-range and forgiving of gaps, so the stagger is a smaller win there.

---

## 6. Gd vs Rh — the detection tradeoff (both valid, different tools)

| | **Gd (prompt)** | **Rh (delayed activation)** |
|---|---|---|
| Signal window | ~**ms**, near-zero background | smeared over ~**minutes**, full background |
| Single-strike SNR | **best** (2–3 counts significant) | weaker (√-background penalty) |
| Per-capture detection | ~**1 %** (gamma → GM) | ~**100 %** (hard beta caught on entry) |
| Spectrum | thermal | **+ epithermal** (1,26 eV resonance → grabs faster neutrons) |
| Moderation need | full | **less** (activates epithermal) |
| Best at | one strike | **accumulated / statistical** fluence |

The Rh **42 s ½-life is a smooth exponential** (activity ∝ e^(−t/61 s), **densest right after T0**),
**not** a fixed delay. It is fine for **TGF correlation** because the precise strike time **T0 already
comes from Tesla** — you **separate TIME (Tesla) from COUNT (Rh)** and run a **matched-filter integral**
from T0 (weight counts by the decay curve). So the delay is a **non-issue for this network**; the real
cost is background smearing, largely offset by the ~100 % per-capture efficiency and the broader
spectrum.

### Why *rhodium* specifically (not silver / vanadium / manganese)

Among the activation metals, **¹⁰³Rh is the sweet spot for this job** — the "Rhodion" variant is Rh for
concrete reasons, not by accident:
- **High capture + a big epithermal resonance** (~145 b thermal *plus* a strong **1,26 eV resonance**,
  the largest resonance integral of the practical targets) → it activates strongly **and** grabs faster
  neutrons with less moderator.
- **Hard 2,44 MeV beta** → the part of its spectrum above ~0,6 MeV crosses the SI-22G's wall (§3); a
  soft-beta emitter would die in it whole.
- **42 s half-life = the timing sweet spot for lightning correlation** — long enough that a TGF neutron
  burst (spread over tens of ms) fully activates and you can integrate the decay tail, short enough that
  it **resets between events**. Vanadium (3,7 min) and manganese (2,6 h) are **too slow** to tie to one
  strike.
- **Proven + platable** — Rh is the classic reactor **self-powered-neutron-detector** material (well
  characterised), and it **electroplates** — directly onto the conductive stainless tube (§3, §4).

**Silver is the budget substitute** — hundreds of times cheaper — and is not the default, because
it activates into three half-life windows and a 250-day isomer (`WHY.md`).

**Overlapping strikes → deconvolution, but it lives on the server — NOT on the board.**
Even rhodium's single 42 s window is *not* one-and-done: one strike smears the activity over several
half-lives (42 → 84 → 168 s…), so in a storm the still-decaying activation of earlier strikes **piles up
under** the next ones — the counted β-rate is a **sum of exponentials, one per recent strike**.
**Mind the data path:** the 13 tubes land on **Quark-Tubes' EXTI** and are merged there in software —
tubes lit by one particle count once — and **Quark-Tubes counts the result into its neutron channel K4**,
sent in its mini frames on an Argus segment (`../BUS.md`). The detector board is a **dumb pulse source**
with no MCU on it at all, and it does **not** do the decay maths. Attributing the overlapping decays to individual strikes — hold a
decaying accumulator, correlate the counted rate against the recent **Tesla T0** times, solve the
**linear** (known-times, known-λ) system for each strike's neutron count (bounded to ~5 half-lives ≈
4 min) — is done **where the cross-detector TGF correlation already lives: the server**, from
the timestamped K4 rate. **Node stays dumb, server does the trigonometry** — the project's standing rule.
So **no extra processor**. Honest cost: Rhodion still **needs that server-side deconvolution**, whereas
the **Gd variant is prompt / dumb-count** (strike → γ → tick, done) — a real point in Gd's favour.

**Reading it — running decay-subtraction (and why the timescale split makes it clean).** The β-rate is
a slow background (ambient activation + stray pulses) plus a series of **sharp rises**, one per neutron
shower, each trailing the **42 s decay tail**. A new shower lands **on the still-decaying tail** of the
previous one(s), so its neutron count is the **excess over the predicted decaying inventory** at that
instant — *you subtract the previous shower's tail to read the new one*. It works cleanly because the
two timescales are far apart: the **rise is fast** (neutron burst + thermalisation, ≪ 1 s, sub-second
moderator die-away) while the **decay is slow** (42 s), so each shower is a **sharp step on a smooth
decaying baseline**, and **Tesla's T0 marks exactly when** the step lands. Algorithm — a **running
inventory**:
1. hold **N** (activated nuclei); each tick decay it `N ·= e^(−λΔt)` → predict the expected rate;
2. at each **Tesla T0**, the **excess** of measured pulses over the prediction = that shower's neutron
   count; add it to N and continue.

Equivalently: known T0 + known λ → a **linear least-squares** fit for each shower's amplitude. Either
form needs the **β-rate and the Tesla T0 together** — which is why it lives **on the server**,
not on the dumb detector board.

The activation-target comparison (Ag / Rh / V / Mn) is in [`../../NEUTRONS.md`](../../NEUTRONS.md).

---

## 7. Reflector & other notes

- **Graphite must be clean (boron-free)** — boron contamination absorbs neutrons and the reflector
  **stops working**. Encapsulate it (lacquer / potting) against the **400 V** (conductive dust).
- **Optional lead ≥ 5 mm** outermost for gamma-background discrimination — weight cost, fit only if the
  background data asks for it.
- The whole assembly also needs a **load-bearing frame + water sealing** (the outer plastic / shell can
  double as this — see §4 Rh shell).

---

## 8. Materials — what to buy for one ring

| item | quantity | source |
|---|---|---|
| `SI-22G` GM tube | 13 (1 centre + 12 in the ring) | Soviet surplus |
| HDPE / PP — the moderator, the core and the outer shell together | ~4 kg — ~3,7 L at the start dimensions (§2), the tube bores taken out; ~3 kg where the core is cast in wax | industrial supplier |
| Gd₂O₃ powder, < 10 µm — the Gd converter, the HDPE route | 10 g — 1 % of the ~1 kg core | — |
| GdCl₃ — for the Gd stearate, the wax route | 10 g — ~35 g of stearate, 3–4 % of the core (`../../NEUTRONS.md`) | — |
| stearin (stearic acid) | 50 g | hobby shop / pharmacy |
| carnauba wax — the wax route's core | ~0,7 kg | hobby shop |
| rosin — the same | ~0,3 kg | hobby shop / soldering |
| Pb sheet, 5 mm — only where the optional outer shield is fitted (§7) | ~0,4 m², ~22 kg | hobby market |
| graphite block or plate, boron-free — the reflector | ~8–13 L, **~15–23 kg**, at 60 mm round the Ø 155 moderator, the caps included | industrial |
| silicone varnish | 50 ml | hobby shop |
| two-part epoxy | 50 ml | hobby shop |

**Never machine beryllium** — graphite is the reflector (`../../README.md`, *Safety*). The Rh
variant swaps the Gd converter for rhodium foil (§6); the rest of the table is the same.

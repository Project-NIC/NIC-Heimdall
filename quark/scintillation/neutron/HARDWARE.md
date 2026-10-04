★ N.I.C. ★

# Neutron — the photomultiplier head and the screen

> **Design-stage concept — nothing built.** The figures are verified against the parts' sheets and
> the parts before a head is built. What `Neutron` is: [`README.md`](README.md); the board its
> anode lands on, channel 1 of `Quark-Neutron/Positron`, is [`../positron/HARDWARE.md`](../positron/HARDWARE.md); the
> firmware's three-window sort is [`../photon/FIRMWARE.md`](../photon/FIRMWARE.md) B6.
> Rejected and superseded states are [`WHY.md`](WHY.md).

**`Neutron` is the low-voltage variant's neutron channel**: the `⁶LiF/ZnS(Ag)` screens read by a
photomultiplier, its anode into a `THS4551` on channel 1 of `Quark-Neutron/Positron`, published as
two counts — thermal and epithermal — in Positron's record. The tube variant's neutron head is
Helion, the He³ / BF₃ tube on `Quark-Tubes` (`../../tubes/helion/`).

## The kV source

**The photomultiplier runs on Helion's kV source, a module of its own** — an `LT8331` flyback on
`750311681`, both secondaries in series, into a half-wave Cockcroft-Walton ladder of 400 V stages,
`FBX`-regulated across the real output (`../../tubes/helion/HARDWARE.md` §2, which owns it whole).
**This head's build of it**: the ladder populated to **three stages**, 1600 V ceiling, the `FBX`
divider tapped for **~1200 V**; **an external clock on `SYNC`** from one of the board's timers,
because the divider's 102 µA sits where Burst Mode's envelope would be in the kHz; the 1,8 kΩ /
100 pF damper across the primary. The source's board stands on the enclosure wall, cabled to the
base, which carries the HV alone.

## The photomultiplier head — a screen read by a tube

**This is the `Neutron` channel**: the neutron-sensitive part is **a stack of two thin
screens with a moderator between them** — the `⁶LiF/ZnS(Ag)` screen for the thermal neutrons, a
sheet of PMMA that slows the epithermal ones, and a second, transparent screen that catches what
the PMMA slowed (*The stack*, below) — and the reader is a photomultiplier on the kV source (above).
Two screens, two counts: the thermal and the epithermal, told apart by the height of the pulse.

**Why a tube and not a SiPM.** A 6 × 6 mm die must see a screen of tens of cm², which forces an
acrylic body between them and makes the screen's own self-absorption the number the whole light
budget stands on (`WHY.md`, *The acrylic body between the screen and a SiPM*). A photocathode is the size of the
screen, so the screen bonds straight to the window and that number stops mattering.

### The stack — two screens and the moderator between them

From the flux inward:

| layer | what it is | what it does |
|---|---|---|
| **`EJ-426HD`, 0,50 mm, on its aluminium backing** — `⁶LiF:ZnS(Ag)` 1:2 by mass, ⁶Li 1,07·10²² /cm³ at ≥ 95 % | the thermal screen, outermost; the backing is its reflector | captures the thermal population, below ~0,5 eV — the cadmium cut-off — at **36 %** (the sheet's figure); passes what is faster |
| **PMMA sheet**, its thickness set on the bench | the moderator | slows the epithermal neutrons that crossed the first screen; transparent to the ZnS light |
| **the second screen** — `EJ-426HD-PE`, the same matrix on its **0,25 mm clear polyester carrier**, the layer **0,15–0,20 mm** — a custom coating weight: Eljen's catalogue stops at 0,32 and 0,50 mm and quotes custom variations, Scintacor makes any thickness to order (its ND, 2:1, free-standing — its polyester carrier is white and opaque); **no gadolinium** | the epithermal screen, against the window | captures what the PMMA slowed — **12,5 % at 0,15 mm, 16,3 % at 0,20 mm**, 25 % at the catalogue 0,32 — by the sheet's own rule, `1 − e^(−nσt)` with σ the 941 b at 0,025 eV averaged over a Maxwellian, 834 b, which reproduces every figure on the sheet; thin so the outer screen's light passes through it and a gamma leaves next to nothing |
| the window | | |

**Both screens are catalogue parts in one order.** Eljen cuts to 400 × 500 mm and Scintacor to
500 × 500, both heat-formable; the outer screen is ordered on the 50 µm aluminium foil or the
0,4 mm reflective aluminium plate, the inner on the clear polyester, and the two together capture
**~46 %** of a thermal flux before the PMMA is counted. The prompt scintillation decays in 200 ns and
**the ZnS tail reaches 10 % at 80 µs** (Scintacor's figure), which is what the readout's peak sort
is built for.

**Why two.** One screen on a 1/v cross-section captures the thermal population and nothing above
it; the second, behind the moderator, adds the epithermal decade and lifts the head's capture
fraction above ~15 %. The two are read as two counts — the thermal and the epithermal — so the
head says not only *a neutron* but which population it came from.

**A third layer is open to a build**: thin screen · PMMA · thin screen · PMMA · thin screen against
the window, every screen a thin one so each passes the light of those outside it. Each layer adds
its capture of a population the PMMA has slowed further, and the stack counts more; the condition
is that the outermost screen's light, attenuated by the screens inside it, stays clear above the
threshold (*The readout — a charge stage, and the peak says which screen*).

**Every interface is bonded, and the outside is white.** Each layer is glued to the next with an
optical adhesive: an unbonded face reflects at the index step and the light of the outer screen
never reaches the window. The stack's rim takes PTFE tape as the reflector — white, unsintered
thread-seal tape with no adhesive, at least 0,5 mm in total — and the outer face has the outer
screen's own aluminium backing.
**The layers are fitted to the tube's window, whatever tube is chosen** — nothing crosses an air
gap, so the stack must lie on the glass over its whole face: on a 5-inch tube with a curved
faceplate the screens and the PMMA are heated and formed to it; a tube with a flat cover glass —
the 3,5-inch class — takes the stack flat, which is what put those tubes on the candidate list.

**Why the second screen is `ZnS` too, and not a transparent film.** A transparent `⁶Li` film with
enough lithium to capture does not exist as a part: the transparent `⁶Li` plastics are laboratory
materials at 0,4–3 % `⁶Li` by weight, and at that loading half a millimetre captures ~2 % of the
thermal neutrons and nothing epithermal; the one transparent `⁶Li` material sold is glass
(`WHY.md`). A `ZnS` layer is opaque above ~0,3 mm and translucent below it, so a thin second
screen passes the outer screen's light attenuated by a fixed factor — which the height windows
absorb, and which is one more thing that separates the two screens' heights. **On a flat window
only**, a **`GS20` `⁶Li` glass disc of 0,5 mm** is the alternative second screen: transparent at
450 nm, ~50 % capture of what reaches it, ~10× less light per capture than `ZnS`; rigid, so it
never forms to a faceplate.

### The optical joint — and the `ZnS` never touches the window

On a flat window the stack is built on **its own glass disc** cut to the photocathode diameter,
and that disc meets the window — **glass to glass**, and nothing of the stack's adhesive reaches
it. On a curved faceplate there is no disc: the formed stack's inner face — the second screen's
polyester carrier — meets the glass through the joint directly.

**An air gap is not an option.** Light leaves glass into air only inside the escape cone, so at
`n` = 1,52 a gap costs **~75 %** of the light before the Fresnel loss. The faceplate is flat to
±50 µm, and filling that is exactly what the joint is for.

| | |
|---|---|
| **optical grease** | best index match, reversible, but migrates over years and gives no mechanical hold — the disc needs its own retention |
| **silicone optical pad** (`EJ-560` class) — **the joint** | compliant, no migration, wants pressure, slightly worse index |
| ~~rigid optical cement~~ | **not at this diameter.** A rigid bond across a ⌀70–115 mm face between two glasses of different expansion, cycled −30 … +60 °C outdoors, cracks windows |

**The joint is the silicone pad**: it does not migrate over an outdoor life, and the stack's frame
holds the pressure it wants. Grease would want a retention of its own and creeps out of the joint;
rigid cement is ruled out.

### Polarity — negative HV, anode at ground

**The cathode sits at −1200 V and the anode at ground**, so the anode goes through its series
resistor straight into the transimpedance stage's virtual ground. **There is no HV coupling
capacitor anywhere in the signal path.**

The cost is a photocathode at full HV inside a grounded shield, and that is what the shield's **air
gap** buys back: 30 mm of air in series with 2 mm of glass drops the field in the glass from
550 V/mm to **7,2 V/mm**, and the sodium migration and glass luminescence that a touching shield
causes do not start.

### The candidates — the 3 to 5 inch class

| | **`R1307-01`** | **`XP3462`** | `9390B` | `XP82B20D` |
|---|---|---|---|---|
| maker | Hamamatsu | Photonis | ET Enterprises | HZC Photonics |
| photocathode | ⌀70 → **38,5 cm²** | ⌀68 → **36,3 cm²** | ⌀115 → 104 cm² | a ⌀88 envelope (3,5″), the active diameter not published |
| stages · structure | 8 · box-and-grid | 8 · **linear focused** | 10 · linear focused | 10 |
| window | borosilicate, **plano-plano**, n 1,50 | **lime glass**, n 1,54, shape not on the sheet | borosilicate, **K 300 ppm · Th 250 ppb · U 100 ppb**, n 1,49; **plano-concave — the outer face flat** (the `9330B`'s envelope, the same drawing) | **low-K borosilicate**, shape not published |
| QE | 30 % @ 420 nm | ~28 % @ 420 nm | 28 % at peak | 28 % @ 404 nm |
| **gain** | **2,7 × 10⁵ at 1000 V** | **10⁶ at 1350 V** (divider A) · **limiting 3 × 10⁶** — at 1200 V ~5 × 10⁵, so the ladder's ×4 stays under it | 0,7 × 10⁶ at 1000 V · limiting 10⁷ | **3 × 10⁶ at 940–1160 V, 10⁷ at 1150–1420 V** (three tubes, IceCube) |
| **gain slope** (log/log) | ~6 — a curve on the sheet, no figure; eight box-and-grid stages at ~0,75 each | **5,5** | **14,2** | 6,5–8,0 spec, **6,6–7,0 measured** |
| max supply | **1500 V** | 2000 V | 2000 V | not published — driven to 2000 V in the calibration curves |
| anode current, continuous | 0,1 mA | **0,2 mA** | 0,1 mA | not published |
| **pulsed linearity** | **not quoted** | **50 mA / 200 mA** | 30 mA / 100 mA | not published |
| magnetic, gain halved at | not quoted | 0,2 mT ⊥ · **0,1 mT ∥** | **0,1 mT** | not published |
| dark rate | 2 nA typ · 20 nA max at 1000 V | 5 000 /s typ · 10 000 max (0,2 p.e.) | 1 500 /s · 1 nA typ | **800–2000 /s at 20 °C spec; ~70 /s at −30 °C measured, three tubes** |
| timing | rise 8 ns · transit 64 ns | rise 3 ns · FWHM 4 ns · transit 40 ns | rise 5 ns · FWHM 8 ns (single electron) · transit 60 ns | rise 2,5–2,8 ns · FWHM 6,5–8,2 ns · TTS 2–4 ns |
| mass | 190 g | 200 g | 420 g | not published |
| temperature, continuous | **−80 … +50 °C** | −30 … +50 °C (+80 for < 30 min) | −30 … +60 °C | not published, measured to −45 °C |

**The class is 3 to 5 inches.** A larger photocathode is area, and area is counts: the 5″ buys
~0,8 km of range at roughly double the shield, the mass and the volume. Past 5″ the faceplate is
curved too deeply for the stack to be formed to it, so a 9″ is out.

**Between the two 3″ tubes the choice turns on one question.** `XP3462` leads on
everything that is *written down*: it quotes the pulsed linearity where Hamamatsu does not, takes
2 kV so the ladder's third and fourth stages stay available, doubles the continuous anode rating, is linear-focused
rather than box-and-grid, and its mu-metal shield (`MS153`) and socket (`FE2019`) are catalogue
accessories. `R1307-01` leads on temperature — **−80 °C against −30** — and on the window. **The
window does not decide it.** Lime glass carries more potassium and thorium than borosilicate, but
what that buys is a few alphas an hour into the second screen — a baseline, not a cut (*Alpha*,
below) — and `K-40`'s betas, which are single-photoelectron dark counts under every window. Lime
glass is on the Photonis sheet, n 1,54. **What decides is the site's winter against what is written down**: a head that must
run below −30 °C takes the `R1307-01` and its unquoted pulsed linearity is a bench measurement;
one that does not takes the `XP3462`.

**`XP3462` already carries two things this section otherwise has to build.** Its envelope has a
**conductive coating at photocathode potential** under a **black paint** — the first is the answer to
the field a grounded shield puts across the glass, the second is worth more than a factor of two in
dark rate at low temperature, because the glass's own radioactivity makes light that otherwise
returns to the cathode. **It also means the outside of the envelope sits at −HV**, so the shield's
air gap is a shock requirement and not only an electrical one.

**`XP82B20D` is a closed enquiry, not a candidate.** No sheet has been obtained beyond what follows,
and the design does not wait for one: the choice is `XP3462` or `R1307-01`, below, and a build that
gets HZC's sheet may weigh the `XP82B20D` against them on it. What is known comes from IceCube's
characterisation rather than a datasheet — three tubes measured, two of them below the bottom of
HZC's own dark-rate spec, gain and timing in the table — and what is missing is the photocathode
diameter, the supply and anode ceilings, the pulsed linearity, the magnetic sensitivity, the mass
and the divider ratios (IceCube's base divides `3:3:1:1…`, HZC's, not a sheet). The mDOM's
`XP82B2F` is the same tube pin-compatible to Hamamatsu's `R12199` and 45 pieces measured
landed at 1048–1451 V for 5 × 10⁶ — the spread the ladder must absorb. MoEDAL-MAPP's Outrigger
runs the same family (`XP82B2FNB`, "functionally the same as the `XP82B20D`") on a resistive
4 MΩ divider rated **2000 V at 500 µA**, the photocathode at ground and positive HV on the
anode, and quotes **~600 cps** of dark count — a second user's operating point, not a sheet
ceiling (*The MAPP Outrigger Technical Proposal*, v1.0, 2023, §2.2 and §9.2).

### The base — the divider, and the ladder that stiffens it

**The base is the underside of `Quark-Neutron/Positron`, around the tube's socket** — the divider
and the decoupling below sit in that board's HV corner and the tube hangs from the socket, its
shield clamped to the enclosure (`../positron/HARDWARE.md`, *The heads are on the board*); the kV
source is its own board, two wires to the divider.

**The tube's datasheet gives a RATIO and never a resistance.** The ratio is the tube's requirement;
the current is the designer's trade between stiffness and the power the HV supply has to make.

**100 µA is enough and it halves that power.** A capture is **~28 pC** at the anode on average —
53 pC in the brighter screen at the trimmed gain (*The divider is soft*, below), less in the other,
and `ZnS(Ag)`'s light spread of two to three about both. A stroke at 1 km is 1 610/s on the
38,5 cm² cathode, **45 nA** of anode current — **0,045 % of a 100 µA divider**, twenty times inside
the 1 % rule that sets linearity. A near strike's 103 kcps is 2,9 µA and sags the gain ~3 %, and the height windows, being ladder
ratios, move with it (`../photon/FIRMWARE.md` B6). A 194 µA divider costs 232 mW against 122 mW
and buys headroom against nothing.

**`XP3462`, divider A, at 1200 V** — eleven gaps, ratio `0,12 · 0,7 · 2,3 · 1,5 · 1×7`, and note the
**two** focusing electrodes:

| gap | ratio | resistor | volts | against the limit |
|---|---|---|---|---|
| `K–G1` | 0,12 | **120 k** | 12 V | max 20 V |
| `G1–G2` | 0,7 | **750 k** | 76 V | — |
| `G2–D1` | 2,3 | **2M4** | 245 V | `K–D1` = 333 V, limit 250–700 V |
| `D1–D2` | 1,5 | **1M5** | 153 V | max 400 V between dynodes |
| `D2–D3` … `D8–A` | 1,0 × 7 | **7× 1M0** | 102 V | `A–D8` limit 80–600 V |

**11,77 MΩ → 102 µA, 122 mW**, every gap inside its limit, every value E24.

*(`R1307-01` is simpler — its datasheet specifies **ten equal gaps**, `K·G·Dy1…Dy8·A` at ratio 1 each,
so 10 × 1M2 at 1200 V gives 120 V a stage and the same ~100 µA.)*

**The last five dynodes are decoupled to the anode's return**, biggest at the end where the pulse
current is:

```
Dy8 100 nF · Dy7 47 nF · Dy6 22 nF · Dy5 10 nF · Dy4 4,7 nF      1 kV parts, 183,7 nF, 18,4 µC
```

**To ground, not across the resistors** — the pulse current loop is `Dy8 → anode → amplifier → ground
→ capacitor → Dy8`, and to ground is the short way round; through the resistors the loop's own
inductance smears the envelope this channel measures. **X7R, and its loss under DC bias is accepted**: the BOM states the value *at the gap's bias*, not
the marked one, and the last dynode needs 53 pF against 100 nF fitted — 1900× over, so half the
marked value lost changes nothing.

**Where the ready-made base is bought instead of drawn**, `9390B`'s are `C647G` (hardpin), `C636K`
(capped) and `C655G` (flying lead) — the **`H` and `J` suffixes taper the last four gaps `2R·3R·4R·3R`
and triple the pulsed linearity to 100 mA**, which this head does not need, and the `I`/`J` pair want
a regulated 450 V on `k–d1` where `G` gets there with `6R` alone.

### The readout

```
   from the CW ladder
   −1200 V ──[ 10 k ]──┬─────────────────────────────────────────▶  K
                       │                                              (cathode at −HV,
                    [ 10 n ]                                           envelope with it)
                     3 kV
                       │
                      GND
                                         the divider, above
                                              │
          anode at 0 V ──[ 1 k ]──┬───────────┴──────────▶  THS4551 charge stage, Cf ‖ Rf, τ ≈ 100 ns
                                  │                         then the AD9251
                           clamp diodes
                            to the rails
```

**No coupling capacitor anywhere in the signal path** — that is what the negative polarity buys. The
anode sits at ground and goes through its series resistor straight into the transimpedance stage's
virtual ground.

**`Rf` 10 kΩ ∥ `Cf` 10 pF — τ ≈ 100 ns**, which is the bandwidth this channel wants: the time
constant smooths the anode's **train of single-photoelectron spikes** into an envelope the converter
can digitise, and stays short enough that each capture's ~200 ns prompt is a rise of its own on the
output (`../photon/SCINTILLATION.md`, *The chain into the converter*). The series resistor is **kilohms and not megohms** — a
megohm against a few pF would smear the microsecond structure the discrimination stands on, which is
the opposite of the He³ front end's problem, where the signal is slow and the input is femtoamps.

### The gain slope is a tube property and it sets how hard the tube self-limits

| | gain slope | HV sag for half gain |
|---|---|---|
| `9390B` | **14,2** | 4,7 % |
| `XP82B20D` | 6,5–8,0 | ~9 % |
| `XP3462` | **5,5** | 11,8 % |

**The droop loop's gain is that exponent**, so a steep tube chokes itself off a cliff and a shallow
one leans into it. The equilibrium is the same either way — **whatever the flood, the tube settles at
an anode current of order the divider's** — but the sag it takes to get there is 28 % on a tube of
slope 14 and 57 % on one of slope 5,5. **A shallow tube therefore wants its decoupling more**, which
is the second reason the ladder above is fitted.

### The shield — two layers, and a gap

**Mu-metal inside, thick steel outside, neither alone.** They answer different fields: the mu-metal's
permeability shunts the Earth's static field, and the steel's **conductivity** stops the transient —
5 mm of steel is ~54 skin depths at 10 kHz, and a stroke at 100 m puts **0,6 gauss** across the tube,
which is where a `9390B` loses half its gain.

**The Earth's field is not the reason.** A bolted-down station does not rotate, so a constant offset
is a gain setting. **The stroke's own transient is the reason**, and it arrives microseconds before
the neutrons being measured.

- **The gap costs little**: at ⌀195 mm instead of ⌀140 the shielding factor falls 18,9 → 13,8, ~27 %,
  and 0,6 gauss still lands at 0,043 — a couple of percent of gain.
- **The shield ends flush with the photocathode plane and never overhangs it.** 5 mm of steel takes
  10–30 % of the thermal neutrons (iron: 2,56 b capture, 11 b scatter), so a chimney above the screen
  is a well that eats solid angle, and in an isotropic field solid angle is counts.
- **Reflective finish, not black** — the station's own, and it keeps the head off the screen's
  temperature ceiling.

### The lead shield — 3 cm, all round

**The head stands in 3 cm of lead, every side and the screen's face included, with 1 mm of copper
inside it.** What it is for is the TGF: the flash's photons at the ground are mostly hundreds of
keV with a tail to tens of MeV, and the tube takes them as a flood it can only ride in gain collapse.
3 cm of lead takes 300 keV down by ~10⁶, 662 keV by ~40, 1 MeV by ~11 and 5 MeV by ~4 — the bulk of
the flash does not arrive, the collapse is shallower and the head is counting sooner. Ordinary
background gamma needs no shield: it is under the threshold already.

- **It costs the neutrons little.** Lead captures a thermal neutron at 0,17 b — ~1,7 % in 3 cm — and
  scatters it at 11 b without slowing it; in an isotropic field a scatter is redistribution, not loss.
  The rule that keeps steel off the screen's face does not apply: steel captures at 2,56 b.
- **The copper is for lead's own K X-rays**, 75–88 keV, which the shield otherwise throws back at the
  stack.
- **Lead under cosmic muons makes neutrons** — the neutron monitor's principle — and the PMMA behind
  it moderates some of them into the screens, so the cosmic background rises. The event stands
  orders above it.
- **Order, from the tube out**: mu-metal, the air gap, the steel, the copper, the lead. The lead's
  conductivity adds to the steel's against the stroke's transient.
- **The mass is ~110 kg** for a shell ⌀260 × 360 mm over the steel, and the enclosure is built
  around it: the shell stands on the enclosure's floor bracket, and the board and the tube hang
  inside it as before.

### The divider is soft, and the decoupling stops at nanofarads

**The last dynodes are decoupled and the ceiling is ~100 nF.** What the capacitors buy is gain inside
the pulse: 53 pC — a capture in the brighter screen at the trimmed gain — against ~20 pF of stray at the last dynode is **2,65 V of sag** out of ~100 V per
stage, about 3 % within one event, and a capacitor removes it.

**What matters is that they stay in nanofarads.** A `9390B` is rated 100 µA of average anode current
and a TGF's gamma flood asks for far more; what stops the tube destroying its last dynode is the
divider failing to hold those potentials — **the gain collapse is the protection** — and a reservoir
big enough to ride through the flood removes it.

```
100 nF at 100 V holds  10 µC — a tenth of a second of the tube's own rated current delivered
                              at once, and inside its 30 mA pulsed rating
 10 µF at 100 V holds   1 mC — ten seconds of it
```

**Nanofarads are therefore free and microfarads are not.** At 100 nF the reservoir empties in under a
millisecond at any current that could damage anything — the protection is delayed by that
millisecond and not removed — while a microfarad rides the whole flood through at full gain.

**The tube is saturated and useless through the prompt flash, and that is acceptable** — the capture
gammas and the neutrons both arrive 10–60 ms behind it. What the channel owes instead is recovery:
it must be counting again within milliseconds (`../photon/SCINTILLATION.md`).

### The enclosure

**The tube is not what the sealing protects.** `9390B` is rated to **202 kPa** absolute, so the few
kPa a box builds with temperature is nothing to it. What the sealing is for is water — and a fully
sealed box is the *cause*, because its breathing pumps moisture in past any imperfect seal.

- **An ePTFE pressure-equalising vent** into an M12 hole — any membrane vent of the class: an
  expanded-PTFE membrane, oleophobic, IP68 to IEC 60529 with the vent fitted; it passes air and
  vapour and blocks liquid. It also keeps radon at outdoor concentration instead of letting it accumulate onto
  the screen (`../photon/SCINTILLATION.md`, *Alpha*).
- **An IP68 gland** for the cable. **Two holes, not one.**
- **Padding inside.** The envelope is glass and it is the only mechanically fragile part in the
  station — hail, transport, and the mast's own movement.

**The enclosure is the unit's**: `Quark-Neutron/Positron` horizontal, the tube in its shield under
it, the block beside it, the kV source's board on the wall. **Out of it goes the Galvani cable
and nothing else** — anode, base, front end and the kilovolts never leave the box.

## The stack's settings

**The PMMA thickness, the second screen's coating weight (0,15 or 0,2 mm — the outer screen's light
through it against its own capture), the threshold and the height windows' edges are set on the
bench** against the two screens' own pulse-height distributions (`../photon/FIRMWARE.md`
§B6, `0x0016 WINDOWS`).

---

## The screen

### The neutron channel — a screen, not a bulk crystal

**Bulk `⁶Li` glass (`GS20`) has a fault a screen does not: it is thick enough to see gamma.** A
solid of appreciable Z interacts with gamma far more than a thin gas does, so in a strong field
small gamma pulses stack and fake a capture — precisely when the head most needs to be believed.

**A `ZnS(Ag)` screen is thin.** A quarter of a millimetre of ZnS in a binder is nearly transparent
to gamma, while the triton and alpha from the capture stop inside it completely; `ZnS(Ag)` is among
the brightest scintillators there is and is specifically efficient for heavy charged particles.

| | `GS20` glass | `ZnS(Ag)` screen |
|---|---|---|
| gamma blindness | moderate — the false-capture risk | **good** |
| lithium needed | bulk enriched Li-6 | **a `⁶LiF` screen holds a fraction of it** — the boron version none |
| pulse-shape discrimination | weak | **the long `ZnS` tail separates by shape** |
| electronics | a photomultiplier on Helion's kV | **the same** |

**Order of enquiry: `⁶LiF/ZnS(Ag)` (`EJ-426` class) → `¹⁰B/ZnS(Ag)` → a boron-lined tube.** The
first two run on the same photomultiplier electronics; the third is Helion's board with a different fill, an
already-drawn fallback. **The boron screen also exists as a bought counter**: Bridgeport's
`PMT-N2000`, `ZnS(Ag)` + ¹⁰B on PMMA light guides, its own photomultiplier, supply and gain
stabilisation, 5 V at 50 mA, a 3 µs dead time and a **TTL pulse out** — a count, which would hang on
`Quark-Tubes` beside the GM tubes and not on this board. It gives one count where this head gives
two, carries its own moderator where the head wants a bare screen, captures about half of what ⁶Li
does per area, and costs 5 400 € a piece; weighed and not taken (`WHY.md`).

**Its cost:** `ZnS(Ag)` is opaque to its own light — a powder in a binder — so the screen must stay
thin, which caps efficiency per layer. Answer it with **area, not thickness**, the same rule as the
beta block. A **moderator is still required** on every route: `⁶Li`, `¹⁰B` and He³ all capture
thermal neutrons, and there are eight orders of magnitude between a fission neutron and a thermal
one.

#### The capture has no energy window — it is 1/v

`⁶Li(n,α)` has no threshold and no band. The cross-section falls as `1/√E` across the whole range,
so the screen is a thermal instrument and is effectively blind above a few keV.

| neutron energy | `σ` | against thermal |
|---|---|---|
| 0,025 eV — thermal | **940 b** | 1 |
| 1 eV | 149 b | 1/6 |
| 1 keV | 4,7 b | 1/200 |
| 100 keV | 0,47 b | 1/2000 |
| 1 MeV | 0,15 b | **1/6000** |

A broad resonance near 240 keV lifts the curve to ~3 b and changes nothing.

#### The lightning population arrives thermal — the atmosphere is the moderator

`¹⁴N(γ,n)` makes neutrons at ~10 MeV, but they scatter on air nuclei the whole way down and reach
the ground **thermal**; that is how they are detected at all, and the detections 0,5–1,7 km from
the stroke read them as thermal captures. **For the event this channel exists for the moderating is
already done and a bare screen is the right one** — a moderator in front of it does not make
thermal neutrons out of that population, it scatters and absorbs the ones that arrived. A moderator
remains what a *fast* population needs, and that population is the cosmic one.

**Thermalisation is also the erasure of direction.** A neutron that has scattered a hundred times
carries nothing of where it came from, so the arriving field is isotropic and no arrangement of
screens yields a bearing. The bearing is Tesla's, from the sferic; this channel says how many.

#### Orientation does not matter, and the isotropic penalty is 0,81

A screen tilted away from the flux presents `cos θ` of its area and `1/cos θ` of its path, and in
the thin limit those cancel exactly: in an isotropic field the reaction rate depends on the
material's **volume** and on nothing about its shape or its attitude. The screen is not quite thin
— 20 % efficiency on a normal beam is `Σt` = 0,223 — so the cancellation is close rather than
exact:

```
efficiency, isotropic field  =  ∫₀¹ μ [1 − e^(−Σt/μ)] dμ  =  0,161
efficiency, normal beam                                   =  0,200
```

**0,81 of the catalogue figure, whatever the screen's attitude.** Flat, and side by side rather
than stacked — a stacked screen shadows the one behind it.

#### What one stroke delivers

Measured ground fluences: **>31 and >52 n/cm²** at 0,5–1,7 km (two detectors, Wada 2020), and
**~1000 n/cm²** for a near strike into a wind turbine (Bowers 2017). The modelled source is
10¹²–10¹³ photoneutrons from ~1 km, which reproduces the first pair. Air attenuates at roughly
150 g/cm² against the 120 g/cm² of a horizontal kilometre, so **the fluence falls exponentially
beyond the geometry** and range cannot be bought with area:

| stroke at | fluence | counts on 64 cm² | on 256 cm² |
|---|---|---|---|
| 1 km | ~26 n/cm² | ~270 | ~1070 |
| 3 km | ~1,3 | ~13 | ~54 |
| 5 km | ~0,10 | ~1 | ~4 |

**Ten counts — enough to carry a shape — costs 48 cm² at 3 km, 177 cm² at 4 km and 621 cm² at
5 km: each further kilometre is ~3,6× the area.** The cosmic background on 64 cm² is ~75/hour,
which is **0,002 counts in a 100 ms window**, so the event stands five orders above it and the
channel is never background-limited. The distance model rests on a single measured point.

#### The burst is 100 ms — thirteen frames

The neutrons arrive spread over ~100 ms by diffusion in the atmosphere, not by anything in the
instrument. At 128 frames/s that is **~13 frames**, so the event is a resolved rise and decay in
the record rather than one spike, and its shape is measurable.

**The counter does not stall on a near stroke.** ~10 300 counts in 100 ms is **103 kcps**, a capture
every ~10 µs on average. Every capture opens with its ~200 ns prompt flash, and at τ ≈ 100 ns each
prompt is its own rise on the stage's output, standing on the tail of the capture before it, so
**a pile-up is read as its rises** — every rise is an event, its peak measured from where the
previous tail stands at that instant. Two captures closer than ~300 ns merge into one rise, ~3 % of
events at 103 kcps, and are told by a peak about twice a single capture's
(`../photon/FIRMWARE.md` B6). The overflow flag stays for the rate no reading can resolve, so a
close stroke reads as *beyond range* and never as silence.

#### The epithermal screen, and what it is not

`⁶Li` and `¹⁰B` capture on a **1/v** cross-section, so a bare screen weights toward the slowest
neutrons there are. The second screen behind the PMMA is what adds the **epithermal decade** — the
population the first screen let through, slowed to where the cross-section is large again — and it
is read as its own count (*The stack*). Fast neutrons above that stay out of reach and are
not chased.

`EJ-254` (boron in plastic) and `EJ-276` (PSD plastic) are not the second screen. `EJ-254`'s capture yields **76 keVee** — quenched by Birks on short-range heavy ions, sitting
on a Compton continuum from 2 mm of plastic that is ~1000× higher — and its discrimination is a
**delayed coincidence** with the prompt recoil proton, which a thermal neutron never makes. Its
60 °C ceiling and 75 °C softening point are the second objection.

#### The readout — a charge stage, and the peak says which screen

The anode signal is a **train of single-photoelectron spikes** — 8 ns FWHM on a `9390B` — several
hundred of them, dense through the ~200 ns prompt and sparse down the `ZnS` tail, which is at 10 % at
80 µs.
**The channel follows its envelope**: a `THS4551` charge stage, the anode on its summing node,
**`Rf` 10 kΩ ∥ `Cf` 10 pF, τ ≈ 100 ns**, so a capture's prompt is a rise of its own and the tail
decays behind it. **One `Cf` on every head; the tube's gain is set by its high voltage**, the `FBX`
tap trimmed so a capture in the brighter screen peaks at ~70 % of the `AD9251`'s range — the tubes'
own spread, 1048–1451 V for one gain, is what the trim absorbs.

**Every rise is an event, and its peak is the measure.** The firmware takes the prompt's peak above
the level the stage stood at just before the rise — on a quiet baseline that is the baseline, on a
capture's tail it is the tail — so a capture following another a microsecond later is measured as
itself and not added to it. A peak, not an area: the area of a capture runs into the next one's,
and the peak is local.

**The peak alone tells the event.** A capture is hundreds of photoelectrons; **a gamma is a few** —
in a thin screen it leaves tens of keV against the capture's 4,78 MeV, and in the window glass or
the PMMA it is a Cherenkov flash of nanoseconds, one or two photoelectrons, which the stage turns
into a hump of its own width and one photoelectron's height. **One threshold in photoelectrons takes
the gamma out** — every gamma, wherever it struck — and above it the two screens land at different
heights, set by the inner screen's transmission of the outer screen's light and by each screen's own
yield. **Two height windows sort them, thermal and epithermal**; the windows' edges are in ladder
units, the peak against the tube's single-photoelectron peak, never in volts. `ZnS(Ag)` gives a
capture's light with a spread of two to three, so the two populations overlap at their edges: the
neutron against gamma is clean, and the screen a capture came from is a statistical sort.

**The rejection is a measured property of the screen, not a hope.** Scintacor quotes an intrinsic
gamma efficiency of **3,05·10⁻⁸ per gamma** for its 2:1 screen at 300 µm, read at 20 mR/h (the
portal-monitor requirement is under 10⁻⁷), and Eljen's `EJ-420` counts at 10⁷ gammas per neutron. On
a 38 cm² photocathode in a 10 µR/h background — ~20 gammas/cm²/s — that is **two false captures a
day**; at 20 mR/h, 0,05 a second. **What would break the threshold is a gamma flood**, hundreds of
small pulses summing on the stage — which is a TGF's, and the lead shield takes it down before it
reaches the tube (*The lead shield*). **The inner screen's transmission of the outer screen's light
does not bear on the body's size**: a capture is hundreds of photoelectrons against a threshold of a
few, so the outer screen stays above the threshold at any transmission above ~5 %, and what the
transmission does move — the two screens' heights — is set in the windows on the bench.

**No comparator on this channel, for three reasons and any one is enough**: it would count a single
event many times over as each photoelectron spike crosses the threshold; a second capture landing on
the first one's tail starts above the threshold and is never seen; and a count with no amplitude
cannot be told from pile-up.

#### Alpha, and why a film settles it

`ZnS(Ag)` is sold as an alpha detector in its own right, so the screen sees alpha by construction.
Two sources: **radon progeny** plated on the surface (`²¹⁸Po` 6,00 MeV, `²¹⁴Po` 7,69 MeV) and the
**`U`/`Th` in the tube's own glass** — the `9390B` datasheet volunteers `K` 300 ppm, `Th` 250 ppb,
`U` 100 ppb for exactly this reason. **The glass contamination never decays**: at 4,5 and 14 billion
years it is a constant for the instrument's whole life, and only the radon-borne part comes and goes.

**Neither is cut, by height or by shape.** Only an alpha born at the surface arrives with its full
6–7,7 MeV; one born deeper in the glass leaves with whatever is left of it, from zero up, so the
glass alphas are a continuum under the capture's 4,78 MeV and the height windows do not separate
them. Shape does not help either: alpha and the capture products are both heavily ionising and
give the same slow `ZnS` pulse. **What settles it is the rate.** At 100 ppb `U` and 250 ppb `Th`
the window's surface emits ~0,1 alpha per cm² an hour — **a few counts an hour from a 38 cm²
photocathode, into the second screen**, against ~120 thermal captures an hour of cosmic
background and hundreds in 100 ms from a stroke. It is a constant of the tube for the
instrument's life, a flat baseline of a few percent of the background, and the baseline is
removed downstream where every baseline is. The tube head's two thresholds (`../../tubes/helion/HARDWARE.md` §4) are a
different mechanism and cut nothing above the capture.

**External alpha is settled by a film.** A 6 MeV alpha ranges ~50 µm in plastic, so the glass disc
the screen is bonded to, and any cover over it, stop everything arriving from outside. What remains
is what is generated inside the screen and in the 50 µm beneath it.

**The boron screen is an enquiry, not a part.** Eljen's neutron line is `EJ-410` / `EJ-420` /
`EJ-426` and carries no boron screen; the addresses are **Scintacor** and **RC Tritec**. Boron
screens have a poor historical record on light yield, which is why the enquiry order puts them
second. On the lithium side **RC Tritec's `⁶LiF/Zn(Cd)S:Ag`** claims ~50 % more light than the common
formulation and is the upgrade to ask about first.

#### Supply is this channel's real risk, on every route

Enriched Li-6 is export-controlled because Li-6 deuteride is fusion fuel, but the harder fact is
that **almost nobody makes it**: US enrichment stopped in 1963 and most current supply is Russian.
Expect end-use statements, months of lead time and volatile prices. **He³ shares the problem** — it
comes from tritium decay and has been short since 2008 (`../../tubes/helion/HARDWARE.md`) — so
both obvious routes hit the same wall.

**Boron is the way out.** `¹⁰B` has a 3840 b cross-section, is enriched in quantity for reactor
control and for medicine, and **natural boron is already 20 % ¹⁰B**. Helion's README carries BF₃
and boron-lined tubes as the cheaper fill, and boron-lined is what industry moved to when He³ ran
short.

**A part that cannot be re-bought in five years is a design fault whatever its performance** — this
station is built in the plural.

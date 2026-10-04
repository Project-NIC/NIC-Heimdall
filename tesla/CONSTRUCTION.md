★ N.I.C. ★

# Tesla — construction

> **Design-stage concept.** The figures are verified against the parts' sheets and the parts
> before a unit is made.

## The antenna — three untuned ferrite rods at 120°, on a triangular frustum

**A ferrite rod's pattern is a figure-8** (∝ cos θ, a deep null perpendicular to its axis), so one
rod is blind in a whole plane, and the station detects **per rod**, because the CFD needs one clean
edge and combining channels destroys the timing.

**Three rods at 120° in azimuth, tilted at the same slant θ.** A rod cannot tell α from α + 180°,
so what counts is distinct axes modulo 180°: three rods at 120° give three axes 60° apart, and
under per-rod detection the worst direction for a horizontally arriving wave dips **−1,25 dB**
(`HARDWARE.md` §3). **θ is not critical** — it is picked for horizon coverage and buildability. The
amplitudes are not used for direction on the node beyond the supplement's raw azimuth; bearing is
the server's product. The converter has four channels and the fourth is spare.

**The frustum**: three rods on the three sloped sides, **the coils at the outer ends**, far apart
for low mutual coupling; the flat top is the platform for the board, and the pitched shape sheds
rain. Two things make the geometry forgiving:

- **VLF is point-like** — λ is 0,6–60 km in band, so rods a few cm apart are electrically one point,
  with no phase or timing difference from the geometry. Offset them for mechanical convenience.
- **Crosstalk calibrates out** — a fixed geometry makes any coupling a constant linear mixing,
  measured once at commissioning and inverted on the node. The rods need to be stable and known,
  not decoupled.

## The enclosure — one plastic box, rods included

**The whole unit lives in one sealed IP68 enclosure**: the rods, the board and the Galvani boards
behind its sockets. **Its only penetrations are the spur's cables** (*The cable*, below); no sensor
lead leaves the box, so no entry protection is owed on any.

- **Plastic, mandatorily.** Metal would shield and eddy-damp the band the unit receives — the rule
  that bans a metal box near Gauss (`../daedalus/CONSTRUCTION.md`); plastic is transparent at VLF.
- **At ground level, away from the mast.** Height buys sferics nothing, and distance takes the rods
  out of the strike terminal's near field.
- **The board sits under the antenna with its digital corner and its Galvani sockets at the edge
  furthest from the rods** (`HARDWARE.md` §8, *Placement*). Rod-to-board coupling inside one volume
  is the bench's 65 mV criterion at the chain input (`HARDWARE.md` §6).
- **No shield is required.** Layout makes LF coupling negligible, the front-end band-pass takes the
  HF, and the ferrite core itself turns lossy at HF, so HF noise does not enter through the
  antenna. A grounded pour over the digital corner is an option, not a requirement.

## The rod — untuned, specified, and length before everything

**Untuned, broadband.** In this band the winding is an inductor of 78 Ω to 8 kΩ, a low-impedance source,
so it needs no high-impedance or JFET-input amplifier; that rule belongs to tuned antennas (MΩ at
resonance) and fA sources. It is wound for **hundreds of turns, not thousands**, with a
low-self-capacitance winding, and shielded by a gapped Faraday shield against E-field pickup. The
winding's self-resonance lands inside the band and is passivated by the front end's virtual short
(`HARDWARE.md` §2).

**The antenna's figure of merit is the product N · A_eff.** The front end's dominant noise, input
referred, is **B_n = e_n / (ω · N · A_eff)**, independent of `L` and `Rf1`: fewer turns alone raise
the noise (half the turns on the same rod is twice the noise), and the win is a longer core with
turns up to the stop rule. **Material is not a noise lever**: µ_rod is demagnetisation-limited
(µᵢ 800 → 2000 buys ~7 % at `l/d` 15), and the core-loss Johnson term is ~1 nV/√Hz. The levers by yield: **rod
length** — two rods end to end ≈ 2–2,5× the signal, `l/d` 15 → 30 takes µ_eff from 94 to 236,
+8,0 dB for one more ferrite stick with nothing on the board touched → turns → parallel-rod
bundling → `Rf1`.

**The stop rule**: once the electronics sit under atmospheric QRN — 10–20 dB below it outdoors,
where the chain already is — further sensitivity buys nothing, and more turns only double every
in-band carrier at S1. **The winding is 150 turns** (`HARDWARE.md` §0.1).

**Ferrite grade — specified, not junk.** A **known Mn-Zn grade** with a datasheet μᵢ, tempco and
loss; cheap is fine, unspecified is not. No-name coarse-grain rods have uncontrolled μ, tempco and
spread, so they drift and vary, which breaks the network's stability and self-calibration.
Consistency matters far more than raw sensitivity; there is sensitivity to spare. **All three rods
come from one batch, with spares.**

**The selection rule.** Signal ∝ **l² / ln(2 · l / d)** — length squared, diameter nearly
irrelevant. **Length composes**: segments glued end to end count as one rod. Look for:

- total length **300–400 mm** (190 at least), ø 8–16 mm;
- **µᵢ ≥ 2000 for long builds** — past l/d ≈ 25 the demagnetisation limit lifts and the material
  starts to count; a short rod does not care;
- Mn-Zn preferred, for its HF-loss self-shield; B_sat ≥ 250 mT;
- documented — µᵢ, losses, tempco.

Mn-Zn rods are found as welding-rod cores, choke and inductor rods and induction-heating cores;
antenna-rod listings are mostly Ni-Zn. Compare candidates by l² / ln at their composed length —
twice the score is one more bit of noise floor.

| rod | what it is | where it stands |
|---|---|---|
| **200 × 10 mm Mn-Zn, µ ≈ 800** | the largest rod still bought off a shelf at sensible money | **the prototype** — built deliberately short of the length target, to show on hardware whether that much rod already suffices |
| **Amidon material 33 `R33-050-750`**, 12,7 × 190 mm | Mn-Zn µ ≈ 800 with a published µᵢ and tempco; µ_rod ≈ 94 at 190 mm | **the network build's reference** |
| **Stormwise VLF rod** | Mn-Zn µ800, 0,1 Hz–500 kHz, sold for lightning reception | an equal alternative to the Amidon rod |
| **TESLA H7 NiZn antenna rod, grooved, 10 × 164 mm** (ferity.cz — old TESLA stock, documented material, one-batch buys) | Ni-Zn, but at l/d 16,4 demagnetisation-limited to µ_rod ≈ 105, level with the Mn-Zn reference's 94 | **the local option — two end to end per arm**, six rods a station plus spares; ø10 has ~60 % of the reference's core area and the pair more than recovers it |

**What Ni-Zn costs**, both already carried by the design: the Mn-Zn HF-loss self-shield is absent,
so **the input protection — `Rs1` and the `BAV199` clamp — is mandatory, not optional**; and the
tempco is worse, which the digital µ(T) correction takes — L is measured over temperature at
characterisation. A no-name or marketplace rod ("residual 1200 mT" is marketing; real Mn-Zn Br is
≈ 150 mT) is acceptable only as a characterised prototype: wound, L measured, tempco logged.

**Winding**: ~150 turns of 0,315 mm enamelled wire, single layer, sectioned, on the centre third of
the rod, ≈ 2,5 mH, the shield stood off the winding (`HARDWARE.md` §0.1). The turn count is set by
the stop rule, not by the resonance.

## Temperature — one thermometer on each rod

**The rod's µ(T) moves the amplitude and never the edge**, because the CFD is amplitude-
independent, so position is unaffected; the amplitude is corrected digitally on the node from **one
NTC per rod** (`HARDWARE.md` §7). Per rod, because the three faces of the frustum see different sun
and the correction wants the temperature of the rod that produced the amplitude. The network needs
each station stable, not identical, and a logged temperature makes a drifting ferrite a
correctable one.

**Mounting.** The NTC bead is glued to the rod and both leads run along the rod before they leave
it, or the reading drags toward the air. The bead and its leads are conformal-coated — class SR
to IPC-CC-830 / IEC 61086, neutral cure, flexible at −40 °C — because condensation shunts a 10 kΩ
leg and reads as heat. **The wiring never forms a turn around a rod** — that is a shorted secondary, which damps
the rod, moves L and Q and lands its own drift in the calibration. Twisted pair, along the axis,
out at the end.

## The cable

**One cable in**: the spur's data cable and the feed's two-core cable, each on its own gland, or
the hybrid of both in one jacket; both land on the Galvani boards inside the box, and the board
sees logic only. There is no inline connector outside the enclosure.

## Sourcing

The board is **JLCPCB-assembled** — the analogue ICs as LCSC Extended, stocked, genuine parts, the
passives Basic. **The rods and the hand-wound shielded windings are not**: the rods are bought
(EU, Amidon, Stormwise or the TESLA stock) and wound by hand.

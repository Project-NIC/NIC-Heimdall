★ N.I.C. ★

# Positron — the graveyard

Rejected alternatives and superseded states, with the reason. Nothing returns from here and
nothing is deleted from it.

## `EJ-200` as the block material — superseded by `EJ-240`

A fast plastic dumps its light inside one SiPM microcell recovery (54 ns), so the cell count is a
hard ceiling: Rh-106 at 3 541 keV landed at **28,5 % occupancy**, against the ~30 % where the
response bends. `EJ-240`'s 285 ns decay spreads the light over five recoveries and drops it to
**3,1 %**. It costs 37 % of the light, 4,0 → 5,0 % FWHM at Y-90; saturation bends the very endpoint
the channel fits, so the slower plastic wins here.

## The 25 × 25 × 2 mm tile — superseded by the 25 mm block

A 2 mm tile read through a wavelength-shifting fibre in a milled groove, chosen for collection. It
ignored electron range: 2 mm of plastic (1,023 g/cm³) is 0,205 g/cm², and a 0,546 MeV electron
ranges 0,18 g/cm². Every harder beta crossed it minimum-ionising (~2 MeV per g/cm²) and left the
same ~0,4 MeV, so Y-90 at 2,28 MeV landed in the bin of a cosmic muon. **The tile counted; it did
not measure.**

25 mm contains the band (Y-90 ranges 10,7 mm, containment holds to ~5 MeV), and its face takes a
6 × 6 mm SiPM directly at 425 nm, the SiPM's peak, so the fibre went too. The price is gamma: 19,8 %
of 662 keV photons interact against the tile's 1,8 %, affordable because `Quark-Photon` watches the
same plate from the same height on the same clock and the gamma is subtracted.

## 11 mm instead of 25 — dropped on the muon

11 mm contains Y-90 exactly (10,7 mm) and takes 9,0 % of 662 keV gammas against 25 mm's 19,8 %. But
a muon deposits ~2 MeV per g/cm²: **at 11 mm that is ~2,2 MeV, on Y-90's 2,28 MeV endpoint**,
inseparable from the top of the beta band; at 25 mm it is ~5,1 MeV, twice anything beta produces —
an energy marker and a veto. 25 mm is also `Quark-Photon`'s envelope, so one head geometry serves
both units.

## A scintillation CRYSTAL for the beta head — never, at any thickness

Electron backscatter scales with Z: ~5 % off plastic, 40–50 % off CsI — **half the betas thrown
back**, the rest giving partial deposits. High Z also absorbs gamma (photoelectric ~Z⁴–Z⁵): 3 mm of
CsI, enough to contain Y-90, takes 11,4 % of 662 keV photons against the tile's 1,8 %. The figure
of merit is beta over gamma; a crystal degrades exactly that.

## Beta spectroscopy proper — out of scope, a different instrument

Telling isotopes apart by beta endpoint wants full containment and an anticoincidence veto from an
adjacent gamma detector, not this station's statistical subtraction — and a continuous spectrum's
endpoint does not name an isotope by itself. **The block gives the spectrum; identification is not
claimed.**

## Fast neutrons in the beta channel — not rejected, accounted for

The block counts fast neutrons and the base build cannot separate them: a hydrogen recoil proton
gives a pulse the plastic shapes like a beta's. The thickness made it ten times worse, knowingly:
the n-p mean free path at 1 MeV is ~5 cm, so 25 mm interacts with ~40 % of crossing fast neutrons
against the tile's ~4 %. **It stays small** — hundredths of a count a second at ground level, far
under the 19,8 % gamma. Pulse-shape discrimination would clean it (variant C in
`../neutron/WHY.md`), except below roughly 1 MeV of neutron energy.

## The charge stage's `Rf · Cf` — 1,5 µs inherited, then 3,3 nF at 601 ns — superseded by 1,5 nF ∥ 402 Ω

First the gamma head's 150 Ω × 10 nF = 1,5 µs, so both of Photon's channels hand the converter the
same pulse. Here, with `Cf` at 10 nF, full scale sat at 14 MeV and the 1 p.e. peak at 0,37 LSB. With no PIN and no crystal on this board nothing wants a long
tail: `Cf` is the range, `Rf` the tail.

Then 3,3 nF ∥ 182 Ω, τ 601 ns, with `Q/Cf` putting 4,8 MeV at full scale, the 1 p.e. peak at
1,11 LSB, and two windows, for events and the p.e. ladder, their ratio folded into `k`. It went
because **the ladder from resolved cells does not exist at a 6 × 6 mm SiPM's dark rate**, and
1,11 LSB is under the converter's noise anyway (`../../WHY.md`, *The ladder from clean singles*).
The ruler is a windowless cumulant ratio and wants the one-cell step well above 1,29 LSB:
1,5 nF ∥ 402 Ω gives 2,44 LSB at 603 ns. The range at `Q/Cf` falls to 2,2 MeV, but the 4,8 MeV
figure had ignored the slow light: `EJ-240`'s 285 ns into a 600 ns tail peaks at 0,51 of `Q/Cf`, so
Rh-106 lands at 83 % of full scale at 1,5 nF (38 % at 3,3 nF). 1 nF ∥ 604 Ω was not taken: it clips
Rh-106 above 2,85 MeV.

## Pulse-shape discrimination on the beta channel — not read

Shaping and PSD were once one decision: PSD wants an amplifier tail shorter than the
scintillator's slow component (100–300 ns in an `EJ-276` class plastic). It went with the material:
**`EJ-240` is not a PSD plastic** and the fast-neutron term is accounted for by rate (*Fast neutrons
in the beta channel*, above), so the tail is set for the ballistic deficit alone.

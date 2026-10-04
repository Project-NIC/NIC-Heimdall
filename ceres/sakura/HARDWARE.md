★ N.I.C. ★

# Sakura — the board, as Sakura uses it

> **Design-stage concept — nothing built.** The figures are verified against the parts' sheets
> and the parts before a board is made.

**Sakura has no board of its own. The board is Ceres's, whole, and `../HARDWARE.md` is the
document to draw from** — the comb, the drive, the follower, the detectors, `C_ref`, the rails and
their filters, the decoupling, the pin table, the bowl, the fill, the channel, the cable, the parts,
the bench criteria: all of it, unchanged. **Ceres and Sakura are one board, as Bifrost and Argus
are one card**; what a unit is, is the curve in its cells and the type it answers on the arm — 7,
`WETNESS`. This document says what differs: the height of the spacer ring, the back and what holds
the unit, where it hangs, and what the thermometer reports.

| | Ceres | Sakura |
|---|---|---|
| the spacer ring | to the bowl's rim less the disc's thickness — the whole bowl one block, because the sand's load goes through it | **a few tenths above the tallest part** — the buck's inductor at 4,0 mm, a ring at 4,2 — a thin block, because nothing presses on the back |
| the back | the borosilicate disc, flush with the rim | **the same borosilicate disc with a holder glued onto it, or a printed back of a plastic with glass-class moisture properties** (below) |
| where it is | horizontal in the patrona's packed sand, glass down | on a thin arm in the canopy, **glass to the sky at 45°**, no radiation shield, a coarse stainless mesh above it |
| the thermometer reports | the soil temperature at the depth | **the temperature at the plate** — a leaf temperature, not the air's; the air is Palatine's bought pod in its shield |
| type on the arm | 6, `VWC`, 0x18 for NUMBER 0 | **7, `WETNESS`, 0x1C for NUMBER 0** |
| the curve's end points | water, dry sand, a saline solution | the plate dry and the plate flooded; a salted film read in `G` |

## The back and the holder

**The back closes the fill against the weather, so it must let no moisture through to the
compound — glass does that, and a plastic qualifies only if it does the same.** Two builds:

- **the glass disc with a holder glued onto it** — the same borosilicate disc as Ceres's, flush with
  the ring's top, and a holder bonded to its outer face: a printed or moulded piece with a lug, or a
  stainless tab. The glass carries the seal, the holder carries only the load, so the holder's
  plastic is free to be anything that holds a screw and stands the sun.
- **a printed or moulded back with the lug in one piece** — only in a plastic whose water uptake and
  vapour permeability are glass-class, its face toward the fill roughened so the compound bonds.
  Not PLA (takes water, softens in the sun), not nylon (2–3 % water), not PP or PE (the compound
  does not bond to them, `../WHY.md`).

In both, **one screw through the lug onto the arm sets the 45° and holds the unit**, and the mesh
standoffs stand on the lug. Nothing presses on the back, so a plastic there costs nothing the sand
would charge on Ceres; the builder chooses by what they can make and source.

## The arm and the mesh

**On a thin arm in the canopy, glass to the sky at 45°** so water runs off, **with no radiation
shield**: the plate has to cool by radiation to dew and dry in the sun as a leaf does, and the sun
is part of the measurement — copper under glass absorbs 40–60 % of sunlight, about a leaf's share.
Over it **a coarse stainless mesh against hail** — ≥ 60 % open, ~10 mm apertures, 20 × 20 cm,
parallel to the plate ~5 cm above it on standoffs from the lug; **never a solid plate**, because
rain is wetness and the sky view is what forms the dew (`WHY.md`). The mesh costs the plate part of
its sky and makes it a leaf inside the canopy rather than the top one, which is where the disease
models look anyway. There is no standard leaf and no reference instrument for wetness; the disease
models were built against plates like this one, so the plate is the standard.

**Parts beyond Ceres's**: the holder — a printed or moulded lug, or a stainless tab, and the
adhesive that bonds it to the disc — or the printed back; one stainless screw; the mesh and its
standoffs. Everything else is Ceres's and is not repeated here.

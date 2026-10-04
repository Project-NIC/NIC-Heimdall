★ N.I.C. ★

# Palatine — siting and the radiation shield

> **Design-stage concept — nothing built.** The figures are verified against the parts' sheets and
> the parts before a board is made.

How Palatine's instruments are placed: the radiation shield, and the exposure of each. The
brackets and bolts are the builder's.

## Air temperature in a radiation shield

**An air thermometer in direct sun reads high** — a dark element in full sun has been measured
25–35 °C over the air. The 2 m probe therefore lives in a radiation shield, and the shield is a
**passive triple-cylinder**, not a louvred screen: WMO (Vol. I, chapter 2, §2.5) gives a naturally
ventilated screen and a naturally ventilated shield the same fault, up to +2,5 K in sun and calm
and −0,5 K on a clear calm night, and the cure for both is ventilation. A screen buys that with
volume, a stand, a double floor in snow and a coat every two years; concentric cylinders buy it with
a chimney — the gaps between the shells warm in the sun and draw air past the sensor exactly when
the wind does not — and the inner shell is washed on both faces, so its temperature stays the
air's. That is the shield WMO describes as the alternative to the screen (§2.5.2), without its fan.

```
                  cap — two white discs, 10 mm apart, overhanging the outer shell by ≥ 20 mm,
                  standing 15 mm above it so the chimney exhausts under it
          ┌──────────────────────────────┐
          │ ┌──────────────────────────┐ │
   ═══════╧═╧══════════════════════════╧═╧═══════   three threaded rods at 120°, polyamide
    ║  ║  ║         ●  the probe         ║  ║  ║    (never steel — a metal rod carries the outer
    ║  ║  ║     on the axis, 2,0 m       ║  ║  ║    shell's heat to the inner), plastic spacer
    ║  ║  ║    touching nothing          ║  ║  ║    rollers cut from tube between the shells
    ║  ║  ║                              ║  ║  ║
    ║  ║  ╚═════ inner  Ø 50 ════════════╝  ║  ║    all three shells open top and bottom,
    ║  ╚═════════ middle Ø 75 ══════════════╝  ║    the bottoms ≥ 120 mm below the probe,
    ╚═════════════ outer Ø 110 ═════════════════╝    so ground radiation does not reach it
                   ↑ a coarse mesh across the bottom against insects and spiders, nothing finer
```

| | |
|---|---|
| **shells** | three lengths of plastic pipe with low thermal conductivity — PP or HDPE, grey PVC only painted — **Ø 50 · 75 · 110 mm**, the house sewer-pipe sizes, gaps of ~12 and ~17 mm; **~300 mm long**, the probe's element at mid-height, every shell open at both ends |
| **the outer shell and the cap** | **the station's solar-reflective white** (`../daedalus/CONSTRUCTION.md`, *Finish*): solar reflectance ≥ 0,80, near-IR included, emittance ≥ 0,85 — the one surface the sun strikes. **A PP or HDPE shell takes the coat only over a polyolefin adhesion promoter**, because nothing holds on a bare polyolefin; the other way is a pipe bought white in a UV-stabilised compound, its reflectance measured |
| **the inner two shells** | **white, any exterior white** over the same primer, or bought white — not black. The sun does not reach them, but the element exchanges long-wave radiation with the inner wall across the whole gap, and a black wall (ε ~0,95) hands the element every tenth of a degree the wall sits off the air, warm by day, cold by night under the open top. A white wall keeps that exchange small and costs nothing |
| **spacers** | three **polyamide threaded rods M6** at 120°, through drilled holes in all three shells, with **rollers of plastic tube** cut to the gap lengths between them; nuts above and below the outer shell. One rod set at the top third, one at the bottom third |
| **the cap** | two discs of the outer shell's material, **Ø 150 mm**, 10 mm apart on the same rods, the lower one 15 mm above the outer shell's rim — the air between them is the roof's second layer, and the overhang shades the gaps at any sun above ~30° |
| **the probe** | screwed to a cross-bar on the inner shell's axis, cable down through the bottom; **no part of it touches a shell** |
| **mounting** | a clamp to the **sensor post**, the shield's axis vertical, the element at **2,0 m** above the ground |
| **maintenance** | the outer shell wiped and the coat renewed when it greys — grime is reflectance lost; the mesh cleared with it |

**A fan is not fitted.** The chimney carries the calm-sun case the fan exists for; where a site
wants the aspirated figure, a 12 V fan sits over the inner shell drawing upward, its tacho read by a
Babel input, and WMO wants that status in the data — a stalled fan is a heated shield.

**Never beside the rain gauge's shell.** The gauge's outer shell keeps the wind off the weighed
vessel, so gusts do not move the weighed mass; a thermometer wants the airflow the shell turns
away. The gauge itself is unshielded for its catch — a catch wind shield is the site's option
(`../pluvius/HARDWARE.md`).

## Exposure

- **Air temperature and humidity** — 2 m, in the shield, on **the sensor post**: a 2 m
  non-conductive or white-coated tube **2–3 m from the mast on its poleward side**, where nothing
  taller stands sunward of it, so no shade falls on the shield with the sun above 7° (WMO siting
  classes 2–3). Over short grass kept under 25 cm, ≥ 10 m from the panel and any artificial surface
  (class 3; 30 m is class 2), clear of walls and paving that store heat and radiate it back
  (`../daedalus/CONSTRUCTION.md`, *The structures*).
- **Pressure** — in the shield with the T/RH probe, or the T/RH/P unit there: WMO puts a barometer
  in a closed box or room at errors above 1 hPa and wind pumping at 2–3 hPa, and asks for a static
  head in open air; the shield is one. **Never in the enclosure.**
- **Ground temperature** — 5 cm above short grass, no shield: its night minimum is the ground
  frost; by day the sun heats it and the reading is not used.
- **Wind** — the mast top, 2–3 m, the agricultural height evapotranspiration is computed at. **By
  WMO's siting classification that is class 4S whatever the terrain** — the classes assume 10 m —
  and the 10 m value is derived by the logarithmic profile with the **roughness length z₀ of the
  terrain 2 km upwind, recorded at commissioning per wind-direction sector** (WMO Vol. I, chapter 5,
  formula 5.3, the height term); the result is a corrected local wind, not a potential wind. Clear of
  obstructions — none on the plot is over 4 m — which bias wind more than anything at this height.
- **Rain and snow** — clear of overhang and splash; the snow radar's mast vertical with a clean
  look down (`CONSTRUCTION.md`, *The snow radar's shroud*).
- **Pyranometer and UV** — levelled, on the mast top's equatorward arm with nothing above them;
  class 1 is no shade with the sun above 5° and no reflecting obstacle above 5° elevation and
  wider than 10°; the anemometer on the poleward arm, higher.
- **Snow depth** — the radar on an arm ≥ 1 m from the mast, over a level white target the size of
  its footprint; snow drifts and scours around a pole.

## The classes are written into the unit

**A site has no one class — every instrument has its own**, 1 to 5 with the flag S, read against
WMO's siting classification at commissioning, re-read when the surroundings change and at least
every five years. The station writes them into Palatine's `SITING` register — temperature and
humidity, precipitation, wind, global radiation — and the terrain's roughness class per 45°
sector into `ROUGH`; both are persisted on the unit, every `SET` of them is a configuration record
in the archive, and the head carries them as the station's metadata beside its coordinates
(`FIRMWARE.md` §8). A reader comparing two stations needs them as much as the values: a class 3
temperature is uncertain by up to 1 K from siting alone, a class 4 by 2 K, and a 2 m wind is
class 4S by definition.

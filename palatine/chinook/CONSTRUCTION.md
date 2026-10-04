★ N.I.C. ★

# Chinook — the housing

> **Design-stage concept — nothing built.** The figures are verified against the parts' sheets and
> the parts before a board is made.

The air units are bought in their own outdoor housings; what the station builds is the **shelter**
that lets outdoor air reach them and keeps rain, spray, sun and insects off. It is a chimney, not a
box: nothing in it is sealed, because sealing is each unit's own.

```
                 cap — two white discs Ø 220 mm, 10 mm apart, the lower one 15 mm above the rim:
                 the chimney exhausts under it, the overhang shades the bore
          ┌────────────────────────────────┐
    ══════╧════════════════════════════════╧══════   three polyamide M6 rods at 120°, through the tube
      ║   ┌──┐                              ║
      ║   │  │  gas units on a rail along   ║        the tube: white PP / HDPE / PVC pipe Ø 160 mm,
      ║   │  │  the axis, sensing faces     ║        ~400 mm long, open at the bottom
      ║   └──┘  to the bore, cables down    ║
      ║   ┌──┐                              ║
      ║   │  │  the particle counter lowest,║
      ║   └┬─┘  its inlet nozzle down       ║
      ║    ▼                                ║
      ╚═════════════════════════════════════╝   mesh 1–2 mm across the mouth, nothing finer
           ═════════════════════════════      baffle disc Ø 120 mm, 30 mm below the mouth, on the same rods
                      ▲ air in, the arm's cable up the bore
```

## The chimney

| | |
|---|---|
| **the tube** | a length of white plastic pipe, **Ø 160 mm, ~400 mm**, the house sewer-pipe size — PP or HDPE painted over a polyolefin primer or bought white in a UV-stabilised compound, PVC painted; the station's solar-reflective white (`../../daedalus/CONSTRUCTION.md`, *Finish*). Open at the bottom, nothing inside the bore but the rail and the units |
| **the cap** | two discs of the tube's material, **Ø 220 mm**, 10 mm apart on the rods, the lower one 15 mm above the rim — the same cap as the thermometer shield's (`../SITING.md`), for the same reasons: the air leaves under it, the gap between the discs is the roof's second layer, and the overhang shades the bore at any sun above ~30° |
| **the rail** | a strip of PVC or a plastic DIN rail on the axis, hung from the rods; **the units on it with their sensing faces toward the bore and their cables down**; no unit touches the wall |
| **the air** | in at the mouth, up past the units, out under the cap. The units' own dissipation — 0,5–2 W each — and the sun on the tube drive it in calm; wind through the mouth and the cap gap does the rest. An electrochemical cell samples by diffusion and wants no flow; a particle counter and a PID bring their own fan or pump |
| **the mouth** | a **mesh of 1–2 mm**, stainless or polyamide, across it against insects and spiders — **nothing finer**: a fine mesh clogs with dust, holds a water film and takes the coarse particles before the counter does. A **baffle disc Ø 120 mm, 30 mm below the mouth** on the same rods, so driven rain and splash do not go up the bore; the mouth itself stands **≥ 1,5 m** above the ground, above splash |
| **no membrane across the bore** | a hydrophobic membrane there would stop the particles and slow every cell's response. **The membrane is the unit's**: every gas unit's diffusion opening carries an ePTFE membrane under its grille, and a unit sold without one gets an ePTFE vent patch over its opening — the class the station's enclosure vents use. **The particle counter carries none**; its inlet is a short tube, **Ø 6–8 mm, cut square, pointing straight down** below the counter, its outlet beside it, both in the bore's air |
| **water** | there is no horizontal surface inside and nothing to seal: rain that reaches the mouth falls out of it. The arm's cable comes up the bore from below; a unit's connector is the unit's own IP rating, which is why `SENSORS.md` buys the housing with the sensor |
| **condensation** | the units run continuously and dissipate, so the air in the tube stands a degree over the ambient and its relative humidity under it; a unit that is never switched off is never colder than the air around it. The one exception is a cold front after a cold night, which every outdoor sensor meets alike |

## Where it stands

- **The inlet 2–3 m above the ground** — the ambient-monitoring height (EU Directive 2008/50/EC,
  Annex III: 1,5–4 m) — on the **sensor post** of `../../daedalus/CONSTRUCTION.md`, below the
  thermometer shield on its own clamp, or on a post of its own; **never on the mast**, which is the
  strike terminal and carries the wind sensor's wake.
- **In free air, ≥ 1 m from the thermometer shield**: both want an undisturbed flow, and the shield
  must not stand in the units' warm plume.
- **≥ 2 m from the battery box vent and the vault hatch** — the station's own gases are not the
  site's.
- **Away from the station's own dust**: not over the bare soil patch, not downwind of the access
  track within a few metres where a site has one; a counter there measures the visit.

## The cold-climate build is a different build

Below 0 °C the cells leave their range and a counter's fan ices. A site that wants air chemistry
through its winter builds a **heated sample chamber**: a sealed box holding the units above 0 °C,
a fan drawing the sample through a heated inlet so the units see air above freezing and under
60 % RH, **10–20 W continuously** on a feed of its own — the arm's 12 V is ~5 W — a site's
decision against its power budget, never the base. The chimney above is the base and is not heated.

## Maintenance

The mesh cleared and the counter's inlet nozzle wiped at every visit; the tube washed when it greys
and repainted when a wash no longer whitens it; the cells and the counter replaced on their lives
(`SENSORS.md`), never recalibrated.

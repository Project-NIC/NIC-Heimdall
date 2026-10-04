★ N.I.C. ★

# Gaia — the siting rules and the computed first wave

> **Design-stage concept — nothing built.** The figures are verified against the parts' sheets and
> the parts before a board is made. **All coordinates are approximate and representative — a siting
> guide, not survey data.**

## The siting rules

1. **Do not pile on where networks already exist.** Countries with dense national networks —
   JP · TW · US · NZ · IT · CH · CL · MX · GR · IS — get only major hubs, for the cross-sensor
   value, not for coverage. The rule governs the computed global first wave, the ordering by global
   blindness; it does **not** restrict local densification — a station is built by whoever wants
   one, where they are.
2. **A station goes only where someone can build and keep it.** Every site is a **named real
   town**, or a real coastal or arc site — never a lattice point in the wilderness. The community is
   the infrastructure; a grid cell with nobody in it gets no station.
3. **One simple algorithm, not a hundred criteria.** Towns × hazard proximity, and a sparse backbone
   fills what is left. Refinement comes from real deployments, not from more rules.
4. **Instrument the threat sector only — the source map is closed.** *(The subsea arm, Tier 2,
   shelved — below.)* On a 50-year horizon tsunami sources are the known subduction arcs, so every
   island site's threat bearing is computable from trench geometry now, and the sondes go **only on
   the source-facing sector**: the lee shore of an island sees diffraction, not a wave front. **A
   4–5-sonde sector fan** — 5 the magnetometer array's minimum (common-mode reference and bearing)
   plus margin, 4 where a DART buoy already watches the deep-water leg upstream. Seas already cabled
   by national tsunami networks (S-net and DONET off Japan, ONC NEPTUNE off Cascadia, GeoNet off
   Hikurangi) keep only every third site, as an interop hub. **An island within reach of two arcs**
   — threat bearings more than 90° apart, Taiwan and Luzon between the Manila and Ryukyu trenches —
   **keeps the ring**, eight sondes, two fans back to back. **The radar sea-gauge goes only where a
   real town within ~100 km can host and keep the mast**; on a bare exposed islet the wave takes the
   mast, and the slope-toe **magnetometer is the gauge** — it reads transport (velocity × depth)
   potted on the seafloor and needs no surface structure. **~800 sondes, and 117 of 168 sites need
   no surface gauge.** The shallow Tier 1 pods off an islet stand in a ring of 4, 8 or 12
   (`../atlantis/README.md`, *The ring*).
5. **The electromagnetic environment.** A station stands **as far as practical from substations,
   industrial plants, electrified rail and tram corridors, buried cable loops and dense utility
   runs** — or the loadout follows the site. Urban soil carries stray and return currents, and
   buried grounding loops make local field distortions that reach full scale on a magnetometer: a
   station sited there measures the grid, not the Earth — the reason observatories enforce
   clean-EM perimeters. Heavy industrial premises are not a candidate for the EM-sensitive fronts
   at all. Where a town site cannot escape the environment, the magnetometer and the other
   EM-sensitive fronts leave that site's loadout and the rest stays.
6. **Longwave reception, where the station runs Pip.** What decides is **not the transmitter's
   distance but the receiver's surroundings** — nominal coverage circles are drawn for free field,
   and a building can cost more signal than a thousand kilometres of path:
   - **Reinforced concrete is a Faraday cage** — the rebar mesh attenuates LF hard; brick, timber
     and old masonry are transparent by comparison.
   - **The ferrite rod is directional** — a figure-8 with a deep null, responding to the magnetic
     component, which is horizontal and **perpendicular** to the direction of travel. The rod is
     laid **across** the bearing to the transmitter, and a wrong orientation can cost everything.
   - **Night beats day** — atmospheric noise falls after dark and the code needs a run of clean
     minutes, so a site is judged by its overnight behaviour, not by a daytime attempt.

   Pip stands outside, its rods oriented at commissioning — decided before the mast is up.

## Two deployment roles

They differ only in the local density of existing stations, and both are valid:

| role | condition | example |
|---|---|---|
| **complement** | no site within reach | Africa, South America, India, the ocean basins |
| **densify** | sites exist, at large spacing | ~100 km between professional sites leaves room for a local group of stations |

## The two tiers — what NIC builds and what is shelved

- **Tier 1 — built: the land and shore network.** The seismic towns, the backbone, and the coastal
  sites as **shore stations** — borehole seismometer, sea gauge, barometer, and GNSS/TEC where
  fitted. Every part installs from a boat ramp or a rooftop, and it carries most of the sensing
  ladder: P-waves, GNSS displacement, TEC, coastal sea level. The short wet hop to a near-shore
  pressure gauge is `../atlantis/README.md`'s.
- **Tier 2 — the subsea arm: a worked concept, shelved for cost.** The slope-toe magnetometer fans,
  the offshore sondes, the hybrid submarine cables, the cable and depth budget — all of it stands in
  `../atlantis/README.md` as a complete, costed reference, and none of it is a build target:
  50–100 km of armoured submarine cable a sonde and up to 500 km a site, ship-laying, permits and
  170–470 bar hardware is government or international-programme scale. The ocean is watched by Tier 1 and the world's
  existing deep systems — DART and the cabled networks — that rule 1 defers to.

## Density is per phenomenon, loadout is per site

- **Where — the density — follows the correlation length of the field.** Ground motion changes
  over kilometres near a fault, so dense (the early-warning argument); the geomagnetic field is
  smooth over hundreds of kilometres inland, so sparse; VLF lightning is heard across a continent,
  so a handful of receivers. The information lives on gradients, boundaries and active zones.
- **What — the loadout — follows the place.** No snow radar in the tropics, no lightning front at
  sea, no rain gauge on an uninhabited islet, cosmic-ray heads only at high latitude or altitude. The
  station finds its own fit-out at boot; the map records the intended loadout per site.

## The computed first wave — `proposed-network.geojson`

Placement is **computed from real data** — GeoNames towns ≥ 20 k population × the hazard lines —
not hand-drawn:

| class | sites | units | loadout |
|---|---|---|---|
| **seismic** — towns ≤ ~400 km from an active belt or arc | 2 086 | 2 237 | seismometer + barometer and weather (+ snow radar ≥ 45°, + air quality in real cities); big cities get extra units for early-warning aperture, +1 per ~1 M, at most 6 |
| **coastal, tsunami arrays** — along the subduction arcs *(Tier 2)* | 168 | ~800 sondes | slope-toe magnetometer + borehole seismometer + barometer as a **4–5-sonde fan on the source-facing sector** (rule 4); radar sea-gauge at the 51 sites a town can keep the mast; 6 two-arc sites keep the ring; 15 sites are interop hubs on cabled seas |
| **backbone** — the biggest town in each otherwise-empty ~800 km cell | 194 | 194 | magnetometer + GNSS-TEC + regional seismometer (+ neutron ≥ 60°) |
| **total** | **2 448** | **~3 200** | |

The seismic top ten falls out of the data: **India 271, Turkey 244, Indonesia 123, Iran 112,
Philippines 107, Algeria 100, Pakistan 97…** — the under-instrumented populated hazard belt.

**The scale.** ~2,4 k sites is the first wave — every real town on a hazard, once. **~10 k units**
is the whole-planet, all-layers build: about double the world's live seismic network, ~8×
INTERMAGNET, ~4× IGS for TEC, ~10× DART, for ~$40–90 M of hardware (the root README's model, $4–9 k a station). Japan-grade ~20 km
early-warning spacing everywhere worth having runs to ~100 k — the ceiling, not the plan. Each added
station improves the network, and joining the world's networks (`../core/INTEROP.md`) is the plan,
not a parallel one.

**Magnetometer units for the whole network: 990** — 796 array sondes *(Tier 2)* and 194 backbone
singles; the seismic towns carry none.

## The islet-first rule

**Where a real islet sits seaward on the slope, the shore station goes on the islet, not the
mainland.** A small island's flank is steep, so the deep water is next door: a 60–80 km mainland
haul collapses to tens of km, and the islet is the natural cluster hub, gateway and power in one
place (`../gauss/ARRAY.md`). It exists where the expensive tail is: **Isla Mocha** (~35 km out, on
the 1960 Valdivia segment) and **Isla Santa María** (off Concepción) for south-central Chile, and
**Islas Marías** for the Mexican stretch of Middle America. The Java south coast, Cascadia and
Hikurangi have no usable islet, and Peru's Lobos de Afuera failed the slope test below (`WHY.md`).

**An islet must sit on the slope, not on the shelf.** Islas Marías measures 36 km to the full toe
and 2 547 m of water already at 20 km — the best in the network; a shelf-locked islet puts the toe
hundreds of kilometres out. The islet buys cable, not pressure: the toe off it is still kilometres
down. The measured per-site numbers are `../atlantis/README.md`'s; the raw run is
`bathy-per-site.csv`.

## Why it can be placed at all

A ground sensor can be placed on someone else's land only if it threatens no one. **Open, civilian,
life-safety** — earthquake and tsunami early warning, geomagnetism and space weather — is what makes
the network deployable across borders; a classified or surveillance build cannot be sited abroad at
all. The same posture lets the data flow into the world's systems — FDSN, SuperMAG, IOC — instead of
beside them.

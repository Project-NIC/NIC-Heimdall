<p align="center">
  <img src="NIC-Atlantis.svg" width="200"/>
</p>

★ N.I.C. ★

# Atlantis — everything that goes in the water

> **Design-stage concept — nothing built.** The figures are verified against the parts' sheets and
> the parts before a board is made.

**Atlantis is the water build of the station's own units: the Galvani boards in their pressure
build, a pod body, and a cable chosen for the environment — nothing more.** It is no unit and adds
none: the pods in the water are Pascal and Gauss, and the boards are `../galvani/`'s.

**Two tiers share a body, a cable recipe and a potting doctrine**; only the length and the loadout
differ:

| | **Tier 1 — built** | **Tier 2 — shelved** |
|---|---|---|
| what | Pascal and Gauss a few hundred metres off a steep islet, in a ring | a magnetometer at the toe of the island slope |
| cable | **one sea cable**, the feed in the data cable's jacket — two conductors, or one with the return through the sea (`../galvani/README.md`, *Under water*); copper pairs to 1 km, glass beyond | **50–100 km** armoured hybrid, ship-laid — 100 km is the ceiling, set by the module (*The link*) |
| feed | **300 V** on a sea return; 48 V serves on two conductors over a copper run | **300 V** |
| the boards | the Galvani boards in their pressure build, the module that suits the run | the same |
| depth | 150–200 m ≈ 15–20 bar | placed at 1,7–4,7 km ≈ 170–470 bar; the build rated to 8 km ≈ 800 bar |

**Tier 2 is shelved for cost, not for physics.** The electronics are the family's 300 V pair and
the 2 Mb/s glass module; **the cable is the project** — 50–100 km of armoured hybrid, ship-laid, is
a millions-class line item once the laying, the survey and the permits are counted, orders of
magnitude above everything else NIC builds. **The wet end's optics is the one part not bought off
the shelf**: the pod takes the module's pressure-tolerant form, asked of the maker. The concept is
worked to the arithmetic below so that, if the tier ever wakes, nothing is derived twice.

```
   SHORE STATION                                                          THE PODS, 150–200 m, a ring round the islet
   300 V source · the module that suits the run  ══ one sea cable ══▶    PASCAL and GAUSS on their pressure boards, oil or jelly in PVDF

   TIER 2, SHELVED:  300 V source · the 2 Mb/s glass  ══ 50–100 km armoured hybrid, ship-laid ══▶  a Gauss pod at 1,7–4,7 km
```

## The sea gauge — a pressure sonde instead of a radar mast

The coastal tsunami sites carry a **down-looking radar sea-gauge**, and it needs a mast a town can
host and keep (`../gaia/SITING.md`, rule 4). **A depth gauge on the wet end measures the same thing
from underneath**: a 30 bar gauge, the `MS5837-30BA`, ~300 m of water,
far past the ~30 m column a tsunami site needs. That removes the mast, which is most of what a
coastal station costs to keep. **It is Pascal** (`../pascal/README.md`), NodBus mini type 3 — how it
is anchored, how deep it goes and why it is burst-sampled are there.

## The ring

**Four, eight, twelve — and the ceiling is the laying.** The Tier 1 sondes stand in a ring round
the islet, Pascal and Gauss alike, 150–200 m down its steep flank. Tier 2's deep sondes at the toe
of the slope stand as a 4–5-sonde fan on the source-facing sector instead, and keep the ring of
eight only where two arcs reach the island (`../gaia/SITING.md`, rule 4):

| sondes | spacing | what a front from any bearing meets |
|---|---|---|
| **4** — the minimum | 90° | one sonde head-on and the two beside it on the flanks, partly; the lee one is the reference |
| **8** — the reconstruction | 45° | two or more head-on and the flanks either side — the bearing and the wrap round the island resolved |
| **12** — the ceiling | 30° | the same, with a sonde to lose |

**Twelve is set by the cable, not by the sensor.** Every sonde is a run of its own laid out from
the shore, and the cable and its laying cost many times the sonde; past twelve a site buys
cable, not information.

## The pod body — small, monolithic, oil-filled

On land a classic box is fine. **In the water pressure is the whole constraint**, and the build
goes the other way: **small, monolithic, one oil-filled board**. The magnetometer is coils, not
MEMS, so it takes pressure directly — the oil transmits it and there is no vessel to crush. **The
whole pod is potted, the optical transceiver included, and the fibre is led straight out,
fusion-spliced, sealed inside the fill.**

**The enemy at depth is a trapped gas bubble** — it compresses, shifts and stresses the pot — so the
build is **a PVDF tube, oil or petroleum jelly, vacuum-degassed: no gas anywhere, and no bladder.** At
800 bar, the build's rating, the fill compresses ~4–5 % of its volume and the PVDF ~2,7 %, and the cold floor adds
~0,5 %; the wall takes the ~2,5–3 % difference by flexing in — a round Ø 40 wall of 2 mm at ~1 %
hoop strain, the fill ~20 bar under the sea — and a wall flattened hot (PVDF forms at ~150–160 °C)
takes it softer still. **Hydrogen is kept out by construction**: no sacrificial anode at the pod,
the fibre in a hermetic tube, the fill degassed, no high-strength steel. The thermometer then reads
the true isothermal cavity temperature. **The shallow gauge pod and the deep sonde differ in wall
thickness and loadout, not in construction.**

**Only air-cavity parts are excluded, and the pod has none.** The laser driver, the receiver and the
processor are solid-body ICs and take the pressure like the rest of the potting; the tilt sensor left
Gauss for this reason, and the crystal is gone too — the pod's HSI is steered by the clock rung
arriving on the link (`../core/blocks/clocks.md`, *The two sondes carry no crystal at all*).

## The optical front, oil- and pressure-proof

**A 1×9 module cannot go to depth**: it is an air-cavity package with a receptacle, and a gas
cavity behind a lid of that area is the one thing the pod forbids; potting it from the outside does
not fix a cavity inside the package. **What goes in the pod is the pressure-tolerant form of the
same module, asked of the maker** — a **fibre-pigtailed hermetic laser and photodiode, a coaxial
Ø 5,6 mm TO-can** with the fibre leaving axially: the die and its lens are sealed behind glass inside
the metal can, the oil touches only the inert metal and glass exterior (an inert fill, silicone oil),
and the hydrostatic pressure is balanced by the oil, so the only stressed cavity is the can's small
window. **The design envelope is the deep build**: ~470 bar at the deepest placement inside the
100 km reach, 170–400 bar where most sondes sit (`../gaia/bathy-per-site.csv`). The pigtail is
fusion-spliced to the cable fibre and led out through a potted subsea fibre penetrator; bare glass
is oil- and pressure-proof.

**What the maker must not lose, whatever the package:** DC coupling with **idle mapped to
laser-dark** — the rule the optical boards are built on (`../galvani/README.md`) — the segment's
clock rate, and TTL levels. The ask is made in hertz, not in Mb/s. DC coupling puts the idle level
straight on the laser and a UART idles high, so the wrong polarity burns the laser continuously.

## The cable — bought, for the sea

**The Tier 1 run** is one sea cable with the feed in its jacket (`../galvani/README.md`, *Under
water*). **The Tier 2 run** is a **hybrid**: single-mode fibre for the NodBus mini segment and copper
for the 300 V feed in one jacket.

- **The strand count is nearly free.** On an armoured cable the cost is the armour and the laying,
  not the glass, so spare fibres are pulled: two or three in use — BiDi or duplex — the rest cold
  spares.
- **Jacket and armour for the sea**: a water-blocked core (gel or swellable tape, so a nick cannot
  wick water along it), steel-wire armour for abrasion and the anchor and fishing hazard in the
  shallows, a UV-stable outer. The ordinary submarine recipe, bought.
- **Laying**: along the seabed off chafe points, **buried through the surf and anchor zone**, on hard
  bottom in the deep, with **slack at each end** and a clamp at the sonde and at the shore, so wave
  and current never pull the penetrator or a splice.
- **Into the pod**: the fibre through a potted subsea fibre penetrator, the copper behind its own
  gland, and on a sea return the cathode on a second one. The penetrators are the only boundaries to
  seal.
- **Ashore**: the cable lands at a shore station, an ordinary land station, whose gateway carries the
  one uplink out for the cluster (`../core/UPLINK_TRANSPORT.md`).
- **No active kit in the sea** — no mid-run repeater, no seafloor power, no hydrophone array.

## The feed — two variants

**One pod, one set of boards, one fibre, 300 V.** What differs is the return path:

| | **A — double insulation** | **B — sea return** |
|---|---|---|
| return | the second conductor; nothing leaves the cable | the sea |
| reference cable | **Type 3744** (doc. 11906565 rev. F): 4× SM G.657.B2 + 4× MM in a steel tube, **7× 1,0 mm², ≤ 20,4 Ω/km**, 3 kV, 10,5 mm, 6 000 m, PU jacket | **OCC-SC500**: 17 mm LW, 8 000 m, > 70 kN, 18 kV DC, armoured builds to 200 m; one conductor, class 1–1,6 Ω/km |
| pod | floats | a titanium cathode on a second penetrator |
| shore | the source board, as any run | the conductor negative; the anode (MMO or high-silicon iron, never plain steel) in ground that stays wet and conductive all year |
| asked of the maker | armour, breaking load, continuous length | the conductor's resistance, the price |

**An option on A — a breach liner.** A thin metal sleeve inside the PVDF shell, spaced off the
board and wired to one feed conductor: seawater through a cracked shell puts that conductor on the
sea, and the station's leak watch reads it at once. Any metal that lasts in seawater until the pod is
recovered serves — aluminium, stainless.

**The line sits at the full V₀ before the pod draws**, so a restart into a discharged bulk is the one
moment the load is not constant-power: the unit power board limits the inrush, or the line pulls
itself down.

## What the pod draws

**Carried as 1 W, sized for 1,5 W.** The module is `OPT2-55A03STR`, `I_TX + I_RX` 100 mA on its
sheet as one figure; the clock channel populates `VccR` only, and the data transmitter is dark
between bursts:

| | at 3,3 V |
|---|---|
| data module, both sections, TX bursting | ~80–100 mA |
| clock module, receive section only | ~20 mA |
| `STM32H523`, one UART and SPI | ~30–60 mA |
| `RM3100`, `INA238` + `ISO1642`, the rest | ~10 mA |
| **on the 3,3 V** | **~0,5–0,6 W** |
| **at the cable**, through the buck and the island at ~85 % | **~0,6–0,7 W** |

**`VccT` and `VccR` are separate supplies with separate grounds**, so the one-way clock link needs
no one-way part: the pod populates `VccR` only and the head `VccT` only. The clock link cannot be
burst — a clock is continuous — so that module draws its full current always.

**The feed is 300 V because 48 V does not carry it**: at 105 km even 1 W needs ~130 V on
2× 1,5 mm², and 48 V delivers ~0,17 W.

**The bulk at the pod is sized by the burst, and the burst by the protocol.** The cable delivers a
constant power and the pod's load is bursty, so the bulk capacitor buffers it:
`C = 2·P_burst·T / (V₁² − V₂²)`. Burst length falls with the rung and with the block size, and a
capacitor cannot rescue a long burst, so the rule is **send often and small** — the burst short
enough that the bulk covers it, the converter sized for the average.

## Reach

Constant-power law as everywhere — `V_end = (V₀ + √(V₀² − 4·P·R_loop)) / 2`, collapse at
V₀² < 4·P·R. Worst case stacks route slack, maximum conductor resistance and no credit for cold
water: **105 km, 20 °C, 300 V**. "Usable" is the house criterion `V_end` ≥ 75 % `V₀`,
`P = 0,1875·V₀²/R`, before the pod's converter:

| cable | loop | collapse | usable |
|---|---|---|---|
| hybrid 2× 1,5 mm² | 2541 Ω | 8,9 W | 6,6 W |
| **Type 3744, 2 conductors** | 4284 Ω | 5,3 W | **3,9 W** |
| Type 3744, 3 + 3 | 1428 Ω | 15,8 W | 11,8 W |
| **OCC-SC500 + sea**, 1,6 Ω/km | ~200 Ω with the groundbed | ~110 W | ~84 W |

**Every row carries the 1,5 W reserve**; the smallest cable that does it is the one the site
permits.

## The link

**Two transfers, not alike.** The **data** link is one module run **full duplex** — BiDi on one
strand or a duplex part on two, the builder's choice — so there is no direction to switch and no `DE`
on the optical side. The **clock** is one-way: a transmitter at the head, the receive section at the
pod. **Two fibres with the BiDi pair, four with duplex parts.**

**The pod is a mini-NOD, so its segment runs the mini rung**, and there the market has parts:
**DC-coupled TTL 1×9 transceivers specified from zero to 2 Mb/s at 100 km**, single-mode 1550 nm,
industrial grade — **`OPT2-55A03STR`** (3,3 V, SC, 1550 nm, **33 dB budget**), or the **BiDi matched
pair `OTB2-35A03STR` + `OTB2-53A03STR`**, whose two ends are not interchangeable. **The industrial
−40 to +85 °C grade**, the shore landing being an outdoor enclosure like any other. The **2¹⁹ clock —
524,288 kHz, 1,05 Mb/s equivalent** — sits inside them; the data is ~400 B/s and never enters the
choice. The 40 B bus's 2²² clock is 8,39 Mb/s equivalent, and nothing DC-coupled carries it past about
40 km — which is why the long link is a mini segment.

## How far out, and how deep — the cable and depth budget

**Concept-grade: these figures pick the cable class, they are not a survey**; a real deployment
measures its own site. Where the sonde sits is set by the magnetometer — full transport signal at the
**toe of the island's own slope**, and the plateau beyond buys ~0 % (`../gauss/ARRAY.md`) — so the
cable length is whatever the local bathymetry says the toe is. Real bathymetry (ETOPO) for 121 of the
168 coastal sites is `../gaia/bathy-per-site.csv`:

| | |
|---|---|
| cable to the full-signal toe | **median 120 km** (min 32, max 258) — inside 100 km at 40 sites |
| what 20 km of cable buys | median **195 m** of water — only 15 steep sites reach ≥ 1 000 m |
| what 50 km buys | median **1 141 m** — 64 sites ≥ 1 000 m, 27 ≥ 2 000 m |
| **what 100 km buys — the reach** | median **2 940 m** — 102 sites ≥ 1 000 m, 85 ≥ 2 000 m; the median site at ~80 % of its toe's depth |
| shelf sites, still under 300 m of water at 100 km | **13** |
| sonde pressure, typical | 1,7–4,0 km ≈ **170–400 bar** (~470 bar at the deepest) |
| cable per site, the maximum | **500 km** — 4–5 sondes, each a run of its own to its toe or to 100 km (800 km at a two-arc ring); median 430 km |

**The reach is 100 km** — the module's (*The link*), with the power worked at 105 km for the route
slack (*Reach*). **Tier 2 is a selection, built site by site**, so the budget is per site, never a
whole-network total. Nothing fits 20 km. **The reach levers**, in order, where the toe lies beyond it:

1. **Islet-first** — the shore station on an islet on the slope (`../gaia/SITING.md`).
2. **Knee siting** — ~85 % of the signal sits just below the toe, at a fraction of the cable.
3. **One mid-run splice.**

**In the water the medium is one module: the 2 Mb/s, from a metre to 100 km**, in its
pressure-tolerant form. The 10 Mb/s module is a 1×9 with an air cavity and stays on land. **Depth is
paid, not dodged** — the toe sits at 3–4,7 km, 300–470 bar, which is what drives the oil-filled build;
an islet shortens the cable, never the pressure.

## The minimal island node — one thing, done well

**A watertight micro-station on a small, often uninhabited island or atoll, built to watch the sea
and to do nothing it does not need.** It is not a weather station; nobody lives there to want rain
or soil data.

- **The radar is the point** — a down-looking **80 GHz sea-level gauge**, the part Palatine uses for
  snow (`../palatine/SENSORS.md`), burst-sampled and averaged over the wave period, so the tsunami
  survives and the wind-waves average out. A surge or tsunami rises over minutes, and a plain radar
  sees it.
- **The magnetometer is the deep upgrade** — a sonde at the toe of the island slope
  (`../gauss/ARRAY.md`), where the bottom drops off steep and close — even so the full-transport
  toe is 32 km of cable at the steepest coastal site and 120 at the median (*How far out*, above).
- **A seismometer in a pipe** — Quake's DN40 borehole sonde (`../quake/CONSTRUCTION.md`), dropped
  down a bored pipe in the rock and wedged. It is the **earliest warning of the three**: the P-wave
  arrives minutes ahead of the water.
- **Housekeeping is tiny** — barometric pressure, temperature and tilt. Pressure is a real
  meteotsunami and storm-surge discriminator for a few cents; temperature and tilt say the platform
  is alive and upright. No rain gauge, no wind mast, no soil probes.

**The two kinds of sensor have opposite seating costs:**

| | the radar | the sondes — magnetometer on the bottom, seismometer in the rock |
|---|---|---|
| lives | above water, in the surf zone | in the quiet, on the deep bottom or down a bored pipe |
| needs | a rigid structure that survives breaking waves, spray and impact | to be lowered and coupled — no surf-zone structure |
| **seating cost** | **the expensive half** | **the cheap half** |

**The electronics and the cable are solved and cheap; the seating — the anchor, the surf-zone mast,
corrosion and fouling — out-costs everything else combined.** That is where the frugal effort goes:
one cheap, survivable mount, standardised, and no gold-plated electronics to save a mount that has
to be built anyway. **The budget is ~$50–100 k with the marine seating in**, against ~$4–9 k for a
land station — still three to five times under a professional buoy (a DART mooring is ~$250–500 k,
most of it the mooring).

## Files

| file | contents |
|---|---|
| [`WHY.md`](WHY.md) | the graveyard |

## Licence

Hardware: CERN-OHL-S v2 (`../LICENSE-HW`) · Software: MIT (`../LICENSE`) — Copyright (c) 2026 NIC — Native Intellect Community

★ N.I.C. ★

# Gauss — construction

> **Design-stage concept — nothing built.** The figures are verified against the parts' sheets
> and the parts before a pod is made.

## Two builds, one board

**The board and its Galvani boards are the same in both; what differs is the tube, the entry and
the fill.** The board carries no cavity part in either — no crystal, no MEMS — so it goes into the
sea build unchanged.

| | **land** | **sea** |
|---|---|---|
| the tube | **PPR or HT pipe**, ~40 mm class, fusion-welded end caps | **PVDF pipe** of the same profile, fusion-welded end caps |
| the entry | a multi-stage cable gland, IP68, sealed with sealant | a **glued gland at the bottom**, the cable water-blocked |
| the fill | optional | **oil or petroleum jelly, vacuum-degassed, no gas anywhere; no bladder** |
| where | in air: a non-ferrous stake in the grass, a well or a borehole above the water | under water: a flooded well, a lake, the sea floor **beside Pascal, to ~250 m** |

**The deep build — 1 to 8 km — is Atlantis's** (`../atlantis/README.md`): the same tube and fill
taken to 800 bar, shelved for cost.

**The profile is set by the coils, not by strength.** The two transverse coils (~15 mm long) want a
bore of at least ~18–20 mm; ~40 mm is the comfortable standard that also takes the Galvani boards.
Diameter and wall are the builder's choice within that. **The length is set by the distance**:
the tube is up to a metre long, ~80 cm between the Galvani boards at the cable end and the coils
at the tip. Quake's bodies are the same pipe in the
Barrel family's 25 cm steps (`../quake/CONSTRUCTION.md`).

## The land build

**PPR (or HT) pipe** — rated 50 years at 70 °C and 10 bar, so centuries at ~10 °C and ~2 bar;
fusion-welded, so the tube is one piece with no seal of its own; non-magnetic and non-conductive.
On land nothing presses on it, but a well floods and a stake stands in wet ground, so it is
**watertight to IP68 — continuous immersion — through a multi-stage cable gland, its thread and
cable entry sealed with sealant** — a neutral-cure silicone or an MS-polymer, never an acetoxy
silicone, whose acid stays in the closed tube with the electronics — and the fill optional. The caps are fused like the tube; the
gland is the one seal, and it costs more than the rest of the body. In a dry bore above the water table the
depth costs nothing — an air column weighs nothing — and the cable, kevlar-membered, is the limit.
**Under water it is the sea build**, whatever the depth.

## The sea build

**PVDF, because in the sea the tube's job is to keep things out, not to hold pressure.** The pod
is pressure-balanced — the fill inside, the wall passes the pressure through and carries no
difference of its own — so strength decides nothing and these do:

- **permeation** — PVDF passes water and gases, hydrogen among them, an order below PE or PP;
- **the fill** — PVDF neither swells nor softens in mineral oil or petroleum jelly, where PP and PE
  do;
- **water uptake** — ~0,04 %, where the polyamides take 1–2 % and hydrolyse;
- **the weld** — it is sold as ordinary industrial pipe and fused like PPR, so the tube stays a tube
  with caps welded on;
- **the field** — non-magnetic and non-conductive.

It is the subsea industry's own material for the pressure sheaths of flexible risers and
umbilicals. PEEK would do as well and costs many times more and does not weld; PTFE creeps and
passes gas; glass is not a tube to drop on a sea floor.

**The fill is oil or petroleum jelly, degassed and filled under vacuum, with no gas anywhere** —
the vacuum draws the air out of the windings with the rest. A trapped
bubble compresses, shifts and stresses the pot, and on the way up it grows. Petroleum jelly does not
flow out of a crack and holds a bubble where it is, and is degassed molten, at ~60 °C; oil degasses
cold.

**The wall is the compliant element and there is no bladder.** From a fill closed at room
temperature to a 4 °C sea floor the fill shrinks by ~1,1 % of its volume and the PVDF by ~0,6 %;
at 25 bar the fill compresses a further ~0,15 % and the PVDF ~0,08 %. The wall takes the ~0,6 %
difference by flexing in, and the fill sits a few bar under the sea — nothing a part notices. **The
fill is closed at room temperature**: molten petroleum jelly is let cool and topped up before the
cap is welded, or it shrinks away from the parts. A bladder would be one more material in the
boundary, and elastomers pass gases far more than PVDF.

**A glued gland is enough, because nothing pushes through it** — the pressure is the same on both
sides. What is left is kept out by construction: **the gland at the bottom**, so the heavier water
stays under the lighter fill and no exchange by density starts; **a water-blocked cable**, so a
nicked jacket does not let water run along the core into the pod.

**A breach liner is the sea build's option**: a thin metal sleeve inside the tube at the cable end,
spaced off the boards and wired to one feed conductor. Seawater that enters through a cracked wall
or a failed gland sinks under the fill to the cable end, reaches the liner and puts that conductor
on the sea, and the source end reads the current at once. It stands at the cable end and not along
the tube because water collects there and because it carries no current in service, so it adds
no field ~80 cm from the coils. **Aluminium or titanium — non-ferrous**; not stainless, which turns
weakly magnetic where it is worked.

**The pods on one segment share one feed**, chained pod to pod, and the run's voltage is chosen
for their sum at the last one.

**The power and communication boards are the family's pressure builds, and the module is the one
that suits the run** (`../galvani/README.md`, *Pressure boards and land boards*).

## Inside the pod

**Two boards in the tube, wired and potted together**: the Galvani boards at the cable end —
supply, isolation, protection, the link — and the sensor board at the tip. **The order is the
layout rule**: the buck at the cable end, the coils at the tip, and every centimetre between them
is 1/r³ of interference (`HARDWARE.md`). **Two thermometers at the tip** — one on the wall, one
between the coils.

## The mount — the axes known

**The tube carries its X, Y and Z marked on the outside, and the mount holds it by them**: Z
vertical, X set in azimuth, both adjustable and locked once set. The calibration below then works
from an orientation known at installation, not guessed.

## Where it stands

- **On land:** a non-ferrous stake, away from the station's own dynamic currents.
- **Down a well or a borehole**, on a kevlar-membered cable — a village well of 170 m is a free
  borehole observatory, thermally stable and seismically quiet.
- **On the sea floor**, beside Pascal and anchored like it: a plain unreinforced footing — **no rebar near a
  magnetometer** — the cable clamped to it and slack beyond it.

## Calibration without rotating the site

A pod down a well cannot be turned in place, and does not need to be.

1. **Tumble the finished pod before it goes in** — a 3D tumble and a sphere fit: hard-iron
   offsets, gains and non-orthogonality are properties of the pod and travel with it.
2. **The magnetometer is its own motion watchdog.** The static field at deployment fixes two of
   the three orientation angles and the network regression the third; a pod that later settles or
   turns shows as **a permanent step in the components with |B| unchanged** — a field event
   changes |B|, a rotation keeps it.
3. **The network calibrates continuously.** Geomagnetic variations are coherent over hundreds of
   km, so regressing the station against reference observatories (INTERMAGNET) and its neighbours
   refines gain, orientation and temperature coefficients for as long as the pod runs.
4. **The absolute baseline is not ours** — the pod's own offset plus the crustal anomaly — and it
   cancels in every variation product: storms, sudden commencements, pulsations, dB/dt, the
   tsunami. Absolutes are the observatories' job.

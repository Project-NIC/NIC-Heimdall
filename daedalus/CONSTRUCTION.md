★ N.I.C. ★

# Daedalus — building and seating the station

> **Design-stage concept — nothing built.** The figures are verified against the parts' sheets and
> the parts before a board is made.

**Only what every station shares**: the structures and what hangs on them, what the station is
protected against, grounding, cable routing and the drawn cables, the enclosures, the vault, the
finish and service. **A seat, a sonde or a sensor tube belongs to the unit that uses it** — the
borehole is `../quake/CONSTRUCTION.md`'s, the soil seat `../palatine/CONSTRUCTION.md`'s, everything in the
water `../atlantis/README.md`'s; the pack and the thermal case are `../core/POWER.md`'s. The rule
throughout: **classic, cheap, off the shelf, adapted by the builder.**

## The structures — a low mast for the sensors, and the rest on whatever the site has

**The number of structures is not fixed; the distances are.** The reference build is **one low
mast for the sensors** — a scaffold tube concreted ~1,5 m into the ground and standing 2–3 m
above it — and everything else on whatever the site offers: the panel on a low frame, the
antennas on the mast or on a post beside the vault, the rain gauge on the ground, the radiation
heads on their own post. A site that wants wind at the WMO 10 m builds a 10 m mast; that is a
structure the builder adds, not this design (`WHY.md`).

**The mast is a pipe, and the cabling inside it is insulated from the metal.** Each wire bundle
runs in its own **double-wall corrugated conduit, or a thick-walled hose, continuous from the head
to inside the enclosure** — the conduit ends only inside protected volumes, so there is no
exposed transition, and a penetration or a wrap seam is where surface creepage kills insulation.
Cable jacket plus conduit wall stack to the **~10 kV standoff target**: PVC holds ~20 kV/mm in
bulk, and the surface path from the steel to a conductor runs the length of the conduit into the
box. **No inner plastic liner in the pipe** — it traps water against the steel and rusts the mast
from inside (`WHY.md`); a drain hole at the foot of the tube instead. So when the mast takes a
strike and rings at kV, the surge stays on the steel and the signals stay clean. This internal
insulation is what makes grounding rule 1 below (every structure IS earthed) safe for the
electronics: the steel can take the strike because the copper never touches it.

**The insulation, the isolation and the entry arresters are ONE system — remove one and the
other two fail.** During a stroke the masthead sits at hundreds of kilovolts (the
earth-resistance term lifts the whole pipe, the inductive term adds along it), and the conduit
stack's ~10 kV is small against that. It survives because the conductors inside are **allowed
to ride the rise**: their only references are the arresters and clamps at the enclosure entry,
which equalise the difference at the entry earth, so the volts across the conduit stay modest.
A conductor hard-bonded to station ground with no entry protection would be pinned low while
the mast rings high — the full difference lands on the conduit, flashes through the "10 kV"
stack, and then through whatever the conductor lands on. The electrical half — what couples
into a mast conductor and what each entry takes — is the protection ladder's
(`../galvani/README.md`, *The protection ladder*).

**Where things stand — the rules, read against WMO-No. 8 (Vol. I, Annex 1.D, the siting
classification; chapters 2, 3, 5 and 6), and everything else is the builder's.** A site has no one
class: temperature, precipitation, wind and radiation each take their own, 1 (a reference site) to
5, and the station's are stated below as the build gives them.

1. **Air temperature and humidity ≥ 10 m from the panel and from any artificial surface over
   ~10 m²** — class 3; 30 m is class 2. A panel runs at 50–60 °C in sun and a thermometer in its
   plume reads the panel. Artificial surfaces may cover at most 10 % of the circle of 10 m around
   the shield and 5 % of the circle of 5 m; the vault's lid, ~1 m², is far under both. Grass it over
   anyway — a station measures the ground it stands on.
2. **The shield stands on a post of its own, on the poleward side of everything taller than it.**
   Classes 2 and 3 allow no shade on the shield while the sun is higher than 7°, so anything above
   the shield's 2 m must stand at least **8× its excess height** away (1/tan 7°), or on the side
   the sun never reaches above 7° — north of it in the northern hemisphere, south of it in the
   southern. The sensor post is a 2 m non-conductive or white-coated tube **2–3 m from the mast on
   its poleward side**, the panel's top stays below 2 m, and the mast, the one thing taller, is on
   the sunward side of nothing. A shield on the mast would be shaded every time the sun passed
   behind the pipe and would stand within side-flash range of the strike terminal (*The radiation
   heads*, below): the same argument that moved the dose-rate heads off the mast moves the
   thermometer off it. The shield's build is `../palatine/SITING.md`.
3. **The rain gauge ≥ 2 m from any structure** (side-flash) **and ≥ 2× the height of any obstacle
   above its orifice** — a panel, a shield, a tree, a fence; a bare pipe with small heads on it is
   not one. That is class 2; class 1 wants either a wind shield or a ring of obstacles of uniform
   height at 2–4× their height, a clearing, which a plot this size does not have. **Orifice 1 m above
   ground** — the national height, and above the vegetation — raised only where snow would reach it
   (`../pluvius/HARDWARE.md`).
4. **Wind on the mast top at 2–3 m, and that is class 4S by construction**: the classification
   assumes 10 m, and a lower sensor is class 4 or 5 with the flag S whatever the terrain. The 10 m
   value is derived by the logarithmic profile with the roughness length of the upwind terrain,
   which the commissioning survey records (`../palatine/SITING.md`); it is a corrected local wind,
   never a potential wind. Obstacles ≤ 4 m are ignored at class 1–2 and ≤ 6 m at class 4, so nothing
   on the plot counts as one; a site that wants the reference height builds a 10 m mast (`WHY.md`).
5. **Global radiation and UV at the mast top on the equatorward arm, nothing above them**: class
   1 is no shade with the sun above 5° and no reflecting obstacle (albedo > 0,5) above 5° elevation
   and wider than 10°; the anemometer goes on the poleward arm, higher, where the sun above 5° never
   stands, and the white mast at 60 mm subtends ~2°, under the width threshold.
6. **Ground cover stays natural and low, and kept under 25 cm around the shield and the gauge** —
   class 3's limit; classes 1–2 want it under 10 cm. Where the grass stops at 25 cm nothing is cut;
   where nettles reach 2 m, a circle around both is. The panel sits as low as the vegetation and the
   snow allow.
7. **Every steel structure set in concrete is an earth electrode and is bonded below grade to
   the others** — the mast, the panel frame, any post: one earth system per site (*Grounding*,
   rule 1). The sensor post is not steel and carries no bond.
8. **Runs between structures go buried, in ≥ 5 kV insulated conduit, in one trench with the
   bond conductor** (*Cable routing*).

The plot is set by rule 1 alone — about 10 m across — and every other placement is the builder's.
**The classes are re-read at every change of the surroundings and at least every five years, and
they are written into Palatine's `SITING` register, the terrain's roughness into `ROUGH`, and
carried by the head as the station's metadata beside its coordinates** (`../palatine/SITING.md`): a
reader comparing stations needs them as much as the values.

**The heads and where they go:**

| unit | where | consequence |
|---|---|---|
| air temperature and humidity, the barometer | **the sensor post**, 2 m, poleward of the mast — the T/RH probe, or the T/RH/P unit, in the passive shield of `../palatine/SITING.md`; the ground thermometer 5 cm above the grass at its foot | the isolated Modbus arm (`../galvani/README.md`). **The barometer is never in a sealed box**: WMO (Vol. I, chapter 3) puts indoor errors above 1 hPa and wind pumping at 2–3 hPa, and the cure is a static head in open air — the shield is one |
| wind | the mast top, poleward arm | rule 4; the Modbus arm |
| GNSS antenna | mast-top just below the wind sensor, or a post by the vault — the shortest coax | coax + two DC-pass arresters (`../core/blocks/gps-pps.md`) |
| global radiation and UV | the mast top, equatorward arm, nothing above them | rule 5; the Modbus arm |
| snow radar | **an arm ≥ 1 m from the mast**, looking down on a level white target the size of its footprint: snow around a pole drifts and scours, and a reading under the pipe is the pipe's | the Modbus arm (`../palatine/CONSTRUCTION.md`, *The snow radar's shroud*) |
| the modem position's antenna | the mast or a post, at the height the link wants — a buried vault cannot radiate | coax + DC-short arrester at the entry; the whip is sacrificial |
| the Wi-Fi antenna, directional | the mast or its own post, aimed at the partner access point, line of sight | coax + DC-short arrester at the entry, on an insulating bracket like every antenna (`../core/UPLINK_TRANSPORT.md`) |
| solar panel | its own low frame, ≥ 10 m from the shield (rule 1), its top below 2 m (rule 2) | the string travels at string voltage to the vault, arresters ahead of the MPPT (`../core/POWER.md`); the 12 V never leaves the vault |
| rain gauge | on the ground, rule 3 | a ModBus arm: its four-wire cable, and the head's 24 V on a 2-core of its own from `G-24-S`, both buried (`../pluvius/HARDWARE.md`) |
| Tesla | ground level, own frustum, away from the mast | one plastic box, the spur its only penetration (`../tesla/CONSTRUCTION.md`) |
| radiation heads | **own ~1 m post beside the station** | below |

**The radiation heads get a post of their own, not the mast.** The dose-rate norm asks for
~1 m above ground — **a height, not a structure** — and the mast is the strike terminal: at
1 m up the pipe sits at ~200 kV during the stroke, and against ~500 kV/m of impulse air
strength a head bolted to it is inside side-flash range (~0,4 m). Instead: a **~1 m
non-conductive post** (GRP tube — the same material as the hail shell — or any plastic/wood
stake; the head weighs nothing), **1–2 m clear of the mast**, with the **leads buried** to the
enclosure. The norm gets its metre, the pulse lines become ordinary short outside runs owing
only their entry transils, and one metre of clearance clears the flash-over arithmetic. In
snow country the post clears the seasonal pack — a buried head reads the snow, not the sky.

```
THE STATION IN SECTION — the mast takes the strike, the plastic keeps it out of the copper

                 wind ─┐   T/RH post, 2 m, poleward                 rain gauge, rule 3
             GNSS ant ─┤   2–3 m   ▯ ← the shield   panel on a low  ┌──┐ orifice 1 m
       snow radar arm ─┤           │   2–3 m off     frame, ≥ 10 m   │  │
     pyranometer, UV ──┤║▓║        │   the mast      from the shield └┬─┘
                       ║▓║  ▓ = one    └──┬───┘ PV string, buried,     │ the arm's cable +
                       ║▓║  conduit      │     at string voltage       │ the 24 V 2-core
 ══grade═══════════════╬▓╬═══════════════╪═════════════════════════════╪═══════════════
   ~1,5 m in the soil  ║▓║               ▓  ≥ 5 kV conduit, one trench ▓
   = an electrode      ╚▓╝               ▓  with the bond conductor    ▓
   by construction      ▓                ▓                             ▓
   bonded below grade ──▓────────────────▓─────────────────────────────▓── every steel
                        ▓   ┌────────────────────────────────────────────┐   structure in
                        └──▶│ vault, 1–2 m, lid + polystyrene:           │   concrete is
                            │ three IP68 boxes side by side —            │   bonded: one
                            │ battery (vented) · power (MPPT, BMS, fuse) │   earth system
                            │ · logic (Mayak, Kronos, cards, Palatine)   │   per site
                            │ one common earthing point,                 │
                            │ a short strap to earth                     │
                            └────────────────────────────────────────────┘
```

## What the station is protected against — and what it is not

**Declared: the conducted and induced surge, the 8/20 µs class. NOT the direct strike, the
10/350 µs class.** This is a rating, not an omission, and the bill of materials has always said
so: the family's gas tubes carry **10 kA at 8/20 µs repeatably and 2 kA at 10/350 µs**
(`../galvani/HARDWARE.md`). A direct strike is 100–200 kA of 10/350. **The ladder is two orders
away from a direct-strike part and was never meant to be one.**

**So the siting rule is a condition and not advice: the station stands outside the zone of direct
strike.** Not on the highest point, not under a down-conductor, not beside one. **Where the mast
must be the tallest thing on the site, that site takes an isolated air-termination system with its
own electrode**, at the separation distance, and the station touches none of it. That is the
builder's work and this project does not price it — but a station built without it is a station
that will be replaced, and the document says so rather than implying protection it does not have.

## Grounding — THE FIXED RULES

Eight rules:

1. **Every structure set in concrete IS earthed, always** — a pipe sunk 1,5 m into the soil is an
   earth electrode by construction, and so is the panel's frame and any post. Treat each as the
   strike terminal it is, **bond them all below grade into one system**, and where a solid
   low-impedance earth matters, help it (several rods in an array, target ≤ 1 Ω — never one
   lonely 2 m rod). **A ring electrode around the mast's base, serving all three boxes, is the
   natural form** — it buys a low potential GRADIENT across the site, which is what three boxes a
   metre apart actually need, rather than a low number on a meter.

   **The panel's frame is bonded or it is kept at the separation distance — "partly earthed" is
   not a state.** An unbonded frame near an air-termination system flashes over by itself, at a
   place nobody chose; a bonded one carries its share and is designed for it. Either is a decision;
   half of one is not. **Siting the array is two independent conditions and neither implies the
   other:** WMO's **≥ 10 m from the T/RH screen**, and clear of the separation distance from any
   air-termination system. A screen ten metres from the panel says nothing about where the mast is.

   **What this project specifies is the topology; what it does not specify is the electrode.**
   Rods, ring, depth, the resistance reached — those are local code, and local code differs by
   continent. The station declares which conductor bonds to which, through what, and at what
   class per port. **That part is the same everywhere; the numbers never will be.**
2. **Everything INSIDE stays maximally ISOLATED from it** — one continuous corrugated conduit
   per bundle (*The structures*), unbroken into the enclosure: containment by isolation. The mast takes the strike; the plastic keeps it out of the copper.
3. **The station's metallic ENTRIES — data, power, and the antenna coaxes — are EARTHED**, and
   the single-point earth is **one rule and no construction: there is exactly ONE common earthing
   point, and everything that joins earthing joins there.**

   **What it is made of does not matter.** Copper, the mast's galvanising, a driven rod, a bar, a
   bolt — anything deliberately conductive and deliberately earthed will serve, and this project
   does not specify which. **What it must never be is a structural point.** A fastener whose job
   is holding something up gets slackened, swapped, painted and re-torqued by somebody who is not
   thinking about earthing, and an earthing connection that shares a mechanical job is the one
   that is silently lost. **An earthing point is there to earth and to do nothing else** — that is
   the basis of every one of them.

   **Every SPD common, and the internal system's own bonding conductor, lands on that one point**,
   so the commons share a literal point and no potential can develop between them. **Two earthing points are a fault**: current then flows between them, and the
   path is through the equipment. Outside, a **short
   fat strap, 25–50 mm², flat braid preferred** (lower inductance than round wire), to its own
   rod — **and the rod is BONDED below grade to the mast's earth: one earth system per site.**
   Two separated electrodes put a strike's I × R (tens of kV) *between* two points that the
   coax shield and the PV wiring both bridge — separation buys nothing and arms the
   difference. The strap stays short; the bond conductor runs buried. Length before
   cross-section: inductance (~1 µH/m) barely cares about thickness, so every centimetre of
   the dump path is voltage at a fast front — but at surge currents the resistance bites too,
   and there the cross-section is a twenty-fold difference:

   | strap | R | drop at 10 kA |
   |---|---|---|
   | 2,5 mm² | ~7,1 mΩ/m | ~71 V/m |
   | 25 mm² | ~0,71 mΩ/m | ~7 V/m |
   | 50 mm² | ~0,36 mΩ/m | ~3,6 V/m |

   **That table is the SMALLER term and must not be read as the answer.** Cross-section fixes
   the resistive drop; **the voltage is set by `L·di/dt`, and inductance barely moves with
   thickness.** A metre of any of those straps is ~1 µH: at the mild 1,25 × 10⁹ A/s it is
   **1250 V**, and at a subsequent stroke's **200 kA/µs = 2 × 10¹¹ A/s it is 200 kV** — against
   7 V of resistive drop on the 25 mm² row. **So the cross-section is chosen for the coulombs —
   fusing and mechanical survival — and never for the voltage.**

   **And the strap is only one term of the path.** The whole of it, at a strike:

   | term | order | at a realistic front |
   |---|---|---|
   | the mast, ~6 m at ~1 µH/m | ~6 µH | 7,5 kV at 1,25 × 10⁹ A/s · **1,2 MV** at 2 × 10¹¹ |
   | the bond conductor and the strap | ~1 µH a metre | 1,25 kV to 200 kV a metre |
   | the lug-to-point lead | ~1 nH a millimetre | tens of volts to kilovolts |
   | **the electrode itself** | 1 Ω well made, **50–200 Ω for one lonely 2 m rod** | **30 kV at 30 kA** · **1,5–6 MV for the lonely rod** |

   **The sum is tens to hundreds of kilovolts and nothing on the bill of materials changes it.**
   That is not a defect in the design — **the design never tried to hold the site at zero.** It
   holds everything on the site at the SAME potential, so that the rise is common-mode to the whole
   station and no component sees a difference. **The difference appears only at the ends of what
   leaves the site**, which is exactly why the far end floats: bonding it would put those tens of
   kilovolts along the run.

   **And the surge never rides an interconnect cable:** the three-electrode tube sits at the
   enclosure entry and its common goes lug-to-point, never down a board-to-board cable whose
   centimetres all count — so on every Galvani board the tube's common goes to the lug, and no pin
   of either body carries it.

   **The internal system is HARD-BONDED to that point** — its own bonding conductor, straight onto
   the point, no gap and no component in the path. **One tie is not a loop**, and the internal
   system has exactly one: the coax hangs on an insulating bracket, every run's far end floats,
   the enclosure is plastic and the conduit unbroken. **A part in that path buys nothing and costs
   the one thing the whole scheme is for** — until it conducts, the internal ground stands its own
   sparkover away from the point that every SPD common lands on, and that is exactly the event the
   SPDs exist for: the tube at the entry throws its common-mode current at the point, and the
   internal ground has to already be there.

   ### The isolating spark gap — one case, and it is the site's, not the station's

   **An isolating spark gap belongs where there are two earthed systems**, which here is one site:
   the one whose mast must be the tallest thing and which therefore takes an **isolated
   air-termination system with its own electrode**. That system is meant to touch nothing of ours
   and the separation distance is what keeps it that way. **Where the geometry cannot give the
   separation distance, the two are bonded through ONE isolating spark gap** — it conducts nothing
   in service, so the isolation stands, and at a strike the two equalise, in both directions.

   **The part is a pure spark gap and nothing else** — no varistor, no combination: a varistor ages
   on every surge it sees, fails toward a short and ends as a leakage path across the very
   separation it was put there to keep. `ISG-50`, order code `A04086`, ČSN EN 62561-3, class N:

   | parameter | value |
   |---|---|
   | `Iimp` | 50 kA |
   | `U_imp`, rated impulse sparkover | 0,90 kV |
   | withstand, `U_WDC` / `U_WAC` | 50 V DC · 35 V AC |
   | insulation resistance | 100 MΩ |
   | enclosure, temperature | IP 67, −40…+80 °C |

   **The sparkover is a few hundred volts, never tens** — a 50 V gap strikes on the converters' own
   transients and on ordinary operating noise, and a gap that strikes in service is a short across
   the separation. **`U_imp` is the rated value at the standard 1,2/50 µs front**: the static
   sparkover is lower and a faster front overshoots it, so 900 V is the number to design against
   and not what the part does. It sits below the weakest insulation standing behind it —
   `750310349`'s 1000 V AC in `G-300-S`, ~1414 V of peak (`../galvani/HARDWARE.md`) — so it fires
   first, which is the whole order of operations.

   **It is the builder's part at that kind of site, like the air-termination system itself, and it
   is not on the station's bill of materials.** Where it is fitted it takes its own fixing on the
   earthing point, never one that also holds something up.

   **The antenna coax belongs to the internal system, top to bottom** — its shield lands on the
   internal ground and reaches the point on the internal system's own bonding conductor, with a
   DC-pass tube between centre and shield at the antenna. Bonding the shield to the mast at the top instead is the
   telecom practice for a **metal** building with a feeder entry plate, where the current is thrown
   off at the wall and never enters; **this enclosure is plastic, so a top bond would drive the
   strike down a few square millimetres of shield onto the internal ground.** It follows that
   **the antenna is mounted on an insulating bracket** — a metal mount bonds the shield to the
   mast through the antenna body and breaks the scheme from the other end, for the sake of a part
   that costs nothing.

   ### The joint made to it

   **The earthing point and its thread — the size, or the diameter where the thread is not
   metric — are designed from the FAULT CURRENT.** The thread also grows with the current and
   with the cross-section of the conductors arriving on it.

   **Czech electricians' practice, recorded as custom and not as a requirement of this design:**
   **M8 at the smallest**, and the stack is **serrated washer · plain washer · the lugs · plain
   washer · spring washer with the square-cut end · nut · second nut**. **Brass or stainless**,
   and stainless **A2 at the least — A4 at the sea and in chemical environments**. The whole
   joint is greased, with a grease chosen for water and for the chemistry of the site: **technical
   petroleum jelly** on brass, copper and stainless — water-resistant, soft from −40 °C, sold
   everywhere — and where aluminium meets copper a **zinc-loaded contact compound** made for that
   pair. **No silicone grease**: it creeps onto the contact faces and insulates them.

   **The other end of each strap is a source board's earth stud**, an M4 plated hole with a pad on
   both layers, where a serrated washer has no place: the stack there is **screw · plain washer ·
   the board · plain washer · the strap's crimped ring lug · plain washer · spring washer · nut,
   all brass** (`../galvani/README.md`, *The earth leaves a source board on a stud*).

4. **The far / potted end has NO earth — and none is missing**: a pod under the sea cannot be
   earthed, and **it must not be tried — an electrode there would INVITE the current
   through the pod** (floating is the protection: with no earth reference, common-mode has no
   path and rides over; in seawater a metal element is also a galvanic corrosion cell).
   Physics is on the floating side too: a strike into the sea dies within metres (~4 S/m —
   the ocean is the best electrode there is), a fast front disperses and attenuates over km
   of cable, and the surge finds its earth where one EXISTS — at the station (rules 1+3).
   What remains at the far end is the **differential** stress — small by construction (the twist)
   and bounded by the line clamps against the internal (−). **The far end carries no gas tube and no chokes** — the
   transil pair on the feed, the series resistors and the transil on the bus, and nothing else
   (`../galvani/README.md`, *The protection ladder*). **A tube there would have nothing to
   discharge into**: the three-electrode part degenerates into two gaps in series when its centre
   floats, and the differential stress the twist leaves is what a transil is for. The sonde's
   centimetre of plastic IS its insulation.

5. **Anything BURIED runs inside an insulation layer** — insulated conduit / jacketing rated
   **≥ 5 kV** — so the soil never gets a cheap way into the copper.

6. **A remote site is not earthed on purpose, and the cable's own impedance is what does the work.**
   This is a positive choice and not a shortfall. **A proper earth is expensive and a bad one is
   worse than none**: rods in an array, a mm²-class strap, the ground work and the check, at every
   site — and every millimetre of that strap is ~1 nH, so at 1,25 × 10⁹ A/s **a metre of it is
   1250 V that would not otherwise exist**.

   **What replaces it is already in the run.** A twisted pair's surge impedance is
   `Z₀ = √(L/C)` ≈ **110 Ω**, so a 10 kV front on the line drives ~90 A rather than kiloamps, and
   the series inductance of the run itself limits how fast even that arrives:

   | run | loop resistance | series inductance | current in 8 µs at a 10 kV drive |
   |---|---|---|---|
   | 100 m | 2,4 Ω | 60 µH | — |
   | 500 m | 12,1 Ω | 300 µH | — |
   | **1000 m** | **24,2 Ω** | **600 µH** | **~133 A** |
   | 2000 m | 48,4 Ω | 1200 µH | — |

   **A kilometre of cable is 600 µH and 24 Ω in the path**, which is a better surge limiter than
   most earths ever achieve.
   **So the practice is: pull it all in one conduit, lay it, and do not fight it.** The energy is
   spent in the cable, the far end floats, and the transil handles what differential stress is
   left.

7. **Why float-plus-one-point beats bonding everything, in the numbers.** A strike develops **~1–200 kV per metre of conductor**
   through `L·di/dt`, so **the more conductor you bond to earth, the more current you invite
   through it** — the voltage then distributes against earth instead of along the wire, and a
   heroic thick down-conductor simply carries more. Isolate everything, drain through the tube at
   the one point, and the whole station **rides the potential rise as one**: no differential path,
   nothing to blow through.

   **"Equipotential" bonding is a fiction at impulse speed.** The same 1 µH/m across a bonding
   conductor develops **25–200 kV per metre** at lightning `di/dt`, so anything bonded through
   metres of copper is **not** equipotential during the stroke. Bonding works for slow and AC
   faults; **at 100 kA/µs geometry rules.** Hauling 10 mm² around the station to chase low drops,
   and then arguing star against daisy-chain, buys nothing the float does not get for free —
   either way the surge current develops potential along the run.

   **And at impulse speed a star ground is not a star**: its legs are inductors. Where a site does
   bond, the professional form is a mesh, not a hub. This station bonds at exactly one point and
   floats everywhere else, which is the same conclusion reached from the other end.

8. **A remote Argus's boards are the sacrificial part, and spacing contains a strike rather than
   preventing one.** Where a remote Argus makes its own outgoing
   runs, four source power boards stand at a position with no electrode — fitted exactly as at a
   station, straps unconnected. **The accepted outcome of a direct strike there is the loss of the
   boards**, so the design goal is replacement rather than protection: the field-replaceable unit is
   a small board, and someone is travelling to the site either way. **Branch spacing inside the
   enclosure is a layout requirement and its job is stated honestly — it does not stop a strike, it
   stops one strike taking all four branches** — so the rule is the four source boards as far
   apart as the enclosure allows, each on its own entry, and no clearance table sets it.

*(A permanent lightning-exposed install still wants a local-code / ČSN EN 62305 review — this
is engineering intent, not a certification.)*

## Cable routing — metallic-run separation rules

Routing determines the exposure before any protection component does. The rules are
consistent with lightning-protection practice (IEC/ČSN EN 62305 separation principles;
ITU-T K-series telecom cable protection):

1. **Route through open, low-exposure corridors** — open field, treeless road verges.
   Isolated tall objects (trees, poles, structures) act as preferential strike terminations
   and elevate soil potential in their surroundings during a discharge; a longer route
   through open terrain is preferred over a shorter one passing tall vegetation.
2. **Maintain ≥ 5 m separation from any buried earthing system** — earth electrodes, down-
   conductor terminations, earthing strips, uninsulated buried conductors. Within that
   range the governing mechanism is ground-potential rise and jacket puncture (not
   induction); separation distance is the only effective mitigation. **The rule is for the
   middle of a run and for foreign earths.** Inside the station's own plot every run starts and
   ends at a bonded electrode; those runs share one trench with the bond conductor, which is
   what makes their two ends equipotential (*The structures*, rule 8).
3. **Where a crossing near a down-conductor is unavoidable: ≥ 1 m separation plus a heavy
   insulating sleeve** over the affected span.
4. **A direct termination on the buried run is outside the protection budget** — the design
   protects against the survivable classes (nearby strikes, induced and conducted
   remnants); the routing rules minimise direct-termination probability, the GDT ladder
   handles the near miss, and the sacrificial-module doctrine bounds the damage at the ends.
5. **The buried insulation rule applies** (grounding rule 5): the run rides its ≥ 5 kV
   insulated conduit/jacketing.
6. **The answer to a genuinely exposed route is glass** — the optical boards exist precisely so
   a metallic run is never forced through high-exposure terrain
   (exposure, not distance — `../galvani/README.md`).

### The interconnect cables — the drawn ones

**A cable with a connector on both ends is a made part and gets a drawing like any other.** The
field runs are not on this list: outdoor Cat 6, the 4-wire ModBus arm, the 2-core 48 V feed,
the PV string, fibre and the GNSS coax are cut to length on site and glanded, so what they need is a pair map and that lives with the port
boards (`../galvani/README.md`). What is drawn is the short stuff inside the enclosure.

| the cable | ends | topology |
|---|---|---|
| **the port cables** — host → its Galvani boards | **two ribbons** of 1,27 mm flat cable: a **12-pin data** cable to the communication board, an **8-pin power** cable to the power board. **`FC-12P` / `FC-8P` IDC sockets** onto **`BX2.54-2xNA`** headers, the socket's strain-relief cover folded and a tie behind it — nothing latches (`../galvani/README.md`) | straight through, one cable per board |
| **the 12 V wire** — the battery rail, from the fuse field to every board that makes its own rails: Mayak, Kronos, each card, Palatine, Sputnik, each `G-48-S` / `G-300-S` / `G-12-S` / `G-24-S` | 2,5 mm², two two-pole terminals per board — an input and a tap (`DGPS2.5R-5.0`, two two-pole blocks); between the vault's boxes one 2,5 mm² pair through a gland | a star from the fuse field, one fuse a board; no board carries it across itself |
| **the in-box link** — host ↔ host: a card to Palatine or Sputnik where either shares the enclosure, Kronos to Sputnik's time port, **and the MasterNOD trunk, Mayak ↔ card, ×4, which is the same cable** | the **12-pin data** body at both ends, **crossed**: `TXD`↔`RXD`, `ID`↔`ID_RET`; `CLK/PPS` and `GND` straight — **6 wires** — the 12 V off the wire on each host's own terminals (`../galvani/README.md`, *The connectors*). **A ribbon of another colour, an `X` label at both ends**: the cable from a host to a Galvani board is straight, and the two must be told apart on sight | point to point, crossed |
| **the 12 V wire at a remote Argus** — a `G-300-U-40`'s terminals → the Argus → its four `G-300-S` | the same terminals, tap after tap | a chain of taps |
| **the time bus**, Kronos → the cards and the Mayak | **one 10-pin ribbon** (`FC-10P` onto `BX2.54-2xNA`) off Kronos's connector, an IDC tap per card and for the Mayak, **30–40 cm in all**; every node carries 2× 10 Ω per pair and a jumpered 80,6 Ω, fitted on Kronos and on the last node only; `CLK` and `PPS_K` as M-LVDS pairs, `DS91C176` drivers and `THVD1450` receivers, `SDA · SCL · ATTN` beside them | one bus along the card row, no terminator plug |

**The in-box link is a cable and not a board.** It carries plain 3,3 V logic — `TXD/RXD` crossed,
`CLK` or `PPS` on the one clock wire, the two `ID` legs crossed so each host reads the other's
number — **six wires**, and the 12 V off the wire on each host's own terminals — with **no transceiver,
no barrier, no ladder and no terminator**, because none of that exists for a run that never
leaves the box. **It makes no rail
either**: the host on the far end makes its own from the 12 V. The crossing is made in the cable —
**the six wires are the data body's conductors 1–6 (`CLK/PPS` · `GND` · `TXD` · `RXD` · `ID` ·
`ID_RET`), and 3–4 and 5–6 are swapped at one end**, two adjacent pairs turned over — and the
plug fits every socket in the box the same way, so every host carries one pinout. The `CLK` series resistor sits
at its **source**, on the host board, not in the cable. **It is drawn with the cables and not
with the boards**, and it has no BOM (`../galvani/README.md`, *The in-box link*).

**The fuses belong to the cabling and not to a board**: PV → MPPT (a fuse or a hydraulic-magnetic
breaker at ~1,1×), MPPT → pack, pack → BMS, and behind the BMS the enclosure's fuse field, **one
fast-acting cartridge to a board**, rated to open only on a real fault (`../galvani/README.md`,
*The rails*). No board carries a fuse of its own; each supply's current limit is its own. **No
rating is written here, on purpose**: a fuse's rated current is not its breaking current, the fault
current comes from the pack, the BMS trip point and the cross-section run to the fuse field, and a
fuse protects the cable as much as the load — so it is sized per install, on the branch's total
input power and its thinnest wire, and never one size for the station.

**What a cable's entry carries, and it is four things:** where it goes from and to · whether
anything crosses inside it or it is straight through · the **recommended** length and conductor
cross-section · and the connector type at each end. Nothing else belongs on it.

**Length is a recommendation and the build owns it.** The actual number falls out of the
enclosure and the card spacing, so the rule is **as short as the routing allows**, and a cable
that has to be longer is looped rather than redrawn. In practice they come out much the same
length anyway — everything on this list lives inside one box, within about half a metre.

**The trunk cables are the one place where equal length is not just tidiness:** all four run the
same length, construction and connectors so the cards are electrically interchangeable and
nothing is ever tuned per port (`../bifrost/HARDWARE.md`). Timing does not need it — no edge rides
the trunk — but service does.

## Enclosure — classic IP68, three boxes

- **A classic IP68 box**, plastic by default, **metal variant when more ruggedness is wanted** —
  and **three of them side by side: battery · power (MPPT, BMS, the fuses) · logic**
  (Mayak, Kronos, the cards, Palatine, gel-topped). Three boxes are one box electrically — the
  12 V crosses on a metre of 2,5 mm² — and separate for what is not electrical: a battery fault
  stays in its own box.
- **The battery box vents.** A LiFePO₄ cell that fails off-gases, and a sealed IP68 box holds the
  pressure; a membrane vent (a Gore-type plug) above the lid lets gas and the summer's
  condensation out and nothing in. The logic and power boxes stay sealed.
- **What the metal variant buys electrically: a shield that is frequency-selective the right way.**
  Skin depth in a 2 mm aluminium wall against the wall thickness — transparent to what is measured,
  opaque to what switches:

  | | 1 Hz | 20 kHz | 140 kHz | 694 kHz |
  |---|---|---|---|---|
  | δ (aluminium) | 84 mm | 0,58 mm | 0,22 mm | 0,10 mm |
  | 2 mm wall | ~0 | 3,5 δ | 9 δ | 20 δ |

  So the box and a switcher's frequency **multiply**: at 20 kHz the wall is the limit, above ~140 kHz
  the seams and the cable are. Two conditions come with it — **a metal box is never the answer near
  Gauss, nor for Tesla** (a ferrous or high-µ enclosure distorts and drifts the field Gauss
  measures — the same rule that keeps a borehole's casing non-ferrous — and a conductive wall shields and
  eddy-damps Tesla's 5–512 kHz band: its box is plastic, rods inside), and **the wall does nothing for the leads**: without a
  feedthrough filter or a common-mode choke at the entry, the cable carries the current straight back
  out and the box shields the quiet part.
- **Cable entries: cable glands to IEC 62444, metric M16 or M20** — the PG9 and PG11 of older
  European stock take the same holes — **polyamide, UV-stabilised, IP68 to IEC 60529**, each one
  sized so the cable's outer diameter sits inside its clamping range, the thread set in sealant.
  Run the cable through plastic insulating grommets/glands where it passes the structure.
- **If it goes in the ground, over-seal it:** sealant over the glands **and** round the box
  perimeter — then standing water is not a failure. **The sealant is a removable one**, so the box
  can still be opened: a **neutral-cure silicone or an MS-polymer** sealant. **Never an acetoxy
  silicone** — it cures by giving off acetic acid, which the closed box keeps and which corrodes the
  copper and brass inside it; a vinegar smell while it cures is the test.
- Boxes mount **on the ground, on the mast, or on the frame** — drill a hole, pass the insulating grommets
  through, pull the cable.

## The head enclosure — cards on edge, in guides

**A fully populated station is a cabinet, not a box.** Counted:

| | boards |
|---|---|
| logic — Mayak · Kronos · 4× Bifrost · Argus · Palatine | **8** |
| Galvani port positions — 4 cards × 4 ports | **16** |
| Galvani port positions — Argus's 4 mini segments | **4** |
| Galvani port positions — Palatine's 4 ModBus arms and `PWR EXT` | **5** |
| | **33** |

Quark-Tubes is outside and does not count; nor do the units themselves.

**Cards stand on edge in guides, screwed to the mounting plate.** Card guides are a stock item —
plastic rails that take a screw at each end — so the plate carries the guides and the guides carry
the cards. Nothing is fastened board by board and a card is pulled by hand.

**Pitch: 20 mm.** No board in the family carries a tall component, so the pitch could be tighter —
what holds it at 20 mm is the **1×9 optical module** on the glass boards, and a pitch that varies
by card position is a build error waiting to happen.

**An 800 × 600 plate holds it with room.** 800 mm at 20 mm pitch is **40 slots a row**, so 33
positions is **one row and a stub of a second**. Board height sets the row spacing; at the
family's ~100 mm that is ~250 mm of the 600, leaving the rest for the battery, the glands and the
cable loops.

**Order the rows by what plugs into what:** Mayak at one edge, the four Bifrosts beside it, Palatine
and Argus below — about 20 cm of the first row — and the Galvani cards fill the rest and the second
row. Every logic-to-logic link is then a short cable inside one row (`../galvani/README.md`).

**A real station is nowhere near this.** Most sites run a handful of spurs, so the plate is mostly
empty and a smaller box does. The 33 is what the enclosure must not *preclude*, not what it carries.
**A vault holds the normal station; the full cabinet stands above ground.** A concrete ring takes
three IP68 boxes, not an 800 × 600 plate — a station that fills the plate is built under a roof
(*Above-ground*).

## Burial — a simple vault (the box's hole, not the sensor's)

Distinct from a sensor's borehole: this is the shallow vault the **enclosure and the pack** sit in. The
thermal case is `../core/POWER.md` §4; mechanically it is deliberately crude:

- A **shaft** — a concrete ring (skruž) or even a **thick plastic pipe** — buried **~1–2 m** deep, with a
  **waterproof plug / rubber seal at the bottom**.
- A **lid on top**, and a **slab of polystyrene under the lid** so the vault equalises to the surrounding
  ground temperature rather than the surface air.
- The boxes go in the shaft **side by side** — battery, power, logic; each is IP68 and over-sealed, so
  standing water in the shaft is not itself a failure — the seal is the barrier. **The battery box's
  membrane vent is carried up through the lid.**

**Check the water table, and drain accordingly.** Where groundwater is **low**, let water leave: an
**openable shaft** with the bottom open or a **French drain (trativod)** under it, and the boxes **slightly
raised** so water soaks away before pooling. Where it is **high**, lean on the sealed-plug + IP68 route (a
bog is fine — the seal, not dryness, protects the box). Either way the electronics never depend on the
shaft staying dry.

## Above-ground option — a roof is enough

**Above ground is the build for a full cabinet station and for an LTO pack; a LiFePO₄ pack above
ground charges only through a BMS that derates below 0 °C** (`../core/POWER.md`). The
weatherproofing is a simple **canopy / roof (stříška)** — or a small
"little house" — over it, **glands at the bottom** so water can't track in, and the reflective finish
(below). That is the whole weatherproofing.

## Finish — one solar-reflective white over the whole structure

**A dark mast or frame reaches 70–80 °C in full sun** — a thermal column that heats every sensor
bolted to it, the temperature channel worst of all, and UV- and thermal-cycles the materials. So
**the whole structure takes one high-reflectance white coat** — mast, frame, the sensor post, enclosures,
detector housings, the radiation shield's outer shell and cap, the rain gauge's shell. It is thermal
design, the physics of a Stevenson screen, and it covers UV and weather in the same coat.

**No product is named, because the station is built where it stands** and a tin bought in one
country is not sold in the next. The coat is any exterior coating that meets every line of the
three tables below; the builder takes what the local market carries and checks it against them.
A figure quoted to a standard equivalent to the one named is accepted — the named ones are the
ones most sheets carry. The two classes that usually meet the tables are the **solar-reflective
roof coatings** (cool-roof class, acrylic or elastomeric) and the **two-component aliphatic
polyurethane topcoats**; that is where to look, not a requirement.

**The optical properties — the coat is chosen on its numbers, not on "looks white":**

| property | requirement | standard / how it is checked |
|---|---|---|
| **total solar reflectance**, 300–2500 nm, **near-IR included** | **≥ 0,80 new, ≥ 0,70 after three years of exposure** | ASTM C1549 (reflectometer) or ASTM E903 / ISO 9050 (spectrophotometer); the aged figure to ASTM D7897 or a three-year weathered rating. **A sheet that quotes only visible reflectance, whiteness or L\* says nothing about the near-IR** — half the sun's energy, invisible, and the reason many white paints reflect the visible and still cook — **and is not accepted** |
| **thermal emittance** | **≥ 0,85** | ASTM C1371 or ASTM E408 — why matt white beats bare shiny metal, which reflects but barely emits |
| **solar reflectance index** | **≥ 100**, which follows from the two above | ASTM E1980 |
| **colour** | **pure white, untinted** — rutile TiO₂ as the white | the tin: a base white with nothing added. An off-white or "warm white" is tinted with carbon black, which absorbs the near-IR the white reflects |

**The durability — exterior, and rated for it:**

| property | requirement | standard / how it is checked |
|---|---|---|
| exposure | **rated for permanent exterior exposure** — never an interior paint, which chalks and washes off outdoors | the sheet |
| UV and weathering | **≥ 1000 h of accelerated weathering with no cracking, no flaking and chalking rating ≤ 2** | ISO 16474-2 / ISO 4892-2 (xenon) or ISO 16474-3 / ASTM G154 (fluorescent UV); chalking to ISO 4628-6. A polyurethane is **aliphatic** — an aromatic one yellows and chalks in sunlight |
| temperature | **service range containing −40 … +80 °C**; **no crack on a 10 mm mandrel at −20 °C** | the sheet; ISO 1519 on a coated offcut after a night at that temperature. A steel mast runs at 70 °C under the coat before it is white and a winter night takes it the other way |
| water, frost, salt | water-resistant and freeze-thaw stable on the sheet; **steel at the sea takes a paint system to ISO 12944 category C5, durability high; inland C3** | the sheet; ISO 12944 names the system, its primer and its thickness per category |
| dirt pickup | **low dirt pickup** — grime is reflectance lost, and the aged-reflectance line above is what measures it | ASTM D3719, or the aged figure |
| adhesion | **class 0 or 1 in a cross-cut test, on every substrate of the build, with its primer**, after seven days of cure and again after 24 h in water | ISO 2409 or ASTM D3359, on an offcut of each material — a knife, a ruler and a roll of tape, done anywhere |
| cure | **air-cures at the site's temperature** — no oven and no baking schedule | the sheet; a factory finish that was baked is accepted, below |

**The substrates — what each one takes under the coat.** A white coat that peels in a year is a
primer wrong for the material, not a paint wrong for the site; the cross-cut test above decides,
on an offcut, before the structure is painted.

| substrate | where | preparation and primer |
|---|---|---|
| **hot-dip galvanised steel** | the scaffold-tube mast, a steel frame or post | degreased, white rust brushed off; a **primer sold for zinc** — etch or wash primer, or a two-component epoxy primer — or a sweep-blast. **Never an alkyd**: zinc saponifies it and the coat comes off in sheets within a year |
| **aluminium** | enclosures, brackets | degreased, abraded to dull; an etch primer or a two-component epoxy primer |
| **GRP** | detector housings, the hail shell, the sensor post | the gelcoat sanded to dull and washed — a moulding carries release agent; the system's own primer where the sheet asks for one |
| **polyolefins — PP, HDPE, PPR** | the radiation shield's shells, the rain gauge's shell where it is a plastic pipe, Marconi's ring | **nothing holds on a bare polyolefin.** Abraded, then a **polyolefin adhesion promoter** (a chlorinated-polyolefin primer) or a flame treatment, then the coat; the cross-cut test decides. The alternative is a part **bought white in a UV-stabilised compound**, its reflectance measured and not assumed |
| **PVC, ABS, polycarbonate** | IP68 boxes, grey PVC pipe | degreased and lightly abraded; most exterior acrylics and two-component polyurethanes hold directly. A solvent-borne coat is tried on an offcut first — a strong solvent crazes polycarbonate |
| **a factory finish** | a white powder-coated steel, a white gelcoat, a white UV-stabilised plastic | accepted where it meets the optical table, **measured**, and left alone |

**The site check, where no reflectometer is to hand**: two plates of the structure's material in
full sun, backed with insulation, one in the coat and one bare or dark, a thermometer on the back
of each; the white one runs within ~10 K of the air where the dark one runs 30–40 K over it. It
confirms the sheet; it does not replace it.

**Renewal**: the coat is washed when it greys, because grime is most of the reflectance lost and
a wash brings most of it back; it is repainted when a wash no longer does. A coat that chalks
when a hand is wiped across it is due.

**A white frame does not replace the radiation shield** — the white lowers the frame's own heating;
direct sun on the element is the shield's job. **Detector housings are glass-fibre GRP, never
carbon**: carbon's one edge is stiffness a 2–3 mm glass tube already has, and it brings
conductivity, HV and galvanic problems.

## Serviceability & longevity — there is almost nothing to service

- **Gel, not potting — so it is repairable.** Sealed units are set in a **removable gel**, not hard casting
  compound: peel the gel, wash the board, **reflow / swap the dead IC, re-gel, done** — a home repair, not
  scrap-and-replace. The gel still gives moisture and vibration protection; it just does not weld the
  board in forever. **The gel is a two-part silicone dielectric gel**, the soft re-enterable kind sold
  for potting connectors and modules: it cures soft at room temperature with no oven, is rated over
  **−50 … +150 °C** or wider, has a volume resistivity of **≥ 10¹⁴ Ω·cm** and a dielectric strength
  of **≥ 15 kV/mm**, and releases nothing acid while it cures. A hard silicone rubber or an epoxy is
  not a gel: neither peels.
- **Long life by parts, not luck.** **No electrolytic anywhere** — film / **polypropylene** caps (105 °C,
  run ≤ half their voltage), quality ceramics, better resistors where it matters; where bulk
  capacitance is needed on a node of ~25 V or less, the family's **hybrid polymer, 47 µF 50 V, ZA
  class** (`../galvani/HARDWARE.md`), and none at all on a pressure board. Nothing runs hot, so there
  is no obvious wear-out mechanism; barring a strike, years with nothing to touch.
- **The one genuinely stressed part** is the **RS-485 termination** — the 10 Ω in series on both
  legs, and the 80,6 Ω put across by its jumper at the two ends of a run — and even that only cooks
  **after a surge**, which the isolation and the sacrificial front contain.
- **The consumables are the pack and Pluvius's pump tube** — a peristaltic tube rated ~1000 h
  (`../pluvius/HARDWARE.md`); nothing else on the station is replaced on a schedule.
- **What decides life is build quality, not silicon.** With the cabling pulled **inside the mast**, the real
  failure path is an all-outside box where **UV degrades the cable behind the glands** — so get the **PCB,
  the terminals/glands, and the cable jacket** right (plus the reflective finish) and the electronics
  outlast the install.

## Optimise the build and the cost — not the data

Construction must also weigh **manufacturability and financial cost**, not just "does it work": prefer the
cheapest build that meets the conditions (a pipe with drilled holes over a bespoke welded mast), and spec
by **function, not part**, so it can be made in volume for a fraction of the price. **This must never come
at the expense of the quality and accuracy of the measured data** — the frugality lives in the *structure
and the process*, never in the physics the sensor needs.

**Where this design stops.** It is drawn for the ground most of the planet offers: soil that can
be dug, a sun that comes back in winter, a vault that stays above freezing. Permanently frozen
ground, a polar night or a site with no winter sun each take a different pack, a different vault
and a different power budget — that is a different build, not a variant of this one.

## The through-line — buildable beats perfect

All of the above is **classic material, off the shelf, forgiving**: a pipe, conduit, an IP68 box, IP68
glands, sealant, a concrete ring, rods, reflective paint. The specifics are left to the **builder** on
purpose — over-specifying the mechanics is how a project never gets built. The *intent* is fixed —
insulate the cable inside the mast, couple the sensor to rock, bond at one point, seal a buried box
hard, reflect the sun — and the builder reaches it with what they have.

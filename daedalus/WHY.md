★ N.I.C. ★

# Daedalus — the graveyard

Rejected alternatives and superseded states, with the reason. Nothing returns from here and
nothing is deleted from it.

## The EDA sheet-numbering scheme — four station-wide series and a board-number axis

`daedalus/ELECTRICAL.md` held four station-wide series — **100** POWER, **200** TIME, **300** DATA,
**400** PROTECTION & PORTS — each number assigned once and never moved. Nineteen were issued
(`110`–`150` · `210`–`250` · `310`–`350` · `401`–`406`, `412`–`415`), and board numbers on a second
axis — `101` Mayak, `201` the Bifrost/Argus card.

It went because the hundreds were meant as a project folder's four document categories and were
built as signal domains instead; a measuring board (Quark-Tubes, Photon, Positron, Tesla, Marconi)
fits no domain;
**the two axes collided** (board `101` against the 100 series, board `401` against sheet `401`,
`G-48-S`); it gave named boards a second identity; and nothing was drawn, so **no number was load-bearing**. Replaced by named
files per project, a board found in its owning `HARDWARE.md`; there is no schematic.
`ELECTRICAL.md` was deleted with it; the station's wiring map is `../core/COMMS.md`.

## Daedalus as the keeper of a board list — superseded

`ELECTRICAL.md` carried a table of board numbers, `README.md` the construction, numbering and file
index together. **A board list in Daedalus goes stale the first time a board moves**, and a builder
reading about a board is already in its project. Daedalus names no board and no part.

## One mast carrying everything — superseded

The reference build was one ~2 m pipe carrying every head and the solar panel, the enclosure at its
foot. **A panel runs at 50–60 °C in sun
and a thermometer in its plume reads the panel**; WMO wants a heat source ≥ 10 m from the
air-temperature sensor for class 3, 30 m for class 2. The panel now stands ≥ 10 m away on its own
frame (`CONSTRUCTION.md`, *The structures*). Three or four fixed masts
went for the same reason: **the rules fix distances, not structures**.

## A 10 m sensor mast — superseded

WMO's 10 m wind reference set the mast at 10 m and, by the 2× obstacle rule, the rain gauge 20 m
off and the plot at ~30 × 30 m. **A 10 m mast is a guyed or lattice structure on a footing**, and
the station is a plot, not a tower. Wind at 2–3 m is representative with a known height
correction, and the station fits inside the 10 m panel-to-thermometer distance.

## The drilled plastic tube inside the mast — superseded

A drilled plastic tube in the steel pipe's bore held the conduits, its holes adding an air gap to
the ~10 kV standoff. **Plastic inside steel traps water and the pipe rusts from inside, unseen.**
Conduit wall and jacket carry the standoff alone (PVC ~20 kV/mm, creepage along the conduit).
Replaced by a double-wall conduit or thick-walled hose and a drain hole at the tube's foot.

## Over-sealing the battery box — superseded for that one box

Every vault box was over-sealed alike against standing water in the shaft. **A failing
LiFePO₄ cell off-gases and a sealed IP68 box holds the pressure**; the battery box vents through a
membrane plug above the lid, the logic and power boxes stay sealed.

## The internal system on the earthing point through an isolating spark gap — superseded

The internal system's bond reached the common earthing point through a spark gap of a few hundred
volts, to stop an earth loop the single-point rule already prevents. **The gap added
the one potential difference the architecture refuses** — until it strikes, the internal ground
stands its sparkover away from the point every SPD common lands on, just when the entry tube throws
its common-mode current there. The internal system is hard-bonded. The gap keeps one place, the
site's: an isolated air-termination system inside the separation distance. Nor is there a second earth
to hold off: every board hangs on the pack, and one on its own source at a live station
(`../core/BRINGUP.md` §0) is a working practice, not a reason for a part in the bond.

## Under-clocked processors, a terminator plug, one consumable, the finish by product class — corrected

Three lines were wrong: nothing is underclocked, the processor rate is one number per tier
(`../core/POWER.md` §1); there is no terminator plug, the 80,6 Ω is a jumper on the boards at a
run's two ends; and the pack is a consumable beside Pluvius's pump tube. The finish named a trade
class and a price — a cool-roof coating (SRI ~102–107) or a 2K polyurethane white, ~1–2 k Kč — with
road-marking paint as the cheap option. **A station built
on any continent cannot be told a trade class and a Czech price**; the section gives properties
with their standards, substrates with primers, and a site check. Road-marking paint went too: made
for asphalt, it holds on nothing here without primer and its near-IR reflectance is published
nowhere.

## The thermometer's shield on the sensor mast — superseded by a post of its own

The shield hung on the mast under the wind sensor and antennas. It fails WMO siting twice: classes
2 and 3 allow no shade with the sun above 7°, and a shield under a pipe is shaded whenever the sun
passes behind it; and **at 2 m on the strike terminal it is inside side-flash range**. It stands on a 2 m non-conductive post 2–3 m
poleward of the mast. The wind stays on the mast top as class 4S, which a 2–3 m anemometer is by
WMO's definition.

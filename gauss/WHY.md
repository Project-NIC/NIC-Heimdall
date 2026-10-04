★ N.I.C. ★

# Gauss — the graveyard

Rejected alternatives and superseded states, with the reason. Nothing returns from here and
nothing is deleted from it.

## The lone spur behind a Bifrost — superseded by the mini retarget

Full-rate Gauss as a NOD on its own remote Bifrost spur, NodBus framing, one unit per spur at the
bottom of the rate ladder. **The NodBus type space carries no Gauss code**, so as mini type 1
behind an Argus it cannot be addressed there; a remote pod is a remote Argus segment, optical where
copper ends.

## The ModBus form — a duty-cycled variometer under Palatine, dropped

Gauss as ModBus type 4 too: a variometer polled once a minute on a Palatine arm, the board sensing
at power-up which wire it was potted onto. It cost a `THVD1450`, a four-wire input, a second
identity and line front in the firmware and a `Vin` divider, and **it lost the pulsation band** a
64 or 128 Hz magnetometer on a clocked segment is for; a minute product is an average of the mini
stream. A land station with one Gauss adds an Argus; ModBus type 4 is free.

## The ±1,5 ppm TCXO, and the crystal after it — removed

A KDS DSB-class 16,367667 MHz TCXO, ±1,5 ppm over −40…+85 °C into `OSC_IN`, for a sonde holding
its own time between anchors. Every form takes the rung on a timer capture, not HSE, so **the
oscillator is out of the time path**: a ±1 % source moves 19 ps across the 1,907 µs between sync
edges at 2¹⁹. It owes only the UART's baud tolerance, which commodity covers three orders over; the
`RM3100`'s rate is its own and the reading is converted. The unclocked battery build that kept the
TCXO is gone.

The house crystal lost next: **a crystal is a sealed gas cavity** and the fill passes hydrostatic
pressure to the parts. The HSI runs disciplined by the rung through the PLL
(`../core/blocks/clocks.md`); a clock multiplier off the rung, the fallback, fell on power
(`../core/WHY.md`).

## The standalone unit as "the no-Quake case" — superseded

Built "only if there is no Quake node", earlier a MOD autodetecting its attachment. The condition
was never Quake but the chip — whether any sensor in the build carries a magnetometer. The MOD and
the autodetection went with the ModBus form (above); one board, no variants.

## A ferrite bead on the RM3100's rail — superseded

A bead is soft iron; beside the coils it distorts the field. An RC replaced it, itself superseded
(below).

## The buck pair, the LDO and the RC on the RM3100 — superseded

A second `LMR43610` to 4,0 V, a `TPS7A2033` to 3,3 V and 33 Ω + 10 µF + 100 nF. The buck idled at
microamps, the LDO bought nothing against the 50 mV ripple limit, and the 33 Ω dropped `DVDD` below
the H523's SPI level, though the sheet rates the inputs only to `DVDD`. One buck and an inductor
per branch replaced all three, at the cable end, a tube's length from the coils.

## `LINE_EN` on PE3 — superseded

A GPIO pulled to run, kept for the bench. Nothing switches the Galvani boards at a unit end:
`LINE_EN` is 10 kΩ to 3,3 V in the socket, PE3 free.

## The in-box Gauss — superseded

`CURRENT` read 0 for a Gauss in the station enclosure with no power body. A measuring unit is never
in the box; Gauss stands at the far end of its run behind a unit-end Galvani pair.

## A tilt sensor in the pod — dropped

An `SCL3300` for live tilt, later an optional variant. An active part with its own supply currents
centimetres from the coils is **the dynamic-field polluter the layout removes**, and orientation is
static: the calibration recovers the axes, a turned pod is a step in the components with |B|
unchanged (`CONSTRUCTION.md`). A cavity-ceramic MEMS part would also have barred the sea build.

## "No deep sea" — superseded by the sea build

The family stopped at the shelf, ~30–50 m, oil an if-ever. Without its last cavity parts — crystal
and tilt sensor — the board goes to sea beside Pascal, ~250 m on a PVDF tube; the 8 km deep build
is Atlantis's, shelved for cost (`CONSTRUCTION.md`).

## A compliance bladder — not fitted

Sized for the fill's compression. Room temperature to 4 °C at 25 bar, the fill shrinks ~0,6 % more
than the PVDF, which flexes to take it; a bladder adds a second boundary material, and elastomers
pass far more gas than PVDF.

## A hydrophone — not built

Gauss hears the tsunami magnetically and Quake the T-phases on land; a hydrophone is a
pressure-rated acoustic front for a signal already covered.

## A land build and a sea build on one tube, opened for service — superseded

Land was PPR, a gland and ordinary connectors, opened for service; pods were "potted through, no
gland, no connector". Two builds on two materials — PPR or HT on land, PVDF at sea — both sealed:
**a well floods and a connector is a seal**, so land is IP68 through one multi-stage gland, the caps
fused.

## The 3×3 grid, the line and the sector fan — superseded

A 3×3 grid at 0,5–1 km or a line along the cable route, ~4 to ~16 sondes; on a one-arc island a
4–5-sonde fan facing the source. Now a ring of 4, 8 or 12, Pascal's rule too; **every sonde is a
laid cable, so twelve is the ceiling**.

## The PLL reference — 16, 4 and 2 MHz refused

`FRACN` pulls one step of the integer `N`, `f_ref / 2¹³` against the VCO: a lower reference buys a
finer step and loses high-side pull; the low side is −1,31 % unless stated. 16 MHz (`M` 4):
+4,86 %, 7,3 ppm a step, but on `f_PLL_IN`'s ceiling exactly, which the HSI's +0,47 % (16,075 MHz)
exceeds. 4 MHz (`M` 16): +0,16 %, 1,8 ppm, under the fractional mode's characterised floor. 2 MHz
(`M` 32): −0,58 / +0,16 %, 0,91 ppm, under it and at `f_PLL_IN`'s minimum. Fitted: 8 MHz (`M` 8),
+1,68 %, 3,6 ppm, the step dithered out.

## `HSITRIM` set once at bring-up — superseded

Closed once from the first gate: a pod under water sits at one temperature. **On a stake in air it
does not** — the HSI drifts −2 / +1 % a year against `FRACN`'s −1,31 / +1,68 %. `HSITRIM` steps a
code whenever `FRACN` nears its window's edge, in Pascal and `Quark-Tubes` too. The rest stands: the
reference never stops, nothing downstream is phase-sensitive, a lost rung is a lost link so
holdover never carries the station's time, and no part may be added at the end of a kilometres-long
feed.

## The interferer table and the fixed-frequency converter — superseded

A 100 nT interferer's residual through the RM3100's ~1 ms window was tabled — 100 nT at 128 Hz,
9 nT at 3,5 kHz, 0,23 nT at a 140 kHz island converter — and the converter held to 140 kHz, no
burst, no boundary mode. The unit power boards are boundary-mode flybacks whose frequency follows
the load, and the `LT8316` bursts below its floor; **the rule is distance, not frequency** — ~80 cm
from buck to coils. The window's upward filtering stands.

## The pressure arithmetic on a crystal's lid — out

"~20 MPa at 5 bar and ~2 GPa at 30 bar" for a 0,1 mm lid over a 2,5 mm span do not scale with the
pressure. The rule stands without them: a crystal is a gas cavity and the fill passes the pressure.

## `0x0015 MUX` — gone

Selected which supply byte streamed; it went with that byte (`../core/WHY.md`); `0x0015` is free.

## A hybrid polymer capacitor at the 12 V input — superseded

The house bank, 47 µF hybrid polymer beside ceramics, in a pressure-balanced body; **a hybrid
polymer is a can with an electrolyte and a seal**, barred from every pod. Now ceramic only, 5×
10 µF 50 V: the pod draws 3–10 mA, and the regulated 12 V carries no transil clamp to reserve
against.

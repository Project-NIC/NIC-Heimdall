★ N.I.C. ★

# Pluvius — the graveyard

Rejected alternatives and superseded states, with the reason. Nothing returns from here and
nothing is deleted from it.

## The first draft of the board — superseded, its sheet deleted

`STM32U073RCT6` + `ADS1235` + an on-board 485 transceiver, `PSM712` + `5KP16A`, MOSFET pump drive,
a OneWire thermometer. The U073 lost to the H523, the protection to the Galvani boards' SMD
ladder; a second quantity is a ModBus type of its own. One of the nine pre-Galvani sheets deleted.

## The pump's drive on the Pluvius board — superseded

A `TPS26601` eFuse, the MCU on `EN` and `FLT`, a link to raw input or a second isolated 12 V board
— **all workarounds for the arm's 0,5 A**, moot once the head had its own supply, whose converter
soft-starts and limits current.

## The pump head inside the enclosure — never true

"The pump head lives inside the enclosure, so the water in it never freezes." The head is at the
gauge and its box freezes at −40 °C too; draining happens above freezing.

## "~13 running hours a year" — wrong arithmetic

13 h is a 20 ml/min head. 12–20 L a year at 600–800 ml/min is 15–35 minutes; standing ages the
head, not running.

## "A modest cell grade is admissible" — half true

The drain's fresh zero removes **only the zero's thermal drift**; non-linearity, hysteresis, creep
and span drift stay. The class is bought: `C4` floor, `C5` working, `C6` where available.

## Loading the cell to ~70 % of capacity — a rule from another instrument

It optimises an absolute reading; here every error that matters scales with load, so a lightly
worked cell is better. 2 g on 30 kg is 1/15 000, nothing for the converter.

## The arm-fed head — retired

A ~12 V head on the arm's 12 V. Its 0,4 A leaves ~4,5 W, **under a peristaltic head's 10–15 W
knee** — a full vessel in hours — and the arm's `SN6507` shortens its pulses on overload, so a
stalling head would silently pull down every sensor on the arm. Per watt: ~2 ml/min at 2,4 W, ~42
at 12 W, ~36 at 22 W, ~60 at ~50 W. The tube's occlusion sets the friction whatever the flow, so a
small head pays it on a fraction of the delivery.

## `WEIGHT` at `0x0000` — superseded by the house map

The house map puts the reading at `0x0000` and the second value at `0x0001` on every unit: `RATE`
and `HOUR` lead, `WEIGHT` takes the `RAW` position. The numbers changed, not the unit.

## The V1.0 sheet's netlist review — closed

Transceiver supply pins reversed, `DIN` shorted to ground, no capacitor on `NRST`, `VCAP`
unverified, 47 Ω on the 485 pair. Moot with the sheet: the house 10 Ω, 100 nF on `NRST`, house
ferrites, the excitation switch below.

## `P-24-S` — the pump card at the battery

A station card, 24 V / 45,6 W on `LT3748` + `750311592`, its own 2,5 mm² cable, a command loop
from Pluvius over a second barrier (`ADuM5028`, `TLP172A` on `EN/UVLO`). It lost to the 48 V feed
(below); the 24 V returned on Palatine's switched `EXT` body, the request in the poll.

## Pluvius on a NodBus data body

Both Galvani bodies on Pluvius, ModBus over a NodBus board's two pairs. **It bound the unit to one
Palatine port**; with its own `THVD1450` it hangs on any arm.

## A 48 V feed of its own, and a pump buck on the feed side

A 48 V cell, a 2-core, a unit power board and a 48 V → 12 V head buck on Pluvius. ~26 W did not
carry a 22 W head plus the board through two conversions, no ≥ 65 V buck was found, and the switch
was at the wrong end: Palatine's `EXT` body has the switch, the ammeter and the `ID`.

## The head on a 300 V run — never built

Kept on, ~0,6 W standing for a motor that runs minutes a year; switched, a 300 V unit board at the
gauge. 300 V is reach, not a pump supply.

## 47 Ω on the SPI, 2,2 kΩ on the I²C, 10 kΩ on `NRST` — superseded

Values of this board alone. The house 33 Ω, 4,7 kΩ, and 100 nF with the internal pull-up.

## The timed drain, totals reassembled upstream, the weight encodings — superseded

The timed drain waited for a dry gap; the counted one runs in rain. Upstream totals from two
instants' readings gave way to the unit's, with the settled bit. `WEIGHT`: int16 at 0,01 g →
uint32 at 0,1 g → uint16 at 1 g, what the cell resolves. Builds A and B went with the head off the
board.

## The bought head with an encoder — superseded

A factory-encoder head, ~€130; the brushless 600 has the pulse on its own wire at no extra. The
couplers moved to the head's tail board, so the cable carries only Pluvius's logic. One pulse a
turn is 0,27 % of a litre, an order under the tube's set — finer buys nothing.

## The buffer as the monsoon mechanism — superseded

A 2000 mm season as a 40 L vessel on a 60 kg cell. Draining in rain, the limit is the head's,
1800 mm/h on the 600; the vessel is the dead-head reserve.

## "The head goes wherever the arm goes" — superseded

The 2-core sized for 50 m; the gauge stands beside the enclosure, cable and hose are metres.

## A drop-counting disdrometer — not the gauge

Two optical gates under a nozzle give drop sizes, but 0,1 mm of nozzle movement throws a drop clear
of a 1 mm gate — **it cannot stand on a pole**.

## The `KPHM400` — dropped

The small head with the five-wire motor. The 600 has the same motor and the flow the vessel wants.

## The hose always full — superseded

Kept full because an emptied hose counted as drained while still in the system. A signed count
leaves the total unmoved, so the brushless 600 reverses a few seconds to empty its tube each
cycle; a brushed head keeps it full. No drain below freezing.

## `6N139` couplers and `BC847` inverters on the head's tail — superseded

Open-collector photodarlingtons, a `BC847` on `RUN` and `DIR` so a dark coupler stopped the head,
the pulse coupler's `V_CC` at 3,3 V under its sheet's 4,5 V. Totem-pole couplers drive both levels,
the `TLP2745`'s buffer logic sets the failure direction, the `TLP2361` runs from 2,7 V.

## `TS5A23159` as the excitation switch — superseded

A dual analogue switch as a DPDT on one `ACX` line, the converter's two-wire mode; before it two
`74LVC1G3157`, whose select wanted 3,5 V at 5 V. The build is the converter's own four-wire bridge
of four MOSFETs on `ACX1` and `ACX2`, unmodified.

## The dead drain on the weight alone — superseded

Weight, revolutions and source-board current are read together: the weight ends the drain, the
other two name the cause — a split tube turns the head at low current with no water moving, which
**the weight alone catches but cannot name**.

## A fixed 5 g dead-drain threshold — superseded

`DEAD_G` in the calibration block, set to head and cell, 20 g default — never under the chain's
noise.

## A cycle run to `EMPTY` in one go — superseded

Minute runs, the weight read between; a stone in the vessel is one fault, not a second run.

## A Zener for the tail board's 5 V — rejected

A shunt across 24 V burns its milliamps all the time and dies at the ~45 V clamp. The
`TPS7A1650`, 3–60 V in, did both, and now the `TPS7A4101` (*below*).

## The gauge without its shell — a lapse of the documents

The rewrites reduced the outer shell to "a roof". The gauge was always two cylinders: the shell
carries the catch and the cell's base and keeps wind off the vessel — **a swaying vessel is noise
on every reading and side-load on the cell**.

## Antifreeze, a heated orifice and an open vessel — not taken

WMO (Vol. I, chapter 6, §6.3.1) winters accumulating gauges on self-mixing antifreeze (methanol and
propylene glycol under oil) or a heated orifice. The charge is tens of litres a year, a mixture the
drain must never pump, and a service visit the station is built not to need; the heater is power
and a part where it must stay simple. Below 0 °C the gauge accumulates and flags `FROZEN`; the snow
radar is the winter instrument. The open vessel lost on evaporation to a closed one, ~2 mm round
the tubes, oil optional.

## A 1000 mm reserve on the 30 kg cell — superseded by 900 mm

20 L of water and ~3 kg of tare are ~23 kg, 77 % of a 30 kg cell, past the 70 % ceiling the sizing
itself sets so that an overflow never reaches the mechanical stop. The 30 kg row now holds 18 L,
900 mm, 21 kg; a site that wants the full 1000 mm takes the next cell up.

## `RATE` alone as Palatine's block — superseded by `RATE · HOUR · STATUS`

The block was `RATE` alone, 4 B, while Palatine's `PWR EXT` rule read `STATUS` bit 1 `PUMP` in every
10 s poll — one document said the poll was one register, the other needed a second. **A request
that rides the poll must be in the poll**, and the house map makes the run contiguous: `RATE` at
`0x0000`, `HOUR` at `0x0001`, `STATUS` at `0x0002`, read as one block of 8 B, the shape Ceres's
already has. The archive gains the hour and the status word — `FROZEN`, the faults — for 4 B.

## The `TPS7A4701` at 5,0 V — superseded by an `LP2985-5.0` and the house π filter

A 1 A, 4 µVrms regulator fed ~21 mA, under a reading that is a ratio to its own output: the
converter's reference is the bridge's sensed excitation, so the regulator's value, drift and
noise divide out (SBAS824, 9.2), and the board's own text already said the front need not be a
precision purchase. JLCPCB on 10 Nov 2026: `TPS7A4701RGWR` `C90084` $11,56 a piece, $9,00 at the
top tier; `LP2985IM5X-5.0` `C5380652` $0,18 and $0,09 — the part NIC-Crazy_Ivan's modules took
for the same reason on 31 Oct 2026. The LDO's weak megahertz is the house π filter's, behind it,
for the price of one 2,2 µH and one 10 µF + 100 nF more.

## `BSS138` in the excitation bridge — superseded by the Basic `2N7002`

The N side's on-resistance is inside the Kelvin-sensed reference and cancels; the CJ `2N7002`
(JLCPCB Basic `C8545`) states 7 Ω at most at 5 V and 50 mA, 0,1 V at the bridge's 14 mA. No
`BSS138` is Basic; the `BSS84` is taken as `LBSS84LT1G`, Preferred (`C8492`), 10 Ω at 5 V.

## The `TPS7A1650` on the tail board — superseded by the `TPS7A4101`

The 60 V part stood at $4,92 with 38 in stock at JLCPCB (10 Nov 2026). The `TPS7A4101DGNR`,
`C111739`, 50 V in, $1,08 with ~7 800 in stock, takes the same ~10 mA in the same MSOP-8 PowerPAD
and keeps 4,6 V over the ~45 V clamp — the 2 V an LDO's input keeps over the highest voltage it
sees (the owner, 10 Nov 2026). It is adjustable: two Basic resistors set the 5 V.

## 4,7 µH on the 5,5 V buck — superseded by the 3,3 V's 10 µH

The branch runs in PFM at its ~20 mA with any inductor, the `LP2985` and the π filter behind it,
so it takes the 3,3 V's 10 µH: a ripple of 0,12–0,18 A at 2,08 MHz, under the part's ~12 µH
ceiling, and one inductor kind on the board, one Extended fee less (11 Nov 2026).

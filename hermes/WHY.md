★ N.I.C. ★

# Hermes — the graveyard

Rejected alternatives and superseded states, with the reason. Nothing returns from here and
nothing is deleted from it.

## The converter on the Mayak's LP UART — superseded

The card sat at 9600, forced by **the LP UART's ~115 kbaud ceiling**. Now the LP I²C: the card is
one register block, an I²C slave, and the LP I²C masters from the LP domain, so the survival loop
keeps its ear (`../mayak/WHY.md`).

## CAN as a fitted face — dropped, then fitted as one of four

**An MPPT or smart BMS with CAN has Modbus RTU on 485 too**, so CAN was dropped for 485 Modbus RTU,
plain UART second. It returned with the I²C out: the H523 has the pins and standby costs
microamps — four faces, an unused one asleep (`README.md`).

## 12 V terminals and a buck on the card — superseded

12 V on two terminals and a `TPS629203`. **Under 10 mA with every transceiver**, they bought
nothing over one polyfuse on the Mayak's 3,3 V, and the Galvani power body already carries the
link — `ID`, I²C, `ALERT`, 3,3 V, return. The card takes 3,3 V there, `ID` at 0,90.

## `ALERT` active low, open-drain — superseded

`ALERT` on PC8 pulled low on a threshold, released by reading `ALERT_CAUSE`. The power body says
**high = alarm**, 100 kΩ to ground on the host so an empty socket is quiet; PC8 is push-pull to it.

## The `SN65HVD230` on the CAN face — superseded by the `TCAN334`

`SN65HVD230`: standby on `RS` at ~0,4 mA, −4 to +16 V. `TCAN334`: **15 µA standby with bus
wake**, ±14 V and ±12 kV IEC 61000-4-2 on the bus, dominant time-outs against a hung H523. The
`TCAN332`, normal mode only, draws 3,5 mA recessive always. 55 mA dominant against ~17 mA raised
the pin budget from 50 to 100 mA and the polyfuse from 0,1 to 0,15 A.

## An isolated 485 face — not taken

`ISO1450` on a `SN6505B` island. The BMS and MPPT share the pack's negative with Hermes: **the
barrier would split a ground joined on the far side**, for three parts and ~0,1 W. The `THVD1450`
stays; bought parts must be common-negative.

## Bare faces and a 120 Ω CAN terminator — superseded by the house rule

Transceivers alone on the cables, 120 Ω on CAN. Every cabled bus now has transil, 2× 10 Ω,
transceiver, the terminator behind the resistors so the bus sees its impedance: 100 Ω on CAN for
120, as 80,6 Ω on 485 for 100 (`../core/HARDWARE.md`).

## Two vendors' maps in the document — superseded by the map the builder fills

An MPPT's Modbus registers and a smart BMS's frame commands, shipped as the card's maps and named
in `core/POWER.md` as the reference set. **The station designs what it builds; these are bought**:
every maker has a dozen units and maps, and both were reworked within a year, before a board
existed. A vendor protocol here also invites trust over its own sheet. What stands: three
arrangements, four buses, the block, and a field-source map the builder writes from the bought
units' documents at commissioning.

★ N.I.C. ★

# Babel — the graveyard

Rejected alternatives and superseded states, with the reason. Nothing returns from here and
nothing is deleted from it.

## One address for the board, sensors as channels — superseded

One ModBus address per board, each sensor a fixed sixteen-register block of channels (position 1's
primary at `0x10`, position 3's at `0x30`), computed by the master with no schema travelling.
**The master must know the population either way**, so it bought nothing and added a block to take
apart; one address per sensor makes a Babel position look like a bought box of the same quantity.
Also closed: a written plain-Modbus address per sensor, argued from four bits of NUMBER capping a
type at sixteen, then from *type 12 is Babel's identity only*. Only a bought unit is hand-written
now.

## The multiplexed block — never built

A bought part packing eight gases into one transfer is demultiplexed by the host; we do not build
one. **The host would carry a schema per such part**, and the station's table would stop being
*through this Bifrost, to this unit* plus sensor numbers. One address per sensor costs a few out of
hundreds.

## Babel's own 32-bit map — superseded by the house map

Primary as uint32 or float in `0x0000`–`0x0001`, secondary in `0x0002`–`0x0003`, `RAW` at
`0x000E`, `STATUS` at `0x000F` — a shape unlike Ceres's or Pluvius's. Replaced by the house map
(`../core/blocks/modbus.md`): **one register per value at the quantity's scale, the same positions
on every unit we build**. Sixteen bits at a scale an order under the instrument carry every
quantity; a bought float is converted.

## Both Galvani bodies, a 5 V rail, a switched sensor rail — the universal board not built

Babel was to cover every case: both Galvani bodies for its own isolated run, a 5 V rail, a
GPIO-switched sensor rail, switches for anything else. No MOD carries a data body: an arm ends at a bought sensor or a MOD with
its own 485, and beyond an arm stands a remote Palatine on a fed run (`../galvani/WHY.md`). **Most
sensors run on 3,3 V over SPI or I²C**; the rest served cases nobody had, and a board for every
case never gets finished. Replaced by the bare base, adapted by the builder on the free pins.

## "3× `TPD4E05U06`, one array per four lines" — superseded by the rule

The protection was written as a count. **The house rule replaced it**: every bus line leaving a
board to a part the builder connects carries 33 Ω at the processor and one ESD channel at the
connector, so the count follows the lines fitted (`../core/HARDWARE.md`).

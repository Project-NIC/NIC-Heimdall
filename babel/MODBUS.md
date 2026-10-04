★ N.I.C. ★

# Babel — the ModBus contract

> **Design-stage concept.** What the board is: `README.md`; the board: `HARDWARE.md`; the
> firmware: `FIRMWARE.md`. The bus itself — framing, the ModBus arm, addressing — is
> `../core/blocks/modbus.md`, which wins on any bus question.

## 1. Role

A Modbus RTU slave on a Palatine arm. The host polls; Babel never initiates a transfer.

## 2. Identity — no type of its own

**A sensor's address says what it measures, not what it is plugged into.** Each fitted position
answers as its own slave on the type of **its quantity** — a thermometer takes the thermometer
type whether it sits here or arrived as a bought box — at `TYPE«2 | NUMBER` like every house
module (`../core/PROTOCOL.md` §2).

| | |
|---|---|
| type | **none** — the type belongs to the quantity at each position. **A board with no position fitted answers once on type 4, bare**: the house block, `SENSORS` 0, `VALUE` 0 with VALID clear |
| addresses | **one per fitted position**, up to four |
| NUMBER | 0..3, unique across the station; **3** from the bench, written by the host's sweep at `0xFF05` |
| one board | **`0xFF01 IDENT`** — every slave a Babel presents reports the same house code, and the host knows the four are one box behind one connector and one feed position |

**The board declares itself at boot by what is populated.** A two-sensor board comes up as two
slaves, a four-sensor board as four; an absent position is not an address and never answers. A
board with nothing fitted is one slave on type 4, so it is on the bus with its identity before it
has a sensor. Nothing is written by hand and nothing is translated. The one-address-per-board map this
replaced is in `WHY.md`.

## 3. Registers — the house map on every slave

There is no Babel map. Each slave answers the house map (`../core/blocks/modbus.md`, *The house
map*), so the host reads a thermometer in one shape whether it is a bought box, a Ceres or a
position on this board.

| register | contents |
|---|---|
| `0x0000` `VALUE` | the reading, one register at the type's scale |
| `0x0001` `VALUE2` | the second value where the part gives a pair — a humidity beside a temperature; `0x8000` where it gives none |
| `0x0002` `STATUS` | bit 0 VALID · bit 1 SENSOR_FAULT |
| `0x0003` `RAW` | the uncorrected reading, bench diagnostics |
| `0x0004`… | the part's further values, one register each |
| `0x0010`… | the part's settings — an offset, a range, a conversion mode — persisted on write |
| `0xFF00`+ | the house block: `VERSION` · `IDENT` · `TAG` · `HEALTH` · `NUMBER` rw · `BOOT_COUNT` · `STATUS` · `RATE` rw · `SENSORS`. No time registers — the leaf bus carries no time |

**One register per value, sixteen bits at the type's scale** — 0,1 °C, 0,1 %, 0,1 hPa. A part
that delivers a float or a 32-bit word is converted here. The sensor's own protocol never
appears on the bus in any form; the host reads finished numbers.

**What a profile adds is the naming**: which bought part sits behind the position, what its
values mean, which type it takes — one table per supported sensor, written with that sensor's
profile.

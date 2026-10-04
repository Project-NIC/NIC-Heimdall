★ N.I.C. ★

# NIC HW Block — ModBus: the Modbus RTU leaf

> **Design-stage concept.** The figures are verified against the parts' sheets and the parts
> before a board is made.

**A sensor hangs on a ModBus arm, and Palatine carries them** — the bought units and the slow house
modules. Argus carries **NodBus mini** segments instead, not arms. The **Mayak carries no ModBus
arm** — nothing is polled for science on the head: its four HP UARTs are the MasterNOD trunks
(`../../mayak/HARDWARE.md`). **The BMS and the MPPT reach it through Hermes on its
LP I²C**, internal, never gated; Hermes's standard face is Modbus RTU
on 485, this block's protocol on a bought unit's terms, not a house arm: no house register block,
no ranging, no address rule.
The host MCU is the Modbus master; **the transceiver is not on it.** A ModBus leaf leaves its
host on a data connector exactly like a NodBus port does, but **the boards a ModBus leaf
can actually use are not the whole family** — see *What a bought unit plugs into* below. Inside the enclosure the leaf
is an **in-box cable** and there is no 485 at all (Kronos's time bus to the cards is M-LVDS, `../../kronos/HARDWARE.md`). The leaf bus is the **slow COTS
class** (below). **ModBus is the slow bus and has no fast form**: what is fast is on NodBus or on
mini, behind a card.

- **The link is `G-I-M-005`**, the Galvani family's one half-duplex board: one pair, one `ISO1450`
  behind the family's barrier and ladder, `DE` from the host turning the line frame by frame, no
  channel B and no clock (`../../galvani/README.md`).
- **ModBus carries no clock and no time of its own.** **A leaf value is stamped by its host when
  it is polled** — which is exactly right for what hangs here: nothing on this bus is fast
  enough for a microsecond to mean anything, and the poll instant is known to the host to better
  than the value's own averaging window.
- **The read interval is a SETTING, not a property of the bus, and this document fixes none.**
  Each unit's measurement time is its own — what it integrates over, how long it needs, how
  often it is worth asking — so the poll cadence is configured per unit at commissioning and can
  be changed after it. Nothing here should be read as a rate a builder has to accept.
  **There is no exception.** A house unit that needs the clock is not a MOD — it goes on
  **NodBus mini** behind Argus, 8 B of payload on the same framing and the same TDMA
  (`../PROTOCOL.md` §2). **What ModBus keeps, and what nothing else can do, is the fan-out:**
  sixteen units on one arm, one USART, because a polled bus multiplexes in time and a TDMA one
  cannot — a mini segment is one unit per slot and costs a whole port for a few of them.
- Standard RS-485 **Modbus RTU** (8N1) on the slow leaf class — **protocol compatibility is
  preserved**. *(A spur speaks NodBus framing, not Modbus — `nodbus.md`.)*
- **An arm is 50 m, and the cap is the 12 V, not the data.** 485 at 19 200 would run far longer; the
  arm's unregulated ~12 V from a `G-12-S` at 0,4 A would not — 50 m of 2,5 mm² loop is 0,7 Ω and
  0,28 V, 500 m is 7 Ω and 2,8 V, under the 10 V floor of half the sensors it feeds. Fifty metres
  keeps the arm a leaf beside the station; what stands further out is a fed run or a remote Argus on glass.
  **And the practical length is the sensors' own cables**: a bought sensor comes with its four
  wires on it, 0,35–0,5 mm², and is stripped at the station into the two boards — the pair onto
  `G-I-M-005`, the +/− onto `G-12-S`, four positions on each, so an arm is a star of up to four
  sensor cables and more chain at a sensor. A plot that wants more is reached by a remote
  Palatine on a fed run, never by longer arms.
- **Termination: NONE on a ModBus arm at the standard 19 200, at any length up to the 50 m cap**
  (*Two rates, one protocol*, below); only an arm run faster than that is terminated,
  on the Galvani board it leaves through. So no plug and no shunt on an ordinary arm. What the line carries instead is **2× 10 Ω series +
  SM712** at the transceiver — resistors that are the protection, and which an unterminated bus
  can afford where a terminated NodBus cannot.
- **Protection: SM712** at the block's transceiver, never on the
  host board. The 10 Ω above are **surge resistors, not matching** — one value on every board of
  ours; nothing on this bus is matched to anything.
- **No protection on the sealed COTS sensors.**

```
THE LEAF, AND WHO IS ALLOWED TO CARRY ONE

  NodBus spur ─▶ PALATINE ─┬─ leaf ─▶ COTS sensor — set to 19 200 and given its address in
                 (a NOD)   │          the learning session, never hunted at run time
                           ├─ leaf ─▶ house MOD — Pluvius · Sakura · Ceres, H523, the
                           │          0xFF00+ register block, address = TYPE«2 | NUMBER
                           └─ leaf ─▶ BABEL — one board, one address per sensor on it

  ARGUS carries no ModBus arm at all — its four segments are NodBus mini
  (../PROTOCOL.md §2), because the units on them need the clock.

  THE MAYAK CARRIES NO ModBus ARM: four HP UARTs are the MasterNOD trunks. The BMS and the MPPT sit behind Hermes on its LP I²C, internal,
  never gated; the Modbus RTU there is the bought unit's own map on Hermes's far side.

  inside the box ──▶ a crossed cable: plain UART, no 485, no termination, no ladder
  leaving the box ──▶ the leaf crosses two Galvani boards like everything else — G-I-M-005 and G-12-S on the short
                      copper run — and that is as far as a bought unit reaches. Anything
                      longer is a NodBus spur, never a longer leaf.
```

**Slow = the arm's isolated 12 V, minimal margins.** **A house MOD on an arm is fed like a bought sensor — the arm's isolated 12 V, a `G-12-S`'s at the
station or the unit board's 12 V terminal on a fed run, on four wires: `A`, `B`, 12 V, GND — and makes
its own rails on board.**
**Every MOD carries the basic set, and only the basic set:** on the pair its own `THVD1450`,
2× 10 Ω and an `SM712`; on the 12 V input **2× `5.0SMDJ18A`** and a buck that outlives their
clamp — **the `LMR43610`, 36 V in**, so a transil firing on the input does not take the rail with
it. No tube and no choke on a MOD: the barrier and the ladder stay on the arm's `G-12-S`, and a
MOD on a 48 V or 300 V unit board keeps the two transils anyway — they cost nothing and the
input is the same input. An all-digital MOD's input stage is therefore **that buck at 3,3 V** —
Babel is that board, and so are Ceres and Sakura, whose front takes the same 3,3 V behind
its own inductors (`../../ceres/HARDWARE.md`). A MOD with a clean analog rail adds a second buck and the LDOs it
needs — Pluvius: a second `LMR43610` to 5,5 V and a `TPS7A4701` at 5,0 V behind it for the converter's
analog side — two of the family's three LDOs, the same parts on every board that wants a 5 V or a
clean 3,3 V.
**The feed is the arm's isolated 12 V** — `G-12-S`'s at the station, the unit power board's
terminal on a fed run — and it follows the battery through the barrier's ratio (10–20 V on the
pin): a sensor is chosen for that window, and a COTS part that cannot run there is not the part —
there is no boost at a leaf. The hard voltages in the station are the feeds, 48 V or 300 V
(`../../galvani/README.md`).
The margins are cut deliberately:
this bus carries sleepy COTS-class sensors polled at leisure, and over-engineering it costs money at
every station for nothing.

## The register model — the options Modbus RTU offers

Modbus RTU is a question-answer protocol over four address spaces. Everything a device can say
or be told is a read or a write into one of them:

| Space | Width | Access | Use here |
|---|---|---|---|
| Coils | 1 bit | read/write | on/off outputs — unused on this bus |
| Discrete inputs | 1 bit | read | binary states — unused on this bus |
| Input registers | 16 bit | read | measured values (FC04 — plain reads; *Two rates, one protocol*) |
| Holding registers | 16 bit | read/write | configuration, commands, values (FC03 read · FC06 write one · FC16 write many) |

The protocol standardises only the reading and writing — **what lives at which register is the
vendor's own choice**, published in the device's register map. That cuts both ways:

- **A bought sensor is reconfigured in place.** Practically every Modbus sensor on the market
  exposes its **address** and **baud rate** (often the parity with them) as writable holding
  registers, and many add **correction offsets** for their measured values — worth writing where
  a reference is at hand. Its measured values, factory identity and status are read-only; a write
  there returns a Modbus **exception**, the slave's error answer, visible on the bus. And a
  bought sensor carries **no spare user registers** — nothing of ours can be stored in it, so
  everything the station knows about a sensor beyond its address (what it is, where it hangs, how
  it is corrected) lives in the station's table, never in the device.
- **Our own MODs are their own vendor.** The `0xFF00+` house block (boot count, ident, health,
  status) is exactly such a vendor map — ours, on the same numbers across every house MOD. And a
  writable register is also how a MOD takes a **command**: Pluvius's TARE and DRAIN are plain
  holding-register writes (`../../pluvius/MODBUS.md`).

Register numbers are per vendor, from each type's datasheet — which is why the provisioning code
takes them as parameters. Some vendors also guard a configuration write: a "save" register, a
power cycle before the new setting takes effect, or a config window after power-up. The bench
procedure built on all this is `../../palatine/HARDWARE.md`, *Commissioning the addresses*.

## The house map — one register map on every unit we build

**Every house MOD answers the same map, and every slave a Babel presents answers it too.** The
reading is always at `0x0000`, so the host reads a thermometer, a soil probe or a rain gauge in
one shape, and a type's profile says only the scale and the names.

| register | name | contents |
|---|---|---|
| `0x0000` | `VALUE` ro | **the reading** — one register, the finished, compensated value at the type's fixed scale |
| `0x0001` | `VALUE2` ro | **the second value where the quantity has one** — the humidity beside a T/RH pod's temperature, the soil temperature beside Ceres's moisture, the temperature at the plate beside Sakura's wetness, the last complete hour beside Pluvius's running total. A type with none reads `0x8000` there and its run may stop at `0x0000` |
| `0x0002` | `STATUS` ro | bit 0 VALID · bit 1 SENSOR_FAULT · bits 2–15 the type's own |
| `0x0003` | `RAW` ro | the uncorrected reading `VALUE` was made from — one register, bench and calibration diagnostics |
| `0x0004` and up | the type's own | state and settings, each register marked r or rw in the profile; a setting persists on write — Pluvius's `TARE` and `DRAIN`, Ceres's `INTERVAL` and curve |
| `0xFF00`+ | the house block | the engine's, identical on every unit — the table below |

**The house block, written out once** — the same numbers on every unit we build, NodBus and
ModBus alike (`../PROTOCOL.md` §9); a unit answers the ones its bus has and an exception, or
`ERROR` on NodBus, for the rest:

| register | name | bus | contents |
|---|---|---|---|
| `0xFF00` | `VERSION` ro | both | the firmware version |
| `0xFF01` | `IDENT` ro | both | the house code — the same on every slave one board presents |
| `0xFF02` | `TAG` ro | both | CRC-16-CCITT of the 96-bit UID |
| `0xFF03` | `slots` ro | NodBus | the NUMBERs and slots the unit takes, 1 to 5 |
| `0xFF04` | `HEALTH` ro | both | the counters — the unit's own list in its profile |
| `0xFF05` | `NUMBER` rw | ModBus | the slave's NUMBER, written by the sweep; on NodBus `ASSIGN_ADDR` does it |
| `0xFF06` | `BOOT_COUNT` ro | ModBus | boots since the cells were last written |
| `0xFF07` | `STATUS` ro | ModBus | the engine's state |
| `0xFF08` | `RATE` rw | ModBus | the arm's rate code, 9 600 or 19 200, persisted |
| `0xFF09` | `SENSORS` ro | both | the present-mask, one bit a part the profile names |
| `0xFF0A` | `VIN_WINDOW` rw | NodBus | the supply window, the header's SUPPLY flag outside it (`../../quake/FIRMWARE.md`) |
| `0xFF0B` | `REPORT_INTERVAL` rw | NodBus | seconds between `REPORT` frames, default 60; 0 disables (`../PROTOCOL.md` §5) |
| `0xFF10` | `ID` ro | NodBus | the bodies' `ID` codes read at boot |
| `0xFF11` | `ROUTE` rw | NodBus | the run's delay the card ranged, ticks of 2²⁷, persisted (`../PROTOCOL.md` §7) |
| `0xFF12` | `SKEW` ro | NodBus | the board's own delays, int8 in the unit's ticks, an image constant (`../PROTOCOL.md` §7) |
| `0xFF13` | `DELAY` rw | NodBus | the frames the unit holds before it sends, written at floor-up, never below 8 (`../PROTOCOL.md` §7) |

**And the house positions in the unit's own range on a NodBus unit** — taken where the unit has
the thing, never reused for anything else: `0x0002 STATUS` · `0x0020 CALIBRATE` w · `0x0021
RANGE` w, the ranging turnaround · `0x0023 SELFTEST` w · `0x0038 REPORT`, `GET` answered with the
`REPORT` frame · `0x003A HEALTH`, `GET` answered with the `HEALTH` frame. A host's ports frame is
`0x0039 PORTS`, where a unit without ports may put a register of its own.

**One register per published value, sixteen bits at the type's scale** — 0,1 °C, 0,1 %, 0,1 mm,
0,1 hPa; the profile fixes it, a signed value reads `0x8000` for *absent*, and every quantity on
this bus fits sixteen bits at a scale an order under its instrument. The one 32-bit pair in the
house is Pluvius's `DRAINED`, a free-running accumulator in its own area, never a reading. **A
block is a contiguous run of these** (`../PROTOCOL.md` §5): the profile says which run a type
ships — `VALUE` alone, or `VALUE` through `RAW` — and the register numbers never ride a frame.

**A unit carries one quantity and, where a bought unit of its class carries a pair, the partner
on `0x0001` — never more.** A T/RH pod is one slave with two values and the host has to read
that shape anyway; so Ceres reports the soil temperature at its depth beside the moisture and
Sakura the temperature at its plate beside the wetness, one address each. A bought part that
packs eight gases into one transfer is demultiplexed by the host, and we do not build one.

## Two rates, one protocol — and no burst

**Everything on a ModBus is primitive RTU, and the house adds nothing to it** — no ranging,
no user-defined function code. There is no burst mode and no pipelining anywhere on the bus — the house path is **higher speed and more
ports** (Palatine's arms), not a cleverer protocol.

**No freeze latch.** A unit that needs its value at one instant goes on NodBus mini, which
carries the clock; ModBus manufactures no simultaneity (`../../bifrost/argus/WHY.md`).

**An arm carries about sixteen units, and the cap is electrical.** Three things set it at once:
**reach**, an unterminated RTU run stopping short long before the address space does; **the
bus's own complexity**, every stub loading the pair and lengthening the turnaround; and **the
rate**, a poll round being serial, so sixteen units is already a round measured in the hundreds
of milliseconds. **That is why Palatine carries four arms** (`../../palatine/HARDWARE.md`) — a
wider plot is a second Palatine on a fed run — and
they divide the electricals, never the addresses: a NUMBER is unique across the station, so the
same type on two arms is two NUMBERs (`../PROTOCOL.md` §2).

**A bought sensor is set once, in the learning mode — never hunted at run time.** The order of
preference: ① **program every sensor to one rate**, 19 200 — one arm, one rate, one short sweep; ② where baud-locked sensors cannot move, **split
the arms by rate** (e.g. two arms at 9 600, two at 19 200 — a rate is a UART property, so the
split costs nothing); ③ **autodetect exists only inside the provisioning session** — the
two-rate sweep (9 600 · 19 200) runs while a sensor is being learned, and never
again. Normal running never switches modes: the roster is verified, not extended
(`../PROTOCOL.md` §7), and an arm's rate is a table entry from commissioning. *(Runtime
autodetect as the standing default was weighed and dropped — `../WHY.md`.)*

**The standard build: everything at 19 200, and the arm's two rates are 9 600 and 19 200 — nothing above.** 19 200 is the highest rate practically every bought
sensor supports and the highest where the no-termination doctrine
holds at any arm length with sixteen units on it (38 400 was weighed and dropped — `../WHY.md`) — so one rate serves every arm up to its 50 m, with
no per-run exceptions. The learning session sets every re-ratable sensor to it and assigns its
**address from the bought range 0x01..0x0F**; house MODs are given their packed `TYPE«2|NUM`
by Palatine's sweep (`../PROTOCOL.md` §2). A baud-locked sensor that cannot move is the one exception: it gets an arm at its
own rate, and the table records it. Nothing else ever differs between arms.

**The learning mode — provisioning through the tunnel, remote by construction.** Plug the
factory-fresh sensor **alone** on an arm (factory defaults collide, so one at a time), and the
handset drives a template: the Mayak puts the arm into a **provisioning session** — `SET` Palatine's `0x0015 PROV`;
the arm's polling pauses, its UART takes the commanded rate, and the arm becomes tunnel
passthrough only — then tunnels the type's FC06 writes (address, rate, parity), verifies the
readback, and closes the session; the arm returns to its roster and its rate. **Writability is shown, never
assumed** — the app attempts the write and shows the sensor's answer (a guarded or read-only
register returns the Modbus exception), and the operator decides; nothing here is
over-automated. **A sensor that must be reached differently** — a save register, a power
cycle, a config window — **gets an exception entry in the roster**, so the quirk is a table
row, not tribal knowledge. One button per sensor, written to the mirror when the sensor
confirms, and the same flow serves the bench and the remote case alike —
nothing new rides the wire, the tunnel already carries any RTU package. **At every boot the
persisted tables are verified, not rebuilt**: everything answers as rostered → the station
runs; a mismatch is a re-announce, never a silent rejoin (`../PROTOCOL.md` §7).

**A house MOD is a slave like a bought sensor: it runs at its arm's rate, set once in the learning
mode and persisted on the board, and nothing on this bus negotiates.** There is no boot rate
above the arm's — ModBus is classic RTU end to end; the 38 400 sync-and-detect start is
NodBus's and never sounds on an arm.

An integrator who wants a FIFO profile
can build one on plain FC04 — a standard read of a sample ring with a sequence register is
legal RTU and needs nothing from us — but no house firmware carries it.

**Transceiver:** the house **ISO1450**, on the Galvani board — ModBus is half duplex, so the
half-duplex part of the family is the one that fits, and 19 200 is nothing to its 50 Mb/s.

## What a bought unit plugs into

**A bought ModBus unit is four wires and a ≤30 V supply.** `A`, `B` and its power — that is the
whole cable, and the power it expects is a 12/24 V-class rail. **`G-I-M-005` and `G-12-S` together give it both**: an
isolated 485 line side and an isolated feed that follows the battery.

**Every other class fails it on the supply, not on the data.** a NodBus spur carries 48 V or 300 V on
its own cable, glass carries no copper at all, and a unit power board hands the unit 12 V on
its two-pole terminal — a run that is already fed answers a bought sensor with that
terminal. Inside the enclosure the leaf takes an
**in-box cable** and there is no 485 to have a class about.

## Long haul — not this bus

**ModBus ends at the local leaf.** A bought unit cannot attach to an optical converter; where
classic sensors must stand far away, a **Palatine** goes there and carries them on its arms.
Anything long is a **NodBus optical spur**, full duplex by construction.

**There is no "ModBus over glass".** A bought slave never runs over an optical converter here;
the case that once seemed to prove otherwise is withdrawn with its premise (`../WHY.md`).

**The transports, then — and note that the protocol does not pick the board.** In-box logic
(UART and I²C on an in-box cable, no 485 anywhere) · NodBus copper (≤ 500 m, shared clock / TDMA,
on `G-I-N-025`; **full duplex, because the feed travels in a 2-core cable of its own and leaves
all four pairs to data**, `../../galvani/README.md`) · ModBus copper leaf (RTU, local,
stop-and-wait, on `G-I-M-005`) · **glass — NodBus full duplex**, point to point. NodBus and ModBus are two
protocols over one family of boards; which board is fitted is decided by where the cable goes,
never by what speaks over it.

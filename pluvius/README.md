<p align="center">
  <img src="NIC-Pluvius.svg" width="200"/>
</p>

★ N.I.C. ★

# Pluvius — the weighing rain gauge

**English** · [Čeština](README.cs.md) · [Русский](README.ru.md)

> **Design-stage concept — nothing built.** The figures are verified against the parts' sheets
> and the parts before a board is made.

**Pluvius weighs precipitation: ModBus type 5, a MOD on a Palatine arm.** A 200 cm² catch pours
into a vessel that **hangs from a load cell**; the cell is read ratiometrically by a weigh-scale
converter and the unit answers **in grams** — 1 mm of rain is 20 g. A counted peristaltic head
drains the vessel and every drain hands back a fresh zero, so the reading is an increment against
a zero refreshed every cycle and the cell's absolute accuracy never enters it. **Snow is not
weighed** — the radar on Palatine's arm measures it; hail falls in, melts and is weighed.

**Weight is the one measure every phase of precipitation arrives in.** Ice weighs what its water
weighs, so the gauge measures through a frost where a tipping bucket stops at 0 °C; drizzle, hail
and dew land as mass; evaporation shows as a loss, visible and accounted for; and nothing moves
in the measuring path. The ⌀160 mm catch clears WMO's 150 mm minimum, below which large single
drops dominate the sampling.

| | |
|---|---|
| class | **house MOD** — a Modbus RTU slave on a Palatine arm, **ModBus type 5**, address `5«2 \| NUMBER`, 0x14 for unit 0; no clock, the host stamps the reading when it polls |
| the catch | **200 cm²**, ⌀160 mm — 1 mm = 20 g |
| the cell | **30 kg, single point, the vessel suspended**, class `C5` — the reference build; the sizing and the smaller rows are `HARDWARE.md`, *Sizing* |
| the converter | `ADS1235`, 24-bit, ratiometric, PGA 64, AC excitation |
| the drain | a **24 V peristaltic head on the switched 24 V of Palatine's `PWR EXT` body**, switched by Palatine on this unit's `PUMP` bit — **the Kamoer `KPHM600` with the five-wire motor is the build**: 12 W, a pulse output, one a revolution, and run and direction wires, all three through optocouplers on a small board in the head's box; the `KPHM900`, two wires and no board, is the option (`HARDWARE.md`, *The head*) |
| feed | the arm's isolated 12 V on the four-wire cable, like every sensor on the arm; the board makes its own 3,3 V and the converter's 5,0 V |
| MCU | `STM32H523VE`, LQFP100 |
| what it publishes | totals — `RATE`, the last complete `HOUR` and `DAY` with the one before each, every one with its settled bit; weight, drained volume and revolutions are state on the tunnel (`MODBUS.md`) |

## How it hangs on the station

- **On a Palatine arm like a bought sensor** — the arm's pair and its isolated 12 V on four
  wires, its own `THVD1450` and the basic set on the board. Palatine polls `RATE · HOUR · STATUS` every 10 s
  and ships the run as one block, the `PUMP` request in its `STATUS`; the rest is read on demand through the tunnel.
- **The head is a load on the station, not a part of this board.** A peristaltic head pays its
  occlusion friction before it delivers anything and the arm's 0,4 A cannot reach it, so it hangs
  on the switched 24 V at the station on a 2-core of its own. A slave cannot speak first, so the
  request rides in the poll: this unit sets `STATUS` bit 1 `PUMP`, Palatine raises the board's
  `ENABLE` and writes `HEAD` back; the unit runs the head, watches the weight fall, stops at the
  base level and takes the zero. `ENABLE` is pulled down at both ends, so a reset, a cut ribbon or
  a dark port leaves the head off (`HARDWARE.md`, *The switch*).
- **The volume is counted and the dead drain is caught by the weight**; the count and the source
  board's ammeter say what failed (`HARDWARE.md`, *The pickup*, *Detecting a dead drain*).
  Accumulation is `weight + counted volume`, which lets the drain run while it rains.
- **`DRAIN` is a written register**, reachable from the server through the tunnel, so a station
  that knows a front is coming empties its vessel before it; a counted drain costs nothing.

## Where it stands

**Beside Palatine's enclosure, at the mast**: the head's 24 V leaves the enclosure on a 2-core and
the hose runs from the vessel to the head, both metres long. At a remote plot the gauge stands
beside the remote Palatine the same way, on its `PWR EXT` body. The catch stands at 1 m; a
deep-snow or mountain site raises the whole unit on a pillar above the maximum snow depth. The
vessel, the shell, the funnel's tube and the hose are `HARDWARE.md`, *The vessel, the shell and the
tubes*.

## The block

```
   a Palatine arm ── A · B · 12 V · GND ──▶ ┌────────────────────────────────────────┐
                                            │ PLUVIUS — H523 · THVD1450, MOD type 5  │── the load cell ── ADS1235 ── grams
                                            └────────────────────────────────────────┘
   Palatine's EXT body ══ switched 24 V, its own 2-core ══▶ the head's tail board ── the head — switched by ENABLE on this unit's PUMP status bit
   PLUVIUS ── 3,3 V · GND · PULSE · RUN · DIR, a thin cable ──▶ the tail board's three optocouplers ── yellow · white · green — the pulse in, run and direction out
```

## Files

| file | contents |
|---|---|
| [`HARDWARE.md`](HARDWARE.md) | the board, the cell and its sizing, the vessel and the shell, the head and its tail board, the pickup, the dead-drain rule, the switch, every pin, the parts |
| [`FIRMWARE.md`](FIRMWARE.md) | the firmware, described — the weight, the drain state machine, the tare, the periods and the totals, the registers, what is tested |
| [`MODBUS.md`](MODBUS.md) | the ModBus contract |
| [`WHY.md`](WHY.md) | the graveyard |

## Licence

Hardware: CERN-OHL-S v2 (`../LICENSE-HW`) · Software: MIT (`../LICENSE`) — Copyright (c) 2026 NIC — Native Intellect Community

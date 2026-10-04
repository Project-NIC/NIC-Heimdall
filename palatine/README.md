<p align="center">
  <img src="NIC-Palatine.svg" width="200"/>
</p>

★ N.I.C. ★

# Palatine — the meteorological base

**English** · [Čeština](README.cs.md) · [Русский](README.ru.md)

> **Design-stage concept — nothing built.** The figures are verified against the parts' sheets and
> the parts before a board is made.

**Palatine is the station's ModBus host.** One `STM32H523` that is the master of four ModBus RTU
arms and a NodBus unit toward its card: it polls the bought sensors and the house MODs its roster
names, each at its own interval and spread over the hour so no second is crowded, ships every reply as a self-delimiting block in its
32 B payload, and relays the head's tunnel packages onto the arms verbatim. **In mode A, the base, it computes nothing
on a value** — no mean, no gust, no unit conversion — and holds no policy beyond the roster the head
wrote into it; **in mode B it is the station's weather logger**, computing the WMO statistics and
shipping a row a minute, ten minutes, hour and day (`WMO.md`). **The weather is Palatine's; what the air is made of is
Chinook's** (`chinook/`), whose bought units hang on these same arms.

| | |
|---|---|
| class | **NOD** — NodBus **type 6**, one slot, address `0x6n`; behind a Bifrost spur |
| MCU | `STM32H523VE`, LQFP100, the house H523 |
| arms | **four ModBus RTU arms, one USART each** (USART2 · USART6 · UART4 · UART5), identical; **two populated as standard, four the ceiling**. Each leaves the box on a communication board and feeds its sensors from a power board, 50 m, about sixteen units an arm. A plot wider than four arms is a second Palatine, never a fifth arm |
| `PWR EXT` | **one power body on its own**, not an arm: a source power board switched by `EN_X` for Pluvius's pump — its `INA238` the load's ammeter (`FIRMWARE.md` §6) |
| power | 12 V on its two terminals — the battery wire in the box, the unit power board's terminals on a fed run — one `LMR43610` to 3,3 V for the MCU and the communication boards; **no sensor rail on this board**, a sensor takes the arm's isolated 12 V |
| load | ~7 W with today's sensors, **~21 W** with four arms on their limit; the run's trip point is measured at commissioning, start-up peak and running load with margin (`HARDWARE.md`, *The budget*) |
| payload | 32 B of **self-delimiting blocks**, `[address][length][data]`, one per ModBus transaction, whole blocks only; decoded by the module's own address and its profile where the archive is read (`../core/PROTOCOL.md` §5) |
| unasked | `PORTS` once a minute — its six sockets, the up port first, then the four arms and `PWR EXT`; the header's flags every frame; `ACK` |
| firmware | four ModBus masters and one NodBus unit: the roster, the sweep of the house MODs, the block ring, the tunnel, the `PWR EXT` rule (`FIRMWARE.md`) |

## What hangs on the arms

**The house MODs** — our own boards, one Modbus slave each, on the arm's 12 V:

- **Pluvius** (`../pluvius/`) — the weighing rain gauge: a load cell under a shielded vessel, answers in grams, owns its drain; its head's 24 V comes from `PWR EXT`
- **Ceres** (`../ceres/`) — soil moisture and soil temperature at one depth, one unit per depth; at −10 and −50 cm; a farm's profile −10 · −20 · −50 · −100 cm
- **Sakura** (`../ceres/sakura/`) — leaf wetness
- **Babel** (`../babel/`) — a sensor not sold as Modbus, converted at the sensor; one slave per fitted position
- **Chinook's air units** (`chinook/`) — the gas and particle set, bought, on an arm like the rest

**The bought RS-485 sensors** — `SENSORS.md` gives the universal station and the farmers' build, a recommended type for each; what is not sold as RS-485 goes through Babel:

- air temperature and humidity at 2 m in a radiation shield, and the ground temperature at 5 cm (`SITING.md`)
- wind, speed and direction at 2 m — mechanical or ultrasonic by climate and budget
- pyranometer, 0–2000 W/m²
- UV — the index, UVA/UVB where the unit gives them
- barometer — absolute, 300–1100 hPa, ≤ ±1 hPa, to −40 °C; or one T/RH/P unit at 2 m
- snow depth — an 80 GHz radar level sensor, non-contact, ±1 mm

**One address is not one sensor**: a unit may carry several quantities under one address, and
the block carries whatever registers it answered. **Rain is weighed, snow is measured by the
radar**; hail falls into the gauge, melts and is weighed.

## Where it stands

**In the head's enclosure by default** — it taps the 12 V wire on its own terminals and its arms
leave the box on Galvani boards. A large station stands a second Palatine at a sensor plot or a
second mast tens of metres out: the same board behind Galvani boards. A remote box buried for its
thermal mass is the builder's siting.

Every arm is one bus behind one transceiver; the arms exist to keep runs apart — a remote
instrument alone on its arm, a handful of thermometers on the next — not to multiply devices.
The parts are rated to −40 °C and every bought probe is bought to −40 °C; an arm whose CRC-miss
rate passes the threshold is switched off and re-fed after a cool-off (`FIRMWARE.md` §9).

## The block

```
   a BIFROST spur ──40 B NodBus, type 6──▶ ┌────────────────────────────────┐ ──arm 1..4, ModBus RTU──▶ bought sensors · Pluvius · Ceres · Sakura · Babel · Chinook's units
                                           │ PALATINE — H523 · LMR43610     │ ──EXT, one power body, EN_X──▶ 24 V, switched, to Pluvius's head
                                           │ polls the roster, packs blocks │
                                           │ into one 32 B record a frame   │
                                           └────────────────────────────────┘
```

## The tree

```
palatine/                     Palatine — the meteo base and the ModBus master  NodBus 6
└── chinook/                  Chinook — air quality: bought units on the arms, not a board
```

## Files

| file | contents |
|---|---|
| [`HARDWARE.md`](HARDWARE.md) | the board — the H523 and every pin, the arms, the bodies, the rails, the budget, the Modbus address map, commissioning the addresses |
| [`CONSTRUCTION.md`](CONSTRUCTION.md) | the soil column — the patrona — and the snow radar's shroud |
| [`FIRMWARE.md`](FIRMWARE.md) | the firmware, described — the arms, the roster and the sweep, the poll, the block ring and the tunnel, the `PWR EXT` rule, the registers, what is tested |
| [`SENSORS.md`](SENSORS.md) | what hangs on the arms — the two builds, and what a bought unit must meet, quantity by quantity |
| [`SITING.md`](SITING.md) | the radiation shield and the exposure of each instrument |
| [`WMO.md`](WMO.md) | mode B — the WMO tables, their columns, how the statistics are made, the registers it adds |
| [`CLIMATE.md`](CLIMATE.md) | describing a finished period against the normal — the WMO classes and the boundary tables |
| [`chinook/`](chinook/) | Chinook — what the air is made of: the bought air units a site fits on these arms, what to buy and what each must meet |
| [`WHY.md`](WHY.md) | the graveyard |

## Licence

Hardware: CERN-OHL-S v2 (`../LICENSE-HW`) · Software: MIT (`../LICENSE`) — Copyright (c) 2026 NIC — Native Intellect Community

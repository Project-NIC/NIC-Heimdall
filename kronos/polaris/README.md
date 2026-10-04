<p align="center">
  <img src="NIC-Polaris.svg" width="200"/>
</p>

★ N.I.C. ★

# Polaris — Kronos's GNSS front, bought

**English** · [Čeština](README.cs.md) · [Русский](README.ru.md)

> **Design-stage concept — nothing built.** The figures are verified against the parts' sheets
> and the parts before a carrier is made.

**A bought receiver module, a bought active antenna, and a carrier the size of the module that
puts them on Kronos's data connector. No processor, no bus, no address.** Polaris is what a
station fits when it has no Sputnik: a time-only GNSS receiver on Kronos's `TIME IN 1` socket,
handing it a lean NMEA stream and a PPS edge. Kronos disciplines and distributes; this is only its
source.

```
   ACGPSA, active antenna, mast-top, on an insulating bracket
        │ coax ≤ 4 m, the station's two DC-pass arresters on it
        ▼ SMA
   ┌────────────────────────────────────┐
   │ POLARIS — the carrier              │
   │   the bought module: NEO-M8N,      │      one Galvani data connector, 12 pins
   │   its bias tee, SMA                │─────────────────────────────────────────▶ KRONOS
   │   ID = GNSS, 33 Ω on the PPS       │      CLK/PPS · GND · TXD · RXD · ID · 3,3 V
   └────────────────────────────────────┘
```

**The receiver never leaves the enclosure** (`../../core/blocks/gps-pps.md`). Only the coax is long,
and RF does not care. So Polaris has no barrier to cross, no feed to make and nothing to range:
it is a carrier, not a Galvani board and not a unit.

**One type, `M8N`, typed into Kronos and never detected.** Kronos checks it at every boot —
`MON-VER` — and configures it to one profile, so a module out of the box and one somebody
reconfigured come up alike. **`UM980` is Sputnik's part and does not appear here** — a station with a
Sputnik has its time source already.

**Its delay is two typed numbers and no measurement.** `CAB` is the coax, metres × ~5 ns/m; `INT`
is the antenna and the module as one constant of the type. Both are subtracted on Kronos, once.

| file | what is in it |
|---|---|
| [`HARDWARE.md`](HARDWARE.md) | what is bought and what it must have, the carrier, the connector pin for pin, the antenna and its mounting, the delay terms |
| [`WHY.md`](WHY.md) | the graveyard — the card of our own with its bias tee, the receiver at the mast top, a code of its own, the four types |

## Licence

Hardware: CERN-OHL-S v2 (`../../LICENSE-HW`) · Software: MIT (`../../LICENSE`) — Copyright (c) 2026 NIC — Native Intellect Community

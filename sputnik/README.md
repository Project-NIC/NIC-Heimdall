<p align="center">
  <img src="NIC-Sputnik.svg" width="200"/>
</p>

★ N.I.C. ★

# Sputnik — the GNSS ionosphere front, and the station's time

> **Design-stage concept — nothing built.** The figures are verified against the parts' sheets
> and the parts before a board is made.

**Sputnik reads the ionosphere from GNSS: NodBus type 8, a NOD.** One multi-constellation,
multi-frequency Unicore **UM980** receiver and one STM32H523 on one board. It streams the raw
observables of every tracked satellite on three frequencies — pseudorange, carrier phase, SNR — at
5 Hz, from which the **total electron content** of every path overhead is computed, and with
Palatine's surface met the **precipitable water vapour**.

**The same receiver is the station's GNSS time source.** Its PPS and a lean NMEA stream go to
Kronos on the board's second socket; on a station with a Sputnik nothing else is fitted, and a
station without one fits Polaris (`../kronos/polaris/`). Kronos stays the authority that disciplines and
distributes — the UM980 is only its source.

## How it hangs on the station

- **In the enclosure, always** — the antenna is sited for the sky on the station's own mast and only
  the coax is long (`../core/blocks/gps-pps.md`, *The receiver lives in the enclosure*). Both
  sockets then take a crossed in-box cable: the NodBus socket to a card's port, the time port to
  Kronos's receiver socket. Both are also built to take a Galvani run each, the time run reversed —
  PPS inward at Kronos — and Kronos ranges it.
- **Up: NodBus, five NUMBERs, five slots** — one queue the H523 fills with 32 B chunks of the 200 ms
  epochs; the master reassembles them by frame index and slot order, and each record's PRN says its
  constellation.
- **Sideways: the time port to Kronos** — the receiver's PPS on channel B straight off its pin, the
  `RMC`/`GGA` stream relayed on channel A, served while Kronos's heartbeat answers. **`END` ends the
  measuring side and never the relay**: the station's time source goes only with the lockdown.

```
   the antenna ──coax──▶ ┌────────────────────────────┐ ── NodBus, type 8, five NUMBERs ──▶ BIFROST
                         │ SPUTNIK — H523 · UM980     │
                         │ raw observables, 5 Hz      │ ── PPS + NMEA, the time port ──▶ KRONOS
                         └────────────────────────────┘
```

## Files

| file | contents |
|---|---|
| [`HARDWARE.md`](HARDWARE.md) | the board — the H523's pins, the two sockets, the UM980, the antenna and its coax, the rail |
| [`BUS.md`](BUS.md) | the two tiers, the epoch, the Tier B record and the geometry record and the frequency backbone, the slot pool, rate and bandwidth, what is archived |
| [`PROCESSING.md`](PROCESSING.md) | TEC and PWV, and where they are computed — the station, a district box or the server, one observable |
| [`FIRMWARE.md`](FIRMWARE.md) | the firmware, described — the receiver's configuration, the epoch and the chunk stream, the five slots, the relay to Kronos, the registers, what is tested |
| [`WHY.md`](WHY.md) | the graveyard — rejected alternatives and superseded states |

## Licence

Hardware: CERN-OHL-S v2 (`../LICENSE-HW`) · Software: MIT (`../LICENSE`) — Copyright (c) 2026 NIC — Native Intellect Community

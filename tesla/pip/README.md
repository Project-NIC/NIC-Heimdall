<p align="center">
  <img src="NIC-Pip.svg" width="200"/>
</p>

★ N.I.C. ★

# Pip — the longwave carriers: the SID channel, and time for the station

**English** · [Čeština](README.cs.md) · [Русский](README.ru.md)

> **Design-stage concept — nothing built.** The figures are verified against the parts' sheets
> and the parts before a board is made.

**Pip is Tesla's board under its own image: NodBus type 12, a NOD.** It does two things with one
signal path:

- **the SID channel** — it tracks the VLF and LF carriers the site hears, the time-code stations,
  the navigation transmitters and eLoran, up to eight, and ships each one's level once a second as
  a carrier record: the D-region rung of the station's ionosphere ladder (Marconi carries the
  F-region, Sputnik the TEC);
- **time** — on the same carriers it decodes the date and the second and measures the station's
  grid against their caesium-derived phase, and on its second socket, where a Kronos is on the
  other end, it presents itself as a GNSS receiver: **PPS on the wire, NMEA on the UART**.

A carrier whose level is a science product is the same carrier whose phase is the time.

**Held back by coverage, finished to a description.** The transmitters reach the world that is
already instrumented; as a time source Pip is useless where no carrier is heard, recovers a rate but
not a date without a time-code station, and a severe solar event takes GNSS and longwave together.
As the SID channel it is useful wherever a carrier is heard, which is most land. `FIRMWARE.md` says
what the image does; the image is written after the board.

Named for **the pips** — the six beeps of the BBC Greenwich Time Signal, on air since 1924.

## How it hangs on the station

**The board is Tesla's, identical to the last part** — the rods, the chain, the converter, the
processor, the rails and both data bodies are populated on every board, and what a board is, is the
image in it. There is no Pip hardware document: `../HARDWARE.md` is the board, `TIME OUT` included.

- **Up: `NB IN`, NodBus type 12, one slot** — behind a Bifrost like any unit, the unit end of its
  run, fed through a power board; a measuring unit never stands in the box. Pip #1 is `0xC1`. The
  clock is the spur's, and Pip has no oscillator of its own to learn.
- **Sideways: `TIME OUT`, to Kronos's second RX/TX + PPS socket** — through a second communication
  board on its own run, NMEA on channel A and the PPS on channel B, driven outward at Pip and
  received at Kronos; it takes its 3,3 V from the board's interface rail as the first does. Pip is a
  time source there, and Kronos ranks it against GNSS and ranges the run.
- **`TIME OUT` serves itself or switches itself off.** Nothing plugged: SID-only. A communication
  board plugged: the socket waits for Kronos's heartbeat, `$PNIC,HELLO` every second, and is served
  from the next second; 30 s without one and it is switched off again. Nothing on `TIME OUT` can
  hold the NodBus side up.
- **What the head does for it:** writes the station's position (`POSITION`) — from GNSS, or typed
  once; 300 m is 1 µs — and Kronos's quality byte (`QUALITY`), so Pip knows whether the grid it
  measures against is GNSS-true. It learns the paths while it is, and steers the second when it is
  not.

```
   3 ferrite rods ──▶ [ Tesla's chain ] ──▶ ADS127L14 ──▶ H7A3, Pip's image
                                                          │
                                                          ├──▶ NB IN — NodBus, type 12: carrier records to the Bifrost
                                                          │
                                                          └──▶ TIME OUT — PPS + NMEA to KRONOS, served while its heartbeat answers
```

## Files

| file | contents |
|---|---|
| [`CARRIERS.md`](CARRIERS.md) | the two services on a carrier, the path, the transmitters and the band, eLoran, the vote and the learning, the impulsive floor and the gate, the arc's comb |
| [`FIRMWARE.md`](FIRMWARE.md) | the firmware, described — the carriers, the SID records, the time, `TIME OUT`, what the image leaves idle |
| [`WHY.md`](WHY.md) | Pip's own graveyard; the board's is `../WHY.md` |

## Licence

Hardware: CERN-OHL-S v2 (`../../LICENSE-HW`) · Software: MIT (`../../LICENSE`) — Copyright (c) 2026 NIC — Native Intellect Community

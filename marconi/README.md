<p align="center">
  <img src="NIC-Marconi.svg" width="200"/>
</p>

★ N.I.C. ★

# Marconi — the HF ionosphere monitor, 0,5 to 16 MHz

**English** · [Čeština](README.cs.md) · [Русский](README.ru.md)

> **Design-stage concept — nothing built.** The figures are verified against the parts' sheets
> and the parts before a board is made.

**Marconi reads the F-region: NodBus type 4, a NOD.** A one-metre air-core loop, a transimpedance
front end and a 16-bit converter sampling the whole 0,5–16 MHz band at once, read by the
STM32H7A3 that transforms it on the board.

**What it measures:**

- **the level of fixed HF transmitters over time** — time stations, channel markers, NAVTEX,
  beacons — found by a background sweep and read every 10–30 s, each as one log level; the
  markers' day/night pairs give absorption against frequency on one path, and the band's daily
  opening and closing is itself the measurement;
- **the passive ionogram** — the chirp sounders caught as they sweep the band, scaled on the node
  to `foF2`, `h'F` and `MUF(3000)`.

| unit | layer | how |
|---|---|---|
| **Pip** | **D-region** | VLF/LF carrier levels — sudden ionospheric disturbances |
| **Marconi** | **F-region** | HF carrier levels, 0,5–16 MHz — MUF / `foF2`, the passive ionogram |
| **Sputnik** | the integrated column | GNSS dual-frequency TEC |

Named for the 1901 transatlantic transmission, which worked because of the layer this board
measures — the Kennelly–Heaviside layer was proposed the year after, to explain it.

## How it hangs on the station

**Marconi is alone on its own Bifrost spur — point-to-point, full duplex, the unit end of the
run.** A power board for its feed and a communication board for its medium plug into its sockets;
the run changes nothing on its own board. The clock is Kronos's, on the spur's clock pair, and the
board locks its core, the converter's encode and every sample to it. **One slot**: a handful of
carriers at 4 B each and three ionogram parameters. 134 MB/s stay inside the box; a few bytes a
second go on the wire.

**The loop stands on the mast with its plane computed from the transmitters' coordinates**, and
the board sits in a box at its feed. Of the units on a station it is the one most exposed to a
strike, and the one sacrificed first.

```
   the loop, 1 m ──▶ LMH5401 ─ RC chain ─ THS4541 ──▶ AD9265-80, 2²⁶ SPS ──PSSI──▶ ┌──────────────────────────┐ ── NodBus spur, type 4 ──▶ BIFROST
                                                                                   │ MARCONI — STM32H7A3      │
                                                         APS25608N PSRAM ◀─OCTOSPI─│ burst · FFT · sweep ·    │
                                                                                   │ chirp scaler             │
                                                                                   └──────────────────────────┘
```

## Files

| file | contents |
|---|---|
| [`BAND.md`](BAND.md) | why 0,5–16 MHz, the targets, the revisit, why one channel carries the range, the product test |
| [`HARDWARE.md`](HARDWARE.md) | the board — the loop as a source, surge, direct sampling and the RC chain, the front end with its values, the clock chain, the PSSI, parts and rails, the burst, the processor's pins, the station interface |
| [`CONSTRUCTION.md`](CONSTRUCTION.md) | the loop's orientation and build, the enclosed ring, the mount that keeps the island floating |
| [`BUS.md`](BUS.md) | the carrier and ionogram records |
| [`FIRMWARE.md`](FIRMWARE.md) | the firmware, described — the burst, the transform, the carriers and the sweep, the chirp and the scaler, the registers, what is tested |
| [`models/`](models/) | `pointing.py` — the loop's orientation from the station's coordinates, the tool behind `CONSTRUCTION.md`'s table |
| [`WHY.md`](WHY.md) | the graveyard — rejected alternatives and superseded states |

## Licence

Hardware: CERN-OHL-S v2 (`../LICENSE-HW`) · Software: MIT (`../LICENSE`) — Copyright (c) 2026 NIC — Native Intellect Community

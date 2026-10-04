★ N.I.C. ★

# Sputnik — the bus and the archive

> **Design-stage concept.** The figures are verified against the parts' sheets and the parts
> before a board is made.

Sputnik is NodBus **type 8**, **five NUMBERs and five slots** on one board, one 40 B DATA frame a
slot a period at 128 frames/s. The frames, the opcodes and the node contract are the station's
(`../core/PROTOCOL.md`); what is Sputnik's is what rides the 32 B payloads: one GNSS epoch every
200 ms, cut into chunks, in two tiers.

## Curate, don't strip

**The raw carrier phase is the one irreplaceable measurement** — TEC and every correction derive
from it, and nothing derives it back — so it is always kept. What is dropped is ballast: the fourth
and fifth frequencies, which add little to TEC, and Doppler, which the phase carries. **No
elevation gate**: every tracked satellite goes up, and a low one's multipath is a quality flag in
its record, not a cut.

**Two tiers, and both are the archive.** The receiver's own ionospheric delay is not sent at all:
it is a broadcast Klobuchar or NeQuick model, ~50–70 % RMS, and the server computes the real thing
from Tier B.

| tier | what | per | on the wire | archived |
|---|---|---|---|---|
| **B** | the raw backbone — pseudorange, carrier phase and SNR on three frequencies | satellite, 31 B | always | **the archive** — RINEX-convertible |
| **C** | the broadcast augmentation products, decoded by the receiver | system, as the pages change | on (`TIER_C`) | **yes** — an independent reference, not derived from our raw |

## What is taken from where

| system | Tier B, three bands | Tier C, once per system |
|---|---|---|
| GPS | L1C + L5 + L2C | — |
| GLONASS | L1 + L3 + L2 | — |
| Galileo | E1 + E5a + E5b | E6-HAS — global PPP and a VTEC grid |
| BeiDou | B1C + B2a + B3I | B2b-PPP |
| QZSS | L1C + L5 + L2C | — |
| NavIC | L5 + S — its only two civil bands | — |

Each band carries **pseudorange, carrier phase and SNR**. **Galileo is a full ranging constellation
and an E6-HAS source**; the augmentation page on an outer band is taken once per epoch into Tier C,
never as a fourth Tier B slot.

## The epoch

**One epoch every 200 ms, one byte stream, cut into 32 B chunks** — `kind` 0 on every chunk, the
epoch boundary in the bytes:

| part | content | size |
|---|---|---|
| **the time reference** | ahead of the first epoch to begin in each station second (the reader sees `unix.0` change): the Unix second the receiver's last `RMC` named — UTC, the archive's `TIME` offset turning it to the station's GPS scale · `pps_tick`, uint32 — the receiver's PPS edge on the node's grid, in 2²⁷ ticks after the frame-0 boundary | 8 B, once a second |
| **the lag** | how far back the epoch belongs, in units of 2⁻¹⁰ s, from the frame the first chunk leaves in to the receiver's epoch tag — a fixed offset, not jitter, carried as a number so it survives any cadence | 2 B |
| **arc records** | where a satellite's arc begins, ahead of its Tier B record: `PRN · 0xFF · flags · slot count`, then `base pseudorange (int64, mm) · base phase (int64, 0,001 cycle)` per slot | 52 B |
| **Tier B**, per satellite | below | 31 B × N |
| **geometry records** | ahead of a satellite's Tier B record, round-robin: `PRN · 0xFE · azimuth (uint16, 0,01°)` | 4 B |
| **Tier C** | the decoded pages that changed since the last epoch, each `source · length · page` | as they come |
| **padding** | zeros to the chunk boundary; **an epoch ending exactly on a boundary is followed by one chunk of zeros**, so an epoch's first chunk is always the first non-zero chunk after padding | ≤ 32 B |

**No per-epoch header — no constellation, no satellite count.** The PRN's range encodes the
constellation, and the records are fixed-stride, so the reader walks them to the zero at a PRN
position. **The epoch's instant is the receiver's tag**: an epoch tagged `week · ms` falls `ms`
milliseconds after the PPS edge `pps_tick` places on the grid, 134 217,728 ticks to the
millisecond.

**The arc base.** A 4 B millimetre field cannot carry a 20 000 km pseudorange, so pseudorange and
phase ride **relative to a base declared when the satellite's arc begins**; a cycle slip the
receiver flags starts a new arc. **The arc records are re-sent round-robin, one satellite per
epoch**, so with 60 satellites every base recurs within 12 s: a reader that joins mid-stream or
lost a chunk has every base within that, and holds a record whose base it lacks rather than guess.

**The azimuth rides the same way.** A satellite moves across the sky about half a degree a minute
and the station does not move, so its azimuth is not sent every epoch: **one geometry record an
epoch, one satellite round-robin**, so every satellite's azimuth recurs within 12 s at 60
satellites. The value is the receiver's own — it knows every satellite's orbit and reports azimuth
and elevation in its `SATSINFO` log (`FIRMWARE.md` §6). The elevation rides every Tier B record;
with the azimuth the pierce point is placed on the station or on the server alike. `0xFE` and the
arc record's `0xFF` sit where a Tier B record carries its elevation, 0–90, so the three shapes
never collide.

## Tier B — the raw backbone, 31 B a satellite

```
Header (4 B):
 0 1B PRN — its range encodes the constellation
 1 1B Elevation (1° / LSB)
 2 1B Flags — lock, cycle slip, health, GLONASS k[3:0] (k + 7, 0–13)
 3 1B Number of frequency slots present (3; 2 for NavIC)

Per frequency slot (9 B, ×3), in the fixed order slot 1 (L1-class) · slot 2 (L5-class) · slot 3 (mid):
 +0 4B Pseudorange — relative to the arc base, mm
 +4 4B Carrier phase — relative to the arc base, 0,001 cycle
 +8 1B SNR — (C/N0 − 20) × 4 → 20–83,75 dB-Hz
```

**The frequency is not on the wire**: (constellation × slot position) → frequency, by the table
below, as RINEX declares observation types per system in its header. GLONASS's FDMA channel `k`
rides the flags byte.

**The backbone — three bands a constellation**: an L1-class, an L5-class and a mid band. L1 and L5
are the widest spread and so the best TEC sensitivity, and L5/E5a/B2a share one frequency across
systems, which cross-validates constellations directly; the mid band gives a redundant
geometry-free combination for the differential code bias and slip checks.

| system | slot 1 (L1-class) | slot 2 (L5-class) | slot 3 (mid) |
|---|---|---|---|
| GPS | L1C 1575,42 | L5 1176,45 | L2C 1227,60 |
| GLONASS | L1 FDMA ~1602 | L3 CDMA 1202,025 | L2 FDMA ~1246 |
| Galileo | E1 1575,42 | E5a 1176,45 | E5b 1207,14 |
| BeiDou | B1C 1575,42 | B2a 1176,45 | B3I 1268,52 |
| QZSS | L1C 1575,42 | L5 1176,45 | L2C 1227,60 |
| NavIC | L5 1176,45 | S 2492,028 | — two slots |

**NavIC has no L1-class band**, so its own two slots are L5 and S; the S band's offset from L5
gives it exceptional TEC leverage. **The UM980 tracks NavIC on L5 alone**, so the S slot stays
empty on this receiver (`HARDWARE.md`) and is kept for one that tracks it.

## Tier C — the augmentation products

| source | carrier | content | the receiver's logs |
|---|---|---|---|
| Galileo **E6-HAS** | 1278,75 | the mask, orbit, clock, code-bias and phase-bias blocks | `E6MASKBLOCK` · `E6ORBITBLOCK` · `E6CLOCKFULLBLOCK` · `E6CLOCKSUBBLOCK` · `E6CBIASBLOCK` · `E6PBIASBLOCK` |
| BeiDou **B2b-PPP** | 1207,14 | the five information types — mask, orbit, clock, biases, URA | `PPPB2BINFO1` … `PPPB2BINFO5` |
| the broadcast ionosphere models | the navigation messages | GPS Klobuchar, BeiDou, BeiDou-3, Galileo NeQuick parameters; the GPS–UTC parameters | `GPSION` · `BDSION` · `BD3ION` · `GALION` · `GPSUTC` |

They change every 1–10 s, or once an hour for the models, so they cost little; every one is logged
`ONCHANGED`. **Two sources that are not here and why**: QZSS **L6E MADOCA** is decoded only in the
receiver's signal groups that drop part of the Tier B backbone, and the backbone wins; the **SBAS
ionospheric grid** has no log on this receiver — it tracks SBAS for its own fix and outputs nothing of
it (`FIRMWARE.md` §6).

## The five slots are one pool

**One H523 owns all five NUMBERs, and the five slots are one queue**: a chunk goes into whichever
slot comes next and the master reassembles by frame index and slot order; the PRN says which
constellation each record is. A fixed constellation-to-slot map would pin the busiest constellation
— BeiDou over Asia, ~20 satellites — near 90 % of one slot; pooled, the load spreads evenly. The
per-constellation grouping is the archive's, not the wire's.

## Rate and bandwidth — 5 Hz, every satellite, no elevation gate

| | 200 ms at 5 Hz |
|---|---|
| the five slots carry | 5 × 25,6 = **128 chunks** |
| the worst sky on Earth — 60 satellites × 31 B, one arc record, one geometry record, Tier C | under 2,3 kB = **70 chunks, 55 %** |
| a typical sky, ~35 satellites | ~30 % |

**8 Hz fits at 87 %; at 10 Hz the worst epoch does not fit, so `SET RATE 10` is refused**, and an
epoch that would overflow the queue is dropped whole, never sent in part. **5 Hz is chosen for the
science, not the wire**: TEC is a 1 Hz discipline (IGS and IONEX run 1 s or 30 s), so 5 Hz already
oversamples it fivefold, and the one thing that needs a high rate — scintillation, 50–100 Hz — is out
of reach on any rate the bus allows. The value is in spatial and frequency diversity — every
constellation, every satellite, many stations, many ray angles — and the headroom is margin.

## What is archived

**Tier B and Tier C**, locally, RINEX-convertible from Tier B for RTKLIB, Bernese, IGS and IONEX
tooling, with the geometry records beside them. Where the science is computed from them is
`PROCESSING.md`.

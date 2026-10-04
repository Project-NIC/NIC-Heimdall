★ N.I.C. ★

# Steinmetz — the firmware, described

> **Design-stage concept — nothing built.** The figures are verified against the parts' sheets
> and the parts before a board is made.

**The image is Tesla's — the ring, the ladder, the strips, the detector, the CFD, the source
tracker, the three NODs, the control plane (`../FIRMWARE.md` §4–§8) — with four changes: the two
type bits mean line classes, the floor record moves to type 3, the class is decided over a source
and not an event, and the reset values are the grid's.** The announce is type 13; Steinmetz #1 is
`0xD1`. Nothing else runs on the core.

## The three NODs — always three

Steinmetz takes **three NUMBERs and three slots a frame, always**: on its port it is the one unit,
and three slots of eight are ~15 % of its 2²⁰ spur (3 × 40 B × 10 bits × 128/s). Every slot is sent
every frame whether or not it has content; an empty payload is zeros, exactly as on Tesla.

| NOD | address | what the 32 B payload carries |
|---|---|---|
| **1 — events** | `0xD1` | up to eight 4 B event records: word A the 16-bit backward offset in ticks of 2⁻²⁰ s, word B `sign << 15 \| magnitude << 2 \| type`, **type 0 arc · 1 corona · 2 partial discharge · 3 UFO**. An event carries the class of the source it belongs to; an impulse locked to no source is UFO |
| **2 — sources and the floor** | `0xD2` | one 4 B record per tracked source every 32nd frame, the layout of NOD 1, the latest pulse and the source's class; up to eight sources. **The floor once a second: type 3, word A = 0, word B the detector's rolling background as 14-bit log in 2⁻⁶ dB** — a UFO is never a tracked source, so a type 3 record in this NOD is the floor and nothing else |
| **8 — the supplement** | `0xD8` | per event, slot for slot with NOD 1 in the same frame: azimuth (uint8, 0..255 over 180°, the frustum's own frame) · rise time (uint8, logarithmic) · `t(mid) − t(lo)` · `t(hi) − t(lo)` (int8 each, whole ticks, saturated). The two deltas are the modal split on the line, 0,52–1,04 µs a kilometre — **the distance to the fault along the line from one unit**; the azimuth and the rise time as on Tesla |

The record shapes, the backward offset, the filling of a frame and of its supplement, `NUMBER > 7`
as the supplement's mark — all `../BUS.md` and `../FIRMWARE.md` §7, unchanged.

## The classes

**A source is classified, an event inherits.** Tesla's source tracker locks a train of impulses
to a fundamental learned from the background — 50 or 60 Hz and their harmonics, DC traction's
300 or 600 Hz — over 0,3 s of intervals, and gives it a slot; Steinmetz reads the slot's
features over the seconds that follow and assigns the class, which every further event of that
slot carries. The features are Tesla's: the phase of each pulse within the mains cycle, the
amplitude against the rolling background, the rise time, the decay.

| class | locked to the mains | amplitude | phase of the pulses | shape |
|---|---|---|---|---|
| **0 arc** | yes | at or above `SRC_MULT` × background | at the voltage peaks, both polarities | rise under a microsecond to microseconds; the rate holds or grows over seconds |
| **1 corona** | yes | between `DET_MULT` and `SRC_MULT` × background | concentrated on the negative peak, 270° | regular intervals within the half-cycle |
| **2 partial discharge** | yes | small, regular | clustered in the rising quadrants, 0–90° and 180–270°, both polarities | fixed phase from cycle to cycle |
| **3 UFO** | no | any | — | a stroke, a vehicle, a switch, anything not locked |

A source that locks but fits no row stays UFO until it does. No lightning class: a stroke is
impulsive and unlocked and is UFO.

**The vehicle's own sources are rejected by geometry, not by spectrum.** Every event already
carries its azimuth from the three rods and its two strip deltas; the node keeps both per slot
beside the histogram. A source on the vehicle — ignition, an inverter, a motor — sits at one
azimuth with zero deltas and stays there however the rate, the amplitude and the spectrum move
with the engine, while every source on a line walks through azimuth as the vehicle passes it and
carries a modal split. **A slot whose azimuth has not moved by more than one step and whose deltas
have stayed at zero for 10 s while the head reports motion is the vehicle's**: it is dropped from
the table, its events ship as UFO, and the slot is free. At a static site the rule is off, because
nothing moves.

**The phase histogram.** For each of the eight slots the node keeps a histogram of its pulses
over the mains cycle — **36 bins of 10°, a count and a summed amplitude per bin**, decaying with
a time constant of 10 s — and decides the class from it. The histogram is not emitted; it answers
`GET PHASE_HIST n` with `kind` 5 register frames, one slot per read — its 72 B paged over three, so the server can see
the pattern behind a class it doubts. It costs ~1 % of the core.

## The registers the image fixes

Tesla's registers (`../FIRMWARE.md` §8), with these reset values:

| register | Tesla | Steinmetz |
|---|---|---|
| `SRC_FMIN` | 100 Hz | **33 Hz** — the grid-only floor; 16,7 Hz traction's 33 Hz envelope is tracked |
| `DET_MULT` | 4 | 4 |
| `SRC_MULT` | 8 | **4** — corona is a product here, not a nuisance, and earns a slot |
| `STRIP_LO` · `STRIP_MID` · `STRIP_HI` | 8–24 · 248–264 · 496–512 kHz | **on the line**: one below the PLC band, one inside it, one past the line traps, set under the line at commissioning |
| `0x001B PHASE_HIST` n | — | **new, read only**: the slot's histogram, 36 × (uint8 count, uint8 log amplitude), `kind` 5, paged |

Each is a register and a `SET`, and a change is an archive epoch, as on Tesla.

## What the head does for it

**A moving unit's position is the head's.** The head reads Kronos's `POSITION` register once a
second and writes it to the archive; the server lays each event on the track by its time. At
28 m/s the interpolation between two seconds is under a metre. At a static site the head reads it
once, as for any station.

**The navigation rate is Kronos's `NAV_RATE`, 1 Hz as shipped, and a build sets what it wants**:
at 5 Hz the head reads `POSITION` five times a second and the fixes are 5,6 m apart instead of
28 m. The pulse and the time do not move with it. The server's trigonometry carries a 5 m position
error without noticing, which is why 1 Hz is the default and 5 Hz a choice, not a requirement.

## What the image leaves idle

`TIME OUT` is not served; the carrier tracker, the lightning branch and the sferic shapes of the
universal image are not in the binary. The board's second data body stays populated and dark.

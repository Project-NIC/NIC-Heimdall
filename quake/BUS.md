★ N.I.C. ★

# Quake — the bus

> **Design-stage concept.** The figures are verified against the parts' sheets and the parts
> before a board is made.

Quake is NodBus **type 5**, one slot, one 40 B DATA frame a period at 128 frames/s. The frames,
the opcodes and the node contract are the station's (`../core/PROTOCOL.md`); what is Quake's is
what the 32 B payload means, what rides the report frame, and the one width decision behind
every axis.

## The frames

```
DATA     unix.0 · frame · TYPE|NUM · slot · kind · status · payload 32 B · CRC-16      = 40 B
CONTROL  unix.0 · frame · TYPE|NUM · SUB · op · arg0 · arg1 · arg2 · rsvd · rsvd · CRC-16 = 12 B
```

- The address byte is `TYPE«4 | NUMBER`: 0x5N, the NUMBER the sweep assigns in `ASSIGN_ADDR`.
  The TYPE says what the 32 B mean; the length says which frame it is. No magic byte, no length
  field.
- `unix.0` is the second mod 256, `frame` the sample within it, 0..127; the absolute second is
  prepended by the card on the way up.
- `SUB` is 0 — a Quake is a spur unit.
- Both frames close with CRC-16-CCITT, one configuration of the H523's CRC unit for both.
- The payload is raw: no compression and no encryption on the node. The one transform on it is
  the levelling matrix (`FIRMWARE.md` §7).

## The DATA payload

| bytes | field | source | range / mode |
|---|---|---|---|
| 0–5 | ADXL355 X/Y/Z | the precise seismic channel | ±2 g (±4 g by `CFG` 0) |
| 6–11 | ICM accelerometer X/Y/Z | the clip-free seismic channel | ±8 g (±16 g by `CFG` 1) |
| 12–17 | ICM gyro X/Y/Z | rotation | ±15,625 dps |
| 18–23 | SCL3300 X/Y/Z | tilt and drift, slow | Mode 1 or Mode 4, read-latest |
| 24–29 | RM3100 X/Y/Z | the magnetic field | 32 Hz poll, interpolated onto the grid at a fixed 4-frame lag |
| 30–31 | reserve | 0 | |

**Every axis is int16, little-endian, levelled.** A sensor that is absent keeps its six bytes at
zero; the head knows the population from the present-mask (`SENSORS`, `FIRMWARE.md` §8). The
slow sensors — the SCL3300 and the RM3100 — ride every frame at their latest value; the
repetition costs nothing on the bus and nothing at storage, and the head decimates them to their
own rate.

**No housekeeping rides the payload.** The trigger is the header's ALARM flag, the supply the
SUPPLY flag, the QC verdict the `status` code and the `FAULT` flag (`../core/PROTOCOL.md` §1).

## 16 bits an axis

The sensors' effective resolution at 128 Hz is 14–15 bits, so two bytes carry the signal with
headroom on every axis.

**The ADXL355 reads 20 bits and drops four for the wire.** The levelling runs at full width; the
packing divides off the low four bits toward zero and saturates to ±32767, so a railed axis clips
and never wraps. What makes the cut harmless is that the surviving step stays buried in the
sensor's own noise, which is the dither: an average lands between steps and recovers resolution
below one LSB. **The constant is three LSB of surviving noise, not four bits**:

| bits dropped | LSB at ±2 g | noise in LSB | |
|---|---|---|---|
| 0 | 3,81 µg | 53 | |
| **4** | **61 µg** | **3,3** | **the setting** |
| 5 | 122 µg | 1,6 | marginal |
| 6 | 244 µg | 0,8 | the dither gone |

The sensor's noise is 25 µg/√Hz × √64 Hz ≈ 200 µg RMS at this rate; the cut adds `LSB/√12` =
17,6 µg in quadrature, **0,4 % more noise**. A part ten times quieter would put 61 µg at 0,33 LSB,
and the row is re-derived with any change of accelerometer.

**The band carried is wider than the signal's, and it stays so.** 64 Hz carried for content below
20 Hz is √(64/20) = 1,8× the necessary noise, 0,84 bit — two orders more than the four dropped
bits. It is filtered downstream and never on the node, because 20–64 Hz holds the sharp part of
an onset.

The ICM-42688-P is natively 16 bits and drops nothing.

## The report frame and `HEALTH`

**`REPORT`, `kind` 6**, once a `REPORT_INTERVAL` (60 s), unasked, in the fourth frame-time of the
node's own block: `VBUS` · `CURRENT` raw off the power board's `INA238` · the TMP117 · the coil
NTC — four int16 words, the temperatures in 0,01 °C, positions 1–3 zeros. Leaving `VIN_WINDOW`
raises the SUPPLY flag in the header, and the head logs the edge (`../core/PROTOCOL.md` §5).

**`HEALTH`, `kind` 8**, on `GET HEALTH` only: the ADXL355, ICM, SCL3300 and MCU die temperatures
as int16 in 0,01 °C at bytes 0–7, then the error counters. The sensors compensate their own dies,
so these are a technician's covariate and not a channel; the node corrects nothing for
temperature.

**The supply is measured at the input only** — no per-rail monitoring. What hangs is the MCU, and
the IWDG and the source end's `ENABLE` are what clear it.

## Sizes

| frame | size | rate | load |
|---|--:|---|--:|
| DATA | 40 B | 128 frames/s | 41 kb/s |
| CONTROL | 12 B | on demand | negligible |

A Quake is alone on its spur and runs the lone-unit rung, 2²⁰, which `BUSCFG` hands down at
enrollment (`../core/blocks/nodbus.md`). The payload length is a TDMA invariant: a variable
length would cost the fixed slot schedule and buy nothing on a wire this empty.

## At storage

**The archive codes this payload in Steim's shape.** HCC turns a segment's rows — up to 8 s of 128 a
second — into one series per field — each axis down its own column — and codes each against its last
value or the line through the last two, with a Rice parameter chosen per 32 samples, so an event
costs bits where it happens (`../core/archive/HCC.md`). **The slow channels need no decimation
before the write**: the tilt and the field ride at their latest value, repeat between updates, and a
repeated value is a zero residual of one bit a frame. The miniSEED export takes the axes' series as
they are.

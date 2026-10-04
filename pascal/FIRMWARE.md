★ N.I.C. ★

# Pascal — the firmware, described

> **Design-stage concept.** The figures are verified against the parts' sheets and the parts
> before the build is written. What the sonde is: `README.md`; what it measures, the board, the
> pins and the timers: `HARDWARE.md`; the mini contract every mini-NOD runs:
> `../quark/tubes/FIRMWARE.md` Part A, §A4; the frames and the opcodes: `../core/PROTOCOL.md`; the
> clock and the mini rung: `../core/blocks/nodbus.md`. Where this document and one of those differ,
> that one wins.

## 1. What the firmware is

**A gauge read in bursts and averaged over the wave.** The firmware starts a pressure conversion
on the MS5837-30BA at 2ⁿ-tick intervals of the grid, sixteen a second, reads each after its
conversion time, keeps a running mean over the wave period, and ships that mean every frame in
the 8 B mini payload; the trigger and the supply are the header's flags, and the temperatures
and the arrived feed go up once a minute in the `REPORT` frame. It
computes no depth and no tide: a change is the measurement, the offset is the archive's
(`HARDWARE.md`, *What it measures*).

| the firmware does | on | how often |
|---|---|---|
| starts and reads a pressure conversion | I2C3, on a TIM2 compare | **16 Hz** — every 8th frame |
| reads a temperature conversion | I2C3 | once a second |
| averages and ships | USART1, one TIM2 compare a round | 128 frames/s |
| reads the power body | I2C1 | once a second |
| sends the report frame | USART1, the fourth frame-time of its own block | once a `REPORT_INTERVAL`, 60 s |

## 2. Boot

| # | step | what happens | if it fails |
|---|---|---|---|
| 1 | reset | `DE` low; the IWDG at **1 s** | — |
| 2 | RC | **there is no crystal on this board** — the HSI runs the core and the UART, free-running for now and disciplined by the rung from step 6 on (§4a); the flash cells read (§9): the set, the averaging window, the alarm threshold, the oversampling; the tag | a set that fails its check is no set |
| 3 | the rail | `PGOOD` (PD14) read | low: `HEALTH` *rail*, `FAULT` up |
| 4 | IDs | ADC1 reads `ID_D`, `ID_P` once, ratiometric | a ratio in no window is *unknown board* |
| 5 | the gauge | I2C3 at 400 kHz: the MS5837's reset command, then its six calibration coefficients read from PROM and their CRC-4 checked; one conversion pair run and the compensated pressure checked to lie within 0,5–35 bar | no answer or a bad CRC: `HEALTH` NO_RESPONSE *gauge*; the node enrols and ships zeros |
| 5a | the thermometer | I2C3: the `TMP117` read once | silent: `HEALTH` NO_RESPONSE *thermometer*; the gauge's temperature carries on |
| 6 | the rung | the segment's rung on `ETR` (PA15) counted against the core for 10 ms; the rung's value learned, and **the ratio sets `HSITRIM` coarsely**, landing the HSI inside `FRACN`'s pull range, and steps again whenever `FRACN` nears the edge of its window; the `FRACN` loop then takes over and runs for the board's life (§4a) | no edges: the core holds at its last code, silent, waiting |
| 7 | enrol | the mini contract, `../quark/tubes/FIRMWARE.md` §A4, type **3**, `GET slots` 1 | — |

## 3. The states

The mini-NOD state machine of `../quark/tubes/FIRMWARE.md` §A3: `RC` → `ENROLLED` → `LOCKED` (TIM2
on the rung, the conversion train running, nothing sent) → `RUNNING` · `ENDED` on `END`
(the conversions stop and the last value is answered; the gauge idles in microamps by itself; the
USART listening until the rung goes) · the rung lost is the fault and the rejoin the return. On a rejoin the train restarts
from the phase edge and the mean is rebuilt, with `status` 4 WARMUP on the frames until the
window is full.

## 4. Time on the node

**TIM2 counts the rung** (external clock mode 2 on `ETR`); a compare every 2ⁿ ticks is the
grid — every frame, 4096 ticks at 2¹⁹, its origin the `SYNC` edge less the written `ROUTE`,
scaled to the rung by a shift (`../core/PROTOCOL.md` §7) — and every 8th compare starts a pressure conversion. A
conversion's instant is its start on the grid plus half its conversion time, **8,6 ms at OSR
8192**, a constant of the setting. The mean of §5 is placed at the middle of its window, a
fixed lag of half the window — **8 s** at the default — which the archive subtracts; nothing
on the wire carries an age.

## 4a. The core's clock — the discipline loop

**The board has no oscillator, so the core's frequency is a measured quantity and not a given.**
The mechanism and its numbers are in `HARDWARE.md`, *The rung disciplines the core*; what the
firmware does with it is this.

**The gate.** `TIM5` counts SYSCLK and is latched by `TIM2`'s frame compare over the internal
trigger — hardware to hardware, nothing in the interrupt path can lengthen the interval being
measured. The difference between two latches must read **1 048 576**; one count is 0,95 ppm.
**Two gate lengths and the loop picks by its own residual:** one frame while it is still moving,
one second (every 128th latch, 0,0075 ppm) once it has settled.

**The correction.** A simple integrator on the error, written to `FRACN[12:0]` with the PLL
running — `PLLFRACEN` cleared, the code written, `PLLFRACEN` set. Where the residual is finer
than one code the loop **alternates between the two adjacent codes** and the duty carries the
average; there is no phase to protect on this board, so the alternation needs no rate limit.

**`HSITRIM` is the coarse control, and it runs for the board's life too.** Its job is to keep the
HSI inside `FRACN`'s pull range. It is set at bring-up from the first gate, and **stepped again
whenever `FRACN` passes a guard band a quarter of its window from either edge**: one code toward
the centre, and `FRACN` runs back. The HSI drifts −2 / +1 % over the junction range, more than
`FRACN` can pull, so a land board of the mini tier would otherwise run `FRACN` onto its rail
between spring and winter — on the sea floor the step may never fire; the UART sees a 0,24 % step for a few frames, a tenth of its tolerance, and the
grid sees nothing. **It is searched by measurement, never by assuming the curve rises** — the step
is 0,24 % typical but goes *negative* at multiples of 32, and as far as −5,2 % at 128, 256 and 384
(`HARDWARE.md`). A candidate code is written, one gate is taken, and a code landing on one of those
boundaries is stepped off it.

**Holdover — two tables, both built, one chosen at the bench.** While the rung is present the
loop stores `FRACN` against the **raw reading** of each of the H523's two temperature sensors —
never against a temperature, so neither part's slope, offset or linearity ever enters
(`HARDWARE.md`). Both are coarse tables, written slowly, kept in RAM and checkpointed to flash.

- **the analogue sensor** — `VSENSE` and `VREFINT` converted in one sequence and the ratio stored.
  **The normalisation is not optional**: un-normalised, one percent of rail reads as 3,1 °C and
  the holdover chases the buck with `PGOOD` still high. Entries are averaged; how many samples is
  the bench's answer.
- **the DTS** — `DTS_DR` read straight. No timer, no conversion, nothing to normalise. While the
  rung runs its reference is exact, because PCLK is exactly what the loop has just measured.

**A build ships with one of them driving `FRACN` and the other as a check.** They watch the same
die by two different physical routes, so a divergence between the tables is a fault report
nothing else on this board produces, and it costs a subtraction.

On a rung loss the loop freezes and the chosen table drives `FRACN` from its reading alone;
`status` carries the degraded flag. On the rejoin the loop closes again from the short gate.

**What is trusted when.** The unit enrols as soon as the baud is inside tolerance — it does not
wait for the long gate. The grid never waits for any of this: it is `TIM2` on the rung and is
exact from the first edge.

## 5. Acquisition — one conversion, one mean

1. The compare fires: the D1 (pressure) conversion command at the configured OSR — **8192**
   by default, 17,2 ms — over I2C3, 2 bytes, 50 µs.
2. A TIM6 compare 18 ms later: the 24-bit result read, 4 bytes, 100 µs; the core in `WFI`
   between. Once a second the D2 (temperature) conversion runs in the next slot of the train in
   place of a D1, and its result is held for the compensation.
3. The datasheet's second-order compensation from the six PROM coefficients: pressure in
   0,01 mbar, temperature in 0,01 °C — the gauge's own arithmetic, in the node.
4. The pressure into a ring of **256 samples** — 16 s at 16 Hz, the wave period a 15 s swell
   demands (`HARDWARE.md`, *What it measures*); the running sum kept, the mean `Σ/256` read every frame.
5. **The ALARM flag**: the 16 s mean against a **1 h** running mean of the same; a departure by
   more than the threshold (`ALARM`, default 10 mbar — half the 20 mbar a tsunami is at 200 m)
   sets the ALARM flag in the header and holds it while it persists.

**No dead time, no overlap.** A conversion pair fits with margin inside the 62,5 ms train
interval, and nothing else touches I2C3.

## 6. The payload

```
  0-3  pressure   uint32, 0,01 mbar — the 16 s running mean         little-endian
  4-7  reserve    0 — the trigger and the supply are the header's flags;
                  the temperatures ride the REPORT frame
```

**The report frame, `kind` 6, once a `REPORT_INTERVAL` (60 s), in the fourth frame-time of the
sonde's own block, behind the data:** `VBUS` · `CURRENT` as the `INA238`'s raw registers — the
sonde stands on the sea floor behind a barrier, so the arrived feed is worth the two words · the
wall `TMP117`, the bottom-water record · the gauge's own temperature, int16 in 0,01 °C; a reply
owed goes first and the report waits a round. `GET REPORT` brings it now, and the
**SUPPLY flag** in the header says when the supply left `VIN_WINDOW` (`../core/PROTOCOL.md` §1,
§5). The gauge's temperature is still converted once a second — the pressure compensation needs
it — it just no longer rides every frame.

## 7. The control plane

| register | r/w | what |
|---|---|---|
| `0x0002 STATUS` | r | the node state, the rung state, the gauge state, the ranging width |
| `0x0010 OSR` | r/w | the oversampling: 256 · 512 · 1024 · 2048 · 4096 · **8192** |
| `0x0011 WINDOW` | r/w | the averaging window in samples, 16–256, default **256** |
| `0x0012 ALARM` | r/w | the threshold in 0,01 mbar, default 1000 |
| `0x0014 RAW` | r | the last D1 and D2, the six coefficients, `kind` 5 |
| `0x0015 HOUR` | r | the 1 h mean, uint32 in 0,01 mbar |
| `0x0021 RANGE` | w | the ranging turnaround, `arg1` the width in ticks |
| `0x0038 REPORT` | r | answers with the `REPORT` frame itself, `kind` 6 |
| `0x003A HEALTH` | r | answers with the `HEALTH` frame, `kind` 8: the MCU die temperature, the discipline loop's `FRACN` and `HSITRIM` and its two raw sensor readings, the last D1 and D2, the error counters |
| `0xFF00`–`0xFF10` | r | `VERSION` · `IDENT` · `TAG` · `slots` 1 · `HEALTH` (echo/CRC misses, resends, rung losses, IWDG resets, gauge misses, alarms) · `SENSORS` (bit 0 the gauge) · `ID` |
| `0xFF0A VIN_WINDOW` | r/w | the supply window, the SUPPLY flag outside it — the house register |
| `0xFF0B REPORT_INTERVAL` | r/w | seconds between `REPORT` frames, default 60; 0 disables |
| `0xFF13 DELAY` | r/w | the frames the unit holds before it sends — written by its Argus at floor-up, 16, and persisted; never below 8, the image's floor, because `RESEND` is answered from this buffer (`../core/PROTOCOL.md` §7) |

`HARD_RESET`: the NUMBER to 15, the settings to their defaults; the gauge's coefficients are the
part's own PROM and are never stored here.

**Up the link unasked:** `ACK` after a critical command and the `REPORT` frame once a minute — nothing else. `ERROR` only answers a command that failed; a fault is the header's `FAULT` flag, and its detail waits in `HEALTH` for the head's `GET HEALTH`. A CONTROL frame reaches the sonde through its Argus — the Argus's NUMBER outside, this sonde's address in `SUB` — and arrives here as an ordinary frame addressed to this sonde with `SUB` 0, so nothing about the dialogue differs from a unit on a spur (`../core/PROTOCOL.md` §1, §5, §7).

## 8. Faults

| trigger | action | reported |
|---|---|---|
| a conversion not answered, or a result of 0 or full scale | the sample skipped, the mean carried on the remaining ring; three in a row and the gauge is reset and re-read from PROM | `HEALTH`; `HEALTH` DEGRADED after ten in a minute, `NO_RESPONSE` after a minute of nothing |
| the compensated pressure outside 0,5–35 bar | the sample dropped | `HEALTH` |
| the rung lost | mute; the rejoin | `status` 1 REJOINED |
| `ALERT` high on PC8 | latched into `HEALTH` | `FAULT` up |
| the IWDG expires | reset | `HEALTH` |

## 9. Persistence

The H523's cells (`../core/PROTOCOL.md` §7): the set at enrolment · `OSR`, `WINDOW`, `ALARM`,
`VIN_WINDOW`, `REPORT_INTERVAL` on `SET`. No value and no time is stored.

## 10. Budget and tests

USART1 · USART3; TIM2 on the rung; TIM4 ranging; TIM6 the conversion timing; I2C3; I2C1;
ADC1 — under 0,1 % of the core; `WFI` otherwise; the board at ~60–185 mA by the communication module
(`HARDWARE.md`, *Rails*). **On a PC:** the compensation against the datasheet's worked example;
the ring and the mean with a skipped sample; the alarm against a synthetic 20 mbar step over
five minutes under a 15 s swell of 10 mbar; the register map; the cells. **On the bench,
against `HARDWARE.md`:** the gauge against a reference at atmosphere before potting; the
conversion train on a scope at 16 Hz with the reads inside 20 ms; the payload's frame index
advancing once per Argus frame with no slip over an hour; the sonde in its slot at 128 Hz for
24 h with zero fillers on a bench segment.

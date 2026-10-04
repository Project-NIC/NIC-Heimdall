★ N.I.C. ★

# Gauss — the firmware, described

> **Design-stage concept.** The figures are verified against the parts' sheets and the parts
> before the build is written. What the sonde is and the payload: `README.md`; the calibration
> without rotating the site: `CONSTRUCTION.md`; the board, the pins and the timers: `HARDWARE.md`;
> the mini contract every mini-NOD runs: `../quark/tubes/FIRMWARE.md` Part A, §A4; the frames and
> the opcodes: `../core/PROTOCOL.md`; the clock and the mini rung: `../core/blocks/nodbus.md`.
> Where this document and one of those differ, that one wins.

## 1. What the firmware is

**A magnetometer read in silence, placed on the grid by arithmetic.** The firmware polls the
RM3100 in single measurement mode, stamps every conversion's end on the grid from `DRDY`'s
capture, corrects each sample by the pod's own calibration and its temperature, interpolates
onto the frame instants, subtracts the stored baseline, and ships three `int16` deviations a
frame in the 8 B mini payload. It computes nothing else — no spectrum, no filter, no event
beyond one threshold bit: the analysis is above it.

| the firmware does | on | how often |
|---|---|---|
| polls the RM3100 | SPI1, `POLL` on a TIM2 compare | **32 Hz** — every 4th frame |
| captures `DRDY` | TIM3 CH2 (PB5) | per conversion |
| interpolates and ships | USART1, one TIM2 compare a round | 128 frames/s |
| reads the thermometers | I2C2 (the wall TMP117/STS35); ADC1 (the NTC between the coils, its divider switched on for the conversion only) | every 10 s |
| reads the power body | I2C1 | once a second |

## 2. Boot

| # | step | what happens | if it fails |
|---|---|---|---|
| 1 | reset | `DE` low, `SSN` (PA4) high; the IWDG at **1 s** | — |
| 2 | RC | **there is no crystal on this board** — the HSI runs the core and the UART, free-running for now and disciplined by the rung from step 7 on (§4a); the flash cells read (§9): the set, the calibration matrix and offsets, the baseline, the cycle count, the temperature coefficients, the alarm threshold; the tag | a set that fails its check is no set: the identity matrix, zero offsets, zero baseline |
| 3 | the rail | `PGOOD` (PD14) read | low: `HEALTH` *rail*, `FAULT` up |
| 4 | IDs | ADC1 reads `ID_D`, `ID_P` once, ratiometric | a ratio in no window is *unknown board* |
| 5 | the sensor | SPI1 mode 0 at 1 MHz: the RM3100's `REVID` read (0x22); the cycle count registers written — **100** per axis by default — and read back; continuous mode off; `DRDY` polarity set | no `REVID`, or a mismatch: `HEALTH` NO_RESPONSE *RM3100*; the node enrols and ships zeros |
| 6 | the thermometers | the wall thermometer read; the NTC divider switched on and read on ADC1 | a silent wall part, or an NTC reading at a rail: `HEALTH` NO_RESPONSE for it; the correction uses the other, or 1 |
| 7 | the rung | the segment's rung on `ETR` (PA15) counted against the core for 10 ms; the rung's value learned, and **the ratio sets `HSITRIM` coarsely**, landing the HSI inside `FRACN`'s pull range; the `FRACN` loop then takes over and runs for the board's life, stepping `HSITRIM` again only when `FRACN` nears the edge of its window (§4a) | no edges: the core holds at its last code, silent, waiting |
| 8 | enrol | the mini contract, `../quark/tubes/FIRMWARE.md` §A4, type **1**, `GET slots` 1 | — |

## 3. The states

The mini-NOD state machine of `../quark/tubes/FIRMWARE.md` §A3: `RC` → `ENROLLED` → `LOCKED` (TIM2
on the rung, the poll train running, nothing sent) → `RUNNING` · `ENDED` on `END` (the
sensor idle — it idles by itself between polls, so nothing is switched; the polls stop and the
last value is answered; the USART listening until the rung goes) · the rung lost is the fault and
the rejoin the return.
On a rejoin the poll train restarts from the phase edge and the first frame's value is
interpolated from the first two polls after it, with `status` 4 WARMUP on the frames until then.

## 4. Time on the node

**TIM2 counts the rung** (external clock mode 2 on `ETR`); a compare every 2ⁿ ticks — every
frame, 4096 ticks at 2¹⁹ — is the grid — its origin the `SYNC` edge less the written `ROUTE`, scaled to the rung by a
shift (`../core/PROTOCOL.md` §7) — and every 4th of them fires a `POLL`. **`DRDY` is captured
on TIM3 CH2**, whose counter runs on the trimmed 2²⁷: the capture is converted to the grid by
the offset between the two counters, read once per poll as the pair (TIM2, TIM3) latched by
the `POLL` compare itself — a fixed few ticks of core time between the two reads, ±1 tick of
2²⁷, 7,5 ns, against a conversion of milliseconds. So every sample carries two numbers on one
grid: when it was asked for and when it was done (`HARDWARE.md`, *Timers*), and the sample's
instant is the middle of its conversion. **At cycle count 100 an axis takes 1,18 ms** — the
manual's maximum single-axis rate of 850 Hz (Table 3-1; 1600 Hz at 50, 440 at 200) — and the
three axes are measured in sequence, **3,5 ms in all**; the instant is `DRDY` minus 1,76 ms, the
middle axis's middle, the three axes taken as one instant — the ±1,2 ms between them is 2° at
5 Hz.

**The interpolation.** The frame at grid instant `g` carries the field at **`g − 4 frames`**, a
fixed lag of one poll interval (31,25 ms), interpolated linearly between the two samples whose
instants bracket it. The lag is a constant of the type and the archive subtracts it; nothing
on the wire carries an age. With a 4 ms placement error at 5 Hz being 7° of phase, the
interpolation holds it under 0,2° — which the Pc1 band at 0,2–5 Hz sees, and nearest-sample picking does not.

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
`FRACN` can pull, so a pod on a stake in air would otherwise run `FRACN` onto its rail between
spring and winter; the UART sees a 0,24 % step for a few frames, a tenth of its tolerance, and the
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
exact from the first edge. **`DRDY`'s timestamp does**: `TIM3` counts core ticks, so the offset it
converts to the grid is only as good as the loop — ppm-class once locked, and the frames before
that already carry `status` 4 WARMUP.

## 5. Acquisition — one poll

1. The `POLL` compare fires: `SSN` low, `POLL` written for all three axes, `SSN` high — the
   chip converts and goes idle; the core goes to `WFI`.
2. `DRDY` rises: TIM3 captures it; the EXTI on the same pin wakes the core; the nine bytes of
   `MX · MY · MZ` are read over SPI1 in silence — the chip is idle, the bus traffic is the only
   activity in the pod, and the next `POLL` is 31 ms away: bus traffic and measurement never overlap.
3. The raw 24-bit counts become nT by the cycle count's gain — **75 counts/µT at 200, scaling
   with the count** — then the pod's calibration: `B = M × (raw − offset)`, `M` the 3×3
   sphere-fit matrix (gains and non-orthogonality) and `offset` the hard-iron vector, both in
   2⁻¹⁴ from the tumble at the wellhead (§9).
4. The temperature correction: `B × (1 + γ × (T − 20 °C))` per axis, γ in 2⁻¹⁶/K from the
   cells, `T` the NTC between the coils — the sensor's own temperature — read every 10 s, the
   divider on for the microseconds of the conversion only, so no current stands in the coils' field.
5. The sample and its instant into a two-deep buffer; the frame's interpolation of §4 reads it.
6. The deviation: `B − baseline`, the baseline a stored int32 per axis in nT, written at
   commissioning from a minute's mean (`SET BASELINE 0`) and never moved by the node — a
   permanent step with |B| unchanged is a rotated pod and the archive's to see, not the
   node's to hide (`CONSTRUCTION.md`, *Calibration without rotating the site*).
7. Clipped to int16 in **nT**: ±32 µT of deviation, and `status` 6 CLIPPED when it clips.

**The ALARM flag** in the header sets when the deviation's magnitude changes by more than the
threshold (`ALARM`, default 200 nT) between two consecutive frames' values, and holds for a
second. **Bytes 6–7** are reserve, 0. The supply and the temperatures ride the **`REPORT` frame**, `kind` 6,
once a `REPORT_INTERVAL` (60 s), in the slot's fourth frame-time behind the data: `VBUS` ·
`CURRENT` as the `INA238`'s raw registers · the wall `TMP117` · the NTC between
the coils, int16 in 0,01 °C; a reply owed goes first and the report waits a round. `GET REPORT`
brings it now, and the **SUPPLY flag** in the header says when the supply left
`VIN_WINDOW` (`../core/PROTOCOL.md` §1, §5).

## 6. The payload

```
  0-1  X   int16, nT, deviation from the baseline      little-endian
  2-3  Y
  4-5  Z
  6-7  reserve  0 — the field trigger and the supply are the header's flags
```

## 7. The control plane

| register | r/w | what |
|---|---|---|
| `0x0002 STATUS` | r | the node state, the rung state, the sensor state, the ranging width |
| `0x0010 CYCLE` | r/w | the cycle count per axis, 50–400, default **100**; the poll rate follows: 32 Hz to 200, 16 Hz above |
| `0x0011 CAL` | r/w | `M` 9 × int16 in 2⁻¹⁴ and `offset` 3 × int32 in raw counts, 30 B, `kind` 5 both ways — written from the tumble as a `kind` 5 frame down |
| `0x0012 BASELINE` | r/w | 3 × int32 in nT; a write of 0 takes a minute's mean as the new baseline |
| `0x0013 GAMMA` | r/w | 3 × int16 temperature coefficients in 2⁻¹⁶/K |
| `0x0014 ALARM` | r/w | the trigger threshold in nT, default 200 |
| `0x0016 RAW` | r | the last sample's raw counts and its instant, `kind` 5 |
| `0x0021 RANGE` | w | the ranging turnaround, `arg1` the width in ticks |
| `0x0038 REPORT` | r | answers with the `REPORT` frame itself, `kind` 6 |
| `0x003A HEALTH` | r | answers with the `HEALTH` frame, `kind` 8: the MCU die temperature, the discipline loop's `FRACN` and `HSITRIM` and its two raw sensor readings, the error counters |
| `0xFF00`–`0xFF10` | r | `VERSION` · `IDENT` · `TAG` · `slots` 1 · `HEALTH` (echo/CRC misses, resends, rung losses, IWDG resets, missed `DRDY`s, clips, alarms) · `SENSORS` (bit 0 RM3100 · 1 the coil NTC · 2 the wall thermometer) · `ID` |
| `0xFF0A VIN_WINDOW` | r/w | the supply window, the SUPPLY flag outside it — the house register (`../quake/FIRMWARE.md`) |
| `0xFF0B REPORT_INTERVAL` | r/w | seconds between `REPORT` frames, default 60; 0 disables |
| `0xFF13 DELAY` | r/w | the frames the unit holds before it sends — written by its Argus at floor-up, 16, and persisted; never below 8, the image's floor, because `RESEND` is answered from this buffer (`../core/PROTOCOL.md` §7) |

`HARD_RESET`: the NUMBER to 15, the baseline and the alarm cleared, the cycle count to 100; **the
calibration and the coefficients are kept** — they are the pod's, potted with it.

**Up the link unasked:** `ACK` after a critical command and the `REPORT` frame once a minute — nothing else. `ERROR` only answers a command that failed; a fault is the header's `FAULT` flag, and its detail waits in `HEALTH` for the head's `GET HEALTH`. A CONTROL frame reaches the pod through its Argus — the Argus's NUMBER outside, this pod's address in `SUB` — and arrives here as an ordinary frame addressed to this pod with `SUB` 0, so nothing about the dialogue differs from a unit on a spur (`../core/PROTOCOL.md` §1, §5, §7).

## 8. Faults

| trigger | action | reported |
|---|---|---|
| no `DRDY` within 50 ms of a `POLL` | the sample skipped, the last value held, the chip re-initialised after three misses | `HEALTH`; `HEALTH` DEGRADED after ten in a minute |
| a raw count at full scale on an axis | the sample marked, `status` 6 CLIPPED | `HEALTH` |
| the rung lost | mute; the rejoin | `status` 1 REJOINED |
| `ALERT` high on PC8 | latched into `HEALTH` | `FAULT` up |
| the IWDG expires | reset | `HEALTH` |

## 9. Persistence

The H523's cells (`../core/PROTOCOL.md` §7): the set at enrolment · `CYCLE`, `ALARM`, `VIN_WINDOW`, `REPORT_INTERVAL` on
`SET` · the baseline on `SET BASELINE` · **the calibration and the coefficients at the tumble,
kept by `HARD_RESET`**.

## 10. Budget and tests

USART1 · USART3; TIM2 on the rung; TIM3 the `DRDY` capture; TIM4 ranging; SPI1; I2C1 the power body; I2C2 the wall thermometer; the
ADC1 for the NTC and the IDs — the interpolation under 0,1 % of the core at 2²⁷; `WFI` between polls; the pod at tens of milliamperes. **On a PC:** the
calibration and the interpolation against a synthetic field on recorded `DRDY` instants; the
lag constant; the alarm; the clip; the register map; the cells. **On the bench, against
`HARDWARE.md`:** the tumble before potting and the sphere fit's residual under 0,5 %; the poll
train at 32 Hz on a scope with `DRDY` inside 20 ms; the pod in its slot at 128 Hz for 24 h with
zero fillers on a bench segment; the noise floor at rest under the RM3100's datasheet figure at
the cycle count.

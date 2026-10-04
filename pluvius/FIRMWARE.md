★ N.I.C. ★

# Pluvius — the firmware, described

> **Design-stage concept.** The figures are verified against the parts' sheets and the parts
> before the build is written. What the gauge is: `README.md`; the board, the chain, the pickup,
> the dead-drain rule: `HARDWARE.md`; the registers: `MODBUS.md`; the slave's side of the arm — the
> RTU frames, the sweep, the rate, the house block — is Babel's and the same here
> (`../babel/FIRMWARE.md` §4, §6). Where this document and one of those differ, that one wins.

## 1. What the firmware is

**It weighs, it drains, and it publishes totals.** The firmware reads the load cell through
the ADS1235 continuously, converts to grams against the calibration in its cells, runs the
drain state machine — start on a fill threshold or a command, count the revolutions, watch the
weight fall, stop at the baseline, take the zero — and keeps the accumulation `weight + Δ
DRAINED` across every cycle. What the host reads is `RATE`, `HOUR`, `DAY` and their previous
periods, each one register with its settled bit; the ingredients answer only when asked
(`MODBUS.md`, *What is actually read*). Every correction — tare, span, buoyancy, creep, the
counted volume — is applied here and never upstream.

| the firmware does | on | how often |
|---|---|---|
| blinks the LED | PD4, 1 kΩ | one 50 ms blink a minute while healthy, two on a fault — the house code (`../core/HARDWARE.md`) |
| reads the bridge | SPI1, the ADS1235, `DRDY#` on PC4 | **20 SPS** continuous, chopped |
| asks for the drain | `STATUS` bit 1 `PUMP` — Palatine reads it and switches `PWR EXT`, and writes `HEAD` (`0x0014`) back while the head runs; the pickup on TIM3 CH1 (PB4) | on the threshold or on `DRAIN` |
| closes the periods | TIM6's second | the hour and the day boundaries |
| reads the thermometers | `TEMP1` on I2C1; `TEMP2` the NTC on ADC1, the divider switched on for the conversion | every 10 s |
| answers the host | USART1, `THVD1450`, 19 200 8N1 | when polled |

## 2. Boot

| # | step | what happens | if it fails |
|---|---|---|---|
| 1 | reset | `STATUS` bit 1 `PUMP` clear — a boot never asks for the head, and the head is Palatine's to switch; `DE` low; `RESET#` (PB0) low then high; `PWDN#` (PB1) high; the IWDG at **2 s** | — |
| 2 | RC | the 2²⁴ crystal validated against the HSI (± 1 %), **PLL1 at M 2 · N 32** — `f_PLL_IN` tops out at 16 MHz — and SYSCLK 2²⁷; the cells read (§9): the NUMBER, the calibration (tare, span, the buoyancy multiplier, the creep constant), `THRESH`, `EMPTY`, the period boundaries, the ml-per-revolution coefficient, the pickup flag; the tag | a set that fails its check is no set: NUMBER 3, span 1, no tare — the gauge answers with `STATUS` VALID clear until calibrated |
| 3 | the converter | SPI1 mode 1 at 4 MHz: the ADS1235's `ID` read; then: PGA **64**, the input pair of `CELL`, **20 SPS** with the sinc3 filter, **`CHOP` = 11 — 4-wire AC excitation, which chops**: `ACX1` on `GPIO0` and `ACX2` on `GPIO1`, complementary, reverse the bridge with every conversion and the pair is combined, **`DELAY` = 0110**, 189 µs (`HARDWARE.md`, *AC bridge excitation*); the reference ratiometric to the excitation; `START` (PC5) high; every register read back | a mismatch: `RESET#` pulsed, once more, then `STATUS` ADC_FAULT and the unit serves its registers with the last values |
| 4 | the first reading | 2 s of conversions discarded — the filter's settling and the chop's first pairs; the weight valid from then | — |
| 5 | the pickup | TIM3 CH1 capture armed on PB4 where the cells say a pickup is fitted; `RUN` (PB3) and `DIR` (PB5) low — the head still, forward; a test: no pulses expected with the drive off | pulses with the drive off: `HEALTH`, the pickup flag cleared for this boot |
| 6 | the slave | USART1 at 19 200, the RTU gap on TIM6; the slave at `5«2 \| NUMBER` — 0x14 for NUMBER 0 — listening | — |

## 3. The states

```
   IDLE (weighing) ──▶ DRAINING (PUMP set, HEAD 1 from the host, runs of a minute, weight falling, revolutions counted) ──▶ TARING (2 s at the baseline) ──▶ IDLE
      ▲                        │
      │                        └──▶ FAULT: HEAD 1 and less than DEAD_G gone in 5 s → PUMP cleared, DRAIN_TIMEOUT set
      └── a command or a boot clears FAULT
```

`IDLE` weighs and accumulates. `DRAINING` starts when `WEIGHT` passes `THRESH`, or on a write
to `DRAIN`, and holds `PUMP` until `WEIGHT` reaches `EMPTY` — the head itself is Palatine's to
switch (`HARDWARE.md`, *The switch*). `TARING` takes the zero. `FAULT`
holds `PUMP` clear, `DRAIN_TIMEOUT` set, until `DRAIN` is written 0 then 1, or a boot; the
weighing continues throughout. There is no sleep: the converter runs, the head is off, and a
cut arm is a boot.

## 4. The weight

Every conversion, 20 a second: the 24-bit count → `grams = (count − tare) × span`, the span
in 2⁻²⁴ g per count from the bench, then the **buoyancy multiplier** — the vessel's
displacement against the air, a constant of the vessel geometry in the cells — and the **creep
correction**: the cell's specified creep as a fraction per decade of time under the standing
load, applied against the time since the last tare. The result, unsigned, **1 g/LSB**, is
`WEIGHT`; the count itself is `COUNT`. A **3 s median** of it, 60 samples, is what the drain and
the thresholds read, so a splash does not start a cycle. Noise at the bench criterion is under
a gram (`HARDWARE.md`, *The load cell*).

**The field check is WMO's** (Vol. I, chapter 6, §6.2.2.2): a bottle of water weighed full and
empty, poured in without splashing, and `C = (W_f − W_e) / A × 10` millimetres compared with the
rise of `WEIGHT` — on 200 cm², 500 ml must read **25 mm**, 500 g. It is done at commissioning and
at every service, over as much of the span as the bottle allows; a disagreement past 1 % is a span
to recalibrate, and a bottle poured in on a dry day is also the drain's own test.

**The accumulation.** `total = WEIGHT + (DRAINED − DRAINED_at_last_tare) × ml_per_rev` in
grams — 1 ml is 1 g — runs continuously; the periods below are differences of it at their
boundaries. `DRAINED` free-runs and never resets (`MODBUS.md`).

## 5. The drain

1. **Start**: `WEIGHT` above `THRESH` for 3 s, or `DRAIN` written 1. **`STATUS` bit 1 `PUMP`
   set** — the request; `PUMP_REV` zeroed. Palatine reads the bit in its next poll, raises
   `ENABLE` on the `PWR EXT` body and writes `HEAD` (`0x0014`) 1 (`HARDWARE.md`, *The switch*). Until
   `HEAD` is 1 the unit weighs and waits — nothing times out, the vessel is a buffer and the
   host may be busy or the body taken; a wait past 60 s is counted in `HEALTH`.
2. **Running**, from `HEAD` = 1: **`RUN` driven high — on a head with control wires that is what turns it**
   (`HARDWARE.md`, *The head's tail*); every second — `PUMP_REV` from TIM3's captures; **the
   dead-drain window: if the 3 s median has fallen by less than `DEAD_G` over the last 5 s with
   `HEAD` at 1 and `RUN` driven, the drain is dead** — `PUMP` cleared, `DRAIN_TIMEOUT` set, `FAULT`,
   and `HEALTH` logs what the other signals said: `PUMP_REV` zero over the window is a head that
   did not turn, `PUMP_REV` climbing is a tube or air (`HARDWARE.md`, *Detecting a dead drain*).
   **The head runs a minute at a time**: after 60 s `RUN` is released and the median read — at or
   below `EMPTY` is step 3, above it and fallen is another minute, not fallen is the fault — so no
   run is a duration the head was told. A cycle is also ended by a **ceiling of 10
   minutes**, a backstop against a stuck bit and not a duration: the ceiling clears
   `PUMP` without `DRAIN_TIMEOUT`, and if `WEIGHT` is still above `THRESH` the next cycle starts
   three seconds later, so a full vessel after a dead head is emptied in instalments — and by
   `DRAIN` written 0. `HEAD`
   written 0 by the host mid-cycle — its `INA238` saw a stall or a dry run, or its own ceiling —
   ends the cycle the same way, `DRAIN_TIMEOUT` set.
3. **Stop**: the median at or below `EMPTY`: `RUN` released. **`EMPTY` is the base the vessel is
   never pumped under**: ~100 ml of water over the intake in the base build, and where oil is
   floated on the vessel, the oil's volume plus 10 mm of water over the intake, so the intake stays
   in water and the oil stays on top (`HARDWARE.md`, *The vessel, the shell and the tubes*). **Where `FLUSH_REV` is set, `DIR`
   high and `RUN` high again for `FLUSH_REV` revolutions on TIM3, subtracted from `DRAINED`, then
   both low** — the hose empty. Then `PUMP` cleared; Palatine drops the body and writes `HEAD` 0. The weight that left, divided by the
   revolutions that turned, is the cycle's **ml per revolution** — taken into the running
   coefficient only if no rain fell during the cycle (the accumulation's inflow term under
   2 g over the cycle), a step from the last value logged in `HEALTH` as a slipped tube, a
   collapse toward zero as air. Its first value is written to `CAL` at commissioning — the tube's
   figure, or a measured one: a set number of revolutions into a measuring cylinder, or a known
   volume poured in and counted out (`HARDWARE.md`, *The pickup*). Where
   no pickup is fitted `DRAINED` and `PUMP_REV` stay at zero and the accumulation is weight
   alone.
4. **The tare**: 2 s after the head stops, the median's value is the new `tare` for the
   calibration — the zero's thermal drift removed, and nothing else (`HARDWARE.md`, *The vessel, the shell
   and the tubes*); `DRAINED_at_last_tare` recorded; the creep clock restarted.
   `TARE` written by the host does the same at any instant without a drain.

**The head is never told a duration and never a speed.** The volume is what the pickup counted
and the weight confirmed; the supply's wander with the battery changes only how long a cycle
takes (`README.md`).

## 6. The periods, and what is published

**The hour and the day are boundaries the unit is set to.** TIM6 counts seconds from boot; the
host's `SET` of the boundary registers says on which second of the hour and which hour of the
day a period closes — the unit has no clock and no time, so the host writes the phase once,
from its own second, and the unit counts from there; a boot loses the phase and `RATE`'s
settled bit says so until the host rewrites it.

| register | what is written into it |
|---|---|
| `RATE` | the accumulation since the last boundary — rewritten every second; bit 15 UNSETTLED while a drain is running or the vessel is still moving after one (the 3 s median's slope above 1 g/s), **and while `FROZEN` is set** — a frozen vessel accumulates, but its rises are melt and its catch may be capped |
| `HOUR` · `HOUR_PREV` | at the hour boundary: the last complete hour in 0,1 mm — 2 g is 0,1 mm on the 200 cm² catch — and the one before; bit 15 UNSETTLED if the boundary landed during a drain or a slope |
| `DAY` · `DAY_PREV` | at the day boundary, the same |
| `WEIGHT` · `COUNT` · `DRAINED` · `PUMP_REV` · `TEMP1` · `TEMP2` · `STATUS` | state, rewritten as it changes |

**Palatine polls `RATE · HOUR · STATUS` and ships the run as one 8 B block** — `STATUS` in it for the
`PUMP` request; the rest is read on demand through
the tunnel, and the archive keeps the periods (`MODBUS.md`, `../palatine/SENSORS.md`).

## 7. The registers

`MODBUS.md`'s map is the contract; the firmware's side of each:

| register | the firmware |
|---|---|
| `0x0000 RATE` · `0x0001 HOUR` | the totals of §6 — the reading and the second value, the house map's first two (`../core/blocks/modbus.md`) |
| `0x0002 STATUS` | bit 0 VALID (calibrated and settled) · bit 1 PUMP · bit 2 ADC_FAULT · bit 3 DRAIN_TIMEOUT · bit 4 FROZEN |
| `0x0003 WEIGHT` | §4, the 3 s median, uint16 g — the house `RAW` position |
| `0x0004`–`0x0006 HOUR_PREV · DAY · DAY_PREV` | the totals of §6 |
| `0x0007`–`0x0008 DRAINED` | ml, free-running |
| `0x0009 PUMP_REV` | this cycle's count |
| `0x000A`–`0x000B TEMP1/2` | `TEMP1` the `TMP117` on the cell body, `TEMP2` an NTC in the vessel on a lead — where fitted, `0x8000` absent |
| `0x000C`–`0x000D COUNT` | the last converter count |
| `0x0010 TARE` | a write ≠ 0 takes the zero now |
| `0x0011 DRAIN` | 1 starts, 0 aborts; reads 1 while running |
| `0x0012 THRESH` · `0x0013 EMPTY` | the fill that starts a cycle and the base level it runs down to, g — about a litre above the base and about 100 ml standing are the kind of figures; persisted |
| `0x0014 HEAD` | **written by the host**, 1 while the `PWR EXT` body is on — the dead-drain window and the revolution count run from it; a 0 mid-cycle ends the cycle |
| `0x0020 CAL` | span, buoyancy, creep, the period boundaries, `DEAD_G` — the fall the 5 s window must show, 20 g default — the bench and the host write them, plain holding registers, persisted |
| `0x0028 FLUSH_REV` | the revolutions run in reverse at the end of a cycle, §5 step 3; 0 on a head with two wires; persisted |
| `0xFF00+` | the house block, as Babel's §6, type **5** |

A write to a read-only register returns the Modbus exception 02.

## 8. Faults

| trigger | action | reported |
|---|---|---|
| a dead drain — the weight not falling | §5 step 2; `HEALTH` says whether the head turned | `DRAIN_TIMEOUT`, `HEALTH` |
| `DRDY#` late by more than 100 ms | the converter reset and reconfigured | `ADC_FAULT` while it lasts, `HEALTH` |
| a count at full scale | the sample dropped; three running is `ADC_FAULT` | `HEALTH` |
| a pickup step of more than 20 % in ml/rev between dry cycles | the coefficient held at the old value | `HEALTH` *slipped tube* |
| `TEMP2` below 0 °C — the vessel freezing | no drain is started; the accumulation runs on | `FROZEN` while it lasts, `RATE` UNSETTLED |
| the IWDG expires | reset; the drive is off by the pull, the cycle in progress is lost and the vessel is weighed as it stands | `BOOT_COUNT` |

## 9. Persistence

The H523's cells (`../core/PROTOCOL.md` §7): the NUMBER on the sweep's write · the calibration
— tare, span, buoyancy, creep, `DEAD_G` — on `TARE` and on `CAL` writes · `THRESH`, `EMPTY`, `FLUSH_REV`, the
boundaries on write · the ml-per-revolution coefficient after every dry cycle · `DRAINED`
**once an hour and at every drain's end**, so a boot loses at most an hour of counted volume ·
the pickup flag. `HARD_RESET` through the tunnel: the NUMBER to 3, the thresholds and
boundaries to their defaults; **the span, the buoyancy and the creep constant are kept** —
they are the vessel's.

## 10. Budget and tests

USART1; SPI1; I2C1; ADC1 for the NTC; TIM3 the pickup; TIM6 the second and the RTU gap; eleven GPIO — under 1 % of the
core at 2²⁷. **On a PC, with the pins as callbacks:** the conversion and the median against a
recorded count stream; a drain cycle with a synthetic falling weight and pulses, ml/rev
computed; the dead-drain window on a weight that does not fall; a boundary landing mid-drain and
its settled bit; the accumulation across three cycles; the tare; the registers and the
exceptions. **On the bench, against `HARDWARE.md`:** the chain's noise under a gram at 20 SPS
with chop; a litre drained and counted within 2 % of the weight that left; a slipped tube
detected inside 5 s; a stone in the vessel: one fault and no second run; the head stopping on a cut command wire; 24 h of polling with the
totals adding up to a metered inflow within 1 %.

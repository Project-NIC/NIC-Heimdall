★ N.I.C. ★

# Quark-Tubes — the firmware, described

> **This document is the deliverable, not a description of code.** It says everything the
> firmware of `Quark-Tubes` does, in the order it does it, with the numbers it uses — enough to
> write the build from, and enough to test it against. What the unit *is*: [`README.md`](README.md);
> the mini frame: [`BUS.md`](BUS.md); the board, its pins and timers: [`HARDWARE.md`](HARDWARE.md);
> the frames, the opcodes and the node contract: [`../../core/PROTOCOL.md`](../../core/PROTOCOL.md);
> the clock, the bus start and the mini rung: [`../../core/blocks/nodbus.md`](../../core/blocks/nodbus.md);
> the card's side of a mini segment: [`../../bifrost/FIRMWARE.md`](../../bifrost/FIRMWARE.md) §8.
> Where this document and one of those differ, that one wins and this one is corrected. The two
> scintillation boards' firmware is [`../scintillation/photon/FIRMWARE.md`](../scintillation/photon/FIRMWARE.md).

**One firmware.** `Quark-Tubes` is an H523 mini-NOD that counts pulses on timers and EXTI and
ships four `uint16` a frame.

---

## A1. What the firmware is

**Five counters read once a frame.** Three GM tubes and a He³/BF₃ tube, where fitted — its
neutron threshold and its LLD — clock five hardware counters that the core never sees a pulse of; the thirteen tubes of a
Gadolin/Rhodion ring raise thirteen EXTI interrupts whose handlers merge them so one particle
counts once. At every frame boundary — a compare on TIM2, which counts the segment's rung — the
firmware reads the counters — cleared on the second's boundary only, so every value is the
running total since the second began (`BUS.md`) — subtracts filtered from less-filtered where the payload
mode says so, corrects for temperature, and puts four `uint16` into the 8 B mini payload. It
also holds the ~400 V module on and watches it. Nothing else: no energy, no status byte, no
spectrum (`BUS.md`).

## A2. Boot

| # | step | what happens | if it fails |
|---|---|---|---|
| 1 | reset | `HV_EN` (PE6) low, `DE` low; the IWDG at **1 s** | — |
| 2 | RC | **there is no crystal on this board** — the HSI runs the core and the UART through PLL1 (`M` 8, N 33 + `FRACN`, SYSCLK 2²⁷), free-running for now and disciplined by the rung from step 8 on (§A5); the flash cells read (§A9): the set, the payload mode, the tube type, the temperature coefficient, the HV setpoint window; the tag computed — CRC-16-CCITT over the 96-bit UID | a set that fails its check is no set |
| 3 | the rail | `PGOOD` (PD14) read | low: `HEALTH` *rail*, `FAULT` up |
| 4 | IDs | ADC1 reads `ID_D`, `ID_P` once, ratiometric | a ratio in no window is *unknown board*; the node runs |
| 5 | the counters | TIM3, TIM8, TIM1, TIM15, TIM12 in external clock mode 1 on their CH1, input filter **4 samples of 2²⁷** (30 ns — a GM pulse is microseconds), counters cleared; the thirteen EXTI lines armed on the rising edge | — |
| 6 | the thermometers | the five NTC dividers read once on ADC1, `NTC_EN` high for the conversions, 16× oversampled — a divider at full scale is an open pair, and the answers are the map of which head has its own; a head without one uses the board's. I2C1: the `INA238` configured; `ALERT` (PB13) an EXTI on the rising edge — high is the alarm, the 100 kΩ holds it low | not one answers: `HEALTH` NO_RESPONSE *thermometer*, the correction held at 1; one of several silent: the channel falls back to the board's sensor and `HEALTH` DEGRADED *thermometer* |
| 7 | HV | `HV_EN` high; `HV_MON` (PC2) read every 100 ms until it enters the persisted window, **2 s** at most | not in window: `HV_EN` low, `HEALTH` NO_RESPONSE *HV*, the node enrols and ships zeros |
| 8 | the rung | the segment's rung on `ETR` (PA15) counted against the core for 10 ms: 2¹⁹ ± 3 %, the mini tier's one rung, and **the ratio sets `HSITRIM` coarsely**, landing the HSI inside `FRACN`'s pull range, stepped again whenever `FRACN` nears the edge of its window; the `FRACN` loop then takes over and runs for the board's life (§A5) | no edges: the core holds at its last code, silent, waiting |
| 9 | enrol | §A4 | — |

## A3. The states

```
   RESET ──▶ RC (38 400, silent) ──▶ ENROLLED ──▶ LOCKED (TIM2 on the rung) ──▶ RUNNING
                 ▲                                          │                        │
                 │◀── rung lost ────────────────────────────┴────────────────────────┤
                 │◀── IWDG reset ────────────────────────────────────────────────────┤
                 └── ENDED (END: the parts stop, the buffer is flushed and answered; then the rung and the feed go)
```

`RC` — the HSI clocks the core, the USART at 38 400, transmitting nothing; the tubes count
into their counters and the counts are discarded. `ENROLLED` — NUMBER and slot held. `LOCKED` —
`BUSCFG` taken, TIM2 counting the rung, the USART on the data rung, no frame sent. `RUNNING` —
`SYNC` loaded the index; a bin every frame. `ENDED` — on `END`: `HV_EN` low (the tubes are the
draw), the counters stopped and the last bin answered, the USART listening; if the node is
brought back HV comes back through step 7 of §A2. **Rung lost** is a fault: TIM2's `ETR` stops advancing — detected
by the core-clocked TIM6 seeing no frame compare for 2 periods — the node mutes and comes
back through the rejoin (§A4).

## A4. The bus — the unit's side, on mini

**The enrolment** is the node contract on a mini segment: `DISCOVER` at 38 400 from the Argus
answered once with the tag in the time bytes after `tag × 10 µs`, listening first on
`RXD_ECHO`, backing off on a bad echo; `ASSIGN_ADDR` by tag gives NUMBER and slot, `ACK`;
`GET slots` answers **1**. `BUSCFG` (count, rate code 7, rung) — the **data rung** 2²⁰ alone or
2²¹ chained goes to the USART; the **sync rung** already on `ETR` is what TIM2 counts, and the
frame period is that rung ÷ 128 — **4096 ticks at 2¹⁹**. The deadline is `round
start + (slot − 1) × width`, the width `4 × 16 B × 10 bits / data rung` — 610 µs at 2²⁰, 305 µs
at 2²¹ — in rung ticks. `SYNC`, on a second's boundary, is captured on TIM2 CH2 (PB3) and loads
`unix.0 · frame` at that edge less `ROUTE` — the written route, 2²⁷ ticks scaled to the rung by
a shift — so the grid is the Argus's (`../../core/PROTOCOL.md` §7); TIM2 CH3's compare train starts
from that origin, one compare per frame,
and **that compare is the bin boundary**.

**The slot.** TIM2 CH4's compare fires the transmit of the 16 B frame: `DE` up, USART1's DMA,
`DE` down after the last stop bit; the echo on USART3 checked; a miss counted in `HEALTH` and
the frame held in the circular buffer of `DELAY` frames. **The gaps:** `GET`, `SET`, `RESEND`, `HARD_RESET`
in the node's block, as §A7. **Ranging** on `SET RANGE` is TIM4 on the disciplined 2²⁷, the house
turnaround at `CCR` = **256 ticks**, as on every H523 unit (`../../quake/FIRMWARE.md` §4).

**The rejoin** after any reset: rung and a persisted set → TIM2 on the rung, the USART on the
data rung, listen; the phase from a neighbour's frame or the Argus's `TICK`, captured on TIM2
CH2; the node fires in the next round with `status` 1 REJOINED. The counters are cleared at
the phase edge, so the first second is a partial one under that status. No rung → RC, silent. No set → 38 400, silent, a
sweep.

**`END`**: `HV_EN` low, the counters stopped, the last bin flushed and the answer sent, the
USART listening for as long as the rung lasts; the node then stops on its watchdog when the rung
drops and **the returning feed is a boot**. No rail is switched; HV is a part's own enable, not
a rail (`../../galvani/README.md`, *`ENABLE`, `LINE_EN` and the three states*).

## A5. Time on the node

**TIM2 counts the rung, not the core.** External clock mode 2 on `ETR` (PA15): the counter
advances on every edge of the segment's sync rung, so a tick is the wire's and the core is
out of the time path. A bin boundary is a compare on that counter, 2ⁿ ticks apart; `frame`
increments at each; `unix.0` every 128th. The core clocks the USART and the ranging timer, and it is
the rung's own: **the discipline loop** — `TIM5` counting SYSCLK, latched by TIM2's frame
compare, must read 1 048 576 a frame (0,95 ppm a count, 0,0075 ppm on the one-second gate);
the error is integrated into `FRACN` with the PLL running and dithered between two codes where
it is finer than one; `HSITRIM` is set at bring-up by measurement, never by assuming the curve rises, stepped off the
multiples of 32, and stepped one code toward the centre whenever `FRACN` nears the edge of its window — a land board drifts more over a year than `FRACN` alone can pull (`HARDWARE.md`, *The rung disciplines the core*). **No holdover
table**: a lost rung mutes the node, the loop freezes on its last code, and on the rejoin it
closes on the one-frame gate before the first frame goes out.

**The thirteen-tube merge** reads TIM2 at every EXTI edge — a 1,9 µs grain at 2¹⁹ — and
counts one neutron when one or more tubes fire within a **window of 16 µs** of the
first: the arrival times are compared, not the pulses. A window opens on any edge, closes at
16 µs, and every edge inside it is the same particle. The handler is 30 cycles.

## A6. Acquisition — one bin

Once a frame, in the TIM2 CH3 compare interrupt, within 1 µs of the boundary:

1. `K1..K4` = the four counters' `CNT`, read at every compare and **cleared at the second's
   boundary only** — each value is the running total since the second began, frame 127 carries
   the second (`BUS.md`); pulses in the microsecond of the clear fall into the neighbouring
   second, which no count rate notices; `K4` is the merge's count where
   the ring is fitted instead of a tube, the register `TUBE` saying which; with a tube, `K4A` —
   TIM12, the head's LLD — is read beside it and never enters the payload;
2. the temperature correction: `K × (1 + α × (T − 20 °C))`, α the per-tube coefficient in the
   cells in 2⁻¹⁶ per kelvin, default 0; `T` the channel's own thermometer where one answered at
   bring-up and the board's otherwise, every sensor read every 10 s, 16× oversampled. **A reading
   is believed only if it is plausible**: inside −45…+90 °C and within 1 °C of the previous one —
   a block of steel does not move faster — else it is dropped and the last one held; three drops
   in a row is `HEALTH` DEGRADED *thermometer n* and the channel falls back to the board's sensor;
3. the payload by mode — **raw**: `K1 · K2 · K3 · K4`; **derived**: `beta = K1 − K2`, `soft γ =
   K2 − K3`, `hard γ = K3`, `neutron = K4`, a negative difference clipped to 0;
4. four `uint16`, little-endian, into the 8 B payload; an unfitted tube's channel reads 0.

**The He³ threshold is watched, not moved.** With a tube on `K4`, the ratio `K4 / K4A` over the
last hour is where the neutron threshold stands on the tube's spectrum; a ratio that moves by
more than **20 %** against its persisted bench value is `HEALTH` DEGRADED *He³ threshold* — gas
gain, HV or a leaking tube, seen before the neutron count itself is wrong (`helion/HARDWARE.md` §4). Both
counts are readable in `HE3`.

**Dead time is not corrected on the node** — a GM tube's dead-time curve is the tube type's and
the archive applies it from `TUBE`; the node ships what it counted.

## A7. The control plane

| op | what the node does |
|---|---|
| `DISCOVER` · `ASSIGN_ADDR` · `BUSCFG` · `SYNC` · `TICK` · `RESEND` | as §A4 |
| `END` | §A4 |
| `HARD_RESET` | the NUMBER to 15, the mode and the tube type to the defaults, the coefficients cleared; then a reset |
| `GET reg` | a byte under `GET`; wider as `kind` 5 |
| `SET reg · value` | written, applied, `ACK`; a change of mode marks the first affected frame `status` 5 CHANGED |

| register | r/w | what |
|---|---|---|
| `0x0002 STATUS` | r | the node state, the rung state, the HV state, the ranging width last measured |
| `0x0010 MODE` | r/w | 0 raw · **1 derived** |
| `0x0011 TUBE` | r/w | the GM tube type code for K1–K3; the K4 detector: 0 none · 1 He³ · 2 BF₃ · 3 Gadolin ring · 4 Rhodion ring |
| `0x0012 ALPHA` | r/w | four × int16, the temperature coefficients in 2⁻¹⁶/K, `kind` 5 |
| `0x0013 HV` | r/w | bit 0 on/off; the monitor window, two × uint16 in raw ADC counts |
| `0x0014 WINDOW` | r/w | the merge window in rung ticks, default the value of 16 µs at the learned rung |
| `0x0021 RANGE` | w | arms the ranging turnaround, `arg1` the width in ticks |
| `0x0031 HV_MON` | r | the module's monitor, raw ADC counts |
| `0x0032 HE3` | r/w | with a tube on `K4`: the last second's `K4A` and `K4`, the hour's ratio, and the bench ratio it is held against — the bench writes that one; `kind` 5 |
| `0x0038 REPORT` | r | answers with the `REPORT` frame itself, `kind` 6: `VBUS` · `CURRENT` raw · `NTC_B`, the board's NTC, int16 in 0,01 °C · 0 |
| `0x003A HEALTH` | r | answers with the `HEALTH` frame, `kind` 8: the MCU die temperature, the thermometer in use per channel — a channel on the board's sensor reading it — the HV monitor, the error counters |
| `0xFF00`–`0xFF10` | r | `VERSION` · `IDENT` · `TAG` · `slots` 1 · `HEALTH` (echo/CRC misses, resends, rung losses, IWDG resets, HV excursions, merge windows opened) · `SENSORS` (bit 0–3 the four inputs' presence as configured, bit 4 a thermometer answered, bit 5 HV in window, **bit 8–11 which channel has a thermometer of its own**) · `ID` |
| `0xFF0A VIN_WINDOW` | r/w | the supply window, the SUPPLY flag outside it — the house register (`../../quake/FIRMWARE.md`) |
| `0xFF0B REPORT_INTERVAL` | r/w | seconds between `REPORT` frames, default 60; 0 disables |
| `0xFF13 DELAY` | r/w | the frames the unit holds before it sends — written by its Argus at floor-up, 16, and persisted; never below 8, the image's floor, because `RESEND` is answered from this buffer (`../../core/PROTOCOL.md` §7) |

**Up the link unasked:** `ACK` after a critical command and the `REPORT` frame once a minute — nothing else. `ERROR` only answers a command that failed; a fault is the header's `FAULT` flag, and its detail waits in `HEALTH` for the head's `GET HEALTH` (`../../core/PROTOCOL.md` §1, §5, §7). A GM channel whose count
is zero for **10 minutes** with HV in window is `HEALTH` DEGRADED, `FAULT` up — background never reads zero
that long.

## A8. Faults

| trigger | action | reported |
|---|---|---|
| `HV_MON` out of window for 1 s | `HV_EN` low, 5 s, `HV_EN` high; three times in an hour and it stays off until `SET HV` | `HEALTH` NO_RESPONSE *HV*, `FAULT` up |
| a counter reads over 60 000 in a second | the value is clipped at 65 535 and `status` 6 CLIPPED rides the frame | `HEALTH` |
| the rung lost | mute; the rejoin | `status` 1 REJOINED |
| `ALERT` high on PB13 | the reading latched into `HEALTH` | `FAULT` up |
| the IWDG expires | reset | `HEALTH` |

## A9. Persistence

The H523's flash, in the node contract's append-only cells (`../../core/PROTOCOL.md` §7).

| cell | content | written |
|---|---|---|
| the set | NUMBER, slot, `BUSCFG`, one check | at enrolment |
| the mode, the tube type | `MODE`, `TUBE` | on `SET` |
| the coefficients | `ALPHA`, `WINDOW`, the bench ratio in `HE3` | on `SET` |
| the HV window | `HV` | on `SET`; `HARD_RESET` keeps it — it is the module's, not the site's |

## A10. Budget and tests

USART1 · USART3; TIM2 on the rung; TIM3 · TIM8 · TIM1 · TIM15 · TIM12 counting; TIM4 ranging; TIM6 the
rung watchdog; fourteen EXTI; I2C1; ADC1 — under 2 % of the core at the ring's worst rate, a
few thousand interrupts a second; `WFI` otherwise. **On a PC:** the merge against a synthetic
ring pattern — thirteen edges within 10 µs counting once, two particles 20 µs apart counting
twice; the derived arithmetic and the clip; the bin read at the compare; the register map; the
cells. **On the bench:** a pulse generator into each timer input counted to the pulse over
10⁶ pulses; the merge on a ring with a source; HV in window within 2 s of enable; the node in
its slot at 128 Hz for 24 h with zero fillers on a bench segment; an `END` and return.

---

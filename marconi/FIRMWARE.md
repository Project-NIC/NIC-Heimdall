★ N.I.C. ★

# Marconi — the firmware, described

> **This document is the deliverable, not a description of code.** It says everything the
> node's firmware does, in the order it does it, with the numbers it uses — enough to write the
> build from, and enough to test the build against. What the unit *is*: [`README.md`](README.md); the band
> and the targets: [`BAND.md`](BAND.md); the board — the front end, the clock chain, the burst,
> the pins and the timers: [`HARDWARE.md`](HARDWARE.md); the record: [`BUS.md`](BUS.md); the frames, the opcodes and the node contract:
> [`../core/PROTOCOL.md`](../core/PROTOCOL.md); the clock and the bus start:
> [`../core/blocks/nodbus.md`](../core/blocks/nodbus.md); the record's shape:
> [`../tesla/BUS.md`](../tesla/BUS.md), and the SID conventions it shares: [`../tesla/pip/FIRMWARE.md`](../tesla/pip/FIRMWARE.md) §6. Where this document
> and one of those differ, that one wins and this one is corrected.

## 1. What the firmware is

**A burst, a transform, a few bytes.** Every revisit the firmware captures **524 288 samples** —
7,81 ms at 2²⁶, 1 MB, one gapless DMA transfer off the PSSI — runs one
transform over them, reads the level of every monitored carrier out of its bin, and ships
one 4 B record per carrier. Beside that a slow background sweep promotes any tone that persists
an hour to a monitored slot, a product test keeps distortion products off that list, and the
ionogram task catches the chirp sounders as they sweep the band and scales the trace to
`foF2`, `h'F` and `MUF(3000)`. Nothing is demodulated and nothing is stored: **134 MB/s inside
the box, a few bytes a second on the wire** (`BUS.md`).

| the firmware does | on | how often |
|---|---|---|
| captures a burst | PSSI 16-bit, DMA into AXI SRAM | every **revisit**, 10–30 s |
| transforms it | the M7, in place | per burst, ~0,3 s |
| reads the carriers | the transform | per burst, ≤ 8 carriers |
| sweeps the background | the same transform | every burst; promotion after **1 h** of persistence |
| catches a chirp | 65 536-sample bursts, 1 ms every **100 ms** while a sounder is due | per the sounder table |
| ships records | USART1, one TIM2 compare a round | up to 8 records a frame; a carrier record per carrier per revisit, the floor once a second |
| watches the converter | `OR`, SPI3 | per burst |
| sends the report frame | USART1, in its own block behind its data: `VBUS` · `CURRENT` raw from the `INA238` · the board's NTC, int16 in 0,01 °C · 0 | once a `REPORT_INTERVAL`, 60 s (`../core/PROTOCOL.md` §5) |

## 2. Boot

| # | step | what happens | if it fails |
|---|---|---|---|
| 1 | reset | `CSB` high, `PDWN` (PG2) high, `SYNC` (PG3) low, the amplifiers' `PD` (PG7) high, `DE` low; the IWDG at **1 s**; the MPU set: the burst buffer in AXI SRAM non-cacheable, the transform's state in DTCM | — |
| 2 | RC | HSI; the flash cells read (§11): the set, the carrier list, the strong list, the revisit, the sounder table, the equalisation curve; the tag computed — CRC-16-CCITT over the 96-bit UID | a set that fails its check is no set: the node boots as fresh |
| 3 | the rails | `PGOOD` (PH2) read; the 1,8 V is the processor's own | low: `HEALTH` *rail*, `FAULT` up |
| 4 | the converter, found | SPI3: `CHIP_ID` (0x01) read — the AD9265's; anything else and the node enrols with `HEALTH` NO_RESPONSE for the front | — |
| 5 | the converter, configured | `PDWN` low; over SPI3: the output in **parallel CMOS, twos complement**, the clock divider **1**, `DCO` **enabled** — it is the PSSI's read clock — the test pattern **off**, the internal reference; every register read back; then one `SYNC` pulse so the pipeline's phase against the encode edge is known | a read-back mismatch: `PDWN` pulsed, the sequence repeated once, then `HEALTH` DEGRADED and the node runs the bus alone |
| 6 | the clock | the data body's `CLK` on `OSC_IN` validated — N clean periods at 2²² against the HSI — and **not taken** until `BUSCFG`; the encode is not started off the HSI: the converter is never encoded from a ±1 % source | no clock: RC, silent, waiting |
| 7 | the amplifiers | `PD` low on the `LMH5401` and the `THS4541`; **500 ms** of settling before the first burst | — |
| 8 | the PSSI | 16-bit receive, `DE`/`RDY` disabled, one DMA linear transfer of 524 288 half-words; one burst of the converter's checkerboard test pattern captured and every line checked | a stuck line is a wiring fault: `HEALTH` DEGRADED *bus*, the line named |
| 9 | IDs and the power body | ADC1 reads `ID_D`, `ID_P` once, ratiometric against the 1,8 V. Then I2C1: the `INA238` on `PWR IN`'s board at 0x40 configured — `APOL` = 1, `ALERT` high on alarm and latched; PC8 on EXTI, rising edge | a ratio in no window is *unknown board*; the node runs. No answer where `ID_P` names a power board: `HEALTH` *no telemetry*, `FAULT` up |
| 10 | enrol | §4 | — |

## 3. The states

```
   RESET ──▶ RC (38 400, silent) ──▶ ENROLLED ──▶ LOCKED (HSE, ENC running) ──▶ RUNNING
                 ▲                                          │                            │
                 │◀── clock lost (CSS) ─────────────────────┴────────────────────────────┤
                 │◀── IWDG reset ────────────────────────────────────────────────────────┤
                 └── ENDED (END: the parts stop, the buffer is flushed and answered; then the clock and the feed go)
```

`RC` — the HSI, transmitting nothing. `ENROLLED` — NUMBER and slot held, still at 38 400.
`LOCKED` — `BUSCFG` taken, PLL1 at 2²⁸, `ENC` at 2²⁶ into the converter, the amplifiers
on, no burst taken yet. `RUNNING` — `SYNC` loaded the index; bursts run on the revisit and
records go up. `ENDED` — on `END`: `PDWN` high, the amplifiers' `PD` high, the last record flushed and
answered, `ENC` kept, the USART listening for as long as the clock lasts; if the node is brought
back the first burst follows the amplifiers' 500 ms. **Clock lost** is a fault: the CSS raises an NMI, the node
drops to the HSI, mutes, `PDWN` high (the converter is not encoded by a dying PLL), and comes
back through the rejoin (§4).

## 4. The bus — the unit's side

**The enrolment** is the node contract, as on every unit: `DISCOVER` at 38 400 answered once
with the tag in the time bytes after `tag × 10 µs`, listening first on `RXD_ECHO`, backing off
on a bad echo; `ASSIGN_ADDR` by tag gives NUMBER and slot, `ACK`; `GET slots` answers **1** —
a handful of carriers at 4 B each is one slot (`BUS.md`). `BUSCFG` (count, rate code 7,
rung) locks HSE onto the wire, PLL1 to 2²⁸, the USART to the rung, the PSSI clocked; the
deadline is `round start + (slot − 1) × width`. `SYNC`, on a second's boundary, is captured on
TIM2 CH2 (PB3) and loads `unix.0 · frame` at that edge less `ROUTE` — 2 × the written route in
ticks of 2²⁸ — so the grid is the card's (`../core/PROTOCOL.md` §7).

**The slot.** A TIM2 compare starts USART1's DMA transmit of the 40 B frame the emission stage
assembled (§7); `DE` up for the frame, down after the last stop bit; the echo on USART3 (PB11)
checked when it ends; a miss counted in `HEALTH` and the frame held in the circular buffer of `DELAY` frames.

**The gaps:** `GET`, `SET`, `RESEND` and `HARD_RESET` in the node's block, as §8. `TICK`
ignored while running; the phase source on a rejoin. **Ranging** on `SET RANGE`: after the next
DATA frame PB6/PB7 switch to TIM4, `DE` up, one-pulse mode — CH2's capture starts the counter,
CH1 raises the return at `CCR` = **512 ticks** of 2²⁸ (1,91 µs) and drops it at the width the
card asked for; CH2 in PWM-input mode captures the incoming pulse's width for `STATUS`; the pins
go back. No interrupt in the path.

**The rejoin** after any reset: clock and a persisted set → HSE locked, the USART on the rung,
listen; a good CRC proves the rung, a second of nothing tries the other; the phase from a
neighbour's frame or the card's `TICK`, captured on TIM2 CH2; the node fires in the next round
with `status` 1 REJOINED. The burst schedule restarts from the phase edge. No clock → RC,
silent. No set → 38 400, silent, a sweep.

**`END`**: `PDWN` high, `PD` high, the last record flushed and the answer sent. The USART keeps listening until the clock goes, so a CONTROL frame addressed to the node
brings it back if the shutdown is called off; otherwise the node stops on its watchdog when
the clock drops and **the returning feed is a boot**. No rail on the board is switched
(`../galvani/README.md`, *Three states*).

## 5. Time on the node

**One 32-bit timer, never reset.** TIM2 counts 2²⁸ from PLL1 and is read as differences: CH2's
capture of every start-bit edge on `RXD` (the phase source), the compare that fires the
transmit, a compare every 2²¹ ticks that advances `frame`. **A burst starts on a TIM2 compare**,
and the MDMA transfer is armed ahead of it so the first word read is the sample encoded at that
edge, within the PSSI's fixed read latency — a fixed count of kernel clocks from the reference
manual, subtracted as part of **`LAT`** (`../core/PROTOCOL.md` §7). So every burst has an index `n₀` on the node's grid: `unix.0 · frame ·
tick`, tick in 2²⁸, and every sample of it is `n₀ + i × 4`.

**The chirp needs the absolute second and gets it from the frame.** A sounder starts its sweep
at a scheduled UTC instant; the node's time is the card's second to ±1 µs, so the group delay
of a received chirp is measured against the schedule directly (§6), and no local reference is
needed. The converter's own latency — the AD9265's **9 encode clocks** from its datasheet, **134 ns at
2²⁶**; its 1,0 ns aperture delay is below a tick and not counted — and
the front end's group delay from the computed chain are in the same constant, `LAT`, with the
PSSI's; the bench of §13 can confirm it and need not.

## 6. Acquisition — the burst, the transform, the carriers, the sweep, the chirp

**The burst.** The DMA reads 524 288 half-words from the PSSI into the 1 MB of AXI SRAM in one
linear transfer. Each half-word is the converter's 16-bit word, twos complement. `OR` (PG4) is
sampled by EXTI during the transfer: an over-range marks that burst *clipped*, its levels are still
read and the frame carries `status` 6 CLIPPED; three clipped bursts in a row is `HEALTH` DEGRADED
*headroom*, and nothing switches — the gain is fixed by `Rf1` at the factory (`HARDWARE.md` §9).

**The transform.** One real channel rides a **half-length complex transform**: the 524 288 real
samples are read as 262 144 complex points, transformed in place in the burst buffer, int16 block
floating point, twiddles in DTCM, and unpacked afterwards by the conjugate-symmetry identity — the
same 262 144-point transform and the same ~0,3 s of the core at 2²⁸ as the two-channel build, for
twice the frequency resolution. A **Hann** window is applied on the way in; the level of a tone is
the power of its peak bin plus its two neighbours, the window's loss folded into the equalisation.
**The bins are 128 Hz wide**, 131 072 of them across 0–2²⁴ Hz. The carrier record's frequency word
stays **256 Hz steps** — the bus format is the band top / 2¹⁶ and does not follow the transform — so
a slot maps to two bins and the level is read from the pair (`BUS.md`).

**The equalisation.** The chain's response — S2's knee at 16 MHz, the loop's self-resonance
lift, the window's loss — is one measured curve, held in the cells as **64 points
in 2⁻⁶ dB across the band** and interpolated per bin; it is written at the bench once per built
board (§13) and every level below is read through it.

**The carriers.** Up to **eight** monitored carriers, each a frequency in 256 Hz steps. Per
burst, per carrier: the equalised level of the bin, converted to the 14-bit log
scale in 2⁻⁶ dB against the absolute scale of the chain — the same dB domain as Tesla's records.
**One level per carrier**, from the one channel. The azimuth factor divides out of the day/night
ratio by itself — it is the same for both frequencies of a pair — and the bearing is computed from
the station's and the transmitter's coordinates, not measured (`CONSTRUCTION.md`). One
carrier record per carrier per revisit.

**The floor.** The median level across all bins from 0,5 to 16 MHz that hold no carrier of the
strong list — the wideband noise floor — once a second as a carrier record at frequency 0.

**The sweep.** Every burst the transform's bins are scanned for tones above the floor by
**20 dB**; a tone seen in every burst for **1 h** at a frequency stable to ±1 bin is a
candidate. A candidate passes the **product test** before it is promoted. It is a **suspect** if
it lies within ±2 bins of `2f`, `3f`, `2f₁ − f₂` or `2f₂ − f₁` of any two entries of the **strong
list** — the sixteen loudest persistent tones the node keeps for this purpose alone, whether
monitored or not — and a suspect is **rejected when its level follows its parents'**: over the
hour's bursts its level against the product's predicted level — `2·L` for `2f`, `3·L` for `3f`,
`2·L₁ + L₂` for a third-order pair, in dB — correlates above **0,9** with a slope between 0,7 and
1,3. A distortion product moves two or three decibels for each of its parents' by construction;
a transmitter on a harmonic frequency fades on its own path and does not. A frequency the head
has written into `CARRIERS` is never tested. A candidate that passes takes the first free monitored slot; with all eight
taken it is reported and not slotted. The **day/night pairs** are the head's to declare: a `SET`
marks two slots as one transmitter, and the node ships both levels and does nothing more — the
ratio is the archive's (`BAND.md`, *The targets*). Broadcast AM is not
excluded by the node; the head removes what it does not want.

**The chirp.** The sounder table (§8) names each sounder the site hears: its start instant within
the hour, its rate, its start and stop frequency. When a sounder is due the node runs
**1 ms bursts every 100 ms** — 65 536 words, the same MDMA path — for the length of the sweep;
in each burst the signal is mixed down at the frequency the schedule predicts for the direct
path at that instant, decimated by 512 to a 128-sample record of a 65 kHz window, transformed,
and the strongest tone's offset from zero, divided by the sounder's rate, is the **group delay
at that frequency** — 100 Hz of offset at 100 kHz/s is 1 ms, and the peak is interpolated to a
hundredth of a bin. The chirp moves under 100 Hz across the 1 ms burst, so it is stationary
within it (`HARDWARE.md` §8). The sequence of (frequency, delay) pairs across the sweep
is the oblique trace; it is scaled on the node:

| parameter | how it is read |
|---|---|
| **`MUF(3000)`** | the junction frequency where the one-hop trace ends, converted from the path's length (the table's) to the 3000 km standard by the secant law |
| **`foF2`** | `MUF` divided by the path's obliquity factor, the same secant law |
| **`h'F`** | the minimum group delay of the one-hop trace, converted to a virtual height by the path geometry |

Each is one `IONOGRAM` record (type 1) with its parameter id; a sweep whose trace has fewer than
**16** valid points ships nothing and counts in `HEALTH`. **The scaler is the simplest one that
gives three numbers** — no O/X separation, no multi-hop, no true-height inversion; the trace
itself, as (frequency, delay) pairs, answers `GET` for a reader that wants more. The sounder table for a site comes from the commissioning survey.

**The long record — the `LONG` option, on a board with the second PSRAM** (`HARDWARE.md` §10).
`SENSORS` bits 2 and 4 report the memories; with one, `LONG` is refused. With two, a `LONG` revisit
replaces the burst by a raw capture of **2 097 152 samples at fs 2²⁶, 31,25 ms, 32 Hz bins,
60,2 dB of processing gain** — no half-band and no decimation, Nyquist stays at 2²⁵.

*The capture.* The PSSI's DMA fills a ring of four 64 kB blocks in AXI SRAM, one block every
0,49 ms; as each block lands an MDMA channel moves it to one of the two memories, blocks
alternating — rows 16k…16k+15 of the record in memory k mod 2 — so each OCTOSPI writes 64 kB in
every 0,98 ms, half its width, and the AXI SRAM never holds more than 256 kB of it. Sixty-four
blocks, and the record stands as a 1024 × 1024 matrix of 4 B complex points: the 2²¹ real samples
read as 2²⁰ complex, a row 4 kB, 4 MB in all.

*The transform.* A 2²⁰-point transform cannot run in place out of a memory that takes no strides,
so it runs as a **four-step FFT with block transposes**, every access to the PSRAM a sequential
run of at least 256 B:

| pass | reads | does | writes |
|---|---|---|---|
| 1 | 64 rows at a time, 256 kB, from the record | the Hann window; transposes in AXI SRAM into 1024 column segments of 64 points | the segments, 256 B each, into a transposed copy — 4 MB more, across the two memories |
| 2 | each column of the copy, 4 kB contiguous | a 1024-point FFT in DTCM, then the twiddle `W_N^(n1·k2)` | the column back in place |
| 3 | 64 columns at a time | the transpose back | into the record region, rows contiguous again |
| 4 | each row, 4 kB | a 1024-point FFT in DTCM | the row in place — bin `k = k2 + 1024·k1` sits at row `k2`, column `k1` |

Sixteen megabytes read and sixteen written over two buses, 2048 sub-transforms of 1024 points out
of DTCM, about **0,5 s of the core** per long record against the burst's 0,3 s. The reader unpacks
the real signal's half-length trick at the bin — `X[k]` and `X[N−k]`, two reads — and a carrier's
256 Hz slot is eight bins; the levels, the sweep and the floor are then read exactly as from the
burst. The record and its copy are gone at the next capture; nothing in the PSRAM is persistent.

**The thermometer**, an NTC on `ADC1` behind `NTC_EN`, is read every 10 s; the temperature rides the `REPORT` frame and corrects
nothing — the equalisation is a bench curve at room temperature and the band's slow
temperature drift is the archive's to see.

## 7. Emission — what goes on the wire

**The rule** (`BUS.md`): Tesla's record shape read in Marconi's context — up
to eight polymorphic 4 B records in the 32 B payload, each word little-endian, the type tag in the low two
bits of word B.

| type | word A | word B |
|---|---|---|
| 0 CARRIER | the frequency, 16-bit, 256 Hz steps | the 14-bit log level `<< 2 \| 0` |
| 1 IONOGRAM | the value, uint16 — kHz for `foF2` and `MUF(3000)`, km for `h'F` | the parameter id `<< 2 \| 1`: 1 `foF2` · 2 `h'F` · 3 `MUF(3000)` |

**Filling the frame**, once per period, before the slot compare: records are taken from the
record queue in order — the floor first when it is due, then the carriers of the last burst,
then the ionogram's — eight to the payload, zeros in empty slots. A burst's eight carriers fill
one frame; the queue never holds more than a revisit's worth. **No count, no `Vin` byte**: an
empty slot is zero and the supply rides the `REPORT` frame.

**The cadence on the wire** is a revisit's records every 10–30 s, the floor once a second and
three parameters per sounder sweep — under 40 B a second on a link that carries 5 kB.

## 8. The control plane

| op | what the node does |
|---|---|
| `DISCOVER` | answers with its tag, §4 — only when not in a running round |
| `ASSIGN_ADDR` by tag | takes NUMBER and slot, writes the cells, `ACK` |
| `BUSCFG` | takes the item; on the third, locks and computes the deadline |
| `SYNC` | loads the index at the captured edge, starts the burst schedule |
| `TICK` | ignored while running; the phase source on a rejoin |
| `END` | §4 |
| `HARD_RESET` | the NUMBER to 15, the carrier and strong lists cleared, the revisit and the sounder table to their defaults; **the equalisation curves are kept** — they are the board's, not the site's; then a reset |
| `RESEND frame · unix.0` | the frame from the circular buffer, in the node's own slot |
| `GET reg` | a byte under `GET`; wider as `kind` 5 |
| `SET reg · value` | written, applied, `ACK`; a change to the carrier list or the revisit marks the first affected frame `status` 5 CHANGED |

**The registers** — the house block and the house positions are the house map's
(`../core/blocks/modbus.md`, *The house map*):

| register | r/w | what |
|---|---|---|
| `0x0002 STATUS` | r | the node state, the clock state, the converter state, the last burst's clip flags, the ranging width last measured |
| `0x0010 REVISIT` | r/w | seconds between bursts, **10**–30, default 20 |
| `0x0011 CARRIERS` | r/w | the monitored list, 8 × uint16 frequency in 256 Hz steps, 0 = free; a pair flag per slot, `kind` 5 |
| `0x0012 STRONG` | r | the strong list, 16 × uint16, `kind` 5 |
| `0x0013 SWEEP` | r/w | bit 0 the sweep on (default 1); the promotion time in minutes, default 60 |
| `0x0014 SOUNDERS` | r/w | the sounder table, one entry per `SET`: index · start second in the hour · rate in Hz/s · start and stop frequency in kHz · path length in km — 12 B, `kind` 5; up to 8 |
| `0x0015 TRACE` | r | the last sweep's (frequency, delay) pairs, up to 64 × (uint16, uint16), `kind` 5, paged |
| `0x0016 EQ` | r/w | the equalisation curve, 64 × int16 in 2⁻⁶ dB, `kind` 5, paged; written at the bench |
| `0x0017 LONG` | r/w | 1 runs the long record in place of the burst (§6); refused on a board without the second PSRAM |
| `0x0021 RANGE` | w | arms the ranging turnaround for the next block, `arg1` the width in ticks |
| `0x0023 SELFTEST` | w | 1 runs the converter's test pattern (§9) |
| `0x0038 REPORT` | r | answers with the `REPORT` frame itself, `kind` 6 |
| `0x003A HEALTH` | r | answers with the `HEALTH` frame, `kind` 8: the MCU die temperature, the converter's and the amplifiers' state, the error counters |
| `0xFF00 VERSION` | r | firmware version |
| `0xFF01 IDENT` | r | the house code |
| `0xFF02 TAG` | r | CRC-16 of the UID |
| `0xFF03 slots` | r | 1 |
| `0xFF04 HEALTH` | r | echo/CRC misses, resends served, clock losses, IWDG resets; bursts, clipped bursts, gapless failures, candidates rejected by the product test, promotions, sweeps scaled and sweeps dropped |
| `0xFF09 SENSORS` | r | the present-mask: bit 0 the converter · bit 1 the amplifiers · bit 2 the PSRAM · bit 3 the thermometer · bit 4 the second PSRAM, the `LONG` option |
| `0xFF0A VIN_WINDOW` | r/w | the supply window, the SUPPLY flag outside it — the house register (`../quake/FIRMWARE.md`) |
| `0xFF0B REPORT_INTERVAL` | r/w | seconds between `REPORT` frames, default 60; 0 disables |
| `0xFF10 ID` | r | the two `ID` codes read at boot |
| `0xFF13 DELAY` | r/w | the frames the unit holds before it sends — written by its parent at floor-up, 32 on a Bifrost port and 16 behind an Argus, and persisted; never below 8, the image's floor, because `RESEND` is answered from this buffer (`../core/PROTOCOL.md` §7) |

**Up the link unasked:** `ACK` after a critical command and the `REPORT` frame once a minute — nothing else. `ERROR` only answers a command that failed; a fault is the header's `FAULT` flag, and its detail waits in `HEALTH` for the head's `GET HEALTH` (`../core/PROTOCOL.md` §1, §5, §7).

## 9. Health and self-test

**`HEALTH` per position**, the house vocabulary: OK · SELFTEST_FAIL · NO_RESPONSE · DEGRADED, read
on `GET HEALTH`, the `FAULT` flag up while any entry is not OK. The
converter's self-test is its own **test pattern**: on `SET SELFTEST` the node switches the
converter to its checkerboard pattern, captures one burst, checks every word, and switches
back — `SELFTEST_FAIL` names a stuck data line by number. A burst that reads a flat zero or a
flat full scale with the amplifiers on is `DEGRADED`; a `PD` that does not change the noise
floor by 10 dB is a dead amplifier and `NO_RESPONSE` for the front.

**The gapless check runs on every burst**: the transfer's word count and the MDMA's completion
time against the expected 7,81 ms ± 1 µs; a short or slow transfer is a burst discarded and a
count in `HEALTH`, and ten in an hour is `HEALTH` DEGRADED *bus*.

## 10. Faults

| trigger | action | reported |
|---|---|---|
| a burst discarded (§9) | the revisit continues; no record from it | `HEALTH` |
| the converter's read-back mismatches after boot (checked once an hour) | reconfigured; a second mismatch is `DEGRADED` | `HEALTH` *converter registers*, `FAULT` up |
| `OR` on three bursts running | nothing switched; the levels ship flagged | `HEALTH` DEGRADED *headroom* |
| a tone on the strong list disappears for a day | dropped from the strong list; a monitored slot is never dropped by the node | — |
| the CSS fires | NMI: HSI, `PDWN` high, mute; the rejoin of §4 | `status` 1 REJOINED |
| `ALERT` on PC8 (EXTI) | the `INA238`'s reading latched into `HEALTH`; nothing to switch on this board — the source end acts on its own `ALERT` | `HEALTH`, `FAULT` up |
| `PGOOD` low | noted | `HEALTH` *rail*, `FAULT` up |
| the IWDG expires | reset; the converter reconfigured by the boot of §2 | `HEALTH` |

## 11. Persistence

The H7A3's flash, in the node contract's append-only cells (`../core/PROTOCOL.md` §7): a cell
is id, value, check; the last good cell per id wins; a full sector is rewritten; the live set
rewritten once a year.

| cell | content | written |
|---|---|---|
| the set | NUMBER, slot, `BUSCFG`, one check | at enrolment |
| the carriers | `CARRIERS`, with the pair flags | on `SET`; on a promotion |
| the strong list | `STRONG` | once a day |
| the revisit and the sweep | `REVISIT`, `SWEEP` | on `SET` |
| the sounder table | `SOUNDERS` | on `SET` |
| the equalisation | 64 × int16 | at the bench, over `SET`; `HARD_RESET` keeps them |

The PSRAM holds nothing persistent — the long record and the trace live there while they are
being worked and are gone at a reset.

## 12. The processor's budget

| resource | used | of |
|---|---|---|
| USARTs | 2 — USART1 the link, USART3 the echo | 10 |
| PSSI | 16-bit receive, `D0`–`D15` | 1 |
| SPI | 1 — SPI3 the converter's registers | 6 |
| OCTOSPI | 1 — OCTOSPI1 on port 2, the PSRAM; 2 with the `LONG` option | 2 |
| MDMA | 1 — the burst; 2 with the long record | 16 |
| DMA | 4 — USART1 TX, USART3 RX, ADC1, I2C1 | 16 |
| timers | TIM2 timebase (CH2 the capture, the slot and burst compares) · TIM3 CH2 the encode · TIM4 ranging · TIM6 the housekeeping second | |
| interrupts, by priority | 0 the RXD capture · 1 the slot compare · 2 the MDMA completion · 3 `OR` · 4 the USART idle lines · 5 SPI3 · 6 `ALERT` · 7 ADC1 · 8 TIM6 | |
| SRAM | the burst 1 MB in AXI SRAM · the transform's twiddles and state in DTCM, 96 kB · the chirp's decimated record · the lists · the circular buffer 32 × 40 B | 1,4 MB |
| PSRAM | the trace; with the `LONG` option the raw record 4 MB and its transposed copy 4 MB, across the two memories | 32 MB, 64 with the option |
| flash | the image · the cells 16 KB | 2 MB |
| CPU | the transform ~0,3 s per revisit — 1,5 % at 20 s, ~0,5 s where `LONG` runs; the sweep and the carriers ~10 ms; a chirp burst ~2 ms per 100 ms while a sounder is due; `WFI` otherwise | 2²⁸ |

**No interrupt sits in a timing path.** The burst starts on a compare and the MDMA reads
whatever the converter encoded from that edge; a late handler moves nothing.

## 13. What is tested, and where

**On a PC, with the pins replaced by callbacks:** the half-length real transform and its
unpacking against a synthetic burst of known tones; the Hann level read against a tone
placed between bins; the equalisation applied and reversed; the sweep's promotion at 1 h and
the product test against a strong list holding 5000 and 10000 kHz — a 15000 kHz candidate
fading on its own kept, the same 15000 kHz following 5000's at 3:1 rejected; the chirp's mix-decimate-transform against a synthetic sweep
with a 2,7 ms delay; the scaler against a recorded oblique trace; the record packing; the
register map; the cells.

**On the bench, against `HARDWARE.md`:** the burst gapless — the MDMA's word count and completion time on every burst, and the
converter's ramp pattern through all 524 288 words with no step missing; the checkerboard pattern
through every line; the PSSI latency constant reproduced to ±1 sample over a hundred bursts from
a tone on the encode edge; WWV 5000 · 10000 · 15000 read within ±1 dB against a calibrated
receiver over a day; the equalisation curve measured with a swept generator into the loop
port; one chirp sounder's trace scaled and compared with a published ionogram of the hour; the
node in its slot at 128 Hz for 24 h with zero fillers on a bench spur.

★ N.I.C. ★

# Pip — the firmware, described

> **This document is the deliverable, not a description of code.** It says everything the
> unit's firmware does, in the order it does it, with the numbers it uses — enough to write the
> build from, and enough to test the build against. What the unit *is*: [`README.md`](README.md); the band, the
> transmitters and the gating: [`CARRIERS.md`](CARRIERS.md); the board — Tesla's, identical:
> [`../HARDWARE.md`](../HARDWARE.md); the converter's bring-up, the ring and the ladder, which Pip
> runs exactly as Tesla does: [`../FIRMWARE.md`](../FIRMWARE.md) §2, §6; Kronos's
> side of the second socket — the source rank, the handover, the heartbeat:
> [`../../kronos/FIRMWARE.md`](../../kronos/FIRMWARE.md) §4; the frames, the opcodes and the node
> contract: [`../../core/PROTOCOL.md`](../../core/PROTOCOL.md). Where this document and one of those
> differ, that one wins and this one is corrected.

## 1. What the firmware is

**Tesla's board, a different image, two sockets.** Pip is a NodBus unit of **type 12** on `NB IN` —
it enrols, takes the station's clock, and ships the levels of the VLF and LF carriers it tracks
as carrier records, eight to a frame: the **SID channel**, the D-region rung of the station's
ionosphere ladder (Marconi carries the F-region, Sputnik the TEC). On the same carriers it decodes the
time — the time-code stations' minute frames, eLoran's pulse groups — and on **`TIME OUT`**, where a
Kronos is on the other end, it presents itself as a GNSS receiver: **PPS on the wire, NMEA on
the UART**. The two jobs are one signal path: a carrier whose level is a science product is the
same carrier whose phase is the time.

**Learning while GNSS is good, steering when it is gone.** The unit's grid is the station's
clock, which Kronos holds to GNSS while it has one. In that state Pip measures every carrier's
phase against a GNSS-true grid and learns the path — the offset per carrier, its daily shape.
When Kronos loses GNSS and says so, Pip places the second where the carriers and the learned
paths put it, and Kronos disciplines to that edge. Pip has no oscillator of its own to learn:
its grid is the wire.

| the firmware does | on | how often |
|---|---|---|
| streams the three rods | the SAI ring, as Tesla | 2²⁰ SPS, always |
| scans the band, tracks the carriers | the 2¹⁸ stage (0–131 kHz) | a scan every minute; a Goertzel per carrier over 1 s blocks; a phase tracker per carrier at 1 Hz loop bandwidth |
| ships carrier records | USART1, one TIM2 compare a round | one record per carrier per second; the floor once a second |
| decodes the time | the tracked carriers | a minute frame per time-code station; eLoran's GRI continuously |
| places PPS, sends NMEA | TIM2 CH1 (PA15) · USART2 on `TIME OUT` | once a second, while `TIME OUT` is served |
| watches `TIME OUT` | `ID_PIP`, Kronos's heartbeat | at boot and every second |

## 2. Boot

Steps 1–6 and 8 of Tesla's boot (`../FIRMWARE.md` §2) — the converter found and
configured, the clock validated and not taken until `BUSCFG`, the thermometers read — are
Pip's, unchanged. What differs:

| # | step | what happens | if it fails |
|---|---|---|---|
| 2 | RC | the cells (§11): the set, the carrier list, the transmitter table's site subset, the learned path offsets, the position, the crosstalk matrix, the µ(T) curves | a set that fails its check is no set: the node boots as fresh, the learning empty |
| 7 | IDs and the power body | ADC1 reads `ID_D`, `ID_P` **and `ID_PIP`** once, ratiometric; the power body as Tesla's step 7. `ID_PIP` at 1,00 is **no `TIME OUT`**: the unit is SID-only and step 7a is skipped. **0,40, 0,50 or 0,55 — a communication board with a channel B — arms `TIME OUT`**; any other window is a board `TIME OUT` does not take | 0,00 is a shorted pin, reported; a ratio in no window, or a board `TIME OUT` does not take, is *unknown board*; `TIME OUT` is not armed either way |
| 7a | `TIME OUT` | USART2 at 115 200 8N1 listening, `DE_PIP` (PE12) low, the PPS compare off; the unit waits for Kronos's heartbeat `$PNIC,HELLO*hh` (§8). Heard within **30 s**: `TIME OUT` is **served** — `DE_PIP` high, the PPS compare armed, NMEA from the next second. Not heard: `TIME OUT` is **timed out** — everything on it stays off, `HEALTH` *`TIME OUT` unanswered*, `FAULT` up, and the unit runs SID-only; a heartbeat arriving later serves it | — |
| 9 | enrol | §4, type 12, `GET slots` 1 | — |

**Nothing on `TIME OUT` can hang the unit.** The socket is served only by the heartbeat and dropped by
its absence; the NodBus side never waits for it.

## 3. The states

The NOD state machine of Tesla's §3 — `RC` → `ENROLLED` → `LOCKED` (the converter streaming,
the trackers running, nothing sent) → `RUNNING` · `ENDED` · the clock-loss fault — with one
state beside it for the socket:

```
   TIME OUT:  ABSENT (ID_PIP 1,00) · ARMED (a board, no heartbeat yet) · SERVED (heartbeat within 30 s) · TIMED OUT (30 s without one)
```

`SERVED` → `TIMED OUT` on 30 s without a heartbeat: `DE_PIP` low, the PPS compare off, NMEA
stopped, `HEALTH`, `FAULT` up. `TIMED OUT` → `SERVED` on the next heartbeat. The NodBus states run
regardless; an `END` stops the trackers and holds the socket as it is; the socket goes with the feed.

## 4. The bus — the unit's side

The node contract as on every H7A3 unit (`../FIRMWARE.md` §4): `DISCOVER` at 38 400 with
the tag, `ASSIGN_ADDR`, `GET slots` **1**, `BUSCFG` locking HSE onto the wire and PLL1 to 2²⁸,
`SYNC` on TIM2 CH2 zeroing the sample index `ROUTE` early, the slot on a TIM2 compare, the echo on USART3, the
gaps, ranging on TIM4 at `CCR` = 512 ticks of 2²⁸, the rejoin. Pip's own `TIME OUT` is ranged by
Kronos: `$PNIC,RANGE` on USART2 arms TIM15 on PA2/PA3 for 500 ms in one-pulse mode, the return
at 512 ticks (`../../kronos/FIRMWARE.md` §7).

## 5. Time on the node

**The grid is the wire.** TIM2 at 2²⁸ counts the station's clock; every sample has an index
since `SYNC`; a carrier's phase is measured against that index. While Kronos is `LOCKED` the
grid is GNSS-true to the station's ±1 µs, and a carrier's phase against it is the path's phase
plus nothing. While Kronos is in `HOLDOVER` the grid drifts at the TCXO's rate, and the same
measurement is the grid's error against the carrier — which is exactly what Pip hands back as
PPS. The head writes Kronos's quality byte into `QUALITY` (§8) on every change, so Pip knows
which of the two it is doing.

**The second on `TIME OUT` is a compare on TIM2 CH1 (PA15)**, one rising edge a second, 100 ms wide,
its position computed by §7 — never a GPIO write.

## 6. The carriers — the SID channel

**The band scan**, once a minute, on the 2¹⁸ stage: a 4096-point FFT over 15,6 ms, the bins
64 Hz wide, tones above the floor by 20 dB collected; a tone seen in every scan for **1 h** at a
frequency stable to ±1 bin is promoted to the carrier list, up to **eight**; the known
transmitters of the site's table (§7) are promoted on the first scan that shows them. A
candidate on `2f` or `3f` of a listed carrier is rejected as a product.

**Per carrier, two things run.** A **Goertzel over 1 s coherent blocks** (≈51 dB of processing
gain over the stage's 131 kHz) gives the level: 14-bit log in 2⁻⁶ dB, absolute, on the station's one dB
scale, corrected by the rod's µ(T) curve as Tesla's amplitudes are. And a **phase tracker** —
a narrowband loop at **1 Hz** bandwidth on the same stage — gives the carrier's phase against
the grid, the quantity §7 uses. The trackers coast across impulses: an envelope detector on
the 5–50 kHz product of the 2¹⁷ stage, Tesla's without its emission, arms on a needle above **4 × the background**, the
tracker's input mutes for the event and the loop carries on across the hole
(`CARRIERS.md`, *The noise floor is impulsive*). **A stroke that clips the chain arms it as well** —
the detector needs to know that something is far above the background, not how far — **and then the
mute runs until the chain has recovered, not for the needle's own length** — the recovery time
measured on the bench (`../HARDWARE.md` §0.4).

**A periodic source is blanked by prediction, not by detection.** The envelope detector also
estimates the fundamental the impulses are locked to — the site's mains, traction or rectifier
ripple, learned from the background as Tesla's classifier learns it (`../DETECTION.md`). Once
a fundamental and a phase hold for a few seconds the gate is **scheduled** on the next firing
instead of armed by it: the mute opens before the front arrives, so nothing leaks past the
detector's own latency. The reactive path keeps running underneath for everything aperiodic, and
the scheduled gate is dropped the moment the lock fails a cycle. **This matters because an arc's
residue is not floor — it is a comb on 100 Hz, and every tracked carrier sits on a whole hundred
hertz** (`CARRIERS.md`, *An arc is the opposite case*).

**Blanking costs level and not phase.** The gate is real and non-negative, so a gated carrier is
scaled by the gate's mean — `20·log₁₀(1−f)` — and its sidebands land a whole comb spacing off,
outside a 1 Hz bin. The phase tracker is untouched by the blanker itself; what the flag below
protects is the level.

**The gated fraction rides the header.** A second in which the blanker muted more than **1 % of the
window** carries **`status` 6 CLIPPED** on that frame where the chain railed, and **`status` 7
DISTURBED** where it did not (`../../core/PROTOCOL.md` §1) — the unit reports that its input was not
the signal and never reports what the source was (`../../core/README.md`, *The second load-bearing
principle*). The threshold is where the level error reaches **0,09 dB** — 5,6
steps of the record's 2⁻⁶ dB scale — and it exists because **blanking and a SID both pull the
carrier down**: without the flag a thunderstorm overhead is indistinguishable from a solar flare,
which is the whole quantity.

**Records.** One carrier record per carrier per second — word A the frequency in steps of
**8 Hz** (the converter's band top / 2¹⁶, as on Tesla; a record is read in the context of its
host type), word B the level, type 2 — and the floor record at frequency 0, the detector's
rolling background, once a second. Up to eight records fill a frame; eight carriers and the floor are
nine records a second, two frames, and the other 126 are zeros. Tesla's record shape, Tesla's
dB domain, Marconi's decoder reads it (`../../marconi/BUS.md`).

**The crosstalk matrix and the µ(T) curves are Tesla's**, measured and written the same way
(`../FIRMWARE.md` §6, §8).

## 7. The time — decoding, the path, the second

**Which transmitters.** The image carries the table of `CARRIERS.md`: for each, its frequency,
its coordinates, its modulation — DCF77's AM/PM minute frame, MSF's, WWVB's, JJY's, BPC's,
RBU's; eLoran's GRI, coding delays and phase codes per chain. The **site subset** is what the
scan finds on the listed frequencies; the head may pin or exclude a transmitter by `SET`.

**The date and the second — from a time-code station.** The carrier's envelope, off the
Goertzel's magnitude at 10 ms resolution, is sliced against the station's format: DCF77's
100/200 ms reductions, MSF's, WWVB's pulse-width code, JJY's, BPC's four-bit symbols; the
missing 59th mark names the minute's start; the frame is decoded with its parity and taken
only when **two consecutive minutes agree**. That gives the calendar and the minute; the
second is counted from the minute mark. eLoran gives no date (`CARRIERS.md`): a station with
eLoran alone recovers the second's phase and holds the date it last had.

**The path.** For each transmitter the great-circle distance from `POSITION` — written by the
head, from the station's own GNSS or typed once — divided by the groundwave velocity, **3,33 µs
per km**, is the nominal delay; it is subtracted from the received second mark, and so is the
chain's own latency **`LAT`** — the converter's 2,95 µs at sinc4, OSR 16 (the sinc4's group delay plus
the datapath pipeline, from its datasheet, `../FIRMWARE.md` §5), plus
the front end's group delay at the carrier's frequency from the computed chain in
`../HARDWARE.md` §2, a constant per carrier held in the image, no bench
(`../../core/PROTOCOL.md` §7). What the nominal misses — ground conductivity, the skywave's share
— is what the learning holds.

**Learning, while `QUALITY` says `LOCKED`.** For every tracked carrier, every second, the
phase against the grid minus the nominal delay is the path residual; it is averaged over
**15 min** and stored per hour of day, **24 × int16** per carrier in 2⁻²⁰ s, and its long-term
mean. The daily shape is the ionosphere's day/night move, repeatable; the mean is the
conductivity term geometry cannot predict. Both go to the cells once a day.

**Steering, while `QUALITY` says anything else.** For every carrier the residual is measured as
before and the learned value for this hour subtracted; what remains is the **grid's error
against that carrier**. The carriers vote: the median of the usable ones, a carrier more than
**3 × the spread** from the median dropped as a dissenter and counted. The PPS compare is
moved to the grid instant that the median names as the true second — by **slew, 1 µs per
second at most**, never a step while any consumer counts on it; the first placement after a
boot is the one step. So the edge Kronos captures is the station's error against the caesium
carriers, and Kronos's loop takes it out.

**What the second is worth** is said on `TIME OUT`, not hidden: near a transmitter the residual is
constant and the second is inside the microsecond; far from it the skywave breathes and the
tens of µs of wander are the truth of the path. `$PNIC` carries the spread.

## 8. `TIME OUT` — Kronos's socket

**Served only under the heartbeat.** Kronos sends `$PNIC,HELLO*hh` on both its sockets every
**second** (`../../kronos/FIRMWARE.md` §4); **30 s** without one drops the socket as §3 says. A socket at `ID_PIP` 1,00 is never armed and never waited for.

**Out, every second, while served:**

| what | content |
|---|---|
| PPS | the rising edge on `PPS_PIP`, placed by §7; 100 ms wide |
| `$GPRMC` | the decoded UTC time and date, status `A` when at least two time-code carriers agree on the minute and the phase vote has three or more members, `V` otherwise; the position from `POSITION` |
| `$GPGGA` | fix quality 1 while status is `A`, 0 otherwise; satellites used = the carriers in the vote; HDOP = the vote's spread in µs ÷ 10 |
| `$PNIC,…*hh` | per carrier: the transmitter's id, the level in 2⁻⁶ dB, the residual in 2⁻²⁰ s, in the vote or dropped; then the hours since `QUALITY` last read `LOCKED`, and the spread |

**In:** `$PNIC,HELLO`, `$PNIC,RANGE` (§4). Nothing else on the UART is parsed.

The socket always carries a communication board — Pip is never in the box. Each end's
own socket sets channel B: **`TIME OUT`'s `B_DIR` is tied to 3,3 V on Pip's side**, so the board there
drives the PPS outward, and Kronos's socket ties its `B_DIR` to ground and receives it
(`../../galvani/README.md`, *The reversed channel*). `LINE_EN` is 10 kΩ to 3,3 V on `TIME OUT` and follows
no pin: the line side is up before the first heartbeat, which is what lets the heartbeat arrive.
**Pip never takes a clock off `TIME OUT`** — its clock is `NB IN`'s, off the spur, on `OSC_IN`.

## 9. The control plane

The house table of every unit (`../FIRMWARE.md` §8), with these registers:

| register | r/w | what |
|---|---|---|
| `0x0002 STATUS` | r | the node state, the clock state, the `TIME OUT` state (§3), the converter's `CLK_CNT`, the ranging width |
| `0x0010 CARRIERS` | r/w | the carrier list, 8 × uint16 frequency in 8 Hz steps, `kind` 5; a scan's promotions written on the head's `SET`, or on their own when `SWEEP` bit 1 is set |
| `0x0011 SWEEP` | r/w | bit 0 the scan on (default 1) · bit 1 promotions take effect without the head |
| `0x0012 SOURCES` | r/w | per transmitter of the table: 0 auto · 1 pinned · 2 excluded |
| `0x0013 POSITION` | r/w | latitude, longitude in 10⁻⁷ °, int32 each; the head writes it |
| `0x0014 QUALITY` | w | Kronos's quality byte as the head read it; written on every change |
| `0x0015 LEARN` | r | per carrier: the mean residual, the 24 hourly values, the age of the learning, `kind` 5 |
| `0x0016 TIME` | r | the decoded UTC, its source, the minute's agreement count, the vote's members and spread |
| `0x0017 XTALK` · `0x0020 CALIBRATE` | r/w · w | as Tesla's |
| `0x0021 RANGE` | w | the `NB IN` turnaround, `arg1` the width |
| `0x0038 REPORT` · `0x003A HEALTH` | r | as Tesla's — the `REPORT` frame, `kind` 6, with `NTC_PCB` as its thermometer; the `HEALTH` frame, `kind` 8, with `TIME OUT`'s state among the counters |
| `0x0039 FLOOR` | r | the background, 14-bit log |
| `0xFF00`–`0xFF10` | r | `VERSION` · `IDENT` · `TAG` · `slots` 1 · `HEALTH` (echo/CRC misses, resends, clock losses, IWDG resets, converter faults; carriers promoted and dropped, dissenters, minutes decoded and rejected, heartbeats missed, `TIME OUT` drops) · `SENSORS` (as Tesla's, plus bit 9 `TIME OUT` served) · `ID` (the three codes) |
| `0xFF0A VIN_WINDOW` · `0xFF0B REPORT_INTERVAL` | r/w | the house registers — the supply window, and seconds between `REPORT` frames, default 60 |
| `0xFF13 DELAY` | r/w | the frames the unit holds before it sends — written by its parent at floor-up, 32 on a Bifrost port and 16 behind an Argus, and persisted; never below 8, the image's floor, because `RESEND` is answered from this buffer (`../../core/PROTOCOL.md` §7) |

`HARD_RESET`: the NUMBER to 15, the carrier list and the learning cleared, `SOURCES` to auto;
the µ(T) curves and the crosstalk matrix kept. **Up the link unasked:** `ACK` after a critical command and the `REPORT` frame once a minute — nothing else. `ERROR` only answers a command that failed; a fault is the header's `FAULT` flag, and its detail waits in `HEALTH` for the head's `GET HEALTH` (`../../core/PROTOCOL.md` §1, §5, §7). The entries: the converter, the rods, `TIME OUT`.

## 10. Faults

| trigger | action | reported |
|---|---|---|
| no heartbeat on `TIME OUT` for 30 s | the socket dropped (§3) | `HEALTH` *`TIME OUT` unanswered*, `FAULT` up |
| no carrier tracked for 10 min | the unit ships the floor only; `$GPRMC` status `V` | `HEALTH` DEGRADED *no carriers* |
| the vote's spread over 100 µs | `$GPGGA` quality 0; the PPS still placed | `HEALTH`, `$PNIC` |
| two consecutive minutes disagree | the decode discarded; the last date held | `HEALTH` |
| the converter's faults, the CSS, the ring | as Tesla's §10 | as Tesla's |

## 11. Persistence

The H7A3's cells: the set at enrolment · `CARRIERS`, `SWEEP`, `SOURCES`, `POSITION` on `SET` ·
the learning — per carrier the mean and the 24 hourly residuals — once a day while `LOCKED` ·
the crosstalk matrix and the µ(T) curves as Tesla's, kept by `HARD_RESET`. No time is stored:
a boot decodes the date again.

## 12. The processor's budget

Tesla's board and Tesla's always-on cost for the ring and the ladder (~9 %); the eight
Goertzels ~1 %, the eight trackers ~2 %, the scan ~1 % averaged, the decoders under 1 %; the
PSRAM unused and the strips not cut — Pip runs no classifier. USART2 and TIM15 on `TIME OUT`, TIM2 CH1 the PPS compare — the resources Tesla's
`HARDWARE.md` lists for the socket. **No interrupt sits in a timing path**: a carrier's phase
is a sample index, the PPS is a compare.

## 13. What is tested, and where

**On a PC, with the pins replaced by callbacks:** the scan and the promotion against a
synthetic band; the Goertzel's level and the tracker's phase against a synthetic DCF77 with a
stepped path delay and a burst of impulses across it; the DCF77, MSF, WWVB and JJY decoders
against recorded minute frames including a parity failure; eLoran's GRI correlation and the
third-zero-crossing pick on a synthetic chain; the path delay from four test positions; the
learning's hourly table and the vote with one dissenter; the slew and the one step; the NMEA
sentences and the heartbeat's 30 s; the register map; the cells.

**On the bench:** the unit at a site that hears DCF77 — the PPS within ±1 µs of GNSS on a
counter over a day after a day of learning; the same with GNSS pulled from Kronos, the
station's second held inside the path's wander for 24 h; with nothing on `TIME OUT`, `DE_PIP` low
for 24 h; a Kronos plugged in after boot served within 2 s of its first heartbeat; `TIME OUT` dropped
30 s after Kronos's cable is pulled, the NodBus frames unbroken through it, and served again 10 s
after it is back; eight carrier records and the floor a second on the spur
for 24 h with zero fillers.

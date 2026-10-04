★ N.I.C. ★

# Kronos — the firmware, described

> **This document is the deliverable, not a description of code.** It says everything the clock
> board's firmware does, in the order it does it, with the numbers it uses — enough to write the
> build from, and enough to test the build against. What the board *is*: [`README.md`](README.md);
> the board, the pins, the timers, the clock tree and the time bus:
> [`HARDWARE.md`](HARDWARE.md); the timing doctrine, the receiver checks and the holdover rule:
> [`../core/blocks/gps-pps.md`](../core/blocks/gps-pps.md); the rates:
> [`../core/blocks/nodbus.md`](../core/blocks/nodbus.md); the quality byte and the time scale:
> [`../core/PROTOCOL.md`](../core/PROTOCOL.md) §4. Where this document and one of those differ,
> that one wins and this one is corrected.

## 1. What the firmware is

**One loop, one edge a second, one message a minute.** The firmware disciplines the PLL to a
received pulse, derives the station's second from the steered oscillator, and names that second
on the label bus. It parses one NMEA stream on each of two sockets, ranks the two sources, and
subtracts the delay table once. That is the whole of it: no bus mastering, no frames, no sensor,
no storage beyond a few flash cells. Between the once-a-second loop and the once-a-minute label
the core idles in `WFI`.

**What leaves the board never steps and never stops.** `CLK` on MCO1 and PPS-K on TIM2 CH1 are
integer divisions of the one VCO; a correction moves the VCO by parts per million and both
outputs move with it. The received pulse is the loop's input and nothing else's: it does not
leave the board, it is not forwarded, and when it disappears PPS-K keeps coming off the TCXO
with the quality byte saying so.

| the firmware does | on | how often |
|---|---|---|
| captures the source PPS | TIM5 CH3 (GNSS) · CH4 (Pip) | every second |
| trims `FRACN` | PLL1 | every second, dithered at 2 ms |
| places PPS-K | TIM2, one stretched period | once at first lock; after that by slew only |
| drives `CLK` | MCO1 ← PLL1_Q | continuous, 2²² |
| parses NMEA | UART4 (GNSS) · USART6 (Pip) | as sentences arrive |
| writes the label | I2C1, general call | once a minute, and on a change of quality |
| answers the head | I2C1, slave | on the head's read or write |
| ranges the receiver's run | TIM5 CH1/CH2 (GNSS) · TIM3 (Pip) | once at commissioning, on command |

**Two counters, and they are not the same object.** The **capture counter** is TIM5, 32 bits at
2²⁷; it is only ever read as the difference between two edges a second apart, so its wrap is not
a limit and nothing is chained. The **station's absolute tick count** is the continuous scale in
the Unix epoch at the timebase rate, **int64 in software**, assembled from the label plus elapsed ticks; no hardware counter anywhere carries its width.

## 2. Boot

| # | step | what happens | if it fails |
|---|---|---|---|
| 1 | reset | `DE_CLK` (PE8) and `DE_PPS` (PE10) low — nothing on the ribbon until the PLL is locked; `DE_1` (PE2) and `DE_2` (PE5) low; `LINE_EN_1` (PE3) and `LINE_EN_2` (PE6) low, so a socket comes up off; the IWDG at **1 s** | — |
| 2 | RC | HSI; the flash cells read (§11): the delay table, the source rank, the leap offset, the last `FRACN`, Pip's phase offset; the tag computed — CRC-16-CCITT over the 96-bit UID | a set that fails its check is no set: the board boots with the defaults of §11 |
| 3 | the TCXO | the CSS armed on HSE; N clean periods of the TCXO on `OSC_IN` counted against the HSI — 2²⁴ ± 1 %, N small; then PLL1 locks (M 2 · N 32 · P 2 · Q 64, `FRACN` at the persisted value or 0) and SYSCLK is 2²⁷ | no TCXO: the board stays on the HSI, `DE_CLK` stays low, the quality register reads `NO_OSC`, and the station has no clock — the head reads it and says so |
| 4 | the rail | `PGOOD` (PD14) read | low: `HEALTH` *rail*, `FAULT` up, the board runs |
| 5 | IDs | ADC1 reads the two `ID` inputs (PC0 · PC2) once, single-ended against `VREF+`, 16× oversampled; each ratio placed in its ±0,025 window: which communication board is on each socket's data body — or Polaris — or 1,00 empty, or 0,00 shorted | a ratio in no window is *unknown board* |
| 6 | the outputs | TIM2 started by TIM5's trigger, free-running at 2²⁷ with period 2²⁷, CH1 in PWM mode 1 with `CCR1` = 2²⁴; `DE_CLK` high — **`CLK` is on the ribbon from here and never goes off again except for a cold-stop**; `DE_PPS` high; PPS-K runs at whatever phase the counter has | — |
| 7 | the sockets | UART4 and USART6 at 115 200 8N1, RX by DMA into a line buffer each, idle-line interrupt; TIM5 CH3/CH4 capture on the rising edge, input filter 4 samples; `DE_1`/`DE_2` high; `LINE_EN_1`/`LINE_EN_2` raised for each socket whose `PORT` cell (§11) says on; on a socket typed `M8N` the receiver dialogue of §4 runs first | a receiver that fails the dialogue is reported and still listened to (§4) |
| 8 | the label bus | I2C1 at 100 kHz, multi-master, own address **0x3C**; the label goes out as a master write to the general-call address 0x00, which every card and the head accept; the register map (§8) live. The quality register reads `UNSYNCED` | — |
| 9 | the first label | nothing is written until a source has placed the second (§6) or the head has seeded one (§8). Until then the cards stay `UNLABELLED` and the head reads `UNSYNCED` | — |

**The clock is on the ribbon within 100 ms of power, locked or not.** A card's PLL and a card's
counter want a clock, not a correct one; what the TCXO alone delivers is 1 ppm, and the quality
byte carries the truth about the second until a source arrives.

## 3. The states

```
   RESET ──▶ RC ──▶ CLOCKED ──┬──▶ LOCKED ◀──▶ HOLDOVER
                              │       ▲
                              └──▶ SEEDED ──┘
                     any ──▶ COLD (GATE 0 from the head: DE_PPS, DE_CLK low)
```

| state | the clock | the second | quality byte [1:0] |
|---|---|---|---|
| `RC` | HSI, nothing on the ribbon | none | — (not readable: the bus is not up) |
| `CLOCKED` | the TCXO through the PLL at the persisted code, `CLK` and PPS-K on the ribbon; a source may be under test or pulling in | none — no label has been written; `SECOND` reads 0 | `UNSYNCED` 0 |
| `LOCKED` | steered: the loop closed on a usable source, phase error under ±1 µs | named by the source, incremented on PPS-K, checked every second | `LOCKED` 2 |
| `HOLDOVER` | `FRACN` follows the temperature table from the last good code (§5) | incremented on PPS-K from the last locked label | `HOLDOVER` 1 |
| `SEEDED` | `FRACN` at the persisted code, no steering | named by the head's write, incremented on PPS-K | `SEEDED` 3 |
| `COLD` | `DE_CLK` low, `DE_PPS` low, the PLL still running | held | — |

**The transitions, and what each one costs the station.** `CLOCKED → LOCKED` is the one step
the second ever takes: PPS-K is placed by one stretched TIM2 period (§6) and the first label
names it; the cards were unlabelled until then and take it. `LOCKED → HOLDOVER` is silent on the
wire — the edge keeps coming — and the quality byte changes, which is a label write of its own.
`HOLDOVER → LOCKED` is a slew, never a step: the phase error accumulated in holdover is pulled
in at the slew ceiling (§5) and the state stays `HOLDOVER` until the error is under ±1 µs.
`SEEDED → LOCKED` is a step, because a seeded second was never precise: the cards hold a
seeded label, the new one disagrees by more than one and they take it (`../bifrost/FIRMWARE.md`
§5). **A step is allowed only while the board has not been `LOCKED` since it booted.** Once it has, only slews — every
index downstream counts on the edge, and a step would break them all at once (§6).

**`COLD` is the station's kill.** The head writes `GATE` = 0 (§8): `DE_PPS` goes low, then
`DE_CLK`, and every card sees its clock drop and goes to RC (`../core/blocks/nodbus.md`, *How a
bus starts and heals*). The PLL keeps running and the loop keeps steering if it has a
source, so the clock that returns on `GATE` = 1 is the disciplined one; the label written 100 ms
after the first PPS-K edge re-labels every card. The order of the two enables is fixed: PPS-K
off before the clock, so no card counts an edge with no clock behind it; on the return the
clock first, PPS-K on the next second boundary.

**`COLD` is not the board's off; the lockdown is.** `GATE` = 0 stops what Kronos sends and leaves
the board running. In the battery lockdown the head puts Kronos itself down after the gate, because
nothing is being timed (`../core/POWER.md`, *The lockdown*) — and **there is no `ENABLE` above this
board**, it hangs on the enclosure's battery wire like every other, so that off is a deep sleep the
head commands. **The depth is Stop and never Standby**: the only thing that can raise Kronos is the
head addressing it on **I2C1**, the time bus, so the board sleeps with that one peripheral still
kernel-clocked and its **wake-on-address-match** armed, everything else down. Standby would leave
nothing listening and there is no wake wire anywhere in the station
(`../core/PROTOCOL.md` §7).

**The pins, set before Stop and held through it** — Stop keeps every output at its last level:

| what | held at | why |
|---|---|---|
| `DE_CLK`, `DE_PPS` | **low** | already, from `GATE` = 0 |
| `I2C1` (PB8 · PB9), the time bus | **released, wake-on-address-match armed** | **it is the wake**; held low it would stop the label bus for every card and the head |
| `I2C3` | released | |
| `MCO1` | stopped with the clock | its driver's `DE` is already low |
| the `TMP117` | its own shutdown mode, by register | microamps; a part put down at the part |
| **the TCXO** | **running** — it has no off: disabled its output it still draws 18 mA (`HARDWARE.md`) | **~20 mA, nearly the whole of what Kronos draws in Stop** |

**The TCXO runs through the lockdown.** It has no off — its output disable leaves 18 mA of the
21 and the oscillator running (`HARDWARE.md`) — so it is the one part of Kronos that sleeps at full
draw, and its self-heating, which is part of the holdover's calibration, stays where it was. On
the wake PLL1 locks to a clock that never stopped.

## 4. The sources

**Two sockets, one parser, one rank.** Each socket is the same thing to the firmware — an NMEA
stream on a USART and a pulse on a TIM5 capture — and the code path is one, instantiated twice.
`RANK` (§8) says which comes first: **GNSS first** by default, **Pip first** at a site with no
sky view. The loop steers on the highest-ranked source that is *usable*; the other is measured
against it and reported.

**The heartbeat, on both sockets.** Every **second** the firmware sends `$PNIC,HELLO*hh` on
UART4 and on USART6, whether or not anything has answered — it is what a house unit on the
other end waits for before it serves the socket: a Pip (`../tesla/pip/FIRMWARE.md` §2) or a Sputnik
relaying its receiver (`../sputnik/FIRMWARE.md` §2). A unit that hears none within 30 s of its
boot switches its socket off, and the next heartbeat serves it again. A bare receiver on the
GNSS socket ignores the sentence.

**The receiver's dialogue — `M8N`, at every boot.** On a socket typed `M8N` (`SRC_TYPE` 1) the
firmware brings the receiver to one profile before it listens for time, in UBX on the same UART. A
socket typed anything else gets no dialogue: a Sputnik configures its own receiver, Pip is Pip.

| # | step | the profile |
|---|---|---|
| 1 | find it | `MON-VER` polled at 115 200; no answer in 200 ms, polled at 9600, the M8's rate as shipped. No answer at either: *receiver silent* in `HEALTH`, and the socket listens at 115 200 |
| 2 | name it | `MON-VER`'s hardware version must read **`00080000`**, u-blox M8. Anything else is *receiver unknown*: reported, not configured, and its NMEA and PPS still go through the three checks below — the checks guard the time, the dialogue only the configuration. The `PROTVER` line of its extensions must read **18.00 or above** (firmware SPG 3.01 and later); below it the receiver serves time and cannot announce a leap second, and is reported *receiver old* |
| 3 | the port | `CFG-PRT` polled for UART1; where it is not **115 200 8N1, UBX + NMEA in and out**, written, and the socket's UART follows to 115 200 |
| 4 | the sentences | `CFG-MSG` polled per sentence on UART1: **`RMC`, `GGA`, `ZDA` once a navigation cycle; `GLL`, `GSA`, `GSV`, `VTG` off** |
| 5 | the rate | `CFG-RATE`: **`NAV_RATE` — 1000 ms as written at manufacture, 200 ms where a build sets it**, one cycle, GPS time. The pulse is step 6's and does not move with it |
| 6 | the pulse | `CFG-TP5` for `TIMEPULSE`: **1 Hz, 100 ms locked and no pulse unlocked, rising edge on the second, UTC grid, antenna cable delay 0 — it ships at 50 ns — and user delay 0**; the delay table here owns the whole sum (§7) |
| 7 | the leap second | not a configuration item: `UBX-NAV-TIMELS` is polled once a minute in service (§7) |

**Each item is polled, compared, and written only where it differs**; every write waits for
`ACK-ACK`, and a `NAK` or no answer in 200 ms counts in `HEALTH` and the item is tried again at the
next boot. **`CFG-CFG` is never sent**: nothing is saved to the receiver's flash, so a receiver that
loses power comes back in whatever state it stores and the next boot brings it to the profile
again. The dialogue takes under a second and the receiver's fix does not wait on it. UBX is read in
this dialogue and in `NAV-TIMELS`; everything else in service is NMEA.

**A source is usable when it passes all three checks for 8 consecutive seconds**, and stops
being usable on the first failure:

| # | check | on what | catches |
|---|---|---|---|
| 1 | interval | two captures a second apart read 2²⁷ ± 268 ticks (±2 ppm: the TCXO's own budget, doubled) | a missing, doubled or jumped pulse |
| 2 | fix-valid | `RMC` status `A` and `GGA` quality > 0 for the second the last edge marked | a free-running PPS with no fix |
| 3 | increment | the `RMC` time and date advance by exactly one second per pulse — at a `NAV_RATE` above 1 Hz the whole-second sentence, fraction .00, is the one paired with the pulse and the others are read for position only | a rollover, a frozen date, a serial-versus-pulse mismatch |

**The sentence names the edge that came before it.** A sentence for second *T* arrives tens of
ms after *T*'s pulse; the parser pairs it with the most recent capture whose age is between
0 and 900 ms and rejects the pairing otherwise. Nothing depends on when the sentence arrives —
the pairing tolerates a late sentence up to the next pulse and no further.

**What is parsed, and nothing more** — under any talker, `GP`, `GL`, `GN` or `GB`, and any NMEA
version, fields past the ones read ignored: `RMC` (time, date, status), `GGA` (fix quality, satellite
count, latitude, longitude, height) and, where the receiver sends them, `ZDA` (a clean time and
date, preferred over `RMC`'s when present) and Pip's `$PNIC` sentence, whose fields are Pip's
(`../tesla/pip/README.md`) — the firmware takes from it the carrier count as the satellite count and
its agreement flag as the fix quality, and stores the rest in `SRC_INFO` (§8) unparsed. A
checksum failure discards the sentence; three in a row on one socket count in `HEALTH`.

**Pip against GNSS.** Where both are fitted and usable, every second gives one subtraction on
TIM5: `CH4 − CH3` is Pip's phase against GNSS, in 2²⁷ ticks. It is averaged over 2¹⁰ seconds
and kept as `PIP_OFFSET` — a signed 32-bit tick count, persisted daily. When GNSS is lost and
Pip takes over, Pip's captures are corrected by it, so the handover moves the phase by Pip's
own drift since the last average and not by its bias. **A handover is a change of quality**: the
label written carries the new source in its status byte, and `HEALTH` counts it. A source that
becomes usable again while a lower-ranked one is steering is taken back after its 8 seconds;
a source flapping more than 4 times in a minute is held out for 10 minutes.

**Neither source: the head seeds.** The head reads `UNSYNCED` for longer than its deadline and
writes `SEED` (§8). The board labels by it, keeps `FRACN` where it is, and reports `SEEDED`.
Nothing is disciplined against the seed — there is no pulse behind it. A source that appears
later takes the station to `LOCKED` by the step rule of §3.

## 5. The discipline loop

**Once a second, on the paired capture.** Every step below runs in the idle-line handler of
the steering socket, after the sentence has been paired with its edge; it is not a timing path
(the capture already holds the number) and it takes microseconds.

1. **The raw interval.** `Δ = capture[n] − capture[n−1]`, 32-bit unsigned subtraction, wrap
   ignored. Nominal 2²⁷ = 134 217 728 ticks. `Δ − 2²⁷` in ticks is the frequency error in units
   of 7,45 × 10⁻⁹ — 134 ticks is one part per million.
2. **The gate.** The error is averaged over the last **64 seconds** by a moving sum, or over
   what it has since the source became usable — 8 edges already resolve 0,001 ppm, 64 resolve
   **0,0001 ppm**, far past what a ±1 ppm TCXO does. A
   sample outside ±4 × the running RMS is dropped as a glitch and counted; the sum is not reset.
3. **The phase.** `φ = (capture[n] − corr) mod 2²⁷`, where `corr` is the delay-table sum in
   ticks (§7): the position of the corrected source edge within TIM5's second. PPS-K is TIM2's
   update event, which falls at `K` on TIM5's count — `K` is the offset between the two
   counters, fixed when TIM5 started TIM2 and moved only by the one stretched period of §6 — so
   the phase error is `e = φ − K`, signed, in ticks. ±134 ticks is ±1 µs.
4. **The controller.** A PI in ticks: the frequency term is the 64 s average of step 2; the
   phase term is `e` divided by a time constant of **256 s**, so a 1 µs error is pulled in at
   0,004 ppm and never excites the loop. Their sum, in ticks per second, is the wanted fractional
   offset from the nominal ratio: 1 tick per second is 1/134 ppm.
5. **The slew ceiling.** The wanted offset is clipped to **±4 `FRACN` codes = ±15,3 ppm** about
   the frequency estimate. A phase error is pulled in at that rate at most: a day of holdover
   (86 ms at 1 ppm) takes 1,6 h, during which the state stays `HOLDOVER` (§3).
   **The temperature table.** The `TMP117` beside the TCXO is read every 10 s. While `LOCKED`, the
   64 s frequency estimate of step 2 is filed against the temperature in a table of 0,5 °C bins
   across −40…+85 °C (250 entries, int16 in 2⁻¹⁰ ppm, an entry replaced by a slow average of what
   it sees), persisted in the cells once an hour when something changed. **An estimate is filed
   only if the temperature stayed inside one bin across its 64 s — 0,5 °C, which is 0,5 °C/min —
   otherwise it is read and interpolated but never written**: the sensor is on the outside of
   the can and the crystal is inside it, so on a fast ramp what would be filed is the gradient
   between them and not the oscillator's tempco. The threshold is the bin itself and not a new
   constant — a sample that does not fit in a bin belongs to none. At the package's time
   constant of tens of seconds the lag it still admits is ~0,25 °C, ~0,01 ppm at the residual
   slope, against a table whose own residual is tenths of a ppm. In `HOLDOVER` the
   frequency term is not frozen at the last code: it is the table's entry for the current
   temperature, interpolated between filled bins, the last good code where the bin is empty —
   so a night's cooling does not walk the second at the raw tempco. The phase term is off, as
   before; `HOLDOVER → LOCKED` slews the residual. `0x90 TEMP` reads the temperature and the
   entry in use.
6. **The dither.** One `FRACN` code is **3,81 ppm** — F_REF / 2¹³ = 1024 Hz on a 2²⁸ VCO —
   coarser than the correction. The wanted offset is split into an integer
   code *c* and a duty *d* ∈ [0, 1): TIM6 raises an interrupt every **2 ms** and the handler
   writes `FRACN = c + 1` for the first `d × 500` interrupts of each second and `c` for the rest,
   `PLLFRACEN` cleared and set around the write. The phase moved during one 2 ms hold at one
   code's 3,81 ppm is 7,6 ns — one tick — so the dither is invisible on the wire. The write
   itself is three register accesses, 50 ns.
7. **Lock.** From `CLOCKED`: the step of §6 on the source's 8th usable second, then `LOCKED`
   when |`e`| < 134 ticks and the frequency estimate has moved less than 0,05 ppm over the
   last 8 s — under 20 s from the first usable second. From `HOLDOVER`: `LOCKED` when |`e`| has
   been slewed under 134 ticks. `HOLDOVER` on the first failed check of §4 while `LOCKED`. In `HOLDOVER` steps 1–4 stop — there is no
   capture to steer on — and `FRACN` follows the temperature table of step 5 through the dither of step 6.
8. **The persisted code.** Once an hour in `LOCKED` the current (`c`, `d`) is written to its
   flash cell, so a boot starts at yesterday's rate and not at the TCXO's raw error.

**The loop never sees an interrupt latency.** The capture is hardware; the sentence's arrival
time is irrelevant; the `FRACN` write lands wherever it lands within its 2 ms slot. Nothing in
steps 1–8 depends on when the code runs, only on what the counter latched.

## 6. PPS-K and the clock out

**TIM2, period 2²⁷, the update event the edge.** TIM2 counts the same 2²⁷ as TIM5, was
started by TIM5's trigger so the two counters differ by a constant `K` the firmware knows to
the tick, and never stops. CH1 on PA5 is PWM mode 1 with `ARR` = 2²⁷ − 1 and `CCR1` = 2²⁴: the
output goes high at every update and low 2²⁴ ticks later — one rising edge a second, a
125 ms pulse, so a receiver that missed the edge still sees a level. The `DS91C176` driver on
the PPS pair takes it as it comes off the pin; no GPIO write is ever made to PA5.

**Placing the edge — once.** On the source's 8th usable second, if the board has not been `LOCKED` since boot, the firmware computes
`δ = (φ − K) mod 2²⁷`, the corrected source edge's position ahead of the next update, and
writes `ARR` = 2²⁷ − 1 + δ with preload on, so the period in progress runs δ ticks long and the
following update lands on the source edge; the next update restores `ARR` and `K` is advanced
by δ. That is the one step, and it is known to the tick because the counter was never written.
The label that follows within 100 ms names the second the stretched period's update marks.
From there the loop of §5 keeps `e` at zero by moving the VCO, and `ARR` is never touched
again except after a cold-stop (§3) or a `HARD_RESET`.

**`CLK` is not the firmware's.** MCO1 carries PLL1_Q at 2²² from the moment the PLL locks;
the firmware sets `MCO1SEL` and `MCO1PRE` once at boot and `DE_CLK` twice in the board's life
(boot, and a cold-stop). What the ribbon carries is exactly 2²² periods per PPS-K edge by
construction, because both are the one VCO divided.

**What the station gets:** a second whose edge is within ±1 µs of the corrected source when
`LOCKED`, within 86 ms per day of it in `HOLDOVER`, and at the head's word in `SEEDED`. The
edge's own placement error is the capture's 2 ns RMS plus the loop's residual, nanoseconds; the
±1 µs is the contract, not the performance (`../core/blocks/gps-pps.md`, *The precision contract*).

## 7. The delay table, the leap offset, and the ranged run

**Everything between the antenna and the capture pin is a bias, not noise**: fixed terms, all
pushing the same way, of known length — so they are subtracted, not tolerated. **Three terms, the
shape of the CGGTTS convention — two typed, one measured:**

| term | what | how it is known | size |
|---|---|---|---|
| **`CAB`** | the antenna cable | typed: metres × the cable's ns/m, ~5 ns/m for coax | ≤ 4 m, ≤ 20 ns — the receiver lives in the enclosure |
| **`INT`** | the antenna and the receiver as one number — the antenna's group delay and LNA, the module from antenna port to PPS pin, the pigtail, the in-box hop | **a constant of the receiver's type**: its datasheet figure, or measured once on the bench against a second receiver on a common antenna | tens of ns |
| **`ROUTE`** | the run of a remoted source | **ranged by this board**, never typed (below) | ~5 ns/m of glass, 1 km ≈ 5 µs; an in-box receiver's run is centimetres and lands in `INT` |

**The sum is rounded to whole ticks of 7,45 ns and held as `corr`, per socket, and subtracted
here once.** It enters the loop at step 3 of §5 and nowhere else, before anything downstream sees
the second; nothing below re-corrects. An in-box receiver's terms reach a fifth of the microsecond
the station delivers; a remoted source's fibre alone is microseconds, and there the subtraction is
what meets the contract at all. **A receiver's own cable-delay setting stays at zero** — this
board owns the whole sum — and the dialogue of §4 sets the receiver's own cable delay and user
delay to 0.

**The sawtooth is not in the table.** A receiver's PPS quantisation changes sign pulse to pulse;
it is not a bias and cannot be subtracted as a constant. The loop's averaging removes it.

**Typed at the head, kept on the board.** `CAB` and `INT` are entered once, in nanoseconds, on
the Handset's corrections table (`../mayak/handset/README.md`), and Kronos keeps its own copy in
the cells (§11). The persisted table is what the board runs on from its first second; the head
reads it (`CORR`, §8) and confirms or overwrites it — **the head is the authority, the board's
copy is what makes a normal boot silent**, and a replaced Kronos is handed the numbers without
anyone typing them again. A written table takes effect on the next loop iteration as a phase
error to be slewed out, so a corrected table moves the second by the difference and not by a
step. The Handset shows every term the station subtracts, the ranged one read-only beside the
typed ones.

**The leap offset.** The label is the continuous scale, GPS time in the Unix epoch, leap-blind
(`../core/PROTOCOL.md` §4): the firmware adds the GPS−UTC offset it holds to the UTC the
sentence names. The offset is a persisted cell, **18 s** as shipped. **On an `M8N` the firmware keeps it
itself**: `UBX-NAV-TIMELS`, polled once a minute, writes `currLs` into the cell where
`srcOfCurrLs` is not 0 — 0 is the firmware's hard-coded value, which can be out of date — and a
`lsChange` of ±1 with `validTimeToLsEvent` set marks the leap pending, its boundary from
`timeToLsEvent`. Elsewhere the head's `LEAP` write carries it, from the UM980's own message or Pip's
`$PNIC`, or typed by the operator; a head write always wins until the receiver next reports a
different value. The firmware reflects a pending leap in the quality byte's bits [3:2] and, at the announced
boundary, keeps the label monotonic while the sentence's UTC repeats a second — check 3 of §4
expects the repeat for that one second and does not fail on it.

**Ranging the receiver's run.** Where the socket's `ID` names a communication board and the
head sets `RANGE` (§8), the firmware ranges the run once, the way a card ranges a spur
(`../bifrost/HARDWARE.md`, *Ranging*):

1. `$PNIC,RANGE*hh` is sent on UART4 — the far end is a processor that owns the run's pins, a
   Sputnik's H523 or a Pip's H7A3, and it arms its turnaround timer for **500 ms** on that sentence;
   after 20 ms UART4 is stopped and PA0 becomes `TIM5_CH1` compare, PA1 `TIM5_CH2` capture.
2. CH1 fires one rising edge at a compare value; the far end's one-pulse timer returns it
   after the house constant; CH2 latches the return.
3. `return − fire − T` is twice the route, in ticks, `T` = **256 ticks** — the house
   turnaround, 1,91 µs on every board that turns an edge (`../bifrost/HARDWARE.md`,
   *Ranging*); **32 launches**, 10 ms apart, the median taken, ±1 tick.
4. UART4 is restored; the route is written to `corr` as `route_ticks / 2` and to its cell, and
   reported read-only as the `route` term.

Pip's run, where Pip stands outside the box, is ranged the same way on USART6/TIM3. A run that
returns nothing in 1 ms is *unranged*: `corr` keeps the persisted route and `HEALTH` says so. A
remoted receiver whose run is unranged is what the head calls NO-GO;
the firmware only reports the absence.

## 8. The label bus and the register map

**I2C1, 100 kHz, multi-master, three kinds of traffic and nothing else on it.** The bus is a
few centimetres of ribbon with the head and up to four cards on it; arbitration is the I²C
specification's and the firmware handles a lost arbitration by retrying after the bus is free.

**The label — a general-call write, 8 bytes, 100 ms after the PPS-K edge.**

| byte | content |
|---|---|
| 0–3 | the Unix second the last PPS-K edge marked, little-endian |
| 4 | the quality byte: [1:0] `UNSYNCED` 0 · `HOLDOVER` 1 · `LOCKED` 2 · `SEEDED` 3 · [3:2] leap pending · [7:4] satellite count, capped at 15 |
| 5 | the source: 0 none · 1 GNSS · 2 Pip · 3 the head's seed; bit 7 set on the first label after a step |
| 6 | reserve, 0 |
| 7 | CRC-8 over bytes 0–6 |

Written **every 60 s**, on the second divisible by 60, and **on every change of
bytes 4 or 5** on the next second. A card that hears it compares, corrects by one or takes it
(`../bifrost/FIRMWARE.md` §5); the head reads it and gates its stamps. **The label is never
written before a second exists** — a board with no source and no seed writes nothing, and the
absence is what `UNLABELLED` on the cards means.

**The registers — Kronos as a slave at 0x3C, one-byte register address, little-endian values.**

| register | r/w | content |
|---|---|---|
| `0x00 SECOND` | r | the Unix second of the last PPS-K edge, 4 B; the same value the label carries |
| `0x04 QUALITY` | r | the quality byte and the source byte of the label, 2 B |
| `0x06 STATE` | r | §3's state, 1 B, plus a flags byte: `NO_OSC` · `RAIL` · `STEP_PENDING` · `UNRANGED_GNSS` · `UNRANGED_PIP` |
| `0x08 PHASE` | r | `e` of the steering source, int32 ticks; `PIP_OFFSET`, int32 ticks |
| `0x10 FREQ` | r | the 64 s frequency estimate, int32 in 2⁻¹⁰ ppm; the current `FRACN` code and duty |
| `0x18 POSITION` | r/w | latitude, longitude in 10⁻⁷ degrees (int32 each), height in cm (int32): what the steering receiver last parsed, or what the head wrote at a site with no receiver. A head write is refused with the receiver's value while a source is usable |
| `0x24 SRC_INFO` | r | per socket: usable flag, the last fix quality, the satellite/carrier count, the seconds since the last usable second (uint32); Pip's last `$PNIC` unparsed, 32 B |
| `0x5C SRC_TYPE` | r/w | 1 B per socket: the receiver type, **1 `M8N` · 4 `UM980`** · 5 `PIP` · 0 none; 2, 3 and 6 are retired and never reissued. It picks the boot dialogue (§4, `M8N` only), the leap-second message and the `INT` default; it is **typed, never detected** (`../core/blocks/gps-pps.md`). A write takes effect at the next boot of that socket |
| `0x5E NAV_RATE` | r/w | 1 B: the receiver's navigation rate, **1 or 5 Hz, 1 as written at manufacture**; applied at step 5 of the profile at the next boot. `POSITION` then follows every `GGA`; the pulse, the steering and the checks do not change — only the whole-second sentence is paired with the pulse |
| `0x60 RANK` | r/w | 1 B: 0 GNSS first · 1 Pip first |
| `0x61 SEED` | w | 4 B: the coarse Unix second the head believes; taken only in `CLOCKED` or `SEEDED`, refused otherwise |
| `0x65 LEAP` | r/w | the GPS−UTC offset, int8 seconds; the pending flag and its boundary (a Unix second, 4 B) |
| `0x70 CORR` | r/w | the delay table per socket: `CAB` and `INT` as int32 ns, then `ROUTE` as int32 ticks read-only, then the summed `corr` in ticks read-only |
| `0x90 TEMP` | r | the TCXO's temperature, int16 in 0,1 °C; the table's entry in use, int16 in 2⁻¹⁰ ppm; the count of filled bins |
| `0xA0 RANGE` | w | 1 B: 1 range the GNSS run · 2 range Pip's run |
| `0xA1 GATE` | w | 1 B: 0 cold-stop, drivers off (§3) · 1 drivers on |
| `0xA2 HARD_RESET` | w | the tag's low 16 bits, as the node contract's confirmation: the cells erased, the board reboots as fresh |
| `0xA4 PORT` | r/w | 1 B per socket: 0 off · 1 on — the socket's `LINE_EN`. The head writes 1 after the run is wired and what is on the far end is established, never before (`../galvani/README.md`, *`ENABLE`, `LINE_EN` and the three states*) |
| `0xE0 IDS` | r | the two `ID` ratios as read at boot, uint16 in 2⁻¹⁰ |
| `0xF0 HEALTH` | r | counters, uint16 each: glitches dropped by the gate, sentence checksum failures per socket, check failures per socket per kind, handovers, holdover entries, lost arbitrations, label writes, steps |
| `0xF8 IDENT` | r | the tag, 2 B; the firmware version, 2 B; the board name, `KRONOS`, 8 B |

A write to a read-only field is refused by not acknowledging the byte. Every r/w register that
survives a boot is a cell (§11) and is written to flash when the write completes.

**Nothing switches a socket but the head's `PORT` write.**

**Nothing is pushed but the label.** `HEALTH` and `SRC_INFO` are read by the head at its
own cadence, and the station's clock SOH byte (`../core/PROTOCOL.md` §4) is what the head
derives from `QUALITY`.

## 9. What Kronos does not do

- **It does not forward the source pulse.** No pin carries PPS-GNSS or Pip's PPS off the board.
- **It does not steer on a seed.** A seed labels; the frequency stays the TCXO's plus the
  persisted code.
- **It does not correct on a card's behalf.** The spur routes are the cards'; `corr` here is
  the receiving chain only.
- **It does not keep time across a power cut.** No cell holds a second; a boot is `CLOCKED`, quality `UNSYNCED`,
  until a source or a seed says otherwise.
- **It saves nothing in the receiver.** The dialogue of §4 runs at every boot and never sends
  `CFG-CFG`.
- **It does not reduce its own clock.** The core runs at 2²⁷ because TIM5 does; the saving is
  in `WFI`.

## 10. Faults

| trigger | action | reported |
|---|---|---|
| the CSS fires (the TCXO stopped) | NMI: SYSCLK to the HSI; `DE_PPS` low, then `DE_CLK` low — a dead clock is better than the HSI's ±1 % on the ribbon; the board reboots after the IWDG expires and re-tries the TCXO | `NO_OSC` in `STATE`, if the bus can still be served |
| the steering source fails a check | `HOLDOVER`; the other source, if usable, steers after its 8 s | a label with the new quality |
| both sources gone for 2 h | still `HOLDOVER`; nothing changes on the wire | `HEALTH` counts the hours |
| the gate drops more than 8 samples in 64 s | the source is marked unusable — its pulse is noise — until it passes 8 clean seconds again | `HEALTH` |
| a sentence pairs with no edge (age > 900 ms) 3 times running | the socket's serial is restarted; check 3 fails for that source | `HEALTH` |
| a paired label disagrees with the board's own count by more than 1 s while `LOCKED` | the source is dropped to unusable and `HOLDOVER` entered — a receiver that jumps its date is a hallucination, not a correction | a label with `HOLDOVER` |
| `PGOOD` low | nothing to switch; noted | `RAIL` in `STATE` |
| I2C1 stuck low for 25 ms | the bus is recovered by nine SCL pulses and a STOP; the label is re-sent on the next second | `HEALTH` lost-arbitration |
| a label's own CRC read back on the bus disagrees (the board monitors its own write as a slave does) | re-sent once on the next second | `HEALTH` |
| the IWDG expires | reset; the persisted code puts the rate back within 0,01 ppm on the first lock; the cards see a clock gap of the boot time and go to RC — the price of a hung clock board, which is why the image is small | `HEALTH` boots |

## 11. Persistence

The H523's flash, in the node contract's append-only cells (`../core/PROTOCOL.md` §7): a cell
is id, value, check; the last good cell per id wins; a full sector is rewritten; the live set
rewritten once a year.

| cell | content | default | written |
|---|---|---|---|
| the delay table | `CAB`, `INT` per socket, ns | all zero | on a `CORR` write |
| the routes | per socket, ticks | none (unranged) | after ranging |
| the receiver types | `SRC_TYPE` per socket | 0 none — typed at commissioning | on write |
| the navigation rate | `NAV_RATE` | 1 Hz | on write |
| the rank | `RANK` | GNSS first | on write |
| the leap offset | `LEAP` | 18 s, nothing pending | on write |
| the code | (`c`, `d`) | 0, 0 | hourly while `LOCKED` |
| Pip's offset | `PIP_OFFSET` | 0 | daily while both usable |
| the position | `POSITION` as written by the head | none | on a head write that was accepted |
| the ports | `PORT` per socket | off | on a write |

**No cell holds a second, a label or a state.** The set carries one check over the whole of it
and a set that fails is no set: the board boots on the defaults.

## 12. The processor's budget

| resource | used | of |
|---|---|---|
| USARTs | 2 — UART4 the GNSS socket, USART6 the Pip socket | 7 |
| I²C | 2 — I2C1 the label bus, I2C3 the `TMP117` | 3 |
| DMA | 3 — the two sockets' RX, ADC1 | 16 |
| timers | TIM5 capture (all four channels) · TIM2 PPS-K · TIM3 Pip's ranging · TIM6 the 2 ms dither | |
| interrupts, by priority | 0 the CSS (NMI) · 1 TIM6 (the `FRACN` write) · 2 the idle line per socket · 3 I2C1 · 4 I2C3 · 5 the ADC. TIM5's captures raise none — the numbers are read in the idle-line handler | |
| SRAM | two line buffers 2 × 128 B · the 64 s sums · the register map 256 B · ~2 kB of state | 272 KB |
| flash | the image · the cells 8 KB | 512 KB |
| CPU | < 1 %: one loop iteration of microseconds a second, 500 register writes a second, one NMEA parse a second per socket; `WFI` otherwise | 2²⁷ |

**No interrupt sits in a timing path.** The source edge is a capture, PPS-K is a compare, the
clock is a divider; the `FRACN` write may land anywhere in its 2 ms slot and moves the phase by
one tick if it lands late.

## 13. What is tested, and where

**On a PC, with the pins replaced by callbacks:** the parser and the three checks against
recorded NMEA — a receiver losing its fix, a date frozen for a minute, a sentence 950 ms late,
a leap second inserted; the pairing of sentences with captures; the loop against a simulated
TCXO with ±1 ppm drift and 20 ns of pulse jitter — lock within 2 min of the first usable
second, |`e`| under 134 ticks for 24 h, a 3 µs glitch dropped by the gate; a day of holdover
followed by re-lock — pulled in at 15 ppm with no step; the handover GNSS → Pip → GNSS with the
phase moved by Pip's drift only; the seed path and the one step; lock under 20 s from the first usable second; the label bytes and their CRC; the heartbeat every second on both sockets;
the receiver dialogue against a recorded NEO-M8N — at 9600 as shipped and at 115 200 configured, a
`NAK`, a wrong hardware version, a receiver that does not answer — and each item written only where
it differed; the register map, every refusal; the cells.

**On the bench, against `HARDWARE.md`:** the TCXO on `OSC_IN` and 2²² on PA8 within 10 ns of
period jitter; `CLK` and PPS-K phase-coherent on the ribbon — exactly 2²² clock periods between
two PPS-K edges, counted on a card; PPS-K against the receiver's PPS on a counter after the
delay table is entered, within ±1 µs over 24 h; the `FRACN` dither invisible on PPS-K at 1 ns
resolution; the ranging constant reproduced to ±1 tick over a hundred launches on a 2 m
cable and within 2 % of length on a 500 m reel; the label heard by four cards and the head
within 2 ms of its start; a cold-stop and return re-labelling every card within 2 s.

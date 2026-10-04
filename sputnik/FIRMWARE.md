★ N.I.C. ★

# Sputnik — the firmware, described

> **This document is the deliverable, not a description of code.** It says everything the
> node's firmware does, in the order it does it, with the numbers it uses — enough to write the
> build from, and enough to test the build against. What the unit *is*: [`README.md`](README.md);
> the board, the pins and the timers: [`HARDWARE.md`](HARDWARE.md); the tiers and the epoch:
> [`BUS.md`](BUS.md); the frames, the opcodes and the node contract:
> [`../core/PROTOCOL.md`](../core/PROTOCOL.md); the clock and the bus start:
> [`../core/blocks/nodbus.md`](../core/blocks/nodbus.md). Where this document and one of those
> differ, that one wins and this one is corrected.

## 1. What the firmware is

**One receiver, one H523, five NUMBERs.** The firmware configures the UM980 at every boot,
reads its raw observables at 5 Hz on COM1, works each epoch up into the tier records of
`BUS.md`, chops the epoch into 32 B payloads and sends them in the five slots the node holds.
Beside that it relays the receiver's lean NMEA from COM2 onto the clock run to Kronos byte for
byte, captures the receiver's PPS on its own timebase, and turns Kronos's ranging edge round on
the clock run's pins. It computes no position, no TEC and no correction: the raw carrier phase
is the archive and everything else is derived at home (`BUS.md`, *Curate, don't strip*).

**The node is a NodBus unit five times over.** It enrols once, takes five consecutive NUMBERs
and five slots, and fires five frames a round. The five slots are one queue — a chunk goes into
whichever slot comes next — so the master reassembles by frame index and slot order, and the PRN
in every record says which constellation it was (`BUS.md`, *The five slots are one pool*).

| the firmware does | on | how often |
|---|---|---|
| configures the receiver | UART5 (COM3) | at boot, and on a `SET` that reaches it |
| reads the satellites' azimuths | USART2 (COM1), `SATSINFO` | every 10 s |
| reads the observables | USART2 (COM1), 921 600, DMA | one epoch every 200 ms |
| builds the epoch | — | per epoch |
| sends chunks | USART1, five TIM2 compares a round | 640 frames/s |
| relays the time stream | UART4 (COM2) → USART6, DMA | as bytes arrive |
| captures the PPS | TIM2 CH1 (PA15) | every second |
| turns the ranging edge | TIM4 (the link) · TIM3 (the clock run) | on command |
| reads the power body | I2C1 | once a second |
| sends the report frame | USART1, in the first NUMBER's block behind its data: `VBUS` · `CURRENT` raw · the H523's die temperature, int16 in 0,01 °C — the board has no thermometer of its own · 0 | once a `REPORT_INTERVAL`, 60 s (`../core/PROTOCOL.md` §5) |

## 2. Boot

| # | step | what happens | if it fails |
|---|---|---|---|
| 1 | reset | `DE` (PE2) low, `DE_T` (PE5) low — both sockets' `LINE_EN` are 10 kΩ pull-ups in the socket and no pin of this board; `GNSS_RST` (PD10) held low; the IWDG at **1 s** | — |
| 2 | RC | HSI; the flash cells read (§11): the set, the rate, `TIER_C`, the signal group; the tag computed — CRC-16-CCITT over the 96-bit UID | a set that fails its check is no set: the node boots as fresh |
| 3 | the rail | `PGOOD` (PD14) read | low: `HEALTH` *rail*, `FAULT` up, the node runs |
| 4 | IDs | ADC1 reads `ID_D` (PC0), `ID_P` (PC1), `ID_T` (PC2) once, 16× oversampled, ratiometric: the boards on the NodBus socket's two bodies and on the clock-run socket, or 1,00 empty, or 0,00 shorted. **`ID_T` = 1,00 means no clock run: the relay of step 7 is not started** | a ratio in no window is *unknown board*; the node runs |
| 5 | the receiver | `GNSS_RST` released; UART5 (COM3) at 115 200 waits for the receiver's boot banner, **3 s** at most; then the configuration of §6 is sent line by line, each answered `$command,…,response: OK` before the next — the first line, `CONFIG SIGNALGROUP 2`, reboots a receiver whose group differed, so after it the node waits for the banner once more, 3 s at most, before line 2 | no banner, or a line refused: `GNSS_RST` pulsed and the sequence repeated once; then `HEALTH` NO_RESPONSE for the receiver and the node enrols anyway, sending empty epochs |
| 6 | the streams | USART2 (COM1) at 921 600, circular DMA into a 4 kB ring, idle-line interrupt; UART4 (COM2) at 115 200, circular DMA into 256 B | — |
| 7 | the relay | if `ID_T` names a board: USART6 at 115 200 listening, `DE_T` low, until Kronos's heartbeat `$PNIC,HELLO*hh` arrives (`../kronos/FIRMWARE.md` §4) — every second from a live Kronos; then `DE_T` high — the heartbeat gates the stream on `DE_T` alone — and the idle-line handler of UART4 queues what arrived as one USART6 DMA transmit. Nothing within **30 s**: the socket stays off, `HEALTH` *clock run unanswered*, `FAULT` up; a heartbeat arriving later serves it | — |
| 8 | the timebase | TIM2 free-running at 2²⁷ once locked (step 10); CH1 captures the PPS on PA15, CH2 the start-bit edges on PB3 | — |
| 9 | the power body | I2C1: the `INA238` on `ID_P`'s board configured; `ALERT` (PC8) an EXTI | no answer where `ID_P` names a power board: `HEALTH` *no telemetry*, `FAULT` up |
| 10 | enrol | §4 | — |

## 3. The states

```
   RESET ──▶ RC (38 400, silent) ──▶ ENROLLED ──▶ LOCKED (HSE on the wire) ──▶ RUNNING
                 ▲    │                 ▲                 │                       │
                 │    └─ DISCOVER heard ┘                 └─ SYNC ────────────────┘
                 │                                                   │
                 │◀── clock lost (CSS) ─────────────────────────────┤
                 │◀── IWDG reset ───────────────────────────────────┤
                 └── ENDED (END: the parts stop, the buffer is flushed and answered; then the clock and the feed go)
```

`RC` — the HSI, the USART at 38 400, transmitting nothing; **the receiver runs and the relay to
Kronos runs in every state but reset**, because on a Sputnik station this receiver is the
station's clock source and the node's own state is not allowed to take it away. `ENROLLED` —
five NUMBERs and five slots held, still at 38 400. `LOCKED` — `BUSCFG` taken, HSE on the wire,
PLL1 at 2²⁷, the USART on the rung, the epoch pipeline running into the chunk queue, nothing
sent. `RUNNING` — `SYNC` loaded the index; chunks go up. `ENDED` — on `END`: the observables
stopped at the receiver (`UNLOG COM1`), the queue emptied, the USART listening; **the relay
untouched**. **Clock lost** is a fault: the CSS raises an NMI, the node drops to the HSI, mutes,
and comes back through the rejoin (§4); the receiver never notices.

## 4. The bus — the unit's side

**The enrolment** is the node contract, as on every unit: `DISCOVER` at 38 400 answered once
with the tag in the time bytes after `tag × 10 µs`, listening first on `RXD_ECHO`, backing off
on a bad echo; `ASSIGN_ADDR` by tag gives a NUMBER and a slot, `ACK`; **`GET slots` answers
5**, and the card gives four more NUMBERs and slots to the same tag, consecutive
(`../core/PROTOCOL.md` §2). `BUSCFG` (count, rate code 7, rung) locks HSE onto the wire, PLL1
to 2²⁷, the USART to the rung; the deadline is `round start + (slot − 1) × width` for each of
the five slots. `SYNC`, on a second's boundary, is captured on TIM2 CH2 (PB3) and loads
`unix.0 · frame` at that edge less `ROUTE`, the written route in ticks of 2²⁷, so the grid is
the card's (`../core/PROTOCOL.md` §7).

**The slots.** Five TIM2 compares a round, one per slot, each starting USART1's DMA transmit
of the next 40 B frame from the chunk queue (§7); `DE` up for the frame, down after the last
stop bit by the USART's own `DEAT`/`DEDT`; the echo on USART3 checked when it ends; a miss
counted in `HEALTH` and the frame held in the circular buffer it is sent from — **one buffer of `DELAY` rounds of
five**, 160 frames at a Bifrost's 32. A `RESEND frame · unix.0` is answered from it in the node's own slot and
displaces that slot's chunk, which is re-queued at the head of the queue; nothing is lost, the
epoch arrives one chunk later.

**The gaps:** `GET`, `SET`, `RESEND` and `HARD_RESET` in the node's block, as §8; a CONTROL
frame addressed to any of the five NUMBERs is the node's. `TICK` ignored while running; the
phase source on a rejoin. **Ranging** on `SET RANGE`: after the next DATA frame PB6/PB7 switch
to TIM4, `DE` up, one-pulse mode — CH2's capture starts the counter, CH1 raises the return at
`CCR` = **256 ticks** of 2²⁷ (1,91 µs) and drops it at the width the card asked for; CH2 in
PWM-input mode captures the incoming pulse's width for `STATUS`; the pins go back. No interrupt
in the path.

**The rejoin** after any reset: clock and a persisted set → HSE locked, the USART on the rung,
listen; a good CRC proves the rung, a second of nothing tries the other; the phase from a
neighbour's frame or the card's `TICK`, captured on TIM2 CH2; the node fires in the next round
with `status` 1 REJOINED on its first frame. The epoch in progress is dropped and the next one
starts the stream. No clock → RC, silent. No set → 38 400, silent, a sweep.

**`END`**: `UNLOG COM1` to the receiver, the queue emptied and answered, the USART listening;
if the node is brought back the observables are re-logged and the stream resumes at the next
epoch. **The receiver is not reset and the relay is not stopped** — **`END` ends Sputnik's
measuring side and never its clock relay**, because Kronos is what that relay feeds
(`../core/POWER.md`, *The lockdown*). **Taking the relay away is taking the station's time
source away, and only the lockdown does it** — there `ENABLE` goes and the whole unit with it.
No rail on the board is switched (`../galvani/README.md`, *Three states*).

## 5. Time on the node

**One 32-bit timer, never reset.** TIM2 counts 2²⁷ from PLL1 and is read as differences. Three
things hang on it: CH2's capture of every start-bit edge on `RXD` (the phase source), the five
compares that fire the slots, a compare every 2²⁰ ticks that advances `frame`; and **CH1's
capture of the receiver's PPS on PA15**, which is what places the receiver's time on the
station's grid.

**The receiver's epochs are on the receiver's time, not the station's.** The UM980 tags every
observation message with its own GPS week and millisecond, and its epochs sit on the top of its
own 200 ms — which is the top of the GPS second within the receiver's own error, tens of
nanoseconds once it has a fix. The PPS edge is the physical mark of that second, and its capture
on TIM2 gives, once a second, `pps_tick` = the position of the receiver's second boundary on the
node's grid, in 2²⁷ ticks after the frame-0 boundary the card's `SYNC` established. It goes up
in the time reference of §7 and the reader needs nothing else: an epoch tagged `week · ms` by
the receiver falls `ms` milliseconds after that edge, and the reader places it on the grid from
`pps_tick` — 134 217,728 ticks to the millisecond.

**Kronos's ranging of the clock run.** `$PNIC,RANGE*hh` arriving on USART6 arms the turnaround:
PC6/PC7 switch from USART6 to TIM3 for **500 ms**, one-pulse mode — CH2's capture of Kronos's
edge starts the counter, CH1 raises the return at `CCR` = **256 ticks** and drops it 256 ticks
later; the relay's bytes queue up meanwhile and go out when the pins return. No interrupt in
the path.

**The same PPS drives the clock run's channel B outward in hardware** — the net on PA15 is the
second socket's `CLK/PPS` through the buffer — and the firmware does nothing for it. Kronos
disciplines against it and ranges the run to know its delay (§6); this node contributes the
turnaround and nothing else.

**No PPS for 3 s** while the receiver reports a fix is a `HEALTH` DEGRADED for the receiver: the
epochs still go up with `pps_tick` from the last edge, and the flag says so.

## 6. The receiver

**Configured at every boot, never saved in the receiver.** The node holds the configuration
and writes it over COM3 at step 5 of §2, so a replaced module behaves like the old one from its
first boot. The sequence, one line at a time, each waited for its `OK`:

| # | line | what it does |
|---|---|---|
| 1 | `CONFIG SIGNALGROUP 2` | **the one group that carries the whole Tier B backbone** — GPS L1C/A · L1C · L2C · L2P(Y) · L5, BDS B1I · B3I · B1C · B2a · B2b, GLONASS G1 · G2 · G3, Galileo E1 · E5a · E5b · E6, QZSS L1C/A · L1C · L2C · L5 · L1S, NavIC L5, SBAS L1C/A. **A group that differs from the receiver's reboots it and is saved by the receiver itself**, which is why this line is first and alone: the node waits for the banner again before the rest |
| 2 | `UNLOG COM1` · `UNLOG COM2` · `UNLOG COM3` | every port silent — `UNLOG` with a port and no message stops everything on it |
| 3 | `CONFIG PPS ENABLE GPS POSITIVE 10000 1000 0 0` | the PPS: rising edge on the GPS second, **10 ms wide** — the widest the receiver takes is 20 000 µs — once a second, no RF and no user delay. `ENABLE` pulses only while the receiver has a fix and holds about 30 s after it is lost |
| 4 | `CONFIG COM1 921600` · `CONFIG COM2 115200` · `CONFIG COM3 115200` | the three ports' rates, all in the receiver's table |
| 5 | `OBSVMCMPB COM1 0.2` | **message 138**, the compressed observations of every tracked satellite and signal, binary, every 200 ms — the receiver's periods are 1 · 0,5 · 0,2 · 0,1 · 0,05 · 0,02 s |
| 6 | `GPSIONB COM1 ONCHANGED` · `BDSIONB` · `BD3IONB` · `GALIONB` · `GPSUTCB`, the same way | **messages 8 · 4 · 21 · 9 · 19**: the broadcast ionosphere models and the GPS–UTC parameters as they change — Tier C |
| 7 | `SATSINFOB COM1 10` | **message 2124**: every tracked satellite's PRN, azimuth in whole degrees, elevation, system and C/N₀ per signal, every 10 s — the geometry records' source (§7) |
| 8 | the augmentation pages, `ONCHANGED` on COM1: `E6MASKBLOCKB` · `E6ORBITBLOCKB` · `E6CLOCKFULLBLOCKB` · `E6CLOCKSUBBLOCKB` · `E6CBIASBLOCKB` · `E6PBIASBLOCKB` (**2319–2324**) · `PPPB2BINFO1B` … `PPPB2BINFO5B` (**2302 · 2304 · 2306 · 2308 · 2310**) | Galileo HAS and BeiDou PPP-B2b as the receiver decodes them — Tier C. The manual marks the B2b logs *supported by specific versions*: a refused line counts in `HEALTH` and the page is absent, nothing else changes |
| 9 | `GPRMC COM2 1` · `GPGGA COM2 1` | the lean time stream for Kronos, NMEA, once a second |

**What is not in the list, and why.** QZSS **L6E MADOCA** is decoded only in signal groups 3 and
10, which drop GPS L1C, GLONASS G3, BeiDou B1C and NavIC — the backbone is the archive and wins,
so MADOCA is not logged on this receiver (`WHY.md`). **The SBAS ionospheric grid has no log**: the
receiver tracks SBAS L1C/A for its own fix and outputs nothing of its messages but the QZSS L1S
types on another product, so SBAS is not a Tier C source. **No `GSV` on COM3**: `SATSINFO` gives the
same azimuths in binary on the port the parser already reads, and COM3 carries the configuration
and its answers alone.

**Every line is ASCII, terminated `\r\n`, and answered by the receiver on the same port**; a
line not answered `OK` in 500 ms is sent again once, then the receiver is `NO_RESPONSE` and the
node enrols with empty epochs. The receiver is left in `SIGNALGROUP 2`, which it saves by itself,
and is **never told `SAVECONFIG`** for anything else — the node contract keeps the configuration
in the node, not the sensor (`../core/PROTOCOL.md` §7).

**Reading COM1.** The Unicore binary framing is one shape for every message: a **24 B header** —
the sync bytes `AA 44 B5`, CPU idle, **message id** (uint16), **body length** (uint16), the time
reference, the time status, **GPS week** (uint16) and **milliseconds of week** (uint32), the
firmware version (uint32), a reserved byte, the leap second, the output delay (uint16) — then the
body, then a **CRC-32 over header and body**. The idle-line handler walks the DMA ring for complete
messages, checks each CRC and hands the body to the parser of its id; a CRC failure discards the
message and counts in `HEALTH`. **The compressed observation** is 4 B of count and **24 B a
signal**: tracking status 32 b · Doppler 28 b in 1/256 Hz · pseudorange 36 b in 1/128 m · carrier
phase 32 b in 1/256 cycle, which wraps and is unwrapped onto the arc base of §7 · the two
standard-deviation indices 4 b each · PRN 8 b · lock time 21 b in 1/32 s · C/N₀ 5 b as 20 + n dB-Hz
· the GLONASS frequency number 6 b · 16 b reserved. **The rate is the receiver's: 5 Hz.** A
`SET RATE` (§8) rewrites line 5 with the new period and the epoch cadence follows.

**The geometry.** `SATSINFO` every 10 s on the same port: per satellite a PRN byte, the **azimuth
as a signed 16-bit whole degree**, the elevation byte, the system byte, then 4 B a signal; the
handler keeps the latest azimuth and elevation per PRN in the table the geometry records are
drawn from (§7). A satellite with no entry yet gets no geometry record until one arrives. **COM3**
carries the configuration lines and their answers and nothing else; the node parses the `RMC` and
`GGA` it relays on COM2 — the second and the date from the one, the position and the fix quality
from the other — and that is the only parsing done on COM2.

## 7. The epoch, and what goes on the wire

**One epoch every 200 ms, one byte stream, chopped into 32 B chunks.** For every `OBSVMCMP`
message the node builds the epoch in a 3 kB buffer:

| part | content | size |
|---|---|---|
| the lag | two bytes: how far back the epoch belongs, in units of **2⁻¹⁰ s**, measured from the frame the first chunk leaves in to the receiver's epoch tag | 2 B |
| Tier B, per satellite | PRN · elevation · flags · slot count, then three 9 B frequency slots in the backbone order of `BUS.md`; pseudorange and carrier phase relative to the satellite's arc base, SNR as `(C/N₀ − 20) × 4` | 31 B × N |
| a geometry record | one satellite an epoch, round-robin: `PRN · 0xFE · azimuth` (uint16, 0,01° — the receiver gives whole degrees, so the value is a multiple of 100), the latest `SATSINFO` value | 4 B |
| Tier C | the decoded augmentation pages that changed since the last epoch, each as `source · length · page` | as they come |
| padding | zeros to the chunk boundary — the reader walks records until a zero at a PRN position; **an epoch that would end exactly on a chunk boundary is followed by one chunk of zeros**, so an epoch's first chunk is always the first non-zero chunk after padding | ≤ 32 B |

**The arc base.** A 4 B millimetre field cannot carry a 20 000 km pseudorange, so every
satellite's pseudorange and carrier phase are sent **relative to a base the node declares when
the satellite's arc begins**: the first epoch a PRN appears in carries an **arc record** ahead
of its Tier B record — `PRN · 0xFF · flags · slot count`, then `base pseudorange (int64, mm) ·
base phase (int64, 0,001 cycle)` per slot, 4 + 3 × 16 = 52 B — and every epoch after carries the 31 B record with
the differences. A cycle slip the receiver flags starts a new arc for that satellite. **The arc records
are re-sent round-robin, one satellite per epoch**, so with 60 satellites every base recurs
within 12 s and a reader joining mid-stream, or one that lost a chunk, has every base within
that; between re-sends a satellite's record with no base in hand is held by the reader, not
guessed.

**The time reference.** The first epoch to begin in each station second — the reader knows
it from the frame header's `unix.0`, which changes at frame 0 — is preceded by 8 B: the Unix
second the receiver's last `RMC` named — UTC, where the receiver's own epoch tags and the station's
seconds are GPS time, so a reader adds the GPS − UTC offset of the archive's `TIME` section
(`../core/archive/HMC.md`) — and `pps_tick` (§5), uint32 — the position of that
second's PPS edge on the grid. That is the one place the node writes an absolute time and it is
the receiver's, not the card's; the card stamps the frame's own second as on every unit.

**The chunk queue.** The epoch buffer is cut into 32 B chunks and appended to a queue of 256
chunks; each slot compare takes the next one; `kind` is 0 on every chunk — the payload is the
unit's own data and the epoch boundary is in the bytes. A chunk sent is kept in the resend
ring (§4). **The queue never fills at 5 Hz**: an epoch of 60 satellites is 60 × 31 B + one arc
record + Tier C, under 2,3 kB, **70 chunks**, against the 128 chunks the five slots carry in
200 ms — 55 %, the worst sky on Earth (`BUS.md`, *Rate and bandwidth*). At 8 Hz it is 80 chunks an
epoch, 87 %; **at 10 Hz it is 64 and the worst epoch does not fit, so `SET RATE 10` is
refused**. An epoch that would overflow the queue is dropped whole and counted — never sent in
part.

**Nothing is computed on the observables.** No TEC, no geometry-free combination, no smoothing,
no elevation gate: every tracked satellite goes up, the low ones flagged by the receiver's own
lock and health bits.

## 8. The control plane

| op | what the node does |
|---|---|
| `DISCOVER` | answers with its tag, §4 — only when not in a running round |
| `ASSIGN_ADDR` by tag | takes a NUMBER and slot, five times; writes the cells; `ACK` |
| `BUSCFG` | takes the item; on the third, locks and computes the five deadlines |
| `SYNC` | loads the index at the captured edge, starts the stream |
| `TICK` | ignored while running; the phase source on a rejoin |
| `END` | §4 |
| `HARD_RESET` | the NUMBERs to 15, the rate, `TIER_C` and the signal group to their defaults; then a reset — the node comes up fresh at 38 400 |
| `RESEND frame · unix.0` | the frame from the circular buffer, in the node's own slot |
| `GET reg` | a byte under `GET`; a wider register as a DATA frame, `kind` 5, the register number first |
| `SET reg · value` | written, applied, `ACK`; a setting that reaches the receiver rewrites its line of §6 and marks the first affected epoch's chunk `status` 5 CHANGED |

**The registers** — the house block and the house positions are the house map's
(`../core/blocks/modbus.md`, *The house map*):

| register | r/w | what |
|---|---|---|
| `0x0002 STATUS` | r | the node state (§3), the clock state, the receiver state (booting · configured · no response · no fix · fix), the ranging width last measured |
| `0x0010 RATE` | r/w | the epoch rate in Hz: 1 · 2 · **5** · 8; 10 refused |
| `0x0011 TIER_C` | r/w | the augmentation pages on the wire, default 1 |
| `0x0012 SIGNALGROUP` | r/w | the receiver's signal group, **2** |
| `0x0021 RANGE` | w | arms the ranging turnaround for the next block, `arg1` the width in ticks |
| `0x0030 GNSS` | r | tracked · used · fix quality · the receiver's week and ms of the last epoch · latitude · longitude · height, `kind` 5 |
| `0x0031 EPOCH` | r | the last epoch's satellite count, size in chunks, lag, the queue depth |
| `0x0038 REPORT` | r | answers with the `REPORT` frame itself, `kind` 6 |
| `0x003A HEALTH` | r | answers with the `HEALTH` frame, `kind` 8: the MCU die temperature, the receiver's state, the error counters |
| `0xFF00 VERSION` | r | firmware version |
| `0xFF01 IDENT` | r | the house code |
| `0xFF02 TAG` | r | CRC-16 of the UID |
| `0xFF03 slots` | r | **5** |
| `0xFF04 HEALTH` | r | echo/CRC misses, resends served, clock losses, IWDG resets, COM1 CRC failures, COM1 overruns, receiver resets, epochs dropped, PPS misses |
| `0xFF09 SENSORS` | r | the present-mask: bit 0 the UM980 |
| `0xFF0A VIN_WINDOW` | r/w | the supply window, the SUPPLY flag outside it — the house register (`../quake/FIRMWARE.md`) |
| `0xFF0B REPORT_INTERVAL` | r/w | seconds between `REPORT` frames, default 60; 0 disables |
| `0xFF10 ID` | r | the three `ID` codes read at boot |
| `0xFF13 DELAY` | r/w | the frames the unit holds before it sends — written by its parent at floor-up, 32 on a Bifrost port and 16 behind an Argus, and persisted; never below 8, the image's floor, because `RESEND` is answered from this buffer (`../core/PROTOCOL.md` §7) |

**Up the link unasked:** `ACK` after a critical command and the `REPORT` frame once a minute — nothing else. `ERROR` only answers a command that failed; a fault is the header's `FAULT` flag, and its detail waits in `HEALTH` for the head's `GET HEALTH` (`../core/PROTOCOL.md` §1, §5, §7).

## 9. Health and self-test

**`HEALTH` for the one sensor**, the house vocabulary: OK · NO_RESPONSE · DEGRADED, read on
`GET HEALTH`, the `FAULT` flag up while it is not OK. `DEGRADED`
is a receiver that answers but has had no fix for 10 minutes, or no PPS for 3 s with a fix, or
whose COM1 stream shows CRC failures above one a minute. There is no self-test to run: the
receiver's own health bits ride in every Tier B flags byte and the station reads them.

**The relay is watched, both ways.** If the clock-run socket is served and no `RMC` has passed
through it for 5 s, the relay's UART4 is restarted and `HEALTH` counts it; Kronos sees the gap
as a failed check on its side and says so itself. **30 s without Kronos's heartbeat drops the
socket** — `DE_T` low, the queue emptied, `HEALTH`, `FAULT` up; the next heartbeat serves it again. The
receiver and its PPS run on regardless.

## 10. Faults

| trigger | action | reported |
|---|---|---|
| no complete message on COM1 for 2 s while `RUNNING` | `GNSS_RST` pulsed 100 ms, the configuration of §6 re-sent; three times in ten minutes and the receiver is `NO_RESPONSE` until the next boot | `HEALTH`, `FAULT` up |
| a COM1 DMA overrun | the ring reset to the next sync bytes; the epoch in progress dropped | `HEALTH` |
| an epoch larger than the queue's free space | dropped whole | `HEALTH`, `EPOCH` |
| the CSS fires | NMI: HSI, mute; the rejoin of §4 | `status` 1 REJOINED |
| `ALERT` on PC8 | the reading latched into `HEALTH`; nothing to switch on this board | `FAULT` up |
| `PGOOD` low | noted | `HEALTH` *rail*, `FAULT` up |
| the IWDG expires | reset; the receiver keeps running through it — `GNSS_RST` is only pulled in step 1 of a power-on boot, not on a watchdog reset (the reset cause register says which) | `HEALTH` |

## 11. Persistence

The H523's flash, in the node contract's append-only cells (`../core/PROTOCOL.md` §7): a cell
is id, value, check; the last good cell per id wins; a full sector is rewritten; the live set
rewritten once a year.

| cell | content | written |
|---|---|---|
| the set | five NUMBERs, five slots, `BUSCFG`, one check | at enrolment |
| the rate | `RATE` | on `SET` |
| Tier C | `TIER_C` | on `SET` |
| the signal group | `SIGNALGROUP` | on `SET` |

**Nothing about the receiver's state is stored** — no almanac, no last fix, no time. The
receiver holds its own almanac in its own memory and starts as fast as it starts.

## 12. The processor's budget

| resource | used | of |
|---|---|---|
| USARTs | 6 — USART1 the link, USART3 the echo, USART2 COM1, UART4 COM2, UART5 COM3, USART6 the clock run | 7 |
| DMA | 8 — USART1 TX, USART3 RX, USART2 RX, UART4 RX, UART5 RX/TX, USART6 TX, ADC1 | 16 |
| timers | TIM2 timebase (CH2 the capture, CH1 the PPS) · TIM4 the link's ranging · TIM3 the clock run's turnaround · TIM6 housekeeping | |
| interrupts, by priority | 0 the RXD capture · 1 the five slot compares · 2 the USART idle lines (COM1 first) · 3 the PPS capture · 4 `ALERT` · 5 I2C1 · 6 the ADC · 7 TIM6 | |
| SRAM | the COM1 ring 4 kB · the epoch buffer 3 kB · the chunk queue 256 × 32 B = 8 kB · the circular buffer 160 × 40 B · the arc table 64 × 3 × 16 B · ~2 kB of state | 272 KB |
| flash | the image · the cells 8 KB | 512 KB |
| CPU | < 3 %: one `OBSVMCMP` of 60 satellites parsed and differenced in under 1 ms, five times a second; five frames a round; `WFI` otherwise | 2²⁷ |

**No interrupt sits in a timing path.** The PPS and the frame edge are captures; the slots are
compares; the ranging turnarounds are one-pulse timers. The epoch's own time is the receiver's
tag, and a late handler moves the lag field, not the epoch.

## 13. What is tested, and where

**On a PC, with the pins replaced by callbacks:** the Unicore framing and the CRC-32 against
recorded COM1 captures — a 61-satellite epoch, a message cut by an overrun, a cycle slip
starting a new arc; the arc base and the differences round-tripped through a reader; the
round-robin re-send of the arc and geometry records; the `SATSINFO` parser against recorded captures; the chunking and the reassembly by slot order with one chunk displaced by a
`RESEND`; the queue at 5 and at 8 Hz with the worst sky, and `SET RATE 10` refused; the register map; the cells.

**On the bench, against `HARDWARE.md`:** the receiver configured from cold in under 5 s and
`OBSVMCMP` arriving at 5,00 Hz; the relay passing `RMC`/`GGA` with no byte lost for 24 h while
the link runs at 128 Hz; the PPS captured on TIM2 CH1 within ±1 tick of the receiver's edge on a
counter; the clock run's turnaround at 256 ticks ±1 over a hundred of Kronos's launches; five
slots at 128 Hz for 24 h with zero fillers on a bench spur; the receiver still streaming to
Kronos through an `END` and a watchdog reset of the node.

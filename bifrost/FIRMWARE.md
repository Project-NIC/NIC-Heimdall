★ N.I.C. ★

# Bifrost / Argus — the firmware, described

> **This document is the deliverable, not a description of code.** It says everything the card's
> firmware does, in the order it does it, with the numbers it uses — enough to write the build
> from, and enough to test the build against. What the card *is* and why: [`README.md`](README.md);
> the board, the pins and the timers: [`HARDWARE.md`](HARDWARE.md); the frames and the opcodes:
> [`../core/PROTOCOL.md`](../core/PROTOCOL.md); the clock and the bus start:
> [`../core/blocks/nodbus.md`](../core/blocks/nodbus.md). Where this document and one of those
> differ, that one wins and this one is corrected.

## 1. What the firmware is

**One build for one board in two roles.** The same image runs a Bifrost and an Argus; the level
on `ATTN` (PD2) at boot decides which, and nothing else about the image changes. What differs
between the roles is a table of constants — the clock ratio, the frame length on a port, the rung
range, what goes up the link — and one extra layer on an Argus: the tiling of four mini payloads
into one 32 B payload.

**The card does four things and nothing else.** It brings its ports up (`FLOOR`), it keeps time
and stamps, it re-slots what its ports say onto its up link, and it drives two pins per port on
command or on its own ladder. It holds no policy, parses no payload, runs no sensor, and carries
no ModBus. Anything more is refused (`README.md`, *The card makes NodBus only*).

| | Bifrost | Argus |
|---|---|---|
| up link | the trunk to the Mayak — point-to-point, 48 B frames, 2²¹ | a NodBus spur to a Bifrost — the card is a unit there, 40 B frames, 2²⁰ or 2²¹ |
| down ports | four NodBus spurs, 40 B frames, rung 2²⁰ or 2²¹ per port; **sync 2²² on all four** | four mini segments, 16 B frames, data rung 2²⁰ or 2²¹; **sync 2¹⁹ on all four** |
| clock in | Kronos's time bus, 2²² on `OSC_IN` | the up port's `CLK`, 2²² on `OSC_IN` |
| the second | PPS-K on TIM2 CH1 + the label on I2C1 | the up port's frames — the card is a unit and counts like one |
| what goes up | every port frame, prepended with `unix.3..1` | 32 B payloads tiled from four 8 B mini payloads, under the card's own NUMBERs |
| units served | ≤ 8 | ≤ 8 sondes; the card takes ⌈sondes/4⌉ NUMBERs upstream |

**The work is a DMA-fed relay plus anchor arithmetic.** Nothing in the data path is computed
per byte; the processor touches a frame three times — a CRC check, three bytes written in front
of it and five zeroed over the old CRC, a CRC written behind them — and the rest is tables and
timers. The load at the full eight
units is under 5 % of the M33 at 2²⁷, which is the design: the scarce resources are UARTs, DMA
channels and timer captures, not cycles.

## 2. Boot

The order is fixed and every step has a state the card reports if it stops there.

| # | step | what happens | if it fails |
|---|---|---|---|
| 1 | reset | every port's `EN` (PE3 · PE6 · PE10 · PE13) and `OE` (PE9 · PB14 · PB15 · PD8) low, every `DE` low; the up port's `LINE_EN` is held high by its socket's 10 kΩ and no pin touches it; the three bucks run whenever the 12 V is there. **The safe state is every port off** — a card that is reset, unpowered or has lost its master leaves every load down | — |
| 2 | RC | HSI, the flash cells read (§12), the tag computed — CRC-16-CCITT over the whole 96-bit UID, into the `TAG` register | a failed cell set is no set: the card boots as fresh |
| 3 | role | `ATTN` (PD2) read once: high → Bifrost, low → Argus. Read once, never again — a card is not re-roled at run time | — |
| 4 | clock | the reception chain validates the clock on `OSC_IN`: N clean periods at the right frequency against the HSI — 2²² in either role, N small; then PLL1 locks (M 1 · N 64 · P 2, both roles) and SYSCLK is 2²⁷; `MCO1` is set to HSE at prescaler 1 (Bifrost) or 8 (Argus), every `OE_n` still low. **No clock:** the card stays on the HSI, reports `UNCLOCKED` on the up link where it can, and waits — a card never runs a round on RC | the Bifrost's up link runs at 38 400 on the HSI so `UNCLOCKED` can be said; an Argus is mute like any unit without a clock |
| 5 | IDs | ADC1 reads the ten `ID` inputs (PC0–PC5 · PA6 · PA7 · PB0 · PB1) once, single-ended against `VREF+`, 16× oversampled; each ratio is placed in its ±0,025 window and the port's two boards are named — which communication board, which power board, or empty (1,00), or a short (0,00 → the port is faulted before it is fed) | a ratio that lands in no window is reported as *unknown board* and the port is not fed |
| 6 | the up link | **Bifrost:** UART4 on the trunk at 38 400 (RC dialogue) — the card answers the Mayak's `DISCOVER` with its tag, takes its NUMBER from `ASSIGN_ADDR`, then the link goes to 2²¹ on `BUSCFG` from the head. **Argus:** the up port is a NodBus unit's link — the card does exactly what §9 says a unit does | — |
| 7 | the second | **Bifrost:** TIM2 runs; the first PPS-K edge on CH1 is captured; the first label on I2C1 loads the second register. Until both have happened the card is `CLOCKED, UNLABELLED` and **cannot stamp**: frames from its ports are held in the circular buffer and the card says so. **Argus:** the card's own `SYNC` from its Bifrost loads `unix.0 · frame` like any unit's | a label older than a minute + 2 s marks the time `STALE`; the second still increments on PPS-K |
| 8 | ports | nothing. The ports stay unfed until the Mayak says `FLOOR`, or — on a card whose flash cells hold a persisted port set — until the card has run its floor by itself (§4). A card with a dead master still brings its units up | — |

**A Bifrost's ports are fed one at a time, in port order, 200 ms apart.** Four island converters
starting at once on the pack's wire is an inrush the wire and its fuses were not sized for;
four starting in sequence is four times nothing.

## 3. The states

```
   CARD                                   PORT (one of four, each on its own)

   RESET ──▶ RC ──▶ CLOCKED ──▶ LABELLED    OFF ──▶ IDENTIFIED ──▶ FED ──▶ PROBING ──▶ SWEEP ──▶ RANGED
              │        │           │                                  ▲                            │
              │        │           └─ ASLEEP (deep, Mayak's word)     │ once a minute, DISCOVER    ▼
              │        └─ the clock gone: RC, ports kept up,           │ at 38 400 on an empty port CONFIGURED (BUSCFG) ──▶ SYNCED
              │           stamping stops, the buffer holds             │                            │           ▲
              └─ UNCLOCKED: every port off                            │                            ▼           │
                                                                       └──────────── DEGRADED ◀── the ladder ── DEAD
```

**Card states.** `RC` — the HSI, no clock validated. `CLOCKED` — PLL locked, no second yet.
`LABELLED` — PPS-K and the label both seen; the card stamps. `ASLEEP` — **the card's off, and
it is a deep sleep and not a state of the bus** (§10): a card sits on the enclosure's battery wire
with no `ENABLE` above it, so the Mayak, which is master over it, commands it down and it wakes on
its trunk USART's start bit (`../core/PROTOCOL.md` §7). **The depth is Stop and never Standby**,
and what fixes it is the port: the trunk is **UART4**, an ordinary full-rate USART and not the
LPUART, because no bus in this station runs on an LPUART. So the card sleeps with **UART4 still
kernel-clocked from a source Stop leaves running and its wake-on-start-bit armed**, and every
other clock, timer and port down. **In Standby there is nothing left to hear the Mayak**, and
there is no wire to shout on — a card in Standby is a site visit. **The clock gone** in operation is not a reset: the card drops to the HSI, keeps every
port fed and every port's clock gate shut (`OE_n` low — the port clock is the clock the card no
longer has), holds the buffer, and reports `UNCLOCKED` up the link at 38 400. The
returning clock is validated as at boot and the ports' rungs restart; the units on them rejoin by
themselves (§9).

**Port states.** `OFF` — `EN` low. `IDENTIFIED` — the two `ID`s read and named. `FED` — `EN`
high, the port's rail and line side on, `OE` high, the clock running at the card's rung. `PROBING` —
no unit known: the port fed, its clock gate shut (`OE` low), `DISCOVER` broadcast once a minute at 38 400. `SWEEP` — the RC dialogue running
(§4). `RANGED` — every unit's route measured. `CONFIGURED` — `BUSCFG` sent, the units on the
wire clock. `SYNCED` — `SYNC` fired, the round running. `DEGRADED` — one or more units on the
ladder (§11). `DEAD` — every unit on the port declared dead; the port stays fed and probing.

## 4. `FLOOR` — bringing a port up

`FLOOR` from the master runs the whole ladder on every port that is `IDENTIFIED` or beyond; the
card runs it by itself at boot on any port whose persisted set says it had units. The ports run
their floors **concurrently** — four independent state machines — and the Mayak is told at each
step by the segment table (§7).

| # | step | detail | time |
|---|---|---|---|
| 1 | feed | `EN` high. The port's `INA238` — 0x40 or 0x41 on its controller, by the socket's `A_SEL` strap — is programmed with `APOL` = 1 (high = alarm): shunt over-voltage at **the port's trip point** — the far end's start-up peak and running load measured at commissioning, written with `SET` into the per-port register `0xFF20 + port` and persisted; until one is written, the cell's ceiling the power board's `ID` names (the communication board's `ID` says nothing about watts) — and bus under-voltage at the cell's collapse point. `ALERT` armed | 200 ms after the previous port |
| 2 | the clock | the port's `OE` high: its gate in the quad buffer passes `MCO1`, **2²² on a Bifrost, 2¹⁹ on an Argus**, named by `ATTN` (HSE at prescaler 1 or 8). No port's `ID` is read to pick it. **The port clock runs from the moment the port is fed and never stops for the start** | at once |
| 3 | settle | the far end's island converter starts and its unit boots on RC. The card waits **2 s**, then reads the `INA238`: no current where a unit was expected is reported. A port that answers no `DISCOVER` in step 4 goes to `PROBING` — its clock output stopped, `DISCOVER` once a minute — and the first reply restarts the clock and the sweep from step 2 | 2 s |
| 4 | `DISCOVER` | broadcast at 38 400 on the port's USART, `DE` raised for the frame. Then the card holds the round open until **700 ms** of silence — every unit not in a running round answers once, its tag in the two time bytes, after `tag × 10 µs` and listening first; two units that collide back off on their own echo and try again with a new delay. The card records every reply with a good CRC: tag, `TYPE\|NUM` as the unit said it | ≤ 655 ms of delays, closed by 700 ms of silence; `DISCOVER` once more and 700 ms again |
| 5 | `ASSIGN_ADDR` | to each tag in turn: the NUMBER the card holds for it (the persisted one, or the next free from the bottom on a fresh port) and the slot. Then `GET slots`: a unit whose type takes more than one slot (Sputnik five by type; Tesla one to three, Steinmetz three, an Argus ⌈sondes/4⌉ — read from the register) gets its further NUMBERs and slots by further `ASSIGN_ADDR`s to the same tag. `ACK` closes each | ≤ 10 ms per unit |
| 6 | the count | **the physical count of the port is the count of `DISCOVER` replies**; the slot count is the sum of the `slots` registers. More than eight slots on one port is refused: the card reports it and the port stays in `SWEEP` | — |
| 7 | the rung | one unit → 2²⁰; two to eight → 2²¹. The handset may pin either through `SET`; a third value is refused. The data rung is the same pair on a mini segment | — |
| 8 | ranging | one unit at a time: `SET` the unit's ranging register (bridge after your next DATA frame — but no DATA frame exists yet at 38 400, so at floor-up the command is *bridge now*: the unit arms its timer and the card fires **1 ms** later). TIM4 · TIM15 · TIM8 · TIM3 (the port's) in one-pulse mode: the pins switch USART → timer, `DE` up, CH1 raises the edge at `CCR`, CH2 captures the return; the pins switch back. The measurement is `2 × route + K`, in 7,45 ns ticks, **`K` the unit's programmed turnaround plus the silicon's own: 2 timer clocks of input synchroniser and 1 of compare on each capture, from the reference manual — the card subtracts `K` = `CCR` + 3 ticks of the unit's rate**; three launches, the median kept. **Halving assumes the clock channel's parts delay the edge as the data channel's do**; where a board's two channels differ, the difference is the unit's `SKEW` (`../core/PROTOCOL.md` §7), not the card's. A unit whose return does not arrive within the window (the whole 4 ms at floor-up) is marked `UNRANGED` in the segment table's status byte — not in its frames' header — and the port still runs. **The route is written to the unit as `0xFF11 ROUTE`, in ticks of 2²⁷, and stored per unit on the card**; the unit places its grid that much early at `SYNC` (`../core/PROTOCOL.md` §7), so its frames arrive in their nominal slots and the card subtracts nothing. **Then on a period**: every `RANGE_INTERVAL` seconds (§10 — 60 by default on a port whose `ID` names a Galvani board, 0, never, on an in-box cable) the port's units are ranged again one at a time, each in the gap behind its own DATA frame, the return `RANGE_WIDTH` ticks wide; three launches, the median. A route that differs from the stored one by **more than 2 ticks** is written to the unit again — it moves its grid by the difference at the next frame boundary — persisted, and reported up as `HEALTH` *route changed*, `FAULT` up with the slot; within 2 ticks nothing changes. **A cable does not change length**: a difference over **128 ticks** (0,95 µs, ~200 m) is not written on one measurement — the port is ranged again at once, and only two consecutive measurements agreeing within 2 ticks are written; a lone outlier is counted in `HEALTH` and dropped. A unit that stops returning keeps its last route and is marked `UNRANGED` | ≤ 15 ms per unit |
| 8a | `DELAY` | `SET 0xFF13 DELAY` to each unit: **32** from a Bifrost, **16** from an Argus to its sondes (the Argus holds the other 16 of the 32 its Bifrost wrote it). The unit persists it and never goes below its image's floor of 8. The card's own delay on the port is what makes the path 128 — 96 on a Bifrost, 16 on an Argus | ≤ 1 ms per unit |
| 9 | `BUSCFG` | three broadcasts, one item each: `NODE_COUNT` (the slot count), `FRAME_RATE` (the code, 7 for 128 Hz — the fastest rate present on the port; the units' `GET rate` answers say what each wants), `RUNG`. Every unit validates the clock it has been hearing and switches HSE onto it — a mini unit takes the 2¹⁹ rung on a timer, not HSE | 3 frames |
| 10 | `SYNC` | **on the next PPS-K edge** (Bifrost) or **on the card's own next `unix.0` boundary** (Argus): the last frame at 38 400, `unix.0` = that second's low byte, `frame` 0, its start-bit edge placed on the grid by TIM2 — the transmit is started by a TIM2 compare, not by the processor, so the edge is where the header says it is. The card's port USART switches to the rung the frame after | 1 frame |
| 11 | the round | the units fire at `round start + (slot − 1) × width`. The card's port receiver is armed for the frame length (40 B or 16 B), idle-line detection ends a frame, and the first round with every slot filled moves the port to `SYNCED`. The segment table goes up (§7) | 1 period |

**A persisted port set makes step 4 a check, not a search.** The card holds, per port, the tags
it last enrolled with their NUMBERs and slots; a `DISCOVER` reply from a known tag on the same
port takes its old NUMBER back and nothing is renumbered. **The head still owns the numbers**:
the table goes up as measured, the Mayak renumbers what collides across the station with
`ASSIGN_ADDR` through the card, and only the card's confirmation writes the mirror.

**Adding a unit to a running port is service.** The handset says so, the card takes that one
port's round down — that port's clock dropped, so its units mute, then steps 4 to 11 again — a few
seconds on that port and nothing else notices. The round is never rewritten while it runs; a unit that
appears on a running port without the service call is heard on the echo receiver as a collision,
reported, and ignored until the next `FLOOR`.

## 5. Time on the card

**Two registers, one timer.** TIM2 runs free at 2²⁷ and is never reset; the **sub-second** is its
count since the last PPS-K capture on CH1, and the **second** is a 32-bit word loaded from the
label and incremented by the card itself on every PPS-K edge. **The captured edge is late by the
bus — a `DS91C176` driver and a `THVD1450` receiver in series, 3,4 + 25 ns typical from the
datasheets, plus the ribbon, ~30 ns — so the second is placed four ticks of 2²⁷ before the
capture**; the receiver's spread to its 40 ns maximum and the pulse skew are left to the budget
and not counted. Reading the time is two loads. A
glitched count — a PPS-K edge that lands more than ±1 µs from where the previous period predicts
it — is rejected, the second still increments on the predicted edge, and the event is counted in
`HEALTH`; three in a row and the card re-anchors on the next accepted edge and reports `GLITCH`.

**The label is a check, not a source.** Once a minute Kronos writes the full Unix second the last
PPS-K edge marked; the card compares it with its own count. Equal: nothing. Off by one: the
card's count is corrected and `HEALTH` counts a slip. Off by more, or the quality byte says
`UNSYNCED`: the card takes the label, stamps, and marks every frame `TIME_UNSYNCED` in the
status it forwards — the head decides what that is worth. **No label for 62 s** marks the time
`STALE`; **no label ever since boot** means the card does not stamp, holds the buffer and lets
frames leave as fillers with reason *card unlabelled*.

**The stamp.** A port frame's `unix.0 · frame` names the instant the unit's sample was taken, on
the unit's own grid, which is the card's grid delayed by the route. The card:

1. takes the frame's arrival edge from TIM2 (the port USART's `RXD` is on the port's ranging
   timer, whose capture on the start bit is read by the receive interrupt — the arrival is known
   to a tick);
2. checks the edge against the slot's compare — the unit placed its grid its route early and
   the run put the frame back onto the card's grid, so the arrival is the nominal slot time
   within the guard; an edge outside the guard is a slipped unit;
3. computes which second that slot falls in and checks that the second's low byte equals the
   frame's `unix.0`. **A match is the only arithmetic the card does on a frame**: `unix.3..1` are
   the high bytes of that second, written in front. **A mismatch is a slipped unit**: the frame is
   not corrected — it goes up as a filler under the unit's address with reason *slipped*, and
   the unit is put on the ladder (§11).

No route is subtracted anywhere on the path: the unit applied it once, at `SYNC`
(`../core/PROTOCOL.md` §7); the card and the head do no delay arithmetic. A unit marked
`UNRANGED` holds no route, runs on the delayed grid, and the segment table carries its state; the
second check tolerates its late arrival within the guard.

**TICK.** Once a second, in the gap after the last slot of the round, the card sends `TICK` on
every `SYNCED` port: a 12 B CONTROL frame with nothing but its header, its start-bit edge placed
by a TIM2 compare on the exact second. A running unit ignores it; a unit coming back on a
point-to-point port takes its phase from it (§9).

## 6. The data path

```
   port USART ──DMA──▶ a 48 B receive buffer per port, the frame landing 3 B in
        │                       │
        │ idle line             ▼  length 40 (16 on Argus) → DATA · 12 → CONTROL · else → junk, counted
        ▼                       │
   the ranging timer's           ▼  CRC-16-CCITT (the hardware unit) → miss → RESEND, once
   capture = the arrival        │
                                ▼  the slot check: the arrival against the slot's window; the header's
                                │  TYPE|NUM and slot against the port table
                                ▼
                   THE CIRCULAR BUFFER — per unit, indexed by frame number, a two-gate FIFO:
                   96 frames on a Bifrost, 16 on an Argus
                                │
                                ▼  a slot leaves at its own index + the delay — as the frame, or as a FILLER
                                │
                   THE STAMP — unix.3..1 written in front, the five reserve bytes zeroed over
                   the port CRC, the trunk CRC computed over all 46
                                │
                                ▼
                   THE TX FIFO — 4 trunk frames, the whole card ──DMA──▶ the up USART
```

**Receive.** Every port USART receives by circular DMA into a per-port buffer; the idle-line
interrupt closes a frame and hands its length and its capture time to the port's receive
routine. The frame is never copied: the buffer it landed in is the buffer that goes up.

**Three lengths, one dispatch.** 40 B (16 B on an Argus segment) is DATA and goes to the circular buffer;
12 B is CONTROL and goes to the control plane (§10); any other length is junk — counted in the
port's `HEALTH`, the buffer freed, nothing answered.

**The CRC miss.** A DATA frame with a bad CRC gets one `RESEND frame · unix.0` in the unit's own
next block — its second frame-time — and the buffer's slot stays open for the repeat; a second miss
on the same frame is not chased: the slot leaves as a filler with reason *arrived corrupt*. A
CONTROL frame with a bad CRC is dropped and counted; the master times out and asks again.

**The slot check.** The frame's arrival must fall inside the window
`round start + (slot − 1) × width ± 2 guard bytes`; its header's `slot` must be the slot the
table holds for its `TYPE|NUM`; a frame that passes the CRC and fails either is a unit talking in
the wrong place — counted, dropped, and the unit put on the ladder after three in a row.

**The circular buffer.** Per unit, indexed by `frame` modulo the depth — a two-gate FIFO: a frame
is written when it arrives, in whatever order, and read at a fixed instant, **its own index plus
the card's delay — 96 frames on a Bifrost, 16 on an Argus**. With the unit's own 32 (or a sonde's
16 and its Argus's 16) every record reaches the head 128 frames, one second, after it was
measured (`../core/blocks/nodbus.md`). A repair — `RESEND` after a CRC miss, a few frames — lands
well inside the delay. The buffer is 8 × 96 × 48 B ≈ 37 kB, in SRAM1.

**The filler.** A slot whose frame is not there at its instant goes up as a DATA frame under the unit's address,
`kind` 3, the reason code in payload byte 0 — 1 *unit missing* (silent in its slot) · 2 *no data
arrived* (the slot was heard but empty) · 3 *arrived corrupt* (two CRC misses) · 4 *slipped*
(the `unix.0` check failed) · 5 *card unlabelled* — and zeros behind it. **The slot is never
given up**: a dead unit costs one filler a frame for as long as the round runs.

**The trunk.** A Bifrost's up link is point-to-point and the card is the only talker in its
direction, so the "trunk TDMA" is the card's own transmit order: frames leave the FIFO in the
order the circular buffer released them, back to back, 48 B each at 2²¹ — 23 % of the link at the full
eight units. CONTROL frames to the head (§10) go into the same FIFO ahead of data. An Argus's up
link is a NodBus slot and the card fires in it like any unit (§8).

**What the head sees.** Every trunk frame carries the whole second, the unit's `frame`, its
`TYPE|NUM`, its slot, its kind and its status, and 32 B the card never read. **The card
recomputes the CRC rather than appending a correction**, so a frame never carries an original and
a fix side by side; what that gives up — the node's own CRC surviving to the head — would guard
the frame against an SRAM upset in the buffer, about one bit in two thousand years a station.
**Every frame carries its own second**, not one anchor frame a second, so a lost frame costs
itself and not the 127 behind it — 8 B a frame, ~4 % of the trunk. **No path field**: port 2
is Bifrost 2 by construction and a slot is a time the card handed out (`../core/PROTOCOL.md` §2). The card's own
frames — the segment table, its `GET` answers, its `PORTS` frame — carry the card's own `TYPE|NUM`.

## 7. The segment table

After `FLOOR`, and on any population change, the card sends **one 32 B payload from its own
address, `kind` 4: 8 × 4 B, one row per slot** — `unit address · port · rung code · status`, in
slot order, zeros for an empty slot. The status byte: 0 running · 1 unranged · 2 on the ladder ·
3 dead · 4 unknown board on the port · 5 port faulted. The tag of each unit follows on request,
`GET` per slot — the head asks for it once, after the table, to renumber what collides.

**The card's `PORTS` frame, from its own address as well — `kind` 7, once a minute and on
`GET PORTS`, in the card's map:** bytes 0–19 the five sockets' `VBUS` and `CURRENT` as raw
`INA238` registers, 4 B a socket, the up port first and the four down ports in port order, an
empty socket zeros; bytes 20–27 the leak watch of down ports 1–4 — the second `INA238`'s raw
`VBUS` where a 300 V source board is seated, 0 elsewhere (`../galvani/HARDWARE.md`, *Leak watch*); bytes
28–29 the card's own temperature, int16 in 0,01 °C; 30–31 reserve. The card sends no `REPORT`
about itself — its arrived feed is socket 0. It goes into the FIFO like the segment table and is
held two deep for `RESEND`. **`GET HEALTH` answers a `kind` 8 frame** in the card's map — the
link-error counts per port. **A unit's `REPORT` frame is a port
frame**: the card prepends the second and passes it, reading nothing. **As an Argus, the card is the one hop that reads `SUB`**: a CONTROL frame to one of its
NUMBERs with `SUB` ≠ 0 is for the sonde `SUB` names — the card writes `TYPE|NUM` ← `SUB`,
`SUB` ← 0, recomputes the CRC and puts the frame in that sonde's third frame-time; the sonde's
`ACK`, `ERROR` or byte answer comes back with `TYPE|NUM` ← the NUMBER and `SUB` ← the sonde.
**It tiles its sondes' report payloads and `kind` 5 and 8 answers by position under the NUMBER
that owns each sonde**, in that NUMBER's own block behind its DATA frame, the other positions
zeros; a `GET REPORT` to the NUMBER with `SUB` 0 makes it ask its four sondes and send the
quartet. **And it carries the sondes' headers in its own**: the NUMBER's frames fly the OR of
the four sondes' ALARM, SUPPLY and FAULT, and the most severe of their state codes in the
protocol's order, a missing sonde as 2 SENSOR (`../core/PROTOCOL.md` §1, §5).

## 8. The Argus layer

An Argus is the same card with the clock coming in on its up port and the tiling switched on.

**Up.** The up port is a NodBus unit's link and the card runs §9 on it: RC at 38 400, its tag in
its `DISCOVER` reply, `ASSIGN_ADDR` by tag, `GET slots` answered with ⌈sondes/4⌉ — computed from
the sondes it has already found on its segments, so **an Argus runs its segments' floors before
it answers its own sweep**, and answers 1 until it has. HSE is the up port's 2²²; `BUSCFG` and
`SYNC` from its Bifrost load its `unix.0 · frame` and it fires its N slots off TIM2 compares
like any unit. The ranging turnaround on the up port is TIM5 in one-pulse mode on PA0/PA1, the
constant `CCR` a number the Bifrost knows; the route the Bifrost writes back as `ROUTE` is
applied at the Argus's `SYNC` like any unit's.

**Down.** Four segments, 16 B frames, 8 B payloads. The **sync rung is 2¹⁹ on all four**, out of
`MCO1` at prescaler 8 through the quad buffer — it is the card's role that names it and no
port's `ID` is read to pick it (`HARDWARE.md`) — and a sonde divides it to its grid by whole
powers of two; the **data rung** is the same pair as everywhere, 2²⁰ alone and 2²¹ chained. The floor (§4) runs unchanged, with `DISCOVER` replies and `slots` counted per segment
and the eight-sonde ceiling counted across the card.

**The tiling.** Each sonde is given, at enrolment, a **position** 0..3 in one of the card's
upstream NUMBERs — the segment table's row says which NUMBER and which position. The circular
buffer is indexed by period: a mini frame that arrives lands, payload untouched, at its position in
the upstream frame of the period its `unix.0 · frame` names; the upstream frame leaves at its
index plus the Argus's 16, an absent sonde's position riding zeros. **No byte is
added** — the frame's own header times all four, and the segment table decodes them.

**Stamping on an Argus** is the unit's: the card's `unix.0 · frame` on the upstream frame is its
own count from its `SYNC`, and the Bifrost above completes the second. Each sonde holds the route the Argus
ranged and wrote to it (§4, step 8 — the segments on the same `RANGE_INTERVAL` and `RANGE_WIDTH`, per segment) and places its grid that much early; the Argus holds the route its Bifrost wrote
and does the same on its up port. So a sonde's sample instant is on the Argus's grid, the
Argus's grid is on its Bifrost's, and the mini frame lands in its period with no arithmetic.
Two hops, no subtraction.

## 9. The card as a unit — what an Argus does on its up port

This is the unit contract (`../core/PROTOCOL.md` §3, §7), restated for the one card that has to
run it:

- **No clock on the up port:** RC, 38 400, silent; the segments stay fed and running if they
  were — their sondes are not the Bifrost's business — and their frames age out into fillers
  the Argus holds, so a returning up link gets nothing stale.
- **A `DISCOVER` at 38 400** is answered with the tag after `tag × 10 µs`, listening first on
  `RXD_ECHO` (PD9), backing off on a bad echo.
- **Clock and a persisted set:** HSE locked, the USART on the persisted rung, listen; a
  `DISCOVER` at 38 400 still wins. Phase from the first readable frame — a neighbour's on a
  shared segment, the Bifrost's `TICK` on a point-to-point port — and the card fires in the next
  round with `status` 1 REJOINED on its first frame.
- **Every frame the card sends is heard on `RXD_ECHO`** and its CRC checked; a miss is
  `NIC_ERRC_BUS` counted and the frame held for the `RESEND` that follows.
- **`END` on the up port**: the card relays `END` to every port, lets the units answer, drops
  their clocks and then their feeds (§10), and only then goes into its own deep sleep. It wakes on
  the next start bit on the trunk — no wire and no pin (`../core/PROTOCOL.md` §7).

### Entering the deep sleep — the pins

**Stop keeps every output at the last level written to it; Standby would float them.** So the
card does not simply stop — it sets each pin to the level it is to hold for the whole sleep, and
only then enters Stop:

| what | held at | why |
|---|---|---|
| the four down ports' `ENABLE`/`LINE_EN` (one GPIO a port) | **low** | the feed and the line side dark |
| the quad buffer's four `OE` (`OE_1`–`OE_4`) | **low** | its outputs Hi-Z and held by their pull-downs; no clock leaves the card |
| `MCO1` on PA8, the port clock | **the pin a GPIO driven low** | Stop halts HSE; the pin is taken off its alternate function first, so its level is set and not wherever the clock stopped |
| the four down ports' `DE` | **low** | |
| **the up port — `LINE_EN` high by its socket's 10 kΩ, `DE` low, `TXD` idle high, `UART4` with its wake-on-start-bit armed** | **untouched** | **it is the wake.** On an Argus its line side is a Galvani board that has to stay lit; and a `TXD` held low is not silence but a **permanent break** at the far end's receiver |
| the echo receiver, `USART3` | off | |
| `I2C1` (the label) · `I2C3` · `I3C1` · `I3C2` | **released, never held low** | a line held low stops a shared bus for everything on it |
| the two `THVD1450` time-bus receivers | `RE#` tied low on the board, nothing to set | **1,4 mA** — a GPIO to switch them would save 4,6 mW and there is none (`HARDWARE.md`, *The card's supply*) |

**On the way back the order is the reverse**: the receivers enabled first, then the clock looked
for, then the ports as `FLOOR` brings them up. **The head raises Kronos before the cards**, so a
Bifrost finds its clock on the ribbon when it looks for it.

## 10. The control plane

**Down the link, CONTROL frames addressed to a unit are relayed, not interpreted.** The card
looks `TYPE|NUM` up in its port table — **as a Bifrost it never reads `SUB`** — queues the 12 B frame (or the 40 B `TUNNEL` or `kind` 5 frame — a wide `SET`) for
that port, and sends it in the unit's own block — its third frame-time, the master's asking
slot — with `DE` raised for the frame. The unit's answer arrives in its fourth frame-time and
goes up the FIFO ahead of data. A broadcast (0xFF) goes to every `SYNCED` port in the same
round. An address the table does not hold is answered by the card with `ERROR`, code *no such
unit*. **One request open per unit**: a second command to a unit whose answer is still owed
waits in the queue; the timeout is the master's, not the card's.

**Addressed to the card itself** (TYPE 2 or 3, its NUMBER):

| op | what the card does |
|---|---|
| `FLOOR` | §4 on every port; the segment table when done |
| `PORT_PWR port · 0/1` | `EN` of that port (0 = all) low or high, at once, its `OE` with it; `ACK`, and the segment table re-sent with the slots' status so the master sees it happen. A port turned off leaves its units' slots on filler with reason *unit missing* |
| `PORT_CLK port · 0/1` | the port's `OE` low or high: its gate in the quad buffer shut or opened, the feed and the line side untouched. The unit mutes on the loss and rejoins on the return by itself; the card does nothing else |
| `END` | relayed to every port first and the units given their answer, then the ports' clocks stopped and their feeds cut, then the card's own deep sleep — the circular buffer is flushed up the trunk before it goes. It wakes on the next start bit on the trunk; the units come back on their feeds, which is a boot for each. There is no `ENABLE` above a card, so this is the only off it has. The next command on the up link wakes it |
| `HARD_RESET` | the flash cells' port sets cleared; the card's own NUMBER back to default; then a reset. The units are not touched — they keep their own sets and are found by the next sweep |
| `GET reg` | up to 16 bits answered under `GET`; a wider register as a DATA frame, `kind` 5, the register number first — and a wider register is written the same way, a `kind` 5 frame down, `ACK` back |
| `SET reg · value` | written, then `ACK`; the registers below |
| `RESEND frame · unix.0` | the card's own frames — the segment table, a `kind` 5 answer, a `PORTS` frame — re-sent from a two-deep hold; port frames are not held after they leave the FIFO, and the head does not ask for them |

**The card's registers.**

| register | r/w | what |
|---|---|---|
| `0x0002 STATUS` | r | the card state (§3), the time state (§5) |
| `0xFF00 VERSION` | r | firmware version |
| `0xFF01 IDENT` | r | the house code |
| `0xFF02 TAG` | r | CRC-16 of the UID |
| `0xFF03 slots` | r | an Argus: ⌈sondes/4⌉; a Bifrost: 0 — it takes no slot |
| `0xFF04 HEALTH` | r | link-error count on the up link; wider: per port CRC misses, junk, slot-check failures, PPS-K glitches, label slips |
| `0xFF13 DELAY` | r/w | an Argus: the 32 its Bifrost writes at floor-up — it holds 16 and writes 16 to each sonde; a Bifrost: 0, it takes none |
| `0xFF10 + port` | r | port state (§3), the two `ID` codes, the `INA238` bus voltage and current |
| `0xFF20 + port` | r/w | **the per-port current limit**, in the `INA238`'s shunt LSBs — the one sanctioned `SET` across every port-carrying board. Written to the part's threshold register and to the flash cells |
| `0xFF30 + port` | r/w | the rung pin: 0 by count · 1 pin 2²⁰ · 2 pin 2²¹ |
| `0xFF40 + port` | r/w | **the `DELAY` written to the port's units** — 32 by default on a Bifrost, 16 on an Argus, never below 8; the card holds 128 less it (an Argus: 32 less it) and the path stays 128; persisted |
| `0xFF50 + slot` | r | the unit's ranged route in 2²⁷ ticks, as written to it, and its tag |
| `0xFF60 + port` | r | the port's `PGOOD`, `SD`, `ALERT` at the moment of the read |
| `0xFF70 + port` | r/w | **`RANGE_INTERVAL`** — uint16, seconds between re-rangings of the port's units; 0 never. Default 60 where the port's `ID` names a Galvani board, 0 on an in-box cable; persisted |
| `0xFF80 + port` | r/w | **`RANGE_WIDTH`** — the return pulse's width the units are asked for, in ticks of 2²⁷ (a unit at 2²⁸ doubles the count); default 128, 0,95 µs — clear of every transceiver's and module's rise time and inside every gap; persisted |

**Up the link, on the card's own initiative:** the segment table after `FLOOR` and on any
change — a port killed, lost or unknown is a slot's status in it (3 dead · 4 unknown board ·
5 port faulted) with the `FAULT` flag up in the card's own frames' header until it clears — the
`PORTS` frame once a minute, and nothing else unasked. The boards found per port at boot, the
port fed or not, and a route moved by more than 2 ticks at a periodic ranging (counted per
slot) are `HEALTH`, read on `GET HEALTH`; the head reads `0xFF50 + slot` when that count has
moved. `ERROR` only answers a command. A Bifrost's tables ride the trunk FIFO; an Argus's ride
its own slots.

## 11. Faults

**The rogue-unit ladder runs per unit, on the card, with nobody's permission.** The master is
told at every step and can override with `PORT_PWR` / `PORT_CLK`; nothing waits for it.

| # | trigger | action | what goes up |
|---|---|---|---|
| 1 | a unit silent in its slot, a slipped `unix.0`, three slot-check failures, two CRC misses on one frame | mark faulty; filler with the reason every round | fillers |
| 2 | — | wait **N = 8 rounds** (62,5 ms at 128 Hz): the unit resets and rejoins on its own; most faults end here | the filler stops |
| 3 | still silent | `PORT_PWR` off, **1 s**, `PORT_PWR` on: the unit boots cold, hears the clock, rejoins. **On a shared segment this takes the neighbours down too** — accepted; on a point-to-point port it costs nobody | the table re-sent, that slot's status 5 *port faulted*; `FAULT` up |
| 4 | still silent after **4 s** | repeat step 3, three attempts in all | the table each time |
| 5 | still nothing | **dead**: a standing filler with reason *unit missing*; the port stays fed; `DISCOVER` once a minute resumes on the port only if every unit on it is dead | the table, that slot's status 3 *dead*; `FAULT` up |

**The port-level faults act on the port, not the unit.**

| trigger | action |
|---|---|
| a port's `ALERT` pin — `ALERT_0` (PC8) for the up port, `ALERT_1`–`ALERT_4` (PD4 · PD5 · PD10 · PD11) for the down ports, the port being the pin: its `INA238` reads a shunt over-voltage above the port's limit, or a bus under-voltage | `EN` and `OE` low within **1 ms** (the EXTI handler does it), the table re-sent with the slot's status 5 *port faulted*, `HEALTH` port · *overcurrent* / *collapse*, `FAULT` up; the port stays off until `PORT_PWR` or the next `FLOOR`. Three trips in an hour and the port is not re-fed by `FLOOR` either — `PORT_PWR` from the head only |
| `SD` high on an optical port (no light) | the port's units go on the ladder at step 1 with reason *no light*; the table re-sent, `FAULT` up while it is dark and down again when the light returns |
| `PGOOD` low on the down-port buck | `HEALTH` *down rail*, `FAULT` up; every down port's units to filler; the card keeps its up link and waits for `PGOOD` |
| a `DISCOVER` reply heard on a `SYNCED` port, or a frame outside every slot for a whole round | *foreign talker*: counted, reported once a minute, ignored |
| the echo receiver hears the up link's own frame with a bad CRC (Argus) | `NIC_ERRC_BUS` counted; the frame held for `RESEND` |
| a port's `ID` reads 0,00 | the port is never fed; `HEALTH` port · *shorted ID*, `FAULT` up |
| an `ID` in no window | not fed; the table's slot status 4 *unknown board*, `FAULT` up |

**The babbling idiot is written down and not solved.** A driver stuck on at a unit holds the
segment; the card sees the CRC storm, reports it, runs the ladder to step 5, and the segment is
found on site by halving.

## 12. Persistence

The card keeps its state in the H523's own flash, in the fixed append-only cells the node
contract defines (`../core/PROTOCOL.md` §7): a cell is id, value, check; writes append; the
reader keeps the last good cell per id; a full 8 KB sector erases and rewrites the live set; the
live set is rewritten once a year on its own.

| cell | content | written |
|---|---|---|
| the card's set | its own NUMBER (an Argus), the up rung, `BUSCFG` as received | at the up link's sweep |
| per port | the tags enrolled with their NUMBERs, slots and positions; the rung pin; the units' `DELAY`; the current limit; `RANGE_INTERVAL`; `RANGE_WIDTH` | after `FLOOR`, on any `SET` |
| the routes | per slot, the ranged route in ticks | after floor-up ranging; on a change of more than 2 ticks at a periodic one |

**The set carries one check over the whole of it, and a set that fails is no set** — the card
boots as fresh and waits for `FLOOR`. Nothing else is stored: no log, no history, no time.

## 13. The processor's budget

| resource | used | of |
|---|---|---|
| USARTs | 6 — UART4 up, USART3 echo, USART1 · USART2 · UART5 · USART6 down | 7 |
| DMA channels | 12 — RX and TX per port and per up link, plus the echo RX; one for the ADC | 16 (GPDMA1 + GPDMA2) |
| timers | TIM2 timebase · TIM5 up-port turnaround · TIM4 · TIM15 · TIM8 · TIM3 ranging · TIM6 housekeeping; the port clock is `MCO1`, one output for all four ports, and no timer | |
| interrupts, by priority | 0 PPS-K capture · 1 `ALERT` · 2 the idle line per port (a frame has arrived) · 3 the TIM2 compares (`SYNC`, `TICK`, an Argus's slots — the transmit is already started by the timer; the interrupt only queues the next) · 4 the up link's idle line · 5 the label's I2C1 and the power boards' I2C3, I3C1 and I3C2 · 6 the ADC · 7 TIM6. The ranging captures raise none — the timers turn the edge round alone | |
| SRAM | circular buffer ~37 kB · buffers 8 × 48 B · FIFO 4 × 48 B · tables 2 × 512 B · ~4 kB of state | 272 KB |
| flash | the image · the cells 8 KB | 512 KB |
| CPU | < 5 % at eight units — a CRC checked, one computed and eight bytes written per frame, a thousand frames a second; `WFI` between interrupts. A builder may run SYSCLK at ÷2 or ÷4 (`HARDWARE.md`, *Clock tree*) | 2²⁷ |

**No interrupt sits in a timing path.** Every instant the station's precision depends on — the
ranging fire and return, `SYNC`'s and `TICK`'s start bits, the PPS-K edge, a frame's arrival —
is a timer capture or a timer-started transmit; the interrupt that follows reads a number the
hardware already latched.

## 14. What is tested, and where

**The host-testable layer is everything above the pins.** The circular buffer, the stamp arithmetic, the
`unix.0` check, the filler rules, the tiling, the sweep's state machine, the ladder, the
register map, the cells — each compiles on a PC with the pins replaced by callbacks and is
tested there with recorded frames: a frame late by 7, a frame with a bad CRC twice, a label off
by one, a PPS-K edge 3 µs early, eight units on one port and a ninth answering `DISCOVER`, an
Argus with five sondes taking two NUMBERs, an `END` mid-round.

**The board layer is tested on the bench against `HARDWARE.md`, *Bench criteria*.**

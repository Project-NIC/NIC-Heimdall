★ N.I.C. ★

# Palatine — the firmware, described

> **This document is the deliverable, not a description of code.** It says everything the
> node's firmware does, in the order it does it, with the numbers it uses — enough to write the
> build from, and enough to test the build against. What the unit *is*: [`README.md`](README.md);
> the board, the pins, the arms and the address map: [`HARDWARE.md`](HARDWARE.md); the sensors:
> [`SENSORS.md`](SENSORS.md); the ModBus leaf, its rates and the register model:
> [`../core/blocks/modbus.md`](../core/blocks/modbus.md); the frames, the sensor block, the
> tunnel and the node contract: [`../core/PROTOCOL.md`](../core/PROTOCOL.md); the clock and the
> bus start: [`../core/blocks/nodbus.md`](../core/blocks/nodbus.md). Where this document and one
> of those differ, that one wins and this one is corrected.

## 1. What the firmware is

**A ModBus master four times over, and a NodBus unit once.** The firmware polls the sensors its
roster names, on four RTU arms, each at its own interval and on its own second of the schedule; it ships every reply as a
self-delimiting block in the 32 B payload, in the shape the reply came back in; and it passes the
head's tunnel packages onto an arm verbatim and their replies back. In mode A, the base, it
computes nothing on a value — no mean, no gust, no tendency, no unit conversion — and holds no
policy beyond the roster the head wrote into it; mode B, the WMO tables, is `WMO.md`, and what it
adds to this firmware — the roles, the statistics, the `UNIX` second, five registers — is written
there. It is the one unit in the station that carries
an arm, so it is the one that verifies a roster at boot, sweeps the house MODs, and runs a
provisioning session when the head asks.

| the firmware does | on | how often |
|---|---|---|
| polls the roster | USART2 · USART6 · UART4 · UART5, one per arm, DMA | per sensor, by its interval, 1 s to 3600 s, at the phase the schedule gave it |
| drives `PWR EXT` | `EN_X` (PD1); the board's `INA238` on I2C1, its own `ALERT` pin | on a slave's request — Pluvius's `STATUS` bit 1 — and off on its clearing, on `ALERT`, on the ceiling, on a lost slave |
| ships blocks | USART1, one TIM2 compare a round | 128 frames/s, the latest blocks repeated |
| relays the tunnel | any arm | when a `kind` 1 frame arrives |
| verifies the roster, sweeps the MODs | every arm | at boot, on `SWEEP`, on a population mismatch |
| gates an arm | `EN` per arm — PE6 · PE8 · PE12 · PE14, joined to `LINE_EN` on the board | by the arm's gating — ungated or gated (the arm table) |
| reads the power bodies | I2C1 · I2C3 · I3C1, six `INA238`s, two to a controller | once a second |
| sends its `PORTS` frame | the six sockets — socket 0 the up port, 1–4 the arms, 5 `PWR EXT` — `VBUS` · `CURRENT` raw, 4 B each, then its own temperature, from its own address in its block's fourth frame-time behind the data; no `REPORT` about itself | once a minute, and on `GET PORTS` (`../core/PROTOCOL.md` §5) |

## 2. Boot

| # | step | what happens | if it fails |
|---|---|---|---|
| 1 | reset | `DE` (PE2) low, every arm's `DE` low, every arm's `EN` low — **an arm is dark until its roster is known**; the up port's line side needs nothing — its `LINE_EN` is a 10 kΩ pull-up in the socket; the IWDG at **1 s** | — |
| 2 | RC | HSI; the flash cells read (§11): the set, the arm table, the roster with its phases, the QC thresholds; the tag computed — CRC-16-CCITT over the 96-bit UID | a set that fails its check is no set: the node boots as fresh, with no roster and every arm dark |
| 3 | the rail | `PGOOD` (PD14) read | low: `HEALTH` *rail*, `FAULT` up, the node runs |
| 4 | IDs | ADC1 reads the eleven `ID` inputs once, 16× oversampled, ratiometric: the up port's two bodies, each arm's two and `PWR EXT`'s one — on an arm a ModBus communication board and an arm power board, or 1,00 empty, or 0,00 shorted | an arm whose data body reads 1,00 is *unfitted* and never fed; a shorted `ID` is *shorted* and never fed; a ratio in no window is *unknown board*, not fed |
| 5 | the power bodies | I2C1, I2C3 and I3C1, **two sockets to a controller at 0x40 and 0x41 — the socket's `A_SEL` strap decides which**: each `INA238` present configured — the thresholds from the arm table's current limit, the arm's load measured at commissioning with margin, `APOL` = 1 so the alarm is the high level, `ALERT` latched; a pin per socket: `ALERT_P` (PC8) · `ALERT_1`–`ALERT_4` (PC9 · PD10 · PD11 · PC12) · `ALERT_X` (PD2), EXTI 8 · 9 · 10 · 11 · 12 · 2 | an `INA238` silent where `ID` names an arm power board: `HEALTH` *no telemetry*, `FAULT` up, the arm still fed |
| 6 | the arms | for each arm in the arm table: `EN` high, **200 ms apart in arm order**; the USART at the arm's rate, 8N1, DMA RX into a 256 B buffer, the RTU gap timed on TIM6; **2 s** of warm-up | — |
| 7 | verify | every roster entry read once (§6); the reply's presence and, for a house MOD, its `0xFF01 IDENT` and `0xFF02 TAG`, compared with the roster | a mismatch — a sensor missing, a tag changed, a house MOD at a default address — is a population change: `HEALTH`, `FAULT` up per entry, and the node announces at RC as a fresh unit does (`../core/PROTOCOL.md` §7) |
| 8 | the sweep | the house MODs' default addresses read (§6); a MOD found at a default is given the lowest free NUMBER of its type | two MODs of one type at the default collide and neither answers cleanly: `HEALTH` *collision on arm n*, `FAULT` up, the operator plugs them one at a time |
| 9 | enrol | §4 | — |

**An arm is fed only once its roster is known.** A fresh node with no roster feeds nothing: the
head writes the roster over `SET` (§8), the node persists it, and the arms come up. That is the
one place a unit in this station waits for the head to say what hangs on it, and it is because a
polled bus cannot be discovered — a roster is verified, never extended, and only the house MODs
are the exception (`../core/PROTOCOL.md` §7).

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

`RC` — the HSI, the USART at 38 400, transmitting nothing; **the arms poll in every state from
step 6 of boot on**, so the latest blocks are ready the moment the node may speak. `ENROLLED` —
NUMBER and slot held, still at 38 400. `LOCKED` — `BUSCFG` taken, HSE on the wire, PLL1 at 2²⁷,
the USART on the rung; nothing sent. `RUNNING` — `SYNC` loaded the index; the node fires every
period with the latest blocks. `ENDED` — on `END`: the arms' polling stopped, the last blocks flushed and
answered, every gated arm's `EN` low, the ungated arms left fed (a sensor that loses its settling
when cut is not cut here either — the port's own `ENABLE` is what ends it), the USART listening;
if the node is brought back, polling resumes. **Clock lost** is a fault: the CSS raises an NMI, the node drops to the HSI,
mutes, and comes back through the rejoin (§4); the arms poll on.

## 4. The bus — the unit's side

**The enrolment** is the node contract, as on every unit: `DISCOVER` at 38 400 answered once
with the tag in the time bytes after `tag × 10 µs`, listening first on `RXD_ECHO`, backing off
on a bad echo; `ASSIGN_ADDR` by tag gives NUMBER and slot, `ACK`; `GET slots` answers **1**.
`BUSCFG` (count, rate code 7, rung) locks HSE onto the wire, PLL1 to 2²⁷, the USART to the
rung; the deadline is `round start + (slot − 1) × width`. `SYNC`, on a second's boundary, is
captured on TIM2 CH2 (PB3) and loads `unix.0 · frame` at that edge less `ROUTE`, the written
route in ticks of 2²⁷, so the grid is the card's (`../core/PROTOCOL.md` §7).

**The slot.** One TIM2 compare a round starts USART1's DMA transmit of the 40 B frame the
block ring assembled (§7); `DE` up for the frame, down after the last stop bit by the USART's
`DEAT`/`DEDT`; the echo on USART3 checked when it ends; a miss counted in `HEALTH` and the
frame held in the circular buffer of `DELAY` frames.

**The gaps:** `GET`, `SET`, `RESEND` and `HARD_RESET` in the node's block, as §8; **`kind` 1
TUNNEL** frames in the gaps are the arms' (§7). `TICK` ignored while running; the phase source
on a rejoin. **Ranging** on `SET RANGE`: after the next DATA frame PB6/PB7 switch to TIM4, `DE`
up, one-pulse mode — CH2's capture starts the counter, CH1 raises the return at `CCR` =
**256 ticks** of 2²⁷ (1,91 µs) and drops it at the width the card asked for; CH2 in PWM-input
mode captures the incoming pulse's width for `STATUS`; the pins go back. No interrupt in the
path. **The arms are never ranged** — ModBus carries no clock and no ranging
(`../core/blocks/modbus.md`).

**The rejoin** after any reset: clock and a persisted set → HSE locked, the USART on the rung,
listen; a good CRC proves the rung, a second of nothing tries the other; the phase from a
neighbour's frame or the card's `TICK`, captured on TIM2 CH2; the node fires in the next round
with `status` 1 REJOINED. The block ring is what it was; nothing on the arms restarts. No clock →
RC, silent. No set → 38 400, silent, a sweep.

**`END`**: polling stopped, the last blocks flushed and the answer sent, the gated arms cut, the
USART listening — if the node is brought back the gated arms return with their warm-up and the
others were never cut. **When the clock drops, every arm's `EN` goes low first**, because a node
stopped on its watchdog cannot watch an arm; the returning feed is a boot and the arms come up as
at boot. No rail on the board is switched (`../galvani/README.md`, *Three states*); an arm's `EN`
is a port's kill, not a rail.

## 5. Time on the node

**One 32-bit timer, never reset.** TIM2 counts 2²⁷ from PLL1 and is read as differences. Two
things hang on it: CH2's capture of every start-bit edge on `RXD` (the phase source) and the
compare that fires the transmit; a compare every 2²⁰ ticks advances `frame`. **No sample on this
node has a time finer than the frame it is shipped in**, and none needs one: the arms carry no
clock, a reply arrives when the sensor answers, and the value is a quantity moving over minutes
(`../core/blocks/gps-pps.md`, *The precision contract*). A block's time is the frame's; the head's
recording rule — `CHANGE` per block by default — selects what is kept of it.

**TIM6 is the arms' clock**: at 2²⁷ ÷ 128 it counts 2⁻²⁰ s; the RTU inter-frame gap of
3,5 characters (**1,82 ms at 19 200**, scaled per arm's rate), the reply timeout and the four
arms' poll schedules are all its compares. The node holds no wall clock: the schedule's second
is the count of `frame` = 0 boundaries since `SYNC`, modulo 3600, and the head sees the hour of
the schedule land wherever it lands.

## 6. The arms — the roster, the sweep, the poll

**The arm table** — four rows, written by the head, persisted:

| field | content |
|---|---|
| fitted | 0 · 1 |
| rate | 9 600 · **19 200** — one rate per arm, set per arm; a baud-locked sensor gets an arm at its own rate |
| gating | **ungated** — fed always · **gated** — fed for the poll window of its hourly sensors and cut between, warm-up in seconds |
| current limit | the arm power board's `INA238` thresholds to programme — the arm's start-up peak and running load, measured at commissioning, with margin; rewritten with `SET ARM` when a sensor is added |

**The roster** — up to **64 entries**, written by the head, persisted, verified at every boot:

| field | content |
|---|---|
| arm | 1..4 |
| address | 0x01..0x0F a bought sensor · `TYPE«2 \| NUMBER` a house MOD or a Babel position (`HARDWARE.md`, *The Modbus address map*) |
| function | 03 holding · 04 input |
| start · count | the contiguous run to read, count ≤ 15 registers — one block of ≤ 30 B; a sensor whose values are not contiguous is two entries, two blocks |
| interval | how often it is read: **1 · 2 · 5 · 10 · 30 · 60 · 300 · 600 · 1800 · 3600 s** — every one a divisor of the hour, so the schedule repeats every 3600 s; **0,25 s for a wind entry in mode B**, four reads in every cell (`WMO.md`) |
| phase | the second within the interval at which it is read — **the node's, not the head's**: computed when the entry is written (*The schedule*, below), persisted with it, read back with it |
| tag | for a house MOD, the tag read at commissioning; 0 for a bought sensor |
| quirk | the exception entry of `../core/blocks/modbus.md`, *The learning mode*: a save register, a config window — carried, not used at run time |

**The poll.** Each arm runs its own state machine on its own USART and DMA, all four
concurrently: take the next due entry of this arm; `DE` up; the 8 B request (address, function,
start, count, CRC-16) by DMA; `DE` down after the last stop bit; wait for the reply — a frame
ends when the line is idle for the RTU gap on TIM6 — with a **timeout of 100 ms** for a bought
sensor and 20 ms for a house MOD; check the CRC; on a good reply write the block into the ring
(§7) under the entry's address; on a bad CRC or a timeout retry **once**, then mark the entry
*missed* this round and count it in `HEALTH`; the RTU gap; the next entry. An entry is due in
the second where the schedule's second minus its phase is a whole number of its intervals; an arm
serves the entries due in a second in roster order. At 19 200 a 15-register read is 8 B out and
35 B back, **~25 ms with the gaps** — the time of a read is its own, 8 + 5 + 2 × count bytes at the
arm's rate plus two RTU gaps.

**The schedule — the reads spread over the hour, so no second is crowded.** Each arm keeps a
table of 3600 one-second cells, each holding the read time already placed in it. When `SET
ROSTER` writes an entry, the node tries every phase from 0 to its interval − 1, and for each the
cells the entry would land in — phase, phase + interval, … through the hour; it takes the phase
whose fullest cell is emptiest, the lowest phase on a tie, and adds the read's time to those
cells. **No cell may pass 800 ms**, leaving a fifth of the second for a retry and the tunnel;
an entry that fits under that at no phase is refused by `SET`. A 600 s entry therefore lands in
the second the fewest others share, and ten sensors at ten minutes are ten reads in ten
different seconds, not ten in one. Clearing the roster clears the table; the phases are
recomputed only when the roster is rewritten, so a sensor keeps its second.

**The gated arm.** A gated arm's entries are placed the other way round: **packed from phase 0
upward into consecutive seconds**, so the arm is fed once a window and not once a sensor. Its
`EN` goes high **warm-up seconds** before the first of them — the warm-up is the arm's, sized to
its slowest sensor, 60 s the default — and low when the last has been served. Its `INA238` is
read while fed; between windows the arm draws nothing. A gated arm takes no entry under 600 s:
**`SET` refuses it**, because a sensor read more often than its warm-up allows is not one worth
cutting. A sensor that must never
be cut is on an ungated arm; the head chooses at commissioning.

**The sweep — house MODs only.** At boot after the verify, and on `SWEEP`, each fitted arm's
USART reads `0xFF01 IDENT` at the **default address of every house type**, `TYPE«2 | 3` for
TYPE 4..61 — 58 reads, **~1,5 s an arm** at 19 200, the four arms in parallel. A MOD answering
at a default is given **the lowest NUMBER of its type not in the roster**, written into its
house register `0xFF05 NUMBER` (FC06), read back at the new address, and added to the roster
under its type's profile with the tag it reports; the head is told by `FAULT` and `HEALTH` and reads the
roster. A Babel presents each fitted position as its own slave on its quantity's type; each
takes a NUMBER of that type, and the shared `IDENT` is what the head uses to know they are one
board (`../babel/MODBUS.md`). **A bought sensor is never swept for and never written to by
this firmware**: its address is written in the learning session, by the head, through the
tunnel.

**The learning session** (`SET PROV arm · rate`): the arm's polling pauses, its USART takes the
commanded rate, and the arm becomes tunnel passthrough only; the head tunnels the type's FC06
writes and their readbacks; `SET PROV 0` closes the session and the arm returns to its rate and
its roster. The node composes nothing in a session and parses nothing — it is a byte pipe with
a paused poll.

### The `PWR EXT` body — the switched supply, and the rule that drives it

**`PWR EXT` is one power body, one load, and the rule that drives it is the load's.** The body is read
at boot like every other: its `ID` says which board is seated (the switched 24 V board reads 0,30) and the
`INA238` is programmed with the load's thresholds. A slave cannot speak first on ModBus, so **the
request rides in the answer this node asks for anyway**: the load's unit sets a bit in a register
the node already polls, and the node acts on it in the same round. One poll period of latency,
and no traffic added.

**The first rule is Pluvius's drain**, and it is the one the station builds. **A Pluvius is a
10 s roster entry** — its `STATUS` is read every ten seconds, so the request and its clearing are
seen within ten seconds, and nothing asks for more: the vessel is a buffer, a few seconds of the
head at either end are nothing to the measurement, and the rain totals are minute quantities:

1. `STATUS` bit 1 `PUMP` read as 1 on a Pluvius in the roster, and `PWR EXT` free → `EN_X` high, and
   `HEAD` (`0x0014`) written 1 to that Pluvius, so its dead-drain window starts when the head
   actually runs and not when it was asked for.
2. While on: the `INA238`'s `ALERT` **over** the limit is a stalled head, **under** the minimum is a
   head running dry or a cut cable — `EN_X` low, `HEAD` written 0, `HEALTH` counts it, `FAULT` up
   says which. A ceiling of **12 minutes** — Pluvius's own 10 plus margin — ends any cycle. A
   Pluvius that misses **three polls** in a row, 30 s, is switched off before it is declared lost.
3. `PUMP` read as 0 → `EN_X` low, `HEAD` written 0. The unit ends its own cycle at `EMPTY`; this
   node only follows.

## 7. The payload — blocks, and the tunnel

**The block ring.** Every roster entry owns one slot in a ring of 64 blocks, `address · length ·
data`, written on every good reply and **emptied on a miss**. The 32 B payload is assembled
before each slot by walking the ring from a cursor: **whole blocks only, as many as fit**, in
roster order; the cursor advances past what was shipped; a leading zero address closes the
payload where the blocks fall short of 32 B. The ring is walked continuously, so **every block
goes up every N frames**, N the number of frames one pass takes — a roster of 16 entries
averaging 8 B is four frames a pass, 32 passes a second; the wind read of every second goes up
within 32 ms of arriving. **A value that missed its round is never shipped**: with no age field there is nothing to say how
late it is, so it must not go up looking fresh (`../core/PROTOCOL.md` §5). The entry's slot stays
empty until the next good reply, `status` 2 SENSOR rides the frames while it is, and `HEALTH` says
which entry and why.

**What a block is.** The ModBus reply with the function code and the CRC taken off — the run of
registers as they came back, big-endian, the profile for the address — held in the head's mirror, shipped with the archive — knowing what each
register is (`../core/PROTOCOL.md` §5). A bought sensor's block is its registers as the vendor
laid them out; a Pluvius block is `RATE · HOUR · STATUS`; a Ceres block is `VWC · TEMP ·
STATUS · RAW`. The node never looks inside, but for the one bit a `PWR EXT` rule reads — Pluvius's
`PUMP` (§6).

**The tunnel.** A `kind` 1 TUNNEL frame in the gaps carries a complete RTU package — address,
function, data, CRC — and the node streams it onto the arm the roster maps that address to (a
package for an address in no roster entry goes to the arm a provisioning session has open, or
is refused with `ERROR`). The arm's poll is paused between its transactions for it; the whole
reply comes back as **`kind` 2 TUNNEL_REPLY** in the node's next gap, verbatim, CRC and all; a
timeout of 100 ms is an `ERROR` from the node. One package open per node; the head retries and
validates the CRC (`../core/PROTOCOL.md` §9). Pluvius's `TARE`, `DRAIN` and thresholds, a
bought sensor's offsets, a remote pre-drain — all of it is this path and none of it is a
register on the node.

## 8. The control plane

| op | what the node does |
|---|---|
| `DISCOVER` | answers with its tag, §4 — only when not in a running round |
| `ASSIGN_ADDR` by tag | takes NUMBER and slot, writes the cells, `ACK` |
| `BUSCFG` | takes the item; on the third, locks and computes the deadline |
| `SYNC` | loads the index at the captured edge, starts firing |
| `TICK` | ignored while running; the phase source on a rejoin |
| `END` | §4 |
| `HARD_RESET` | the NUMBER to 15, the arm table, the roster and the schedule cleared, and the QC thresholds to their defaults; then a reset — the node comes up fresh at 38 400 with every arm dark |
| `RESEND frame · unix.0` | the frame from the circular buffer, in the node's own slot |
| `GET reg` | up to 16 bits under `GET`; a wider register as a DATA frame, `kind` 5, the register number first |
| `SET reg · value` | up to 16 bits in `arg1 · arg2`; a wider register — an `ARM` row, a `ROSTER` entry — arrives as a `kind` 5 frame, the register number first; written, applied, `ACK`; a change to the arm table or the roster marks the first affected frame `status` 5 CHANGED |

**The registers.**

| register | r/w | what |
|---|---|---|
| `0x0002 STATUS` | r | the node state (§3), the clock state, per arm: fed · gated-off · session · faulted, the ranging width last measured |
| `0x0010 ARM 1..4` | r/w | the arm table row: fitted · rate code · gating · warm-up seconds · current limit — 6 B, `kind` 5 |
| `0x0014 ROSTER` | r/w | the roster, one entry per `SET`: index · arm · address · function · start · count · interval · tag · quirk — 10 B, `kind` 5; read back with the phase the node gave it; index 0xFF with a zero address clears the roster |
| `0x0015 PROV` | w | `arm · rate code` opens a learning session on that arm; `0 · 0` closes it |
| `0x0016 SWEEP` | w | 1 runs the house sweep on every fitted arm now |
| `0x0017 QC` | r/w | `arg1` the CRC-miss rate that faults an arm, in misses per hundred transactions, default **20**; `arg2` the cool-off in minutes, default 10 — one `SET` |
| `0x001D SITING` | r/w | **the WMO siting classes of the station's instruments** (WMO-No. 8, Vol. I, Annex 1.D), four nibbles: air temperature and humidity · precipitation · wind · global radiation, each `class 1–5` in three bits and the `S` flag in the fourth; 0 = not classified. Written at commissioning, rewritten at every change of the surroundings and at least every five years; persisted, and carried by the head as the station's metadata beside its coordinates |
| `0x001E ROUGH` | r/w | **the roughness class of the terrain upwind**, Davenport–Wieringa index 1–8 per 45° sector from north, eight nibbles — 4 B, `kind` 5; 0 = not surveyed. What the 10 m wind is derived with (`SITING.md`); persisted and carried with `SITING` |
| `0x0021 RANGE` | w | arms the ranging turnaround for the next block, `arg1` the width in ticks |
| `0x0031 MISSED` | r | the roster indices missed in the last pass, as a 64-bit mask, `kind` 5 |
| `0x0039 PORTS` | r | answers with the `PORTS` frame itself, `kind` 7 — behind a barrier socket 0 is the up port's reading, and the SUPPLY flag in the header when it leaves `0xFF0A VIN_WINDOW`; in the box the body is empty, socket 0 reads zero and the code never sets |
| `0x003A HEALTH` | r | answers with the `HEALTH` frame, `kind` 8: the H523's die temperature, per arm the `EN` state and the transactions and misses in the last minute |
| `0xFF00 VERSION` | r | firmware version |
| `0xFF01 IDENT` | r | the house code |
| `0xFF02 TAG` | r | CRC-16 of the UID |
| `0xFF03 slots` | r | 1 |
| `0xFF04 HEALTH` | r | echo/CRC misses, resends served, clock losses, IWDG resets, I²C recoveries; per arm: CRC misses, timeouts, retries, arm faults, sweeps run |
| `0xFF09 SENSORS` | r | the present list: per roster entry, present · missing · faulted — 64 × 2 bits, `kind` 5 |
| `0xFF0B REPORT_INTERVAL` | r/w | seconds between `PORTS` frames, default 60; 0 disables |
| `0xFF10 ID` | r | the eleven `ID` codes read at boot |
| `0xFF13 DELAY` | r/w | the frames the unit holds before it sends — written by its parent at floor-up, 32 on a Bifrost port and 16 behind an Argus, and persisted; never below 8, the image's floor, because `RESEND` is answered from this buffer (`../core/PROTOCOL.md` §7) |

**Up the link unasked:** `ACK` after a critical command and the `PORTS` frame once a minute — and a tunnel reply, nothing else. `ERROR` only answers a command that failed; a fault is the header's `FAULT` flag, and its detail waits in `HEALTH` for the head's `GET HEALTH` (`../core/PROTOCOL.md` §1, §5, §7).

## 9. Health and QC

**`HEALTH` per roster entry**, one vocabulary: OK · NO_RESPONSE · DEGRADED, read on `GET HEALTH`;
any entry other than OK raises the header's `FAULT` flag. `NO_RESPONSE` is two
consecutive missed polls; `DEGRADED` is a house MOD whose `STATUS` register reports a fault bit
the profile marks as such, or a bought sensor answering with a Modbus exception. Held for every
entry from boot. The value-plausibility QC — range, impossible step, persistence —
runs where the archive is read; the node ships numbers.

**The arm-level QC is the one thermal rule this unit has.** An arm whose CRC-miss rate
over the last hundred transactions passes the `QC` threshold is **faulted**: `EN` low, polling
stopped, `HEALTH` *arm faulted* for every entry on it and *arm n CRC rate*, `FAULT` up; after the
cool-off the arm is fed again with its warm-up and polling resumes — three faults in a day and
the arm stays off until `SET ARM` rewrites its row. That is the whole cold protection: a bus whose
transceivers or sensors are past their temperature is not polled into garbage, and it comes
back by itself when it recovers.

## 10. Faults

| trigger | action | reported |
|---|---|---|
| an `ALERT` pin | the socket is the pin, and that `INA238` is read; **that arm's `EN` low within 1 ms** if it is an arm's, the reading latched; the arm stays off until `SET ARM` or the next boot — three trips in an hour and it is not re-fed at boot either | `HEALTH` *arm n overcurrent* / *collapse*, `FAULT` up |
| a roster entry misses twice | `NO_RESPONSE`; polled on at its interval | `HEALTH`, `FAULT` up, `SENSORS` |
| a house MOD's tag differs from the roster's | a replaced module: the entry is marked, the announce runs, the head decides | `HEALTH` *replaced*, `FAULT` up |
| a house MOD found at a default with its type's four NUMBERs taken | not assigned | `HEALTH` *type full*, `FAULT` up |
| a tunnel package with no open session and no roster arm | refused | `ERROR` |
| the CSS fires | NMI: HSI, mute; the rejoin of §4; the arms poll on | `status` 1 REJOINED |
| `PGOOD` low | noted | `HEALTH` *rail*, `FAULT` up |
| an I²C controller stuck | nine clocks and a STOP; that controller's two readings skipped | `HEALTH` |
| the IWDG expires | reset; every arm cut and re-fed by the boot order | `HEALTH` |

## 11. Persistence

The H523's flash, in the node contract's append-only cells (`../core/PROTOCOL.md` §7): a cell
is id, value, check; the last good cell per id wins; a full sector is rewritten; the live set
rewritten once a year.

| cell | content | written |
|---|---|---|
| the set | NUMBER, slot, `BUSCFG`, one check | at enrolment |
| the arm table | four rows | on `SET ARM` |
| the roster | up to 64 entries with their phases, and the present-mask as last verified | on `SET ROSTER`, after a sweep assigned a MOD |
| the QC thresholds | `QC` | on `SET` |
| the siting classes and the roughness | `SITING` · `ROUGH` | on `SET`, at commissioning and at every re-classification |

**No value is stored.** A boot ships nothing until the first poll answers; the block ring starts
empty and the payload is a leading zero until then.

## 12. The processor's budget

| resource | used | of |
|---|---|---|
| UARTs | 6 — USART1 the link, USART3 the echo, USART2 · USART6 · UART4 · UART5 the arms; the LPUART is free | 7 |
| I²C | 3 — the six power bodies, two to a controller: I2C1 the up port's and `PWR EXT`, I2C3 arms 1 and 2, I3C1 (I²C legacy) arms 3 and 4 | 3 + 2 I3C |
| DMA | 11 — USART1 TX, USART3 RX, RX and TX per arm, ADC1 | 16 |
| timers | TIM2 timebase (CH2 the capture) · TIM4 ranging · TIM6 the arms' microsecond clock | |
| interrupts, by priority | 0 the RXD capture · 1 the slot compare · 2 `ALERT` · 3 the link's idle line · 4 the arms' idle lines · 5 TIM6 compares · 6 the three I²C controllers · 7 the ADC | |
| SRAM | the block ring 64 × 32 B · four arm buffers 256 B · the roster 64 × 10 B · the schedule 4 arms × 3600 cells × 2 B, the read time in ms, ~28 kB · the circular buffer 32 × 40 B · ~2 kB of state | 272 KB |
| flash | the image · the cells 8 KB | 512 KB |
| CPU | < 1 %: a payload assembled and a frame sent 128 times a second, a few RTU transactions a second per arm; `WFI` otherwise | 2²⁷ |

**No interrupt sits in a timing path.** The frame edge is a capture and the slot is a compare;
nothing on an arm has a time the station keeps.

## 13. What is tested, and where

**On a PC, with the pins replaced by callbacks:** the poll state machine per arm against
recorded RTU exchanges — a reply cut short, a CRC miss then a good retry, an exception
response, two entries due at once; the schedule — ten 600 s entries on one arm landing in ten
different seconds, an entry refused when no phase keeps its cells under 800 ms, a gated arm's
entries packed into consecutive seconds; the block ring and the payload walk with a roster of 1, 16
and 64 entries, a block of 30 B not fitting and going first in the next frame, the zero
terminator; the tunnel — a package routed by roster, one refused, a reply paired; the
learning session pausing exactly one arm; the sweep against a simulated MOD at a default and
against two colliding; the verify against a changed tag; the gated arm's warm-up and cut; the
QC fault and the cool-off; the register map, every refusal; the cells.

**On the bench, against `HARDWARE.md`:** four arms polling sixteen bought sensors each at
19 200 for 24 h with a miss rate under 0,1 %; the arms fed 200 ms apart on a scope; a gated
arm at zero current between polls on its `INA238`; `ALERT` to `EN` low in under 1 ms; a Pluvius
`DRAIN` tunnelled from the head and its reply back in under 60 ms; `PWR EXT` on within one poll of a
Pluvius raising `PUMP` and off within one of its clearing, `ALERT` to `EN_X` low in under 1 ms; the link's ranging constant
reproduced to ±1 tick over a hundred launches; the node's frame in its slot at 128 Hz for 24 h
with zero fillers on a bench spur.

★ N.I.C. ★

# Core — the graveyard

Rejected alternatives and superseded states, with the reason. Nothing returns from here and
nothing is deleted from it.

## A slave-class MCU — the U073, then the H503 — not kept

The house MODs ran on an `STM32U073` for its price and its `TSC`. The capacitive fronts want a
2²⁷ Hz timer and two differential ADCs (`../ceres/HARDWARE.md`), ranging a fast timer on every
unit (`../bifrost/HARDWARE.md`, *Ranging*), and **one part number is one firmware base and one
tooling**. The cheaper `STM32H503` — one ADC, none differential, a second code — saved a dollar
that did not pay for that. The U073 board layer and profiles (`core/mod/`, `gauss/mod/`,
`palatine/mod/`) went too.

## The GNSS capability spec — superseded by named types

`blocks/gps-pps.md` let any receiver qualify on NMEA + PPS, u-blox M8 the reference. **NMEA + PPS
is a common minimum, not a contract**: boot dialogue, leap-second message and receiver delay
differ by maker (UBX, Unicore's own), so any receiver means protocol tables for chips nobody
fitted and leaves `INT` an unknown of each build instead of a constant of a part. Replaced by
named types — `M8N` on Polaris, `UM980` on Sputnik (`../kronos/polaris/WHY.md`); the capability
list is the bar a type must clear.

## `TYPE«4 | NUMBER` on all three buses — superseded on ModBus

One packing served every bus: TYPE 1..14, NUMBER 1..15, in `0x11..0xEF`. **ModBus runs out of
types, not instances**: its types grow with every quantity a bought sensor cannot deliver
finished, and once a Babel's positions answer as sensors fourteen is the wall; fifteen of one
quantity no station reaches. ModBus went to `TYPE«3 | NUMBER` (TYPE 2..30, NUMBER 1..7), then
`TYPE«2 | NUMBER` (TYPE 4..61, NUMBER 0..3): a plot has twenty kinds of sensor, four of any one,
and `61«2|3` is 247, Modbus's legal ceiling. NodBus and mini keep the nibble; each space is read
per carrying host, so the split costs a table row and no ambiguity.

## The "ModBus over glass" proof — withdrawn

It rested on Pascal, Atlantis Tier 1's gauge, running plain RTU over the optical kit — but Pascal
is NodBus mini type 3 (`../pascal/README.md`). **Our own module on a link that suited us never
generalised to a bought one.** Its tagged, pipelined "RTU burst" went too: classic ModBus does not
know it. ModBus ends at the local leaf (`blocks/modbus.md`).

## Subcommutation — cancelled

`nic-mux` (Bresenham schedule, byte-walk, `NIC_OP_MUXTAB`) outlived its last user as "a canonical
option". No front needs it — seismo keeps slow channels at latest value, the collecting fronts use
tagged records, Sputnik's epoch spreading agrees nothing in advance — and **a mux schedule must be
kept in step at both ends**, the coupling records avoid. Opcode 19 went to EVENT.

## The wait ladder — declined

Option B: after bus silence node *k* waits *k* byte-times, dead slots self-compressing. It is
arbitration in miniature, fails point-to-point where there is nothing to hear, and a deaf node
collides where an anchored one mutes; **holes cost nothing at 25 % occupancy**. Anchored slots
with a busy-hold stand.

## The semantic ModBus bridge — superseded by the pass-through tunnel

Reads rode `GET(class)`, writes CFG ops, the node composing the RTU frame. **No 8 B frame could
carry module + register + value** without multi-frame conventions, and ModBus knowledge sat in the
node. The Mayak composes the whole RTU package, CRC included; the node is a byte pipe, like every
commercial gateway.

## Runtime ModBus autodetect — demoted to the learning mode

Autodetect as the default had the arm hunting rates at every encounter. **A sensor is set once**,
so the sweep belongs to provisioning and running works a verified roster at a fixed per-arm rate
(`blocks/modbus.md`). A provisioning socket on the head was not built: the tunnel provisions any
arm remotely.

## TICK on demand, and the first-boot restart on a returning clock — superseded

Two rejoin rules contradicted each other: the card sending `TICK` on demand and as a re-align, and
a returning unit restarting first-boot-style to wait for BUSCFG and SYNC, **never re-broadcast
mid-run**. Replaced by the rejoin of `PROTOCOL.md` §3: the unit reads its phase off a neighbour's
frame (why the sixth header byte carries the slot) or, point-to-point, the card's once-a-second
`TICK`; the recovery ladder lost re-synchronise, start-alone and release. A slot-trim opcode stays
unwritten: the late fire, ≤ 2,5 µs on 500 m of copper, sits inside the guard bytes and busy-hold.

## The magic byte, `tid|kind` and the opcode-per-slot family — superseded by the final header

The header had a magic byte, `tid|kind` (a 4-bit transaction id; tunnel, reply, error flags), a
spare sixth byte, and the time as `frame · unix.0`, the card inserting `unix.1..3` little-endian
behind `unix.0`. **Length tells frames apart**, so the magic byte carried nothing; one question
has one answer, so the id, its allocator and the depth-8 reply FIFO went; the flags became `kind`
and `status`. The time leads, most-significant-first, so the card prepends `unix.3..1`: **a
little-endian insert copies every frame on the busiest processor**, which sank a uniform
little-endian.

On CONTROL `value · rsvd` became `arg0 · arg1`; `CFG(0..7)`, `GET(0..7)` and `BUSCFG(0..7)`
collapsed into registers behind `GET`/`SET`, `BUSCFG` taking its item in `arg0`. `STATUS` is
`DISCOVER`'s reply. `TOKEN`, a slot grant for repair, went: slots are clock-anchored and a lost
unit rejoins.

## PPS-SEED, the ATTN interrupt, two I²C buses, and the role by firmware and socket — superseded by the time socket

Kronos reached the cards over "three wires on the upstream connector" the 24-pin body had no pins
for, took a seed pulse from the head (PPS-SEED), ran two I²C buses (labels; an ESP-mastered
register link with an ATTN interrupt), and a card's role came from firmware and socket. Replaced
by one time-socket pinout for cards and head, with `ATTN` tied to 3V3 on Kronos against a card pull-down: **the presence level makes the role, so one firmware
serves both** and no Argus jumper is needed. The seed went: a head with only Wi-Fi NTP or a LoRa
beacon has no edge worth disciplining a 0,3 ppm oscillator against, so it writes the coarse second
and Kronos tags the time seeded.

## A unit asking for a repeat of what it received badly — declined

A point-to-point unit with a bad CRC could ask again, but **the master already times out and
repeats**, and on a shared segment units answering one broken broadcast collide. A unit checks its
transmission only where it shares the wire (`PROTOCOL.md` §3).

## Skipping the echo check on a lone unit — declined

A lone unit is already checked by the card's CRC, so a `BUSCFG` `SHARED` item could let it skip
its echo check. **The saving is not worth a second code path in every unit.** A segment's physical
count stays the card's, from the `DISCOVER` replies.

## The address walk and the one-at-a-time rule — superseded by the tag

Walking addresses (two hundred bytes in about a minute) and plugging same-type units one at a time
(two on one default byte garbled each other) went with the tag: CRC-16 over the 96-bit UID, in the
time bytes of a `DISCOVER` reply and an `ASSIGN_ADDR`, unused before `SYNC`. **One broadcast finds
everything.**

Answering at `tag × slot` was declined: **on RC at ±1 % the error is proportional**, so tags
within one per cent collide every round. Ethernet's listen-before-talk with a
rehashed wait stands; a binary search over the tag bits
(1-Wire ROM search, RFID anti-collision) needs more of the card than listening.

Payload length in the `DISCOVER` reply — dropped: the width is the segment's (32 B spur, 8 B mini);
`slots` stayed a register, as Argus's count depends on what hangs on it. Gating the spur clock at
start — dropped: it served start-up ranging on the idle clock channel, and ranging moved to the
data channel; a unit ignores the clock until `BUSCFG`.

## Ranging on a ModBus arm — removed

`blocks/modbus.md` gave the arm a house ranging extension, a leftover of the retired fast class
that leaked into the `ID` table, the handset's corrections table and Kronos's README. **ModBus
serves sensors that need no synchronisation**: a value is stamped at the poll, and microseconds of
propagation are nothing to a reading that changes by the minute.

## "No re-ranging in operation" — superseded

`blocks/gps-pps.md` listed re-ranging among what a ±1 µs station does not engineer; the card now
re-ranges every port periodically (`../bifrost/FIRMWARE.md` §4). The point stands: no servo,
temperature table or PTP on top, and the one live measurement is the card's own, in its own gap.

## The delay table's six typed terms — superseded by three

Six typed terms — antenna group delay, coax, LNA, receiver delay, pigtail, in-box buffer hop —
plus the ranged route. **Five of the six sum to under 0,1 µs against ±1 µs**; only the antenna
cable matters. Now CGGTTS's `CAB`, `INT` (antenna and receiver as one), `ROUTE` (ranged, remote
receiver). `PIP_OFFSET` and the leap offset were never delays; they stay states of a source.

## The route subtracted at the card's stamp — superseded by the unit placing its grid

The card subtracted each unit's ranged route at the stamp, an Argus its segments' when tiling, its
Bifrost the Argus's, and the head kept routes in its corrections table — from when a route was a
typed length. With the card measuring it, **the unit is handed the route once** and takes its
`SYNC` edge `ROUTE` early, so frames arrive in nominal slots. Cost: one `SET` after ranging or a
change over 2 ticks, one cell. Gone: the per-frame subtraction on every card and twice on an
Argus's path, the corrections table's spur rows, and the route's ceiling against the frame period;
only the ranging round trip must fit its gap.

## The project tick 2⁻²² s — withdrawn, the tick is 2⁻²⁰

Two bits moved from the impulsive record's amplitude to its offset — 18 bits at 2⁻²² s, a 0,24 µs
tick — as ±0,48 µs quantisation was half the budget against ~0,15 µs below Kronos. Withdrawn to 16
bits of offset, 14 of amplitude, 2 of type (2¹⁶ steps over 8 × 7,8125 ms is 2⁻²⁰). **The finer
tick bought precision the measurement lacks**: a return stroke's onset is smeared by about a
microsecond; root-summed, not added, the rows give 0,63 µs; and an 18-bit offset spanning into
word B costs a 32-bit load and three shifts. The exporters' constant is 10⁶/2²⁰.

## Underclocking a node to save power — dropped

Dynamic power scales with frequency, static draw and analogue fronts do not, so the saving is
small; and **the time arithmetic is binary from the timebase up** — a non-binary rate leaves a
remainder in every divider and moves the ranging tick off 7,45 ns. H523s run at 2²⁷, H7A3s at 2²⁸;
power is saved at the parts (`POWER.md` §1).

## A third `RUNG` value for the link with no clock wire — dropped

`BUSCFG`'s `RUNG` is the baud rung — 2²⁰ alone, 2²¹ for two to eight — so **"no clock" answers a
question the register was never asked**; the MasterNOD link runs 2²¹. A pin answers it: a card
reads `ATTN`, a unit its `ID` resistor (on mini, the sync rung).

## A house unit negotiating its ModBus rate up to 115 200 — withdrawn; 38 400 on an arm — dropped

`blocks/modbus.md` had a house MOD negotiate from 9 600 or 19 200 up to 115 200 — NodBus doctrine
on the wrong bus. **An arm is one UART at one rate shared with bought sensors**: a MOD that
switches strands its neighbours, and no-termination holds only to 19 200 at 500 m. A house MOD runs
at its arm's rate, set once in the learning mode. 38 400 went too: on sixteen units of long cable,
reflections, capacitance and inductance cap the count, and 38 400 took that margin. The arm runs
9 600 or 19 200.

## The streamed supply byte — withdrawn

Tier A sent the input voltage every frame behind a Galvani barrier — Quake's byte 31, Gauss's and
Pascal's mini byte 7 with a `MUX` register alternating voltage and current — so a fast supply
transient would not wait for a 1/min poll. **The byte repeated itself 99,99 % of the time**; the
`status` byte gives the change free: a unit whose `INA238` leaves `0xFF0A VIN_WINDOW` sets SUPPLY
in its own header — no byte, no gap, no collision, which an unsolicited `ERROR` on a chained
segment could cause. One rule for every unit closed PROTOCOL's Palatine-and-Argus exception. The
flag-plus-`GET` form fell in turn to `REPORT` (next entry).

## Temperature as a `GET`-only query, and the supply as an edge plus `GET` — superseded by the report frames

Temperature was pulled by `GET 0x0030 + class` about once a minute, the supply was SUPPLY plus
`GET VIN`, and Pascal alone streamed its temperature every frame. **The 48 V leak watch is the
difference between the station end's and the unit end's `INA238` currents**, wanted at full
resolution on a cadence without polling eight units a minute. A unit's mostly empty fourth
frame-time carries a frame of its own `kind` for no bytes, through the Argus's tiling: `REPORT`
(`kind` 6) unasked once a minute, `PORTS` (7) from a host, `HEALTH` (8) on `GET`. Pascal's
per-frame temperature went: the compensation needs the conversion, the archive not 128 times a
second (`PROTOCOL.md` §5).

Weighed and dropped: a `TEMP` opcode — no room for the two `INA238` registers;
temperature in mini reserve bytes under a header flag — the flag dies in tiling, the per-TYPE
schema breaks; one frame for units and host — a full quartet fills 32 B; a daily `LEAK` kind —
folded into `PORTS`; 8 B a socket — five sockets need two frames; sensors' die temperatures —
self-compensated, so `HEALTH`; the name `STATUS` — the header byte is `status`; per-quantity
`GET`s — one answer over several frames.

## `DIAG`, and `ERROR` sent unasked — superseded by the `FAULT` flag and `HEALTH` on `GET`

`DIAG` (op 13) went up unasked per sensor at boot and on change (OK · SELFTEST_FAIL · NO_RESPONSE ·
DEGRADED, plus the present-mask), and `ERROR` (op 12) unasked on a latched `ALERT` or a killed
port. **All of it was already said or belongs to a question**: a dead sensor is zeros, trouble is
`status` code 2, the cause the head asks once. Now `HEALTH` on `GET HEALTH`, flagged by the
header's `FAULT` bit; `ERROR` is only a negative answer (a card's unasked one duplicated its
segment table). Opcode 13 is not reused.

## CONTROL at 8 B, and the sonde addressed by its byte alone — superseded by the 12 B CONTROL with `SUB`

CONTROL was `unix.0 · frame · TYPE|NUM · op · arg0 · arg1 · CRC16`, a sonde behind an Argus
reached by address hop by hop. **The mini and NodBus type spaces share the byte**, mini types
landing on host numbers — Gauss 0x1n on the Mayak's, Quark 0x2n on the Bifrosts', Pascal 0x3n on
the Arguses' — so a `GET` to Pascal 1 reads as Argus 1 on a trunk, and a Bifrost knows only its
children. `SUB` names the last hop, and only an Argus acts on it. Merging the spaces was refused: three NodBus codes and three mini types are left. 12 B, not 16,
because **16 B is the mini DATA frame and length alone tells frames apart**; `arg2` gave `SET` and
a byte-wide `GET` 16 bits (Pascal's `ALARM`, default 1000).

Dropped: a `TUNNEL` frame to the Argus — 40 B for a 12 B question; a second address byte in DATA —
position carries identity and the 40 B frame had no byte spare; the whole path in the frame — the
wire is the path. The sondes' headers ride the Argus's as the OR of flags and the worst state code
(`../bifrost/argus/WHY.md`).

## `kind` 5 as a `GET` answer only — superseded by `kind` 5 in both directions

`kind` 5 REGISTER only answered a `GET` and `SET` carried a byte, so **nothing could write a wide
register** — Palatine's arm-table row (6 B) and roster entry (10 B), Gauss's calibration matrix
(30 B), Quake's levelling matrix. Now `SET` has 16 bits and `kind` 5 also goes downward in the
unit's third frame-time, answered `ACK`, relayed like `TUNNEL` and by `SUB` — no new kind or opcode
(`PROTOCOL.md` §1, §9).

## The alarm byte in the payload — withdrawn

Quake's byte 30 and Gauss's and Pascal's mini byte 6 carried the trigger — `nq-detect`, a field
step, a pressure step — with Quake's QC errorflag. **The `status` byte already had the bits**,
ALARM and SUPPLY among its four flags, at the slot's edge, free to the payload and dispatched
unread; QC rides `status` 2. A state code 8 SUPPLY lived an hour: a flag coexists with CLIPPED or
SENSOR, a code cannot.

## The 15-minute LoRa summary, and the "SOH stream" — superseded by an hourly heartbeat and a state

The head sent its 16 B values frame every 15 minutes, and the minute SOH was called a "stream"
while §5 said there was none. **Four OKs an hour bought nothing a server could act on**; an hourly
frame tells alive from dead. The SOH is a RAM state. 16 B stayed — one LoRa packet under its preamble.

## The DS18B20 — withdrawn; an NTC on the ADC takes every lead and every coil

The 1-Wire part sat between Gauss's and Quake's magnetometer coils and at the Ceres, Sakura and
Babel electrodes. Its conversion draws ~1,5 mA for 750 ms — **a dipole of tens of nT at 5 mm,
units at 10 mm**, every 10 s, in a sensor built for tenths of a nT, fitting no gap with the RM3100
converting 25 of every 31 ms. An NTC on a switched divider draws ~90 µA for microseconds, its
ratio cancels the reference, and 0,3–0,5 °C uncalibrated (0,1 °C after one point) serves every
correction. TMP117 or STS35 stay where a board or wall wants a product-grade number.

## "Our own units never multiplex" — softened to one quantity and a pair

The rule bought the table-free station — one sensor, one address — and gains one clause: **a unit
may carry the pair a bought unit of its class carries**, on `0x0001`. A T/RH pod is one slave with
two values; a two-slave Ceres was unlike its bought equivalent, and Babel's `VALUE2` had bent the
rule already (`blocks/modbus.md`).

## An external clock multiplier on the sondes — dropped

Pascal and Gauss carry no crystal (no gas cavity in a pressure body); if HSI trim proved too coarse
for the baud, a multiplier was to take the 2¹⁹ rung ×8 to 2²². **Such a PLL buffer draws milliamps
continuously** — more than the rest of a sonde at its duty, at the end of a kilometres-long feed
paid in copper. The rung-disciplined HSI stands (`blocks/clocks.md`); a shortfall calls for a
better use of the rung, not a part.

## The rung into `OSC_IN` instead of the sondes' discipline loop — refused by the medium

A 2²² rung would clear the H523's external-clock input (from 4 MHz, DS14540 Rev 3, Table 38) and
feed `OSC_IN` as HSE bypass, `M` 1 · `N` 64 to 2²⁸, without `FRACN` or `HSITRIM`. **The glass
refuses it**: off copper a mini segment runs `G-O-2-100`, a 2 Mb/s module; 2¹⁹ is 1,05 Mb/s of
transitions, 2²² is 8,4 Mb/s. Hence `G-O-2-100`'s 2¹⁹ on the `ID` scale and on all four Argus
segments; Gauss's deep form is 8 km of cable, so neither sonde could raise it.

## `SLEEP` — the two-mode sleep opcode, and sleep as an operating mode

Opcode 6, broadcast: `arg0` 0 clock-held (HSE-locked, woken over its UART), `arg0` 1
cold-stop (clock dropped, woken by its return). A kill was
`SLEEP` then the feed; the battery deficit ran on `SLEEP` 1; every `FIRMWARE.md` had a `HELD` and a
`cold` branch. **The station does not save power in normal running**: an idle unit is ended and
`ENABLE` cuts its branch, so the far end draws nothing, where a sleeper still ran its rails, island
converter and interface. Opcode 6 is `END`, a skippable courtesy before `PORT_CLK` 0 and
`PORT_PWR` 0; station-wide is the lockdown (`POWER.md`). Kept: the wake is the existing link, no
wire or pin.

## The filter parts — the 33 µH standard, the 600 Ω bead and the RC — superseded

`blocks/protection-power.md` set a shielded 33 µH 5×5 mm inductor behind 10× 100 nF + 10 µF,
written for 100 kHz-class module converters; no board carried it. Boards then used the 0603 bead
`GZ1608D601TF` against clock edges, 2,2 µH against bucks, and 33 Ω + 10 µF + 100 nF on
magnetometer rails. **The bead is 600 Ω at 100 MHz but a few ohms at the 1–2,7 MHz every rail
carries first**, and the 1×9 module sheets ask ~1 µH, so every position became 2,2 µH into 10 µF +
100 nF. That lasted a day on fast converter interfaces, whose 17–44 MHz fundamentals and harmonics
sit **above the inductor's 69 MHz self-resonance, where it is a capacitor**: they add a bead after the
10 µF, the 0805 `GZ2012D301TF` (300 Ω, 0,20 Ω, 500 mA; the 0603 has
0,45 Ω, 200 mA).

## Three equally good crystal values — superseded

`blocks/clocks.md` let a board fit 2²², 2²³ or 2²⁴ by stock, `M` and `N` following. **The four
MODs carry one part**, `SWXBEABVF0-16.777216` (2²⁴, M 2 · N 32, 12 pF load pair); another value is
a substitution at the order, not a choice. `Quark-Tubes` went to the rung-disciplined HSI
(`../quark/WHY.md`).

## The node's connector as it stood on the one-body contract — superseded

`HARDWARE.md` had logic "and the 12 V from the card's port on the same connector" over a crossed
in-box cable, "its own LDOs, and no buck", a unit moved "from inside the box to the far end of a
spur", `DE` as power gate, a terminator plug in the last board's free `OUT` socket, a ladder by
board. **A measuring unit is never in the enclosure**, so a node always meets a power and a
communication board: 12 V on its own terminals into `LMR43610` bucks, `ENABLE` the gate, `DE` the
driver enable, 80,6 Ω on a jumper, the ladder by the cable's end (`../galvani/WHY.md`).

## The 14-day pack — superseded

Sized for ≥ 14 days of full load: 14 kWh usable at 70 % DoD, 4S5P of Winston's 300 Ah LiFeYPO₄,
**~194 kg — a vehicle battery by weight and price**; earlier, "a ~6000 Kč solar and battery for
weeks". Now 2P4S of 314 Ah LiFePO₄, 8,0 kWh used 20–80 %, about five days, the lockdown carrying
the rest; LiFeYPO₄ stays for a vault that may see frost (`POWER.md` §3).

## The assembly ring and the reassembly horizon — superseded

A ring per unit on the card, 16 frames deep, up to a second per link; a slot left when finished or
at age 16, and the head closed a second at T+1 s. **On a two-hop path — sonde, Argus, Bifrost —
two rings could each take a second**, so the horizon broke and nothing released at a known
instant. Replaced by two-gate FIFOs read at frame index plus the parent-set `0xFF13 DELAY`: every
record reaches the head exactly one second after it was measured.

## 2²⁴ on Kronos's time bus — superseded by 2²²

The in-box clock went from 2²² to the TCXO's 2²⁴, a Bifrost's PLL ×8 to 2²⁷ and its timer cutting
2²² back out. **The ×8 bought a quieter PLL, which the ±1 µs contract cannot see** — every unit
multiplies from 2²² anyway. Back at 2²², both roles lock one PLL (`M` 1 · `N` 64) instead of two trees,
a Bifrost passing the edge to its spurs undivided on `MCO1`, and the ribbon runs at a quarter of
the frequency.

## The pre-Galvani surge design — `blocks/protection-485.md` and `blocks/protection-power.md`

SHORT and LONG tiers by run length, a gas tube per site, `5KP16A` and `1.5KE6.8A` transils, board
series 1 and 2, the N board, an optional fuse box. **Galvani replaced every row**: the cable's end
decides what is fitted, `5.0SMDJ` transils among it (`../galvani/WHY.md`). Live parts moved first:
the `L·di/dt` case and the fuse ladder to `../daedalus/CONSTRUCTION.md`, the 80 + 10 termination
split and the `SM712`/10 Ω coordination beside their boards.

## The fast ModBus class and the per-spur link table — removed from `blocks/modbus.md`

The block kept the retired fast leaf class — full-rate Gauss at ~1 200 B/s a sonde, four to a
host, **beyond any 9 600/19 200 bus** — and a per-spur "spur link class" baud with "more small
bridge cards". Gauss is a mini-NOD behind Argus, the spur rungs are two (`blocks/nodbus.md`), the
cards `../bifrost/`. The surge legs' 33 Ω and 22 Ω went with it; the freeze latch's history is
Argus's (`../bifrost/argus/WHY.md`).

## A precision oscillator in every node — lost to the wire clock

It loses twice. On parts: eight to sixteen expensive oscillators against one disciplined source,
buying frequency only — phase still needs hardware-timestamped PTP — while the cable had a spare
pair for the clock. **On arithmetic**: corrected every TDMA frame, a 50 ppm crystal drifts
7,8 ms × 50 ppm ≈ 390 ns, and a software UART timestamp carries ~µs of granularity and jitter, so
averaging fixes the rate, never the phase. Hardware PTP and White Rabbit are a disciplined carrier
plus phase measurement in silicon at every hop — the wire clock by construction, its one term the
cable's delay, ranged.

## One firmware for the node's lifetime — superseded by the over-bus stream

A potted unit was never to be updated, `BOOT0` the only path. The over-bus load, gone with the
burst mode, is back as `LOAD` · the stream · `APPLY` (`PROTOCOL.md` §7): **potted units with no
update path are a maintenance trap**, and the master's line on a full-duplex run carries the stream
for nothing. Kept: firmware right before potting, `BOOT0` on every H523.

## Bursting, the rung ladder and the 4–5× slot window — removed from `blocks/nodbus.md`

Bursting — cancelled. It manufactured a gap on a slow half-duplex link; **the schedule already has
the gap** (three of a unit's four frame-times), the turnaround is hardware `DE` in the guard bytes,
and full duplex saves nothing. No burst, no pipelining, no `N × 40` rule; `RESEND` is one frame in
its own slot.

The rung ladder, 2¹⁷ to above 2²¹. **On a terminated line the reach is the clock channel's, not
the baud's**, so a slow rung only held the driver longer — 20 % of a lone unit's period at 2¹⁸,
5 % at 2²⁰; 2²¹ carries the full eight, so nothing above it.

The 4–5× slot window held the dialogue and the turnaround padding, for a multidrop trunk toward
500 m with loose oscillators. The dialogue moved between slots, the turnaround into guard bytes,
the trunk into Bifrost spurs, and **the wire clock leaves nothing to drift**: a slot is a frame
plus guard bytes.

The fed copper spur's half duplex, NodBus's last, flipped when long copper went to glass and the
feed to its own 2-core cable (`../galvani/WHY.md`).

## The shared RX, the carrier board, the pigtail and the remoted receiver — removed from `blocks/gps-pps.md`

The receiver's TX fanned to head and clock processor **coupled two firmwares**: a field the head
wanted changed the clock processor. Each got its own link; later the head lost its GNSS link — its
time is PPS-K and the label.

The five-wire pigtail (`VCC · GND · TX · RX · PPS`), a Sputnik's COM port and Pip "presenting as a
GPS on the same five wires": the receiver lands on Kronos's `TIME IN` sockets, Galvani data bodies
(`../mayak/WHY.md`). One carrier board for the clock processor and the ESP: Kronos and the Mayak
are two boards on the time bus.

"Where the sky view is somewhere else, the whole receiver goes there": **a site whose mast cannot
see the sky is sited wrong**; the reversed channel and Kronos's ranging stay fitted, used only by
Pip.

"Crystal: a power-of-two value in the 4–50 MHz HSE range", a ±20–50 ppm part with GPS: Kronos
carries a 2²⁴ TCXO on every station. Holdover "freezes `FRACN` at its last good value": it follows
the temperature table from the last good code (`../kronos/FIRMWARE.md`).

## The Cat 6 feed arithmetic, the loose distribution and the floating node — removed from `POWER.md`

"Why 48: `P_max = V²/(4·R_line)`, Cat 6 loop ≈ 135 Ω/km, ~1 km at ~4,3 W" — a feed on the data
pairs; it has its own 2-core cable, reach in `../galvani/README.md`. "BMS output → terminals on a
small DIN rail → each board by wire": the 12 V goes through the fuse field, one fuse a board. "The
remote node floats mechanically, no per-node isolated DC-DC": each unit end has an isolated
island. "A local unit on the same wire": no measuring unit is in the enclosure. "Pluvius's pump
runs from a buck on the feed side": it runs on `G-24-S` on Palatine's `PWR EXT`. The tsunami array
"gated by clock-laser sleep and duty-cycling", and "a cold-stop or battery-survival deployment"
saving an order of magnitude: the deep tier is shelved, nothing duty-cycles, the lockdown is
survival.

## The end-to-end CRC, source routing and the per-rate streams — removed from `PROTOCOL.md`

Never recomputing the CRC, so the node's survives end to end, defends only against an SRAM upset
in the card's ~6 kB buffer: **at ~1000 FIT/Mbit, one bit roughly every two thousand years per station**. `PATH|SLOT` source routing, as IP's LSRR,
is deprecated and disabled everywhere; it belongs where topology is dynamic and hops hold no state.
The type names `seismo`, `basic`, `iono`, `mag`: the type is the board's name. "Master-side storage
is per-rate streams": the head decodes no DATA payload and archives by TYPE
(`../mayak/FIRMWARE.md` §6); big-endian ModBus registers are turned where the block is decoded.

## NIC-MLA and NIC-DMD as the station's archive — superseded by HMC and HCC

NIC-Arduino's NIC-MLA records and NIC-DMD compression (D30) were built for an append-only
ATmega328 — a record every fifteen minutes, a 16–64 B LoRa packet. The station outgrew them four
ways:

- **MLA's one schema a file**: dozens of TYPEs cut by a table that did not describe them; HMC
  carries a layout per TYPE.
- **MLA's index per record**, where the station queries by second and address.
- **DMD's delta chain**: a keyframe every seventh packet, so a record cost up to six reads and a
  damaged one took six with it; HCC codes each second alone.
- **DMD's row deltas** break multi-byte carries and ignore the series down the second: Quake
  16,8 B a sample quiet, 22,9 B in an event, against ~7,6 / ~14,9 for Steim on columns, as HCC codes.

Kept: the pre-allocated 1 MB file with descriptions from the back, the FAT touched only at open, a
CRC per block; not the in-file header mirror — the second card is the mirror.

## "The head archives every frame and decodes no payload" — superseded by the recording rules

The head wrote every frame, 128 a second, fillers included. **The archive cost the same whatever a
unit measured** — a Palatine changing a few values an hour cost a Quake, a Tesla's quiet sky was
95 % zeros — and a satellite-only station could not say what to send. Each unit now has a recording
and a send rule (`archive/HMC.md`) comparing bytes, never values: the archive stays raw, the head
interprets no payload.

## HMC's first form — one file for the station, one block a second — superseded

One station file series of self-contained one-second blocks, every address's 128 rows in each,
64 MB files closed at midnight or on a header change. **Units differ by orders of magnitude, so one
block shape served none**: a still unit cost ~35 B a second, every second a keyframe; a settings
change closed the station's file; the uplink streamed seconds where a link wants packets. Replaced
by per-unit files with segments.

## KSF as an optional encryption layer — removed

No MAC by default, KSF (NIC-Arduino's cipher) for private deployments. Nothing is encrypted: **the
archive is open data**, and control, the one thing worth protecting, gets a signature on the
command channel, not a cipher on the data.

## One-second segments, and a self-synchronising code — weighed

A one-second segment would lose at most a second in a crash but repay the predictors' warm-up each
time — **a tenth of the segment** for an order-8 predictor at 128 frames a second. The segment is
8 s; the backup cell writes the buffers when 12 V is lost. No Huffman or Rice code lets a reader
find a codeword boundary from an arbitrary position; COBS, a zero after every segment, does for one
byte in 254.

## One 1 MB file for every unit — superseded by a size per address

The file kept MLA's pre-allocated 1 MB. **A Quake under `ALL` filled it in minutes**, a new file
and a FAT write each time, while a Palatine under `CHANGE` took months over the same megabyte. The
size is now a setting per address, 1 MB × 2^`size` up to 128 MB, recorded in the header: 64 MB for
an address under `ALL`, 1 MB for every other (`archive/HMC.md`).

## The archive's seconds read as UTC — corrected: leap-blind, with a `TIME` section

HMC called its seconds Unix seconds and its paths UTC, while Kronos labels the station's scale
leap-blind, GPS time in the Unix epoch, 18 s ahead in 2026 — and nothing in the file carried the
difference, so an exporter could not reach UTC from the file alone. **The seconds stay leap-blind;
a `TIME` section carries the GPS − UTC offset and an announced leap**, written in a config when
either changes.

## The modem out-only on every module — superseded: by what the modem can

The radio position was LoRa, and a LoRa downlink able to reconfigure a buried node was refused.
**An LTE-M modem is a link like Wi-Fi**: it carries the authenticated session — `HELLO`, the
`CURSOR`, the command channel under its MAC, the live board. LoRa and a satellite module stay
out-only, a resend request their one downlink.

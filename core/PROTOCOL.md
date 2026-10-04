★ N.I.C. ★

# NIC Core — Protocol & Data Architecture

> **Design-stage concept — nothing built.** The figures are verified against the parts' sheets and
> the parts before a board is made.

**The station's protocol and data design** — the frames, addressing, the TDMA schedule, the time
model, the data tiers, storage and uplink, and the node's contract. `DESIGN.md` is the index of
decisions; the bus hardware and its clock are `blocks/nodbus.md`'s; a front's own decisions stay in
its unit's documents.

---

## 1. The frames

**Two frames, fixed-length, told apart by their length, delimited by the idle line.** There is
no magic byte and no length field: a frame ends when the line goes idle, and what arrived is
either the DATA length — 40 B on a spur, 16 B on mini, 48 B on the trunk — or 12 B. Every frame
is fixed-length so the master pre-computes the whole schedule; **a header byte is either on
every frame or on none** — no optional or variable-length fields.

```
DATA — one frame. The card prepends; it never moves a byte.

  a node emits                                                    40 B   (mini: 16 B)
  ┌──────── header 6 B ─────────────────────┬── payload 32 B ──┬── 2 ──┐
  │unix.0│frame│TYPE|NUM│slot│kind│status   │   (mini: 8 B)    │ CRC16 │
  └──────┴─────┴────────┴────┴────┴─────────┴──────────────────┴───────┘

  the card completes the second                                                       48 B
  ┌── 3 ───────────────┬──────── the node's 6 ───────────────────┬── payload 32 B ──┬─ 5 ──┬── 2 ──┐
  │unix.3│unix.2│unix.1│unix.0│frame│TYPE|NUM│slot│kind│status   │                  │ rsvd │ CRC16 │
  └──────┴──────┴──────┴──────┴─────┴────────┴────┴────┴─────────┴──────────────────┴──────┴───────┘
   └──── time, most significant first ────┘                                          recomputed

CONTROL — either direction, on demand, in the gaps                                      12 B
┌────────┬───────┬──────────┬─────┬────┬──────┬──────┬──────┬──────┬──────┬───────┐
│ unix.0 │ frame │ TYPE|NUM │ SUB │ op │ arg0 │ arg1 │ arg2 │ rsvd │ rsvd │ CRC16 │
└────────┴───────┴──────────┴─────┴────┴──────┴──────┴──────┴──────┴──────┴───────┘
 └── the header both frames share ──┘
```

**The time comes first, most significant byte first — the one field in the station that is not
little-endian, and it is that way for the card.** The spur frame lands three bytes into the trunk
buffer; the card writes `unix.3 · unix.2 · unix.1` in front of it, zeroes the five reserve bytes
over the spur's CRC it has already checked, and recomputes the one CRC over all 46 — **the frame
is never moved in memory**, a thousand times a second on the card. Putting the second in
little-endian, after `unix.0`, would cost a copy of every frame on the busiest processor in the
station to save the head four loads. Every value inside a payload stays little-endian, native to
every MCU here — **except a ModBus block** (§5, *The sensor block*), which carries its registers
big-endian as the sensor returned them, because the host that polls composes nothing and the
same bytes travel the tunnel unchanged; they are turned where the block is decoded. The
header's time is read with one helper and that is the whole cost. The spur
already reads the same way — `unix.0`, then `frame`: the second, then the sample in it.

**Both frames open the same way: time, then address.** A CONTROL frame carries the time of its
own start-bit edge, so any frame the card sends in the gap phases a returning unit whatever it
says (§3), and an `ERROR` or `EVENT` from a unit carries its instant without an age field.

- **`unix.0`** — the second, mod 256, the node's own count from `SYNC` (§4); the card supplies
  `unix.1..3`. **`frame`** — the sample within the second, 0..255, which covers 256 samples/s.
- **`TYPE|NUM`** — `TYPE«4 | NUMBER`, the identity, and it never changes meaning (§2). The card
  checks it against the slot the frame arrived in; the head keys the roster and the archive by
  it. In a frame going down it is the addressee, 0xFF the broadcast.
- **`slot`** — the position in the round the unit fires from, the one `ASSIGN_ADDR` gave it. The
  card checks that the frame's place in time matches it. On a shared segment a unit coming back
  reads a neighbour's `slot` with its `unix.0 · frame` to find the round and fires in the next
  one (§3), and the echo receiver reads it to check that the round runs in order. The card
  carries it through unchanged.
- **`kind`** — what the payload is. **0 DATA** the unit's own samples · **1 TUNNEL** a ModBus
  package going down (§9) · **2 TUNNEL_REPLY** the ModBus arm's answer going up, verbatim · **3 FILLER**
  the card's frame for an absent unit, the reason code in the payload (§10) · **4 TABLE** the
  card's segment table, 8 × 4 B (§10) · **5 REGISTER** a register too wide for CONTROL, **either
  direction** — up as a `GET`'s answer, down as a wide `SET`'s value — the register number first,
  the value after it, zeros to 32 B · **6 REPORT** a unit about itself — its arrived feed and its
  temperature, 8 B a unit, once a `REPORT_INTERVAL` (§5) · **7 PORTS** a host about its power
  sockets — `VBUS` and `CURRENT` per socket, the leak watch and the host's temperature, in the
  host's TYPE map, from the host's own address, once a minute (§5) · **8 HEALTH** a board's
  internals in its TYPE map, **on `GET HEALTH` only**, never unasked (§5) · **9 STREAM** a `LOAD`'s
  frames going down, an image or a table (§7). A unit's DATA frame
  writes 0. The head dispatches on this byte and nothing else: 0 to the archive by TYPE, 2 to
  the open question, 3 marks the absence, 4 rebuilds the roster, 6–8 to the report tables.
  **A frame of kind 5–8 never stands where the DATA frame stands**: it rides a later frame-time
  of the sender's own block, behind the data (`blocks/nodbus.md`), and a reply owed goes first.
- **`status`** — the unit's state at the instant of the frame: **the low nibble one state code,
  the high nibble four flags**, independent of it and of each other. The code: **0 OK** ·
  **1 REJOINED** the first frame after a rejoin · **2 SENSOR** a sensor is faulty — the zeros in the
  payload say which, `HEALTH` says how · **3 RC** the frame was captured on RC and is delivered from the buffer ·
  **4 WARMUP** a sensor was soft-reset and is settling · **5 CHANGED** a setting took effect with
  this frame, the head writes the unit's new configuration into its archive from this second (§5) · **6 CLIPPED** a value
  hit its range · **7 DISTURBED** the value did not clip, but the input was not the signal — the
  unit's own running background says so and the unit never names the source (`README.md`, *The
  second load-bearing principle*); 8..15 free. The flags: **bit 4
  ALARM** — the measurement's own trigger, a departure from the unit's running normal by more
  than its threshold (a shake, a pressure step, a field step; the threshold is the unit's
  register), set while the condition lasts — the unit holds no window and sends no event, the
  head decides what it is worth (§9, `EVENT`) · **bit 5 SUPPLY** — the arrived supply is outside
  the unit's window, under-voltage or over-current where the board measures it, held while it
  lasts; the value rides the unit's `REPORT` frame once a minute and one `GET REPORT` away at
  once, and the head logs the edge · **bit 6 FAULT** — the board
  holds something in its `HEALTH` other than OK: a sensor's verdict, a latched `ALERT`, a rail's
  `PGOOD` low, a lost discipline; held while it lasts, and the head pulls `GET HEALTH` once per
  edge, as it pulls `GET REPORT` on SUPPLY. **Nothing about a fault is sent unasked beyond this
  bit** · bit 7 reserve. **No unit carries an alarm or a supply byte in its DATA payload** — the header says it,
  every frame, at the slot's own edge, and the value rides the `REPORT` frame (§5). The head writes the byte into the record and dispatches on
  the flags; the card passes it through.
- **`SUB`** — the one hop the wire does not name. A CONTROL frame travels on a trunk that IS a
  card and a port that IS a segment, so the path needs no byte — except the last hop, where four
  sondes share one slot under one address. **`SUB` 0: the frame is for `TYPE|NUM` itself** — a
  unit on a spur, a card, an Argus NUMBER (a `GET REPORT` there brings the quartet). **`SUB` ≠ 0:
  `TYPE|NUM` is the host of a segment and `SUB` the sonde behind it**, in the mini space, where
  it is unique. A Bifrost relays by `TYPE|NUM` and never reads `SUB`; **an Argus swaps** — down,
  `TYPE|NUM` ← `SUB`, `SUB` ← 0, the CRC recomputed, the frame into the sonde's third
  frame-time; up, the sonde's `ACK`, `ERROR` or byte answer gets `TYPE|NUM` ← the Argus's
  NUMBER, `SUB` ← the sonde. **A unit therefore always sees its own address and `SUB` 0**, and
  the firmware of a sonde and of a spur unit is the same firmware. The two address spaces
  collide in the byte — Pascal 1 and Argus 1 are both 0x31 — and `SUB` is what keeps them
  apart on the trunk without merging them.
- **`op · arg0 · arg1 · arg2`** — a verb and three whole-byte arguments; the table is §9. `arg2`
  is the high byte of a 16-bit value on `SET` and on a byte-wide `GET` answer, 0 elsewhere.
  Nothing is packed into a half-byte anywhere. **The two reserve bytes are 0 and undefined.**
- **No transaction id.** One question, one answer: the master has at most one request open per
  unit, and the next frame of the matching kind from that unit is the reply — `GET` answers
  under `GET`, a tunnel package with `TUNNEL_REPLY`. The timeout is the master's.
- **One CRC, and whoever last touched the frame owns it.** The card completes the frame for the
  trunk and recomputes. **Appending corrections instead would be worse, not safer:** a frame
  carrying an original plus two corrections carries three answers, and the reader has to decide
  which is true. One value that is overwritten is coherent.
- **CRC-16-CCITT on both** — one polynomial, so the H523's hardware CRC unit is configured once.
- **Payload = 32 B, a power of two:** it tiles a 512 B SD sector exactly (16/sector, no
  straddle) and keeps DMA/ring arithmetic clean. The `DISCOVER` reply reports the length the
  master decodes by, TYPE says what it means.
- **Frozen: 40 B emitted, 16 B on mini, 48 B delivered, 32 B of payload.** The five reserve bytes
  exist so the next change has somewhere to go instead of forcing a rewrite.

## 2. Identity & addressing — the address IS the identity

**One byte says what a unit is and which one it is: `TYPE«4 | NUMBER`.** Quake #2 is `0x52`,
Gauss #2 is `0x12`, Pluvius #2 on ModBus is `0x16`. **Three units numbered 2 collide with nothing
and need nothing translated.** The TYPE is the board's from the factory; the NUMBER is given to the
unit by the host of its bus at the sweep (*How a unit gets its number*, below) and the unit keeps
it. Nothing is printed on a box, and nothing anywhere maps one number onto another.

**The 14-type ceiling is the price of that and it is worth paying.** What it is *not* is a shared
budget.

### Three buses, three type spaces

**A module's code never appears on another bus's frame.** A host reads its own bus and packs the
values into its own payload as self-describing blocks, and **a record is read in the context of the host
that carried it** — the frame's own address says which. So the spaces are independent, and each
is free to spend the byte the way its own bus needs. **The same holds for a record type inside a
payload**: Tesla's event types 0..3 and Marconi's carrier type 0 are two tables, one per unit
type, and a number means nothing without the address that carried it.

**The buses do not split the byte the same way, because they do not carry the same thing.** A
Bifrost serves eight units and its type list is short and closed; a ModBus arm serves about
sixteen and its type list grows with every quantity that cannot be bought as a finished RS-485
box. **So NodBus and mini pack `TYPE«4 | NUMBER` and ModBus packs `TYPE«2 | NUMBER`** — ModBus
buys types with the two bits it does not need for instances.

| **NodBus** — 40 B, `TYPE«4 \| NUMBER` | | **mini** — 16 B, `TYPE«4 \| NUMBER` | | **ModBus** — Palatine, `TYPE«2 \| NUMBER` | |
|---|---|---|---|---|---|
| 1 | Mayak | **1** | **Gauss** | 4 | Babel, bare |
| 2 | Bifrost | 2 | Quark-Tubes | 5 | Pluvius |
| 3 | Argus | 3 | Pascal | 6 | Ceres |
| 4 | **Marconi** | | | 7 | Sakura |
| 5 | Quake | | | 8..61 | **free — one per quantity at a position, claimed when its profile is written** |
| 6 | Palatine | | | | |
| 7 | Tesla | | | | |
| 8 | Sputnik | | | | |
| 9 | **Photon** | | | | |
| 10 | *reserved — `Neutron`* | | | | |
| 11 | **Positron** — carries `Neutron`'s two counts | | | | |
| 12 | **Pip** | | | | |
| 13 | **Steinmetz** | | | | |

**NodBus and mini: TYPE 1..14, NUMBER 1..15, landing `0x11..0xEF`.** `0` would put the pack
inside `0x01..0x0F`, the hand-written range for bought units, and `15«4|15` is `0xFF`.

**ModBus: TYPE 4..61, NUMBER 0..3, landing `16..247`.** TYPE 3 would land at 12..15, inside the
same hand-written range; TYPE 62 starts at 248 and Modbus's legal range ends at **247**, which
`61«2|3` hits exactly. A bare Babel #0 is 16 decimal, Sakura #0 is 28, the ceiling is 247.

**Six bits for types and two for instances, because that is the shape of a plot.** Nobody stands
three CO₂ sensors on one station; everybody stands twenty gas sensors of different kinds beside
four thermometers and three hygrometers. And **a type is a quantity at a position**: air
temperature at 2 m, soil temperature at −20 cm and soil temperature at −50 cm are three types, not
three instances of one. Four of a type is the ceiling; a fifth thermometer is a new type.

**The three type tables are the station's whole schema.** One small table per bus — code, name,
quantity, position, unit, the block layout — is all a database or the server needs to read a
record: cut the address byte, look the type up, and the content is known. The tables are a few
kilobytes of text, so a station can ship them as a file over any link, and a new sensor is a new
row, never a new format. **Codes are claimed in order as profiles are written**; the first claims
on ModBus, so the shape of the table is not left to the imagination:

| code | quantity, position | unit |
|---|---|---|
| 4 | **Babel, bare** — a board with no position fitted answers here once, with its house block and zeros, so the board is on the bus before it is given a sensor; a fitted position answers on its quantity's type and the bare slave is not presented (`../babel/MODBUS.md`) | |
| 5 | Pluvius — precipitation, weighed | g, mm |
| 6 | Ceres — soil moisture, and the soil temperature at its depth on `0x0001` | %, °C |
| 7 | Sakura — leaf wetness, and the temperature at the plate on `0x0001` | %, °C |
| 8 | air temperature, 2 m | °C |
| 9 | relative humidity, 2 m | % |
| 10 | soil temperature, −20 cm | °C |
| 11 | soil temperature, −50 cm | °C |
| 12 | soil temperature, −100 cm | °C |
| 13 | soil moisture, −50 cm | % |
| 14 | barometric pressure | hPa |
| 15 | wind speed, 2 m | m/s |
| 16 | wind direction, 2 m | ° |
| 17 | global radiation | W/m² |
| 18 | CO₂ | ppm |
| 19 | NO₂ | ppb |
| 20 | O₃ | ppb |
| 21 | PM2.5 | µg/m³ |
| 22 | PM10 | µg/m³ |
| 23 | UV — the index, and UVA / UVB irradiance where the unit gives them | index, W/m² |
| 24 | ground temperature, 5 cm | °C |
| 25..61 | free | |

**Gauss is mini type 1 and nothing else.** A sonde is 6 B of magnetometer, which is a mini
payload, and the segment carries the clock: its NodBus row went with the mini retarget and its
ModBus row after it (`../gauss/WHY.md`) — one bus, one firmware, no listening for which.

**Bought sensors keep `0x01..0x0F`** — fifteen hand-written addresses against the eight a full
weather station fits. Ours never land below 16, so the two ranges cannot meet. A build that ever
needs more than fifteen takes them off the top of the type space, which is unallocated; that is a
build's note, not a change to this rule.

**A NUMBER is unique across the station, not across an arm.** Palatine's arms are an electrical
division — reach, capacity and rate (`blocks/modbus.md`) — never an address space, so two
boxes of one type on different arms are still two NUMBERs. An identity that repeated per arm would
need the arm to disambiguate it, and that is the translation table this design exists to avoid.

### NodBus mini — the same bus, one payload wide

**8 B of payload instead of 32, everything else identical**: the same framing, the same TDMA, the
same clock, the same `TYPE«4 | NUMBER`. It hangs behind **Argus**, which aggregates its segments
into its own slots upstream.

**Why it exists:** a sonde is 6 B of measurement. Giving it a 32 B slot and a Bifrost port of its
own wasted both; putting it on ModBus cost the clock. Mini is **synchronous, so the slot is the
time** — no marker, no age field, nothing to reconcile. A record assembles because it arrived when
it was due.

**What it costs, and it is structural: a mini segment takes a whole UART.** TDMA needs a
continuous ear, so segments cannot share a port. **A polled bus can** — which is exactly why
ModBus fans several arms off one UART where mini gives **four segments on an Argus**. That is the
trade and it is paid for the timing, and it is the reason ModBus stays.

**Argus announces as several NODs.** One 32 B slot does not carry a full Argus, so it takes **N
NUMBERs of its own TYPE and N slots**, N sized to the loadout. That is ordinary addressing, not a
special case — a card counts slots, and a unit needing more payload takes more of them. **N =
⌈minis/4⌉ and the loadout stops at eight**, so an Argus is at most two units: four full-rate
sondes are one slot, eight are two (`../bifrost/argus/README.md`).

**On NodBus and mini the type is the board's name** — one vocabulary, nothing to learn twice.

**Babel has no type of its own, and that is the point.** A Babel presents outward as **one ModBus
slave per fitted sensor position** — and a board with nothing fitted answers once on **type 4, bare**, so it is on the bus with its identity and zeros — registering after start by what is populated, and each takes
**the type of the quantity it measures** — a thermometer is a thermometer whether it sits on a
Babel or arrived as a bought box. What tells the master that several of those slaves are one
board is `0xFF01 IDENT` in the house block, which every house MOD already carries: the addresses
say what each sensor is, IDENT says what it is plugged into. **`babel/` stays a project; it
stopped being a type.**

**Our own units carry one quantity, and a paired second one on `0x0001` where a bought unit of
that class carries the pair — never more.** A T/RH pod is one slave with two values and the
host reads that shape anyway, so Ceres reports the soil temperature at its depth beside the
moisture and Sakura the temperature at its plate beside the wetness, one address each
(`blocks/modbus.md`, *The house map*). A bought part that packs eight gases into a single
transfer gets demultiplexed by the host — we can read one, we do not build one. One unit, one
address is what keeps the station's table down to *through this Bifrost, to this unit* plus a
list of sensor numbers, with nothing to unpick and no schema travelling.

### The path is not in the frame

**The frame says *who*; *where* is derivable and does not belong in 128 frames a second:**

- **which card** — the Mayak knows the trunk port the frame arrived on, and **port 2 is Bifrost 2
  by construction.** Topology is strict: a card's number is its port, never an assignment. So the
  path is the wiring, not a stored fact.
- **which slot** — a slot is a *time*. The card handed them out and knows them; nothing needs it
  written down in a frame.

**The tables that do exist are wiring records, not renamings.** None of them says "address 7 is
the unit numbered 2" — that is the translation table this design exists to avoid. Each is built by
the commissioning sweep, none is typed in, and if all of them were lost the data would still
decode; only the route for a query would have to be rediscovered.

| table | where | what is in it |
|---|---|---|
| slots | Bifrost | which unit holds which slot on which port |
| arms | Palatine | which address answered on which of its arms |
| the tree | Mayak | the whole station — who, where, under whom |

**A lookup is one instruction.** `TYPE«4 | NUMBER` is a byte, so a table is a **256-entry array
indexed by the address itself** — 512 B of RAM, no search, no compare loop. Reading a routing
byte out of a frame would be two loads; this is one. The frame is in SRAM too, so there was never
a difference in kind — and what the frame version does cost is wire time, 128 times a second,
for ever. The topology is bolted to a mast, there are two hops, and every hop already holds a
table because it has to.

- **The number is the sweep's, never the MCU's 96-bit UID and never a label on a box.** A unit
  persists it across plain resets; `HARD_RESET` puts it back to the default.
- **The roster lives on the master**, keyed by **(bus, TYPE, NUMBER)**, and holds the whole tree —
  though it needs the MOD rows only to route a tunnelled query. It is the spine of the
  configuration mirror (`../mayak/FIRMWARE.md` §7), and **the Mayak can emit the tree on demand** to
  the handset or the uplink, which is where seeing the topology belongs.
- **Enrollment guard (optional): the 2-byte house code** — the `IDENT` register, read with
  `GET` right after the `DISCOVER` reply (§9); a master with a code configured refuses a
  mismatch, one with none accepts all. A neighbour filter, not cryptography (real auth is the command MAC, §6).

### How a unit gets its number — the sweep

**A fresh unit leaves the bench with its TYPE and the default NUMBER — the top of the range, 15
on NodBus and mini, 3 on ModBus.** The host of the bus — a Bifrost for its ports, an Argus for
its segments, Palatine for its arms — runs the sweep at bring-up (`FLOOR`), in the RC dialogue at
38 400, and the sweep is a broadcast, not a walk:

- **Every unit carries a tag: CRC-16-CCITT over the whole 96-bit UID of its MCU**, computed at
  every boot into a register, readable by `GET`, the same polynomial as the frame CRC so the
  hardware CRC unit is configured once. **The tag is the whole UID hashed, never a slice of it**:
  the low bytes of an STM32 UID are the wafer coordinates and repeat across every wafer of a
  lot, and a lot is what one reel holds. Every unit computes it the same way; that is a rule of
  the contract.
- **`DISCOVER` is one broadcast, and every unit that is not enrolled in a running round answers
  once** — a fresh unit on its default byte, and a unit carrying a persisted set on the name it
  holds, so that a unit moved from another station is found under the number it brought. The
  reply is CONTROL with **the tag in the two time bytes**, which carry nothing before `SYNC`:
  `tag.0 · tag.1 · TYPE|NUM · 0 · op · 0 · 0 · 0 · 0 · 0 · CRC`. Nothing else rides it — the width is the
  segment's and the type says the rest.
- **Several units answer one broadcast the way Ethernet did it**, and the echo receiver is what
  makes it work: **listen before talking** — a unit that sees an edge on its RX while it waits
  holds off until the bus is quiet and starts its wait again; **wait `tag × 10 µs`** before the
  first attempt; **a collision is a bad CRC on its own echo**, after which the unit waits a new
  delay, CRC-16 over the UID and the attempt number, so two near tags do not meet twice. The
  collision window is the one-way propagation plus the transceivers plus the detection, about
  3 µs on 500 m of copper against a 26 µs bit; a unit alone on a point-to-point port never has
  the case. RC's ±1 % does not matter — this is not a schedule. **The base of 10 µs is what makes
  the spread the right size**: the tag is a full 16 bits, so the delay runs 0 to **655 ms** and
  averages 327 ms, which is the second the dialogue can afford; and 10 µs is still wider than the
  3 µs detection window, so even two tags one apart hear each other in time.
- **A round is closed by SILENCE, not by a count, and the silence is 700 ms.** The host holds the
  window open and ends the round when nothing has been on the wire for **700 ms** — one whole
  delay span (655 ms) plus a reply plus margin. It then **broadcasts `DISCOVER` once more and
  waits the same 700 ms**: a second silent window is what says the segment is complete, and a unit
  that was holding off through the first one is caught by the second. **Nothing is written until
  the round is over** — the count of a segment is the count of replies and the numbers are the
  Mayak's, so a reply arriving mid-round is collected and not acted on. At 38 400 a reply is
  2,08 ms, so eight units take **about 1,5 s, and a little over 5 s in the worst case** — once, at
  bring-up.
- **`ASSIGN_ADDR` is addressed by tag**: `tag.0 · tag.1 · TYPE|NUM · 0 · op · address · slot · 0 · 0 · 0 · CRC`, and
  only the unit whose tag matches takes it, rewrites itself and keeps both. **In the RC dialogue the
  tag IS the address, because `TYPE|NUM` is not one yet**: a segment of fresh units all stand on the
  same default byte — eight thermometers all arriving as the same type and the same number — and
  nothing in that byte tells them apart. The tag does, and only because it is the whole UID hashed.
  `TYPE|NUM` becomes an address the moment `ASSIGN_ADDR` has been taken and not before. The host
  asks what needs asking **after** that, addressed and unambiguous: a Sputnik takes five slots by
  its type; where the type does not say — Tesla one to three where it splits, Steinmetz three, Argus
  ⌈minis/4⌉ — the host reads the `slots` register with `GET` and hands the following numbers and
  slots to the same tag with further `ASSIGN_ADDR`s. A unit is one box on the wire whatever it
  counts as in the round, so **the physical count of a segment is the count of `DISCOVER` replies**,
  which is what the card's table holds; the rung follows the slots.
- **The Mayak owns the numbers, the card only carries them.** A NUMBER is unique across the
  station, and only the head sees the station: the card sends what it found up as its table —
  TYPE|NUM as each unit said it, and the tag — the head keeps what is valid, renumbers what
  collides or came from elsewhere with `ASSIGN_ADDR` through the card, the card confirms, and
  only then the mirror is written (`../mayak/FIRMWARE.md` §7). Two Teslas that both arrive as #5 leave
  as #1 and #2 with nobody at the site.

**Two sweeps, and the mirror is the difference.** The **initial** sweep runs on an empty mirror:
everything found is new, numbers are handed out from the bottom, and where a bought sensor needs
a profile the handset asks the human what it is. The **service** sweep runs on a filled mirror:
what answers with a name the mirror holds is unchanged, and what is **missing** and what stands
on a **default byte** is put to the human — *Quake #1 is gone and a fresh Quake stands on port 2:
replaced?* On confirmation the newcomer takes the missing number and the data series continues
under it. In service mode nothing is assigned without that confirmation.

**A human is always in the loop.** The handset reads and writes every value the station holds;
first assembly is manual where a profile has to be chosen, and the automatic sweep is the default
behaviour, not a promise that nobody has to be there. A unit is configured through the station's
own buses; a separate programming port on the Mayak is not built.

**`HARD_RESET` forgets what the sweep gave and nothing else**: the number goes back to the
default, the configuration cells to their defaults where the sensor allows it; the TYPE is the
board's and stays, and a factory calibration the unit cannot reach stays untouched.

## 3. The schedule — self-running TDMA

The clock is delivered (born on Kronos, fanned through the Bifrosts); each per-bus master
assigns slots at init (`ASSIGN_ADDR`, `arg1`), says go, then **listens and asks on demand**. Each node fires at its own
clock-anchored deadline — round start + (slot−1)×width — computed once at init, driven by the
shared clock; a running unit never waits for a predecessor. Only a unit coming back reads the
round's phase off the wire — a neighbour's frame, or the card's `TICK` (*The rejoin*, below).
Slots are disjoint → collision-free without
arbitration; a missing node leaves silence and nobody shifts. A node **silent in its own slot
is, by definition, a fault** — no polling, no discovery search; the master escalates
(retry → reset → the card's rogue-unit ladder, `../bifrost/FIRMWARE.md` §11).

**The bus has exactly two modes, and the clock is the switch between them.**

- **Self-running** — the normal state, above: after `BUSCFG` + `SYNC` every unit fires its
  slot off the shared clock and nobody polls anything; the master listens, stamps, and asks
  on demand.
- **Configuring — the dialogue through the gaps.** The deterministic idle between slots is a
  full command channel: `GET` queries, `SET` writes, tunnel relays, resends — the master
  reaches any unit mid-flight and the schedule never stops for it.
  **The schedule can still run a shared pair** — the master's gaps ordered with the units' slots
  on one line, as a half-duplex segment needs — and no build uses it: every NodBus run is full
  duplex (`../galvani/README.md`), and a shared pair halves the bus's throughput, because the
  master's channel and the units' would take turns on it. The capability stays in the schedule
  and nothing is wired for it.
  Once locked, a unit changes mode by itself — seeing the clock IS the mode switch, in both
  directions: clock present → lock and run; clock absent → RC, listen, wait. The same lever
  scales from one unit to the whole station (`END` and the kill switch, §7).

**The rejoin — a unit comes back by itself, and nothing is negotiated.** A fault in a unit
(CSS, the watchdog, a brown-out) is a reset, and a reset unit reads three things: *is the clock
there*, *do I hold a persisted set* (number, slot, BUSCFG), *have I heard the phase*.

- **No clock:** RC, 38 400, silent. It waits for a sweep.
- **Clock and a persisted set:** it locks HSE, puts its UART on its persisted rung and listens.
  **But a `DISCOVER` heard at 38 400 wins over the clock**: a segment being started is not a
  running round, and the unit answers with the name it holds and lets the sweep validate it
  (§2) — the clock on the wire means nothing until `BUSCFG` says what to do with it.
  Any frame with a good CRC proves the rung; a second of nothing and it tries the other one.
  The phase comes from the first frame it can read: **a neighbour's DATA frame** — the index
  from `unix.0 · frame`, the round's start from the neighbour's slot byte and the width — or,
  on a port where no neighbour speaks, **the card's `TICK`** (§9). It fires in the next round.
  The neighbour's edge arrives late by one propagation, ≤ 2,5 µs on 500 m of copper, about five
  bit times at 2²¹, which the guard bytes and the busy-hold absorb; no correction is exchanged.
- **Clock and no set** — fresh, or after `HARD_RESET`, or a set that fails its check (§7): it
  stays on 38 400, silent, and waits for a sweep. It is the same unit the service sweep shows
  the human on its default byte.

The card does nothing for any of this. Its part is the silent slot: filler with the reason
code every round, and the power cut after the unit has had its rounds to come back (§10).

**Three checks, one per hop, and every unit runs its own.** The head checks the CRC of what the
trunk brings; the card checks the CRC of what a spur brings and answers a miss with `RESEND`; a
unit hears its own frame on the echo receiver, checks its CRC, and on a miss holds the frame
ready for the `RESEND` it knows is coming (`blocks/nodbus.md`, *The link under the protocol*). **It does that
alone on a point-to-point link exactly as on a shared segment** — the condition would save
nothing and cost a second code path, so there is one. A unit never asks for a repeat of what it
received badly — the master times out and asks again, and a second channel for that was
weighed and declined (`WHY.md`).

**Whose rule it is.** The rejoin and the slot byte belong to the two TDMA buses — **NodBus**
behind a Bifrost and **NodBus mini** behind an Argus, the same header on both. The **trunk** is
point-to-point with one talker per direction and the card is the only one that could come back
on it: a card that reset runs its floor again on `FLOOR` (§10), nothing rejoins a trunk, and the
slot byte rides it only because the card carries the header through. **ModBus** has no frame of
this kind and nothing to rejoin: a polled slave answers when asked.
**A port with no unit keeps its clock off and is asked `DISCOVER` once a minute at 38 400**, so a
unit plugged into a free port is found without anyone; `DISCOVER` exists only in the RC dialogue
— in a running round the card knows what it has. **Adding a unit to a running segment is
service**: the handset says so, the card takes the segment's round down and runs the start
again — sweep, `BUSCFG`, `SYNC` — a few seconds on that one segment and nothing else
notices. The round is never rewritten while it runs (§10).

- **The rate is one of two rungs, 2²⁰ alone and 2²¹ chained** — the ×4 arithmetic, the slot and
  its guard bytes, and the busy-hold live in [`blocks/nodbus.md`](blocks/nodbus.md), the one place.
- **The frame rate is a 1-byte power-of-two CODE** (`rate_hz = 1 << code`), never literal Hz:
  design range 16–512 Hz (codes 4–9); a station with none of the fast fronts may run below.
  **128 Hz is forced by the fast fronts** — seismo, Tesla, Sputnik — not by seismo alone. The
  per-bus master sends the fastest present
  rate's code in `BUSCFG`.
- **One bus, not a slow/fast pair** — a second bus would double transceivers and bring-up for
  no gain; only the repetition rate is per-deployment.
- **The idle time is the design, not waste:** the deterministic gaps are the master's injection
  window (queries, tunnel relays, resends) and what makes fault detection free. A negotiated,
  packet-switched bus would buy arbitration and variable timing and lose the fixed schedule.
- **The head's own cadences never hang on PPS** — free-running timers, 1 s · 1 min · 1 h, survive
  a lost source; a tick only sets a flag, and the work runs on the comms core.

## 4. Time

The full time architecture lives with its hardware — the binary timescale and index split in
[`blocks/nodbus.md`](blocks/nodbus.md), *The network clock*, the discipline loop and
distribution in `../kronos/FIRMWARE.md` and `../kronos/HARDWARE.md`, the stamping in `../bifrost/FIRMWARE.md` and the ranging in `../bifrost/HARDWARE.md`, the
sources in [`blocks/gps-pps.md`](blocks/gps-pps.md). What belongs to the protocol layer:

- **Anchor + index, never a timestamp per sample.** Samples are exactly evenly spaced by
  construction, so a record stores the second once and every sample's time is computed —
  miniSEED practice. A spur carries only the index — the second's low byte and the frame, loaded
  from `SYNC`'s header and counted from there (`blocks/nodbus.md`, *How a bus starts and heals*); the card
  prepends the rest of the second from Kronos and checks the low byte against it; the trunk
  carries the 4 B second.
- **Leap seconds: acquire on a continuous scale, derive UTC only at the edge.** The anchor
  timescale is GPS-time-in-Unix-epoch — monotonic, leap-blind; the live GPS-UTC offset and a
  latched announcement are held beside it, carried into the archive's `TIME` section and applied
  only at export (`archive/HMC.md`). An inserted
  second yields the repeated UTC second by construction; no data path sees a discontinuity.
- **Clock SOH is a low-rate channel, never a bit in every record.** Clock quality is
  master-known and slow-changing, so one status byte is a field of the head's **SOH — a state held in RAM, refreshed once a minute,
  shown to the phone and carried in the radio heartbeat's flags, never a record** — and a change
  of it is a line in the event log: bits [1:0] UNSYNCED 0 / HOLDOVER 1 / LOCKED 2 / SEEDED 3 (`blocks/gps-pps.md`, *Without a receiver*) ·
  [3:2] leap-pending · [7:4] sat count. Cold start writes UNSYNCED until the first fix.
- **Settling is flagged, never dropped:** 30 min for any station with Quake, 5 min for a weather
  station. Settling and clock-unsynced (the SOH) are two independent "not ready" signals.
- **The pipeline runs exactly one second behind; the stamps do not.** Every record reaches the
  head 128 frames after its measurement, on every path (§7, *`DELAY`*; `blocks/nodbus.md`). A
  stamp is the measurement instant by construction — anchor + index — so transit and buffering
  never move it. **A second is complete at the head one second after it ends**: its last frame
  has then arrived, and what missed its instant arrived as a filler.

## 5. The data model

**One bus, two widths — and two rules cover every front: big and slow spreads across frames;
small and fast ships now.** Sputnik spreads its epoch over its five slots' frames; Palatine and
Marconi spread collected values as records; Quake and Tesla ship their own samples at full rate
on fixed offsets; a mini-NOD ships its 8 B in its slot. A new front classifies itself by those
two rules and nothing else is invented.

- **Three priority tiers.** **A — streamed every frame, the DATA frame:** the primary
  measurement and nothing else — no alarm byte, no supply byte, no temperature; the trigger and
  the supply edge are flags in the header (§1). **B — the report frames, unasked once a minute,
  and the health frame on request.** **`kind` 6 `REPORT` — a unit about itself**, once a
  `REPORT_INTERVAL` (default 60 s, the house register `0xFF0B`): **8 B a unit, four little-endian
  16-bit words** — `VBUS` and `CURRENT` as the `INA238`'s raw registers (the head knows the board
  from its `ID` and converts; nothing is rounded on the way; 0 in the box, where no power body is
  fitted), the board's temperature from its one main thermometer as int16 in 0,01 °C, and a
  fourth word the TYPE defines or leaves 0. **Sensor die temperatures do not ride it** — the
  sensors compensate themselves, and what a technician wants to see is one `GET HEALTH` away.
  **The 8 B tile four to a 32 B payload by position at every level**: a lone 40 B unit fills
  position 0 and the rest is zeros, an Argus fills its sondes' positions under the NUMBER that
  owns them, and eight sondes are two NUMBERs and two frames. **The frame rides the unit's own
  block, in a frame-time behind its DATA frame, never in its place** (`blocks/nodbus.md`); a
  reply the unit owes goes first and the report waits a round. **`kind` 7 `PORTS` — a host
  about its power sockets**, once a minute, from its own address, **in the host's own TYPE map**
  like every payload: **4 B a socket, `VBUS` · `CURRENT` raw, socket 0 the up port** and the
  rest in socket order — **a card**: sockets 0–4 (up, down 1–4) at bytes 0–19, the leak watch of
  down ports 1–4 at 20–27 (the second `INA238`'s raw `VBUS`, 0 where no `G-300-S` is seated),
  the card's temperature at 28–29, reserve 30–31; **Palatine**: sockets 0–5 (up, arms 1–4,
  `EXT`) at bytes 0–23, its temperature at 24–25, reserve after. A host sends no `REPORT` about
  itself — its own arrived feed is socket 0. So the head holds what left the station and what
  arrived, and **the 48 V leak watch is a subtraction at the head**; the 300 V watch rides the
  same frame every minute and the head holds the baseline (`../galvani/HARDWARE.md`, *Leak
  watch*). **`kind` 8 `HEALTH` — a board's internals, on `GET HEALTH` only**, in the TYPE's map:
  sensor die temperatures, a coil NTC, error counters, whatever the TYPE puts there; never sent
  unasked. **`GET` answers with the frame, not with a word**: `GET REPORT` brings the unit's
  report frame in its next block, `GET REPORT` to an Argus NUMBER brings the whole quartet
  (the Argus asks its sondes and tiles), `GET PORTS` the host's ports frame, `GET HEALTH` the
  health frame — the same frames, the same decoder, and the head has one open question per
  address. The per-quantity registers went with it: there is no `GET VIN` and no `GET TEMP`.
  The SUPPLY flag stays for the edge. **The report frames are the archive's slow channel and
  never a stream in the SOH.** A ModBus build carries the same as registers — a polled bus
  streams nothing. **Kronos is on no bus and sends no frame**: its faults go to the Mayak over
  the time bus's I²C and the Mayak writes them into the SOH. *(The streamed supply byte, the
  temperature as a `GET`-only query, and the three-frame draft with a `LEAK` kind are in
  `WHY.md`.)*
  **C — never transmitted:** anything used only
  for on-node correction.
- **No front multiplexes its payload.** Quake runs full rate — every axis every frame, the slow
  tilt and field riding at their latest value on fixed offsets — Palatine and Marconi use records,
  and Sputnik's epoch spreading is "big and slow spreads", parsed to the zeros with no schedule
  agreed.
- **A per-TYPE layout** — per field its offset, width, byte order, channel, scale, unit and name —
  held in the head's mirror and written into the archive's header (`archive/HMC.md`). One frame
  format; a TYPE with no layout is archived with none, its bytes kept and the TYPE unknown to the
  reader, never a crash.

### Argus tiles, it does not tag — four payloads to a payload

On its own segment a mini-NOD needs no tag: the slot is the time and its 8 B payload *is* its data.
The hop up keeps that property: **four 8 B mini payloads tile the 32 B payload exactly, each in a
fixed position assigned at enrollment.** The segment table (§10) maps NUMBER + position → mini
address — the sweep built it, the mirror holds it: the path is not in the frame, and no identity
byte rides one either; the slot byte is the Argus's own, as on every unit. An absent sonde's
position rides zeros — a hole, as on the wire — and `HEALTH` says why. **The same tiling carries a
sonde's `REPORT` frame and its `kind` 5 answer**: the 8 B ride the position the segment table gives
the sonde, under the NUMBER that owns it, in that NUMBER's own block behind its DATA frame, and the
other three positions ride zeros — a `GET` to the third sonde comes back as 16 zeros, its 8 B,
8 zeros, and the head, holding one open question on that address, reads it by position. Eight
sondes are two NUMBERs and each NUMBER's report frame is its own, so nothing need say which quartet
— the address does. A `GET REPORT` to the NUMBER brings its whole quartet: the Argus asks its four
sondes and tiles the answers. **The sondes' headers do not ride up, so the Argus carries them in its
own**: the flags of the NUMBER's frames are the OR of its four sondes' ALARM, SUPPLY and FAULT that
period, and the state code is the most severe of the four in a fixed order — 2 SENSOR · 1 REJOINED ·
3 RC · 4 WARMUP · 6 CLIPPED · 7 DISTURBED · 5 CHANGED · 0 OK — a missing sonde counting as 2. The
head reads which sonde by one `GET REPORT` or `GET HEALTH` to the NUMBER and the positions. **It is
the same system chained:** every master owns the same ≤8-row table of its own children and knows
nothing deeper; the head composes the reported tables into the station tree, so a rearranged
loadout is a table edit, never a data change. The frame's header times all four: the circular
buffer places every mini frame into the upstream frame of its own period, so nothing carries an
age. Argus takes **N NUMBERs of its own TYPE, N = ⌈minis/4⌉** — four full-rate Gauss are one slot,
eight are two — each NUMBER under the 256/s per-address index ceiling, so `RESEND` stays
unambiguous.

### The sensor block — self-delimiting, and it is the second mode

**A polled front's payload describes itself.** A fixed schema works where a front reads its own
fast sensors — Quake, Tesla. It cannot describe **Palatine**, whose arms carry whatever a site
hung on them, different at every station, each read in its own transaction. So its payload
carries **blocks** instead of offsets — **one block per ModBus transaction, in the shape the
transaction came back in**:

```
  0     module address   TYPE|NUMBER — 0x14 is Pluvius #0 on ModBus.  0 = END OF DATA
                         0xF8–0xFB, which ModBus never assigns: Palatine's WMO tables in mode B
  1     length           data bytes that follow — even, at most 30 (15 registers)
  2..   data             the registers as they came back, big-endian — the one payload
                         in the station that is not little-endian; the decoder turns it
```

**It is the ModBus response with the function code and the CRC taken off**, so the host composes
nothing: it read a contiguous run, it ships that run. The parser is
`while (addr) { addr; length; skip(length); }` — every block's start falls out of the length of the
one before it — and **a leading zero address ends the
payload** — no count field, because the payload is a fixed 32 B and a module address is never 0.

**The register numbers are NOT on the wire.** Which register a block starts at is a constant of
the type, so it lives in that type's **profile** — in the head's mirror and at the decoder — never
in a frame 128 times a second — the same rule that keeps the path out of the header. What the frame carries is the run
itself; **which run of that module it is, the profile tells from the block's length.** **A profile
row is the whole of what is known about a type**: the start register and the length of
each run it reads, and for every register in it the quantity, the scale, the sign and whether it
is the low or high half of a 32-bit value; a house type's row is written once with the type, a
bought sensor's is picked or typed on the handset at the initial sweep, and the host that polls
the sensor carries only the start and the length.

**The block is decoded by the reader of the archive, never by the head.** Palatine checks the
RTU CRC and strips the address, the function code and the CRC; it does not know what a register
means. The head archives the block verbatim and **holds the profiles in its configuration mirror,
shipped with the archive**, so any reader — the exporters, the server, the handset showing a live
value — decodes with the same profiles and the same library. The archive stays raw, and a
corrected profile re-derives everything from the same bytes.

**A block never splits, and it carries no position.** A run is at most 30 B. A module with more
readable registers than that exposes **several runs, each a block, each with a length of its own**
— the profile keeps the lengths distinct, so the decoder tells a module's runs apart by address and
length and nothing else. Runs travel in profile order, and a run that would straddle the end of a
payload waits for the next one, whole. A lost frame costs its own blocks and nothing else: nothing
is reassembled, nothing is reconciled, and there is no sequence number to keep in step. **A 32-bit
value is two adjacent registers inside a block**, exactly as ModBus itself carries it — the
profile says the value is 32-bit and the decoder joins them. A sensor that would need more than a
payload for one value is not carried; anyone who wants one writes a multiplex in firmware and
touches no hardware.

**A mini-NOD needs none of this.** Its 8 B payload *is* its data — the frame header carries the
address and the slot carries the time, so there is nothing to tag. Blocks are what a *host*
builds when it collects from several modules; a unit that owns its own frame just fills it
(`../quark/tubes/BUS.md` is the worked example).

**Two bytes of overhead per block, not per value**, which is what makes it worth doing: an
eight-gas box is one block of 18 B where one record a value would have been 32 B, and a
four-register sensor is 10 B. The address is the module's own, so **a decoder can read a Palatine
it has never seen** — no schema to send up first, nothing to agree, which is the same
table-free property the addressing has, one floor down.

**There is no age field, and the round's own length is what replaces it.** A block carries no
stamp of its own: its time is the frame's, and what bounds the error is **how long a full sweep
of the arms takes — about a second**, four arms in parallel at 9 600 with sixteen units each.
Against a set of quantities whose fastest is a **one-minute average** (WMO CIMO gives 1–10 min
for pressure, temperature and humidity, 2 or 10 min for wind), a second is not an error worth
carrying bytes to describe — the reported value is an average over a minute and has no single
instant to be late against.

**What that costs is a rule, and the rule is hard: a block that cannot be shipped inside the
round is dropped, not sent stale.** With no age field there is nothing to say *how* late a value
is, so one that missed its round must not go up looking fresh. A dead unit's timeout is the case
that produces this, and the filler's reason code says why the value is missing.

**Four arms of up to sixteen units are asynchronous by nature**, and blocks carry them without a
schedule agreed in advance: a missing sensor wastes nothing and a new one redeals nothing. At a
value a minute against 128 frames a second the bandwidth is not a consideration: **a whole station's
weather set is a handful of blocks a second against 128 payloads.**

**The line is where the data comes from, not how fast it is.** A front whose *own* sensors stream
at full rate keeps fixed offsets — Quake's three axes are six bytes a frame, and 4 B of record
overhead each would be absurd. **A front that collects from elsewhere uses records, all the way
across its payload.**

| front | fixed offsets | records |
|---|---|---|
| **Quake · Tesla** | its own fast sensors | Tesla's floor record rides the rest |
| **Pip** | none | the SID carriers — up to 8 carrier records a second, Tesla's `type = 2` shape, 8 Hz steps |
| **Marconi** | none — nothing it ships is fast | **up to 8** polymorphic 4 B records fill the payload — carrier levels and scaled ionogram parameters, the shapes in `../marconi/BUS.md`. Its carrier record is Pip's SID record read in the type-4 context: **the frequency step is the band top / 2¹⁶** (256 Hz against Pip's 8 Hz), amplitudes everywhere on the one 2⁻⁶ dB log scale, the type tag always the low 2 bits of the second word |
| **Palatine** | **none** | **blocks** — one per ModBus transaction, packed until the payload is full |
| **Argus** | **4 × 8 B mini payloads at enrollment-fixed positions** — the segment table decodes | none |

**Palatine has no fast sensor of its own, so its payload is blocks and nothing else, in one slot** —
in mode B some of them are its own WMO table rows, a block each from an address ModBus never assigns
(`../palatine/WMO.md`).
The **radiation does not pass through Palatine** — Quark-Tubes is a mini-NOD on Argus and owns its own
frames (`../quark/tubes/BUS.md`). The whole payload is the meteo set — about sixteen units an
arm, Chinook's air units among them — which asks for a couple of values a minute.

**And Palatine carries no supply in its DATA payload, like every unit.** Its `PORTS` frame reads the up port's `INA238` as socket 0
where it stands behind a Galvani barrier — a second mast, a sensor plot out on 48 V or 300 V —
and 0 in the head's enclosure, where the battery is hard-wired half a metre from a head that
reads it off the BMS; its arms' and `EXT`'s feeds are sockets 1–5 of the same frame (§5). The SUPPLY flag says when the reading leaves its window. The same goes for Argus.
- **The head records what each unit's recording rule keeps, and interprets no DATA payload.**
  Every unit has its own series of HMC files. The rule — `ALL`, `CHANGE`, `DECIMATE`,
  `INTERVAL`, `NONZERO`, `NONE` — keeps or drops a frame, on Palatine a ModBus block, **by
  comparing bytes; it never converts a value**, so what reaches the card is raw. Every state edge
  and every report frame is kept whatever the rule. The defaults are per TYPE — `CHANGE` on
  Palatine, `NONZERO` on Tesla, `ALL` on the rest — and the head's mirror holds the rule per
  address, changed from the handset or by the server. What is kept is coded by the shorter of HCC and raw — HCC column by
  column along the TYPE's layout — cut by width, never read, lossless whatever the layout says
  (`archive/HMC.md`, `archive/HCC.md`, `../mayak/FIRMWARE.md` §6). Beside it runs **the event
  log, the series of the head's own address 0** — the head's second, the source address, a code,
  a detail, a value — written when something changes and never otherwise. **No SOH stream anywhere**: the SOH
  is a state the head holds and shows, not a record.
- **Config rides registers** — the protocol knows only `SET register value` (§9); what a
  register means past the common map is the front glue's. A new front adds registers, never
  opcodes. They carry sensor sensitivity/range and the like — not a backdoor to a smart node.
- **A runtime setting change is a config in the archive:** the head writes the unit's new
  settings into its file from that second, and a reader applies them from there; the node applies
  and keeps no history.

## 6. Storage & uplink

- **The modem carries what the modem can** — on every modem a fixed 16-byte values frame (id,
  second, voltage, temperature, event count, node-alive mask, flags, last peak; CRC-16), sent **as a
  heartbeat once an hour and immediately on a marker, every marker restarting the hour**; markers
  are rate-limited. Nothing that is merely OK is sent — the heartbeat exists because silence cannot
  be told from death, and it carries the two numbers that predict a fault before it is one, the pack
  and the alive mask (`../mayak/FIRMWARE.md` §9). **On LoRa and a satellite module the modem is
  out-only**: the one downlink is a resend request — a radio link able to reconfigure a buried node
  is an attack surface for almost no benefit. **An LTE-M / NB-IoT modem is a link like Wi-Fi** and
  carries the authenticated session below — `HELLO`, the `CURSOR`, the command channel under its
  MAC, the live board — within the tariff's volume, never the archive's. **Wi-Fi, the rich uplink,
  is optional and fully decoupled:** bulk archive upload, NTP, a small API, the head's own OTA; the
  station records and runs without it (`UPLINK_TRANSPORT.md`).
- **Two packets go up, and each unit's send rule decides the second** (`archive/HMC.md`, *The
  uplink*). **The archive packet** is a file's segments as they lie on the card; the server writes
  them into the same files, and reads them with the same library as a pulled card — it goes where a
  link carries the volume. **The live packet** is what each unit's send rule for that link selects
  from what was recorded since the last one, at the link's cadence — the server's live view, never
  its archive. A station with no Wi-Fi that sends Palatine's hourly row over the modem and keeps
  everything else on its card is the same mechanism with other rules. HMC and HCC live only from the
  head on (variable length would wreck the TDMA). Under `ALL` everywhere the full station's raw
  ceiling is ~1,1 Mb/s, less after HCC; the SD buffers bursts.
- **Session protocol: HELLO / CURSOR / cumulative ACK.** HELLO carries station identity;
  CURSOR is the server's durable high-water mark **per address** — the file's sequence and the
  last description entry it holds; ACK is cumulative durability, so a lost ACK is harmless.
  Resume is server-authoritative and idempotent — the head discards its guess, opens the file the
  cursor names and sends forward from the entry after it; a segment sent twice is the same bytes
  under the same entry, so conservative rewinds are free. The SD card is the buffer; the head
  retains nothing it cannot re-derive.
- **The spool:** a buffer per address, written as one segment at 16 kB or 8 s — per-frame I/O is
  wrong on both ends (flash write amplification and FAT hot-spots on the card; header overhead on
  the link). Durable-first: SD, then the uplink. A crash costs at most what the buffers held; a
  lost 12 V costs nothing, the backup cell writes them.
- **SD integrity, three layers, FAT kept** (exFAT as the base — big cards and files, still
  drops into any PC): CRC-framed
  segments in files allocated whole when they open — 1 MB × 2^size, 64 MB under `ALL`, 1 MB
  otherwise — the data growing from the front and 16 B description
  entries from the back, the data written before its entry (a torn write costs at most the last
  segment, and the last good entry is found by binary search at boot — nothing is repaired);
  pre-allocation so steady writes never touch FAT metadata; the head's backup cell, which carries
  it through a lost 12 V long enough to write every buffer, as the last line
  (`../mayak/HARDWARE.md`).
- **The uplink is link-aware, four tiers:** the SD stores what the recording rules keep ·
  markers always (the crude instant ping — the server is the real detector; a false ping fails to
  correlate and drops) · **the live packet** at the cadence each link is given, cut by the send
  rules · **bounded windows on demand** — the server requests `(ts, ±window)` and the head cuts
  that out of the files and sends it as an archive packet. A cheap link carries the archive; an
  expensive one stays markers, the live packet and windows. No science is lost — the record lives
  on the card.
- **The format is the contract, not the component.** A station may be built from any parts; the
  network requires only that it emits the NIC format — swapping a receiver rewrites local glue,
  never the protocol. Every station is self-describing (address + format version + the format's
  description on every card, `HMC.txt`). **Reading is unrestricted** — open, unencrypted, no key,
  and **nothing the station sends is encrypted**; **writing and control are authenticated** — the
  command MAC uses the two-tier station secret: **factory K0 · software K1, 64 bits each, K0‖K1 =
  the 128-bit base**.
  - **K0 is made on the board and leaves it once.** The head's TRNG generates it at the first
    boot in the factory; it lives in the module's encrypted flash behind the Key Manager, read
    protected, and the provisioning tool reads it **once** over the service USB into the
    operator's key store. It never travels by air and never reaches a handset. A station whose
    K0 is lost to the operator is re-flashed over `BOOT0`/USB and makes a new one.
  - **K1 is the working key and K0 roots its change.** Every command is MAC'd with K0‖K1 —
    except **`SET K1`, which is MAC'd with K0 alone**: the key store always holds K0, so a lost,
    leaked or forgotten K1 is overwritten from the server without a visit. A handset receives a
    station's K1 from the operator's server — a signed-in technician, the stations assigned, a
    bounded validity — and loads it before leaving where it will work offline; a handset never
    holds K0, so a lost handset costs one re-key.
  - **The nonce is a counter, not the frame index.** Each station keeps a **32-bit command
    counter**, monotonic, persisted in the head's append-only configuration cells so it survives a
    reboot; a command whose counter is not above the stored one is dropped. The station reports
    the counter in `HELLO` and as a register over BLE, and the server and the handset — one
    counter space between them — send stored + 1. The MAC covers the 3-byte station identity,
    the counter and the command.
  - **The archive session is authenticated, the data is not.** `HELLO` is MAC'd with K0‖K1 and
    carries the counter, and **every archive packet carries an 8 B truncated HMAC chained to its
    session**, so nobody writes segments into a station's files but the station — 8 B on a packet
    of ~16 kB segments. The live packet and the markers on the modem carry no MAC: a false marker fails
    to correlate, and a reading is open by doctrine. **Nothing is encrypted and nothing needs a key
    to be read**; a key is needed only to write — to the station, or into its archive.
  The S31's TRNG, SHA engine and Key Manager carry this in hardware (`../mayak/HARDWARE.md`).
- **The server coordinates, the station stays dumb.** The server is the operator's, not the
  station's. Markers and the live packet go up; **the live board comes back down** — the server
  fans every head's live packet and markers out to every head that subscribes, and a head shows or
  archives what it receives and acts on none of it. The server correlates across stations and
  vehicles and sends back only what it needs — group commands (a regional pre-drain before a
  storm, a config push) and the bounded window requests — all over the authenticated command
  channel. The server's own brains (correlation, spatial QC, maps,
  targeting) are a separate project, never the station's.

## 7. The node's contract

- **Anything large goes down as a stream: `LOAD`, then the frames, then `APPLY`.** A firmware
  image or a table (a classifier's model, a calibration set) is not cut into pieces with their own
  headers; the master says once what is coming and then sends it, 32 B a frame, back to back, on
  **its own line** — every NodBus run is full duplex and the master's TX is nobody's slot, so the
  stream costs the round nothing and the unit keeps measuring while it receives.
  - **`LOAD`** (op 18, M→N or M→C): `arg0`·`arg1` the number of DATA frames that follow (16 bits,
    2 MB), `arg2` the slot — 0 the firmware image, 1 a table. The unit answers ACK and opens its
    second flash slot.
  - **The stream**: DATA frames, `kind` 9, the master's header and 32 B of payload each, the frame's
    own CRC-16 and nothing added. **The first frame is the image header** — the TYPE it is for, the
    slot, the byte length (uint32), the CRC-32 of the body, the version — and the body follows,
    **the last frame padded with zeros to 32 B**. The `frame` field counts the stream; a CRC miss or
    a gap aborts it, the unit answers ERROR and the master starts the stream again from `LOAD` — no
    per-frame resend, because a restart costs seconds: at 2²¹ the line carries ~160 kB/s of payload,
    a 512 kB image in under four seconds. **Through an Argus the card cuts each 32 B into four 8 B
    mini payloads** at its segment's data rung, 2²⁰ alone or 2²¹ chained — 2²⁰ ÷ 10 bits × 8 B / 16
    B ≈ 52 kB/s, ~105 kB/s at 2²¹ — a mini-NOD's image in under a minute. At the end the unit checks
    the CRC-32 against the whole body and answers ACK, or ERROR with the reason.
  - **`APPLY`** (op 19, M→N or M→C): boot the loaded slot **as a trial**. The bootloader marks the
    new image untried and starts it; the image confirms itself the first time it answers the
    master's `GET VERSION` — it is on the bus and talking — and an untried image that resets
    before that, by watchdog or fault, hands the next boot back to the old slot. A table in slot 1
    needs no `APPLY`: the unit takes it at its next boot.
  - **Who loads whom**: a Bifrost loads its units, an Argus its mini-NODs, the Mayak its cards over
    the trunk — the same link. Kronos and Hermes stand in the enclosure on I²C and keep the H523's
    `BOOT0` path; the head's own image comes over Wi-Fi. `BOOT0` stays on every H523 as the
    zero-firmware factory path. Sensors carry no NV config — the node reconfigures them at every
    boot; calibration matrices persist in MCU flash.
- **"Dumb" means: owns no global state** — no wall-clock, no storage, no orchestration. Inside
  its box the node is a capable worker: auto-detects its population, assembles its own packet,
  manages its own power, recovers from clock loss, resends from its buffer. The constraint is
  power-driven, not a capability limit.
- **The unit places its own grid, and nothing downstream does delay arithmetic.** The clock reaches
  a unit late by its run's propagation, so the grid it would derive from `SYNC` is late by the same.
  The card measures the run (ranging, `../bifrost/FIRMWARE.md` §4) and writes the result to the unit
  as house register **`0xFF11 ROUTE`**, in ticks of 2²⁷, before `SYNC` and again whenever a periodic
  ranging moves it by more than 2 ticks; the unit takes its `SYNC` edge **`ROUTE` early** — a shift
  to its own timer's rate — so its frame starts, its slot compares and its sample-index origin sit
  on the card's grid, and its frames arrive at the card in their nominal slots (the run's
  propagation up puts back what the unit took off). A sonde behind an Argus does the same with the
  route the Argus wrote, the Argus with the route its Bifrost wrote: every grid in the station is
  Kronos's. `ROUTE` is persisted with the set, so a rejoin has it. A unit that holds no route runs
  on the delayed grid, and the card reports it unranged in the segment table's status byte, not in
  the frame header. The card checks a frame's second and prepends the high bytes; no route is
  subtracted at the card, at the head or in the archive. **The board's own delays go into the same
  shift, as `SKEW`** — house register **`0xFF12`**, int8 in the unit's own ticks, applied with
  `ROUTE`: the origin is the captured edge less `ROUTE` less `SKEW`. `SKEW` is a constant of the
  image from datasheets, never a site's business: the capture synchroniser's 2 timer clocks, plus
  the difference between the clock channel's parts and the data channel's on the communication board
  the unit reads on its `ID` — a house table by `ID` code, **0 for every board whose two channels
  use one part family** (`G-I-N-025`: ISO145x on both, 19 + 36 ns typical on either channel from the
  datasheet; the optical boards: one module type on both; an in-box cable: wires), so the table is
  all zeros until a board's datasheets say otherwise. One register, one table, no measurement.
- **Every record reaches the head one second after it was measured, and the unit holds its part
  of that second — house register `0xFF13 DELAY`**, in frames, written by its parent at floor-up
  and persisted with the set. The unit's buffer is a two-gate FIFO: a frame is written when it is
  made and sent at its own index plus `DELAY`. **`DELAY` is never below 8** — a floor fixed in the
  image, because this buffer is what `RESEND` is answered from. A Bifrost writes 32 and holds 96
  itself; an Argus told 32 holds 16 and writes 16 to its sondes; 128 on every path
  (`blocks/nodbus.md`).
- **A measuring unit subtracts its own chain's latency, `LAT`, where it makes the record — from
  the datasheets, not from a bench.** A converter's digital filter and the analogue front end
  ahead of it delay the input by a constant: sample `n` describes the input as it was `LAT`
  earlier. Every unit whose record carries a time inside the frame — Tesla's edges, Pip's
  carrier phases — holds `LAT` as a constant of its image in its own ticks: the converter's
  latency as the datasheet gives it for the configuration used, plus the front end's group
  delay from the computed chain in its `HARDWARE.md` at the band the time is taken in. It is
  subtracted once, at the record, like `ROUTE` at `SYNC`; a bench can refine it and need not.
  A frame-synchronous unit that converts on its own schedule places the conversion's instant
  the same way (Pascal: the start plus half the conversion time).
- **A unit is never left asleep to save power, and `END` is the one verb.** The station does not
  save power in normal running — every rail was sized for its own board's full load — so a unit
  with nothing to do is not parked, it is **ended and its branch is switched off**. `END` takes no
  argument: stop measuring, flush what is buffered, answer, expect the supply to go.
- **OFF has two implementations, and which one applies is decided by whether the board has a feed
  above it.** **Behind a port** — every measuring unit, a remote Argus, a bought device on a
  `G-12-S`, a load on a `G-24-S` — off is **`ENABLE` down on the source power board**, and the
  far end then draws nothing at all; what is left is the source cell's own quiescent.
  **In the enclosure** — Kronos, every Bifrost/Argus, the Mayak itself — there is no `ENABLE`
  above the board and none is wanted, because the battery is a wire and not a switch
  (`../galvani/README.md`); off there is **a deep sleep the Mayak commands**, being master over
  every one of them, and it costs milliwatts. **A deep sleep is not an operating mode. It is the
  off switch for a board that has no other one.**
- **The wake is the link the board already has, never a wire.** A card wakes on its trunk USART's
  start bit, Kronos on an address match on the time bus I²C, a unit behind a port on its supply
  returning. **No wake wire, no wake pin, nothing added to any connector** — and `WAKE` has no
  opcode for the same reason.
- **Which fixes how DEEP an in-box board may sleep: Stop, and never Standby.** The link that has
  to raise it is an ordinary full-rate port — **no bus in this station runs on an LPUART**
  (`blocks/nodbus.md`), so the trunk is a plain USART and the time bus a plain I²C. A board
  therefore sleeps in a state that **keeps that one peripheral clocked from a source Stop does not
  take away**, with its wake-on-start-bit or wake-on-address-match armed, and everything else
  down. **The part does it on exactly the two peripherals in question**: `UART4` carries wake-up
  from Stop and `I2C1` carries it too (DS14540 Rev 3, Tables 9 and 7). **Standby would be deeper and would also be the end of it**: there is nothing left to hear
  the Mayak and no wire to shout on, so a board in Standby is a board somebody drives out to.
  A unit behind a port has no such constraint — its supply comes back and that is a boot.
  The clock IS the kill switch, surgical at every level (`blocks/nodbus.md`); the
  station-level survival loop is the power doctrine's (`POWER.md`, `../mayak/FIRMWARE.md` §11).
- **The off sequence is ordered, and the order is `END` first**, then the clock, then the feed:
  `END` → `PORT_CLK` 0 (the unit falls to RC and mutes) → `PORT_PWR` 0 (§10, which owns both).
  **`END` is courtesy and can be skipped; the clock and the feed are what actually do it.** A few
  seconds either way costs nothing and it is what stops a unit being cut in the middle of a write.
  The same order runs the battery deficit — the head sees the pack going through Hermes and ends the
  units itself, ahead of the BMS (`POWER.md`). **The unordered case still
  exists and is accepted**: the BMS dropping the pack is a hard reset of everything at once, and
  every unit's configuration survives it in its own store.
- **Self-test runs on the node, in quiet windows** — every seismic sensor's hardware self-test
  (a known internal force → a datasheet-bounded delta), folded into the health report. The worst failure for a buried instrument is silently recording
  garbage while looking fine.
- **Per-sensor diagnostics are mandatory on every TYPE.** One node-local sensor-# space (Modbus
  address or fixed SPI index), one vocabulary (OK / SELFTEST_FAIL / NO_RESPONSE / DEGRADED),
  **one transport: the `HEALTH` frame, `kind` 8, on `GET HEALTH`** (§5) — and one signal that
  there is something to read, **the header's `FAULT` flag** (§1). Nothing about health goes up
  unasked: the flag rises, the head asks once. After a floor's sweep the head pulls `HEALTH`
  from every unit found, which is how it learns the health **and the present-mask** (NodBus:
  in the `HEALTH` frame; Modbus: house register `0xFF09 SENSORS` beside `0xFF01 IDENT`),
  archived into the station metadata — "not fitted" is never indistinguishable from "fitted,
  reading zero". **A population change is a re-announce, never a silent rejoin:** the node
  persists its mask and a mismatch at boot forces the full RC announce — population is
  identity. NodBus discovers (WHO_AM_I); **Modbus verifies a configured roster, never extends
  it** (house MODs self-describe and are the exception).
- **Value-plausibility QC is the third health source** — range, physically-impossible step
  (slow scalars only; event channels are exempt), persistence (raw fronts: loss of the ±LSB
  noise floor; compensated units: status registers + cross-checks), and cross-sensor
  consistency as the discriminator. Run at the write cadence, not continuously. On suspicion
  the node may soft-reset the sensor (logged, warm-up flagged). **Flag, never fake** — the raw
  value is always logged; the verdict rides the header's `status` code and its `FAULT` flag (§1),
  the detail waits in `HEALTH`; the measurement's trigger and the supply's edge are the header's
  flags the same way.
- **Configuration lives in the MCU's own flash, in fixed append-only cells** — no memory part,
  no filesystem. A cell = id + value + check; writes append, the reader keeps the last good
  cell per id; a full 8 KB sector (512 cells) erases and rewrites the live set. No mirror
  sector: the head holds the authority and re-seeds at boot — the flash copy makes a boot
  silent, not possible. **The persisted set — number, slot, BUSCFG — carries one check over the
  whole set, and a set that fails it is no set**: the unit boots as fresh (§3). **The live set is
  rewritten once a year on its own**, erase and write, to refresh the charge in the cells — one
  cycle a year against a ten-thousand-cycle part. A unit that loses its set twice has a failing
  flash and is replaced, not debugged.
- **Indicator LEDs:** the station-wide rule is `core/HARDWARE.md`'s (none standing anywhere;
  the head's one service LED lives only in the button-gated window).
- **Station position is acquired, never typed**: Kronos's `POSITION` register carries what the
  steering receiver last parsed, and the head reads it, persists it and writes it into the
  archive header's `STATION` section. A written position is for a site with no receiver, and Kronos refuses it while a
  source is usable. Coarse ±5 m is plenty — stations sit > 100 m apart; the UM980 build is exact.

## 8. The station model

A NIC **Station** = one head + the trunks + whatever fronts are plugged in; a "product" is a
node configuration. Removing a node frees a TDMA slot, and a freed slot is an **open extension
point** — anyone can build a node for it, and an unknown TYPE is logged raw and flagged, never
a crash. The platform is to be small portable-C libraries reaching hardware only through callbacks,
glued by the one MCU-specific layer, every library compiling and unit-testing on a host — none is
written yet; the one code that exists is the archive's Python reference (`archive/ref/`). Fronts
share the base platform and differ by their front delta — base-identical, never bit-for-bit.

## 9. The opcode index

The human index of everything the master, a card and a node say on the CONTROL plane. `M` =
master, `N` = node, `C` = card, `bcast` = 0xFF. **The numbers are this table's** — the firmware
header is written from it, never the other way round.

**Verbs only for what does something; everything that is read or set is a register.** A unit
has a register map — version, link health, the house code, a calibration zero, the front's own
settings — and the master reaches all of it with two verbs, `GET` and `SET`. A register is either
there or it is not, so the master never has to remember what a unit supports, and a new front
adds registers, never opcodes. The map is one across every unit we build, NodBus and ModBus
alike (`0x0000` the value · `0x0001` a second value where the quantity has one · `0x0002` STATUS
· `0x0003` RAW · from `0x0004` the unit's own · `0xFF00+` the house block, §5); `blocks/modbus.md`,
*The house map*, carries it in full.

| op | # | dir | arg0 | arg1 | what |
|---|---|---|---|---|---|
| DISCOVER | 1 | bcast; the reply N→M under the same op | 0 | 0 | who is there? — RC dialogue only. Every unit not in a running round answers once, **its tag (CRC-16 of the UID) in the two time bytes**, after `tag × 10 µs`, listening first and backing off on a bad echo; **the round ends on 700 ms of silence and is then broadcast once more to confirm it** (§2). Also the empty-port probe, once a minute at 38 400 (§3) |
| ASSIGN_ADDR | 2 | M→N, **by tag in the two time bytes** | the address | the slot | the number and the position the sweep gives the unit whose tag matches; a multi-slot unit gets its further numbers and slots by further `ASSIGN_ADDR`s to the same tag after the host has read `slots` (§2) |
| BUSCFG | 3 | bcast | item: 0 NODE_COUNT · 1 FRAME_RATE code · 2 RUNG | value | the bring-up schedule, one frame per item; the rung is one of two, **2²⁰ for a lone unit, 2²¹ for two to eight** (`blocks/nodbus.md`) |
| SYNC | 4 | bcast | — | — | the start, **fired on a second's boundary**: its header reads that second's low byte and `frame` 0, and the node loads both at this edge, switches to the rung and begins the round. HSE is already on the wire — the node took it at BUSCFG (`blocks/nodbus.md`, *How a bus starts and heals*) |
| TICK | 5 | M→N | — | — | nothing but its header: **once a second, in the gap after the last slot**, the time at its own start-bit edge — the phase for a unit coming back with no neighbour to hear (§3). A running unit ignores it |
| END | 6 | bcast or addressed | — | — | **stop measuring, flush, answer, expect the supply to go.** Courtesy before the clock and the feed, and skippable — `PORT_CLK` and `PORT_PWR` are what do it (§7, §10). WAKE deliberately has no opcode: a board wakes on the link it already has |
| HARD_RESET | 7 | bcast or addressed | — | — | back to the default number and the default configuration cells; the TYPE and a factory calibration stay (§2) |
| RESEND | 8 | M→N | frame | unix.0 | CRC miss → send that frame again from the buffer |
| GET | 9 | M→N; the reply N→M under the same op | register | reply: the value, `arg1` low and `arg2` high | up to 16 bits answer here; a wider register answers as a DATA frame, `kind` 5, the register number first; **`GET REPORT` · `GET PORTS` · `GET HEALTH` answer with that frame itself** (§5). The node's own registers only — never the ModBus bridge, which is the tunnel's alone |
| SET | 10 | M→N | register | value, `arg1` low and `arg2` high | written, then ACK. **A register wider than 16 bits is written as a DATA frame down, `kind` 5, the register number first, the value after it, zeros to 32 B — the same frame a `GET` answers with, sent the other way** — in the unit's third frame-time like any CONTROL, and answered `ACK`; a card relays it as it relays a `TUNNEL` frame, an Argus by `SUB` onto the segment. **One sanctioned use across every port-carrying board: the per-port current limit** — the number the firmware trips on, read from the block's INA238 and executed with `PORT_PWR`, so a limit is set from the handset at run time and **nothing in the frame grows for it** (`../galvani/HARDWARE.md`, *Protecting the feed*) |
| ACK | 11 | N→M | the op acknowledged | — | |
| ERROR | 12 | N→M, **only as the answer to a command** | code | detail | the op that failed and why — a register the node does not have, a tunnel that timed out, a calibration too noisy. **Never unasked**: a board's own fault is the header's `FAULT` flag and `HEALTH` on `GET` (§1, §7) |
| DIAG | 13 | — | — | — | **retired, the number not reused** — its vocabulary is the `HEALTH` frame's, its trigger the `FAULT` flag (`WHY.md`) |
| EVENT | 14 | bcast | event class | — | the station event board: the source is the address, the instant is the header's. Policy lives in the Mayak's config; a receiver may only do something **local and reversible** (tag a record, hold a buffer, pre-drain). At the head an alarm or EVENT also feeds the **marker pipeline** (§6): an archive marker for search, the immediate rate-limited radio marker, and the head's **event assessment — short windows, never real time** (`../mayak/README.md`) — the server stays the real detector. µs correlation stays with the stamps and the server |
| PORT_PWR | 15 | M→C | port, **0 = every port on the card** | 0 off · 1 on | **switch a port's feed and line side** — its `ENABLE` and `LINE_EN`, one GPIO. §10 |
| PORT_CLK | 16 | M→C | port, 0 = all | 0 drop · 1 run | **rip or restore a port's clock**. §10 |
| FLOOR | 17 | M→C | — | — | **bring your floor up** — the card's one original verb. §10 |
| LOAD | 18 | M→N, M→C | frames to follow, `arg0` low and `arg1` high | slot in `arg2`: 0 image · 1 table | **a stream is coming**: that many `kind` 9 DATA frames on the master's own line, the first the image header, the last zero-padded; ACK opens the slot, ACK or ERROR closes the stream on the CRC-32 (§7) |
| APPLY | 19 | M→N, M→C | — | — | **boot the loaded image as a trial**; it confirms itself on its first answered `GET VERSION`, else the old slot comes back (§7) |

**What is a register and not a verb.** A unit's state answers `DISCOVER`. `VERSION`, `HEALTH`,
`IDENT`, `CALIBRATE`, **`TAG`** (the CRC-16 of the UID, §2) and **`slots`** are registers behind
`GET`/`SET`. `BUSCFG` is one op with the item in `arg0`. No slot is ever granted: the round is
clock-anchored.

Two notes that keep the table honest. **The ModBus tunnel — the one ModBus path, and the
bridge node is a byte pipe.** The Mayak composes the complete RTU package (module address,
function, data, its RTU CRC) and sends it down as a **DATA frame, `kind` 1 TUNNEL**, in the gaps
(§1). The bridge node — Palatine; no other unit carries an arm — unwraps the payload and streams
it onto the arm verbatim; the sensor's whole reply comes up verbatim as **`kind` 2 TUNNEL_REPLY**
in the gaps. **One question, one answer**: the Mayak holds one package open per bridge and pairs
the reply by that alone. **The node composes nothing, parses nothing and retries nothing** — a
timeout is an `ERROR` from the node, and end-to-end retry is the Mayak's, which also validates
the RTU CRC itself. A package is capped at the 32 B payload — no fragmentation; **the mini bus
has no ModBus tunnel** (no arm, no ModBus registers) — a sonde's own dialogue is the plain
CONTROL frame addressed through its Argus by `SUB` (§1). `GET`/`SET` stay for the node's own registers only.
**The fault ladder:** one CRC miss → RESEND; persistent misbehaviour → HARD_RESET; still dead →
declared dead and reported over the uplink (the card-level rogue-unit ladder is §10 and
`../bifrost/FIRMWARE.md` §11).

## 10. The card's control surface — Mayak ↔ Bifrost

**`M→C` is the master talking to a card, not to a node.** A card is addressed like anything else
(TYPE 2 + NUMBER), and the CONTROL frame is the same 12 B, `SUB` 0. **The card holds no policy for any of
it** — it drives a pin or runs a sequence and reports; every decision is the master's.

**The card has three verbs and the reporting that comes back, and that is the ceiling**: *bring
your floor up*, *power a port*, *clock a port*. Anything more and the card stops being a bridge.

### What the master can say

| op | argument | what the card does |
|---|---|---|
| **FLOOR** | — | enrolment, ranging, clock start and re-anchor run on the card; results travel up as data |
| **PORT_PWR** | `arg0` = port, **0 = all** · `arg1` = 0 off / 1 on | drives that port's `ENABLE` and `LINE_EN`, one GPIO. **This is the hard reset**: the far unit loses its supply completely, not just its clock, so nothing of its state survives |
| **PORT_CLK** | `arg0` = port, **0 = all** · `arg1` = 0 drop / 1 run | rips or restores that port's clock. **The softer kill** — the unit stays powered, mutes on unnegotiated clock loss and rejoins by itself when the clock returns (§3) |

**Why both kills exist, and they are not the same tool.** The clock rip is surgical and cheap —
the unit is alive throughout and comes back in one round. **Cutting the power is the only thing
that clears state a confused unit is holding**, including a part with no reset pin. A unit that
will not answer a clock restart gets the supply pulled.

### What the card says back

| op | when | carries |
|---|---|---|
| **`FAULT` in its own frames' header, and the table** | while a port is killed, lost or unknown | the segment table re-sent on the change, the slot's status saying which and why — 3 dead · 4 unknown board · 5 port faulted; the detail on `GET HEALTH`. Nothing unasked beyond the flag and the table |
| **filler frames** | every round a proxied unit is absent | a DATA frame under the unit's address, `kind` 3, the reason code in the payload — *unit missing* · *no data arrived* · *arrived corrupt*. **The slot is never given up**, so absence is positive data rather than silence |
| **the segment table** | after FLOOR, and on any population change | one 32 B payload from the card's own address, `kind` 4 — **8 × 4 B: unit address · port · rung code · status** — the wiring the sweep built, delivered as data. **The same table shape lives at every level**: a card reports its spurs' units, an Argus reports its positions' sondes, and each master knows only its own children; **the Mayak composes the reported tables into the station tree** and decodes by walking it — port → table → slot → unit → (a carrier's) table → position → sonde. The roster and the mirror are written from these, never typed |
| **`PORTS`** | once a minute, and on `GET PORTS` | one 32 B payload from the card's own address, `kind` 7, in the card's map — `VBUS` · `CURRENT` per power socket as raw `INA238` registers, 4 B a socket, the up port first then down 1–4; the four down ports' leak-watch words; the card's temperature (§5) |
| **`HEALTH`** | on `GET HEALTH` only | from the card's own address, `kind` 8, in the card's map — the link-error counts per port and the rest of the card's internals (§5) |
| **a unit's `REPORT`** | as it arrives | passed through like a DATA frame — the second prepended, nothing read. An Argus tiles its sondes' 8 B report payloads by position under the NUMBER that owns them, as it tiles data |

### The recovery ladder — the sequence, in order

**The card runs this alone.** The master is told at every step and can override with the opcodes
above, but nothing waits for it — a site with a dead uplink still recovers its own units.

| # | step | what the master sees |
|---|---|---|
| 1 | mark the unit faulty, **hold its slot with filler** | filler + reason code, every round |
| 2 | **wait 8 rounds**, 62,5 ms at 128 Hz — the unit resets itself and rejoins on its own (§3); most faults end here | the filler stops; normal data resumes |
| 3 | still silent → **cut the port's power** at the Galvani `EN`, let it sit down, **restore it** — the unit boots cold, hears the clock, rejoins | the table re-sent, that slot's status 5 *port faulted*; `FAULT` up on the card's frames |
| 4 | still silent → **retry, three attempts total** | the table each time |
| 5 | still nothing → **declare it dead** and keep filling | a standing reason code, and the uplink carries the detail |

**Step 2 is the one that earns the ladder.** The unit's own reset and rejoin need nothing from
the card, so the card does nothing first; the power cut is for the unit that will not come back
by itself, because it is the only thing that clears state a confused unit is holding.

**The round is never rewritten.** Not during the ladder, not after a unit is declared dead: a
missing unit is an absent slot, not a smaller round (§3).

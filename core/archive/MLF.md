★ N.I.C. ★

# MLF — the station's archive, Heimdall's profile

> **Design-stage concept — nothing built.** The format is fixed here; its figures are reasoned,
> and NRC's ratios measured on open recordings only.

The station's archive is written as **MLF, the Matroshka Log File** — its core in NIC-SDK's
[`MLF.md`](https://github.com/Project-NIC/NIC-SDK/blob/main/MLF.md): the file taken whole, the data growing up and the description down,
segments and configs, COBS, the codecs, the checks. **This page is Heimdall's profile**: what the
core leaves to the project (MLF §10). The segments are coded by **NRC**, the NIC Rice Codec, its
core in NIC-SDK's [`NRC.md`](https://github.com/Project-NIC/NIC-SDK/blob/main/NRC.md) and Heimdall's reading of the records in
[`NRC.md`](NRC.md) here.

The archive is what the Mayak writes to its two cards, what the uplink carries and what the server
stores. **Every unit has its own series of files** — a Quake its own, a Palatine its own, a Tesla its
own — because the units differ in everything an archive cares about: a Quake pours 128 frames a
second, a Palatine changes a few values an hour, a Tesla sends zeros most of the time. **What a
unit's file holds is chosen by its recording rule; what leaves the station is chosen by its send
rule**, and the two are independent: a station in a field with no Wi-Fi sends a small packet of
weather an hour over the head's modem and keeps everything else on its cards until they are read.

## The rules — the head selects, it never computes

**A rule keeps or drops a frame by comparing bytes; it never converts a value.** What reaches the
card is the unit's own bytes, raw, in its own frames: the archive stays derive-from-raw, a mean or
a calibrated value is the server's, and the head interprets no payload — it compares bytes and
cuts payloads along the layout's widths.

| rule | keeps |
|---|---|
| `ALL` | every frame |
| `CHANGE` | a frame whose payload differs from the last one kept; **on a ModBus-block payload, per block** — a block whose bytes differ from the last kept block of the same module and length |
| `DECIMATE n` | every `n`-th frame of the second, from frame 0 |
| `INTERVAL s` | the first frame of every `s` seconds, aligned to seconds divisible by `s`; on a block payload, the first block of each module in each interval |
| `NONZERO` | a frame whose payload is not all zeros |
| `NONE` | nothing — not even what the paragraph below keeps; it is the off switch |

`CHANGE` and `NONZERO` take an optional **`at least every s`**: a frame is kept anyway when `s`
seconds have passed since the last kept — under `CHANGE` the last of the same block, under
`NONZERO` the last of any — so a quiet unit is seen to be alive. `DECIMATE` with `n` of 0 or 1 and
`INTERVAL` with `s` of 0 keep every frame; `INTERVAL` keeps a record when nothing of its kind — the
frame, or a block of its module — was kept in its interval, the second divided by `s`.

**Whatever the rule but `NONE`, these are always kept**: the first record an address offers, a
record whose `kind` or `status` differs from the last one kept — so every state edge, every ALARM,
SUPPLY and FAULT edge, the first filler and the first frame after it reach the card — and every
frame of `kind` 5–8, the reports and answers.

**A ModBus payload is cut into its blocks** — `address · length · data`, whole, up to a zero
address (`../../palatine/FIRMWARE.md` §7) — and a frame of `kind` 0 offers each block to the rule as
a record of its own; a payload with no block, or with a block that runs past the 32 B, is offered
as a frame, and so is a frame of any other kind. The head compares against at most 256 blocks of
different module and length; a block past them is kept.

**A rule is set per address and has a default per TYPE**, held in the head's configuration mirror,
changed from the handset or by the server over the authenticated channel:

| TYPE | default recording rule |
|---|---|
| Palatine | `CHANGE`, per block, at least every 3600 s |
| Tesla | `NONZERO`, at least every 60 s |
| every other | `ALL` |

**The send rule** uses the same vocabulary over what was recorded — it cannot send what the
recording rule dropped — and on a block payload may name the modules it takes: a block of another
module is not sent, and a frame goes to the rule. It is set per address and per link (*The
uplink*).

## The files

- **The size follows the unit's rate** (MLF §1, `size`): a setting per address in the head's
  configuration mirror, set like the rules. The default is **64 MB for an address recorded under
  `ALL`** and **1 MB for every other** — a Quake fills 64 MB in hours, a Palatine under `CHANGE`
  takes months over one. The base is exFAT.
- **A file closes only when full**, as the core has it — a Palatine's file runs for months, a
  Quake's for hours.
- **The path is the address and the day the file opened**, on the station's scale:
  `52/20261001/120000.mlf` — the address in two upper-case hex digits, then the date, then the
  file's first second, the second read as a calendar without leap seconds. The head's event log is
  the series of address `00`.
- **The cards fill in a circle** (the owner, 14 Nov 2026). Both cards carry the same files, and
  **512 MB is kept free on each**: when either falls below it, the oldest files go — by their first
  second, whatever the address, never a file still open — from both cards, until both are above it
  again, so a write never meets a full card. A file goes whole; exFAT frees no part of one. A card
  of hundreds of GB holds a year and more of a station under its default rules, so the circle
  turns only where nobody comes; a file given up before the server acknowledged it whole is a line
  in the event log.
- **The two cards carry the same bytes.** The second card is the mirror.

## The header — the source and the sections

**The source** (MLF §2, bytes 16–19): **the station** in bytes 16–18, its identity, little-endian,
and **the address** in byte 19 — `TYPE«4 | NUMBER`, 0 for the event log.

**Every second in the file is the station's continuous scale** — GPS time in the Unix epoch,
leap-blind, as Kronos labels it (`../PROTOCOL.md` §4): the header, the records, the configs and the
entries alike. UTC is derived at export, from the `TIME` section in force.

A section is framed as the core frames it (MLF §3); a reader skips an id it does not know.

**Heimdall's sections**, ids 1–9 of the profile's 1–127 (MLF §3); id 8 is not used — a codec's
table rides in the core's `CODEC`, 128:

| id | section | |
|---|---|---|
| 1 | `STATION` | `name 32 B` UTF-8 · `latitude 4 B` · `longitude 4 B` (int32, 10⁻⁷ degree) · `elevation 2 B` (int16, metres, `0x8000` unknown) — the position the head writes to Kronos's `POSITION` |
| 2 | `UNIT` | `space 1 B` (1 NodBus) · `type 1 B` · `card 1 B` · `port 1 B` · `slot 1 B` · `rows 2 B` (frames a second, 128 or 256) · `positions 4 B` (an Argus NUMBER: the mini address at positions 0..3, position 0 the first byte, 0 empty) |
| 3 | `LAYOUT` | the payload layout of the unit's TYPE — and on an Argus NUMBER one per mini TYPE seated |
| 4 | `PROFILE` | one per ModBus type a Palatine reads |
| 5 | `SETTINGS` | the unit's registers, and its sondes' |
| 6 | `RULES` | the recording rule, then `links 1 B` and per link `link 1 B · rule · modules 1 B · module addresses` — a rule is `mode 1 B` (0 `ALL` · 1 `CHANGE` · 2 `DECIMATE` · 3 `INTERVAL` · 4 `NONZERO` · 5 `NONE`) `· n 1 B · s 2 B` (seconds, 0 none); a link is numbered as the head's uplink table numbers it, modules 0 meaning all |
| 7 | `RESPONSE` | `SUB 1 B · length 2 B ·` the unit's write-once calibration block verbatim, for a sensor that needs a response beyond its factory sensitivity (`../INTEROP.md`) |
| 9 | `TIME` | `offset 1 B` (int8, GPS − UTC in seconds, 18 in 2026) · `pending 1 B` (int8, the announced leap, +1, −1 or 0) · `at 4 B` (the second it takes effect, 0 none) — Kronos's `LEAP` register as the head reads it; a config is written whenever it changes |

**`LAYOUT`** — what a TYPE's 32 B payload holds:

```
[0] space     1 B   1 NodBus · 2 mini
[1] type      1 B
[2] revision  1 B   the payload revision the unit's own document defines
[3] class     1 B   how the bytes no field describes are read:
                    0 nothing left · 1 ModBus blocks · 2 the TYPE's records
[4] count     1 B   fields
[5] reserved  3 B
[8] fields    count × 32 B:
    offset 1 · width 1 (1..4 bytes) · flags 1 (bit 0 signed, bit 1 big-endian) · channel 1 ·
    exp10 1 (int8) · ref 1 · mantissa 2 (int16, 0 ≡ 1) · add 4 (int32) ·
    name 8 · unit 8 (ASCII, NUL-padded) · reserved 4
    physical = (raw + add) × mantissa × 10^exp10
```

Fields sharing a **channel** are one series in time — two samples of one quantity in one payload
give both fields one channel. **`ref` names a reference channel**, plus one, 0 none: a channel
measuring the same motion or field as this one — Quake's two accelerometers on one axis — against
which NRC may code this one as the difference; the reference is always a lower channel. **The class says what the fields do not**: Quake, Gauss, Pascal and
the `Quark` boards are fields and class 0; **Palatine** is class 1 with no fields, its payload
read with the `PROFILE` sections; **a record front** — Pip, Marconi, Sputnik, and Tesla behind its
fixed offsets — is class 2, its records read by the TYPE's decoder as its own `BUS.md` defines it
at the layout's revision. **An Argus NUMBER's payload is four 8 B positions**, each read by the
mini layout of the sonde `UNIT` seats there. A TYPE the head holds no layout for is archived with
none: a reader gets its bytes and an unknown TYPE, never a failure.

**`PROFILE`** — `type 1 B` (ModBus 4..61) · `runs 1 B`, then per run `start register 2 B ·
registers 1 B` and per register `flags 1 B` (bit 0 signed · bit 1 low half of a 32-bit value · bit
2 high half) `· exp10 1 B · mantissa 2 B · add 4 B · name 8 B · unit 8 B`.

**`SETTINGS`** — per unit or sonde: `SUB 1 B` (0 the unit itself) `· count 1 B`, then `register
2 B · length 1 B · value`, as the unit holds it.

## The data — the records

**A record** is one kept frame, or on a block payload one kept block, with its time:
`second 4 B · frame 1 B · kind 1 B · status 1 B · form 1 B` (0 frame, 1 block) `· length 1 B ·
bytes` — 32 B of payload for a frame, the block as it rode (`module · length · data`) for a block.
A block's data is at most 253 B, so its record's length fits its byte. Codec 0 (MLF §4) is the
records back to back; codec 1 is NRC, read as [`NRC.md`](NRC.md) says. **A segment is one write of
one address's kept records.**

**A late frame is just a record.** A frame repaired after its second was written arrives as a
record with an older second; the reader places every record by its second and frame, and of two
records for one frame the later written is the one.

**A config** (MLF §4) is written when a section of the header changes — a setting, a rule, a
layout, a seat on an Argus. **A setting change does not close the file**: the unit's `CHANGED` state code marks the frame, the
head writes a config from that second, and a reader applies the last config at or before each
record's second over the header.

## Carrying its own description

**Nothing a reader needs is outside the file and its card** (MLF §8): the layouts, the profiles and
the settings ride in the header and the configs, and **every card carries `MLF.txt` at its root** —
MLF's core, NRC's, this profile and [`NRC.md`](NRC.md) here, in words. **The server keeps the same
text beside its archive.**

## Writing it — the head

**Each address has a buffer in PSRAM, and a buffer becomes a segment when it holds 16 kB or 8 s
after its first record, whichever first.** Eight seconds and not one: every segment restarts its
predictors, and at 128 frames a second a one-second segment would spend about a tenth of itself on
the warm-up of an order-8 predictor. A Quake writes a segment every few seconds; a Palatine
under `CHANGE` writes a small one when something changed. At the close of a second
(`../../mayak/FIRMWARE.md` §6) the head walks the second's frames by address, applies each
address's recording rule and appends what it keeps to that buffer. **A crash costs what the
buffers held, 8 s at most; a lost 12 V costs nothing** — the backup cell writes every buffer to
both cards before the head goes down (`../../mayak/HARDWARE.md`).

**The head cuts payloads, it does not read them.** NRC codes a frame column by column along the
layout's widths, and the rules compare bytes; no value is read anywhere. **A wrong layout costs
compression and never data** — NRC is lossless whatever widths it is given.

**The event log** is the series of address `00`: one record a line — `second 4 B · source 1 B ·
code 1 B · detail 2 B · value 4 B`, the head's second when the line was written, the source address
(0 the head, Hermes by its `ID`), and the code, detail and value of `../../mayak/FIRMWARE.md` §6 —
always kept, coded raw.

## The uplink

**Nothing is encrypted.** The archive is open data; what is authenticated is the command channel
and the archive session (`../PROTOCOL.md` §6) — an HMAC on every command and on `HELLO`, and an 8 B
truncated HMAC on every archive packet, so nobody writes into a station's files but the station.
The live packet and the values frame go bare. A key is needed only to write, never to read.

**Two packets, and the send rules decide the second.**

- **The archive packet** — a file's segments and configs as they lie on the card, each with its
  entry, and the file's header the first time that file goes up. The server writes them into the
  same file layout and holds the same files the cards hold; a full file needs nothing more. The
  `CURSOR` is per address: the file's sequence and the last entry the server holds, and the head
  sends forward from there. **This packet goes only where a link carries the volume** — Wi-Fi in a
  window.
- **The live packet** — what each address's send rule for that link selects from what was recorded
  since the last live packet, coded fresh by NRC as one segment per address, at the cadence the link
  is given. It is the server's live view, never its archive: the archive arrives later in archive
  packets or on a pulled card. **A station with no Wi-Fi** sends one live packet an hour over the
  modem — a mode-B Palatine's `HOUR` row, and its `DAY` row once a day (`../../palatine/WMO.md`),
  every other address `NONE` — and its cards hold the rest.
- **A window `(ts, ±window)`** asked by the server is an archive packet cut to the segments whose
  span touches it.

The modem's 32 B values frame is neither: it is the heartbeat and the marker, built by the head
(`../../mayak/FIRMWARE.md` §9); a live packet on the modem goes beside it, a frame of its own.

**A mode-B Palatine's tables are series of their own.** Each of its table blocks, addresses
`0xF8`–`0xFB` (`../../palatine/WMO.md`), is stored under the path `6n-F8/`… beside the Palatine's
ordinary series, under the same `CHANGE` rule, which keeps every row once.

## Reading it

**One reference library, C and Python, writes and reads MLF and NRC with this profile**; the head writes with the C,
and the server, the exporters and the viewer read with either. A query is an address and a
time: the path gives the files, the description's binary search the segments (MLF §6), the configs the
header in force, NRC the records.

**The archive stays raw.** Physical units, ModBus registers and record fields are derived on read
from the layouts and profiles, so a corrected layout re-derives everything from the same bytes.

**The exporters are readers** ([`EXPORTERS.md`](EXPORTERS.md)). miniSEED takes a channel's series
and the counts as they are, its response in StationXML; every other format takes the physical
value the layout's scale gives; the second and the frame index become decimal time only there, by
the one constant 10⁶/2²⁰. A gap a rule left is not a hole: the rule in force says why the record
is not there.

## On the server

**The archive of record is the files**, in the same layout the cards hold, sent up as the uplink
describes and pulled cards copied in. **Over them the server keeps a catalogue**, built from the
files' descriptions and headers: one row per segment — station, address, file, offset, first and
last second, codec — and one per config. A query is a lookup in the catalogue, then the segments
read. **The catalogue is never the archive**: it is rebuilt from the files at any time, and a
corrected layout re-derives everything from the same bytes. One station or a small operator keeps
it in SQLite, one file beside the archive; a network of many stations in PostgreSQL. What the
world takes is written from the archive by the exporters (`EXPORTERS.md`); its SQL exporter is
this catalogue with the values in it.

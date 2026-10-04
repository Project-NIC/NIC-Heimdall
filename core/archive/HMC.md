★ N.I.C. ★

# HMC — the Heimdall Matryoshka Container

> **Design-stage concept — nothing built.** The format is fixed here; its figures are reasoned,
> and HCC's ratios assumed (`HCC.md`, *What is assumed*).

HMC is the station's archive: what the Mayak writes to its two cards, what the uplink carries and
what the server stores. **Every unit has its own series of files** — a Quake its own, a Palatine its
own, a Tesla its own — because the units differ in everything an archive cares about: a Quake pours
128 frames a second, a Palatine changes a few values an hour, a Tesla sends zeros most of the time.
**What a unit's file holds is chosen by its recording rule; what leaves the station is chosen by its
send rule**, and the two are independent: a station in a field with no Wi-Fi sends a small packet of
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
| `NONE` | nothing |

`CHANGE` and `NONZERO` take an optional **`at least every s`**: a frame is kept anyway when `s`
seconds have passed since the last, so a quiet unit is seen to be alive.

**Whatever the rule, these are always kept**: a frame whose `kind` or `status` differs from the
last one kept — so every state edge, every ALARM, SUPPLY and FAULT edge, the first filler and the
first frame after it reach the card — and every frame of `kind` 5–8, the reports and answers.

**A rule is set per address and has a default per TYPE**, held in the head's configuration mirror,
changed from the handset or by the server over the authenticated channel:

| TYPE | default recording rule |
|---|---|
| Palatine | `CHANGE`, per block, at least every 3600 s |
| Tesla | `NONZERO`, at least every 60 s |
| every other | `ALL` |

**The send rule** uses the same vocabulary over what was recorded — it cannot send what the
recording rule dropped — and on a block payload may name the modules it takes. It is set per
address and per link (*The uplink*).

## The files

```
offset 0                                                             1 MB × 2^size
┌────────┬──────────────────────────────┬──────────┬──────────────────────────────┐
│ HEADER │ DATA → segments, configs     │  unused  │ ← DESCRIPTION, 16 B entries  │
└────────┴──────────────────────────────┴──────────┴──────────────────────────────┘
```

- **A file is 1 MB × 2^`size` — 1, 2, 4 … 128 MB — allocated whole when it opens**, so the FAT is
  touched only when a file opens. On exFAT, the base, the unwritten part reads as zeros. **The size
  follows the unit's rate**: it is a setting per address in the head's configuration mirror, set
  like the rules, and the file records its own in the header. The default is **64 MB for an address
  recorded under `ALL`** and **1 MB for every other** — a Quake fills 64 MB in hours, a Palatine
  under `CHANGE` takes months over one.
- **Data grows up from the header, the description grows down from the end.** The file is full
  when the next segment and its entry would meet; the next file opens. **A file closes on nothing
  else** — a Palatine's file runs for months, a Quake's for hours.
- **The path is the address and the day the file opened**, on the station's scale:
  `52/20261001/120000.hmc` — the address in hex, then the date, then the file's first second. The
  head's event log is the series of address `00`.
- **The two cards carry the same bytes.** The second card is the mirror.
- **Everything is little-endian.** Segments and configs carry CRC-32 (IEEE 802.3, the one zlib
  computes) — a segment reaches ~16 kB; a description entry carries the station's CRC-16-CCITT.

## The header

```
[0]   magic        4 B   "HMC" 0x00
[4]   version      1 B   2
[5]   address      1 B   TYPE«4 | NUMBER, 0 for the event log
[6]   sequence     2 B   the file's number in its address's series
[8]   length       4 B   the whole header, magic to CRC
[12]  first second 4 B   the second the file opened
[16]  station      3 B   the station identity
[19]  size         1 B   the file is 1 MB × 2^size, 0…7
[20]  sections     …     until length − 4
[end] CRC-32       4 B
```

**Every second in the file is the station's continuous scale** — GPS time in the Unix epoch,
leap-blind, as Kronos labels it (`../PROTOCOL.md` §4): the header, the records, the configs and the
entries alike. UTC is derived at export, from the `TIME` section in force.

**A section** is `id 1 B · reserved 3 B · length 4 B · body`; a reader skips an id it does not
know.

| id | section | |
|---|---|---|
| 1 | `STATION` | `name 32 B` UTF-8 · `latitude 4 B` · `longitude 4 B` (int32, 10⁻⁷ degree) · `elevation 2 B` (int16, metres, `0x8000` unknown) — the position the head writes to Kronos's `POSITION` |
| 2 | `UNIT` | `space 1 B` (1 NodBus) · `type 1 B` · `card 1 B` · `port 1 B` · `slot 1 B` · `rows 2 B` (frames a second, 128 or 256) · `positions 4 B` (an Argus NUMBER: the mini address at positions 0..3, 0 empty) |
| 3 | `LAYOUT` | the payload layout of the unit's TYPE — and on an Argus NUMBER one per mini TYPE seated |
| 4 | `PROFILE` | one per ModBus type a Palatine reads |
| 5 | `SETTINGS` | the unit's registers, and its sondes' |
| 6 | `RULES` | the recording rule, then `links 1 B` and per link `link 1 B · rule · modules 1 B · module addresses` — a rule is `mode 1 B` (0 `ALL` · 1 `CHANGE` · 2 `DECIMATE` · 3 `INTERVAL` · 4 `NONZERO` · 5 `NONE`) `· n 1 B · s 2 B` (seconds, 0 none); a link is numbered as the head's uplink table numbers it, modules 0 meaning all |
| 7 | `RESPONSE` | `SUB 1 B · length 2 B ·` the unit's write-once calibration block verbatim, for a sensor that needs a response beyond its factory sensitivity (`../INTEROP.md`) |
| 8 | `CODEC` | `codec 1 B · revision 1 B · length 2 B · table` — the table a codec needs to decode, for a codec that needs one (*Carrying its own description*, below); HCC needs none and writes none |
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
which HCC may code this one as the difference; the reference is always a lower channel. **The class says what the fields do not**: Quake, Gauss, Pascal and
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

## The data — segments and configs

**A segment is one write of one address's kept records**, coded by a codec:

```
[0]   sync          4 B   "HSEG"
[4]   length        4 B   sync to CRC
[8]   first second  4 B   the oldest record's second
[12]  last second   4 B   the newest
[16]  codec         1 B   0 raw · 1 HCC
[17]  reserved      3 B
[20]  body          …
[end] CRC-32        4 B
```

**A record** is one kept frame, or on a block payload one kept block, with its time:
`second 4 B · frame 1 B · kind 1 B · status 1 B · form 1 B` (0 frame, 1 block) `· length 1 B ·
bytes` — 32 B of payload for a frame, the block as it rode (`module · length · data`) for a block.
Codec 0 is the records back to back; **codec 1 is HCC** (`HCC.md`). **The codecs are a set**: the
head codes a segment with every codec it has and writes the shortest, and a codec added later takes
the next number — a reader skips a segment whose codec it does not know.

**The data stream is COBS-framed.** Every segment and every config is written COBS-encoded — no
zero byte inside it — and followed by one 0x00; the cost is at most one byte in 254. A reader
dropped anywhere in the data — a damaged entry, a file whose description is lost, a packet cut
short — skips to the next zero and reads on. The entries point at the framed bytes.

**A late frame is just a record.** A frame repaired after its second was written arrives as a
record with an older second; the reader places every record by its second and frame, and of two
records for one frame the later written is the one.

**A config** is written when a section of the header changes — a setting, a rule, a layout, a
seat on an Argus — and holds from the second it names:

```
"HCFG" 4 B · length 4 B · from second 4 B · reserved 4 B · sections as in the header · CRC-32 4 B
```

**A setting change does not close the file**: the unit's `CHANGED` state code marks the frame, the
head writes a config from that second, and a reader applies the last config at or before each
record's second over the header.

## The description — 16 B entries from the end

```
[0]  second   4 B   a segment: its last second · a config: its from second
[4]  offset   4 B   where it starts in the file
[8]  length   4 B
[12] type     1 B   1 segment · 2 config
[13] reserved 1 B
[14] CRC-16   2 B   over the 14 before
```

**Entries are fixed-stride and their seconds never go back**, so a second is a binary search over
the description of a live file or a full one alike. A free entry reads all zeros and fails its
CRC.

**Writing order: the data first, then its entry.** An entry never points at bytes not yet
written. At boot the head finds each series' last file, binary-searches its description for the
last good entry, and goes on writing behind that entry's data. Nothing is repaired, truncated or
rebuilt.

## Carrying its own description

**Nothing a reader needs is outside the file and its card.** The layouts, the profiles and the
settings already ride in the header; the codec and the format ride with them:

- **Every card carries the format's description in words**: `HMC.txt` at the card's root, UTF-8 —
  this document, `HCC.md`, and the description of every codec the head writes, written when the
  card is prepared and rewritten when a codec is added. The server keeps the same text beside its
  archive. Whoever finds a card in fifty years reads the text and writes a reader from it: the
  descriptions are exact to the bit, and no code is stored or needed.
- **A codec that needs a table carries it in the file**: a `CODEC` section in the header, and in a
  config from the second it changes — so a table updated by a firmware release never orphans a
  file written with the old one; the reader takes the table in force for each segment from the
  configs, as it takes a layout. **HCC needs no table**: its predictor coefficients and its Rice
  parameters ride inside each segment's own stream.

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

**The head cuts payloads, it does not read them.** HCC codes a frame column by column along the
layout's widths, and the rules compare bytes; no value is read anywhere. **A wrong layout costs
compression and never data** — HCC is lossless whatever widths it is given.

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
  since the last live packet, coded fresh by HCC as one segment per address, at the cadence the link
  is given. It is the server's live view, never its archive: the archive arrives later in archive
  packets or on a pulled card. **A station with no Wi-Fi** sends one live packet an hour over the
  modem — a mode-B Palatine's `HOUR` row, and its `DAY` row once a day (`../../palatine/WMO.md`),
  every other address `NONE` — and its cards hold the rest.
- **A window `(ts, ±window)`** asked by the server is an archive packet cut to the segments whose
  span touches it.

The modem's 16 B values frame is neither: it is the heartbeat and the marker, built by the head
(`../../mayak/FIRMWARE.md` §9); a live packet on the modem goes beside it, a frame of its own.

**A mode-B Palatine's tables are series of their own.** Each of its table blocks, addresses
`0xF8`–`0xFB` (`../../palatine/WMO.md`), is stored under the path `6n-F8/`… beside the Palatine's
ordinary series, under the same `CHANGE` rule, which keeps every row once.

## Reading it

**One reference library, C and Python, writes and reads HMC and HCC**; the head writes with the C,
and the server, the exporters and the viewer read with either. A query is an address and a
time: the path gives the files, the description's binary search the segments, the configs the
header in force, HCC the records.

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

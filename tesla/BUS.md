★ N.I.C. ★

# Tesla — the bus

> **Design-stage concept.** The figures are verified against the parts' sheets and the parts
> before a board is made.

Tesla is NodBus **type 7**, one, two or three NODs, one 40 B DATA frame a NOD a period at
128 frames/s. The frames, the opcodes and the node contract are the station's
(`../core/PROTOCOL.md`); what is Tesla's is what the 32 B payload means, how the strike time is
carried, the supplement NOD and the rule for what goes on the wire.

## The frames

```
DATA     unix.0 · frame · TYPE|NUM · slot · kind · status · payload 32 B · CRC-16      = 40 B
CONTROL  unix.0 · frame · TYPE|NUM · SUB · op · arg0 · arg1 · arg2 · rsvd · rsvd · CRC-16 = 12 B
```

- The address byte is `TYPE«4 | NUMBER`: 0x7N, the NUMBER the sweep assigns in `ASSIGN_ADDR`.
- `SUB` is 0 — Tesla is a spur unit.
- **The payload is eight 4 B records and nothing else** — `payload[i*4]` for i = 0..7, no
  housekeeping field, no reserved region and no layout to look up. Anything can read the frame
  without knowing which board sent it.

## The strike time — a backward offset

**The node detects at `T_event` but transmits in a later frame** — the classifier's latency
crosses one or more 7,8125 ms frames — so the record carries how long ago, not a stamp:

- a **16-bit backward offset in project ticks**, 2⁻²⁰ s ≈ 0,954 µs, the tick
  `../core/blocks/nodbus.md` names. The field spans 62,5 ms = **8 frames**, the most
  lateness it can express; the unit's own buffer of 32 frames holds it with room. **Two bytes and
  eight frames of reach are what set the tick**, and the field is one whole 16-bit word.
- the receiver reconstructs **`T_event = the transmitting frame's start − offset`** — the frame
  *start*, fixed by the frame's own index, not the transmit moment. The index survives the card's
  re-slotting onto the trunk, so the offset resolves anywhere downstream, and a subtraction that
  underflows borrows from the second. Every node shares the one master clock, so the offset is
  network time with no per-node drift and no absolute counter to ship.

**A tick is one sample** — the chain runs at 2²⁰ SPS — and the record's rounding, ±0,48 µs, is the
largest single term in the station's time budget, accepted as such: a stroke's onset reaches two
stations smeared by about a microsecond whatever the field does (`../core/blocks/gps-pps.md`,
*The budget*). The node computes in quarter-ticks and rounds once (`FIRMWARE.md` §5).

## The event record — 4 B, eight to a payload

**Each event is 4 bytes, so a 32 B payload carries exactly eight.** The node ships candidates;
the server does the direction-finding.

The 4 B are **polymorphic**: the low 2 bits are a `type` tag that reinterprets the other 30. Two
record shapes share the slot — an **impulsive** record and a **carrier** record — one size, two
meanings, no variable-length parser.

```
Event (4 B), two little-endian 16-bit words like every payload value in the station.
Word A = bytes 0-1, ONE WHOLE 16-BIT FIELD; word B = bytes 2-3
(14-bit quantity << 2 | type) — THE TYPE TAG IS ALWAYS THE LOW 2 BITS OF WORD B.
16 + 14 + 2. Nothing straddles the word boundary: a reader takes two native
16-bit words and masks once.

 type = 0 LIGHTNING (impulsive)
   A, the whole word: offset — 16 bits, ticks of 2⁻²⁰ s backward from the
      frame's start; 0,954 µs a step, ±0,48 µs; 62,5 ms = 8 frames of reach
   B, the top 14 bits: amplitude — SIGN (= polarity) + 13-bit LOGARITHMIC
      magnitude, LSB 2⁻⁶ dB → 128 dB of span at 0,016 dB a step, where the rod
      calibrates to ~1 dB. The receiver's 98 dB — the 9,1 pT floor to the
      690 nT clip — rides in it

 type = 1 interference / RFI (impulsive, same layout as 0)
 type = 3 UFO — unknown impulsive (same layout as 0)

 type = 2 CARRIER (a level record — Tesla emits exactly one, its own floor at frequency 0)
   A: carrier frequency, 16-bit — the step is THE BAND TOP / 2¹⁶ = 8 Hz here,
      so the field spans 0…2¹⁹ (a record is read in the context of the host
      that carried it; on Marconi the same field steps 256 Hz to 2²⁴)
   B: 14-bit logarithmic level, LSB 2⁻⁶ dB, absolute — the same dB domain and
      the same step as the impulsive amplitude, so one reader serves both shapes

 An empty slot reads zero: the log scale reserves code 0 below its floor, and
 an impulsive record with zero magnitude is no event.

 FREQUENCY 0 IN A CARRIER RECORD IS THE FLOOR: the station's running wideband
 noise floor — the detector's rolling self-calibrated baseline, once a second.
 It is the reference every amplitude is judged against: the server computes
 true SNR, normalises detection efficiency across stations when it fuses them,
 and has the site's QRN climate in one number.
```

**Why a logarithmic amplitude.** 13 bits linear over the 690 nT clip is an LSB of 84,6 pT
against a 9,1 pT floor — the bottom 19 dB of the receiver would quantise to zero. In dB the whole
range rides at constant relative precision.

**The amplitude is signed and polarity is the server's.** A loop's sign is entangled with bearing
— a +CG to the north and a −CG to the south read alike — so the node ships the raw signed value
and the server resolves polarity from the TOA-derived azimuth; cross-station consistency, one
strike one polarity everywhere, confirms it and calibrates each station's wiring sign. Strip the
sign at the node and polarity is gone for good.

**One amplitude on the wire, three on the node.** The converter samples the three rods
simultaneously and the 3×3 crosstalk matrix is inverted across them; the record carries the one
amplitude four bytes hold. The other two become the supplement's azimuth.

**No event count, no overflow flag, no supply byte.** An empty slot is all zeros and a real event
never is, so the count is the master's arithmetic; **eight of eight populated is the overflow
flag**, and fewer than eight proves nothing was dropped. The supply is the header's SUPPLY flag and
the `REPORT` frame (`../core/PROTOCOL.md` §5).

**No stroke grouping on the node.** The node streams raw timestamped events; the server groups
same-flash strokes and runs the TOA. Sending more events beats collapsing them.

## The supplement record — four whole bytes, in a NOD of its own

**The 4 B event record does not change.** What an event knows beyond it rides a second record of
four whole bytes, **in a NOD of its own, bound to the event by position**.

**A supplement NOD is its base's NUMBER + 7.** Bases 1..7, supplements 8..14, 15 free:

| NUMBER | address | what it carries |
|---|---|---|
| **Tesla 1** | `0x71` | the event records |
| **Tesla 2** | `0x72` | the source states and the floor record |
| **Tesla 8** | `0x78` | **the supplement to Tesla 1**, one row per event |
| Tesla 9 | `0x79` | held by the rule for Tesla 2; not populated |

`NUMBER > 7` says a record is a supplement and `NUMBER − 7` says whose — one comparison and one
subtraction, no table. The supplement has its own NUMBER, so it takes its own slot and never
competes with the source states for a payload.

**The binding is position.** Slot `i` of Tesla 8's payload supplements slot `i` of Tesla 1's
payload **of the same frame** — the two frames carry the same `unix.0 · frame`, so they pair by
construction; no sequence number, no back-reference. It is the key the Argus tiles its mini
payloads by (`../core/blocks/nodbus.md`).

```
Supplement row (4 B) — four independent whole bytes, nothing packed, no type tag

 byte 0   AZIMUTH          uint8   0..255 over 180°, 0,70° a step — RAW, in the
                                   frustum's own mechanical frame. The node has no
                                   north and applies no correction; the server adds
                                   the learned per-station table and resolves the
                                   180° from the TOA azimuth
 byte 1   RISE TIME        uint8   logarithmic, ~0,5 µs to ~500 µs — a sferic's edge
                                   is under a microsecond, a tracking insulator's
                                   tens
 byte 2   Δt mid − lo      int8    whole ticks, ±121 µs, signed
 byte 3   Δt hi  − lo      int8    whole ticks, ±121 µs, signed
```

A server reads a row as `>BBbb`.

- **Two deltas, not three edge times**: three times are described by two differences, and the
  absolute time is Tesla 1's offset.
- **What the deltas measure.** The Earth–ionosphere waveguide is dispersive near cutoff, so a
  distant sferic arrives with its bands **tens of µs apart** — the low strip last, both deltas
  negative — and a local source with none. On a power line the same two bytes read the **modal
  split**: the aerial mode runs at 0,98 c and the ground mode at 0,8 c, so the separation grows
  **0,52 to 1,04 µs a kilometre** by the line's height. One tick is about a kilometre and the
  field reaches past a hundred.
- **They range, they do not classify.** A sferic at 1000 km and an arc at 20 km both give tens of
  µs with the same sign; the mains lock says which it is (`DETECTION.md`), the deltas say how far.
- **The strips' frequencies are not in the record.** As a feature the delay is monotonic in
  distance whatever the strips are; as a number the conversion wants the frequencies, and the
  archive table's header carries them — a placement change rolls the table (`status` 5 CHANGED,
  `../core/PROTOCOL.md` §5). The inversion is the server's, because it needs the waveguide's
  cutoff and only the server has the ionosphere (Marconi's `foF2` and `h'F`, Pip's SID).
- **The azimuth is the one new computation** — a Clarke transform and one `atan2` from the three
  rods' µ(T)-corrected peaks, ~55 cycles an event, 0,02 % of the core at 1024 events/s. The rise
  time and the three strip edges are already computed for the classifier.
- **The azimuth is qualified.** It is raw and uncalibrated in the frustum's frame, and the server
  owns the per-station table and the 180°. It holds for a local groundwave source, which has no
  polarisation or ionospheric path error; for a sferic 500 km away it does not, and position there
  is TOA across stations.
- **Losing one of the two frames is harmless.** The event stands alone without its supplement; an
  orphaned supplement is dropped; the event never travels in the supplement NOD.
- **The price is slots**: the three-NOD build takes three of the card's eight unit slots.

## What goes on the wire — one rule, about the source

**Eight slots at 128 Hz is 1024 events a second, and that is the ceiling.** It is the order of the
class: a deployed VLF/LF station processes about 1000 signals a second, sustained interference
being what pins it there. A severe cell runs 60–500 flashes a minute, each with several return
strokes and in-cloud pulses, so a multi-cell outbreak within 100 km puts hundreds to over a
thousand impulses a second on the antenna.

Weather saturates in bursts and passes; man-made interference drones — one arcing gap fires twice
a mains cycle, 100 a second, 300 when all three phase pairs go, until somebody repairs it:

> **Independent events are shipped one by one. A persistent source is shipped as a state —
> one record every 32 frames (four a second) for as long as it lasts.**

- **Lightning needs no special case**: every stroke is a different place, instant and amplitude,
  and nothing about it looks persistent. **Arcing is one condition sampled 300 times**: a
  five-minute episode costs 1 200 records instead of 90 000. The quarter-second cadence shows
  whether the fault moves.
- **The state record carries a real pulse time** — the offset + amplitude layout — so the server
  collects µs correlation points at 4 Hz for as long as the arc lasts. **A short arc**, a snapped
  conductor burning out inside a second, never reaches the collapse: the lock test needs ~0,3 s of
  intervals, so its pulses ship raw. Along a known line route the location is one-dimensional, and
  kilometre-class at ±1 µs. The types are advisory — the server may reclassify; the timestamps are
  the data.
- **This is not stroke grouping.** Return strokes are separate discharges the server needs one by
  one; an arc is one emitter standing still, and its repetition carries nothing. The test is
  whether the events are independent.
- **Per source, not per condition.** More than one line can fault at once. Phase within the mains
  cycle separates lines on different phases by 120°, and amplitude is in the record; sources too
  alike to separate merge. **Eight tracked sources at most** — 32 records a second, 3 % of the
  frame; a source past the cap is counted and not slotted.
- **UFO takes the same treatment for the repeating kind only.** A drone gets four records a second
  like any persistent source; a one-off unknown impulse keeps its own record — that lone oddity is
  why the class exists.
- **Priority when the slots run short: lightning · arcing · UFO.** With the collapsing it should
  never bind; it makes the arithmetic safe, and a rule replaces a per-site setting.

## One NOD, two, or three

| NODs | NUMBERs | NOD 1 | NOD 2 | NOD 8 |
|---|---|---|---|---|
| one | one, from the card | everything, under the priority; the floor behind it | — | — |
| two | two, from the card | lightning records only | the source states and the floor record | — |
| three | the two, and NOD 1's + 7 | lightning records only | the source states and the floor record | the supplement, slot for slot with NOD 1 |

With two NODs the board is a multi-slot unit like Sputnik (`../core/PROTOCOL.md` §2): the
quarter-second states and the strokes stop competing for one payload and the priority has nothing
to resolve. **The head condenses before it writes** — a value read every second may be written
every ten minutes, by the convention of the country the station stands in.

## Sizes

| frame | size | rate | load a NOD |
|---|--:|---|--:|
| DATA | 40 B | 128 frames/s | 41 kb/s |
| CONTROL | 12 B | on demand | negligible |

The payload length is a TDMA invariant: a variable length would cost the fixed slot schedule and
buy nothing.

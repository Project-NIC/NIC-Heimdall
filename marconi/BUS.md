★ N.I.C. ★

# Marconi — the bus

> **Design-stage concept.** The figures are verified against the parts' sheets and the parts
> before a board is made.

**NodBus type 4, one slot, 40 B, behind a Bifrost.** Marconi #1 is `0x41`. Everything runs on the
board and what ships is **a log level per carrier** — the record Pip's SID channel ships — plus the
scaled ionogram parameters: 134 MB/s inside the box, a few bytes a second on the wire. A handful of
carriers at 4 B each is one slot, so Marconi presents as one unit.

## The record

**Tesla's record shape, read in Marconi's context** — up to eight polymorphic 4 B records in the
32 B payload. Word A = bytes 0-1, 16 bits of context; word B = bytes 2-3, a 14-bit quantity << 2 |
type — **the type tag is always the low 2 bits of word B**, on every record of every board. A record
is read in the context of the host that carried it, so the frequency step is **the band top /
2¹⁶**: 256 Hz on this type-4 host, and word A spans 0–2²⁴; on Tesla and Pip the same field steps
8 Hz to 2¹⁹.

```
Record (4 B), two little-endian 16-bit words; tag = low 2 bits of word B:

 type = 0 CARRIER (a monitored carrier's level)
   A: carrier frequency, 16-bit, 256 Hz steps → 0…2²⁴ Hz
   B: 14-bit logarithmic level, LSB 2⁻⁶ dB, absolute, self-contained — the same
      dB domain as Tesla's carrier and impulsive records

 type = 1 IONOGRAM (one scaled parameter)
   A: value, uint16 — the unit is the parameter's
   B: parameter id (14-bit) + type:
        1 = foF2      value in 1 kHz steps
        2 = h'F       value in km
        3 = MUF(3000) value in 1 kHz steps
      the rest of the id space is spare

 types 2, 3 — spare.  An empty slot reads zero (the log scale reserves code 0
 below its floor).  FREQUENCY 0 IN A CARRIER RECORD IS RESERVED for the
 station's running wideband noise floor, shipped once a second — the reference
 every level is judged against, and the site's HF noise climate in one number.
```

**One decoder serves the whole ionosphere ladder**: the carrier record is Pip's SID record in shape
and scale — the type tag (0 here, 2 on Pip) and the frequency step are what the host type
implies — so the server reads Pip's D-region and Marconi's F-region carriers with one routine,
and every amplitude anywhere is on the one 2⁻⁶ dB scale. No count field and no variable-length parser: an empty slot is zero.

**The ionogram ships as parameters, not as an image** — `foF2`, `h'F`, `MUF(3000)` scaled on the
node. The node ships what was measured, not what it looked like, as Tesla and Quark do.

## Sizes

| frame | size | rate | load |
|---|--:|---|--:|
| DATA | 40 B | up to 8 records a frame; a carrier record per carrier per revisit, the floor once a second | a few bytes a second |
| CONTROL | 12 B | on demand | negligible |

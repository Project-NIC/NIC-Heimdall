★ N.I.C. ★

# HCC — the Heimdall Compression Codec

> **Design-stage concept — nothing built.** The coding is fixed here; **its ratios are assumed, not
> measured** (*What is assumed*, below).

HCC is codec 1 of an HMC segment (`HMC.md`): it codes **one address's kept records** into one bit
stream, and back, losslessly. **A stream depends on nothing outside itself and the address's
layout**: no state carries from one segment to the next, no table is trained, no model adapts.

## In plain words — no table, a rule

**HCC stores no table and builds none.** A Huffman or an ANS code needs a table of how often each
symbol comes, and the reader needs that table to decode. HCC does without one: **a Rice code is a
rule, not a table** — one small number, `k`, says how to write every value of a run of 32, and that
number travels in the stream in front of the 32. The predictor's coefficients travel the same way,
in the series they serve. So a segment carries everything needed to read it, and nothing is kept
anywhere else.

**Four steps, one series at a time:**

1. **Predict.** Guess each value from the ones before it — the same as the last (ORDER 1), on the
   line through the last two (ORDER 2), or from the last eight (LPC); or say every value is equal
   (CONST), or give up and write them as they are (RAW).
2. **Keep the miss.** Write only the difference between the value and the guess, the residual. A
   good guess leaves small numbers.
3. **Fold the sign.** 0, −1, +1, −2, +2 … become 0, 1, 2, 3, 4 …, so a small miss either way is a
   small number.
4. **Rice-code it.** Pick `k` for the next 32 residuals; each is written as `u >> k` ones and a zero,
   then its `k` low bits. Small numbers take few bits; one large one costs only itself.

**An example** — a temperature series in hundredths of a degree, 16-bit: `2150 2151 2151 2153
2152`. ORDER 1 keeps the misses `+1 0 +2 −1`, folded `2 0 4 1`. With `k` 1 they are written
`10·0  0·0  110·0  0·1` — 3 + 2 + 4 + 2 = **11 bits for four values that raw take 64**. `k` 0 writes
the same 11, and the coder takes the smaller `k` on a tie. With the bookkeeping — the mode, the
first value, the `k` — the series is 35 bits against 83 for RAW; on a segment of thousands of
values the bookkeeping vanishes and what is left is the sensor's own noise.

**The coder tries every mode and keeps the shortest**, per series; HMC above it keeps the shorter
of HCC and raw, per segment. **Room to grow is reserved in both**: modes 5–7 are free in the series,
and codec numbers after 1 are free in HMC. A codec added there brings its description to the card
(`HMC.md`, *Carrying its own description*), and its table, if it needs one, in a `CODEC` section.

## Why it has this shape

- **Columns, not rows.** A record is a row of fields; what is alike is one field down the segment
  — an axis, a field component, a counter — so HCC turns the rows into series, one per field, and
  codes each on its own. That is the shape Steim and FLAC code, and the shape miniSEED wants.
- **Prediction, Rice codes, partitions.** A sampled quantity changes little from one record to
  the next: the difference to the last value, to the straight line through the last two, or to a
  linear prediction from the last eight is small, and a Rice code writes a small number in a few
  bits. The parameter is chosen per 32 values, so an event costs bits where it happens and the
  quiet rest stays cheap.
- **Better than Steim, and where from.** Steim packs first differences into 32-bit words whose
  fields share one width; HCC writes bit by bit, predicts up to order 8 where the signal is
  coloured — the microseism peak — and **codes one channel against another** where two sensors
  see the same motion, which a one-channel-per-record format cannot do. What is left is the
  sensor's own noise, and no lossless code goes below it.
- **Field widths from the layout.** A difference is taken on the field as the unit wrote it — 2 B,
  3 B, 4 B — never byte by byte across a multi-byte value.
- **The time is a series like any other.** Under `ALL` the records are consecutive frames and the
  time costs about a bit a record; under a sparse rule it costs what the gaps cost.
- **Every segment is a keyframe.** A segment is read alone, a window is cut at segment boundaries,
  and a damaged segment costs itself only.
- **Integers only in the decoder, and nothing to store.** Shifts, adds, multiplies and compares;
  no table in ROM. The coder alone computes in floating point, to find the predictor, and it does
  so in a fixed order so its output is fixed too.

## The input

- **`n` records** in the order they were kept, each `second 4 B · frame 1 B · kind 1 B · status
  1 B · form 1 B · bytes` — a **frame** record (form 0) with its 32 B payload, or a **block**
  record (form 1) with the ModBus block as it rode: `module 1 B · length 1 B · data`.
- **The layout** of the address's TYPE (`HMC.md`, `LAYOUT`): for each field its offset, width in
  bytes, byte order and channel. **An Argus NUMBER's layout is composed** — the mini layout of the
  sonde at each position, offsets moved by 8 × position, channels taken position by position after
  the last one used. **Every byte no field covers is a one-byte channel of its own**, after the
  declared channels, in offset order, so every layout covers the 32 B and a TYPE with no layout is
  32 one-byte channels.

The coder reads a field as an unsigned integer of 8 × width bits in its byte order. **Sign does
not matter to it**: the arithmetic is modulo 2^w and the same bits come back either way.

## The stream

Bits are written most significant first; the stream ends padded with zero bits to a whole byte.

```
1  COUNT        γ(n)
2  HEADERS      five series over all n records:
                second 32 bits · frame 8 · kind 8 · status 8 · form 8
3  DATA         the frame records of kind 0 — one series per layout channel, ascending
4  OTHER        the frame records of any other kind — 32 one-byte series
5  BLOCKS       γ(g + 1), the groups — one per module and data length, in order of first
                appearance — each as module 8 · length 8; then a group-index series over the
                block records, 8 bits; then per group its data as one-byte series
```

**A channel's series runs in time**: record by record, and inside a record field by field in
offset order, so two samples of one quantity in one payload follow each other. **A group's
series** are its records' data bytes, byte position by byte position — a ModBus register's high
byte and low byte are two series, which is where a slow register's constant high byte goes to
nothing.

## A series

`n` values of `w` bits. Nothing is written when `n` is 0. **Where the channel's layout names a
reference**, one bit first: 1 — the series is coded as `xᵢ − yᵢ` modulo 2^w, `y` the reference
channel's series, already decoded; 0 — as itself. Then a 3-bit mode:

| mode | | written |
|---|---|---|
| 0 | **CONST** | one value, `w` bits — every value is equal |
| 1 | **ORDER 1** | `x₀` in `w` bits, then the residuals `rᵢ = xᵢ − xᵢ₋₁` for `i` = 1 … `n − 1` |
| 2 | **ORDER 2** | `x₀`, `x₁` in `w` bits, then `rᵢ = xᵢ − 2xᵢ₋₁ + xᵢ₋₂` for `i` = 2 … `n − 1` |
| 3 | **LPC** | `p − 1` in 3 bits, `s` in 4 bits, `p` coefficients `a₁ … aₚ` in 15 bits signed, `x₀ … xₚ₋₁` in `w` bits, then `rᵢ = xᵢ − ((a₁xᵢ₋₁ + … + aₚxᵢ₋ₚ) >> s)` for `i` = `p` … `n − 1` |
| 4 | **RAW** | `n` values, `w` bits each |
| 5–7 | — | not used; a stream carrying one is damaged |

**LPC reads the values with their sign** — a field the layout marks signed as a signed `w`-bit
number, any other as unsigned — and forms the sum in 64-bit signed integers, the shift `s`
arithmetic. Every other mode, and every residual, works modulo 2^w and ignores the sign.

**A residual is taken modulo 2^w and read as a signed `w`-bit number**, so it always fits in `w`
bits whatever the values do, and the decoder adds it back modulo 2^w. It is then folded to an
unsigned number, `u = (r << 1) ⊕ (r >> (w − 1))` with an arithmetic shift — 0, −1, 1, −2, 2 …
become 0, 1, 2, 3, 4 …

**The residuals go in partitions of 32**, in order, the last one shorter. Each partition opens
with a **5-bit parameter `k`**:

- **`k` 0 … 30 — a Rice code**: each `u` as `q = u >> k` one-bits and a zero, then the `k` low bits
  of `u`.
- **`k` 31 — the escape**: each `u` in `w` bits.

**γ — the Elias gamma code** of an integer `m ≥ 1`: `L = ⌊log₂ m⌋` zero bits, then `m` in
`L + 1` bits.

## The coder's choices

**The stream is a function of its input.** The decoder accepts any valid choice, but the coder
makes exactly these, so the C and the Python write the same bytes and a test compares them:

- **Per partition**: the `k` from 0 to min(30, `w`) that writes the fewest bits, the smaller `k`
  on a tie; the escape where it writes no more than that.
- **The predictor**: the autocorrelation of the series for lags 0 … 8 summed in 64-bit integers;
  the Levinson–Durbin recursion on it in IEEE-754 double, in the order the recursion is written,
  without fused multiply-add, giving the coefficients for every order 1 … 8 it reaches; each set
  quantised with the largest `s` ≤ 15 that keeps every |`aⱼ`| · 2^s below 2^14, rounded half away
  from zero. Every order is tried.
- **The reference**: tried where the layout names one, the series coded both ways.
- **Per series**: the reference choice and the mode — and under LPC the order — that write the
  fewest bits; on a tie no reference, the lower mode, the lower order. CONST only when every value
  is equal.

**The worst case is bounded.** A stream is never longer than codec 0's records by more than its
own bookkeeping — four bits a series and five a partition.

## Decoding

The decoder holds the address's layout from the header and the config in force, and reads the
parts in order: the count, the five header series — which say which table each record falls in
and, for a frame, its kind — then the tables, each series written back into its fields, and the
records rebuilt in their order. **A stream that does not end within its last byte, or ends with
bits left over in more than that byte, is damaged**, and the segment is reported as such — HMC's
CRC has already said the segment is whole, so this is a coder fault, never a medium fault.

**So is a stream that says what no coder writes**, and the decoder checks it before it trusts it:
a count above **65 536** — HMC closes a segment at 16 kB, under 1 500 records (`HMC.md`, *Writing
it — the head*), and a count is the one number a few damaged bits can turn into gigabytes; a form
other than 0 or 1; mode 2 on fewer than two values or LPC of an order not below `n`; a mode 5–7;
a block's group index past the groups written. A coder holds the same ceiling and never writes a
segment over it.

## What is assumed

**HCC is assumed to code a series in no more than Steim-2 does, and a coloured or paired series in
less.** The reasoning, not a measurement: Steim-2 is first differences packed into 32-bit words
whose fields share a width; HCC's ORDER 1 is the same differences written bit by bit, with a Rice
parameter chosen per 32 values, and it adds ORDER 2, LPC to order 8 and the reference channel, each
taken only where it writes fewer bits. Its worst case is bounded by codec 0 (*The coder's
choices*), and HMC writes the shorter of the two per segment, so a series HCC codes badly costs
the raw size and never more.

**The ratios are settled by whoever writes the full codec**, against the full environment and
millions of recorded segments — the way NIC-Arduino's DMD was settled; a handful of synthetic
cases decides nothing. The Python reference and `ref/measure.py` are where that starts.

## What it is tested against

On a PC, against recorded traffic and against synthetic segments: a Quake under `ALL`, a Palatine
under `CHANGE` with one module changing and the rest held, a Tesla under `NONZERO` with a burst, a
filler run with its edges, a 256-row unit with two samples to a payload, a Quake event coded
against its reference channel, an LPC series that wraps its field's range, an Argus NUMBER with an
empty position, a TYPE with no layout, a late repaired frame. Every case round-trips byte for
byte, and the C and the Python streams are identical.

★ N.I.C. ★

# NRC — the archive's records as series, Heimdall's profile

> **Design-stage concept — nothing built.** The coding is fixed here; its ratios are measured on
> open recordings only.

**NRC, the NIC Rice Codec**, is codec 1 of an MLF segment ([`MLF.md`](MLF.md)); its core — the
series, the modes, the Rice partitions, the coder's choices — is NIC-SDK's
[`NRC.md`](https://github.com/Project-NIC/NIC-SDK/blob/main/NRC.md). **This page is Heimdall's profile** (NRC §7 there): how one address's kept
records become series, the parts of the stream, its ceiling and how the records come back.

## The input

- **`n` records** in the order they were kept, each `second 4 B · frame 1 B · kind 1 B · status
  1 B · form 1 B · bytes` — a **frame** record (form 0) with its 32 B payload, or a **block**
  record (form 1) with the ModBus block as it rode: `module 1 B · length 1 B · data`.
- **The layout** of the address's TYPE (`MLF.md`, `LAYOUT`): for each field its offset, width in
  bytes, byte order and channel. **An Argus NUMBER's layout is composed** — the mini layout of the
  sonde at each position, offsets moved by 8 × position, channels taken position by position after
  the last one used. **Every byte no field covers is a one-byte channel of its own**, after the
  declared channels, in offset order, so every layout covers the 32 B and a TYPE with no layout is
  32 one-byte channels.

The coder reads a field as an unsigned integer of 8 × width bits in its byte order. **Sign does
not matter to it**: the arithmetic is modulo 2^w and the same bits come back either way.

## The layout in force

**NRC codes a segment with the layout in force where the segment lies**: the header's sections as
changed by every config written before it in the file — so the head writes its buffer out as a
segment before it writes a config that changes the layout. The layout is read from the sections:
an Argus NUMBER — a `UNIT` with a position seated — takes at each position the mini `LAYOUT` (space
2) of the seated TYPE, `TYPE` being the address's high four bits; any other unit the `LAYOUT` of its
`UNIT`'s space and type; a TYPE with no `LAYOUT` — or a file with no `UNIT` — one channel a byte.
The first `UNIT` and the first `LAYOUT` of a space and type count. A field gives NRC its offset,
width, the two flags and its channel and `ref`; the scale and the names are the reader's.

## The stream

The bits, the padding and γ as NRC's core has them (§2 there); the parts in this order:

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

## Decoding

The decoder holds the address's layout from the header and the config in force, and reads the
parts in order: the count, the five header series — which say which table each record falls in
and, for a frame, its kind — then the tables, each series written back into its fields, and the
records rebuilt in their order. **A stream that does not end within its last byte, or ends with
bits left over in more than that byte, is damaged**, and the segment is reported as such — MLF's
CRC has already said the segment is whole, so this is a coder fault, never a medium fault.

**The checks beyond the core's** (NRC §5 there): **a count above 65 536** — MLF closes a segment
at 16 kB, under 1 500 records (*Writing it — the head*, [`MLF.md`](MLF.md)); a form other than 0 or
1; a block's group index past the groups written. A coder holds the same ceiling and never writes a
segment over it.

## Measured

The table of NRC's core (§8 there) is measured by NIC-SDK's `tools/nrcmeasure.py` on the series its
README says how to fetch — GE.MORC's seismometer and Budkov's magnetometer.

## What it is tested against

On a PC, against recorded traffic and against synthetic segments: a Quake under `ALL`, a Palatine
under `CHANGE` with one module changing and the rest held, a Tesla under `NONZERO` with a burst, a
filler run with its edges, a 256-row unit with two samples to a payload, a Quake event coded
against its reference channel, an LPC series that wraps its field's range, an Argus NUMBER with an
empty position, a TYPE with no layout, a late repaired frame. Every case round-trips byte for
byte, and the C and the Python streams are identical.

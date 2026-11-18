"""NRC — Heimdall's profile, the Python reference.

One address's kept records cut into series and coded by NRC's core (NIC-SDK, `tools/nrc.py`), and
back, exactly as `core/archive/NRC.md` fixes it: the count, the five header series, the data by the
layout's channels, the other kinds, the blocks. The C (`c/hnrc.c`) writes the same bytes.

A record is a tuple (second, frame, kind, status, form, data):
    form 0 — a frame, data = 32 payload bytes
    form 1 — a block, data = module · length · bytes as the ModBus block rode

A layout is a list of fields (offset, width, signed, big_endian, channel, ref) with ref the
reference channel + 1 or 0; `compose_layout` fills the bytes no field covers with one-byte
channels, so every layout covers the 32 B.
"""

from __future__ import annotations

from dataclasses import dataclass
from typing import Iterable, List, Optional, Sequence, Tuple

import sdk  # noqa: F401
from nrc import BitReader, BitWriter, finish, read_series, write_series  # noqa: F401
from nrc import _mode_bits  # noqa: F401  (the measurement's)

Record = Tuple[int, int, int, int, int, bytes]

MAX_RECORDS = 1 << 16   # NRC.md, *Decoding*: a count above it is damaged; MLF's 16 kB holds < 1 500


# ---------------------------------------------------------------- the layout

@dataclass(frozen=True)
class Field:
    offset: int
    width: int          # 1..4 bytes
    signed: bool
    big_endian: bool
    channel: int
    ref: int            # reference channel + 1, 0 none


def compose_layout(fields: Sequence[Field], payload: int = 32) -> List[Field]:
    """The declared fields plus a one-byte channel for every byte none covers, after the declared
    channels, in offset order (NRC.md, *The input*)."""
    covered = bytearray(payload)
    for f in fields:
        for b in range(f.offset, f.offset + f.width):
            covered[b] = 1
    next_ch = max((f.channel for f in fields), default=-1) + 1
    out = list(fields)
    for b in range(payload):
        if not covered[b]:
            out.append(Field(b, 1, False, False, next_ch, 0))
            next_ch += 1
    return out


def argus_layout(positions: Sequence[Optional[Sequence[Field]]]) -> List[Field]:
    """An Argus NUMBER's layout composed from the mini layout at each 8 B position."""
    out: List[Field] = []
    next_ch = 0
    for pos, mini in enumerate(positions):
        if not mini:
            continue
        base = next_ch
        for f in mini:
            out.append(Field(f.offset + 8 * pos, f.width, f.signed, f.big_endian,
                             base + f.channel, (base + f.ref) if f.ref else 0))
        next_ch = max(f.channel for f in out) + 1
    return compose_layout(out)


def _channels(layout: Sequence[Field]) -> List[Tuple[int, List[Field], bool, int]]:
    """(channel, its fields in offset order, signed, ref) ascending by channel."""
    chans = {}
    for f in layout:
        chans.setdefault(f.channel, []).append(f)
    out = []
    for ch in sorted(chans):
        fs = sorted(chans[ch], key=lambda f: f.offset)
        out.append((ch, fs, fs[0].signed, fs[0].ref))
    return out


def _read_field(payload: bytes, f: Field) -> int:
    b = payload[f.offset:f.offset + f.width]
    return int.from_bytes(b, "big" if f.big_endian else "little", signed=False)


def _write_field(payload: bytearray, f: Field, v: int) -> None:
    payload[f.offset:f.offset + f.width] = (v & ((1 << (8 * f.width)) - 1)).to_bytes(
        f.width, "big" if f.big_endian else "little")


# ---------------------------------------------------------------- the stream

def encode(records: Sequence[Record], layout: Sequence[Field]) -> bytes:
    """The records of one address, in the order kept, to one NRC stream."""
    bw = BitWriter()
    n = len(records)
    if n == 0:
        raise ValueError("a segment holds at least one record")
    if n > MAX_RECORDS:
        raise ValueError("a segment holds at most %d records" % MAX_RECORDS)
    bw.gamma(n)
    # 2 HEADERS
    write_series(bw, [r[0] for r in records], 32)
    write_series(bw, [r[1] for r in records], 8)
    write_series(bw, [r[2] for r in records], 8)
    write_series(bw, [r[3] for r in records], 8)
    write_series(bw, [r[4] for r in records], 8)
    # 3 DATA — frame records of kind 0, one series per channel
    data = [r[5] for r in records if r[4] == 0 and r[2] == 0]
    done = {}
    for ch, fs, signed, ref in _channels(layout):
        xs = [_read_field(pl, f) for pl in data for f in fs]
        refv = done.get(ref - 1) if ref else None
        if refv is not None and len(refv) != len(xs):
            refv = None
        write_series(bw, xs, 8 * fs[0].width, signed, refv)
        done[ch] = xs
    # 4 OTHER — frame records of any other kind, 32 one-byte series
    other = [r[5] for r in records if r[4] == 0 and r[2] != 0]
    for b in range(32):
        write_series(bw, [pl[b] for pl in other], 8)
    # 5 BLOCKS
    blocks = [r[5] for r in records if r[4] == 1]
    groups: List[Tuple[int, int]] = []
    for bl in blocks:
        g = (bl[0], bl[1])
        if g not in groups:
            groups.append(g)
    if len(groups) > 256:
        raise ValueError("a segment holds at most 256 block groups — the index is one byte")
    bw.gamma(len(groups) + 1)
    for module, length in groups:
        bw.write(module, 8)
        bw.write(length, 8)
    if blocks:
        write_series(bw, [groups.index((bl[0], bl[1])) for bl in blocks], 8)
        for gi, (module, length) in enumerate(groups):
            mine = [bl[2:2 + length] for bl in blocks if (bl[0], bl[1]) == (module, length)]
            for b in range(length):
                write_series(bw, [d[b] for d in mine], 8)
    return bw.bytes()


def decode(stream: bytes, layout: Sequence[Field]) -> List[Record]:
    br = BitReader(stream)
    n = br.gamma()
    if n > MAX_RECORDS:
        raise ValueError("NRC stream damaged: a count of %d records" % n)
    seconds = read_series(br, n, 32)
    frames = read_series(br, n, 8)
    kinds = read_series(br, n, 8)
    statuses = read_series(br, n, 8)
    forms = read_series(br, n, 8)
    if any(f > 1 for f in forms):
        raise ValueError("NRC stream damaged: a form other than frame or block")
    nd = sum(1 for i in range(n) if forms[i] == 0 and kinds[i] == 0)
    no = sum(1 for i in range(n) if forms[i] == 0 and kinds[i] != 0)
    nb = sum(1 for i in range(n) if forms[i] == 1)
    data = [bytearray(32) for _ in range(nd)]
    done = {}
    for ch, fs, signed, ref in _channels(layout):
        refv = done.get(ref - 1) if ref else None
        if refv is not None and len(refv) != nd * len(fs):
            refv = None
        xs = read_series(br, nd * len(fs), 8 * fs[0].width, signed, refv)
        done[ch] = xs
        it = iter(xs)
        for pl in data:
            for f in fs:
                _write_field(pl, f, next(it))
    other = [bytearray(32) for _ in range(no)]
    for b in range(32):
        for pl, v in zip(other, read_series(br, no, 8)):
            pl[b] = v
    g = br.gamma() - 1
    groups = [(br.read(8), br.read(8)) for _ in range(g)]
    blocks: List[bytes] = []
    if nb:
        gidx = read_series(br, nb, 8)
        if any(x >= g for x in gidx):
            raise ValueError("NRC stream damaged: a block's group index past the %d groups" % g)
        per_group = {}
        for gi, (module, length) in enumerate(groups):
            cnt = sum(1 for x in gidx if x == gi)
            cols = [read_series(br, cnt, 8) for _ in range(length)]
            per_group[gi] = [bytes(module.to_bytes(1, "little") + length.to_bytes(1, "little")
                                   + bytes(cols[b][i] for b in range(length))) for i in range(cnt)]
        counters = {gi: 0 for gi in range(g)}
        for x in gidx:
            blocks.append(per_group[x][counters[x]])
            counters[x] += 1
    finish(br)
    di = oi = bi = 0
    out: List[Record] = []
    for i in range(n):
        if forms[i] == 1:
            payload = blocks[bi]
            bi += 1
        elif kinds[i] == 0:
            payload = bytes(data[di])
            di += 1
        else:
            payload = bytes(other[oi])
            oi += 1
        out.append((seconds[i], frames[i], kinds[i], statuses[i], forms[i], payload))
    return out


def raw_length(records: Iterable[Record]) -> int:
    """Codec 0's length of the same records: 9 B of record head and the bytes."""
    return sum(9 + len(r[5]) for r in records)

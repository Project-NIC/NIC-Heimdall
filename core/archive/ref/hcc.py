"""HCC — the Heimdall Compression Codec, the Python reference.

Codes one address's kept records into one bit stream and back, losslessly, exactly as
`core/archive/HCC.md` fixes it: columns, prediction up to order 8, Rice codes in partitions of 32,
an optional reference channel, Elias gamma counts. The coder's choices are the document's, so the
C and this write the same bytes.

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

Record = Tuple[int, int, int, int, int, bytes]

MAX_RECORDS = 1 << 16   # HCC.md, *Decoding*: a count above it is damaged; HMC's 16 kB holds < 1 500


# ---------------------------------------------------------------- bit I/O

class BitWriter:
    __slots__ = ("buf", "acc", "nbits")

    def __init__(self) -> None:
        self.buf = bytearray()
        self.acc = 0
        self.nbits = 0

    def write(self, value: int, width: int) -> None:
        if width == 0:
            return
        self.acc = (self.acc << width) | (value & ((1 << width) - 1))
        self.nbits += width
        while self.nbits >= 8:
            self.nbits -= 8
            self.buf.append((self.acc >> self.nbits) & 0xFF)
        self.acc &= (1 << self.nbits) - 1

    def unary(self, q: int) -> None:
        while q > 0:
            n = min(q, 32)
            self.write((1 << n) - 1, n)
            q -= n
        self.write(0, 1)

    def gamma(self, m: int) -> None:
        assert m >= 1
        length = m.bit_length() - 1
        self.write(0, length)
        self.write(m, length + 1)

    def bytes(self) -> bytes:
        if self.nbits:
            self.buf.append((self.acc << (8 - self.nbits)) & 0xFF)
            self.acc = 0
            self.nbits = 0
        return bytes(self.buf)

    def bit_length(self) -> int:
        return len(self.buf) * 8 + self.nbits


class BitReader:
    __slots__ = ("data", "pos", "end")

    def __init__(self, data: bytes) -> None:
        self.data = data
        self.pos = 0
        self.end = len(data) * 8

    def read(self, width: int) -> int:
        if width == 0:
            return 0
        if self.pos + width > self.end:
            raise ValueError("HCC stream damaged: read past its end")
        v = 0
        p = self.pos
        for _ in range(width):
            v = (v << 1) | ((self.data[p >> 3] >> (7 - (p & 7))) & 1)
            p += 1
        self.pos = p
        return v

    def unary(self) -> int:
        q = 0
        while self.read(1):
            q += 1
        return q

    def gamma(self) -> int:
        length = 0
        while self.read(1) == 0:
            length += 1
            if length > 32:
                raise ValueError("HCC stream damaged: gamma code")
        return (1 << length) | self.read(length)


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
    channels, in offset order (HCC.md, *The input*)."""
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


# ---------------------------------------------------------------- a series

MAX_ORDER = 8


def _levinson(r: Sequence[float], max_order: int) -> List[List[float]]:
    """Levinson–Durbin, in the order written, no fused multiply-add: the coefficient sets for
    every order it reaches, a[p] = [a1 .. ap] with x̂ᵢ = Σ aⱼ xᵢ₋ⱼ."""
    sets: List[List[float]] = []
    if r[0] <= 0.0:
        return sets
    err = r[0]
    a: List[float] = []
    for p in range(1, max_order + 1):
        acc = r[p]
        for j in range(1, p):
            acc = acc - a[j - 1] * r[p - j]
        if err <= 0.0:
            break
        k = acc / err
        if not (-1.0 < k < 1.0):
            break
        new = [0.0] * p
        for j in range(1, p):
            new[j - 1] = a[j - 1] - k * a[p - j - 1]
        new[p - 1] = k
        a = new
        err = err * (1.0 - k * k)
        sets.append(list(a))
    return sets


def _quantise(a: Sequence[float]) -> Optional[Tuple[int, List[int]]]:
    """The largest s ≤ 15 that keeps every |aⱼ|·2^s below 2^14, rounded half away from zero."""
    amax = max(abs(x) for x in a)
    if amax == 0.0:
        return 15, [0] * len(a)
    s = 15
    while s >= 0 and amax * (1 << s) >= (1 << 14):
        s -= 1
    if s < 0:
        return None
    q = []
    for x in a:
        v = x * (1 << s)
        qi = int(abs(v) + 0.5)
        q.append(qi if v >= 0 else -qi)
    if any(abs(qi) >= (1 << 14) for qi in q):
        return None
    return s, q


def _to_signed(v: int, w: int) -> int:
    return v - (1 << w) if v & (1 << (w - 1)) else v


def _fold(r: int, w: int) -> int:
    r = _to_signed(r & ((1 << w) - 1), w)
    return (r << 1) ^ (r >> (w - 1)) if r < 0 else r << 1


def _unfold(u: int) -> int:
    return -(u >> 1) - 1 if u & 1 else u >> 1


def _partition_bits(us: Sequence[int], k: int, w: int) -> int:
    if k == 31:
        return 5 + w * len(us)
    return 5 + sum((u >> k) + 1 + k for u in us)


def _best_k(us: Sequence[int], w: int) -> int:
    best_k, best = 0, None
    for k in range(0, min(30, w) + 1):
        b = _partition_bits(us, k, w)
        if best is None or b < best:
            best, best_k = b, k
    if _partition_bits(us, 31, w) <= best:
        return 31
    return best_k


def _write_residuals(bw: BitWriter, rs: Sequence[int], w: int) -> None:
    us = [_fold(r, w) for r in rs]
    for i in range(0, len(us), 32):
        part = us[i:i + 32]
        k = _best_k(part, w)
        bw.write(k, 5)
        if k == 31:
            for u in part:
                bw.write(u, w)
        else:
            for u in part:
                bw.unary(u >> k)
                bw.write(u, k)


def _residual_bits(rs: Sequence[int], w: int) -> int:
    us = [_fold(r, w) for r in rs]
    total = 0
    for i in range(0, len(us), 32):
        part = us[i:i + 32]
        total += _partition_bits(part, _best_k(part, w), w)
    return total


def _read_residuals(br: BitReader, n: int, w: int) -> List[int]:
    out = []
    while len(out) < n:
        m = min(32, n - len(out))
        k = br.read(5)
        for _ in range(m):
            if k == 31:
                u = br.read(w)
            else:
                q = br.unary()
                u = (q << k) | br.read(k)
            out.append(_unfold(u))
    return out


def _lpc_candidates(xs: Sequence[int], w: int, signed: bool):
    n = len(xs)
    vals = [_to_signed(x, w) for x in xs] if signed else list(xs)
    r = []
    for lag in range(0, MAX_ORDER + 1):
        if lag >= n:
            break
        acc = 0
        for i in range(lag, n):
            acc += vals[i] * vals[i - lag]
        r.append(float(acc))
    if len(r) < 2:
        return []
    out = []
    for a in _levinson(r, min(MAX_ORDER, len(r) - 1)):
        q = _quantise(a)
        if q is None:
            continue
        s, coeffs = q
        p = len(coeffs)
        if p >= n:
            continue
        res = []
        for i in range(p, n):
            acc = 0
            for j in range(1, p + 1):
                acc += coeffs[j - 1] * vals[i - j]
            res.append((vals[i] - (acc >> s)) & ((1 << w) - 1))
        out.append((p, s, coeffs, res))
    return out


def _mode_bits(xs: Sequence[int], w: int, signed: bool):
    """Every mode's cost; returns a list of (bits, mode, payload) with the coder's tie rule
    (the lower mode, the lower order) applied by stable ordering."""
    n = len(xs)
    mask = (1 << w) - 1
    out = []
    if all(x == xs[0] for x in xs):
        out.append((3 + w, 0, None))
    if n >= 1:
        rs = [(xs[i] - xs[i - 1]) & mask for i in range(1, n)]
        out.append((3 + w + _residual_bits(rs, w), 1, rs))
    if n >= 2:
        rs = [(xs[i] - 2 * xs[i - 1] + xs[i - 2]) & mask for i in range(2, n)]
        out.append((3 + 2 * w + _residual_bits(rs, w), 2, rs))
    for p, s, coeffs, res in _lpc_candidates(xs, w, signed):
        out.append((3 + 3 + 4 + 15 * p + p * w + _residual_bits(res, w), 3, (p, s, coeffs, res)))
    out.append((3 + n * w, 4, None))
    return out


def _write_mode(bw: BitWriter, xs: Sequence[int], w: int, mode: int, payload) -> None:
    bw.write(mode, 3)
    if mode == 0:
        bw.write(xs[0], w)
    elif mode == 1:
        bw.write(xs[0], w)
        _write_residuals(bw, payload, w)
    elif mode == 2:
        bw.write(xs[0], w)
        bw.write(xs[1], w)
        _write_residuals(bw, payload, w)
    elif mode == 3:
        p, s, coeffs, res = payload
        bw.write(p - 1, 3)
        bw.write(s, 4)
        for c in coeffs:
            bw.write(c & 0x7FFF, 15)
        for i in range(p):
            bw.write(xs[i], w)
        _write_residuals(bw, res, w)
    else:
        for x in xs:
            bw.write(x, w)


def write_series(bw: BitWriter, xs: Sequence[int], w: int, signed: bool = False,
                 ref: Optional[Sequence[int]] = None) -> None:
    """One series of n values of w bits; `ref` the reference channel's values where the layout
    names one (the one leading bit is written then)."""
    n = len(xs)
    if n == 0:
        return
    mask = (1 << w) - 1
    best = None
    if ref is not None:
        cands = [(0, xs)]
        cands.append((1, [(x - y) & mask for x, y in zip(xs, ref)]))
    else:
        cands = [(None, xs)]
    for use_ref, vals in cands:
        for bits, mode, payload in _mode_bits(vals, w, signed):
            key = (bits, use_ref or 0, mode, (payload[0] if mode == 3 else 0))
            if best is None or key < best[0]:
                best = (key, use_ref, vals, mode, payload)
    _, use_ref, vals, mode, payload = best
    if ref is not None:
        bw.write(use_ref, 1)
    _write_mode(bw, vals, w, mode, payload)


def read_series(br: BitReader, n: int, w: int, signed: bool = False,
                ref: Optional[Sequence[int]] = None) -> List[int]:
    if n == 0:
        return []
    mask = (1 << w) - 1
    use_ref = br.read(1) if ref is not None else 0
    mode = br.read(3)
    if mode == 0:
        xs = [br.read(w)] * n
    elif mode == 1:
        xs = [br.read(w)]
        for r in _read_residuals(br, n - 1, w):
            xs.append((xs[-1] + r) & mask)
    elif mode == 2:
        if n < 2:
            raise ValueError("HCC stream damaged: order 2 on %d value" % n)
        xs = [br.read(w), br.read(w)]
        for r in _read_residuals(br, n - 2, w):
            xs.append((r + 2 * xs[-1] - xs[-2]) & mask)
    elif mode == 3:
        p = br.read(3) + 1
        if p >= n:
            raise ValueError("HCC stream damaged: LPC order %d on %d values" % (p, n))
        s = br.read(4)
        coeffs = [_to_signed(br.read(15), 15) for _ in range(p)]
        xs = [br.read(w) for _ in range(p)]
        vals = [_to_signed(x, w) for x in xs] if signed else list(xs)
        for r in _read_residuals(br, n - p, w):
            acc = 0
            for j in range(1, p + 1):
                acc += coeffs[j - 1] * vals[-j]
            v = (r + (acc >> s)) & mask
            xs.append(v)
            vals.append(_to_signed(v, w) if signed else v)
    elif mode == 4:
        xs = [br.read(w) for _ in range(n)]
    else:
        raise ValueError("HCC stream damaged: mode %d" % mode)
    if use_ref:
        xs = [(x + y) & mask for x, y in zip(xs, ref)]
    return xs


# ---------------------------------------------------------------- the stream

def encode(records: Sequence[Record], layout: Sequence[Field]) -> bytes:
    """The records of one address, in the order kept, to one HCC stream."""
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
        raise ValueError("HCC stream damaged: a count of %d records" % n)
    seconds = read_series(br, n, 32)
    frames = read_series(br, n, 8)
    kinds = read_series(br, n, 8)
    statuses = read_series(br, n, 8)
    forms = read_series(br, n, 8)
    if any(f > 1 for f in forms):
        raise ValueError("HCC stream damaged: a form other than frame or block")
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
            raise ValueError("HCC stream damaged: a block's group index past the %d groups" % g)
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
    if br.end - br.pos >= 8:
        raise ValueError("HCC stream damaged: bits left over")
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

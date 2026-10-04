"""Steim-2 — the miniSEED compression HCC is measured against.

A faithful packer of the standard: 64-byte frames of sixteen 32-bit words, word 0 the sixteen
2-bit nibbles, frame 0 carrying x₀ and xₙ in words 1 and 2, every other word one of the seven
packings of first differences — 1×30 bits, 2×15, 3×10, 4×8, 5×6, 6×5, 7×4 — chosen greedily, the
most differences a word can take. Returns the byte length, which is what the comparison needs,
and the frames for a round-trip check.
"""

from __future__ import annotations

from typing import List, Sequence, Tuple

# (count, bits each, nibble, dnib) — in the order the packer tries them, most differences first
_FORMS: List[Tuple[int, int, int, int]] = [
    (7, 4, 3, 2),
    (6, 5, 3, 1),
    (5, 6, 3, 0),
    (4, 8, 1, -1),
    (3, 10, 2, 3),
    (2, 15, 2, 2),
    (1, 30, 2, 1),
]


def _fits(d: int, bits: int) -> bool:
    lo = -(1 << (bits - 1))
    return lo <= d < -lo


def encode(xs: Sequence[int]) -> List[bytes]:
    """The series as Steim-2 frames. Every value must fit a signed 32-bit integer, every first
    difference a signed 31-bit one (the record is split there in a real writer; this one raises)."""
    n = len(xs)
    if n == 0:
        return []
    diffs = [xs[0]] + [xs[i] - xs[i - 1] for i in range(1, n)]
    for d in diffs[1:]:
        if not _fits(d, 31):
            raise ValueError("Steim-2 cannot carry a difference of %d" % d)
    frames: List[bytes] = []
    i = 0
    first = True
    while i < n:
        words: List[int] = [0]
        nibbles = 0
        if first:
            words.append(xs[0] & 0xFFFFFFFF)
            words.append(xs[-1] & 0xFFFFFFFF)
            nibbles = 0  # words 1 and 2 are nibble 0
            first = False
        while len(words) < 16 and i < n:
            for count, bits, nib, dnib in _FORMS:
                if i + count <= n and all(_fits(d, bits) for d in diffs[i:i + count]):
                    break
            else:
                # the tail is shorter than the smallest count it could fill: pack what is left
                left = n - i
                for count, bits, nib, dnib in _FORMS:
                    if count <= left and all(_fits(d, bits) for d in diffs[i:i + count]):
                        break
            body = 0
            for d in diffs[i:i + count]:
                body = (body << bits) | (d & ((1 << bits) - 1))
            w = body if dnib < 0 else (dnib << 30) | body
            nibbles = (nibbles << 2) | nib
            words.append(w & 0xFFFFFFFF)
            i += count
        # pad the frame
        pad = 16 - len(words)
        nibbles <<= 2 * pad
        words += [0] * pad
        words[0] = nibbles & 0xFFFFFFFF
        frames.append(b"".join(w.to_bytes(4, "big") for w in words))
    return frames


def decode(frames: Sequence[bytes], n: int) -> List[int]:
    out: List[int] = []
    first = True
    skip = True   # the first difference is x0 - x(-1) and is not used
    for fr in frames:
        words = [int.from_bytes(fr[4 * k:4 * k + 4], "big") for k in range(16)]
        nibbles = words[0]
        for k in range(1, 16):
            nib = (nibbles >> (2 * (15 - k))) & 3
            w = words[k]
            if first and k in (1, 2):
                if k == 1:
                    out.append(_s32(w))
                continue
            if nib == 0:
                continue
            if nib == 1:
                vals = [_sx((w >> (8 * (3 - j))) & 0xFF, 8) for j in range(4)]
            else:
                dnib = w >> 30
                if nib == 2:
                    count, bits = {1: (1, 30), 2: (2, 15), 3: (3, 10)}[dnib]
                else:
                    count, bits = {0: (5, 6), 1: (6, 5), 2: (7, 4)}[dnib]
                vals = [_sx((w >> (bits * (count - 1 - j))) & ((1 << bits) - 1), bits)
                        for j in range(count)]
            for d in vals:
                if skip:
                    skip = False
                    continue
                out.append(out[-1] + d)
        first = False
    return out[:n]


def _sx(v: int, bits: int) -> int:
    return v - (1 << bits) if v & (1 << (bits - 1)) else v


def _s32(v: int) -> int:
    return _sx(v, 32)


def byte_length(xs: Sequence[int]) -> int:
    return 64 * len(encode(xs))

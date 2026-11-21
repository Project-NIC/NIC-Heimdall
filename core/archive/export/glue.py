"""Heimdall's glue for NIC-SDK's exporters (tools/mlfseries.py, tools/mlf2mseed.py,
tools/mlf2iaga.py): what the profile H says a record is, as `../MLF.md` and `../NRC.md` fix it.
The SDK walks the files, the configs and the sections in force; this reads a segment.

A channel is the unit's LAYOUT channel: its fields in offset order, the k-th of n at
(frame · n + k) / (n · rows) in the second, so its rate is n · rows. It is named by its fields'
name when one channel bears it, or as `#c` — channel c — always. The time is the record's second
less GPS − UTC from the `TIME` in force; a `CLIPPED` frame is SEED's clipping bit, a `DISTURBED`
one its glitches bit; a later record of one frame replaces an earlier one (the SDK keeps the later).
SEED's orientation is the name's last letter — Z, N, E; X 1, Y 2 — else the channel's number.

    python3 NIC-SDK/tools/mlf2mseed.py glue.py OUT.mseed NET STA LOC '#0' FILE…
    python3 NIC-SDK/tools/mlf2iaga.py glue.py DIR CODE SOURCE minute '#0,#1,#2' FILE…
"""
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), "..", "ref"))
import hmlf  # noqa: E402  (it puts NIC-SDK's tools on the path)

LETTER = hmlf.LETTER
CLIPPED, DISTURBED = 6, 7
QUALITY = {CLIPPED: 0x02, DISTURBED: 0x08}


def _body(sections, sid):
    return next((s.body for s in sections if s.id == sid), None)


def _layout(sections):
    """The unit's rows and the fields of its own TYPE's layout."""
    b = _body(sections, hmlf.UNIT)
    unit = hmlf.parse_unit(b) if b is not None else None
    rows = unit[5] if unit else 128
    for s in sections:
        if s.id == hmlf.LAYOUT and unit:
            space, type_, _, _, fs = hmlf.parse_layout(s.body)
            if (space, type_) == (unit[0], unit[1]):
                return rows, fs
    return rows, []


def _channel_of(fields, name):
    if name.startswith("#") and name[1:].isdigit():
        c = int(name[1:])
        return c if any(f.channel == c for f in fields) else None
    cs = {f.channel for f in fields if f.name.rstrip(b"\0").decode("ascii", "replace") == name}
    return cs.pop() if len(cs) == 1 else None


def _fields(sections, name):
    rows, fs = _layout(sections)
    c = _channel_of(fs, name)
    if c is None:
        return rows, None, []
    return rows, c, sorted((f for f in fs if f.channel == c), key=lambda f: f.offset)


def channel(sections, name):
    rows, c, fs = _fields(sections, name)
    if not fs:
        return None
    f = fs[0]
    nm = f.name.rstrip(b"\0").decode("ascii", "replace")
    last = nm.rstrip()[-1:].upper()
    orient = last if last in "ZNE" and last else "1" if last == "X" else "2" if last == "Y" else str(c % 9 + 1)
    return dict(name=nm or "#%d" % c, orient=orient,
                unit=f.unit.rstrip(b"\0").decode("ascii", "replace") or "count",
                exp10=f.exp10, mantissa=f.mantissa, add=f.add, rate_num=len(fs) * rows, rate_den=1)


def station(sections):
    b = _body(sections, hmlf.STATION)
    if b is None:
        return None
    name, lat, lon, elev = struct.unpack_from("<32siih", b)
    return dict(name=name.rstrip(b"\0").decode("utf-8", "replace"), lat=lat / 1e7, lon=lon / 1e7,
                elev=float(elev) if elev != -0x8000 else 0.0)


def samples(sections, seg, name):
    rows, _, fs = _fields(sections, name)
    if not fs:
        return
    n = len(fs)
    t = _body(sections, hmlf.TIME)
    off = struct.unpack_from("<b", t)[0] if t else 0
    recs = hmlf.segment_records(seg, hmlf.nrc_layout(sections))
    if recs is None:
        raise ValueError("H: a segment not read")
    for sec, fr, kind, st, form, data in recs:
        if form or kind:
            continue                    # a FILLER, an event line: no sample
        for k, f in enumerate(fs):
            raw = int.from_bytes(data[f.offset:f.offset + f.width], "big" if f.big_endian else "little")
            if f.signed and raw >> (8 * f.width - 1):
                raw -= 1 << (8 * f.width)
            yield ((sec - off) * rows + fr) * n + k, raw, QUALITY.get(st, 0)

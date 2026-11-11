"""The exporters, the Python reference, as `core/archive/EXPORTERS.md` enters them: a channel's
series read out of an address's HMC files, and written as miniSEED 2.4 with Steim-2 in 512 B
records, with one StationXML document beside it.

An export is a function of the files and the request: the same bytes in, the same bytes out.
"""

from __future__ import annotations

import struct
from dataclasses import dataclass
from fractions import Fraction
from typing import Dict, List, Optional, Sequence, Tuple

import hmc
from hmc import LayoutField

CLIPPED, DISTURBED = 6, 7
US = 10 ** 6


@dataclass(frozen=True)
class Sample:
    t: Fraction          # UTC, seconds since 1970-01-01
    raw: int             # the counts, signed as the field says
    status: int


@dataclass
class Series:
    address: int
    channel: int
    rate: Fraction
    fields: List[LayoutField]
    station: Optional[Tuple[bytes, int, int, int]]    # name, latitude, longitude, elevation
    samples: List[Sample]


def _signed(v: int, width: int) -> int:
    bits = 8 * width
    return v - (1 << bits) if v & (1 << (bits - 1)) else v


def _layout_fields(sections, channel: int) -> Tuple[List[LayoutField], int]:
    """The channel's fields in offset order, and the unit's rows."""
    unit = next((hmc.parse_unit(s.body) for s in sections if s.id == hmc.UNIT), None)
    rows = unit[5] if unit else 128
    lay = None
    for s in sections:
        if s.id == hmc.LAYOUT and unit:
            space, type_, _, _, fs = hmc.parse_layout(s.body)
            if (space, type_) == (unit[0], unit[1]) and lay is None:
                lay = fs
    fs = sorted((f for f in (lay or []) if f.channel == channel), key=lambda f: f.offset)
    return fs, rows


def _offset_at(times: Sequence[Tuple[int, int]], second: int) -> int:
    """GPS − UTC from the TIME section in force at the second."""
    off = 0
    for frm, o in times:
        if frm <= second:
            off = o
    return off


def read_series(images: Sequence[bytes], channel: int) -> Series:
    """One channel of one address, from its files in sequence order: every sample at its UTC
    instant — the second less GPS − UTC, plus (frame · n + k) / (n · rows) for the k-th of the
    channel's n fields in a payload — a later record of one frame replacing an earlier one."""
    kept: Dict[Tuple[int, int], Tuple[int, int, bytes]] = {}
    fields, rows, address, station = None, 128, None, None
    times: List[Tuple[int, int]] = []
    for image in images:
        r = hmc.Reader(image)
        address = r.header.address
        sections = list(r.header.sections)
        for s in sections:
            if s.id == hmc.TIME:
                times.append((r.header.first_second, struct.unpack_from("<b", s.body)[0]))
            if s.id == hmc.STATION and station is None:
                name, lat, lon, elev = struct.unpack_from("<32siih", s.body)
                station = (name.rstrip(b"\x00"), lat, lon, elev)
        for t, obj, recs in r.objects():
            if t == hmc.CFG:
                sections = hmc.in_force(sections, obj["sections"])
                for s in obj["sections"]:
                    if s.id == hmc.TIME:
                        times.append((obj["from"], struct.unpack_from("<b", s.body)[0]))
                continue
            if t != hmc.SEG or not isinstance(recs, list):
                continue
            fs, rw = _layout_fields(sections, channel)
            if fields is None:
                fields, rows = fs, rw
            for sec, fr, kind, st, form, data in recs:
                if form == 0 and kind == 0:
                    kept[(sec, fr)] = (st, sec, data)
    fields = fields or []
    n = len(fields)
    if not n:
        raise ValueError("export: the channel has no field in the layout")
    times.sort()
    out = []
    for (sec, fr), (st, _, data) in sorted(kept.items()):
        base = Fraction(sec - _offset_at(times, sec)) + Fraction(fr, rows)
        for k, f in enumerate(fields):
            raw = int.from_bytes(data[f.offset:f.offset + f.width], "big" if f.big_endian else "little")
            out.append(Sample(base + Fraction(k, n * rows), _signed(raw, f.width) if f.signed else raw, st))
    out.sort(key=lambda s: s.t)
    return Series(address, channel, Fraction(n * rows), fields, station, out)


# ---------------------------------------------------------------- SEED codes

def seed_channel(rate: Fraction, unit: bytes, name: bytes, channel: int) -> str:
    """The band by the rate, the instrument by the quantity, the orientation by the channel's name."""
    r = float(rate)
    band = ("F" if r >= 1000 else "C" if r >= 250 else "H" if r >= 80 else "B" if r >= 10 else
            "M" if r > 1 else "L" if r >= 0.5 else "V" if r >= 0.05 else "U" if r >= 0.005 else "R")
    u = unit.decode("ascii", "replace").strip().lower().replace("^", "").replace("**", "")
    inst = ("N" if u in ("m/s2", "m/s/s") else "H" if u == "m/s" else "F" if u in ("nt", "t") else
            "D" if u in ("pa", "hpa", "kpa", "bar", "mbar") else "K" if u in ("k", "degc", "c") else "Y")
    last = name.decode("ascii", "replace").strip()[-1:].upper()
    orient = {"Z": "Z", "N": "N", "E": "E", "X": "1", "Y": "2"}.get(last, str((channel % 9) + 1))
    return band + inst + orient


# ---------------------------------------------------------------- miniSEED

_FORMS = [(7, 4, 3, 2), (6, 5, 3, 1), (5, 6, 3, 0), (4, 8, 1, -1), (3, 10, 2, 3), (2, 15, 2, 2), (1, 30, 2, 1)]


def _fits(d: int, bits: int) -> bool:
    return -(1 << (bits - 1)) <= d < (1 << (bits - 1))


def steim2_record(xs: Sequence[int], prev: Optional[int], frames: int = 7) -> Tuple[bytes, int]:
    """As many samples as `frames` Steim-2 frames take: (the frames, the count). Frame 0's words
    1 and 2 are the first and the last sample; the first difference is from the sample before,
    0 at a run's start; each word takes the most differences that fit, greedily."""
    n = len(xs)
    diffs = [xs[0] - prev if prev is not None else 0] + [xs[i] - xs[i - 1] for i in range(1, n)]
    out: List[List[int]] = []
    i = 0
    for fno in range(frames):
        if i >= n:
            break
        words, nibs = [0], []
        if fno == 0:
            words += [xs[0] & 0xFFFFFFFF, 0]
            nibs += [0, 0]
        while len(words) < 16 and i < n:
            for count, bits, nib, dnib in _FORMS:
                if count <= n - i and all(_fits(d, bits) for d in diffs[i:i + count]):
                    break
            else:
                raise ValueError("export: a difference past Steim-2's 30 bits")
            body = 0
            for d in diffs[i:i + count]:
                body = (body << bits) | (d & ((1 << bits) - 1))
            words.append((body if dnib < 0 else (dnib << 30) | body) & 0xFFFFFFFF)
            nibs.append(nib)
            i += count
        nibs += [0] * (16 - len(words))
        words += [0] * (16 - len(words))
        w0 = 0
        for nb in nibs:
            w0 = (w0 << 2) | nb
        words[0] = w0
        out.append(words)
    out[0][2] = xs[i - 1] & 0xFFFFFFFF
    return b"".join(w.to_bytes(4, "big") for ws in out for w in ws), i


def _btime(t: Fraction) -> Tuple[bytes, int]:
    """SEED's BTIME and blockette 1001's microseconds, −50 … 49."""
    us = round(t * US)
    units, rem = divmod(us, 100)
    if rem >= 50:
        units, rem = units + 1, rem - 100
    secs, tenths = divmod(units, 10000)
    days, s = divmod(secs, 86400)
    import datetime
    d = datetime.date(1970, 1, 1) + datetime.timedelta(days=days)
    doy = d.timetuple().tm_yday
    return struct.pack(">HHBBBBH", d.year, doy, s // 3600, s // 60 % 60, s % 60, 0, tenths), rem


def _rate(rate: Fraction) -> Tuple[int, int]:
    if rate.denominator == 1:
        return int(rate), 1
    if rate.numerator == 1:
        return -rate.denominator, 1
    return rate.numerator, -rate.denominator


def write_mseed(series: Series, network: str, station: str, location: str = "00",
                channel: Optional[str] = None) -> bytes:
    """The series as 512 B records: a gap — two samples not one period apart — ends a record; a
    record holding a CLIPPED sample carries the digitizer-clipping bit, a DISTURBED one the
    glitches bit."""
    f = series.fields[0]
    cha = channel or seed_channel(series.rate, f.unit, f.name, series.channel)
    period = 1 / series.rate
    out, seq, i = bytearray(), 1, 0
    s = series.samples
    while i < len(s):
        j = i + 1
        while j < len(s) and s[j].t - s[j - 1].t == period:
            j += 1
        run = s[i:j]
        prev = None
        k = 0
        while k < len(run):
            data, count = steim2_record([x.raw for x in run[k:k + 800]], prev)
            chunk = run[k:k + count]
            flags = (0x02 if any(x.status == CLIPPED for x in chunk) else 0) | \
                    (0x08 if any(x.status == DISTURBED for x in chunk) else 0)
            bt, us = _btime(chunk[0].t)
            fac, mult = _rate(series.rate)
            head = (b"%06dD " % seq + station.encode().ljust(5)[:5] + location.encode().ljust(2)[:2]
                    + cha.encode().ljust(3)[:3] + network.encode().ljust(2)[:2] + bt
                    + struct.pack(">HhhBBBBiHH", count, fac, mult, 0, 0, flags, 2, 0, 64, 48))
            b1000 = struct.pack(">HHBBBB", 1000, 56, 11, 1, 9, 0)
            b1001 = struct.pack(">HHBbBB", 1001, 0, 100, us, 0, len(data) // 64)
            rec = head + b1000 + b1001 + data
            out += rec + bytes(512 - len(rec))
            prev = chunk[-1].raw
            seq += 1
            k += count
        i = j
    return bytes(out)


# ---------------------------------------------------------------- StationXML

def write_stationxml(series_list: Sequence[Series], network: str, station: str, start: str,
                     location: str = "00") -> bytes:
    """FDSN StationXML 1.2: the station from `STATION`, a channel a series, its sensitivity flat
    from the layout's scale — counts per unit is 1 / (mantissa · 10^exp10)."""
    from xml.sax.saxutils import escape
    st = series_list[0].station or (b"", 0, 0, -0x8000)
    lat, lon = st[1] / 1e7, st[2] / 1e7
    elev = 0 if st[3] == -0x8000 else st[3]
    ch = []
    for s in series_list:
        f = s.fields[0]
        scale = (f.mantissa or 1) * Fraction(10) ** f.exp10
        unit = f.unit.decode("ascii", "replace") or "count"
        cha = seed_channel(s.rate, f.unit, f.name, s.channel)
        ch.append(f"""   <Channel code="{cha}" locationCode="{location}" startDate="{start}">
    <Latitude>{lat:.7f}</Latitude>
    <Longitude>{lon:.7f}</Longitude>
    <Elevation>{elev}</Elevation>
    <Depth>0</Depth>
    <SampleRate>{float(s.rate):g}</SampleRate>
    <Response>
     <InstrumentSensitivity>
      <Value>{float(1 / scale):.10g}</Value>
      <Frequency>1</Frequency>
      <InputUnits><Name>{escape(unit)}</Name></InputUnits>
      <OutputUnits><Name>count</Name></OutputUnits>
     </InstrumentSensitivity>
    </Response>
   </Channel>""")
    name = escape(st[0].decode("utf-8", "replace"))
    xml = f"""<?xml version="1.0" encoding="UTF-8"?>
<FDSNStationXML xmlns="http://www.fdsn.org/xml/station/1" schemaVersion="1.2">
 <Source>NIC-Heimdall</Source>
 <Created>{start}</Created>
 <Network code="{network}">
  <Station code="{station}" startDate="{start}">
   <Latitude>{lat:.7f}</Latitude>
   <Longitude>{lon:.7f}</Longitude>
   <Elevation>{elev}</Elevation>
   <Site><Name>{name}</Name></Site>
{chr(10).join(ch)}
  </Station>
 </Network>
</FDSNStationXML>
"""
    return xml.encode()

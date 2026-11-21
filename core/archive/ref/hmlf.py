"""MLF — Heimdall's profile, the Python reference.

One address's file as `core/archive/MLF.md` fixes it, on MLF's core (NIC-SDK, `tools/mlf.py`): the
source — the station in bytes 16–18, the address in 19 —, Heimdall's sections, the records and the
event log's lines, the codec set of raw and NRC, the path, the recording and send rules. The C
(`c/hmlf.c`) writes the same bytes, and `tests/test_c.py` compares them.

A record is NRC's profile's: (second, frame, kind, status, form, data). An event-log line is
(second, source, code, detail, value).
"""

from __future__ import annotations

import struct
from dataclasses import dataclass, field
from typing import Dict, Iterator, List, Optional, Sequence, Tuple

import sdk  # noqa: F401
import mlf
from mlf import (CFG, CODEC, MB, NRC, RAW, SEG, Section, cobs_decode, cobs_encode, padded,  # noqa: F401
                 config_bytes, crc16, crc32, entry_bytes, framed, in_force, parse_entry,
                 parse_object, parse_sections, scan, sections_bytes)
import hnrc
from hnrc import Field

STATION, UNIT, LAYOUT, PROFILE, SETTINGS, RULES, RESPONSE = range(1, 8)
TIME = 9                             # 8 not used; a codec's table is the core's CODEC, 255
LETTER = "H"                         # the profile's letter, the mark's fourth byte
MAX_BLOCK = 255 - 2                  # a record's length is one byte: module · length · data


# ---------------------------------------------------------------- sections

@dataclass(frozen=True)
class LayoutField:
    offset: int
    width: int
    signed: bool = False
    big_endian: bool = False
    channel: int = 0
    ref: int = 0
    exp10: int = 0
    mantissa: int = 0
    add: int = 0
    name: bytes = b""
    unit: bytes = b""


def layout_section(space: int, type_: int, revision: int, cls: int,
                   fields: Sequence[LayoutField]) -> Section:
    b = bytearray(struct.pack("<BBBBB3x", space, type_, revision, cls, len(fields)))
    for f in fields:
        b += struct.pack("<BBBBbBhi8s8s4x", f.offset, f.width,
                         int(f.signed) | int(f.big_endian) << 1, f.channel, f.exp10, f.ref,
                         f.mantissa, f.add, f.name, f.unit)
    return Section(LAYOUT, bytes(b))


def parse_layout(body: bytes) -> Tuple[int, int, int, int, List[LayoutField]]:
    if len(body) < 8:
        raise ValueError("MLF: a LAYOUT cut short")
    space, type_, revision, cls, count = struct.unpack_from("<BBBBB3x", body)
    if len(body) < 8 + 32 * count:
        raise ValueError("MLF: a LAYOUT cut short")
    fields = []
    for i in range(count):
        o, w, fl, ch, e, ref, m, add, name, unit = struct.unpack_from("<BBBBbBhi8s8s4x", body, 8 + 32 * i)
        fields.append(LayoutField(o, w, bool(fl & 1), bool(fl & 2), ch, ref, e, m, add,
                                  name.rstrip(b"\x00"), unit.rstrip(b"\x00")))
    return space, type_, revision, cls, fields


def unit_section(space: int, type_: int, card: int, port: int, slot: int, rows: int,
                 positions: Sequence[int] = (0, 0, 0, 0)) -> Section:
    return Section(UNIT, struct.pack("<BBBBBH4B", space, type_, card, port, slot, rows, *positions))


def parse_unit(body: bytes) -> Tuple[int, int, int, int, int, int, Tuple[int, int, int, int]]:
    if len(body) < 11:
        raise ValueError("MLF: a UNIT cut short")
    v = struct.unpack_from("<BBBBBH4B", body)
    return v[0], v[1], v[2], v[3], v[4], v[5], tuple(v[6:10])


def station_section(name: bytes, latitude: int, longitude: int, elevation: int = -0x8000) -> Section:
    return Section(STATION, struct.pack("<32siih", name, latitude, longitude, elevation))


def time_section(offset: int, pending: int = 0, at: int = 0) -> Section:
    return Section(TIME, struct.pack("<bbI", offset, pending, at))


def nrc_layout(sections: Sequence[Section]) -> List[Field]:
    """The layout NRC codes a segment with, from the sections in force: an Argus NUMBER's four
    positions by the mini layouts `UNIT` seats there, any other unit by its TYPE's `LAYOUT`, and
    with no `LAYOUT` one channel a byte."""
    layouts: Dict[Tuple[int, int], List[LayoutField]] = {}
    unit = None
    for s in sections:
        if s.id == LAYOUT:
            space, type_, _, _, fs = parse_layout(s.body)
            layouts.setdefault((space, type_), fs)
        elif s.id == UNIT and unit is None:
            unit = parse_unit(s.body)

    def fields(fs):
        return [Field(f.offset, f.width, f.signed, f.big_endian, f.channel, f.ref) for f in fs]

    if unit is None:
        return hnrc.compose_layout([])
    space, type_, _, _, _, _, positions = unit
    if any(positions):
        minis = [fields(layouts[(2, a >> 4)]) if a and (2, a >> 4) in layouts else None
                 for a in positions]
        return hnrc.argus_layout(minis)
    return hnrc.compose_layout(fields(layouts.get((space, type_), [])))


# ---------------------------------------------------------------- the header

@dataclass
class Header:
    address: int
    sequence: int
    first_second: int
    station: int
    size: int
    sections: List[Section] = field(default_factory=list)
    length: int = 0


def to_core(h: Header) -> mlf.Header:
    """The source: the station in its three low bytes, the address in its high one."""
    return mlf.Header(h.sequence, h.first_second, (h.station & 0xFFFFFF) | (h.address << 24),
                      h.size, list(h.sections), h.length, LETTER)


def from_core(c: mlf.Header) -> Header:
    return Header(c.source >> 24, c.sequence, c.first_second, c.source & 0xFFFFFF, c.size,
                  c.sections, c.length)


def header_bytes(h: Header) -> bytes:
    return mlf.header_bytes(to_core(h))


def parse_header(b: bytes) -> Header:
    return from_core(mlf.parse_header(b))


# ---------------------------------------------------------------- segments and configs

def raw_body(records: Sequence[hnrc.Record]) -> bytes:
    return b"".join(struct.pack("<IBBBBB", r[0], r[1], r[2], r[3], r[4], len(r[5])) + r[5]
                    for r in records)


def parse_raw(body: bytes) -> List[hnrc.Record]:
    out, i = [], 0
    while i < len(body):
        if i + 9 > len(body):
            raise ValueError("MLF: a raw record cut short")
        sec, fr, kind, st, form, ln = struct.unpack_from("<IBBBBB", body, i)
        if i + 9 + ln > len(body):
            raise ValueError("MLF: a raw record cut short")
        out.append((sec, fr, kind, st, form, bytes(body[i + 9:i + 9 + ln])))
        i += 9 + ln
    return out


def event_body(lines: Sequence[Tuple[int, int, int, int, int]]) -> bytes:
    """The event log's records, address 00: second · source · code · detail · value."""
    return b"".join(struct.pack("<IBBHI", *ln) for ln in lines)


def parse_events(body: bytes) -> List[Tuple[int, int, int, int, int]]:
    if len(body) % 12:
        raise ValueError("MLF: an event line cut short")
    return [struct.unpack_from("<IBBHI", body, i) for i in range(0, len(body), 12)]


def segment_bytes(records: Sequence[hnrc.Record], layout: Sequence[Field]) -> bytes:
    """One write of kept records: coded raw and by NRC, the shorter written, raw on a tie."""
    if not records:
        raise ValueError("a segment holds at least one record")
    for r in records:
        if r[4] == 0 and len(r[5]) != 32 or r[4] == 1 and not (2 <= len(r[5]) <= 2 + MAX_BLOCK
                                                               and len(r[5]) == 2 + r[5][1]) \
                or r[4] > 1:
            raise ValueError("MLF: a record that is neither a frame nor a block")
    first, last = min(r[0] for r in records), max(r[0] for r in records)
    bodies = [(RAW, raw_body(records))]
    try:
        bodies.append((NRC, hnrc.encode(records, layout)))
    except ValueError:
        pass
    codec, body = mlf.shortest(bodies)
    return mlf.segment_bytes(first, last, codec, body)


def event_segment(lines: Sequence[Tuple[int, int, int, int, int]]) -> bytes:
    if not lines:
        raise ValueError("a segment holds at least one line")
    return mlf.segment_bytes(min(ln[0] for ln in lines), max(ln[0] for ln in lines), RAW,
                             event_body(lines))


def segment_records(seg: dict, layout: Sequence[Field]) -> Optional[List[hnrc.Record]]:
    """A segment's records, or None for a codec this reader does not know."""
    if seg["codec"] == RAW:
        return parse_raw(seg["body"])
    if seg["codec"] == NRC:
        return hnrc.decode(seg["body"], layout)
    return None


# ---------------------------------------------------------------- a file

def path(address: int, first_second: int) -> str:
    """`52/20261001/120000.mlf`: the address in hex, the day and the first second on the
    station's scale, read as a calendar without leap seconds."""
    days, s = divmod(first_second, 86400)
    # civil date from days since 1970-01-01 (Howard Hinnant's algorithm)
    z = days + 719468
    era, doe = divmod(z, 146097)
    yoe = (doe - doe // 1460 + doe // 36524 - doe // 146096) // 365
    doy = doe - (365 * yoe + yoe // 4 - yoe // 100)
    mp = (5 * doy + 2) // 153
    d = doy - (153 * mp + 2) // 5 + 1
    m = mp + 3 if mp < 10 else mp - 9
    y = yoe + era * 400 + (m <= 2)
    return f"{address:02X}/{y:04d}{m:02d}{d:02d}/{s // 3600:02d}{s // 60 % 60:02d}{s % 60:02d}.mlf"


class Writer(mlf.Writer):
    """MLF's writer with Heimdall's header, records and event lines."""

    def __init__(self, header: Header):
        super().__init__(to_core(header))

    def segment(self, records: Sequence[hnrc.Record]) -> bool:
        seg = segment_bytes(records, nrc_layout(self.sections))
        return self.append(seg, SEG, max(r[0] for r in records))

    def events(self, lines) -> bool:
        return self.append(event_segment(lines), SEG, max(ln[0] for ln in lines))


class Reader(mlf.Reader):
    """MLF's reader with Heimdall's header; each segment decoded with the layout in force."""

    def __init__(self, image: bytes):
        super().__init__(image)
        self.header = from_core(self.header)

    def objects(self) -> Iterator[Tuple[int, dict, Optional[list]]]:
        """(type, object, records): a segment's records, None where its codec is unknown, the
        string 'damaged' where it does not decode; a config's records None."""
        for t, obj, sections in super().objects():
            if t == 0:
                yield 0, obj, "damaged"
                continue
            if t == CFG:
                yield t, obj, None
                continue
            try:
                if self.header.address == 0:
                    recs = parse_events(obj["body"]) if obj["codec"] == RAW else None
                else:
                    recs = segment_records(obj, nrc_layout(sections))
            except ValueError:
                recs = "damaged"
            yield t, obj, recs


# ---------------------------------------------------------------- the rules

ALL, CHANGE, DECIMATE, INTERVAL, NONZERO, NONE = range(6)
MAX_BLOCK_KEYS = 256


@dataclass(frozen=True)
class Rule:
    """A recording or send rule: `n` for DECIMATE, `s` the interval of INTERVAL or the *at least
    every* of CHANGE and NONZERO (0 none); `modules` a send rule's ModBus modules, () all."""
    mode: int
    n: int = 0
    s: int = 0
    modules: Tuple[int, ...] = ()


def rule_bytes(r: Rule) -> bytes:
    return struct.pack("<BBH", r.mode, r.n, r.s)


def rules_section(recording: Rule, links: Sequence[Tuple[int, Rule]] = ()) -> Section:
    b = rule_bytes(recording) + bytes([len(links)])
    for link, r in links:
        b += bytes([link]) + rule_bytes(r) + bytes([len(r.modules)]) + bytes(r.modules)
    return Section(RULES, b)


def parse_rules(body: bytes) -> Tuple[Rule, List[Tuple[int, Rule]]]:
    try:
        mode, n, s = struct.unpack_from("<BBH", body)
        rec, links, i = Rule(mode, n, s), [], 5
        for _ in range(body[4]):
            link, mode, n, s, k = struct.unpack_from("<BBBHB", body, i)
            if i + 6 + k > len(body):
                raise ValueError
            links.append((link, Rule(mode, n, s, tuple(body[i + 6:i + 6 + k]))))
            i += 6 + k
        return rec, links
    except (struct.error, IndexError, ValueError):
        raise ValueError("MLF: a RULES section cut short")


def split_blocks(payload: bytes) -> Optional[List[bytes]]:
    """A ModBus payload's whole blocks, `address · length · data` until a zero address or the
    end; None when a block runs past the 32 B — the frame is then kept as a frame."""
    out, i = [], 0
    while i < len(payload) and payload[i] != 0:
        if i + 2 > len(payload) or i + 2 + payload[i + 1] > len(payload):
            return None
        out.append(bytes(payload[i:i + 2 + payload[i + 1]]))
        i += 2 + payload[i + 1]
    return out


class Selector:
    """One address's rule, applied as the head walks the second's frames (`frame`) or as a link's
    send rule walks what was recorded (`record`). Returns the records kept."""

    def __init__(self, rule: Rule, blocks: bool = False):
        self.rule, self.blocks = rule, blocks
        self.state = None                    # (kind, status) of the last record kept
        self.last: Dict[object, Tuple[bytes, int]] = {}
        self.interval: Dict[object, int] = {}
        self.nzero = None                    # the second of the last record NONZERO kept

    def frame(self, second: int, frame: int, kind: int, status: int, payload: bytes) -> List[hnrc.Record]:
        if self.blocks and kind == 0:
            bl = split_blocks(payload)
            if bl:
                return [r for r in ((second, frame, kind, status, 1, b) for b in bl) if self._keep(r)]
        r = (second, frame, kind, status, 0, bytes(payload))
        return [r] if self._keep(r) else []

    def record(self, r: hnrc.Record) -> List[hnrc.Record]:
        if r[4] == 1 and self.rule.modules and r[5][0] not in self.rule.modules:
            return []
        return [r] if self._keep(r) else []

    def _keep(self, r: hnrc.Record) -> bool:
        rule = self.rule
        if rule.mode == NONE:
            return False
        sec, fr, kind, st, form, data = r
        key = (data[0], data[1]) if form == 1 else "frame"
        ikey = data[0] if form == 1 else "frame"
        if key not in self.last and form == 1 and \
                sum(1 for k in self.last if k != "frame") >= MAX_BLOCK_KEYS:
            keep, store = True, False
        else:
            store = True
            if self.state is None or (kind, st) != self.state or 5 <= kind <= 8 or rule.mode == ALL:
                keep = True
            elif rule.mode == CHANGE:
                prev = self.last.get(key)
                keep = prev is None or prev[0] != data or (rule.s and sec - prev[1] >= rule.s)
            elif rule.mode == NONZERO:
                keep = any(data) or (rule.s and (self.nzero is None or sec - self.nzero >= rule.s))
            elif rule.mode == DECIMATE:
                keep = rule.n <= 1 or fr % rule.n == 0
            elif rule.mode == INTERVAL:
                keep = rule.s == 0 or self.interval.get(ikey) != sec // rule.s
            else:
                keep = False
        if keep:
            self.state = (kind, st)
            self.nzero = sec
            if store:
                self.last[key] = (data, sec)
            if rule.s:
                self.interval[ikey] = sec // rule.s
        return bool(keep)

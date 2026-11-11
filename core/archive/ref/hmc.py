"""HMC — the Heimdall Matryoshka Container, the Python reference.

One address's file as `core/archive/HMC.md` fixes it: the header and its sections, the data —
segments and configs, each COBS-framed and followed by a zero — growing up from the header, and
the description's 16 B entries growing down from the end. Writes and reads a whole file in
memory; the C (`c/hmc.c`) writes the same bytes, and `tests/test_c.py` compares them.

A record is HCC's: (second, frame, kind, status, form, data). An event-log line is
(second, source, code, detail, value).
"""

from __future__ import annotations

import struct
import zlib
from dataclasses import dataclass, field
from typing import Dict, Iterator, List, Optional, Sequence, Tuple

import hcc
from hcc import Field

MAGIC = b"HMC\x00"
VERSION = 2
MB = 1 << 20
SEG, CFG = 1, 2                      # an entry's type
SYNC = {SEG: b"HSEG", CFG: b"HCFG"}
STATION, UNIT, LAYOUT, PROFILE, SETTINGS, RULES, RESPONSE, CODEC, TIME = range(1, 10)
RAW, HCC = 0, 1                      # a segment's codec
MAX_BLOCK = 255 - 2                  # a record's length is one byte: module · length · data


# ---------------------------------------------------------------- checks and framing

def crc32(b: bytes) -> int:
    """IEEE 802.3, the one zlib computes."""
    return zlib.crc32(b) & 0xFFFFFFFF


def crc16(b: bytes) -> int:
    """CRC-16/CCITT-FALSE: polynomial 0x1021, initial 0xFFFF, no reflection, no final XOR."""
    c = 0xFFFF
    for x in b:
        c ^= x << 8
        for _ in range(8):
            c = ((c << 1) ^ 0x1021) & 0xFFFF if c & 0x8000 else (c << 1) & 0xFFFF
    return c


def cobs_encode(data: bytes) -> bytes:
    """COBS as Cheshire and Baker's encoder writes it: a block of up to 254 bytes behind its code,
    and a data that ends on a full block closed by an empty one."""
    out = bytearray(b"\x00")
    at, code = 0, 1
    for b in data:
        if b == 0:
            out[at] = code
            at, code = len(out), 1
            out.append(0)
        else:
            out.append(b)
            code += 1
            if code == 0xFF:
                out[at] = code
                at, code = len(out), 1
                out.append(0)
    out[at] = code
    return bytes(out)


def cobs_decode(enc: bytes) -> bytes:
    out = bytearray()
    i, n = 0, len(enc)
    while i < n:
        c = enc[i]
        if c == 0 or i + c > n:
            raise ValueError("HMC: COBS damaged")
        block = enc[i + 1:i + c]
        if 0 in block:
            raise ValueError("HMC: COBS damaged")
        out += block
        i += c
        if c < 0xFF and i < n:
            out.append(0)
    return bytes(out)


def framed(obj: bytes) -> bytes:
    return cobs_encode(obj) + b"\x00"


# ---------------------------------------------------------------- sections

@dataclass(frozen=True)
class Section:
    id: int
    body: bytes


def sections_bytes(sections: Sequence[Section]) -> bytes:
    return b"".join(struct.pack("<B3xI", s.id, len(s.body)) + s.body for s in sections)


def parse_sections(b: bytes) -> List[Section]:
    out, i = [], 0
    while i < len(b):
        if i + 8 > len(b):
            raise ValueError("HMC: a section cut short")
        sid, ln = struct.unpack_from("<B3xI", b, i)
        if i + 8 + ln > len(b):
            raise ValueError("HMC: a section cut short")
        out.append(Section(sid, bytes(b[i + 8:i + 8 + ln])))
        i += 8 + ln
    return out


def in_force(old: Sequence[Section], new: Sequence[Section]) -> List[Section]:
    """A config's sections over those in force: every id it carries replaces all of that id."""
    ids = {s.id for s in new}
    return [s for s in old if s.id not in ids] + list(new)


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
        raise ValueError("HMC: a LAYOUT cut short")
    space, type_, revision, cls, count = struct.unpack_from("<BBBBB3x", body)
    if len(body) < 8 + 32 * count:
        raise ValueError("HMC: a LAYOUT cut short")
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
        raise ValueError("HMC: a UNIT cut short")
    v = struct.unpack_from("<BBBBBH4B", body)
    return v[0], v[1], v[2], v[3], v[4], v[5], tuple(v[6:10])


def station_section(name: bytes, latitude: int, longitude: int, elevation: int = -0x8000) -> Section:
    return Section(STATION, struct.pack("<32siih", name, latitude, longitude, elevation))


def time_section(offset: int, pending: int = 0, at: int = 0) -> Section:
    return Section(TIME, struct.pack("<bbI", offset, pending, at))


def hcc_layout(sections: Sequence[Section]) -> List[Field]:
    """The layout HCC codes a segment with, from the sections in force: an Argus NUMBER's four
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
        return hcc.compose_layout([])
    space, type_, _, _, _, _, positions = unit
    if any(positions):
        minis = [fields(layouts[(2, a >> 4)]) if a and (2, a >> 4) in layouts else None
                 for a in positions]
        return hcc.argus_layout(minis)
    return hcc.compose_layout(fields(layouts.get((space, type_), [])))


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


def header_bytes(h: Header) -> bytes:
    body = sections_bytes(h.sections)
    length = 20 + len(body) + 4
    b = MAGIC + struct.pack("<BBHII", VERSION, h.address, h.sequence, length, h.first_second) + \
        h.station.to_bytes(3, "little") + bytes([h.size]) + body
    return b + struct.pack("<I", crc32(b))


def parse_header(b: bytes) -> Header:
    if len(b) < 24 or b[:4] != MAGIC or b[4] != VERSION:
        raise ValueError("HMC: not a version-2 file")
    address, sequence, length, first = struct.unpack_from("<BHII", b, 5)
    if length < 24 or length > len(b) or struct.unpack_from("<I", b, length - 4)[0] != crc32(b[:length - 4]):
        raise ValueError("HMC: the header is damaged")
    size = b[19]
    if size > 7:
        raise ValueError("HMC: a size past 128 MB")
    return Header(address, sequence, first, int.from_bytes(b[16:19], "little"), size,
                  parse_sections(b[20:length - 4]), length)


# ---------------------------------------------------------------- segments and configs

def raw_body(records: Sequence[hcc.Record]) -> bytes:
    return b"".join(struct.pack("<IBBBBB", r[0], r[1], r[2], r[3], r[4], len(r[5])) + r[5]
                    for r in records)


def parse_raw(body: bytes) -> List[hcc.Record]:
    out, i = [], 0
    while i < len(body):
        if i + 9 > len(body):
            raise ValueError("HMC: a raw record cut short")
        sec, fr, kind, st, form, ln = struct.unpack_from("<IBBBBB", body, i)
        if i + 9 + ln > len(body):
            raise ValueError("HMC: a raw record cut short")
        out.append((sec, fr, kind, st, form, bytes(body[i + 9:i + 9 + ln])))
        i += 9 + ln
    return out


def event_body(lines: Sequence[Tuple[int, int, int, int, int]]) -> bytes:
    """The event log's records, address 00: second · source · code · detail · value."""
    return b"".join(struct.pack("<IBBHI", *ln) for ln in lines)


def parse_events(body: bytes) -> List[Tuple[int, int, int, int, int]]:
    if len(body) % 12:
        raise ValueError("HMC: an event line cut short")
    return [struct.unpack_from("<IBBHI", body, i) for i in range(0, len(body), 12)]


def _segment(first: int, last: int, codec: int, body: bytes) -> bytes:
    b = b"HSEG" + struct.pack("<IIIB3x", 20 + len(body) + 4, first, last, codec) + body
    return b + struct.pack("<I", crc32(b))


def segment_bytes(records: Sequence[hcc.Record], layout: Sequence[Field]) -> bytes:
    """One write of kept records: coded raw and by HCC, the shorter written, raw on a tie."""
    if not records:
        raise ValueError("a segment holds at least one record")
    for r in records:
        if r[4] == 0 and len(r[5]) != 32 or r[4] == 1 and not (2 <= len(r[5]) <= 2 + MAX_BLOCK
                                                               and len(r[5]) == 2 + r[5][1]) \
                or r[4] > 1:
            raise ValueError("HMC: a record that is neither a frame nor a block")
    first, last = min(r[0] for r in records), max(r[0] for r in records)
    body, codec = raw_body(records), RAW
    try:
        coded = hcc.encode(records, layout)
        if len(coded) < len(body):
            body, codec = coded, HCC
    except ValueError:
        pass
    return _segment(first, last, codec, body)


def event_segment(lines: Sequence[Tuple[int, int, int, int, int]]) -> bytes:
    if not lines:
        raise ValueError("a segment holds at least one line")
    return _segment(min(ln[0] for ln in lines), max(ln[0] for ln in lines), RAW, event_body(lines))


def config_bytes(from_second: int, sections: Sequence[Section]) -> bytes:
    body = sections_bytes(sections)
    b = b"HCFG" + struct.pack("<II4x", 16 + len(body) + 4, from_second) + body
    return b + struct.pack("<I", crc32(b))


def parse_object(b: bytes) -> Tuple[int, dict]:
    """A segment or a config, unframed: (SEG, {first, last, codec, body}) or (CFG, {from,
    sections}); ValueError when it is not whole."""
    if len(b) < 20 or b[:4] not in (b"HSEG", b"HCFG"):
        raise ValueError("HMC: no sync")
    length = struct.unpack_from("<I", b, 4)[0]
    if length != len(b) or struct.unpack_from("<I", b, length - 4)[0] != crc32(b[:length - 4]):
        raise ValueError("HMC: an object is damaged")
    if b[:4] == b"HSEG":
        if length < 24:
            raise ValueError("HMC: a segment cut short")
        first, last, codec = struct.unpack_from("<IIB", b, 8)
        return SEG, {"first": first, "last": last, "codec": codec, "body": bytes(b[20:length - 4])}
    return CFG, {"from": struct.unpack_from("<I", b, 8)[0], "sections": parse_sections(b[16:length - 4])}


def segment_records(seg: dict, layout: Sequence[Field]) -> Optional[List[hcc.Record]]:
    """A segment's records, or None for a codec this reader does not know."""
    if seg["codec"] == RAW:
        return parse_raw(seg["body"])
    if seg["codec"] == HCC:
        return hcc.decode(seg["body"], layout)
    return None


def entry_bytes(second: int, offset: int, length: int, type_: int) -> bytes:
    b = struct.pack("<IIIBx", second, offset, length, type_)
    return b + struct.pack("<H", crc16(b))


def parse_entry(b: bytes) -> Optional[Tuple[int, int, int, int]]:
    """(second, offset, length, type), or None for a free or damaged entry."""
    if struct.unpack_from("<H", b, 14)[0] != crc16(b[:14]) or b[12] not in (SEG, CFG):
        return None
    second, offset, length, type_ = struct.unpack_from("<IIIB", b)
    return second, offset, length, type_


# ---------------------------------------------------------------- a file

def path(address: int, first_second: int) -> str:
    """`52/20261001/120000.hmc`: the address in hex, the day and the first second on the
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
    return f"{address:02X}/{y:04d}{m:02d}{d:02d}/{s // 3600:02d}{s // 60 % 60:02d}{s % 60:02d}.hmc"


class Writer:
    """A file in memory: the header, then each object appended as the head writes it — its framed
    bytes first, then its entry. `append` returns False when the file is full."""

    def __init__(self, header: Header):
        self.size = MB << header.size
        self.image = bytearray(self.size)
        hb = header_bytes(header)
        self.image[:len(hb)] = hb
        self.end = len(hb)
        self.entries = 0
        self.last = 0
        self.sections = list(header.sections)

    def append(self, obj: bytes, type_: int, second: int) -> bool:
        if second < self.last:
            raise ValueError("HMC: an entry's second goes back")
        fb = framed(obj)
        desc = self.size - 16 * (self.entries + 1)
        if self.end + len(fb) > desc:
            return False
        self.image[self.end:self.end + len(fb)] = fb
        self.image[desc:desc + 16] = entry_bytes(second, self.end, len(fb) - 1, type_)
        self.end += len(fb)
        self.entries += 1
        self.last = second
        return True

    def segment(self, records: Sequence[hcc.Record]) -> bool:
        seg = segment_bytes(records, hcc_layout(self.sections))
        return self.append(seg, SEG, max(r[0] for r in records))

    def events(self, lines) -> bool:
        return self.append(event_segment(lines), SEG, max(ln[0] for ln in lines))

    def config(self, from_second: int, sections: Sequence[Section]) -> bool:
        if not self.append(config_bytes(from_second, sections), CFG, from_second):
            return False
        self.sections = in_force(self.sections, sections)
        return True


class Reader:
    """A whole file: the header, the description's good entries, and the objects they point at,
    in the order written, each segment decoded with the layout in force where it lies."""

    def __init__(self, image: bytes):
        self.image = bytes(image)
        self.header = parse_header(self.image)
        if len(self.image) != MB << self.header.size:
            raise ValueError("HMC: the file is not its size")
        self.entries = self._entries()

    def _entry(self, i: int):
        at = len(self.image) - 16 * (i + 1)
        return parse_entry(self.image[at:at + 16])

    def _entries(self) -> List[Optional[Tuple[int, int, int, int]]]:
        """The written entries are good ones, a free one all zeros and failing its CRC, so a binary
        search finds the last good entry; a damaged one the search stepped over is None."""
        lo, hi = 0, (len(self.image) - self.header.length) // 16
        while lo < hi:
            mid = (lo + hi) // 2
            if self._entry(mid) is None:
                hi = mid
            else:
                lo = mid + 1
        return [self._entry(i) for i in range(lo)]

    def _second(self, i: int) -> int:
        """An entry's second; a damaged one takes the last good one's before it, 0 with none."""
        while i >= 0 and self.entries[i] is None:
            i -= 1
        return self.entries[i][0] if i >= 0 else 0

    def find(self, second: int) -> int:
        """The index of the first entry whose second is not before `second`."""
        lo, hi = 0, len(self.entries)
        while lo < hi:
            mid = (lo + hi) // 2
            if self._second(mid) < second:
                lo = mid + 1
            else:
                hi = mid
        return lo

    def object(self, i: int) -> Tuple[int, dict]:
        if self.entries[i] is None:
            raise ValueError("HMC: a damaged entry")
        second, offset, length, type_ = self.entries[i]
        if offset + length >= len(self.image) or self.image[offset + length] != 0:
            raise ValueError("HMC: an entry points past its object")
        t, obj = parse_object(cobs_decode(self.image[offset:offset + length]))
        if t != type_:
            raise ValueError("HMC: an entry of the wrong type")
        return t, obj

    def objects(self) -> Iterator[Tuple[int, dict, Optional[list]]]:
        """(type, object, records): a segment's records, None where its codec is unknown, the
        string 'damaged' where it does not decode; a config's records None."""
        sections = list(self.header.sections)
        for i in range(len(self.entries)):
            try:
                t, obj = self.object(i)
            except ValueError:
                yield 0, {"entry": self.entries[i]}, "damaged"
                continue
            if t == CFG:
                sections = in_force(sections, obj["sections"])
                yield t, obj, None
                continue
            try:
                if self.header.address == 0:
                    recs = parse_events(obj["body"]) if obj["codec"] == RAW else None
                else:
                    recs = segment_records(obj, hcc_layout(sections))
            except ValueError:
                recs = "damaged"
            yield t, obj, recs


def scan(image: bytes, start: int) -> Iterator[Tuple[int, dict]]:
    """A file whose description is lost, or a packet cut short: every whole object between the
    zeros from `start` on."""
    i, n = start, len(image)
    while i < n:
        j = image.find(b"\x00", i)
        if j < 0:
            j = n
        if j > i:
            try:
                yield parse_object(cobs_decode(image[i:j]))
            except ValueError:
                pass
        i = j + 1


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
        raise ValueError("HMC: a RULES section cut short")


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

    def frame(self, second: int, frame: int, kind: int, status: int, payload: bytes) -> List[hcc.Record]:
        if self.blocks and kind == 0:
            bl = split_blocks(payload)
            if bl:
                return [r for r in ((second, frame, kind, status, 1, b) for b in bl) if self._keep(r)]
        r = (second, frame, kind, status, 0, bytes(payload))
        return [r] if self._keep(r) else []

    def record(self, r: hcc.Record) -> List[hcc.Record]:
        if r[4] == 1 and self.rule.modules and r[5][0] not in self.rule.modules:
            return []
        return [r] if self._keep(r) else []

    def _keep(self, r: hcc.Record) -> bool:
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

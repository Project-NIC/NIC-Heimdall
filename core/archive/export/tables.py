"""The CSV and SQL exporters, as `../EXPORTERS.md` enters them: every record of an address read out
of its files with the layout, the rule and GPS − UTC in force at it; written as one CSV file an
address, or into SQLite tables that PostgreSQL takes as they are, with the catalogue of the files
beside them.
"""

from __future__ import annotations

import csv
import io
import os
import sqlite3
import struct
import sys
from dataclasses import dataclass
from fractions import Fraction
from typing import Dict, Iterator, List, Optional, Sequence, Tuple

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), "..", "ref"))
import hcc  # noqa: E402
import hmc  # noqa: E402
from hmc import LayoutField  # noqa: E402

RULE_NAMES = ["ALL", "CHANGE", "DECIMATE", "INTERVAL", "NONZERO", "NONE"]


@dataclass
class Context:
    """What is in force at a record: the layout's fields — on an Argus NUMBER the mini layouts at
    their positions, numbered as HCC numbers them — its revision, the recording rule, GPS − UTC."""
    fields: List[LayoutField]
    revision: int
    rule: str
    offset: int
    rows: int
    type_: int


def _context(sections: Sequence[hmc.Section]) -> Context:
    unit = next((hmc.parse_unit(s.body) for s in sections if s.id == hmc.UNIT), None)
    layouts: Dict[Tuple[int, int], Tuple[int, List[LayoutField]]] = {}
    for s in sections:
        if s.id == hmc.LAYOUT:
            space, type_, rev, _, fs = hmc.parse_layout(s.body)
            layouts.setdefault((space, type_), (rev, fs))
    fields, rev = [], 0
    if unit and any(unit[6]):
        nxt = 0
        for pos, a in enumerate(unit[6]):
            if not a or (2, a >> 4) not in layouts:
                continue
            r, fs = layouts[(2, a >> 4)]
            rev = max(rev, r)
            base = nxt
            for f in fs:
                fields.append(LayoutField(f.offset + 8 * pos, f.width, f.signed, f.big_endian,
                                          base + f.channel, (base + f.ref) if f.ref else 0,
                                          f.exp10, f.mantissa, f.add, f.name, f.unit))
            nxt = max(f.channel for f in fields) + 1
    elif unit and (unit[0], unit[1]) in layouts:
        rev, fields = layouts[(unit[0], unit[1])]
    rule = "ALL"
    for s in sections:
        if s.id == hmc.RULES:
            r, _ = hmc.parse_rules(s.body)
            rule = RULE_NAMES[r.mode] if r.mode < len(RULE_NAMES) else str(r.mode)
            rule += f" {r.n}" if r.mode == hmc.DECIMATE else f" {r.s} s" if r.mode == hmc.INTERVAL else \
                (f", at least every {r.s} s" if r.s else "")
    offset = next((struct.unpack_from("<b", s.body)[0] for s in sections if s.id == hmc.TIME), 0)
    return Context(sorted(fields, key=lambda f: f.offset), rev, rule, offset,
                   unit[5] if unit else 128, unit[1] if unit else 0)


@dataclass
class Row:
    second: int
    frame: int
    kind: int
    status: int
    form: int
    data: bytes
    utc: Fraction
    ctx: Context


def read_records(images: Sequence[bytes]) -> Tuple[hmc.Header, List[Row], List[dict]]:
    """Every record of one address, its files in sequence order: (the first header, the records
    ordered by second and frame — of two for one frame the later written — and the catalogue's
    rows: one a segment and one a config)."""
    kept: Dict[Tuple[int, int, int, bytes], Row] = {}
    order: Dict[Tuple, int] = {}
    catalogue, first, n = [], None, 0
    for image in images:
        r = hmc.Reader(image)
        first = first or r.header
        path = hmc.path(r.header.address, r.header.first_second)
        sections = list(r.header.sections)
        ctx = _context(sections)
        for i, (t, obj, recs) in enumerate(r.objects()):
            e = r.entries[i]
            if t == hmc.CFG:
                sections = hmc.in_force(sections, obj["sections"])
                ctx = _context(sections)
                catalogue.append({"kind": "config", "file": path, "offset": e[1], "length": e[2],
                                  "from": obj["from"], "rule": ctx.rule, "revision": ctx.revision,
                                  "sections": hmc.sections_bytes(obj["sections"])})
                continue
            if t != hmc.SEG:
                continue
            catalogue.append({"kind": "segment", "file": path, "offset": e[1], "length": e[2],
                              "first": obj["first"], "last": obj["last"], "codec": obj["codec"]})
            if not isinstance(recs, list):
                continue
            for rec in recs:
                if r.header.address == 0:
                    sec, src, code, detail, value = rec
                    key = (sec, n, 0, b"")
                    kept[key] = Row(sec, 0, 0, 0, 2, struct.pack("<BBHI", src, code, detail, value),
                                    Fraction(sec - ctx.offset), ctx)
                    n += 1
                    continue
                sec, fr, kind, st, form, data = rec
                key = (sec, fr, form, data[:2] if form == 1 else b"")
                kept[key] = Row(sec, fr, kind, st, form, data,
                                Fraction(sec - ctx.offset) + Fraction(fr, ctx.rows), ctx)
    rows = sorted(kept.values(), key=lambda x: (x.utc, x.frame))
    return first, rows, catalogue


def field_values(row: Row) -> List[Tuple[LayoutField, int, float]]:
    """A data frame's fields in offset order: (field, raw, physical)."""
    out = []
    if row.form != 0 or row.kind != 0:
        return out
    for f in row.ctx.fields:
        raw = int.from_bytes(row.data[f.offset:f.offset + f.width], "big" if f.big_endian else "little")
        if f.signed and raw & (1 << (8 * f.width - 1)):
            raw -= 1 << (8 * f.width)
        out.append((f, raw, (raw + f.add) * (f.mantissa or 1) * 10.0 ** f.exp10))
    return out


def iso(t: Fraction) -> str:
    """ISO 8601 UTC to the microsecond."""
    import datetime
    us = round(t * 10 ** 6)
    d = datetime.datetime(1970, 1, 1) + datetime.timedelta(microseconds=us)
    return d.strftime("%Y-%m-%dT%H:%M:%S.%fZ")


def _column(f: LayoutField, k: int, many: bool) -> str:
    name = f.name.decode("ascii", "replace") or f"ch{f.channel}"
    unit = f.unit.decode("ascii", "replace")
    return f"{name}{'.' + str(k) if many else ''}" + (f" [{unit}]" if unit else "")


# ---------------------------------------------------------------- CSV

def write_csv(images: Sequence[bytes], raw: bool = False) -> str:
    """One address's file: a comment block — station, address, TYPE, revision, rule — the header,
    then a line a record: UTC, second, frame, kind, status, the values, and the bytes of a record
    the layout does not read (another kind, a block, an event line) in hex. A comment repeats
    when a config changes what is in force."""
    header, rows, _ = read_records(images)
    st = next((struct.unpack_from("<32siih", s.body) for s in header.sections if s.id == hmc.STATION), None)
    out = io.StringIO()
    w = csv.writer(out, lineterminator="\n")
    ctx = None
    for row in rows:
        if row.ctx is not ctx:
            ctx = row.ctx
            out.write(f"# station {st[0].rstrip(bytes(1)).decode('utf-8', 'replace') if st else ''} "
                      f"({header.station:06X}), address {header.address:02X}, TYPE {ctx.type_}, "
                      f"layout revision {ctx.revision}, rule {ctx.rule}, GPS - UTC {ctx.offset} s\n")
            per = {}
            for f in ctx.fields:
                per.setdefault(f.channel, []).append(f)
            cols = [_column(f, per[f.channel].index(f), len(per[f.channel]) > 1) for f in ctx.fields]
            w.writerow(["utc", "second", "frame", "kind", "status"] + cols + ["bytes"])
        vals = field_values(row)
        line = [iso(row.utc), row.second, row.frame, row.kind, row.status]
        if vals:
            line += [r if raw else repr(v) for _, r, v in vals] + [""]
        else:
            line += [""] * len(ctx.fields) + [row.data.hex()]
        w.writerow(line)
    return out.getvalue()


# ---------------------------------------------------------------- SQL

SCHEMA = """
CREATE TABLE IF NOT EXISTS stations (station INTEGER PRIMARY KEY, name TEXT, latitude REAL,
    longitude REAL, elevation INTEGER);
CREATE TABLE IF NOT EXISTS units (station INTEGER, address INTEGER, type INTEGER, rows INTEGER,
    first_second INTEGER, PRIMARY KEY (station, address, first_second));
CREATE TABLE IF NOT EXISTS configs (station INTEGER, address INTEGER, from_second INTEGER,
    rule TEXT, layout_revision INTEGER, sections BLOB, file TEXT, file_offset INTEGER,
    PRIMARY KEY (station, address, file, file_offset));
CREATE TABLE IF NOT EXISTS fields (station INTEGER, address INTEGER, from_second INTEGER,
    channel INTEGER, sample INTEGER, field_offset INTEGER, name TEXT, unit TEXT, exp10 INTEGER,
    mantissa INTEGER, add_raw INTEGER, PRIMARY KEY (station, address, from_second, field_offset));
CREATE TABLE IF NOT EXISTS samples (station INTEGER, address INTEGER, second INTEGER, frame INTEGER,
    kind INTEGER, status INTEGER, channel INTEGER, sample INTEGER, raw INTEGER, value REAL,
    utc TEXT, PRIMARY KEY (station, address, second, frame, channel, sample));
CREATE TABLE IF NOT EXISTS events (station INTEGER, second INTEGER, line INTEGER, source INTEGER,
    code INTEGER, detail INTEGER, value INTEGER, utc TEXT, PRIMARY KEY (station, second, line));
CREATE TABLE IF NOT EXISTS segments (station INTEGER, address INTEGER, file TEXT,
    file_offset INTEGER, length INTEGER, first_second INTEGER, last_second INTEGER,
    codec INTEGER, PRIMARY KEY (station, file, file_offset));
"""


def write_sql(db: sqlite3.Connection, images: Sequence[bytes]) -> None:
    """One address's files into the tables: rebuilt rows replace what they key — a late frame's
    later record wins, as the reader takes it."""
    header, rows, catalogue = read_records(images)
    db.executescript(SCHEMA)
    sta, adr = header.station, header.address
    st = next((struct.unpack_from("<32siih", s.body) for s in header.sections if s.id == hmc.STATION), None)
    if st:
        db.execute("INSERT OR REPLACE INTO stations VALUES (?,?,?,?,?)",
                   (sta, st[0].rstrip(bytes(1)).decode("utf-8", "replace"), st[1] / 1e7, st[2] / 1e7,
                    None if st[3] == -0x8000 else st[3]))
    unit = next((hmc.parse_unit(s.body) for s in header.sections if s.id == hmc.UNIT), None)
    db.execute("INSERT OR REPLACE INTO units VALUES (?,?,?,?,?)",
               (sta, adr, unit[1] if unit else None, unit[5] if unit else None, header.first_second))
    seen = set()
    for row in rows:
        if id(row.ctx) not in seen:
            seen.add(id(row.ctx))
            per: Dict[int, int] = {}
            for f in row.ctx.fields:
                k = per.get(f.channel, 0)
                per[f.channel] = k + 1
                db.execute("INSERT OR REPLACE INTO fields VALUES (?,?,?,?,?,?,?,?,?,?,?)",
                           (sta, adr, row.second, f.channel, k, f.offset, f.name.decode("ascii", "replace"),
                            f.unit.decode("ascii", "replace"), f.exp10, f.mantissa or 1, f.add))
        if row.form == 2:
            src, code, detail, value = struct.unpack("<BBHI", row.data)
            n = db.execute("SELECT COUNT(*) FROM events WHERE station=? AND second=?", (sta, row.second)).fetchone()[0]
            db.execute("INSERT INTO events VALUES (?,?,?,?,?,?,?,?)",
                       (sta, row.second, n, src, code, detail, value, iso(row.utc)))
            continue
        per = {}
        for f, raw, val in field_values(row):
            k = per.get(f.channel, 0)
            per[f.channel] = k + 1
            db.execute("INSERT OR REPLACE INTO samples VALUES (?,?,?,?,?,?,?,?,?,?,?)",
                       (sta, adr, row.second, row.frame, row.kind, row.status, f.channel, k, raw, val,
                        iso(row.utc)))
    for c in catalogue:
        if c["kind"] == "segment":
            db.execute("INSERT OR REPLACE INTO segments VALUES (?,?,?,?,?,?,?,?)",
                       (sta, adr, c["file"], c["offset"], c["length"], c["first"], c["last"], c["codec"]))
        else:
            db.execute("INSERT OR REPLACE INTO configs VALUES (?,?,?,?,?,?,?,?)",
                       (sta, adr, c["from"], c["rule"], c["revision"], c["sections"], c["file"], c["offset"]))
    db.commit()

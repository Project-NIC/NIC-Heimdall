"""The CSV and SQL exporters on the reference's synthetic files: every value against the reference
library's own reading — the records HCC decodes, cut by the layout HCC codes with — a late frame's
later record kept, the catalogue's rows one a segment and one a config, a window queried."""
import csv, io, os, sqlite3, sys
HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(HERE, ".."))
sys.path.insert(0, os.path.join(HERE, "..", "..", "ref"))
sys.path.insert(0, os.path.join(HERE, "..", "..", "ref", "tests"))
import hmc, tables
import test_hmc


def reference(image):
    """(second, frame) → the raw values of the declared fields, in offset order, as the reference
    library reads them; and the counts of segments and configs."""
    r = hmc.Reader(image)
    sections, out, segs, cfgs = list(r.header.sections), {}, 0, 0
    for t, obj, recs in r.objects():
        if t == hmc.CFG:
            sections = hmc.in_force(sections, obj["sections"])
            cfgs += 1
            continue
        segs += 1
        declared = [f for f in hmc.hcc_layout(sections) if not (f.width == 1 and not f.signed and f.channel >= 0
                    and not any(lf.offset == f.offset for lf in tables._context(sections).fields))]
        for rec in recs:
            if r.header.address == 0 or rec[4] != 0 or rec[2] != 0:
                continue
            vals = []
            for f in sorted(declared, key=lambda f: f.offset):
                v = int.from_bytes(rec[5][f.offset:f.offset + f.width], "big" if f.big_endian else "little")
                if f.signed and v & (1 << (8 * f.width - 1)):
                    v -= 1 << (8 * f.width)
                vals.append(v)
            if vals:                                                # a TYPE with no layout has none
                out[(rec[0], rec[1])] = vals
    return out, segs, cfgs


def main():
    for name, header, ops in test_hmc.scenarios():
        ops = list(ops)
        late = None
        if ops[0][0] == "seg" and ops[0][1][0][4] == 0 and ops[-1][0] == "seg":
            late = ops[0][1][5][:5] + (bytes([0x7F]) * 32,)       # a frame repaired, in the next segment
            ops[-1] = ("seg", list(ops[-1][1]) + [late])
        w, kept = test_hmc.write(header, ops)
        image = bytes(w.image)
        ref, segs, cfgs = reference(image)
        text = tables.write_csv([image], raw=True)
        rows = [r for r in csv.reader(io.StringIO("\n".join(l for l in text.splitlines() if not l.startswith("#"))))]
        data = {}
        for r in rows:
            if r[0] == "utc":
                continue
            if r[-1] == "" and len(r) > 6:
                data[(int(r[1]), int(r[2]))] = [int(x) for x in r[5:-1]]
        assert data == ref, name
        if late and (late[0], late[1]) in ref and ops[-1] in kept:  # the later written stands
            assert all(v in (0x7F, 0x7F7F, 0x7F7F7F, 0x7F7F7F7F) for v in data[(late[0], late[1])]), name
        physical = tables.write_csv([image])
        assert len(physical.splitlines()) == len(text.splitlines())
        db = sqlite3.connect(":memory:")
        tables.write_sql(db, [image])
        nseg = db.execute("SELECT COUNT(*) FROM segments").fetchone()[0]
        ncfg = db.execute("SELECT COUNT(*) FROM configs").fetchone()[0]
        assert (nseg, ncfg) == (segs, cfgs), (name, nseg, ncfg, segs, cfgs)
        if header.address == 0:
            n = db.execute("SELECT COUNT(*) FROM events").fetchone()[0]
            assert n == sum(len(op[1]) for op in kept if op[0] == "ev"), name
            print(f"{name:20s} {n} event lines")
            continue
        got = {}
        for sec, fr, raw in db.execute("SELECT second, frame, raw FROM samples ORDER BY second, frame, "
                                       "(SELECT field_offset FROM fields f WHERE f.station = samples.station "
                                       "AND f.address = samples.address AND f.channel = samples.channel "
                                       "AND f.sample = samples.sample ORDER BY from_second LIMIT 1)"):
            got.setdefault((sec, fr), []).append(raw)
        assert got == ref, name
        if ref:
            lo, hi = sorted(ref)[len(ref) // 3][0], sorted(ref)[len(ref) // 2][0]
            q = db.execute("SELECT COUNT(DISTINCT second || '.' || frame) FROM samples WHERE second BETWEEN ? AND ?",
                           (lo, hi)).fetchone()[0]
            assert q == sum(1 for k in ref if lo <= k[0] <= hi), name
        print(f"{name:20s} {len(ref)} frames, CSV and SQL value for value; {segs} segments, {cfgs} configs catalogued")
    print("every table reads as the reference library does")


if __name__ == "__main__":
    main()

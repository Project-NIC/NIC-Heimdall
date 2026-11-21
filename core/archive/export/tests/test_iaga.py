"""Heimdall's files to IAGA-2002 through NIC-SDK's `tools/mlf2iaga.py` and Heimdall's glue: a
synthetic 32 Hz magnetometer, 20 minutes with one frame CLIPPED and two seconds missing, written,
read back by an independent fixed-column parser — the station from `STATION`, GPS − UTC from
`TIME`, the gap's minute missing, a minute where the sine through the Gaussian says. The filters
themselves are NIC-SDK's to test (its `tests/test_iaga_reference.py`)."""
import math, os, random, sys, tempfile
HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(HERE, ".."))
sys.path.insert(0, os.path.join(HERE, "..", "..", "ref"))
import hmlf
from hmlf import Header, LayoutField
import mlfseries, mlf2iaga, iaga

T0 = 1_790_812_800 + 3 * 3600          # 2026-10-01 03:00:00 UTC


def parse(text):
    """Fixed columns, as the format lays them: date, time, then four values of ten."""
    head, rows = [], {}
    for line in text.splitlines():
        assert len(line) == 70, (len(line), line)
        if line.startswith(" "):
            head.append((line[1:24].strip(), line[24:69].strip()))
        elif line.startswith("DATE"):
            names = [line[32:42].strip(), line[42:52].strip(), line[52:62].strip(), line[62:69].strip()]
        else:
            rows[line[:23]] = [float(line[30 + 10 * k:40 + 10 * k]) for k in range(4)]
    return dict(head), names, rows


def main():
    rng = random.Random(1)
    fields = [LayoutField(3 * c, 3, True, False, c, 0, -2, 1, 0, b"mag" + bytes("XYZ"[c], "ascii"), b"nT")
              for c in range(3)]
    header = Header(0x11, 1, T0, 0x123456, 2, [
        hmlf.station_section(b"Test", 490_719_000, 140_190_000, 496), hmlf.unit_section(1, 1, 1, 1, 1, 32),
        hmlf.layout_section(1, 1, 1, 0, fields), hmlf.time_section(18)])
    w = hmlf.Writer(header)
    recs, base = [], [2_040_000, 166_000, 4_460_000]
    for i in range(32 * 1200):
        sec, fr = T0 + 18 + i // 32, i % 32
        if 600 <= i // 32 < 602:
            continue
        pl = bytearray(32)
        for c in range(3):
            v = base[c] + int(500 * math.sin(2 * math.pi * (i / 32) / 600 + c)) + rng.randint(-3, 3)
            pl[3 * c:3 * c + 3] = (v & 0xFFFFFF).to_bytes(3, "little")
        recs.append((sec, fr, 0, 6 if i == 32 * 300 + 5 else 0, 0, bytes(pl)))
        if len(recs) == 300:
            assert w.segment(recs)
            recs = []
    if recs:
        assert w.segment(recs)
    tmp = tempfile.mkdtemp()
    path = os.path.join(tmp, "mag.mlf")
    open(path, "wb").write(bytes(w.image))
    g = mlfseries.load_glue(os.path.join(HERE, "..", "glue.py"))
    ch, _, v = mlfseries.series([path], g, "magX")
    assert ch["rate_num"] == 32 and v[0][0] == T0 * 32           # GPS − UTC taken off
    out, = mlf2iaga.export(g, tmp, "TST", "a test", "minute", "magX,magY,magZ", [path])
    assert os.path.basename(out) == "tst20261001vmin.min"
    head, names, rows = parse(open(out).read())
    assert head["IAGA Code"] == "TST" and head["Reported"].startswith("XYZ")
    assert head["Geodetic Longitude"] == "014.019" and len(rows) == 1440
    assert names[:3] == ["TSTX", "TSTY", "TSTZ"]
    gw = iaga.gaussian()
    for c in range(3):
        assert rows["2026-10-01 03:10:00.000"][c] == 99999.0       # the two missing seconds' minute
        assert rows["2026-10-01 00:00:00.000"][c] == 99999.0
        filled = sum(1 for k, r in rows.items() if k.startswith("2026-10-01 03:") and r[c] != 99999.0)
        assert filled == 18, filled          # the first and the last minute cannot fill, nor the gap's
        truth = base[c] / 100 + sum(wk * 5 * math.sin(2 * math.pi * (300 + k - 45) / 600 + c)
                                    for k, wk in enumerate(gw))
        assert abs(rows["2026-10-01 03:05:00.000"][c] - truth) < 0.03
    print("a 32 Hz day through NIC-SDK and Heimdall's glue, read back by its columns: the station, "
          "the gap, the minutes the filters give")


if __name__ == "__main__":
    main()

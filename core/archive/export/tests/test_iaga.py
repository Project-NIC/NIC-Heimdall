"""The IAGA-2002 exporter: the filters against SciPy's and against an observatory's own minutes,
and a synthetic 32 Hz magnetometer's files exported, read back by an independent fixed-column
parser and checked against the filters run on its raw series. Needs SciPy (with ObsPy).

The observatory check runs where `BDV_SEC` and `BDV_MIN` name a day of Budkov's second and minute
values (`../../ref/README.md` says how to fetch them)."""
import math, os, random, sys
from fractions import Fraction
HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(HERE, ".."))
sys.path.insert(0, os.path.join(HERE, "..", "..", "ref"))
sys.path.insert(0, os.path.join(HERE, "..", "..", "ref", "tests"))
import hmc, export, measure
from hmc import Header, LayoutField
from scipy import signal as sps

T0 = 1_790_812_800 + 3 * 3600          # 2026-10-01 03:00:00 UTC, a day number 20727 + 3 h
DAY = T0 // 86400


def parse(text):
    """Fixed columns, as the format lays them: date, time, then four values of ten."""
    head, rows = [], {}
    for line in text.decode().splitlines():
        assert len(line) == 70, (len(line), line)
        if line.startswith(" "):
            head.append((line[1:24].strip(), line[24:69].strip()))
        elif line.startswith("DATE"):
            names = [line[32:42].strip(), line[42:52].strip(), line[52:62].strip(), line[62:69].strip()]
        else:
            rows[line[:23]] = [float(line[30 + 10 * k:40 + 10 * k]) for k in range(4)]
    return dict(head), names, rows


def main():
    # the filters
    for r in (10, 32, 64):
        h = export.lowpass_second(r)
        assert max(abs(a - b) for a, b in zip(h, sps.firwin(len(h), 0.25, window="blackman", fs=float(r)))) < 1e-15
    g = sps.get_window(("gaussian", 15.8734), 91, fftbins=False)
    assert max(abs(a - b) for a, b in zip(export.gaussian_minute(), g / g.sum())) < 1e-15
    # an observatory's own minutes from its own seconds
    if os.environ.get("BDV_SEC") and os.environ.get("BDV_MIN"):
        sec = {n.split(":")[1]: xs for n, xs, _ in measure.read_iaga2002(os.environ["BDV_SEC"])}
        mins = {n.split(":")[1]: xs for n, xs, _ in measure.read_iaga2002(os.environ["BDV_MIN"])}
        for c in sec:
            out = export._filter({i: x / 100 for i, x in enumerate(sec[c])}, export.gaussian_minute(), 60,
                                 range(1, 1439))
            worst = max(abs(round(out[i], 2) - mins[c][i] / 100) for i in range(1, 1439))
            assert worst <= 0.0100001, (c, worst)
        print("Budkov's minutes from its seconds: within 0,01 nT, its rounding")
    # a 32 Hz magnetometer, 20 minutes, one frame CLIPPED and two seconds missing
    rng = random.Random(1)
    fields = [LayoutField(3 * c, 3, True, False, c, 0, -2, 0, 0, b"mag" + bytes("XYZ"[c], "ascii"), b"nT")
              for c in range(3)]
    header = Header(0x11, 1, T0, 0x123456, 2, [
        hmc.station_section(b"Test", 490_719_000, 140_190_000, 496), hmc.unit_section(1, 1, 1, 1, 1, 32),
        hmc.layout_section(1, 1, 1, 0, fields), hmc.time_section(18)])
    w = hmc.Writer(header)
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
    series = [export.read_series([bytes(w.image)], c) for c in range(3)]
    assert series[0].rate == 32 and series[0].samples[0].t == T0
    text = export.write_iaga2002(series, DAY, "TST", "a test")
    head, names, rows = parse(text)
    assert head["IAGA Code"] == "TST" and head["Reported"] == "XYZF" and head["Geodetic Longitude"] == "014.019"
    assert names == ["TSTX", "TSTY", "TSTZ", "TSTF"] and len(rows) == 1440
    # against the filters run here on the raw series
    for c, s in enumerate(series):
        phys = export.physical(s)
        vals = {int(t * 32): v for t, v in phys.items()}
        secs = export._filter(vals, export.lowpass_second(32), 32, range(T0 - 45, T0 + 1246))
        mins = export._filter({k: v for k, v in secs.items() if v is not None}, export.gaussian_minute(), 60,
                              range(T0 // 60, T0 // 60 + 21))
        got = 0
        for m, v in mins.items():
            dt = __import__('datetime')
            key = f"{dt.datetime(1970, 1, 1) + dt.timedelta(seconds=m * 60):%Y-%m-%d %H:%M:%S}.000"
            want = 99999.0 if v is None else round(v, 2)
            assert abs(rows[key][c] - want) < 0.0051, (c, key, rows[key][c], want)
            got += v is not None
        assert got == 18, got          # the first and the last minute cannot fill, nor the gap's
        assert rows["2026-10-01 03:10:00.000"][c] == 99999.0
        g = export.gaussian_minute()                               # the sine through the same weights
        truth = base[c] / 100 + sum(w * 5 * math.sin(2 * math.pi * (300 + k - 45) / 600 + c)
                                    for k, w in enumerate(g))
        assert abs(rows["2026-10-01 03:05:00.000"][c] - truth) < 0.03
    assert all(r[3] == 88888.0 for r in rows.values())
    assert rows["2026-10-01 00:00:00.000"][0] == 99999.0
    sec_text = export.write_iaga2002(series, DAY, "TST", "a test", "second")
    _, _, srows = parse(sec_text)
    assert len(srows) == 86400 and srows["2026-10-01 03:05:00.000"][0] != 99999.0
    print("the filters as SciPy's; a 32 Hz day written, read back by its columns, every minute as the "
          "filters give it")


if __name__ == "__main__":
    main()

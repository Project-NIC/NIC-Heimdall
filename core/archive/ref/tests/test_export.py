"""The miniSEED and StationXML exporters on a synthetic Quake's files, read back by ObsPy (and so
libmseed): every sample and its instant as recorded, a gap where a segment is missing, the
quality bits of CLIPPED and DISTURBED frames, the Steim-2 frames against `steim2.py`, and the
StationXML against the FDSN schema. Needs ObsPy."""
import io, os, struct, sys
from fractions import Fraction
sys.path.insert(0, os.path.join(os.path.dirname(__file__), ".."))
sys.path.insert(0, os.path.dirname(__file__))
import hmc, export, steim2
import test_hmc
from obspy import read, read_inventory, UTCDateTime
from obspy.io.stationxml.core import validate_stationxml


def main():
    name, header, ops = test_hmc.scenarios()[0]                  # the Quake, two configs
    # a CLIPPED and a DISTURBED frame, and one segment left out: a gap
    ops = [list(op) for op in ops]
    segs = [k for k, op in enumerate(ops) if op[0] == "seg"]
    recs = list(ops[segs[1]][1])
    recs[10] = recs[10][:3] + (export.CLIPPED,) + recs[10][4:]
    recs[-5] = recs[-5][:3] + (export.DISTURBED,) + recs[-5][4:]
    ops[segs[1]][1] = recs
    gap = ops.pop(segs[3])
    w, kept = test_hmc.write(header, [tuple(op) for op in ops])
    images = [bytes(w.image)]
    series = [export.read_series(images, c) for c in range(3)]
    s0 = series[0]
    assert s0.rate == 256 and len(s0.samples) == 2 * sum(len(op[1]) for op in ops if op[0] == "seg")
    data = export.write_mseed(s0, "XX", "HEIM")
    assert len(data) % 512 == 0
    st = read(io.BytesIO(data))
    st.merge(method=-1)
    assert len(st) == 2, st                                      # the gap splits the series
    want = [x.raw for x in s0.samples]
    got = [int(v) for tr in st for v in tr.data]
    assert got == want, "the samples differ"
    first = s0.samples[0].t
    assert abs(st[0].stats.starttime - UTCDateTime(float(first))) < 1e-6
    assert st[0].stats.sampling_rate == 256 and st[0].stats.channel == "CN1"
    k = next(i for i in range(1, len(s0.samples)) if s0.samples[i].t - s0.samples[i - 1].t != Fraction(1, 256))
    after = s0.samples[k]
    assert after.t - s0.samples[k - 1].t > len(gap[1]) / 128 - 1   # the left-out segment's length
    assert abs(st[1].stats.starttime - UTCDateTime(float(after.t))) < 1e-6
    # the quality bits, record by record, and the frames against steim2.py
    flags = [data[k + 38] for k in range(0, len(data), 512)]
    assert any(f & 0x02 for f in flags) and any(f & 0x08 for f in flags)
    assert sum(1 for f in flags if f) == 2
    at = 0
    for k in range(0, len(data), 512):
        n = struct.unpack_from(">H", data, k + 30)[0]
        nfr = data[k + 63]
        frames = [data[k + 64 + 64 * j:k + 128 + 64 * j] for j in range(nfr)]
        assert steim2.decode(frames, n) == want[at:at + n]
        at += n
    # every channel, and the station beside them
    for c, s in enumerate(series):
        tr = read(io.BytesIO(export.write_mseed(s, "XX", "HEIM")))
        assert [int(v) for t in tr for v in t.data] == [x.raw for x in s.samples], c
    xml = export.write_stationxml(series, "XX", "HEIM", "2026-10-01T00:00:00")
    ok, errors = validate_stationxml(io.BytesIO(xml))
    assert ok, errors
    inv = read_inventory(io.BytesIO(xml))
    chans = inv[0][0].channels
    assert [c.code for c in chans] == ["CN1", "CN2", "CN3"]
    assert abs(chans[0].response.instrument_sensitivity.value - 1e6) < 1e-3
    assert abs(inv[0][0].latitude - 50.1234567) < 1e-7
    # the same request, the same bytes
    assert export.write_mseed(export.read_series(images, 0), "XX", "HEIM") == data
    print(f"{len(data) // 512} miniSEED records of {len(want)} samples read back by ObsPy, "
          f"a gap and two quality bits where they belong; StationXML valid")


if __name__ == "__main__":
    main()

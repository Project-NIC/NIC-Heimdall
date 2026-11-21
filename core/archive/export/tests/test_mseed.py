"""Heimdall's files to miniSEED through NIC-SDK's `tools/mlf2mseed.py` and Heimdall's glue: a
synthetic Quake's file, read back by ObsPy (and so libmseed) — every sample as the profile decodes
it, a gap where a segment is missing, the quality bits of the CLIPPED and DISTURBED frames, the
StationXML against the FDSN schema, the same bytes for the same request. Needs ObsPy; the Steim-2
frames themselves are NIC-SDK's to test."""
import io, os, sys, tempfile
HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(HERE, ".."))
sys.path.insert(0, os.path.join(HERE, "..", "..", "ref"))
sys.path.insert(0, os.path.join(HERE, "..", "..", "ref", "tests"))
import hmlf  # noqa: F401  (NIC-SDK's tools on the path)
import glue, test_hmlf
import mlfseries, mlf2mseed
from obspy import read, read_inventory, UTCDateTime
from obspy.io.stationxml.core import validate_stationxml


def main():
    name, header, ops = test_hmlf.scenarios()[0]                  # the Quake, two configs
    ops = [list(op) for op in ops]
    segs = [k for k, op in enumerate(ops) if op[0] == "seg"]
    recs = list(ops[segs[1]][1])
    recs[10] = recs[10][:3] + (glue.CLIPPED,) + recs[10][4:]
    recs[-5] = recs[-5][:3] + (glue.DISTURBED,) + recs[-5][4:]
    ops[segs[1]][1] = recs
    gap = ops.pop(segs[3])                                        # one segment left out: a gap
    w, _ = test_hmlf.write(header, [tuple(op) for op in ops])
    tmp = tempfile.mkdtemp()
    path = os.path.join(tmp, "quake.mlf")
    open(path, "wb").write(bytes(w.image))
    g = mlfseries.load_glue(os.path.join(HERE, "..", "glue.py"))
    out = os.path.join(tmp, "c0.mseed")
    for c in range(3):
        ch, st, v = mlfseries.series([path], g, "#%d" % c)
        assert ch["rate_num"] == 256 and len(v) == 2 * sum(len(op[1]) for op in ops if op[0] == "seg")
        mlf2mseed.export(g, out, "XX", "HEIM", "00", "#%d" % c, [path])
        tr = read(out)
        tr.merge(method=-1)
        assert len(tr) == 2, tr                                   # the gap splits the series
        assert [int(x) for t in tr for x in t.data] == [s[1] for s in v], c
        assert tr[0].stats.sampling_rate == 256
        assert abs(tr[0].stats.starttime - UTCDateTime(v[0][0] / 256)) < 1e-6
        k = next(i for i in range(1, len(v)) if v[i][0] != v[i - 1][0] + 1)
        assert (v[k][0] - v[k - 1][0]) / 256 > len(gap[1]) / 128 - 1
        assert abs(tr[1].stats.starttime - UTCDateTime(v[k][0] / 256)) < 1e-6
    data = open(out, "rb").read()
    mlf2mseed.export(g, out, "XX", "HEIM", "00", "#0", [path])
    data = open(out, "rb").read()
    flags = [data[i + 38] for i in range(0, len(data), 512)]
    assert any(f & 0x02 for f in flags) and any(f & 0x08 for f in flags)
    assert sum(1 for f in flags if f) == 2
    xml = open(out + ".xml", "rb").read()
    ok, errors = validate_stationxml(io.BytesIO(xml))
    assert ok, errors
    inv = read_inventory(io.BytesIO(xml))
    assert abs(inv[0][0].latitude - 50.1234567) < 1e-7
    assert abs(inv[0][0].channels[0].response.instrument_sensitivity.value - 1e6) < 1e-3
    mlf2mseed.export(g, out, "XX", "HEIM", "00", "#0", [path])    # the same request, the same bytes
    assert open(out, "rb").read() == data
    print(f"{len(data) // 512} miniSEED records through NIC-SDK and Heimdall's glue, read back by "
          f"ObsPy; a gap and two quality bits where they belong; StationXML valid")


if __name__ == "__main__":
    main()

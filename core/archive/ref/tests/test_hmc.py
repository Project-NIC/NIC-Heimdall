"""HMC on synthetic files: every scenario written and read back record for record — a Quake with
a config that changes its layout mid-file, a Palatine's blocks, an Argus NUMBER, the event log, a
file filled to its end; a damaged object read past, and a lost description recovered by the zeros."""
import os, random, struct, sys
sys.path.insert(0, os.path.join(os.path.dirname(__file__), ".."))
import hcc, hmc
from hmc import Header, LayoutField, Section

T0 = 1_790_856_000          # 2026-10-01 12:00:00 on the station's scale


def chunks(records, seconds=8, limit=16384):
    """The head's buffer: a segment at 16 kB of records or 8 s after its first."""
    out, cur, size = [], [], 0
    for r in records:
        if cur and (r[0] - cur[0][0] >= seconds or size + 9 + len(r[5]) > limit):
            out.append(cur); cur, size = [], 0
        cur.append(r); size += 9 + len(r[5])
    if cur:
        out.append(cur)
    return out


def quake_fields(rev=1):
    f = [LayoutField(3 * (2 * c + s), 3, True, False, c, 0, -6, 0, 0, b"acc", b"m/s2")
         for c in range(3) for s in range(2)]
    f += [LayoutField(18 + 2 * (2 * c + s), 2, True, False, 3 + c, c + 1 if rev == 1 else 0)
          for c in range(3) for s in range(2)]
    return f


def quake_records(n, rng, t0=T0):
    pls = []
    x, v = [0.0] * 3, [0.0] * 3
    for i in range(n):
        pl = bytearray(32)
        for c in range(3):
            for s in range(2):
                v[c] = 0.98 * v[c] + rng.gauss(0, 30)
                x[c] = 0.995 * x[c] + v[c]
                val = int(x[c])
                pl[3 * (2 * c + s):3 * (2 * c + s) + 3] = (val & 0xFFFFFF).to_bytes(3, "little")
                pl[18 + 2 * (2 * c + s):20 + 2 * (2 * c + s)] = ((val + rng.randint(-2, 2)) & 0xFFFF).to_bytes(2, "little")
        pls.append(bytes(pl))
    return [(t0 + i // 128, i % 128, 0, 0, 0, pl) for i, pl in enumerate(pls)]


def scenarios(seed=1):
    """(name, header, ops): ops are ('seg', records) · ('cfg', from second, sections) ·
    ('ev', lines)."""
    rng = random.Random(seed)
    out = []
    station = hmc.station_section(b"Heimdall test", 501_234_567, 143_456_789, 312)
    # a Quake, a config that changes nothing HCC reads, then one that changes its layout
    secs = [station, hmc.unit_section(1, 5, 1, 2, 3, 128),
            hmc.layout_section(1, 5, 1, 0, quake_fields(1)), hmc.time_section(18)]
    recs = quake_records(128 * 40, rng)
    ops = [("seg", c) for c in chunks(recs[:128 * 16])]
    ops.append(("cfg", T0 + 16, [hmc.time_section(18, 1, T0 + 86400)]))
    ops += [("seg", c) for c in chunks(recs[128 * 16:128 * 28])]
    ops.append(("cfg", T0 + 28, [hmc.layout_section(1, 5, 2, 0, quake_fields(2))]))
    ops += [("seg", c) for c in chunks(recs[128 * 28:])]
    out.append(("Quake, two configs", Header(0x52, 7, T0, 0x123456, 0, secs), ops))
    # a Palatine: blocks, one module changing
    blocks = []
    for i in range(400):
        module = i % 4
        data = bytes([0x10, 0x20, 0, 5, 1, 0xF4, 0, 0]) if module != 2 else \
            bytes([0x10, 0x20, 0, 5, (i >> 8) & 0xFF, i & 0xFF, 0, 0])
        blocks.append((T0 + 30 * i, 0, 1, 0, 1, bytes([module, len(data)]) + data))
    pal = [station, hmc.unit_section(1, 6, 1, 1, 2, 128), hmc.layout_section(1, 6, 1, 1, [])]
    out.append(("Palatine blocks", Header(0x61, 0, T0, 0x123456, 0, pal),
                [("seg", c) for c in chunks(blocks, seconds=3600)]))
    # an Argus NUMBER, minis at positions 0, 2 and 3
    mini = [LayoutField(0, 2, True, False, 0), LayoutField(2, 2, True, False, 1),
            LayoutField(4, 2, True, False, 2), LayoutField(6, 2, False, False, 3)]
    arg = [hmc.unit_section(1, 3, 2, 1, 4, 128, (0x12, 0, 0x13, 0x31)),
           hmc.layout_section(2, 1, 1, 0, mini), hmc.layout_section(2, 3, 1, 0, mini[:2])]
    ar = []
    for i in range(128 * 12):
        pl = bytearray(32)
        for pos in (0, 2, 3):
            for c in range(3):
                v = int(2000 * __import__("math").sin(i / 40 + c + pos)) + rng.randint(-3, 3)
                pl[8 * pos + 2 * c:8 * pos + 2 * c + 2] = (v & 0xFFFF).to_bytes(2, "little")
        ar.append((T0 + i // 128, i % 128, 0, 0, 0, bytes(pl)))
    out.append(("Argus NUMBER", Header(0x31, 2, T0, 0x123456, 0, arg), [("seg", c) for c in chunks(ar)]))
    # the event log, address 00
    lines = [(T0 + 7 * i, rng.choice([0, 0, 0x90]), rng.randrange(1, 40), rng.randrange(1 << 16),
              rng.randrange(1 << 32)) for i in range(300)]
    out.append(("event log", Header(0, 1, T0, 0x123456, 0, [station]),
                [("ev", lines[i:i + 50]) for i in range(0, 300, 50)]))
    # noise until the file is full — every segment raw, the description deep
    noise = [(T0 + i, i & 0x7F, 0, 0, 0, bytes(rng.randrange(256) for _ in range(32))) for i in range(40000)]
    out.append(("full file", Header(0x77, 3, T0, 0x123456, 0, [hmc.unit_section(1, 7, 1, 3, 1, 128)]),
                [("seg", noise[i:i + 30]) for i in range(0, 40000, 30)]))
    return out


def write(header, ops):
    w = hmc.Writer(header)
    kept = []
    for op in ops:
        ok = (w.segment(op[1]) if op[0] == "seg" else w.events(op[1]) if op[0] == "ev"
              else w.config(op[1], op[2]))
        if not ok:
            break
        kept.append(op)
    return w, kept


def main():
    assert hmc.path(0x52, T0) == "52/20261001/120000.hmc", hmc.path(0x52, T0)
    assert hmc.path(0xF8, 0) == "F8/19700101/000000.hmc"
    assert hmc.cobs_decode(hmc.cobs_encode(bytes(range(1, 255)))) == bytes(range(1, 255))
    for n in (0, 1, 253, 254, 255, 508, 1000):
        b = bytes(random.Random(n).choice([0, 1, 7, 255]) for _ in range(n))
        e = hmc.cobs_encode(b)
        assert 0 not in e and hmc.cobs_decode(e) == b and len(e) <= n + n // 254 + 1, n
    assert hmc.parse_entry(bytes(16)) is None
    for name, header, ops in scenarios():
        w, kept = write(header, ops)
        r = hmc.Reader(w.image)
        assert r.header.address == header.address and r.header.sections == header.sections, name
        got = list(r.objects())
        assert len(got) == len(kept), (name, len(got), len(kept))
        raw = coded = 0
        for op, (t, obj, recs) in zip(kept, got):
            if op[0] == "cfg":
                assert t == hmc.CFG and obj["from"] == op[1] and obj["sections"] == op[2], name
            else:
                assert t == hmc.SEG and recs == list(op[1]), name
                raw += 20 + 4 + (sum(9 + len(x[5]) for x in op[1]) if op[0] == "seg" else 12 * len(op[1]))
                coded += 20 + 4 + len(obj["body"])
        # a second found by the description
        sec = kept[len(kept) // 2][1][-1][0] if kept[len(kept) // 2][0] != "cfg" else kept[len(kept) // 2][1]
        assert r.entries[r.find(sec)][0] >= sec and (r.find(sec) == 0 or r.entries[r.find(sec) - 1][0] < sec)
        full = "full" if len(kept) < len(ops) else "room"
        print(f"{name:20s} {len(kept):5d} objects, {raw:8d} B raw -> {coded:8d} B written "
              f"({coded / raw * 100:5.1f} %), {full}")
        # one object damaged: read as damaged, the rest read on
        img = bytearray(w.image)
        off = r.entries[1][1]
        img[off + 3] ^= 0x40
        res = list(hmc.Reader(img).objects())
        assert res[1][2] == "damaged" or res[1][0] == 0, name
        assert all(a[2] == b[2] for k, (a, b) in enumerate(zip(res, got)) if k != 1 and not
                   (got[1][0] == hmc.CFG and k > 1)), name
        # the description lost: the zeros give every object back
        img = bytearray(w.image)
        img[len(img) - 16 * len(r.entries):] = bytes(16 * len(r.entries))
        assert hmc.Reader(img).entries == []
        found = list(hmc.scan(img, r.header.length))
        assert [t for t, _ in found] == [t for t, _, _ in got], name
    print("every file reads back")


if __name__ == "__main__":
    main()

"""The cases HCC.md names, on synthetic records: every one round-trips byte for byte."""
import os, random, sys
sys.path.insert(0, os.path.join(os.path.dirname(__file__), ".."))
import hcc, steim2
from hcc import Field, compose_layout, argus_layout

random.seed(1)

def frames(payloads, second0=1_700_000_000, kind=0, status=0, fps=128):
    out = []
    for i, pl in enumerate(payloads):
        out.append((second0 + i // fps, i % fps, kind, status, 0, bytes(pl)))
    return out

def quake_layout():
    # three axes of a 24-bit accelerometer, two samples a payload, plus a second sensor's three
    # axes coded against the first — Quake's reference channel
    f = []
    off = 0
    for ch in range(3):
        for s in range(2):
            f.append(Field(off, 3, True, False, ch, 0)); off += 3
    for ch in range(3):
        for s in range(2):
            f.append(Field(off, 2, True, False, 3 + ch, ch + 1)); off += 2
    return compose_layout(f)

def quake_payloads(n, amp=2000, event_at=None):
    pls = []
    x = [0.0]*3; v = [0.0]*3
    for i in range(n):
        pl = bytearray(32); off = 0
        for ch in range(3):
            for s in range(2):
                v[ch] = 0.98*v[ch] + random.gauss(0, 30)
                x[ch] = 0.995*x[ch] + v[ch]
                val = int(x[ch]) + (int(amp*random.gauss(0, 1)) if event_at is not None and i > event_at else 0)
                pl[off:off+3] = (val & 0xFFFFFF).to_bytes(3, "little"); off += 3
        for ch in range(3):
            for s in range(2):
                base = int.from_bytes(pl[3*(2*ch+s):3*(2*ch+s)+3], "little", signed=True)
                val = base + random.randint(-2, 2)
                pl[off:off+2] = (val & 0xFFFF).to_bytes(2, "little"); off += 2
        pl[30:32] = bytes(random.choice([0, 0, 0, 1]) for _ in range(2))
        pls.append(pl)
    return pls

def check(records, layout, name):
    s = hcc.encode(records, layout)
    back = hcc.decode(s, layout)
    assert back == list(records), name
    raw = hcc.raw_length(records)
    print(f"{name:34s} {raw:7d} B raw -> {len(s):6d} B HCC  ({len(s)/raw*100:5.1f} %)")
    return s

# a Quake under ALL, 8 s
check(frames(quake_payloads(1024)), quake_layout(), "Quake ALL 8 s")
# a Quake event coded against its reference channel
check(frames(quake_payloads(512, event_at=256)), quake_layout(), "Quake event, reference channel")
# a Palatine under CHANGE: block records, one module changing and the rest held
blocks = []
sec = 1_700_000_000
for i in range(200):
    module = i % 4; length = 8
    data = bytes([0x10, 0x20, 0x00, 0x05, 0x01, 0xF4, 0x00, 0x00]) if module != 2 else \
           bytes([0x10, 0x20, 0x00, 0x05, (i >> 8) & 0xFF, i & 0xFF, 0x00, 0x00])
    blocks.append((sec + i, 0, 1, 0, 1, bytes([module, length]) + data))
check(blocks, compose_layout([]), "Palatine CHANGE blocks")
# a Tesla under NONZERO with a burst: kind 0 frames interleaved with kind 2 (event) frames
tes = frames(quake_payloads(256))
tes += [(1_700_000_002, 5, 2, 0, 0, bytes(random.randint(0, 255) for _ in range(32))) for _ in range(20)]
check(tes, quake_layout(), "Tesla NONZERO with a burst")
# a filler run with its edges: status 3 frames, zero payloads
fill = frames([bytearray(32)] * 300, status=3)
check(fill, quake_layout(), "filler run")
# a 256-row unit with two samples to a payload — the Quake layout already has two
# an LPC series that wraps its field's range
wrap = []
for i in range(600):
    pl = bytearray(32)
    v = (i * 90000) & 0xFFFFFF
    pl[0:3] = v.to_bytes(3, "little"); pl[3:6] = v.to_bytes(3, "little")
    wrap.append(pl)
check(frames(wrap), quake_layout(), "LPC series wrapping its range")
# an Argus NUMBER with an empty position
mini = [Field(0, 2, True, False, 0, 0), Field(2, 2, True, False, 1, 0), Field(4, 2, True, False, 2, 0), Field(6, 2, False, False, 3, 0)]
lay = argus_layout([mini, None, mini, mini])
arg = []
for i in range(400):
    pl = bytearray(32)
    for pos in (0, 2, 3):
        for ch in range(3):
            v = int(2000*__import__("math").sin(i/40 + ch + pos)) + random.randint(-3, 3)
            pl[8*pos+2*ch:8*pos+2*ch+2] = (v & 0xFFFF).to_bytes(2, "little")
        pl[8*pos+6:8*pos+8] = (i & 0xFFFF).to_bytes(2, "little")
    arg.append(pl)
check(frames(arg), lay, "Argus NUMBER, empty position")
# a TYPE with no layout
check(frames(quake_payloads(256)), compose_layout([]), "TYPE with no layout")
# a late repaired frame
late = frames(quake_payloads(300))
late.append((1_700_000_000, 7, 0, 1, 0, late[7][5]))
check(late, quake_layout(), "late repaired frame")
# Steim-2 round trip and size on a series
xs = [int(5000*__import__("math").sin(i/30)) + random.randint(-20, 20) for i in range(4096)]
fr = steim2.encode(xs); assert steim2.decode(fr, len(xs)) == xs
print(f"Steim-2 on 4096 samples: {64*len(fr)} B = {64*len(fr)*8/len(xs):.2f} bits/sample")
print("all round-trips ok")

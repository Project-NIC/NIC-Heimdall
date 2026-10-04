"""A damaged stream fails cleanly: the decoder raises ValueError and nothing else, in bounded time.

HMC's CRC says a segment is whole before HCC sees it, so a damaged stream is a coder fault and
rare — but a reader of an old archive must still say "damaged" and read on, never crash, hang or
exhaust memory on one (HCC.md, *Decoding*). Every stream here is cut, extended, bit-flipped,
overwritten or invented, and each case is reported with what reproduces it.
"""
import math, os, random, signal, sys, time
sys.path.insert(0, os.path.join(os.path.dirname(__file__), ".."))
import hcc
from hcc import Field, compose_layout, argus_layout

SEED = int(os.environ.get("HCC_FUZZ_SEED", "1"))
ROUNDS = int(os.environ.get("HCC_FUZZ_ROUNDS", "400"))
LIMIT_S = 5.0          # one decode of a few kB never needs this long
rng = random.Random(SEED)


def frames(payloads, second0=1_700_000_000, kind=0, status=0, fps=128):
    return [(second0 + i // fps, i % fps, kind, status, 0, bytes(pl)) for i, pl in enumerate(payloads)]


def wave(n, chans=3, width=2, amp=3000):
    out = []
    for i in range(n):
        pl = bytearray(32)
        for ch in range(chans):
            v = int(amp * math.sin(i / 25 + ch)) + rng.randint(-5, 5)
            pl[width * ch:width * ch + width] = (v & ((1 << 8 * width) - 1)).to_bytes(width, "little")
        out.append(pl)
    return out


def cases():
    """(name, layout, records): every part of the stream — headers, LPC, the reference channel,
    the OTHER table, block groups — appears in at least one."""
    acc = [Field(3 * ch, 3, True, False, ch, 0) for ch in range(3)]
    acc += [Field(9 + 2 * ch, 2, True, False, 3 + ch, ch + 1) for ch in range(3)]
    quake = compose_layout(acc)
    out = [("quake", quake, frames(wave(200, 3, 3, 200000)))]
    mini = [Field(0, 2, True, False, 0, 0), Field(2, 2, True, False, 1, 0), Field(4, 2, True, False, 2, 0)]
    out.append(("argus", argus_layout([mini, None, mini, mini]), frames(wave(150))))
    blocks = [(1_700_000_000 + i, 0, 1, 0, 1, bytes([i % 3, 4]) + bytes([1, 2, i & 0xFF, 0])) for i in range(60)]
    out.append(("blocks", compose_layout([]), blocks))
    mixed = frames(wave(80), kind=0)
    mixed += [(1_700_000_001, 3, 2, 0, 0, bytes(rng.randrange(256) for _ in range(32))) for _ in range(10)]
    mixed += blocks[:10]
    out.append(("mixed", quake, mixed))
    out.append(("filler", quake, frames([bytearray(32)] * 120, status=3)))
    return out


class Timeout(Exception):
    pass


def _alarm(signum, frame):
    raise Timeout()


def decode(stream, layout):
    """('ok', records) · ('damaged', message) · ('fault', the exception) — within LIMIT_S."""
    use_alarm = hasattr(signal, "setitimer")
    if use_alarm:
        signal.signal(signal.SIGALRM, _alarm)
        signal.setitimer(signal.ITIMER_REAL, LIMIT_S)
    t0 = time.monotonic()
    try:
        return "ok", hcc.decode(stream, layout)
    except ValueError as e:
        return "damaged", str(e)
    except Timeout:
        return "fault", Timeout(f"no answer in {LIMIT_S} s")
    except Exception as e:          # anything but ValueError is the defect this test exists for
        return "fault", e
    finally:
        if use_alarm:
            signal.setitimer(signal.ITIMER_REAL, 0)
        elif time.monotonic() - t0 > LIMIT_S:
            raise AssertionError(f"a decode took {time.monotonic() - t0:.1f} s")


faults = []


def expect(name, how, stream, layout, must_be_damaged):
    verdict, what = decode(stream, layout)
    if verdict == "fault" or (must_be_damaged and verdict != "damaged"):
        faults.append(f"{name}, {how}: {verdict} — {what!r}" if verdict == "fault"
                      else f"{name}, {how}: decoded, but a cut or extended stream must read as damaged")
    return verdict


tally = {"ok": 0, "damaged": 0, "fault": 0}
for name, layout, records in cases():
    good = hcc.encode(records, layout)
    assert hcc.decode(good, layout) == records, name
    # cut short — at every byte boundary: always damaged, the last byte always carries data
    for cut in range(len(good)):
        tally[expect(name, f"cut to {cut} B", good[:cut], layout, True)] += 1
    # extended by one to sixteen bytes: always damaged, bits left over
    for extra in range(1, 17):
        tally[expect(name, f"{extra} B appended", good + bytes(rng.randrange(256) for _ in range(extra)),
                     layout, True)] += 1
    # one bit flipped, a byte overwritten, a run zeroed or set
    for r in range(ROUNDS):
        s = bytearray(good)
        kind = r % 4
        p = rng.randrange(len(s))
        if kind == 0:
            s[p] ^= 1 << rng.randrange(8); how = f"bit {rng.randrange(8)} of byte {p} flipped"
        elif kind == 1:
            s[p] = rng.randrange(256); how = f"byte {p} overwritten"
        elif kind == 2:
            n = rng.randint(1, 8); s[p:p + n] = bytes(len(s[p:p + n])); how = f"{n} B zeroed at {p}"
        else:
            n = rng.randint(1, 8); s[p:p + n] = b"\xff" * len(s[p:p + n]); how = f"{n} B set at {p}"
        tally[expect(name, how + f" (seed {SEED}, round {r})", bytes(s), layout, False)] += 1
# streams that were never HCC
layout = cases()[0][1]
for r in range(ROUNDS):
    junk = bytes(rng.randrange(256) for _ in range(rng.randint(0, 64)))
    tally[expect("junk", f"{len(junk)} random B (seed {SEED}, round {r})", junk, layout, False)] += 1
# a count no coder writes: 70 B that once claimed a million records and took the decoder
# seconds and hundreds of MB to refuse — CONST headers and CONST data cost no bits per record
for n in (65_537, 1 << 20, 1 << 32):          # HCC.md: a count above 65 536 is damaged
    bw = hcc.BitWriter()
    bw.gamma(n)
    bw.write(0, 3); bw.write(1_700_000_000, 32)
    for _ in range(4):
        bw.write(0, 3); bw.write(0, 8)
    for _ in range(40):
        bw.write(0, 3); bw.write(0, 8)
    tally[expect("count", f"γ({n}) over CONST series", bw.bytes(), compose_layout([]), True)] += 1
for fill in (b"", b"\x00", b"\x00" * 64, b"\xff" * 64, b"\x80" + b"\x00" * 7):
    tally[expect("junk", f"{fill[:4].hex()}… × {len(fill)} B", fill, layout, False)] += 1

print(f"{sum(tally.values())} damaged streams: {tally['damaged']} read as damaged, "
      f"{tally['ok']} decoded to other records, {tally['fault']} faults")
if faults:
    print("\n".join(faults[:20]) + (f"\n… and {len(faults) - 20} more" if len(faults) > 20 else ""))
    sys.exit(1)
print("every damaged stream failed cleanly")

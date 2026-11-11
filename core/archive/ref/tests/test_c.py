"""The C reference against the Python: the same bytes from the same records, the same records
from the same bytes, and the same verdict on every damaged stream (HCC.md, *What it is tested
against*). Builds ../c/ with $CC (cc by default) into a temporary directory; `HCC_C_SEED` and
`HCC_C_ROUNDS` widen the random part."""
import math, os, random, shlex, struct, subprocess, sys, tempfile

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(HERE, ".."))
sys.path.insert(0, HERE)
import hcc, hmc
from hcc import Field, compose_layout, argus_layout
import test_hcc, test_damaged, test_hmc

SEED = int(os.environ.get("HCC_C_SEED", "1"))
ROUNDS = int(os.environ.get("HCC_C_ROUNDS", "60"))
rng = random.Random(SEED)


def build():
    src = os.path.join(HERE, "..", "c")
    out = os.path.join(tempfile.mkdtemp(prefix="hcc-c-"), "hcc_tool")
    subprocess.run(shlex.split(os.environ.get("CC", "cc")) + ["-std=c99", "-O2", "-Wall", "-Wextra", "-Wpedantic",
                    "-Werror", "-ffp-contract=off", "-o", out, os.path.join(src, "hcc.c"),
                    os.path.join(src, "hmc.c"), os.path.join(src, "hcc_tool.c"), "-lm"], check=True)
    return out


# ---------------------------------------------------------------- the wire to the tool

def ser_fields(fs):
    b = bytearray([len(fs)])
    for f in fs:
        b += bytes([f.offset, f.width, int(f.signed), int(f.big_endian), f.channel, f.ref])
    return bytes(b)


def ser_records(rs):
    b = bytearray(struct.pack("<I", len(rs)))
    for sec, fr, kind, st, form, data in rs:
        b += struct.pack("<IBBBBH", sec, fr, kind, st, form, len(data)) + data
    return bytes(b)


class Tool:
    def __init__(self, path):
        self.path, self.req, self.what = path, [], []
        self.events = False

    def encode(self, layout, records):
        self.req.append(b"E" + ser_fields(layout) + ser_records(records)); self.what.append("E")

    def decode(self, layout, stream):
        self.req.append(b"D" + ser_fields(layout) + struct.pack("<I", len(stream)) + stream)
        self.what.append("D")

    def compose(self, fields):
        self.req.append(b"L" + ser_fields(fields)); self.what.append("L")

    def argus(self, positions):
        self.req.append(b"A" + bytes([len(positions)]) + b"".join(ser_fields(p or []) for p in positions))
        self.what.append("L")

    def write_file(self, header, ops):
        b = bytearray(b"W" + struct.pack("<BHIIB", header.address, header.sequence, header.first_second,
                                          header.station, header.size))
        sb = hmc.sections_bytes(header.sections)
        b += struct.pack("<I", len(sb)) + sb
        for op in ops:
            if op[0] == "seg":
                b += b"S" + ser_records(op[1])
            elif op[0] == "ev":
                b += b"V" + struct.pack("<I", len(op[1])) + b"".join(struct.pack("<IBBHI", *ln) for ln in op[1])
            else:
                sb = hmc.sections_bytes(op[2])
                b += b"C" + struct.pack("<II", op[1], len(sb)) + sb
        self.req.append(bytes(b) + b"Z"); self.what.append("W")

    def read_file(self, image):
        self.req.append(b"R" + struct.pack("<I", len(image)) + bytes(image)); self.what.append("R")

    def select(self, rule, blocks, frames=None, records=None):
        b = bytearray(b"K" + struct.pack("<BBHB", rule.mode, rule.n, rule.s, len(rule.modules))
                      + bytes(rule.modules) + bytes([int(blocks)]))
        if frames is not None:
            b += b"\x00" + struct.pack("<I", len(frames))
            for sec, fr, kind, st, pl in frames:
                b += struct.pack("<IBBB", sec, fr, kind, st) + pl
        else:
            b += b"\x01" + ser_records(records)
        self.req.append(bytes(b)); self.what.append("K")

    def hmc_path(self, address, second):
        self.req.append(b"P" + struct.pack("<BI", address, second)); self.what.append("P")

    def run(self):
        out = subprocess.run([self.path], input=b"".join(self.req), capture_output=True, check=True).stdout
        at, answers = 0, []
        for w in self.what:
            assert out[at:at + 1] == w.encode(), "the tool answered out of step"
            at += 1
            if w == "E":
                st, n = out[at], struct.unpack_from("<I", out, at + 1)[0]
                answers.append((st, out[at + 5:at + 5 + n])); at += 5 + n
            elif w == "D":
                st = out[at]; at += 1
                if st != 0:
                    answers.append((st, None)); continue
                n = struct.unpack_from("<I", out, at)[0]; at += 4
                recs = []
                for _ in range(n):
                    sec, fr, kind, s, form, ln = struct.unpack_from("<IBBBBH", out, at); at += 10
                    recs.append((sec, fr, kind, s, form, out[at:at + ln])); at += ln
                answers.append((0, recs))
            elif w == "W":
                st, kept, n = out[at], *struct.unpack_from("<II", out, at + 1)
                answers.append((st, kept, out[at + 9:at + 9 + n])); at += 9 + n
            elif w == "K":
                n = struct.unpack_from("<I", out, at)[0]; at += 4
                recs = []
                for _ in range(n):
                    sec, fr, kind, s_, form, ln = struct.unpack_from("<IBBBBH", out, at); at += 10
                    recs.append((sec, fr, kind, s_, form, out[at:at + ln])); at += ln
                answers.append(recs)
            elif w == "P":
                n = out[at]; answers.append(out[at + 1:at + 1 + n].decode()); at += 1 + n
            elif w == "R":
                st = out[at]; at += 1
                if st:
                    answers.append((st, None)); continue
                ne = struct.unpack_from("<I", out, at)[0]; at += 4
                objs = []
                for _ in range(ne):
                    k = out[at]; at += 1
                    if k == 0:
                        objs.append(("damaged",))
                    elif k == 2:
                        fr, ln = struct.unpack_from("<II", out, at); at += 8
                        objs.append(("cfg", fr, out[at:at + ln])); at += ln
                    else:
                        first, last, codec, rs = struct.unpack_from("<IIBB", out, at); at += 10
                        if rs == 5:
                            objs.append(("seg", first, last, codec, None)); continue
                        if rs != 0:
                            objs.append(("seg", first, last, codec, "damaged")); continue
                        n = struct.unpack_from("<I", out, at)[0]; at += 4
                        recs = []
                        for _ in range(n):
                            if self.events:
                                recs.append(struct.unpack_from("<IBBHI", out, at)); at += 12
                            else:
                                sec, fr_, kind, s_, form, ln = struct.unpack_from("<IBBBBH", out, at); at += 10
                                recs.append((sec, fr_, kind, s_, form, out[at:at + ln])); at += ln
                        objs.append(("seg", first, last, codec, recs))
                answers.append((0, objs))
            else:
                n = out[at]; at += 1
                fs = []
                for _ in range(n):
                    o, w_, sg, be, ch, ref = out[at:at + 6]; at += 6
                    fs.append(Field(o, w_, bool(sg), bool(be), ch, ref))
                answers.append(fs)
        self.req, self.what = [], []
        return answers


# ---------------------------------------------------------------- random segments

def random_case(i):
    """A layout of 1–4 byte fields, signed or not, either byte order, some carried by an earlier
    channel; values walking, held, noisy, at the extremes, wrapping their range; frames of other
    kinds and blocks among them; seconds anywhere in 32 bits."""
    fields, off, ch = [], 0, 0
    while off < 32 and rng.random() < 0.85:
        w = rng.choice([1, 2, 3, 4])
        nf = rng.choice([1, 1, 2, 3])
        if off + w * nf > 32:
            break
        sgn, be = rng.random() < 0.5, rng.random() < 0.3
        ref = rng.randrange(ch) + 1 if ch and rng.random() < 0.35 else 0
        for _ in range(nf):
            fields.append(Field(off, w, sgn, be, ch, ref)); off += w
        ch += 1
    layout = compose_layout(fields)
    n = rng.choice([1, 2, 3, 4, 9, 31, 32, 33, 64, 65, 200, 500])
    style = rng.choice(["walk", "held", "noise", "extreme", "sine", "wrap", "slow"])
    state = {}
    payloads = []
    for k in range(n):
        pl = bytearray(rng.randrange(256) if style == "noise" else 0 for _ in range(32))
        vals = {}
        for f in layout:
            bits = 8 * f.width
            top = (1 << bits) - 1
            key = (f.channel, f.offset)
            if style == "walk":
                v = state.get(key, rng.randrange(top + 1)) + rng.randint(-50, 50)
            elif style == "held":
                v = state.get(key, rng.randrange(top + 1))
            elif style == "noise":
                v = rng.randrange(top + 1)
            elif style == "extreme":
                v = rng.choice([0, 1, top, top - 1, 1 << (bits - 1), (1 << (bits - 1)) - 1])
            elif style == "sine":
                v = int((top // 2) * math.sin(k / (5 + f.channel)) * 0.99) + rng.randint(-3, 3)
            elif style == "wrap":
                v = state.get(key, 0) + (top // 7)
            else:
                v = state.get(key, rng.randrange(top + 1)) + (1 if rng.random() < 0.1 else 0)
            if f.ref and (f.ref - 1) in vals and rng.random() < 0.9:
                v = vals[f.ref - 1] + rng.randint(-2, 2)
            v &= top
            state[key] = v
            vals.setdefault(f.channel, v)
            pl[f.offset:f.offset + f.width] = v.to_bytes(f.width, "big" if f.big_endian else "little")
        payloads.append(bytes(pl))
    sec0 = rng.choice([0, 1_700_000_000, (1 << 32) - 1 - n, rng.randrange(1 << 32)])
    records = []
    for k, pl in enumerate(payloads):
        sec = (sec0 + k // rng.choice([1, 8, 128])) & 0xFFFFFFFF if style != "noise" else rng.randrange(1 << 32)
        r = rng.random()
        if r < 0.08:
            records.append((sec, k & 0xFF, rng.randrange(1, 9), rng.randrange(256), 0, pl))
        elif r < 0.14:
            module, ln = rng.randrange(6), rng.choice([0, 1, 4, 8, 17])
            records.append((sec, 0, 1, 0, 1, bytes([module, ln]) + bytes(rng.randrange(256) for _ in range(ln))))
        else:
            records.append((sec, k & 0xFF, 0, rng.choice([0, 0, 0, 3]), 0, pl))
    return f"random {i} ({style}, n {n})", layout, records


def mutations(good):
    """Cut short — at every byte of a short stream, at 200 places of a long one — extended, a bit
    flipped, a byte overwritten, a run zeroed or set."""
    cuts = range(len(good)) if len(good) <= 400 else sorted(rng.sample(range(len(good)), 200))
    for cut in cuts:
        yield good[:cut]
    for extra in (1, 2, 5, 16):
        yield good + bytes(rng.randrange(256) for _ in range(extra))
    for r in range(ROUNDS):
        s = bytearray(good)
        p = rng.randrange(len(s))
        kind = r % 4
        if kind == 0:
            s[p] ^= 1 << rng.randrange(8)
        elif kind == 1:
            s[p] = rng.randrange(256)
        else:
            n = rng.randint(1, 8)
            s[p:p + n] = (b"\x00" if kind == 2 else b"\xff") * len(s[p:p + n])
        yield bytes(s)


def frame_stream(n):
    """A unit's frames as the head sees them: payloads held, changing, zero, or a ModBus ring of
    blocks — some changing, one now and then running past the payload; kinds and states with
    edges; a second of 128 frames or a few, and a late frame now and then."""
    style = rng.choice(["held", "change", "zero", "blocks"])
    sec, fr, kind, st = rng.randrange(1 << 31), 0, 0, 0
    mods = [(rng.randrange(1, 40), rng.randrange(1, 9)) for _ in range(rng.randint(1, 6))]
    vals = {(m, ln): bytes(rng.randrange(256) for _ in range(ln)) for m, ln in mods}
    pl = bytes(rng.randrange(256) for _ in range(32))
    out = []
    for i in range(n):
        if rng.random() < 0.3:
            sec += rng.choice([1, 1, 2, 7, 61, 3600])
            fr = 0
        else:
            fr = (fr + 1) & 0x7F
        if rng.random() < 0.05:
            kind = rng.choice([0, 0, 0, 1, 2, 5, 6, 8, 9])
        if rng.random() < 0.05:
            st = rng.randrange(4)
        if style == "held":
            p = pl
        elif style == "change":
            if rng.random() < 0.2:
                pl = bytes(rng.randrange(256) for _ in range(32))
            p = pl
        elif style == "zero":
            p = bytes(32) if rng.random() < 0.7 else bytes(rng.randrange(256) for _ in range(32))
        else:
            for m, ln in mods:
                if rng.random() < 0.15:
                    vals[(m, ln)] = bytes(rng.randrange(256) for _ in range(ln))
            b = bytearray()
            for m, ln in rng.sample(mods, len(mods)):
                if len(b) + 2 + ln <= 32:
                    b += bytes([m, ln]) + vals[(m, ln)]
            if rng.random() < 0.03 and len(b) < 30:
                b += bytes([7, 40])
            p = bytes(b) + bytes(32 - len(b)) if len(b) < 32 else bytes(b[:32])
        s_ = sec - rng.randrange(1, 5) if rng.random() < 0.02 and sec > 5 else sec
        out.append((s_, fr, kind, st, p))
    return style == "blocks", out


def random_rule(send=False):
    mode = rng.choice([hmc.ALL, hmc.CHANGE, hmc.DECIMATE, hmc.INTERVAL, hmc.NONZERO, hmc.NONE])
    n = rng.choice([0, 1, 2, 4, 32]) if mode == hmc.DECIMATE else 0
    s = rng.choice([0, 1, 7, 60, 3600]) if mode in (hmc.CHANGE, hmc.NONZERO, hmc.INTERVAL) else 0
    mods = tuple(rng.sample(range(1, 40), rng.randint(1, 4))) if send and rng.random() < 0.5 else ()
    return hmc.Rule(mode, n, s, mods)


def py_read(image):
    """The Python reader's view of a file, in the C tool's terms."""
    try:
        r = hmc.Reader(image)
    except ValueError:
        return 1, None
    out = []
    for t, obj, recs in r.objects():
        if t == 0:
            out.append(("damaged",))
        elif t == hmc.CFG:
            out.append(("cfg", obj["from"], hmc.sections_bytes(obj["sections"])))
        else:
            out.append(("seg", obj["first"], obj["last"], obj["codec"],
                        [tuple(x) for x in recs] if isinstance(recs, list) else recs))
    return 0, out


def py_decode(stream, layout):
    try:
        return 0, hcc.decode(stream, layout)
    except ValueError:
        return 1, None


def main():
    tool = Tool(build())
    fails = []

    # the layouts
    declared = [Field(3 * c, 3, True, False, c, 0) for c in range(3)] + \
               [Field(9 + 2 * c, 2, True, False, 3 + c, c + 1) for c in range(3)]
    mini = [Field(0, 2, True, False, 0, 0), Field(2, 2, True, False, 1, 0),
            Field(4, 2, True, False, 2, 1), Field(6, 2, False, True, 3, 0)]
    tool.compose(declared); tool.compose([])
    tool.argus([mini, None, mini, mini]); tool.argus([None, mini[:2], None, mini])
    want = [compose_layout(declared), compose_layout([]), argus_layout([mini, None, mini, mini]),
            argus_layout([None, mini[:2], None, mini])]
    for i, (got, w) in enumerate(zip(tool.run(), want)):
        if got != w:
            fails.append(f"layout {i}: the C composes {got}, the Python {w}")

    # the documents' cases, the damaged-stream cases and random segments: the same bytes
    named = test_hcc.cases()
    cases = named + test_damaged.cases() + [random_case(i) for i in range(ROUNDS)]
    streams = []
    for name, layout, records in cases:
        s = hcc.encode(records, layout)
        streams.append(s)
        tool.encode(layout, records)
        tool.decode(layout, s)
    answers = tool.run()
    raw = coded = 0
    for (name, layout, records), s, (est, cs), (dst, back) in zip(cases, streams, answers[0::2], answers[1::2]):
        raw += hcc.raw_length(records); coded += len(s)
        if est != 0 or cs != s:
            where = next((i for i, (a, b) in enumerate(zip(cs or b"", s)) if a != b), min(len(cs or b""), len(s)))
            fails.append(f"{name}: the C writes {len(cs or b'')} B (status {est}), the Python {len(s)} B; "
                         f"first difference at byte {where}")
        if dst != 0 or back != list(records):
            fails.append(f"{name}: the C reads its stream back {'wrong' if dst == 0 else 'as damaged'}")

    # damaged streams: the same verdict and, where one decodes, the same records
    checked = 0
    for (name, layout, records), s in list(zip(cases, streams))[len(named):len(named) + 5 + 15]:
        bad = list(mutations(s))
        for b in bad:
            tool.decode(layout, b)
        for b, (cst, crecs) in zip(bad, tool.run()):
            pst, precs = py_decode(b, layout)
            checked += 1
            if cst != pst or crecs != precs:
                fails.append(f"{name}, a damaged stream of {len(b)} B ({b[:12].hex()}…): the C says "
                             f"{'ok' if cst == 0 else cst}, the Python {'ok' if pst == 0 else 'damaged'}")
    junk = [bytes(rng.randrange(256) for _ in range(rng.randint(0, 64))) for _ in range(ROUNDS)]
    junk += [b"", b"\x00", b"\x00" * 64, b"\xff" * 64, b"\x80" + b"\x00" * 7]
    layout = test_damaged.cases()[0][1]
    for b in junk:
        tool.decode(layout, b)
    for b, (cst, crecs) in zip(junk, tool.run()):
        pst, precs = py_decode(b, layout)
        checked += 1
        if cst != pst or crecs != precs:
            fails.append(f"junk {b.hex()}: the C says {cst}, the Python {pst}")

    # HMC: the same file from the same writes, the same reading of every file and its damage
    files = test_hmc.scenarios(SEED)
    images = []
    for name, header, ops in files:
        w, kept = test_hmc.write(header, ops)
        images.append((name, header, w.image, len(kept)))
        tool.write_file(header, ops)
    for (name, header, image, kept), (st, ckept, cimage) in zip(images, tool.run()):
        if st != 0 or ckept != kept or cimage != image:
            where = next((i for i, (a, b) in enumerate(zip(cimage, image)) if a != b), None)
            fails.append(f"HMC {name}: the C writes {ckept} objects (status {st}), the Python {kept}; "
                         f"first difference at byte {where}")
    hmc_checked = 0
    for name, header, image, kept in images:
        variants = [image]
        r = hmc.Reader(image)
        for k in range(ROUNDS // 3):
            img = bytearray(image)
            e = rng.choice([x for x in r.entries if x])
            where = rng.choice([e[1] + rng.randrange(e[2] + 1), len(img) - 16 * rng.randrange(1, len(r.entries) + 2),
                                rng.randrange(r.header.length)])
            img[where] ^= 1 << rng.randrange(8)
            variants.append(bytes(img))
        tool.events = header.address == 0
        for v in variants:
            tool.read_file(v)
        for v, got in zip(variants, tool.run()):
            want = py_read(v)
            hmc_checked += 1
            if got != want:
                fails.append(f"HMC {name}: the C reads a file {'as written' if v is image else 'with a bit flipped'} "
                             f"differently from the Python")
    # the rules: the same records kept from the same frames, and the same sent from the same kept
    sel = []
    for _ in range(ROUNDS * 2):
        blocks, frames = frame_stream(rng.choice([1, 5, 50, 400]))
        rule = random_rule()
        kept = []
        py = hmc.Selector(rule, blocks)
        for f in frames:
            kept += py.frame(*f)
        send = random_rule(send=True)
        sp = hmc.Selector(send, blocks)
        sent = [r for k in kept for r in sp.record(k)]
        sel.append((rule, send, kept, sent))
        tool.select(rule, blocks, frames=frames)
        tool.select(send, blocks, records=kept)
    answers = tool.run()
    for (rule, send, kept, sent), ck, cs in zip(sel, answers[0::2], answers[1::2]):
        if ck != kept or cs != sent:
            fails.append(f"rules {rule} / {send}: the C keeps {len(ck)} and sends {len(cs)}, "
                         f"the Python {len(kept)} and {len(sent)}")
    print(f"{len(sel)} frame streams through a recording rule and a send rule by both")

    for sec in (0, test_hmc.T0, 951_782_400, 4_107_542_400, (1 << 32) - 1):
        tool.hmc_path(0x52, sec)
    for sec, got in zip((0, test_hmc.T0, 951_782_400, 4_107_542_400, (1 << 32) - 1), tool.run()):
        if got != hmc.path(0x52, sec):
            fails.append(f"HMC path of {sec}: the C {got}, the Python {hmc.path(0x52, sec)}")
    print(f"{len(files)} HMC files written by both and compared byte for byte; {hmc_checked} files, "
          f"whole and damaged, read by both")

    print(f"{len(cases)} segments, {raw} B raw -> {coded} B HCC ({coded / raw * 100:.1f} %): "
          f"the C and the Python streams compared byte for byte")
    print(f"{checked} damaged streams: the C's verdict and records against the Python's")
    if fails:
        print("\n".join(fails[:20]) + (f"\n… and {len(fails) - 20} more" if len(fails) > 20 else ""))
        sys.exit(1)
    print("the C and the Python agree")


if __name__ == "__main__":
    main()

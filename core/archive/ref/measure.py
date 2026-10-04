"""Measure HCC against Steim-2 on recorded series.

    python3 measure.py FILE [FILE ...] [--seconds 1,2,4,8,16,32] [--ref]

FILE is miniSEED (anything obspy reads) or IAGA-2002 text. Every trace is taken as one integer
series — a miniSEED trace as its counts, an IAGA-2002 column as nT × 100. Per segment length the
table gives bits per sample for Steim-2 and for HCC's series coder in each of its modes, and the
coder's own choice; with --ref, traces whose names differ only in their station are paired and
the second is coded against the first as a reference channel.
"""

from __future__ import annotations

import argparse
import math
import os
import sys

sys.path.insert(0, os.path.dirname(__file__))
import hcc  # noqa: E402
import steim2  # noqa: E402


def read_iaga2002(path):
    """IAGA-2002: the four columns after the header, as nT × 100; a missing value (99999) ends
    nothing, it is carried as the value before it so the series stays whole."""
    cols = None
    series = {}
    names = []
    with open(path, encoding="utf-8", errors="replace") as f:
        for line in f:
            if line.startswith(" ") or line.startswith("#"):
                if line.startswith("DATE"):
                    pass
                continue
            if line.startswith("DATE"):
                names = line.split()[3:7]
                cols = {n: [] for n in names}
                continue
            if cols is None:
                continue
            parts = line.split()
            if len(parts) < 7:
                continue
            for n, v in zip(names, parts[3:7]):
                x = float(v)
                if x >= 88888.0:
                    x = cols[n][-1] / 100.0 if cols[n] else 0.0
                cols[n].append(int(round(x * 100)))
    base = os.path.basename(path)
    return [(f"{base}:{n}", cols[n], 1.0 / 60.0 if "min" in base else 1.0) for n in names if cols]


def read_file(path):
    if path.lower().endswith((".min", ".sec", ".txt", ".iaga")):
        return read_iaga2002(path)
    from obspy import read
    st = read(path)
    out = []
    for tr in st:
        data = tr.data
        if data.dtype.kind == "f":
            data = (data * 1.0).round().astype("int64")
        out.append((tr.id, [int(x) for x in data], float(tr.stats.sampling_rate)))
    return out


def series_bits(xs, w, mode=None, order=None, signed=True, ref=None):
    """Bits HCC writes for a series under one mode (or the coder's own choice when mode is None)."""
    mask = (1 << w) - 1
    vals = [x & mask for x in xs]
    if mode is None:
        bw = hcc.BitWriter()
        hcc.write_series(bw, vals, w, signed, [y & mask for y in ref] if ref is not None else None)
        return bw.bit_length()
    if ref is not None:
        vals = [(x - y) & mask for x, y in zip(vals, ref)]
    for bits, m, payload in hcc._mode_bits(vals, w, signed):
        if m == mode and (order is None or m != 3 or payload[0] == order):
            return bits + (1 if ref is not None else 0)
    return None


def width_for(xs):
    m = max(abs(x) for x in xs) if xs else 0
    return max(8, 8 * math.ceil((m.bit_length() + 1) / 8))


def measure(name, xs, rate, seconds, ref=None):
    w = width_for(xs if ref is None else xs + ref)
    print(f"\n{name}: {len(xs)} samples at {rate:g} Hz, {w}-bit fields"
          + (" — coded against the reference" if ref is not None else ""))
    head = f"{'segment':>8s} {'Steim-2':>8s} {'order1':>8s} {'order2':>8s}" + "".join(
        f" {'LPC' + str(p):>7s}" for p in range(1, 9)) + f" {'HCC':>8s}"
    print(head)
    lmax = max(2, int(round(max(seconds) * rate)))
    common = (len(xs) // lmax) * lmax if len(xs) >= lmax else len(xs)
    xs = xs[:common]
    ref = ref[:common] if ref is not None else None
    for sec in seconds:
        L = max(2, int(round(sec * rate)))
        if L > len(xs):
            continue
        nseg = len(xs) // L
        tot = {"steim": 0, 1: 0, 2: 0, "hcc": 0}
        lpc = {p: 0 for p in range(1, 9)}
        lpc_ok = {p: True for p in range(1, 9)}
        n = 0
        for k in range(nseg):
            seg = xs[k * L:(k + 1) * L]
            rseg = ref[k * L:(k + 1) * L] if ref is not None else None
            n += L
            tot["steim"] += 8 * steim2.byte_length(seg)
            tot[1] += series_bits(seg, w, 1, ref=rseg)
            tot[2] += series_bits(seg, w, 2, ref=rseg)
            tot["hcc"] += series_bits(seg, w, None, ref=rseg)
            for p in range(1, 9):
                b = series_bits(seg, w, 3, p, ref=rseg)
                if b is None:
                    lpc_ok[p] = False
                else:
                    lpc[p] += b
        row = f"{sec:6g} s {tot['steim']/n:8.2f} {tot[1]/n:8.2f} {tot[2]/n:8.2f}"
        for p in range(1, 9):
            row += f" {lpc[p]/n:7.2f}" if lpc_ok[p] else f" {'—':>7s}"
        row += f" {tot['hcc']/n:8.2f}"
        print(row)


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("files", nargs="+")
    ap.add_argument("--seconds", default="1,2,4,8,16,32")
    ap.add_argument("--ref", action="store_true")
    ap.add_argument("--max", type=int, default=200_000, help="samples per trace at most")
    a = ap.parse_args()
    seconds = [float(s) for s in a.seconds.split(",")]
    traces = []
    for f in a.files:
        traces += read_file(f)
    traces = [(n, xs[:a.max], r) for n, xs, r in traces]
    for name, xs, rate in traces:
        measure(name, xs, rate, seconds)
    if a.ref:
        by_comp = {}
        for name, xs, rate in traces:
            parts = name.split(".")
            key = (parts[-1], rate) if len(parts) >= 4 else (name, rate)
            by_comp.setdefault(key, []).append((name, xs))
        for key, lst in by_comp.items():
            for i in range(1, len(lst)):
                a_name, a_xs = lst[0]
                b_name, b_xs = lst[i]
                m = min(len(a_xs), len(b_xs))
                measure(f"{b_name} against {a_name}", b_xs[:m], key[1], seconds, ref=a_xs[:m])


if __name__ == "__main__":
    main()

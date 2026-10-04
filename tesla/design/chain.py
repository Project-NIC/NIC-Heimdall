#!/usr/bin/env python3
"""
NIC-Tesla — one channel's chain, end to end: the clip field and the noise floor from the parts.

HARDWARE.md §0.2 gives the parts and §0.4 / §5 the two numbers they make: the field that clips the
converter and the field-referred noise floor. This computes both from the parts, frequency by
frequency, the way the document reasons about them:

  rod      EMF = jω · (N·A·µ_eff) · B, the loop closed by S1's virtual short through R_w + 2·Rs1
  S1       differential transimpedance Z_f1 = Rf1 ∥ Cf1; noise e_n × (1 + Z_f1/Z_src) with Z_src the
           coil ∥ its 126 pF, √(4kT·Rf1), i_n·Rf1, and the loop resistance's √(4kT·R)
  between  Cin in series with the 990 Ω per leg, the two-section ladder (R1·C1, R2·C2, C across the
           pair), Rg into S2's summing node — solved as the circuit, not as poles
  S2       ×Rf2 ∥ Cf2 on that current, and its own 17 nV/√Hz at the output
  ADC      51 nV/√Hz — 25,1 µV over the sinc4's 240 kHz noise bandwidth; the sinc4 shapes signal and
           this noise alike and the node's FIR flattens the whole response, so the floor is
           integrated 5–512 kHz after that flattening

Every figure is differential, as the document writes them. µ_eff and L come from rod_calc.py, so
the floor and the clip follow the rod actually built.

Needs nothing but Python. Run:  python3 chain.py
"""
import cmath
import math
import os
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import rod_calc

KB, T = 1.380649e-23, 300.0

PARTS = dict(               # HARDWARE.md §0.2, differential where the document says so
    Rf1=4420.0, Cf1=31e-12,                    # S1, differential
    Rs1=15.0,                                  # per leg, in series with the coil
    C_in=126.2e-12,                            # the winding's 125 pF and S1's 1,2 pF
    e_n=3.3e-9, i_n=0.5e-12,                   # THS4551
    Cin=33e-9, R1=270.0, C1=330e-12, R2=390.0, C2=330e-12, Rg=330.0,   # per leg; C1, C2 across the pair
    Rf2=2000.0, Cf2=68e-12,                    # S2, per leg
    e_s2=17e-9,                                # S2's own noise at its output
    e_adc=25.1e-6 / math.sqrt(240e3),          # the converter, input-referred density
    V_fs=4.096,                                # ±4,096 V differential
    f_lo=5e3, f_hi=512e3,
)


def par(a, b):
    return a * b / (a + b)


def stages(f, L, R_loop, p=PARTS):
    """(S1 transimpedance V/A, the S1 output → converter voltage gain, Z_f1, Z_src) at f."""
    w = 2 * math.pi * f
    jw = 1j * w
    Zf1 = par(p['Rf1'], 1 / (jw * p['Cf1']))
    Zcoil = R_loop + jw * L
    Zsrc = par(Zcoil, 1 / (jw * p['C_in']))
    # half-circuit of the differential network between S1 and S2: per leg Cin, R1, then C1 across the
    # pair (2·C1 to the midpoint), R2, 2·C2, Rg into the virtual ground; S2's per-leg Rf2 ∥ Cf2.
    # Solve from the summing node backwards for the input impedance and the current split.
    zc = lambda c: 1 / (jw * c)
    Zg = p['Rg']                                   # into the virtual ground
    Z = p['R2'] + par(zc(2 * p['C2']), Zg)         # seen after C1's node
    Zin = zc(p['Cin']) + p['R1'] + par(zc(2 * p['C1']), Z)
    # current through the chain for 1 V per leg at its input, then down to Rg
    i0 = 1 / Zin
    v1 = 1 - i0 * (zc(p['Cin']) + p['R1'])          # at C1's node
    i2 = v1 / Z
    v2 = v1 - i2 * p['R2']                          # at C2's node
    ig = v2 / Zg
    Zf2 = par(p['Rf2'], zc(p['Cf2']))
    A2 = ig * Zf2                                   # per-leg voltage gain; the pair keeps it
    return Zf1, A2, Zf1, Zsrc


def analyse(nAmu, L, R_loop, p=PARTS, points=4000):
    """The chain for an antenna of N·A·µ_eff (m²), inductance L and loop resistance R_loop."""
    f_lo, f_hi = p['f_lo'], p['f_hi']
    fs = [f_lo * (f_hi / f_lo) ** (i / (points - 1)) for i in range(points)]
    rows = []
    for f in fs:
        w = 2 * math.pi * f
        Zf1, A2, _, Zsrc = stages(f, L, R_loop, p)
        Zcoil = R_loop + 1j * w * L
        T = abs(1j * w * nAmu / Zcoil * Zf1 * A2)            # V at the converter per T of field
        ng = abs(1 + Zf1 / Zsrc)
        s1 = (p['e_n'] * ng) ** 2 + 4 * KB * T_K(p) * p['Rf1'] * abs(Zf1 / p['Rf1']) ** 2 \
            + (p['i_n'] * abs(Zf1)) ** 2 + 4 * KB * T_K(p) * R_loop * abs(Zf1 / Zcoil) ** 2
        chain = s1 * abs(A2) ** 2 + p['e_s2'] ** 2          # V²/Hz at the converter's input
        rows.append((f, T, chain, p['e_adc'] ** 2))
    # the flat transfer the FIR restores everything to, and the one the clip is read at: mid-band,
    # where the corners are far on either side (30 kHz, rod_calc.py's reference)
    T0 = min(rows, key=lambda r: abs(r[0] - 30e3))[1]

    def integ(lo, hi, which):
        acc, prev = 0.0, None
        for f, Tf, ch, ad in rows:
            if f < lo or f > hi:
                continue
            v = {'chain': ch, 'adc': ad}[which] / Tf ** 2        # field-referred, T²/Hz
            if prev:
                acc += 0.5 * (v + prev[1]) * (f - prev[0])
            prev = (f, v)
        return math.sqrt(acc)

    # the clip as §0.4 states it: the full scale back through S2's flat ×Rf2/(R1+R2+Rg) and Rf1·N·A·µ/L
    gain2 = p['Rf2'] / (p['R1'] + p['R2'] + p['Rg'])
    out = {'T0': T0, 'clip': p['V_fs'] * L / (gain2 * p['Rf1'] * nAmu)}
    for name, lo, hi in (('band', f_lo, f_hi), ('low', 8e3, 24e3), ('middle', 248e3, 264e3), ('high', 496e3, 512e3)):
        c, a = integ(lo, hi, 'chain'), integ(lo, hi, 'adc')
        out[name] = dict(chain=c, adc=a, total=math.hypot(c, a))
    return out


def T_K(p):
    return T


def antenna(length_mm, dia_mm, mu_i, L_mH=None):
    """N·A·µ_eff, L and the loop resistance of a rod as rod_calc.py winds it."""
    turns = rod_calc.TURNS
    mu = rod_calc.mu_rod(mu_i, length_mm / 1000, dia_mm / 1000)[0]
    A = rod_calc.core_area(dia_mm / 1000)
    L = L_mH * 1e-3 if L_mH else rod_calc.inductance(mu, turns, A, length_mm / 1000 * rod_calc.DESIGN['winding'],
                                                     rod_calc.DESIGN['k'])
    return turns * A * mu, L, 2 * PARTS['Rs1'] + rod_calc.rw_for(turns, dia_mm), mu


def report(name, length_mm, dia_mm, mu_i, L_mH=None):
    nAmu, L, R, mu = antenna(length_mm, dia_mm, mu_i, L_mH)
    r = analyse(nAmu, L, R)
    b = r['band']
    print(f"── {name}: µ_eff {mu:.0f}, N·A·µ {nAmu:.3f} m², L {L * 1e3:.2f} mH ──")
    print(f"   transfer {r['T0'] / 1e6:.2f} V/µT at 30 kHz   clip {r['clip'] * 1e9:.0f} nT")
    print(f"   floor 5–512 kHz {b['total'] * 1e12:.2f} pT  (chain {b['chain'] * 1e12:.2f} ⊕ converter {b['adc'] * 1e12:.2f})"
          f"  = {b['chain'] * r['T0'] * 1e6:.1f} / {b['adc'] * r['T0'] * 1e6:.1f} / {b['total'] * r['T0'] * 1e6:.1f} µV")
    print("   strips  low {low:.2f} · middle {middle:.2f} · high {high:.2f} pT".format(
        **{k: r[k]['total'] * 1e12 for k in ('low', 'middle', 'high')}))
    print()
    return r


if __name__ == "__main__":
    print("NIC-Tesla — the chain from the parts of HARDWARE.md §0.2\n")
    report("prototype 200 × 10 mm, L the design value (2,5 mH) — HARDWARE.md §0.4", 200, 10, 800, 2.5)
    report("prototype 200 × 10 mm, L from rod_calc.py", 200, 10, 800)
    report("Amidon R33 190 × 12,7 mm, L from rod_calc.py", 190, 12.7, 800)
    report("H7 pair 328 × 10 mm, L from rod_calc.py", 328, 10, 700)

#!/usr/bin/env python3
"""
NIC-Tesla — ferrite-rod antenna design calculator (current-mode / transimpedance front-end).

Purpose: DECIDE BEFORE BUILDING. Measure a rod you can actually buy with a ruler
(length + diameter), guess mu_i from the material (>=500 is plenty — for a slender
rod the exact value barely matters, see below), pick a winding, and this prints
the effective rod permeability, inductance, sensitivity and the B-field noise
floor. Then it says whether it hears lightning.

Physics recorded so it never has to be re-derived (owner asked, 2026-07-22):

  * mu_rod is DEMAGNETISATION-limited, set by SHAPE not volume. Slenderness
    m = length/diameter is the whole story: mu_rod ~ m^2 / ln(...) for large m.
    A long thin rod approaches mu_i; a short fat one collapses toward 1.
    => "long thin" beats "short fat" of equal cost/volume, and the flux capture
       mu_rod * area ~ length^2 (diameter cancels: area up, mu_rod down together).

  * Because mu_rod is shape-limited, the material's exact mu_i is SECONDARY for
    sensitivity (mu_i 500 vs 2000 -> mu_rod ~128 vs ~154 at l/d 20). So a
    mystery vendor rod is fine if it is long and thin — measure it, don't chase a
    datasheet mu_i. Material matters for LOSS (Mn-Zn HF self-shield) and TEMPCO
    (drift), not for this thin-vs-fat question.

  * Compose length by gluing rods END-TO-END (7 in a line ~= 49x signal). Bundling
    them SIDE-BY-SIDE just makes a fat rod -> demagnetisation eats it. Line, not bundle.

  * Current-mode SNR against the op-amp voltage noise e_n is:
        SNR = EMF / e_n = omega * N * mu_rod * A * B / e_n     (Rf cancels)
    so the B-field noise floor is  b_n(f) = e_n_eq / (omega * N * mu_rod * A).

Assumptions / honesty:
  * mu_rod: the centre third of a CYLINDER, where the winding sits — the prolate-spheroid
    demagnetisation factor taken at l/d x 1.088, which reproduces rod_field.py's solve of
    the rod within 3 % over l/d 10-35 and mu_i 500-2000. The bare spheroid at l/d reads
    10-15 % low: a cylinder is magnetically longer than its spheroid.
  * L: L = k * mu0 * mu_rod * N^2 * A / l_w  with l_w the WINDING's length — the
    centre third of the rod (HARDWARE.md §0.1) — and a coupling factor k, 0.5-0.7 on a
    real coil, so L is printed as that range and at 0.6. MEASURE the built coil; this is
    the design estimate.
  * Noise: op-amp e_n (dominant) + winding thermal; Rf Johnson and the low-end
    noise-gain peaking are sub-dominant in-band and omitted (the 5 kHz high-pass
    cuts where noise gain would run away). Band-integrated from f_lo to f_hi.

Stdlib only. Run:  python3 rod_calc.py
Edit the ROD and DESIGN blocks at the bottom, or import and call report().
"""

import math

# ---------- constants ----------
MU0 = 4e-7 * math.pi        # H/m
KB  = 1.380649e-23          # J/K
T_K = 300.0                 # K (ambient, thermal noise)

# ---------- geometry / material ----------
def demag_axial(m):
    """Axial demagnetisation factor of a cylinder length/dia = m,
    prolate-spheroid approximation. m<=1 -> sphere limit 1/3."""
    if m <= 1.0:
        return 1.0 / 3.0
    s = math.sqrt(m * m - 1.0)
    return (1.0 / (m * m - 1.0)) * ((m / (2.0 * s)) * math.log((m + s) / (m - s)) - 1.0)

CYLINDER = 1.088   # a cylinder's centre third reads as the spheroid of l/d x 1.088 (rod_field.py)

def mu_rod(mu_i, length_m, dia_m):
    """Effective permeability over the centre third of a cylindrical rod."""
    m = length_m / dia_m
    Nd = demag_axial(CYLINDER * m)
    return mu_i / (1.0 + Nd * (mu_i - 1.0)), Nd, m

# ---------- electrical ----------
def core_area(dia_m):
    return math.pi * (dia_m / 2.0) ** 2

def inductance(mu_r, N, area_m2, winding_m, k=0.6):
    """Rod-antenna inductance estimate over a winding of length winding_m.
    k = coupling/geometry factor (measure to confirm)."""
    return k * MU0 * mu_r * N * N * area_m2 / winding_m

def sensitivity_Vperf(N, mu_r, area_m2, f):
    """EMF sensitivity S(f) such that EMF = S * B, at frequency f. Units V/T."""
    return 2.0 * math.pi * f * N * mu_r * area_m2

def e_n_equiv(e_n, Rw):
    """Input-referred (as series EMF) voltage-noise density: op-amp + winding thermal."""
    return math.sqrt(e_n * e_n + 4.0 * KB * T_K * Rw)

def bnoise_density(e_n_eq, N, mu_r, area_m2, f):
    """B-field noise floor spectral density at f, in T/sqrt(Hz)."""
    return e_n_eq / (2.0 * math.pi * f * N * mu_r * area_m2)

def bnoise_rms(e_n_eq, N, mu_r, area_m2, f_lo, f_hi):
    """Band-integrated B-noise (RMS Tesla). b_n^2 ~ 1/f^2 -> closed form."""
    C = e_n_eq / (2.0 * math.pi * N * mu_r * area_m2)
    integral = C * C * (1.0 / f_lo - 1.0 / f_hi)
    return math.sqrt(integral)

# ---------- report ----------
def report(name, length_mm, dia_mm, mu_i, turns, Rw_ohm, design):
    L_m = length_mm / 1000.0
    d_m = dia_mm / 1000.0
    A = core_area(d_m)
    mu_r, Nd, m = mu_rod(mu_i, L_m, d_m)
    L = inductance(mu_r, turns, A, L_m * design["winding"], design["k"])
    f_ref = design["f_ref"]
    e_n_eq = e_n_equiv(design["e_n"], Rw_ohm)
    S = sensitivity_Vperf(turns, mu_r, A, f_ref)
    bn = bnoise_density(e_n_eq, turns, mu_r, A, f_ref)
    bn_rms = bnoise_rms(e_n_eq, turns, mu_r, A, design["f_lo"], design["f_hi"])
    ref_B = design["ref_sferic_T"]
    snr = ref_B / bn_rms

    print(f"── {name} : {length_mm:.0f} mm x {dia_mm:.0f} mm, mu_i={mu_i:.0f}, N={turns} ──")
    print(f"   slenderness l/d   = {m:.1f}")
    print(f"   demag factor Nd   = {Nd:.4f}")
    print(f"   mu_rod (effective)= {mu_r:.0f}      <- shape-limited, not mu_i")
    lo, hi = (inductance(mu_r, turns, A, L_m * design["winding"], k) for k in (0.5, 0.7))
    print(f"   inductance L      = {L*1e3:.2f} mH   ({lo*1e3:.2f}-{hi*1e3:.2f} for k 0.5-0.7, the centre third, MEASURE)")
    print(f"   winding R_w       = {Rw_ohm:.2f} ohm  ({Rw_ohm / WIRE_OHM_M:.1f} m of {WIRE_MM} mm wire)")
    print(f"   sensitivity @{f_ref/1e3:.0f}kHz = {S/1e3:.0f} kV/T")
    print(f"   B-noise floor     = {bn*1e15:.1f} fT/sqrt(Hz) @ {f_ref/1e3:.0f} kHz")
    print(f"   B-noise band RMS  = {bn_rms*1e12:.2f} pT  ({design['f_lo']/1e3:.0f}-{design['f_hi']/1e3:.0f} kHz)")
    print(f"   SNR vs {ref_B*1e9:.0f} nT sferic = {snr:.0f}x  ({20*math.log10(snr):.0f} dB)")
    print()
    return dict(mu_rod=mu_r, L_mH=L*1e3, L_range_mH=(lo*1e3, hi*1e3), Rw=Rw_ohm, bn_fT=bn*1e15, bn_rms_pT=bn_rms*1e12, snr=snr)


# ============ EDIT HERE ============
DESIGN = dict(
    e_n=3.3e-9,           # THS4551 input voltage-noise density, V/sqrt(Hz)
    k=0.6,                # inductance coupling factor (measure the real coil)
    winding=1/3,          # the winding covers the centre third of the rod (HARDWARE.md §0.1)
    f_ref=30e3,           # reference frequency for the spot figures
    f_lo=5e3, f_hi=512e3,  # the band, 5-512 kHz (HARDWARE.md §2)
    ref_sferic_T=1e-9,    # a modest 1 nT sferic (~typical strike at ~100 km) for the SNR line
)

# turns: 150 on every rod, HARDWARE.md §0.1. The count is the stop rule's
# (CONSTRUCTION.md): more turns buy no reception under QRN and double every
# in-band carrier at S1. No SRF model here: the self-resonance (~285 kHz) lands
# inside the band and S1's virtual short passivates it (HARDWARE.md §2) — MEASURE
# the wound rod.
TURNS = 150

# the wire: 0,315 mm enamelled copper, grade 1, <= 0,352 mm over the enamel — 150 turns
# are 53 mm of single layer, inside the centre third of a 190 mm rod with room for the
# gaps between sections (HARDWARE.md §0.1). 0,221 ohm/m at 20 C; one turn per rod
# circumference — computed, not measured.
WIRE_MM, WIRE_OVER_MM, WIRE_OHM_M = 0.315, 0.352, 0.221

def rw_for(turns, dia_mm):
    return turns * math.pi * dia_mm / 1000.0 * WIRE_OHM_M

def winding_fits(turns, length_mm, span=1/3):
    """(single-layer length, the span it must fit), mm."""
    return turns * WIRE_OVER_MM, length_mm * span

if __name__ == "__main__":
    print("NIC-Tesla ferrite-rod antenna calculator — decide before building\n")

    # The owner's two eBay/AliExpress candidates (same material assumed, mu_i=800):
    report("Candidate A  long/thin", 200, 10, 800, TURNS, rw_for(TURNS, 10), DESIGN)
    report("Candidate B  short/fat", 67, 30, 800, TURNS, rw_for(TURNS, 30), DESIGN)

    # The composed build: two TESLA H7 (164 mm) end-to-end per arm:
    report("H7 pair end-to-end",    328, 10, 700, TURNS, rw_for(TURNS, 10), DESIGN)

    # Reference: a single long documented Mn-Zn rod (Amidon R33 class):
    report("Amidon R33 (ref)",      190, 12.7, 800, TURNS, rw_for(TURNS, 12.7), DESIGN)

    print("Read-off: mu_rod tracks slenderness (l/d), NOT diameter or volume.")
    print("The short/fat candidate wastes its ferrite (mu_rod collapses).")
    print("All slender builds sit far below outdoor atmospheric QRN — one rod hears every storm.")
    print("SRF: the winding's self-resonance (~285 kHz) sits inside the band and S1's")
    print("virtual short passivates it (HARDWARE.md §2: ~150 turns, ~2,5 mH) — measure the wound rod.")

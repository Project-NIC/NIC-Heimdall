#!/usr/bin/env python3
"""
NIC-Tesla — the ferrite rod's effective permeability, solved: the field of the rod itself.

rod_calc.py uses a closed form for µ_rod. This is what that closed form is calibrated against:
the rod cut into slices along its axis, each slice uniformly magnetised, the field every slice
makes at every other averaged over the cross-section (the fluxmetric value, which is what a
winding links), and M = (µᵢ − 1)·H solved for all slices at once in a uniform axial field.

What comes out is µ_eff averaged over the winding's span — the centre third, where Tesla's
winding sits (HARDWARE.md §0.1). The result:

  * a cylinder is magnetically longer than the prolate spheroid of the same l/d: its centre third
    reads like a spheroid of l/d × 1,088, within 3 % over l/d 10–35 and µᵢ 500–2000;
  * at µᵢ → ∞ and l/d 10 it gives N ≈ 0,0175, the published fluxmetric factor of a cylinder.

Needs numpy. Run:  python3 rod_field.py            (the table, ~10 s)
"""
import math
import sys

import numpy as np


def mu_eff(mu_i, length, dia, slices=300, rings=4, nr=6, nphi=12, span=1 / 3):
    """µ_eff of a rod of µᵢ, averaged over the centre `span` of its length (any length unit)."""
    R = dia / 2
    z = np.linspace(-length / 2, length / 2, slices + 1)
    zc = 0.5 * (z[1:] + z[:-1])
    # a source disc sampled on a polar grid, area-weighted
    rs = (np.arange(nr) + 0.5) / nr * R
    phis = (np.arange(nphi) + 0.5) / nphi * 2 * np.pi
    sx = (rs[:, None] * np.cos(phis)[None, :]).ravel()
    sy = (rs[:, None] * np.sin(phis)[None, :]).ravel()
    sw = np.repeat(2 * np.pi * rs * (R / nr) / nphi, nphi)
    # the field is read on rings across the target section, weighted by their area
    tr = (np.arange(rings) + 0.5) / rings * R
    tw = tr / tr.sum()

    def disc(dz):
        """H_z, section-averaged, of a unit-density charged disc at axial offsets dz."""
        out = np.zeros((len(dz), rings))
        for k, r in enumerate(tr):
            rr = np.sqrt((r - sx)[None, :] ** 2 + sy[None, :] ** 2 + dz[:, None] ** 2)
            out[:, k] = (sw[None, :] * dz[:, None] / (4 * np.pi * rr ** 3)).sum(1)
        return out @ tw

    # a slice magnetised M carries +M on its top face and −M on its bottom face
    A = np.empty((slices, slices))
    for j in range(slices):
        A[:, j] = disc(zc - z[j + 1]) - disc(zc - z[j])
    chi = mu_i - 1
    M = np.linalg.solve(np.eye(slices) - chi * A, chi * np.ones(slices))
    B = 1 + A @ M + M                     # B / (µ0·H0) along the rod
    return float(B[np.abs(zc) < length * span / 2].mean())


if __name__ == "__main__":
    sys.path.insert(0, __file__.rsplit("/", 1)[0] if "/" in __file__ else ".")
    import rod_calc
    print("µ_eff over the centre third: the solve against rod_calc.py's closed form\n")
    print(f"{'rod':28s} {'l/d':>5s} {'solved':>8s} {'closed':>8s}")
    for name, mu, l, d in [("Amidon R33, 190 × 12,7", 800, 190, 12.7), ("prototype, 200 × 10", 800, 200, 10),
                           ("TESLA H7, 164 × 10", 700, 164, 10), ("H7 pair, 328 × 10", 700, 328, 10)]:
        closed = rod_calc.mu_rod(mu, l / 1000, d / 1000)[0]
        print(f"{name:28s} {l / d:5.1f} {mu_eff(mu, l, d):8.1f} {closed:8.1f}")
    print(f"\nµᵢ → ∞, l/d 10: N = {1 / mu_eff(1e6, 100, 10):.4f}")

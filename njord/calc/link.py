"""Njord — the link's reach and Ratatoskr's I²C leads. A rough calculation.

Sensitivity: Semtech SX1261/2 rev 1.2 §3 — 125 kHz, Rx boosted: SF7 −124 dBm, SF12 −137 dBm; between,
LoRa's demodulation floor (SNR −7,5 dB at SF7, 2,5 dB a step to −20 dB at SF12, Semtech AN1200.22)
over the same noise floor, which gives both sheet points back. Power: h1.8 5 mW e.r.p. (≈ 9 dBm
e.i.r.p.); h1.7 the module's 22 dBm into a 2 dBi antenna. Path: the two-ray ground model past its
breakpoint (antennas low over flat ground), 40·log d − 20·log(h1·h2), which is what a unit a metre
over the grass sees; free space before it. Excess loss for trees, buildings — GUESS, bracketed."""
import math

LAM = 3e8 / 868e6
def sens(sf):
    return -174 + 10 * math.log10(125e3) + 6 - (7.5 + 2.5 * (sf - 7))   # NF 6 dB, as the sheet's points imply
def loss(d, h1, h2):
    fs = 20 * math.log10(4 * math.pi * d / LAM)
    bp = 4 * math.pi * h1 * h2 / LAM
    return fs if d < bp else 40 * math.log10(d) - 20 * math.log10(h1 * h2)
def reach(budget, h1, h2):
    d = 10.0
    while loss(d * 1.02, h1, h2) <= budget:
        d *= 1.02
    return d

print("sensitivity:", ", ".join(f"SF{s} {sens(s):.1f} dBm" for s in (7, 9, 10, 12)))
RX_ANT, CABLE, FADE = 2, 2, 10          # dBi at Iris, dB of Iris's coax, dB of margin — GUESS
for label, eirp in (("h1.8, 5 mW", 9.2), ("h1.7, 22 dBm + 2 dBi", 24.0),
                    ("h1.7, Ratatoskr on 2,4 V, ~19 dBm", 21.0)):   # SX1261/2 rev 1.2 TXDRP: −3 dB at 2,4 V
    for sf in (7, 9, 10):
        for h1, h2 in ((1, 3), (1, 8)):
            out = []
            for excess in (0, 15, 30):
                b = eirp + RX_ANT - CABLE - FADE - sens(sf) - excess
                out.append(f"{reach(b, h1, h2)/1000:5.1f}")
            print(f"{label:22} SF{sf:2}  unit {h1} m, Iris {h2} m: reach "
                  f"{out[0]} km open · {out[1]} km through 15 dB of trees · {out[2]} km through 30 dB")

# --- Ratatoskr's I²C leads: the branch down (SHT45 at 5 cm) and up (SHT45 at 2 m), the box at 1 m. NXP UM10204: standard mode, rise time ≤ 1000 ns = 0,8473·Rp·Cb; Rp ≥ (VDD − 0,4 V)/3 mA; the rail 2,4 V.
# Cable ~100 pF a metre (a twisted four-core — GUESS, the cable's sheet), 10 pF a part, 20 pF of board.
print()
for name, metres, parts in (("down", 1.0, 1), ("up", 1.0, 1)):
    cb = 100e-12 * metres + 10e-12 * parts + 20e-12
    rmax = 1000e-9 / (0.8473 * cb)
    rmin = (2.4 - 0.4) / 3e-3
    print(f"I2C branch {name}: {metres:.1f} m, {cb*1e12:.0f} pF — pull-ups {rmin/1e3:.2f}–{rmax/1e3:.1f} kΩ at 2,4 V;"
          f" 2,2 kΩ gives a {0.8473*2.2e3*cb*1e9:.0f} ns rise")

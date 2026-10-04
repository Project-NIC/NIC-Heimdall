<p align="center">
  <img src="NIC-QuarkScintillation.svg" width="200"/>
</p>

★ N.I.C. ★

# Quark-Scintillation — the low-voltage radiation units

> **Design-stage concept — nothing built.** **The low-voltage build of Quark**, the
> radiation part ([`../README.md`](../README.md)); the high-voltage build is
> [`Quark-Tubes`](../tubes/). This file is the group's map: what stands in it and where each
> thing is written.

**Scintillators read by electronics, on two boards.** A crystal, a plastic block or a screen turns
a particle into a flash of light; a SiPM, a PIN diode or a photomultiplier turns the light into a
pulse; the board digitises the pulse as a waveform, and its area is the deposited energy. So this
side gives **counts and energy**, where the tubes give counts. Nothing on it runs above ~30 V
except `Neutron`'s photomultiplier.

| unit | quantity | sensing part | board | on the bus |
|---|---|---|---|---|
| **[Photon](photon/)** | γ + X-ray | CsI(Tl) cube read by a SiPM and a PIN together, one measurement switched by energy | `Quark-Photon` | **NOD, NodBus type 9** |
| **[Positron](positron/)** | β, both signs | a 25 mm plastic scintillator block + SiPM | `Quark-Neutron/Positron` | **NOD, NodBus type 11** |
| **[Neutron](neutron/)** | neutron, thermal and epithermal | two `⁶LiF/ZnS(Ag)` screens with PMMA between, read by a photomultiplier | `Quark-Neutron/Positron`, channel 1 | **a channel of Positron** — its two counts ride Positron's record; type 10 reserved |

**Two boards, one design.** Both are `STM32H7A3IIT6` boards with the `AD9251-80` on the PSSI —
the same digital half, one firmware image. `Quark-Photon` carries the SiPM and the
PIN, sampled simultaneously, with the guard ring; `Quark-Neutron/Positron` one SiPM channel and the
photomultiplier's, no `LTC6268` and no guard ring. **The photomultiplier's ~1,2 kV comes from
Helion's kV source** (`../tubes/helion/HARDWARE.md`), built for this head.

**One record for both units**: 32 B every frame, every field an accumulator reset on the second —
count · energy sum · largest event · 12 energy bands on Photon, 10 and the two neutron counts on
Positron.

## The tree

```
scintillation/  Quark-Scintillation — low voltage
├── photon/     Photon — γ / X-ray, on Quark-Photon                      NodBus 9
├── positron/   Positron — β, on Quark-Neutron/Positron                  NodBus 11
└── neutron/    Neutron — the photomultiplier channel on the same board  type 10 reserved
```

## Where it is written

The group's shared documents live with Photon, the first of the two boards:

| | |
|---|---|
| the scintillation physics and the whole front end | [`photon/SCINTILLATION.md`](photon/SCINTILLATION.md) |
| the H7A3 layer both boards share, and `Quark-Photon` | [`photon/HARDWARE.md`](photon/HARDWARE.md) |
| the record contract | [`photon/BUS.md`](photon/BUS.md) |
| the one firmware image | [`photon/FIRMWARE.md`](photon/FIRMWARE.md) |
| `Quark-Neutron/Positron` and the beta unit | [`positron/`](positron/) |
| the photomultiplier channel | [`neutron/`](neutron/) |
| the neutron physics both builds share | [`../NEUTRONS.md`](../NEUTRONS.md) |

## Licence

Hardware: CERN-OHL-S v2 (`../../LICENSE-HW`) · Software: MIT (`../../LICENSE`) — Copyright (c) 2026 NIC — Native Intellect Community

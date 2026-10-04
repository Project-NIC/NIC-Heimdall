<p align="center">
  <img src="NIC-Ceres.svg" width="200"/>
</p>

★ N.I.C. ★

# Ceres — the soil-moisture sensor

> **Design-stage concept — nothing built.** The figures are verified against the parts' sheets
> and the parts before a board is made.

**Ceres measures soil volumetric water content at a WMO reference depth, and the soil temperature
there: ModBus type 6, a MOD on a Palatine arm.** A comb electrode on the board's underside reads
through 1 mm of borosilicate glass — the board is potted in a borosilicate bowl, comb down, glass
on both faces — and is read as an admittance at three frequencies: the quadrature part is the
water, the in-phase part the ions, and the frequencies separate the two. The unit answers a
finished, compensated value. Several units lie in the ground in a **patrona**, a tube packed on the
bench and knocked into the ground as one piece (`../palatine/CONSTRUCTION.md`, *The soil column*),
which is what makes the depth repeatable between stations.

| | |
|---|---|
| class | **house MOD** — a Modbus RTU slave on a Palatine arm, **ModBus type 6**, address `6«2 \| NUMBER`, 0x18 for unit 0; no clock, the host stamps the reading when it polls |
| what it reports | the water content on `0x0000`, the soil temperature at the depth on `0x0001` — one unit, one address, two values (`MODBUS.md`) |
| the sensing element | the board's own comb under the bowl's glass, horizontal in the patrona's packed sand, glass down |
| the read | a synchronous detector at 2²⁰ / 2²² / 2²⁴ Hz — 1,05 / 4,19 / 16,8 MHz — against a reference capacitor through the same chain (`HARDWARE.md`) |
| the parts | `STM32H523VE` · `THS4541` · 3× `74LVC1G3157` · `74LVC1G17` · `TMP117` or `STS35` · `THVD1450` · `LMR43610` |
| depths | the standard series 5 · 10 · 20 · 50 · 100 cm — the station's pair −10 and −50 cm, a farm's profile −10 · −20 · −50 · −100 cm; which depths a station carries is a crop-and-site decision |
| feed | the arm's isolated 12 V on the four-wire cable, like every sensor on the arm; the board makes its own 3,3 V |
| the cable | flat, four conductors, soldered to the board and potted through the bowl's channel; no connector on the unit |

**Sakura is the same board in the same bowl, hung in the canopy** — `sakura/`.

## The block

```
   a Palatine arm ── A · B · 12 V · GND ── the flat cable, up the patrona
        │
   ┌────┴─────────────────────────────────────────────────────────────────────────┐
   │ CERES — the board potted in a borosilicate bowl                              │
   │  H523 · THVD1450 · the detector chain                                        │
   │  the comb on the underside ── 1 mm of glass ── the packed sand at the depth  │
   │  the thermometer on the board ── the soil temperature there                  │
   └──────────────────────────────────────────────────────────────────────────────┘
```

## The tree

```
ceres/       Ceres — soil moisture and the soil temperature at its depth  ModBus 6
└── sakura/  Sakura — leaf wetness, Ceres's board in the same bowl        ModBus 7
```

## Files

| file | contents |
|---|---|
| [`HARDWARE.md`](HARDWARE.md) | the board and the bowl — the measurement, the chain, the rails, the potting, every pin, the parts, the bench criteria; the one board of Ceres and Sakura |
| [`FIRMWARE.md`](FIRMWARE.md) | the firmware, described — the read cycle at three frequencies, the admittance arithmetic, the curve and the compensation, what is tested |
| [`MODBUS.md`](MODBUS.md) | the ModBus contract of both units |
| [`WHY.md`](WHY.md) | the graveyard |
| [`sakura/`](sakura/) | Sakura — the same board as the leaf-wetness sensor |

## Licence

Hardware: CERN-OHL-S v2 (`../LICENSE-HW`) · Software: MIT (`../LICENSE`) — Copyright (c) 2026 NIC — Native Intellect Community

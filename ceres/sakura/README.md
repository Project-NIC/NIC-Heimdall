<p align="center">
  <img src="NIC-Sakura.svg" width="200"/>
</p>

★ N.I.C. ★

# Sakura — the leaf-wetness sensor

**English** · [Čeština](README.cs.md) · [Русский](README.ru.md)

> **Design-stage concept — nothing built.** The figures are verified against the parts' sheets
> and the parts before a board is made.

**Sakura measures leaf-surface wetness — the in-canopy condition that drives fungal-disease models
and dew and frost timing — and the temperature of the plate that carries it: ModBus type 7, a MOD
on a Palatine arm.** It is Ceres's board in Ceres's borosilicate bowl, the comb reading through
1 mm of glass as an admittance at three frequencies (`../HARDWARE.md`); what differs is the back,
where it hangs and the curve in its cells (`HARDWARE.md`, `FIRMWARE.md`).

**Where it hangs decides what it measures.** On a thin arm in the canopy, **glass to the sky at
45°** so water runs off, **with no radiation shield**: the plate must cool by radiation to form dew
and warm in the sun to dry, as a leaf does. Over it only a coarse stainless mesh against hail, never
a solid plate, because rain is wetness and the sky view is what forms the dew. There is no standard
leaf and no reference instrument; the disease models were built against plates like this one, so
the plate is the standard, and the species is the agronomist's calibration downstream.

| | |
|---|---|
| class | **house MOD** — a Modbus RTU slave on a Palatine arm, **ModBus type 7**, address `7«2 \| NUMBER`, 0x1C for unit 0; no clock |
| what it reports | the wetness on `0x0000`, the temperature at the plate on `0x0001` — a leaf temperature, not the air's (`../MODBUS.md`) |
| the sensing element | the board's own comb under the bowl's glass, the glass to the sky at 45° |
| the read | a synchronous detector at 2²⁰ / 2²² / 2²⁴ Hz — 1,05 / 4,19 / 16,8 MHz (`../HARDWARE.md`) |
| the parts | `STM32H523VE` · `THS4541` · 3× `74LVC1G3157` · `74LVC1G17` · `TMP117` or `STS35` · `THVD1450` · `LMR43610` — Ceres's board, whole |
| feed | the arm's isolated 12 V on the four-wire cable; the board makes its own 3,3 V |
| the cable | flat, four conductors, soldered to the board and potted through the bowl's channel; no connector on the unit |

## The block

```
   a Palatine arm ── A · B · 12 V · GND ── the flat cable
        │
   ┌────┴────────────────────────────────────────────────────────────┐
   │ SAKURA — Ceres's board potted in the same bowl                  │
   │  the comb on the underside ── 1 mm of glass ── the sky, at 45°  │
   │  the thermometer on the board ── the plate's temperature        │
   └─────────────────────────────────────────────────────────────────┘
     held on a thin arm in the canopy by the holder on its back;
     a coarse stainless mesh ~5 cm above the glass, against hail
```

## Files

| file | contents |
|---|---|
| [`HARDWARE.md`](HARDWARE.md) | what differs from Ceres's board: the ring, the back and its holder, the arm and the mesh; the board itself is `../HARDWARE.md` |
| [`FIRMWARE.md`](FIRMWARE.md) | what differs from Ceres's firmware: the type, the register, the curve's end points, the film correction; the firmware itself is `../FIRMWARE.md` |
| [`../MODBUS.md`](../MODBUS.md) | the ModBus contract of both units |
| [`WHY.md`](WHY.md) | the graveyard, Sakura's own; the board's is `../WHY.md` |

## Licence

Hardware: CERN-OHL-S v2 (`../../LICENSE-HW`) · Software: MIT (`../../LICENSE`) — Copyright (c) 2026 NIC — Native Intellect Community

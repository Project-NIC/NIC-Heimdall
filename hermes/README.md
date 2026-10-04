<p align="center">
  <img src="NIC-Hermes.svg" width="200"/>
</p>

★ N.I.C. ★

# Hermes — the BMS/MPPT converter

**English** · [Čeština](README.cs.md) · [Русский](README.ru.md)

> **Design-stage concept — nothing built.** The figures are verified against the parts' sheets
> and the parts before a board is made.

**Hermes is a small H523 card between the Mayak's LP I²C and the bought power boards.** It polls
the BMS and the MPPT on whatever bus they come with, holds the latest register set, and the Mayak
reads it as one block over the LP I²C — the survival loop reads finished values instead of waiting
out a bus round. Hermes measures nothing; measuring is the BMS's job. **The link is internal and
never gated: it is how the head finds out when to come back.**

| | |
|---|---|
| class | house card, **no bus type** — not a NodBus node and not a ModBus slave; a peripheral of the Mayak on its power body |
| MCU | `STM32H523VE`, LQFP100, at 4 MHz on the CSI or 8 MHz on the HSI ÷ 8; no crystal; every peripheral it does not use is off |
| toward the head | **I²C slave** on the Mayak's **LP I²C**, `ALERT` on an LP GPIO, on the Galvani **power body**, `ID` at **0,90** |
| toward the power boards | **485 Modbus RTU on `THVD1450`**, the standard face · a 3,3 V UART · an I²C out through a `PCA9306` · CAN on `TCAN334` — all four fitted, the unused ones asleep; the faces are not isolated and the bought parts must be common-negative (`HARDWARE.md`) |
| the parts | `STM32H523VE` · `THVD1450` · `TCAN334` · `PCA9306` · the `ID` resistor · four two-pole `DGPS2.5R-5.0` blocks · two pin headers |
| power in | **3,3 V off the Mayak over the body**, behind a 0,15 A polyfuse on the Mayak; no 12 V terminal, no buck |
| draw | **≤ 100 mA on the 3,3 V pin**, every face fitted and polling — the budget |
| firmware | one profile of the shared `nic-mod` engine, the one profile with two faces: master toward the BMS and the MPPT, slave toward the Mayak (`FIRMWARE.md`) |

## The block

```
   MAYAK ──LP I²C + ALERT─▶ HERMES ──485 Modbus RTU (THVD1450)──▶ the MPPT, or a BMS on 485
   (I²C master,  power      (H523,   ├─ UART 3,3 V TTL ───────────▶ a BMS on its own UART
    3,3 V out)   body        polls,  ├─ I²C out (PCA9306 for 5 V) ▶ a part that offers one
                             holds)  └─ CAN (TCAN334) ────────────▶ a part with no other bus

   Which units, and their protocols, are the build's: the card runs the MAP the builder fills
   for the units bought (FIRMWARE.md §5); the document names no vendor.
```

## Files

| file | contents |
|---|---|
| [`HARDWARE.md`](HARDWARE.md) | the board — the rail, the clock, every pin, the faces and their protection, the body, the parts, the bench criteria |
| [`FIRMWARE.md`](FIRMWARE.md) | the firmware, described — the arrangements and the buses a pack is run with, the map the builder fills, the probe, the block, the thresholds and `ALERT`, what is tested |
| [`WHY.md`](WHY.md) | the graveyard |

## Licence

Hardware: CERN-OHL-S v2 (`../LICENSE-HW`) · Software: MIT (`../LICENSE`) — Copyright (c) 2026 NIC — Native Intellect Community

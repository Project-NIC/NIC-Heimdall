★ N.I.C. ★

# Galvani — BOM

> **Design-stage concept — nothing drawn, nothing built.** This file collects into one place what
> [`HARDWARE.md`](HARDWARE.md) decides board by board; where the two disagree, `HARDWARE.md` wins.
> **It is a concept: before a schematic is drawn, every figure here is verified against the parts'
> sheets and the parts themselves.**

## How to read a table

Every row carries a **how** column, and it says what a builder is allowed to change.

| how | meaning |
|---|---|
| **code** | **order this part.** It was chosen against a datasheet figure and a substitute has to be re-derived — controllers, FETs, windings, rectifiers, tubes, transils, isolators, connectors |
| **value** | **the number is the design.** The body is free, the value and the rating are not — a shunt, a feedback divider, an `ID` resistor |
| **class** | **take what you can get.** Package, voltage and dielectric are given; the manufacturer is not. Order a name you trust — Murata, Samsung, Panasonic, Yageo, KEMET — and stock is the only criterion |

**Derating is part of the class, not a preference.** A ceramic at its rated voltage has lost most
of its value to DC bias, so a rail's capacitor is rated at least 1,75× the rail and a node behind
a transil is rated at the clamp, never at the rail. **Every board that meets a cable is a 2 oz stack
on both layers**; the two optical boards are 1 oz.

**Every board is one drawing and the tables follow its blocks**, in the order the current flows.
Three blocks are the same on every board that carries them and are written once, here; a board's
own table names them by their block name.

---

## The common blocks

### Block T · the telemetry front — every power board

| ref | part / value | how | note |
|---|---|---|---|
| U_T | **`INA238`** | code | `ADCRANGE` = 1, ±40,96 mV; conversion 4,12 ms × 128 averages; `APOL` = 1. `A1` to ground on the board, `A0` from the isolator's channel A — 0x40 / 0x41 by the socket's `A_SEL` |
| U_I | **`ISO1642`**, DW-16 | code | the I²C both ways, `A_SEL` down on channel A, `ALERT` up on channel B; side 1 on the host's 3,3 V, side 2 on block S |
| R_SHUNT | **1210 metal-element sense resistor, 1 %, ≤ 75 ppm/°C, 0,5 W** — the value is the board's | value | Kelvin-connected to `IN+`/`IN−`; low-side, in the − pole |
| R_F1, R_F2 · C_F | **2× 82 Ω 0603 · 100 nF 50 V X7R 0603** | class | the shunt input filter, the sheet's figure 7-1 — a 10 µs corner; 0,18 % gain error, calibrated out |
| R_VT · R_VB | **the `VBUS` divider — top leg the board's, bottom 100 kΩ 1 % 0603**; none on 12 V and 24 V | value | 90,9 kΩ 1206 on 48 V; 3× 422 kΩ 1206 in series on 300 V. The pin's own 1 MΩ in parallel makes the bottom 90,9 kΩ ±2 %, one calibration at commissioning |
| C_VBUS | **10 nF 50 V X7R 0603**, `VBUS` to `GND` | class | the low-pass that keeps a strike's front off the pin — 0,45 ms on 48 V, ~0,85 ms on 300 V |
| C_T1, C_T2 | **100 nF 50 V X7R 0603** at `VS` and at `VCC2` | class | |

### Block S · the measuring-side island — every power board and every copper communication board

| ref | part / value | how | note |
|---|---|---|---|
| U_S | **`SN6505B`** | code | push-pull driver, its own 420 kHz oscillator. `EN` tied to the 3,3 V on a power board — always on; on a communication board `EN` is `LINE_EN` |
| T_S | **`750313734`** | code | Würth, 1:1,1 centre-tap, **5 kV** — the one island winding in the family |
| D_S1, D_S2 | **`PMEG10020ELR`** ×2 | code | the centre-tapped secondary's rectifier |
| C_S1 | **10 µF 50 V X5R 1206 + 100 nF 50 V X7R 0603** at `VCC` | class | basic parts |
| C_S2 | **10 µF 50 V X5R 1206 + 100 nF 50 V X7R 0603** on the isolated 3,3 V | class | no LDO: the raw rectified rail wanders a few hundred millivolts and everything on it takes 2,7–5,5 V |

### Block J · the power connector, and the switch

| ref | part / value | how | note |
|---|---|---|---|
| `PWR` | **`BX2.54-2x4NA`** | code | the power connector, 8 pins: `ENABLE` 1 · `GND` 2 · `ID` 3 · `SDA` 4 · `A_SEL` 5 · `SCL` 6 · `ALERT` 7 · `3,3 V` 8. The cable end is `FC-8P` |
| R_ID | **1 %, E96 — the value is the board's** | value | to ground; read against the host's 10 kΩ |
| R_EN | **100 kΩ 0603** | value | on a source board from `EN`/`UVLO` to ground, `ENABLE` from the connector drives the pin; on a unit board from `EN` to the input — the board runs whenever the feed is there |

*The data connector on a communication board is `BX2.54-2x6NA`, 12 pins, cable end `FC-12P`; its
pins are `README.md`, *The connectors*.*

---

# `G-48-S` — power, source, 48 V

The battery-side feed cell: taps the 12 V wire on its own terminals, makes the 48 V for one run,
measures it, and holds the source end of the ladder.

```
  12 V off the wire ─▶ L_IN ─▶ LT3748 · Q1 ─▶ T1 ─▶ D1 ─▶ bulk ─▶ D2 ─▶ transils ─▶ chokes ─▶ ═══ 48 V
                                                                          2036-07-SM ─────────▶ ⏚ earth
       block T on the output · block S · block J: 3,3 V · ENABLE · I²C · ID
```

## 1 · The input — 12 V off the wire

| ref | part / value | how | note |
|---|---|---|---|
| `12V` | **`DGPS2.5R-5.0`** ×2 | `10060008518` | Degson, 2 poles, 5 mm, PUSH-SNAP, 0,75–2,5 mm², 20 A, 400 V. **Input and tap, one node** — 2,6 A at the 11 V floor |
| TVS_IN | **`5.0SMDJ14A`** | code | across `12V`, ahead of `L_IN` — stands off 14 V, conducts from 15,6–17,2 V; with the board's fuse in the enclosure's fuse field it clears a reversed pack or a held overvoltage |
| L_IN | **1,5 µH**, shielded, `I_SAT` ≥ 8 A | value | with the bulk it is the filter that keeps the cell's triangle off the shared 12 V wire, ~20 dB at 41 kHz |
| C_IN | **2× hybrid polymer 47 µF 50 V** (Panasonic ZA `EEHZA1H470P`) **+ 2× 10 µF 50 V X5R 1206** | class | the hybrids carry the 3,1 A rms and their ESR damps the filter's peak to ~0,8 Ω against the cell's −4,3 Ω |

## 2 · The cell — `LT3748` + an external FET

| ref | part / value | how | note |
|---|---|---|---|
| U1 | **`LT3748`**, MSOP-16E | code | no-opto flyback controller, boundary mode, no burst — the board carries a minimum load of 0,65 W |
| Q1 | **`ISC165N15NM6`** | code | 150 V; 26 V on the plateau, 78 V at the leakage clamp's worst corner |
| T1 | **`750310988`** | code | 1:4,42 · `L_PRI` 14 µH ±10 % · `I_SAT` 15 A min · leakage 200 nH · 1500 V AC · SMD 32,3 × 27,0 × 13,7 mm |
| R_SENSE | **9,1 mΩ, 2512, 1 W** | value | 0,9 W peak dissipation. `I_pk` 9,90 A at the guaranteed 90 mV, the 110 mV corner 12,1 A — 81 % of the winding's saturation |
| R_GATE | **10 Ω 0603** | class | |
| R_C, C_C | **24,9 kΩ + 4,7 nF X7R ∥ 100 pF C0G, 0603** | class | the sheet's compensation |
| C_SS | **100 nF 50 V X7R 0603** | class | 0,05 V/ms at `V_C` |
| R_REF | **6,04 kΩ 1 %** | value | the part is trimmed at this value |
| R_FB | **54,9 kΩ 1 %** | value | `R_REF · N_PS · [(V_OUT + V_F) + V_TC] / V_BG` at `N_PS` 1/4,42, `V_F` 0,7 V |
| R_TC | **243 kΩ 1 %** | value | `R_FB / N_PS` |
| R_CL | **2× 2,43 kΩ, 2512, 2 W class, in series** | value | the leakage clamp: 50 V, 0,52 W rated, 0,68 W at the corner |
| C_CL | **2,2 µF 100 V X7R 1210** | class | |
| D_CL | **`V5N22-M3`** | code | the clamp's Schottky, the drain to `C_CL` |
| C_VIN · C_VCC | **10 µF 50 V X5R 1206 · 4,7 µF 50 V X7R 1206** | class | at `V_IN` and `INTVCC` |

## 3 · The rectifier and the output

| ref | part / value | how | note |
|---|---|---|---|
| D1 | **`V5N22-M3`** | code | 220 V / 5 A Schottky at 0,54 A; `PIV` 114 V |
| C_OUT | **6× 2,2 µF 100 V X7R 1210** | class | ~7 µF at 48 V, ripple ~4 % |
| D2 | **`US3M`** | code | the blocking diode between the bulk and the transil, 1000 V / 3 A, `I_FSM` 100 A |

## 4 · The ladder — source end

| ref | part / value | how | note |
|---|---|---|---|
| GDT1 | **`2036-07-SM`** | code | three electrodes, the centre to the common earthing point; 60 V minimum sparkover, 52 V holdover — quenches itself on 48 V |
| TVS1, TVS2 | **`5.0SMDJ54A`** ×2 | code | in parallel across the pair, clamp ~87 V; one reel, symmetric traces |
| L1, L2 | **22 µH**, surge-rated **5 A** | value | between the tube and the clamp |
| `FEED OUT` | **`DGPS2.5R-5.0`** ×2 | `10060008518` | the feed out, two pairs — one pole to a net |
| `⏚` | the earth stud: M4 plated hole, pad ≥ 10 mm both layers; screw · washer · board · washer · ring lug · washer · spring washer · nut, brass; 2,5 mm² strap with a 4 mm crimped ring lug | — | bolted at a station; the hole empty at a remote site |

## 5 · Telemetry, island, connector

**Block T on the output**: `R_SHUNT` **50 mΩ**, `R_VT` **90,9 kΩ 1206**, `R_VB` 100 kΩ. **Block S.**
**Block J**: `R_ID` **15,0 kΩ**, 0,60; `R_EN` to ground, `ENABLE` drives `EN/UVLO`.

---

# `G-300-S` — power, source, 300 V

Same cell, same controller, same FET; the winding, the shunt, the rectifier and the codes are the
300 V ones, and the leak watch is added. ~47 W delivered, 50,6 W in — one number at a station and
at a remote site; the shunt allows ~66 W and block T's threshold holds the 47.

## 1 · The input

As `G-48-S`: **`DGPS2.5R-5.0`** ×2, `TVS_IN` **`5.0SMDJ14A`**, `L_IN` 1,5 µH; **`C_IN` 3× hybrid 47 µF 50 V + 2× 10 µF 50 V 1206** —
three, because the ripple is 3,9 A rms. The tap is 4,6 A at the 11 V floor.

## 2 · The cell

| ref | part / value | how | note |
|---|---|---|---|
| U1 · Q1 | **`LT3748`** · **`ISC165N15NM6`** | code | 45 V on the plateau, 132 V at the clamp's corner |
| T1 | **`750310349`** | code | 1:10 · `L_PRI` 5 µH ±10 % · `I_SAT` 25 A typ, `I_R` 18 A thermal · EE35/18/10, wire leads. **1000 V AC insulation, under the family's 1,5 kV floor — accepted for the concept; a production run orders a custom-insulated winding** |
| R_SENSE | **2× 11 mΩ in parallel — 5,5 mΩ — 2512, 2 W** | value | the largest the winding rule allows: 4,41 µH needed against 4,5 µH at −10 %. `I_pk` 16,4 A at 90 mV; the 110 mV corner 20,0 A, 80 % of the 25 A saturation. 0,77 W peak at the declared ~47 W, 1,1 W in each part at the corner |
| R_GATE · R_C, C_C · C_SS · C_VIN, C_VCC | as `G-48-S` | class | |
| R_REF · R_FB · R_TC | **6,04 kΩ · 150 kΩ · 1,5 MΩ**, 1 % | value | at `N_PS` 1/10, `V_F` 1,0 V |
| R_CL | **2× 2,67 kΩ, 2512, 2 W class** | value | 90 V, 1,5 W rated, 2,35 W at the corner |
| C_CL | **100 nF 1 kV X7R 1812** | class | |
| D_CL | **`V5N22-M3`** | code | |

## 3 · The rectifier and the output

| ref | part / value | how | note |
|---|---|---|---|
| D1 | **`US3M`** | code | 435–450 V reverse; 1000 V / 3 A ultrafast |
| C_OUT | **4× 100 nF 1 kV X7R 1812** | class | ~0,28 µF at 300 V, ripple ~1,5 % |
| D2 | **`US3M`** | code | the blocking diode, 1000 V against the ~350 V the tube's 650 V let-through stands over the bulk; `I_FSM` 100 A against the bulk's ~29 A discharge into a fired tube |

## 4 · The ladder — source end

| ref | part / value | how | note |
|---|---|---|---|
| GDT1 | **`2036-30-SM`** | code | 300 V ±20 %, 240 V minimum against 150 V a gap; holdover 135 V — **does not quench on 300 V**: the current limit holds the arc, `ENABLE` ends it |
| TVS1, TVS2 | **`5.0SMDJ350A`** ×2 | code | clamp ~565 V |
| L1, L2 | **22 µH**, surge-rated **5 A**, rated for 650 V across | value | |
| `FEED OUT` | **`DGPS2.5R-5.0`** ×2 | `10060008518` | 300 V out |
| `⏚` | the earth stud and strap | — | as `G-48-S` |

## 5 · Telemetry, island, connector — and the leak watch

**Block T on the output**: `R_SHUNT` **200 mΩ**, `R_VT` **3× 422 kΩ 1206 in series**, `R_VB` 100 kΩ.
**Block S.** **Block J**: `R_ID` **18,7 kΩ**, 0,65.

| ref | part / value | how | note |
|---|---|---|---|
| U_L | **`INA238`**, the second | code | `A1` to `VS`, `A0` from `A_SEL` → 0x44 / 0x45; `VBUS` only, shunt inputs tied, `ALERT` unwired — the host polls it |
| R_A | **4× 2,21 MΩ 1206, 1 %, in series** | value | + to the tube's centre, 8,84 MΩ; 37 V a part at the rail, 163 V at a gap's sparkover |
| R_B | **4× 1,96 MΩ 1206, 1 %, in series** | value | the centre to `VBUS`, 7,84 MΩ; with the pin's 1 MΩ the lower leg equals `R_A` and the centre sits at half the feed |
| C_L | **10 nF 50 V X7R 0603**, `VBUS` to `GND` | class | τ ≈ 9 ms |
| C_L2 | **100 nF 50 V X7R 0603** at `VS` | class | |

---

# `G-48-U` — power, unit, 48 V

The far end of a 48 V run. **A pressure board — solid parts only.** Takes the feed, makes 12 V
and nothing lower, hands it out on its two terminals, passes the feed on to the next unit. No buck.

## 1 · The input — 48 V off the cable

| ref | part / value | how | note |
|---|---|---|---|
| `FEED IN` | **`DGPS2.5R-5.0`** ×2 | `10060008518` | the feed in and on, straight through — the chain |
| TVS1, TVS2 | **`5.0SMDJ54A`** ×2 | code | clamp ~87 V — the whole ladder on this board; no tube, no chokes, no footprint for either |
| C_BULK | **3× 2,2 µF 100 V X7R 1210** | class | ~3,6 µF at 48 V; the run damps itself |

## 2 · The cell

| ref | part / value | how | note |
|---|---|---|---|
| U1 · Q1 | **`LT3748`** · **`ISC165N15NM6`** | code | 82 V on the plateau, 133 V at the clamp's corner, 151 V with a surge residue on the bulk — 1 V into the rated avalanche, `E_AS` 123 mJ |
| T1 | **`750311607`** | code | 2,5:1 · `L_PRI` 14 µH · `I_SAT` 9,5 A · 1500 V AC · 29,1 × 23,1 × 11,4 mm |
| R_SENSE | **15 mΩ, 2512, 1 W** | value | the largest the winding rule allows: 12,5 µH needed against 12,6 µH at −10 %. `I_pk` 6,0 A at 90 mV, 3,00 A at the declared ~25 W; the corner 7,33 A, 77 % of saturation, 0,81 W peak |
| R_REF · R_FB · R_TC | **6,04 kΩ · 165 kΩ · 66,5 kΩ**, 1 % | value | at `N_PS` 2,5, `V_F` 0,75 V; 194 µA through `R_FB` against the sheet's ~200 |
| R_C, C_C · C_SS · R_GATE | as `G-48-S` | class | |
| C_VIN · C_VCC | **2,2 µF 100 V X7R 1210 · 4,7 µF 50 V X7R 1206** | class | the gate drive comes off 48 V through the part's LDO, ~0,64 W at the light-load clamp |
| R_CL | **2× 845 Ω, 2512, 2 W class** | value | 60 V, 2,1 W rated, 4,1 W at the corner — a fault, until the `INA238` trips the source |
| C_CL | **2,2 µF 100 V X7R 1210** | class | |
| D_CL | **`V5N22-M3`** | code | |

## 3 · The rectifier and the output

| ref | part / value | how | note |
|---|---|---|---|
| D1 | **`V10P10-M3`** | code | 100 V / 10 A; `PIV` 32 V, 2,0 A average, 7,5 A secondary peak at the rated point, 18,3 A at the shunt's 110 mV corner — a fault, carried by its surge rating until the source trips |
| L_OUT | **100 µH** | value | on the 12 V output |
| C_OUT | **8× 10 µF 50 V X5R 1206** | class | ceramic only — 3,6 A rms shared eight ways |
| TVS3 | **`5.0SMDJ12A`** | code | on the 12 V output: breakdown 13,3–14,7 V, holds the rail under the 17 V of the smallest buck behind it below the cell's floor |
| `12V` | **`DGPS2.5R-5.0`** ×2 | `10060008518` | the 12 V out — the unit's rails, and the tap a bought device takes at ≤ 0,5 A |

## 4 · Telemetry, island, connector

**Block T on the input**: `R_SHUNT` **50 mΩ**, `R_VT` **90,9 kΩ 1206**, `R_VB` 100 kΩ — all ceramic
and solid, so the block is pressure-fit as it stands. **Block S**, ceramic. **Block J**: `R_ID`
**23,2 kΩ**, 0,70; `R_EN` to the input.

---

# `G-300-U-6` · `G-300-U-40` — power, unit, 300 V

**Two boards, one drawing, and what splits them is the enclosure.** `G-300-U-6` is a pressure
board, `G-300-U-40` is land only. `LT8316` + `FCD260N65S3` on both; the winding, `R_SNS`, `IREG/SS`,
the rectifier, the bank and `R_ID` differ.

## 1 · The input — 300 V off the cable

| ref | part / value | how | note |
|---|---|---|---|
| `FEED IN` | **`DGPS2.5R-5.0`** ×2 | `10060008518` | straight through |
| TVS1, TVS2 | **`5.0SMDJ350A`** ×2 | code | clamp ~565 V — the whole ladder; no tube, no chokes |
| C_BANK, `G-300-U-6` | **5× 1 µF 500 V X7R 2220** | class | ~2 µF at 300 V — pressure, ceramic |
| C_BANK, `G-300-U-40` | **4× 1 µF 630 V polypropylene film, 15 mm pitch + 100 nF 1 kV X7R 1812** | class | land only; a pressure build takes the ceramic bank and recalculates |

## 2 · The cell

| ref | part / value | how | note |
|---|---|---|---|
| U1 | **`LT8316`** | code | 16–560 V in, quasi-resonant boundary mode, 140 kHz clamp, burst to 3,5 kHz |
| Q1 | **`FCD260N65S3`**, D-PAK | code | 650 V; 402 V of it on `-40` at the steady rail; 472 V on `-6`, 550 V on its clamp |
| T1, `G-300-U-40` | **`11328-T078`** | code | Sumida PQ2620, 8:1:1, `N_TS` 1, 670 µH, 3,0 A, reinforced 3 kV, 31 × 28,5 × 23,5 mm, pins |
| T1, `G-300-U-6` | **`11338-T195`** | code | Sumida CEEH178, 14:1:1,7, 1000 µH ±10 %, 0,9 A, leakage 34 µH max, basic 3 kV, 19 × 17,4 × 8,6 mm, SMD |
| R_SNS | **58 mΩ** on `-40` · **130 mΩ** on `-6`, 1210 | value | `-40`: `I_pk` 1,55 A at 90 mV · `-6`: the largest the sampling floor allows, 0,33 A at full load |
| R_IREG | **76,8 kΩ** on `-40` (4,24 A) · **15,0 kΩ** on `-6` (0,65 A), 1 % | value | the output-current regulation point, `2,5 MΩ · I_OUT · R_SNS / N_PS` |
| R_FB1 · R_FB2 · R_TC, `G-300-U-40` | **10,0 kΩ · 90,9 kΩ · 249 kΩ**, 1 % | value | `N_TS` 1, `V_F` 0,3 V, the diode's TC −1,5 mV/°C; `R_TC` from TC to FB |
| R_FB1 · R_FB2 · R_TC, `G-300-U-6` | **10,0 kΩ · 162 kΩ · 261 kΩ**, 1 % | value | `N_TS` 1,7 — 12,04 V out; `BIAS` 20,9 V |
| D_BIAS · C_BIAS | **`PMEG10020ELR` · 4,7 µF 50 V X7R 1206** | code / class | `BIAS` off the tertiary, 12,3 V, inside the 10–30 V window |
| C_INTVCC | **4,7 µF 50 V X7R 1206** | class | |
| R_GATE | **10 Ω 0603** | class | |
| snubber, `G-300-U-40` | **RC footprint, not fitted** unless the drain ring asks for it; an RCD instead takes `US3M` | — | |
| D_CL · C_CL · R_CL, `G-300-U-6` | **`US3M` · 100 nF 1 kV X7R 1812 · 2× 38,3 kΩ 2512 in series** | code / class / value | the RCD clamp, fitted: 250 V above the input, 0,81 W — the winding's 34 µH of leakage |

## 3 · The rectifier and the output

| ref | part / value | how | note |
|---|---|---|---|
| D1, `G-300-U-40` | **`V10P10-M3`** | code | 100 V / 10 A; `PIV` 50 V, 3,7 A average — two thirds of the board's loss |
| D1, `G-300-U-6` | **`V10P10-M3`** | code | 100 V / 10 A; `PIV` 33 V, 0,5 A, 4,6 A peak |
| C_OUT, `G-300-U-6` | **8× 10 µF 50 V X5R 1206** | class | ceramic |
| C_OUT, `G-300-U-40` | **2× hybrid polymer 47 µF 50 V + 2× 10 µF 50 V X5R 1206** | class | land |
| TVS3 | **`5.0SMDJ12A`** | code | on the 12 V output, both boards — as on `G-48-U` |
| `12V` | **`DGPS2.5R-5.0`** ×2 | `10060008518` | 12 V out — 3,33 A on `-40` |

## 4 · Telemetry, island, connector

**Block T on the input**: `R_SHUNT` **200 mΩ**, `R_VT` **3× 422 kΩ 1206**, `R_VB` 100 kΩ. **Block S**
(ceramic on `-6`). **Block J**: `R_ID` **30,1 kΩ** (0,75) on `-6`, **40,2 kΩ** (0,80) on `-40`; `R_EN`
to the input.

---

# `G-12-S` — the isolated 12 V, source

The 12 V across a barrier for a bought ModBus device a few metres out: unregulated, follows the
pack, 0,4 A. No unit-end partner.

## 1 · The input

| ref | part / value | how | note |
|---|---|---|---|
| `12V` | **`DGPS2.5R-5.0`** ×2 | `10060008518` | input and tap; 0,5 A in |
| TVS_IN | **`5.0SMDJ14A`** | code | across `12V`, ahead of `L_IN` — stands off 14 V, conducts from 15,6–17,2 V; with the board's fuse in the enclosure's fuse field it clears a reversed pack or a held overvoltage |
| L_IN | **`SWPA252012S2R2MT`**, 2,2 µH | code | the house filter inductor, into `C_IN` |
| C_IN | **2× 10 µF 50 V X5R 1206 + 100 nF 50 V X7R 0603** at `VCC` | class | |

## 2 · The converter

| ref | part / value | how | note |
|---|---|---|---|
| U1 | **`SN6507`** | code | push-pull, 3–36 V in, 0,5 A; `ENABLE` on its `EN` |
| R_LIM | **34,8 kΩ 1 %** | value | ~0,7 A — above the switches' 0,5 A on purpose; the overload is block T's, not the OCP's |
| R_CLK | **7,87 kΩ 1 %** | value | ~1,26 MHz typical, ≥ 1,07 MHz — the winding's 7,5 Vµs asks ≥ 1,0 MHz at the lowest rate at 15 V |
| T1 | **`750319691`** | code | Würth, N 1,13, 7,5 Vµs, 2,5 kV, 8,5 × 12,9 × 5,2 mm |
| D1, D2 | **`PMEG10020ELR`** ×2 | code | `V_R` 51 V needed, 100 V fitted |
| C_OUT | **2× 10 µF 50 V X5R 1206 + 100 nF** | class | on the ~12 V |

## 3 · The ladder and the output

| ref | part / value | how | note |
|---|---|---|---|
| TVS1, TVS2 | **`5.0SMDJ18A`** ×2 | code | 18 V clears the 16,2 V full-pack light-load corner |
| L1, L2 | **22 µH**, surge-rated **5 A** | value | |
| GDT1 | **`2036-07-SM`** | code | to the common earthing point — the board never travels |
| `FEED OUT` | **`DGPS2.5R-5.0`** ×4 | `10060008518` | eight poles — four positions, 4× + and 4× −, the star of four sensor cables |

## 4 · Telemetry, island, connector

**Block T on the isolated output**: `R_SHUNT` **50 mΩ**, `VBUS` straight to the pin, no divider.
**Block S.** **Block J**: `R_ID` **56,2 kΩ**, 0,85; `R_EN` to ground.

---

# `G-24-S` — power, source, 24 V, switched

A switched 24 V across a barrier for a load that is not a sensor (a pump), on the host's dedicated power
socket; ~33 W; `ENABLE` is the switch and the board stands off between runs.

## 1 · The input

As `G-48-S`: **`DGPS2.5R-5.0`** ×2, `TVS_IN` **`5.0SMDJ14A`**, `L_IN` 1,5 µH, **`C_IN` 2× hybrid 47 µF 50 V + 2× 10 µF 50 V 1206**;
~3 A while the load runs.

## 2 · The cell

| ref | part / value | how | note |
|---|---|---|---|
| U1 · Q1 | **`LT3748`** · **`ISC165N15NM6`** | code | `EN/UVLO` from `ENABLE` through `R_EN` — the switch |
| T1 | **`750311592`** | code | 1:1:0,44 — the third winding open · `L_PRI` 8 µH ±10 % · `I_SAT` 18 A typ · leakage 0,4 µH max · 1500 V AC · SMD 32,3 × 27,0 × 13,7 mm |
| R_SENSE | **10 mΩ, 2512, 1 W** | value | `I_pk` 9,0 A at 90 mV, ~33 W at 24 V, 112 kHz at full load |
| R_REF · R_FB · R_TC | **6,04 kΩ · 124 kΩ · 124 kΩ**, 1 % | value | at `N_PS` 1, `V_F` 0,5 V |
| R_GATE · R_C, C_C · C_SS · C_VIN, C_VCC | as `G-48-S` | class | |
| R_CL | **2× 2,05 kΩ, 2512, 2 W class, in series** | value | 100 V, 2,4 W rated, 2,9 W at the corner — the winding's 0,4 µH of leakage |
| C_CL · D_CL | **100 nF 1 kV X7R 1812** · **`V5N22-M3`** | class / code | |

## 3 · The rectifier and the output

| ref | part / value | how | note |
|---|---|---|---|
| D1 | **`V10P10-M3`** | code | `PIV` 39 V at ~1,4 A |
| C_OUT | **4× 2,2 µF 100 V X7R 1210** | class | the 100 V part, 2,2× over the 45 V clamp |
| D2 | **`US3M`** | code | the blocking diode |

## 4 · The ladder and the output

| ref | part / value | how | note |
|---|---|---|---|
| GDT1 | **`2036-07-SM`** | code | quenches on 24 V as on 48 |
| TVS1, TVS2 | **`5.0SMDJ28A`** ×2 | code | 28 V is 1,17× the rail, clamp ~45 V |
| L1, L2 | **22 µH**, surge-rated **5 A** | value | |
| `FEED OUT` | **`DGPS2.5R-5.0`** | `10060008518` | one two-pole block, the load's 2-core |

## 5 · Telemetry, island, connector

**Block T on the output**: `R_SHUNT` **20 mΩ**, `VBUS` straight to the pin. **Block S.** **Block J**:
`R_ID` **4,32 kΩ**, 0,30; `R_EN` to ground — the switch.

---

# `G-I-N-025` — communication, 485 on copper, NodBus

Both ends of a run, one board: data full duplex, the echo receiver, the clock or the PPS on
channel B. No terminal for 12 V, no buck: 3,3 V from the host over the data connector.

## 1 · The connector and the island

| ref | part / value | how | note |
|---|---|---|---|
| `DATA` | **`BX2.54-2x6NA`** | code | the data connector, 12 pins; cable end `FC-12P` |
| R_ID | **6,65 kΩ 1 %** | value | 0,40 |
| R_LE | **100 kΩ 0603** | value | `LINE_EN` pull-down at the `SN6505B`'s `EN`; at a unit end pulled to run |
| block S | `SN6505B` · `750313734` · 2× `PMEG10020ELR` · the capacitors | — | `EN` = `LINE_EN`; ~100 mA of isolated 3,3 V at a source end |

## 2 · The line side

| ref | part / value | how | note |
|---|---|---|---|
| U2, U3 | **`ISO1452`** ×2 | code | data, both directions; and the echo-check receiver, its driver tied off, its UART TX unconnected |
| U4 | **`ISO1450`** | code | channel B, either way: `D` and `R` both on the `CLK/PPS` pin, `DE` and `RE#` both on `B_DIR` — `R` is at high impedance while the driver is on |
| C_D | **100 nF 50 V X7R 0603** ×4 | class | at each isolator's `VCC1` and `VCC2` |
| R_S1 … R_S6 | **6× 10 Ω, pulse-withstanding, 2512 anti-surge or MELF** | value | two per pair, on every board, always |
| TVS_P1 … TVS_P3 | **`SM712`** ×3 | code | across each pair |
| R_T1 … R_T3 · JP1 … JP3 | **3× 80,6 Ω 1 % 1206 · 3× 2-pin 2,54 mm header with a jumper** | value | one per pair; **the jumper fitted on the first and the last board of a segment, on no board between** — 80,6 + 2 × 10 is the ~100 Ω the UTP wants |
| GDT1 | **`2036-07-SM`** | code | the footprint on every board; the tube and its strap fitted at the source end only |
| `LINE CU` | **`DGPS2.5R-5.0`** ×4, 8 poles | `10060008518` | the four pairs, one pole to a net; a chained run twisted into the pole it shares |

---

# `G-I-M-005` — communication, 485 on copper, ModBus

`G-I-N-025` stripped to one transceiver on one pair, half duplex; the arm's 12 V is on `G-12-S`
beside it.

| ref | part / value | how | note |
|---|---|---|---|
| `DATA` · R_ID · R_LE | **`BX2.54-2x6NA`** · **8,25 kΩ** (0,45) · 100 kΩ | code / value | `B_DIR` lands on nothing |
| block S | as `G-I-N-025` | — | |
| U2 | **`ISO1450`** | code | `A`/`B`, `DE` from the host frame by frame |
| C_D | **100 nF** ×2 | class | |
| R_S1, R_S2 · TVS_P1 | **2× 10 Ω, pulse-withstanding · `SM712`** | value / code | |
| R_T1 · JP1 | **80,6 Ω 1 % 1206 · a 2-pin header with a jumper** | value | fitted on this board — the source end of the arm; the far end is the sensor's own |
| GDT1 | **`2036-07-SM`** | code | fitted and strapped — this board only ever stands at a source end |
| `LINE CU` | **`DGPS2.5R-5.0`** ×4, 8 poles | `10060008518` | four positions, 4× `A` and 4× `B` |

---

# `G-O-10-10` — communication, glass, 10 Mb/s

No barrier, no ladder, no island, no buck — the fibre isolates by construction. 1 oz stack.

| ref | part / value | how | note |
|---|---|---|---|
| `DATA` · R_ID | **`BX2.54-2x6NA`** · **10,0 kΩ** (0,50) | code / value | `LINE_EN` not populated |
| M1, M2 | **`OPT10-31103STR`** (duplex SC) or **`-PTR`** (FC pigtail) ×2 | code | 1310 nm, 10 km, 0–10 Mb/s, TTL, 3,3 V, industrial — **the grade letter `T`**; soldered, not socketed |
| L1 … L4 · C_π | **`SWPA252012S2R2MT`** ×4, the station's shielded 2,2 µH · **100 nF** on the host side and **10 µF 50 V X5R 1206 + 100 nF** at the pin, per `Vcc` | code / class | the module sheet's π filter on each `VccT` and `VccR` — the sheets ask a coil of ~1 µH, not a bead, where the supply is not quiet, and every host rail here is a buck at 2–2,7 MHz; the 10 µF is the local bulk for the keyed laser's step |
| U2 | **`74AUP1G126`** | code | from the `CLK/PPS` pin to channel B's `TD`, `OE` on `B_DIR` — active high |
| U5 | **`74AUP1G125`** | code | from channel B's `RD` to the pin, `OE` on `B_DIR` — active low |
| U3 · U4 | **`TPS22917`** on `VccT` · **`TPS22917L`** on `VccR` | code | both `ON` from `B_DIR` — a static select, not a switch |
| R_TD · R_RD | **100 kΩ 0603** ×2 | value | `TD` and the `125`'s input to ground, so neither floats behind a shut gate; `B_DIR` itself is tied in the socket |

---

# `G-O-2-100` — communication, glass, 2 Mb/s

The same board with the 2 Mb/s module seated; channel B turns round as on `G-O-10-10`, with the
same gates, load switches and resistors. One module, 1 m to 100 km — its receiver takes −39 to 0 dBm.
The catalogue modules are land parts; the pressure build is the same part obtained from the maker
in a pressure-tolerant form.

| ref | part / value | how | note |
|---|---|---|---|
| `DATA` · R_ID | **`BX2.54-2x6NA`** · **12,1 kΩ** (0,55) | code / value | |
| M1, M2 | **`OPT2-55A03STR`** ×2, or the BiDi pair **`OTB2-35A03STR` + `OTB2-53A03STR`** | code | 1550 nm DFB, 33 dB budget; the BiDi pair is matched and its ends are not interchangeable |
| L1 … L4 · C_π · U2 … U5 · R_TD · R_RD | as `G-O-10-10` | code / class | |
| — | a fixed optical attenuator in the connector on a run much shorter than the module's rating | — | a stocking note, not a board part: every 2 Mb/s module sits on its overload ceiling with no fibre in front of it |

★ N.I.C. ★

# Hermes — hardware

> **Design-stage concept — nothing built.** The figures are verified against the parts' sheets
> and the parts before a board is made.

**The board:** an STM32H523 (LQFP100), a `THVD1450`, a `TCAN334`, a `PCA9306`, the `ID`
resistor, four two-pole terminal blocks, two pin headers, the SWD header, the status LED and the
power body. Nothing else — no buck, no LDO,
no crystal, no I²C pull-up toward the head.

## Rails

| rail | source | load |
|---|---|---|
| **3,3 V** | the Mayak's, over the power body, behind the Mayak's 0,15 A polyfuse | the H523, the three transceivers, the pull-ups — **≤ 100 mA in all**, the budget on the pin |

No 12 V terminal and no regulator on the board. The `PCA9306`'s high side takes the bought
part's own rail on the I²C-out terminal, which is what the part exists for.

**Draw, by part:** the H523 at 8 MHz from flash ~1–2 mA · `THVD1450` receiving ~1 mA, in
shutdown (`RE#` high, `DE` low) microamps · `TCAN334` in standby (`STB` high) 15 µA below
85 °C, 20 µA above, 3,5 mA recessive and up to **55 mA on dominant bits** into 60 Ω — the one
figure that sets the budget. **The budget on the pin is 100 mA**, which leaves the parts' own
numbers room to move without moving the polyfuse, and the polyfuse's 0,15 A hold is one and a
half times it.

## Clock

**4 MHz on the CSI or 8 MHz on the HSI ÷ 8**; no crystal, no PLL. What runs: one I²C (slave to
the head), two USARTs (485, TTL), one I²C (out), the FDCAN — each clocked only while its face is
in use. Everything else off. The rates the faces carry: Modbus RTU up to 115 200 8N1 on the 485, a BMS's
UART at 9 600, CAN at 250 kb/s, the head's I²C 100–400 kHz — 4 MHz holds all but 400 kHz I²C.

## The processor — pins, timers, clock tree

*The record to draw the H523 from and to set CubeMX by. Package LQFP100, `STM32H523VE`; pin
numbers and alternate functions from DS14540 Rev 3, Table 13. Every pin below is either
assigned or listed free — nothing is left to guess.*

### Peripherals

| peripheral | instance | pins | job | mode |
|---|---|---|---|---|
| toward the head | **I2C1**, slave | PB8 `SCL` · PB9 `SDA` | the power body, the Mayak's LP I²C — the pull-ups are the Mayak's 4,7 kΩ; `ALERT` on PC8, push-pull, high = alarm | 100–400 kHz, the head's choice |
| 485 Modbus RTU | **USART1** | PB6 `TX` · PB7 `RX` · PE2 `DE` · PE3 `RE#` | the `THVD1450` — the MPPT, or a BMS with a 485 port, at the map's rate up to 115 200 | the standard face |
| UART TTL | **USART2** | PA2 `TX` · PA3 `RX` | a BMS's own UART, direct on the terminal | 3,3 V levels, 5 V-tolerant inputs |
| I²C out | **I2C3**, master | PD6 `SCL` · PD7 `SDA` | the `PCA9306`'s low side | 100 kHz |
| CAN | **FDCAN1** | PD1 `TX` · PD0 `RX` · PE5 `STB` | the `TCAN334`, asleep until a part wants it | classic CAN, 250 kb/s |
| clock | **CSI** or **HSI ÷ 8** | none | 4 or 8 MHz, no crystal, no PLL | |
| debug | **SWD** | PA13 · PA14 | | no JTAG, no trace |

The `ID` resistor, **90,9 kΩ to ground**, sits on the body's `ID` pin and no processor pin reads it; the Mayak does, against its 10 kΩ, as 0,90.

### Pins

| pin | port | signal on the board | AF / function | dir | note |
|---|---|---|---|---|---|
| 1 | PE2 | `DE_485` | GPIO | out | the `THVD1450`'s `DE` |
| 2 | PE3 | `RE_485` | GPIO | out | the `THVD1450`'s `RE#` — high with `DE` low is shutdown |
| 3 | PE4 | — | | | free |
| 4 | PE5 | `CAN_STB` | GPIO | out | the `TCAN334`'s `STB`, pin 8 — high is standby with wake |
| 5 | PE6 | — | | | free |
| 6 | VBAT | — | | | tied to `VDD`; no RTC anywhere |
| 7 | PC13 | — | | | free (tamper/RTC pin, unused) |
| 8 | PC14 | — | | | free — no 32 kHz crystal |
| 9 | PC15 | — | | | free |
| 10 | VSS | | | | |
| 11 | VDD | 3,3 V | | | |
| 12 | PH0 | — | |  | free — no crystal, the CSI or the HSI |
| 13 | PH1 | — | |  | free |
| 14 | NRST | reset | |  | 100 nF, no pull beyond the internal |
| 15 | PC0 | — | | | free (ADC) |
| 16 | PC1 | — | | | free (ADC) |
| 17 | PC2 | — | | | free (ADC) |
| 18 | PC3 | — | | | free (ADC) |
| 19 | VSSA | | | | |
| 20 | VREF− | | | | |
| 21 | VREF+ | 3,3 V through the shielded 2,2 µH, 10 µF + 100 nF at the pins — the ADC reference | | | |
| 22 | VDDA | 3,3 V through the shielded 2,2 µH, 10 µF + 100 nF at the pins | | | |
| 23 | PA0 | — | | | free (ADC) |
| 24 | PA1 | — | | | free (ADC) |
| 25 | PA2 | `TX_TTL` | `USART2_TX` | out | the UART terminal, direct |
| 26 | PA3 | `RX_TTL` | `USART2_RX` | in | 5 V-tolerant |
| 27 | VSS | | | | |
| 28 | VDD | 3,3 V | | | |
| 29 | PA4 | — | | | free (ADC) |
| 30 | PA5 | — | | | free (ADC) |
| 31 | PA6 | — | | | free (ADC) |
| 32 | PA7 | — | | | free (ADC) |
| 33 | PC4 | — | | | free (ADC) |
| 34 | PC5 | — | | | free (ADC) |
| 35 | PB0 | — | | | free (ADC) |
| 36 | PB1 | — | | | free (ADC) |
| 37 | PB2 | — | | | free |
| 38 | PE7 | — | | | free |
| 39 | PE8 | — | | | free |
| 40 | PE9 | — | | | free |
| 41 | PE10 | — | | | free |
| 42 | PE11 | — | | | free |
| 43 | PE12 | — | | | free |
| 44 | PE13 | — | | | free |
| 45 | PE14 | — | | | free |
| 46 | PE15 | — | | | free |
| 47 | PB10 | — | | | free |
| 48 | VCAP | 2× 1 µF 50 V 0805 + 2× 100 nF 50 V 0603 — 2,2 µF, the sheet's 2,2 µF ±20 % | | | |
| 49 | VSS | | | | |
| 50 | VDD | 3,3 V | | | |
| 51 | PB12 | — | | | free |
| 52 | PB13 | — | | | free |
| 53 | PB14 | — | | | free |
| 54 | PB15 | — | | | free |
| 55 | PD8 | — | | | free |
| 56 | PD9 | — | | | free |
| 57 | PD10 | — | | | free |
| 58 | PD11 | — | | | free |
| 59 | PD12 | — | | | free |
| 60 | PD13 | — | | | free |
| 61 | PD14 | — | | | free |
| 62 | PD15 | — | | | free |
| 63 | PC6 | — | | | free |
| 64 | PC7 | — | | | free |
| 65 | PC8 | `ALERT` | GPIO, push-pull | out | the power body — **high = alarm**, low otherwise; the Mayak's 100 kΩ to ground holds it low while the H523 is in reset |
| 66 | PC9 | — | | | free |
| 67 | PA8 | — | | | free |
| 68 | PA9 | — | | | free |
| 69 | PA10 | — | | | free |
| 70 | PA11 | — | | | free (USB, unused) |
| 71 | PA12 | — | | | free (USB, unused) |
| 72 | PA13 | `SWDIO` | SWD | i/o | |
| 73 | VDDUSB | 3,3 V | | | tied, USB unused |
| 74 | VSS | | | | |
| 75 | VDD | 3,3 V | | | |
| 76 | PA14 | `SWCLK` | SWD | in | |
| 77 | PA15 | — | | | free |
| 78 | PC10 | — | | | free |
| 79 | PC11 | — | | | free |
| 80 | PC12 | — | | | free |
| 81 | PD0 | `CAN_RX` | `FDCAN1_RX` | in | the `TCAN334`'s `RXD`, pin 4 |
| 82 | PD1 | `CAN_TX` | `FDCAN1_TX` | out | the `TCAN334`'s `TXD`, pin 1 |
| 83 | PD2 | — | | | free |
| 84 | PD3 | — | | | free |
| 85 | PD4 | `LED` | GPIO | out | the status LED, fitted — 1 kΩ from the 3,3 V, one 50 ms blink a minute while healthy, two on a fault (`../core/HARDWARE.md`) |
| 86 | PD5 | — | | | free |
| 87 | PD6 | `SCL_OUT` | `I2C3_SCL`, master | i/o | the `PCA9306`'s low side |
| 88 | PD7 | `SDA_OUT` | `I2C3_SDA`, master | i/o | |
| 89 | PB3 | — | | | free |
| 90 | PB4 | — | | | free |
| 91 | PB5 | — | | | free |
| 92 | PB6 | `TX_485` | `USART1_TX` | out | the `THVD1450`'s `D` |
| 93 | PB7 | `RX_485` | `USART1_RX` | in | the `THVD1450`'s `R` |
| 94 | BOOT0 | 10 kΩ to ground | |  | |
| 95 | PB8 | `SCL` | `I2C1_SCL`, slave | i/o | the power body — the Mayak's LP I²C |
| 96 | PB9 | `SDA` | `I2C1_SDA`, slave | i/o | |
| 97 | PE0 | — | | | free |
| 98 | VCAP | 2× 1 µF 50 V 0805 + 2× 100 nF 50 V 0603 — 2,2 µF, the sheet's 2,2 µF ±20 % | | | |
| 99 | VSS | | | | |
| 100 | VDD | 3,3 V | | | |

**17 GPIO used, 63 free** (PA0 · PA1 · PA4–PA12 · PA15 · PB0–PB5 · PB10 · PB12–PB15 · PC0–PC7 · PC9–PC15 · PD2 · PD3 · PD5 · PD8–PD15 · PE0 · PE4 · PE6–PE15 · PH0 · PH1).

### Timers

| timer | width | clock | channels used | role |
|---|---|---|---|---|
| **TIM6** | 16 | the core clock | none | the Modbus RTU frame gap (3,5 characters) and the poll cadence |
| all others | | | | free — nothing on this board is timed to a grid |

### Clock tree

| | |
|---|---|
| source | **CSI 4 MHz**, or **HSI 64 MHz ÷ 8 = 8 MHz** where the head runs its I²C at 400 kHz |
| PLL | none |
| SYSCLK, AHB, APB1/2 | the source, undivided |
| peripheral kernels | each face's clock enabled only while the face is in use |

No crystal and no `OSC_IN`: the rates this board carries — 115 200, 9600, 250 kb/s, 100–400 kHz —
all sit under the CSI's tolerance, and the board has no time role.

### Analogue and reference

**Every filter inductor on this board is a π filter: 10 µF 1206 + 100 nF 0603 before it on the rail and
the same pair at the pins behind it** — the house rule (`../core/POWER.md`, *The filter parts*).

`VDDA` and `VREF+` on the 3,3 V through the shielded 2,2 µH `SWPA252012S2R2MT`, 10 µF + 100 nF; nothing is measured on this
board and no ADC channel is used.

## Transceivers

| face | part | supply | control | fault tolerance on the bus pins |
|---|---|---|---|---|
| 485 | `THVD1450` (TI, SOIC-8), behind 2× 10 Ω and an `SM712` at the terminal | 3,3 V | `RE#` high + `DE` low = shutdown | ±18 V on `A`/`B` |
| CAN | `TCAN334` (TI, SOIC-8), behind 2× 10 Ω and a `PESD2CAN` at the terminal | 3,3 V | `STB` high = standby, 15 µA, a bus wake-up pattern driving `RXD` low; `STB` low = normal; `SHDN` (pin 5) tied to ground | ±14 V on `CANH`/`CANL`, ±12 V common mode, ±12 kV IEC 61000-4-2 on the bus pins, dominant time-outs on `TXD` and `RXD` |
| I²C out | `PCA9306` (TI) | 3,3 V low side, the bought part's rail high side | — | — |

**Every bus that meets a cable carries the house three parts, in the house order** — from the
terminal inward the transil, the series resistors, the transceiver (`../core/HARDWARE.md`): on the
485 pair an `SM712` and 2× 10 Ω, on the CAN pair a `PESD2CAN` and 2× 10 Ω. **CAN termination:**
**100 Ω across `CANH`/`CANL` behind a jumper, on the transceiver side of the 10 Ω pair, so the bus
sees its 120 Ω**; fitted when Hermes is one end of the bus, which on an in-box cable to one BMS it
is. **485:** no termination and no bias on the board; the cable is in-box and short, and the
bought unit carries its own.

**3,3 V is enough on every face and no face carries a supply.** 485 is differential — a 3,3 V
transceiver drives ≥ 1,5 V into a receiver that thresholds at ±200 mV, whatever the far end runs
on. CAN is the same: recessive 2,5 V, dominant 3,5 V / 1,5 V, and the `TCAN334` is interoperable
with 5 V transceivers. A CAN bus is `CANH · CANL · GND` and nothing else; the BMS and the MPPT
power themselves from the pack. A supply pin a unit puts on its connector — many an MPPT carries +5 V on its RJ45 — is left unconnected.

**The faces are not isolated, and the bought parts must be common-negative.** The BMS, the MPPT
and Hermes stand in the one enclosure on the pack's negative, which is the station's ground, so a
barrier on the 485 or the CAN would separate a ground that is joined on the far side anyway
(`WHY.md`). An MPPT or a BMS whose communication ground is its positive rail — a common-positive
regulator — is not a part for this card.

## Terminals

| terminal | part | pins | goes to |
|---|---|---|---|
| 485 | 2× **`DGPS2.5R-5.0`**, three of four poles | `A · B · GND` | the MPPT's or the BMS's 485 — a unit's RJ45 cut to bare wires, any supply pin on it left unconnected |
| CAN | 2× **`DGPS2.5R-5.0`**, three of four poles | `CANH · CANL · GND` | a part with no other bus |
| UART | 2,54 mm pin header, 1×3 | `TX · RX · GND` | a BMS's UART port |
| I²C out | 2,54 mm pin header, 1×4 | `SDA · SCL · V_ref · GND` — `V_ref` is the bought part's rail into the `PCA9306`'s high side | a part that offers I²C |

The field terminals are the family's one block, `DGPS2.5R-5.0`, two poles each, at the card's right
edge; the power body, the card's input, at the left (`../galvani/README.md`, *The connectors*).
The UART and the I²C out are headers because a bought BMS's UART and an I²C part come
on a flying lead with a crimped plug.

## The power body

The Galvani **power body**, 8-pin 2×4 `BX2.54-2x4NA`, pin for pin as every power body. Hermes is
not a power board: `ENABLE` and `A_SEL` land on nothing, and its I²C address is its own.

| pin | signal | on this board |
|---|---|---|
| 1 | `ENABLE` | not connected — the link is never gated |
| 2 | `GND` | ground |
| 3 | `ID` | **90,9 kΩ to ground** — 0,90 against the Mayak's 10 kΩ 1 % |
| 4 | `SDA` | PB9, `I2C1_SDA` slave — no pull-up on this board, the Mayak's 4,7 kΩ holds the bus |
| 5 | `A_SEL` | not connected — the Mayak straps it to ground; nothing here reads it |
| 6 | `SCL` | PB8, `I2C1_SCL` slave |
| 7 | `ALERT` | PC8, **push-pull, high = alarm** — into the Mayak's `GPIO1`, 100 kΩ to ground there |
| 8 | `3,3 V` | the board's whole supply, from the Mayak's rail behind its 0,15 A polyfuse |

## Bench criteria

- Draw on the 3,3 V pin, every face fitted and the 485 polling at 115 200: **≤ 100 mA**.
- The `ID` pin reads **0,90 ± 0,025** of the Mayak's full scale.
- One block read a minute from the head completes at 100 kHz in under 10 ms.
- `ALERT` rises within one poll cycle of a written threshold being crossed.

## Parts

| part | qty | where |
|---|---|---|
| `STM32H523VE`, LQFP100 | 1 | the processor — no crystal |
| 2× 1 µF 50 V 0805 + 2× 100 nF 0603 | 2 sets | `VCAP` |
| 100 nF 50 V | one a supply pin | every `VDD`, the three transceivers |
| 10 µF 50 V | 1 | the processor's bulk |
| 100 nF · 10 kΩ | 1 each | `NRST` · `BOOT0` to ground |
| 2,2 µH `SWPA252012S2R2MT` + 2× (10 µF + 100 nF) | 1 set | `VDDA` / `VREF+`, π |
| `THVD1450` · 2× 10 Ω · `SM712` | 1 set | the 485 face |
| `TCAN334` · 2× 10 Ω · `PESD2CAN` · 100 Ω + a 2-pin jumper | 1 set | the CAN face and its termination |
| `PCA9306` + 2× 4,7 kΩ | 1 | the I²C out, the pull-ups on the low side |
| 90,9 kΩ | 1 | `ID`, 0,90 |
| 1 kΩ + LED | 1 | the status LED on PD4 |
| `DGPS2.5R-5.0`, two-pole | 4 | the 485 and the CAN terminals |
| 2,54 mm pin headers, 1×3 · 1×4 | 1 each | the UART · the I²C out |
| `BX2.54-2x4NA` | 1 | the power body |
| 6-pin header | 1 | SWD |

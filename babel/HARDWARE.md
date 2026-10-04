★ N.I.C. ★

# Babel — the board

> **Design-stage concept — nothing built.** The figures are verified against the parts' sheets
> and the parts before a board is made.

Parts, values and pins are the house's (`../galvani/HARDWARE.md`, *The buck cell*; `../core/blocks/modbus.md`, the basic
set; `../core/POWER.md`, the capacitor and filter rules).

## 1. What is on it

| part | job |
|---|---|
| `STM32H523VE`, LQFP100 | the Modbus slaves, the sensor drivers |
| `LMR43610R3RPER` | 12 V → 3,3 V, the one rail; 36 V input, so it outlives the transils' ~29 V clamp |
| `THVD1450` + 2× 10 Ω + `SM712` on the pair · 2× `5.0SMDJ18A` on the 12 V input | the basic set every MOD carries |
| 2× `SWPA252012S2R2MT`, 2,2 µH | the sensor rail's filter and the `VDDA`/`VREF+` filter, π each |
| `TPD4E05U06` | one ESD channel per header signal line, with 33 Ω at the processor — the house rule for a line that leaves the board (`../core/HARDWARE.md`) |
| `SWXBEABVF0-16.777216` | the house crystal, 2²⁴ |

## 2. Power

The arm hands the board its isolated 12 V, and the board makes one rail from it.

```
 12 V, the arm ─▶ 2× 5.0SMDJ18A ─▶ LMR43610 ─▶ 3,3 V ─┬─▶ STM32H523 · THVD1450
                                                        ├─▶ 2,2 µH ─▶ VDDA / VREF+, 10 µF + 100 nF both sides
                                                        └─▶ 2,2 µH ─▶ the header's 3,3 V, 10 µF + 100 nF both sides
```

**The buck**: `RT` 7,50 kΩ → 2,08 MHz, auto mode by the `R3` order code; 4,7 µH shielded,
`I_SAT` ≥ 3,5 A; `R_FBT` 28,0 kΩ / `R_FBB` 12,1 kΩ → 3,31 V, `C_FF` 22 pF C0G across `R_FBT`;
`C_IN` 4,7 µF + 100 nF at `VIN`; `C_OUT` 3× 10 µF 1206 + 100 nF; `VCC` 1 µF; `BOOT` 100 nF to `SW`;
`PGOOD` to PD14 through its pull-up.

**The sensor rail** is the 3,3 V behind the 2,2 µH, 10 µF + 100 nF before it and at the header, so the
transceiver's and the processor's steps do not land on the sensor. It is 3,3 V, not switched, and
it is the whole of what the header supplies: a sensor that wants 5 V, an enable, a clean analogue
rail or a second bus gets it from the builder who adapts the board, on the free pins. This
document is the base.

**Decoupling** — the three rules of every MOD: ① the 12 V side is 50 V and its bulk is a hybrid polymer — 47 µF 50 V + 2× 10 µF 50 V
1206 + 100 nF behind the transils; ② below the buck every capacitor is a 50 V X7R — 100 nF at
every `VDD` of the H523 with 10 µF of bulk, 100 nF + 10 µF at the `THVD1450`, `VCAP` 2× 1 µF 50 V
0805 + 2× 100 nF 50 V 0603 on each of pins 48 and 98, 2,2 µF; ③ every timing position is C0G — the crystal's 12 pF pair, `C_FF`.

## 3. The terminal

**Connectors: `MB IN` — the arm's four wires; the sensor positions by their sensor** (`../galvani/README.md`, *Connector names*).

**Four poles on two `DGPS2.5R-5.0`** — 5 mm, 0,75–2,5 mm² — in the order **`A` · `B` ·
`GND` · 12 V**, the family's order on every MOD: ground stands between the pair and the supply
because the `SM712` holds off +12 V only and the arm's unregulated 12 V reaches 16,2 V, so a cable
stripped one pole off can never land the supply on `B`. A MOD is an end station: one cable, four
wires, nothing carries on. No Galvani body, no clock, no telemetry pin.

The line front: `THVD1450` on USART1, `DE` on PE2, `RE#` tied low — the receiver is never off;
2× 10 Ω in series with the pair and the `SM712` across it on the terminal side.

## 4. The sensor header

**The header is a 2,54 mm pin header, 2×7: the twelve signal lines, 3,3 V and GND.** Every interface
has its own peripheral and its own pins, so a sensor may use any of them at once.

| interface | pins | on board |
|---|---|---|
| I²C | `SCL_S` PD6 · `SDA_S` PD7 — I2C3 | 4,7 kΩ pull-ups to the sensor rail; 33 Ω at the processor |
| SPI | `SCK_S` PA5 · `MISO_S` PA6 · `MOSI_S` PA7 — SPI1 · `CS_S` PA4 | 33 Ω at the processor |
| UART | `TX_S` PA2 · `RX_S` PA3 — USART2 | 33 Ω at the processor |
| 1-Wire | `DQ` PD3, open drain | 4,7 kΩ pull-up; 33 Ω at the processor |
| GPIO | `GPIO_S1` PE5 · `GPIO_S2` PE6 | 33 Ω at the processor |
| clock | `CLK_S` PA8 — MCO1 | a power of two off the PLL, 2²⁰…2²⁴, for a sensor that needs one supplied; 33 Ω at the processor |
| supply | 3,3 V · GND | the filtered rail, §2 |

**Every signal line that leaves this board to a sensor carries two things, the house rule for a
bus line on a connector (`../core/HARDWARE.md`, *Every bus that meets a cable or a connector*): in this order from the header inward, one channel of a low-capacitance ESD array, `TPD4E05U06` — 0,5 pF a
line, 5,5 V working, IEC 61000-4-2 level 4 — at the header, then 33 Ω in series, then the processor pin. The count of arrays follows the lines fitted. That
is ESD only: the surge path stops on the arm's Galvani boards, and what reaches this board is what
the basic set takes.

## 5. The processor — pins, timers, clock tree

*Package LQFP100, `STM32H523VE`; pin numbers and alternate functions from DS14540 Rev 3, Table
13. Every pin is assigned or listed free.*

### Peripherals

| peripheral | instance | pins | job | mode |
|---|---|---|---|---|
| the arm | **USART1** | PB6 `TXD` · PB7 `RXD` · PE2 `DE` | the `THVD1450`; one Modbus slave per fitted position on the one line | RTU, 19 200 8N1 (9 600 on `RATE`); `DE` by `DEAT`/`DEDT` |
| sensor I²C | **I2C3** | PD6 · PD7 | the header | 100 or 400 kHz, the profile's |
| sensor SPI | **SPI1** + `CS` PA4 | PA5 · PA6 · PA7 | the header | the profile's mode |
| sensor UART | **USART2** | PA2 · PA3 | the header | the profile's rate |
| 1-Wire | GPIO, open drain | PD3 | the header | bit-banged on TIM6 |
| general | GPIO | PE5 · PE6 | the header | the profile's |
| a clock out | **MCO1** | PA8 | the header | 2²⁰…2²⁴ off the PLL |
| clock | **HSE crystal** | PH0 · PH1 | 2²⁴ | |
| power good | GPIO | PD14 | the buck's `PGOOD` | |
| LED | GPIO | PD4 | the status LED | |
| debug | **SWD** | PA13 · PA14 | | no JTAG, no trace |

### Pins

| pin | port | signal on the board | AF / function | dir | note |
|---|---|---|---|---|---|
| 1 | PE2 | `DE` | GPIO | out | the `THVD1450`'s `DE`; `RE#` tied low |
| 2 | PE3 | — | | | free |
| 3 | PE4 | — | | | free |
| 4 | PE5 | `GPIO_S1` | GPIO | i/o | the header |
| 5 | PE6 | `GPIO_S2` | GPIO | i/o | the header |
| 6 | VBAT | — | | | tied to `VDD`; no RTC anywhere |
| 7 | PC13 | — | | | free |
| 8 | PC14 | — | | | free — no 32 kHz crystal |
| 9 | PC15 | — | | | free |
| 10 | VSS | | | | |
| 11 | VDD | 3,3 V | | | |
| 12 | PH0 | `OSC_IN` | HSE crystal | | 2²⁴ = 16,777216 MHz, 12 pF C0G |
| 13 | PH1 | `OSC_OUT` | HSE crystal | | 12 pF C0G |
| 14 | NRST | reset | | | 100 nF, no pull beyond the internal |
| 15 | PC0 | — | | | free (ADC) |
| 16 | PC1 | — | | | free (ADC) |
| 17 | PC2 | — | | | free (ADC) |
| 18 | PC3 | — | | | free (ADC) |
| 19 | VSSA | | | | |
| 20 | VREF− | | | | |
| 21 | VREF+ | 3,3 V through the shielded 2,2 µH `SWPA252012S2R2MT`, 10 µF + 100 nF both sides | | | the house filter on every `VDDA`/`VREF+`, used or not |
| 22 | VDDA | the same node | | | |
| 23 | PA0 | — | | | free (ADC) |
| 24 | PA1 | — | | | free (ADC) |
| 25 | PA2 | `TX_S` | `USART2_TX` | out | the header — UART |
| 26 | PA3 | `RX_S` | `USART2_RX` | in | |
| 27 | VSS | | | | |
| 28 | VDD | 3,3 V | | | |
| 29 | PA4 | `CS_S` | GPIO | out | the header — SPI |
| 30 | PA5 | `SCK_S` | `SPI1_SCK` | out | |
| 31 | PA6 | `MISO_S` | `SPI1_MISO` | in | |
| 32 | PA7 | `MOSI_S` | `SPI1_MOSI` | out | |
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
| 61 | PD14 | `PGOOD` | GPIO | in | the buck's window |
| 62 | PD15 | — | | | free |
| 63 | PC6 | — | | | free |
| 64 | PC7 | — | | | free |
| 65 | PC8 | — | | | free |
| 66 | PC9 | — | | | free |
| 67 | PA8 | `CLK_S` | `MCO1` | out | the header — a clock for a sensor that needs one, 2²⁰…2²⁴ |
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
| 81 | PD0 | — | | | free |
| 82 | PD1 | — | | | free |
| 83 | PD2 | — | | | free |
| 84 | PD3 | `DQ` | GPIO, open drain | i/o | the header — 1-Wire, 4,7 kΩ up |
| 85 | PD4 | `LED` | GPIO | out | the status LED, 1 kΩ from the 3,3 V — one 50 ms blink a minute while healthy, two on a fault (`../core/HARDWARE.md`) |
| 86 | PD5 | — | | | free |
| 87 | PD6 | `SCL_S` | `I2C3_SCL` | i/o | the header — I²C, 4,7 kΩ up |
| 88 | PD7 | `SDA_S` | `I2C3_SDA` | i/o | 4,7 kΩ up |
| 89 | PB3 | — | | | free |
| 90 | PB4 | — | | | free |
| 91 | PB5 | — | | | free |
| 92 | PB6 | `TXD` | `USART1_TX` | out | the `THVD1450`'s `D` |
| 93 | PB7 | `RXD` | `USART1_RX` | in | the `THVD1450`'s `R` |
| 94 | BOOT0 | 10 kΩ to ground | | | |
| 95 | PB8 | — | | | free |
| 96 | PB9 | — | | | free |
| 97 | PE0 | — | | | free |
| 98 | VCAP | 2× 1 µF 50 V 0805 + 2× 100 nF 50 V 0603 — 2,2 µF, the sheet's 2,2 µF ±20 % | | | |
| 99 | VSS | | | | |
| 100 | VDD | 3,3 V | | | |

**21 GPIO used, 59 free** (PA0 · PA1 · PA9–PA12 · PA15 · PB0–PB5 · PB8–PB10 · PB12–PB15 · PC0–PC15 · PD0–PD2 · PD5 · PD8–PD13 · PD15 · PE0 · PE3 · PE4 · PE7–PE15).

### Timers

| timer | bits | kernel | pins | job |
|---|---|---|---|---|
| **TIM6** | 16 | 2²⁷ ÷ prescaler | none | the RTU frame gap, the 1-Wire bit timing, the profiles' conversion waits |
| all others | | | | free — nothing on this board is timed to a grid; a polled arm carries no clock and the host stamps the value when it polls |

### Clock tree

| | |
|---|---|
| HSE | the house crystal, 2²⁴ = 16,777216 MHz — `SWXBEABVF0-16.777216`, Starwave XB 5032, ±20 ppm over −40…+85 °C, `C_L` 10 pF; **12 pF C0G** on each side (`../core/blocks/clocks.md`) |
| PLL1 | **M 2 · N 32 · P 2** — the PLL input is 2²³ because `f_PLL_IN` is 2…16 MHz (DS14540 Rev 3, Table 46); VCO 2²⁸, `P` 2²⁷ |
| SYSCLK, AHB, APB1/2, the timer kernel | **2²⁷ = 134,217728 MHz** |
| MCO1 | PA8, 2²⁰…2²⁴ off the PLL, a power of two — on when a profile asks |

## 6. Bring-up

SWD, NRST and BOOT0 populated; the LED fitted; nothing is DNP (`../core/HARDWARE.md`).

## 7. Parts

| part | qty | where |
|---|---|---|
| `STM32H523VE`, LQFP100 | 1 | the processor |
| `SWXBEABVF0-16.777216` + 2× 12 pF C0G | 1 set | HSE |
| 2× 1 µF 50 V 0805 + 2× 100 nF 50 V 0603 | 2 sets | `VCAP` |
| 100 nF 50 V | one a supply pin | every `VDD` |
| 10 µF 50 V | 1 | the processor's bulk |
| 2,2 µH `SWPA252012S2R2MT` + 2× (10 µF + 100 nF) | 1 set | `VDDA` / `VREF+`, π |
| 100 nF · 10 kΩ | 1 each | `NRST` · `BOOT0` to ground |
| `LMR43610R3RPER` | 1 | the 3,3 V |
| 4,7 µH shielded, `I_SAT` ≥ 3,5 A | 1 | the buck's inductor |
| 28,0 k · 12,1 k · 22 pF C0G · 7,50 kΩ | 1 set | the divider, `C_FF`, `RT` for 2,08 MHz |
| 4,7 µF 50 V + 100 nF · 1 µF · 100 nF | 1 set | `VIN` · `VCC` · `BOOT` |
| 3× 10 µF 50 V 1206 + 100 nF | 1 set | the 3,3 V node |
| 100 kΩ | 1 | `PG` up, to PD14 |
| 47 µF 50 V hybrid polymer + 2× 10 µF 50 V 1206 + 100 nF | 1 set | the 12 V input, behind the transils |
| `5.0SMDJ18A` | 2 | the 12 V input |
| `THVD1450` · 2× 10 Ω · `SM712` | 1 set | the line front |
| 100 nF + 10 µF | 1 set | the transceiver's supply |
| 2,2 µH `SWPA252012S2R2MT` + 2× (10 µF + 100 nF) | 1 set | the sensor rail at the header, π |
| 4,7 kΩ | 3 | I2C3's two pull-ups, the 1-Wire pull-up |
| 33 Ω | one a header signal line | at the processor pin, twelve lines |
| `TPD4E05U06` | one channel a header signal line | at the header |
| 1 kΩ + LED | 1 | the status LED on PD4 |
| `DGPS2.5R-5.0`, two-pole | 2 | `MB IN` |
| 2,54 mm pin header, 2×7 | 1 | the sensor header |
| 6-pin header | 1 | SWD |

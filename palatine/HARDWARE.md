★ N.I.C. ★

# Palatine — hardware

> **Design-stage concept — nothing built.** The figures are verified against the parts' sheets and
> the parts before a board is made.

## What is on it

| part | job |
|---|---|
| **`STM32H523VE`**, LQFP100 | the ModBus master of four arms and the NodBus unit toward its card. No analogue front, no sensor of its own, no SPI or ADC measurement — every value arrives over an arm |
| **`LMR43610`**, 3,3 V | the one buck: the MCU, the NodBus communication board and the four ModBus communication boards hang on it (*The rails*) |
| **four Galvani port positions** | one per arm, each a 12-pin data body and an 8-pin power body; the arm's communication board and power board plug in, and the line side, the barrier and the sensors' isolated 12 V are theirs |
| **`PWR EXT`** | a fifth power body on its own — the switched source board for Pluvius's pump, `EN_X` its switch, its `INA238` the load's ammeter |
| **the up port** | the unit's own two bodies toward its card, or the crossed in-box cable |
| **2× `DGPS2.5R-5.0`** | the 12 V in and the tap, one node; nothing of the 12 V crosses the board |
| LED on PD4, button on PD5 | the status blink; the commissioning button |

**The rain gauge is not on this board.** Pluvius is a ModBus module on an arm with its own cell,
vessel and pump (`../pluvius/`); Palatine polls it and gets grams and a status word like any
bought sensor.

## The processor — pins, timers, clock tree

Package LQFP100, `STM32H523VE`; pin numbers and alternate functions from DS14540 Rev 3, Table 13.
Every pin is assigned or listed free.

### Peripherals

| peripheral | instance | pins | job | mode |
|---|---|---|---|---|
| the NodBus link | **USART1** + **TIM4** | PB6 `TXD` · PB7 `RXD` | the spur to its card, 8N1 at the rung `BUSCFG` hands down — 2²⁰ alone, 2²¹ chained | the pins switch USART → TIM4 for the ranging instant |
| the frame anchor | **TIM2** (32-bit) | PB3 ← `RXD` on CH2 | the card's frame start on the grid | never reset, read as differences |
| echo receiver | **USART3**, RX only | PD9 `RXD_ECHO` | hears the board's own frames | |
| arm 1 | **USART2** | PA2 `TXD` · PA3 `RXD` · PE5 `DE` · PE6 `EN` | ModBus RTU master, the arm's communication and power boards | 19 200 8N1, the rate a table entry per arm |
| arm 2 | **USART6** | PC6 · PC7 · PE7 · PE8 | same | |
| arm 3 | **UART4** | PC10 · PC11 · PE10 · PE12 | same | |
| arm 4 | **UART5**, `SWAP` set | PB12 · PB13 · PE13 · PE14 | same | |
| `PWR EXT` — the switched supply body | GPIO · **ADC1** · **I2C1** | PD1 `EN_X` · PA0 `ID_X` | **the fifth power body, on its own**: a source power board switched by `ENABLE` for Pluvius's pump, on the switched 24 V board (`../galvani/README.md`) | `EN_X` carries a 100 kΩ pull-down — a reset leaves the load off; the board's `INA238` on I2C1 beside the up port's, its `ALERT` a pin of its own |
| power bodies | **I2C1 · I2C3 · I3C1**, master on each | I2C1 PB8 `SCL` · PB9 `SDA` · I2C3 PD6 `SCL` · PD7 `SDA` · **I3C1 PD12 `SCL` · PD13 `SDA`, in I²C legacy** | six `INA238`s — the up port's, the four arm power boards and the `PWR EXT` body's board — **two sockets to a controller, and two is the ceiling**: `A_SEL` is one bit, so a bus carries 0x40 and 0x41 and nothing more (`../galvani/README.md`). The up port and `PWR EXT` on I2C1, arms 1 and 2 on I2C3, arms 3 and 4 on I3C1; in each pair the first socket's `A_SEL` is strapped low and the second's high. **I2C2 does not exist on this board**: its only `SCL` pad is PB10 and both its `SDA` pads are taken — PB12 by arm 4's `TXD`, PB3 by the `RXD` capture — so the third controller is I3C1, which is I²C-capable and has PD12/PD13 to itself | 100 kHz; **an `ALERT` pin per socket, six of them, no wire-OR**: `ALERT_P` PC8 · `ALERT_1` PC9 · `ALERT_2` PD10 · `ALERT_3` PD11 · `ALERT_4` PC12 · `ALERT_X` PD2 — **EXTI 8 · 9 · 10 · 11 · 12 · 2, six different numbers**, because EXTI is shared by pin number across ports and the board's seventh EXTI user is `BUTTON` on PD5 |
| ID reads | **ADC1** | eleven channels, below | one resistor per body: two on the up port, two per arm, one on `PWR EXT` | read once at bring-up, before a body is fed |
| clock in | **HSE bypass** | PH0 | the data body's `CLK`, 2²² | PH1 unconnected |
| debug | **SWD** | PA13 · PA14 | | no JTAG, no trace |

Six of the seven UARTs; the LPUART is free. An arm's `DE` is a GPIO like every driver enable in
the station, and an arm's kill is one GPIO into both of its bodies (*The rails*). No timer channel
serves an arm: ModBus carries no clock and is never ranged, and TIM6 times the RTU gaps on all four.

### Pins

| pin | port | signal on the board | AF / function | dir | note |
|---|---|---|---|---|---|
| 1 | PE2 | `DE` | GPIO | out | the data body's driver enable |
| 2 | PE3 | — | | | free |
| 3 | PE4 | `SD` | GPIO | in | the up port's data body, optical no-light — 100 kΩ to ground |
| 4 | PE5 | `DE_A1` | GPIO | out | arm 1 driver enable |
| 5 | PE6 | `EN_A1` | GPIO | out | arm 1: the power board's `ENABLE` and the communication board's `LINE_EN`, one net |
| 6 | VBAT | — | | | tied to `VDD`; no RTC anywhere |
| 7 | PC13 | — | | | free (tamper/RTC pin, unused) |
| 8 | PC14 | — | | | free — no 32 kHz crystal |
| 9 | PC15 | — | | | free |
| 10 | VSS | | | | |
| 11 | VDD | 3,3 V | | | |
| 12 | PH0 | `CLK_IN` | `OSC_IN`, bypass | in | the data body's `CLK`, 2²² |
| 13 | PH1 | — | |  | unconnected in bypass mode |
| 14 | NRST | reset | |  | 100 nF, no pull beyond the internal |
| 15 | PC0 | `ID_D` | `ADC12_INP10` | in | the data body's `ID` |
| 16 | PC1 | — | | | free |
| 17 | PC2 | `ID_A1D` | `ADC12_INP12` | in | arm 1, data body |
| 18 | PC3 | `ID_A1P` | `ADC12_INP13` | in | arm 1, power body |
| 19 | VSSA | | | | |
| 20 | VREF− | | | | |
| 21 | VREF+ | 3,3 V through the shielded 2,2 µH, 10 µF + 100 nF — the ADC reference | | | |
| 22 | VDDA | the same node as `VREF+` | | | |
| 23 | PA0 | `ID_X` | `ADC12_INP0` | in | the `PWR EXT` body's `ID` — the switched supply board |
| 24 | PA1 | — | | | free (ADC) |
| 25 | PA2 | `TXD_A1` | `USART2_TX` / `TIM15_CH1` | out | ModBus arm 1 |
| 26 | PA3 | `RXD_A1` | `USART2_RX` / `TIM15_CH2` | in | RTU, 19 200 8N1 |
| 27 | VSS | | | | |
| 28 | VDD | 3,3 V | | | |
| 29 | PA4 | `ID_A3D` | `ADC12_INP18` | in | |
| 30 | PA5 | `ID_A3P` | `ADC12_INP19` | in | |
| 31 | PA6 | `ID_A4D` | `ADC12_INP3` | in | |
| 32 | PA7 | `ID_A4P` | `ADC12_INP7` | in | |
| 33 | PC4 | `ID_A2D` | `ADC12_INP4` | in | |
| 34 | PC5 | `ID_A2P` | `ADC12_INP8` | in | |
| 35 | PB0 | — | | | free (ADC) |
| 36 | PB1 | `ID_P` | `ADC12_INP5` | in | the up port's power body `ID` |
| 37 | PB2 | — | | | free |
| 38 | PE7 | `DE_A2` | GPIO | out | |
| 39 | PE8 | `EN_A2` | GPIO | out | |
| 40 | PE9 | — | | | free |
| 41 | PE10 | `DE_A3` | GPIO | out | |
| 42 | PE11 | — | | | free |
| 43 | PE12 | `EN_A3` | GPIO | out | |
| 44 | PE13 | `DE_A4` | GPIO | out | |
| 45 | PE14 | `EN_A4` | GPIO | out | |
| 46 | PE15 | — | | | free |
| 47 | PB10 | — | | | free |
| 48 | VCAP | 2× 1 µF 50 V 0805 + 2× 100 nF 50 V 0603 — 2,2 µF, the sheet's 2,2 µF ±20 % | | | |
| 49 | VSS | | | | |
| 50 | VDD | 3,3 V | | | |
| 51 | PB12 | `TXD_A4` | `UART5_TX` (swapped) / `TIM8_CH3` | out | ModBus arm 4 — `SWAP` makes this pin the transmitter |
| 52 | PB13 | `RXD_A4` | `UART5_RX` (swapped) / `TIM8_CH2` | in | |
| 53 | PB14 | — | | | free |
| 54 | PB15 | — | | | free |
| 55 | PD8 | — | | | free |
| 56 | PD9 | `RXD_ECHO` | `USART3_RX` | in | the echo check on the board's own transmission |
| 57 | PD10 | `ALERT_2` | GPIO, EXTI10 | in | arm 2's power board — off its `ISO1642`, high is the alarm |
| 58 | PD11 | `ALERT_3` | GPIO, EXTI11 | in | arm 3's power board |
| 59 | PD12 | `SCL_3` | `I3C1_SCL`, I²C legacy | i/o | arms 3 and 4 — their power boards' `INA238`s |
| 60 | PD13 | `SDA_3` | `I3C1_SDA`, I²C legacy | i/o | |
| 61 | PD14 | `PGOOD` | GPIO | in | the 3,3 V buck's window |
| 62 | PD15 | — | | | free |
| 63 | PC6 | `TXD_A2` | `USART6_TX` / `TIM3_CH1` | out | ModBus arm 2 |
| 64 | PC7 | `RXD_A2` | `USART6_RX` / `TIM3_CH2` | in | |
| 65 | PC8 | `ALERT_P` | GPIO, EXTI8 | in | the up port's power board — **one of six, a pin per socket, no wire-OR**; the others are `ALERT_1`–`ALERT_4` on PC9 · PD10 · PD11 · PC12 and `ALERT_X` on PD2 |
| 66 | PC9 | `ALERT_1` | GPIO, EXTI9 | in | arm 1's power board |
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
| 78 | PC10 | `TXD_A3` | `UART4_TX` | out | ModBus arm 3 |
| 79 | PC11 | `RXD_A3` | `UART4_RX` | in | |
| 80 | PC12 | `ALERT_4` | GPIO, EXTI12 | in | arm 4's power board |
| 81 | PD0 | — | | | free |
| 82 | PD1 | `EN_X` | GPIO | out | the `PWR EXT` body's `ENABLE` — the switched supply's switch; 100 kΩ pull-down, so a reset leaves the load off |
| 83 | PD2 | `ALERT_X` | GPIO, EXTI2 | in | the `PWR EXT` body's board — beside `EN_X` on PD1 |
| 84 | PD3 | — | | | free |
| 85 | PD4 | `LED` | GPIO | out | the status LED, fitted — 1 kΩ from the 3,3 V, one 50 ms blink a minute while healthy, two on a fault (`../core/HARDWARE.md`) |
| 86 | PD5 | `BUTTON` | GPIO, EXTI | in | the commissioning button (`FIRMWARE.md`) |
| 87 | PD6 | `SCL_2` | `I2C3_SCL` | i/o | arms 1 and 2 — their power boards' `INA238`s. |
| 88 | PD7 | `SDA_2` | `I2C3_SDA` | i/o | |
| 89 | PB3 | `RXD` | `TIM2_CH2` capture | in | the same net as PB7 — the timebase captures the start edge of the card's frame |
| 90 | PB4 | — | | | free |
| 91 | PB5 | — | | | free |
| 92 | PB6 | `TXD` | `USART1_TX` / `TIM4_CH1` | out | the NodBus link, to the data body |
| 93 | PB7 | `RXD` | `USART1_RX` / `TIM4_CH2` | in | the pins switch USART → TIM4 for the ranging instant |
| 94 | BOOT0 | 10 kΩ to ground | |  | |
| 95 | PB8 | `SCL` | `I2C1_SCL` | i/o | the up port's and `PWR EXT`'s power bodies — their boards' `INA238`s, 0x40 and 0x41 |
| 96 | PB9 | `SDA` | `I2C1_SDA` | i/o | |
| 97 | PE0 | — | | | free |
| 98 | VCAP | 2× 1 µF 50 V 0805 + 2× 100 nF 50 V 0603 — 2,2 µF, the sheet's 2,2 µF ±20 % | | | |
| 99 | VSS | | | | |
| 100 | VDD | 3,3 V | | | |

**52 GPIO used, 27 free, `PH1` unconnected** (PA1 · PA8–PA12 · PA15 · PB0 · PB2 · PB4 · PB5 · PB10 · PB14 · PB15 · PC1 · PC13–PC15 · PD0 · PD3 · PD8 · PD15 · PE0 · PE3 · PE9 · PE11 · PE15). The `PWR EXT` body takes two pins of its own, `EN_X` and `ID_X`; its `INA238` shares I2C1 with the up port's and its `ALERT` is one of the six.

### The sockets, pin by pin

**Connectors: `NB IN` · `PWR IN` · `MB OUT 1`…`4` · `PWR OUT 1`…`4` · `PWR EXT` · `12V`** (`../galvani/README.md`, *Connector names*).

**The up port** is a unit end: everything on its Galvani boards runs whenever Palatine does, held
by resistors, and no processor pin switches it. In the enclosure it takes the crossed in-box cable
to its card instead of a communication board.

*The up port's data body:*

| pin | signal | on this board |
|---|---|---|
| 1 | `CLK/PPS` | the card's 2²² into PH0, `OSC_IN` in HSE bypass |
| 2 | `GND` | ground |
| 3 | `TXD` | PB6, `USART1_TX` / `TIM4_CH1` |
| 4 | `RXD` | PB7, `USART1_RX` / `TIM4_CH2`, and PB3, `TIM2_CH2` capture, on the same net |
| 5 | `ID` | PC0 `ID_D`, ADC, **10 kΩ 1 % to `VREF+`** |
| 6 | `ID_RET` | **2,49 kΩ to ground** — Palatine, 0,20, landing on the card's `ID` across a crossed cable |
| 7 | `RXD_ECHO` | PD9, `USART3_RX` — the echo check |
| 8 | `DE` | PE2, GPIO |
| 9 | `B_DIR` | **tied to ground** — this end listens on channel B |
| 10 | `LINE_EN` | **10 kΩ to 3,3 V**, no processor pin — the line side runs whenever the unit does |
| 11 | `SD` | PE4, GPIO in, **100 kΩ to ground** — unpopulated on copper, read quiet |
| 12 | `3,3 V` | the `LMR43610`'s 3,3 V — the communication board's whole supply |

*The up port's power body* — the feed board on a fed run, empty in the enclosure:

| pin | signal | on this board |
|---|---|---|
| 1 | `ENABLE` | not connected — the unit power board pulls it to its input |
| 2 | `GND` | ground |
| 3 | `ID` | PB1 `ID_P`, ADC, **10 kΩ 1 % to `VREF+`** |
| 4 | `SDA` | PB9, `I2C1_SDA`, **4,7 kΩ to 3,3 V** |
| 5 | `A_SEL` | **to ground** — 0x40, the first socket on I2C1 |
| 6 | `SCL` | PB8, `I2C1_SCL`, **4,7 kΩ to 3,3 V** |
| 7 | `ALERT` | PC8 `ALERT_P`, EXTI8, **100 kΩ to ground, high = alarm** |
| 8 | `3,3 V` | the `LMR43610`'s 3,3 V — the power board's supply |

**An arm** is a source end: a ModBus communication board on the data body and an arm power board on the power body, both
switched by the arm's one `EN` GPIO, which boots low — the boards pull down, so every arm is dark
until the firmware feeds it. Four identical positions; per arm:

| arm | `TXD` · `RXD` | `DE` | `EN` | data `ID` | power `ID` | I²C | `A_SEL` | `ALERT` |
|---|---|---|---|---|---|---|---|---|
| 1 | PA2 · PA3 (USART2) | PE5 | PE6 | PC2 | PC3 | I2C3, PD6 · PD7 | ground, 0x40 | PC9 `ALERT_1` |
| 2 | PC6 · PC7 (USART6) | PE7 | PE8 | PC4 | PC5 | I2C3 | 3,3 V, 0x41 | PD10 `ALERT_2` |
| 3 | PC10 · PC11 (UART4) | PE10 | PE12 | PA4 | PA5 | I3C1, PD12 · PD13 | ground, 0x40 | PD11 `ALERT_3` |
| 4 | PB12 · PB13 (UART5, `SWAP`) | PE13 | PE14 | PA6 | PA7 | I3C1 | 3,3 V, 0x41 | PC12 `ALERT_4` |

*An arm's data body — the ModBus communication board:*

| pin | signal | on this board |
|---|---|---|
| 1 | `CLK/PPS` | not connected — a ModBus arm has no channel B |
| 2 | `GND` | ground |
| 3 | `TXD` | the arm's USART TX |
| 4 | `RXD` | the arm's USART RX |
| 5 | `ID` | the arm's data `ID` pin, ADC, **10 kΩ 1 % to `VREF+`** |
| 6 | `ID_RET` | not connected — an arm never meets a host |
| 7 | `RXD_ECHO` | not connected — a master never echo-checks |
| 8 | `DE` | the arm's `DE` GPIO |
| 9 | `B_DIR` | not connected — no channel B |
| 10 | `LINE_EN` | the arm's `EN` GPIO, **the same net as the power body's `ENABLE`** |
| 11 | `SD` | not connected — a ModBus communication board is copper and carries no module |
| 12 | `3,3 V` | the `LMR43610`'s 3,3 V — the board's whole supply |

*An arm's power body — the arm power board:*

| pin | signal | on this board |
|---|---|---|
| 1 | `ENABLE` | the arm's `EN` GPIO, the same net as `LINE_EN` |
| 2 | `GND` | ground |
| 3 | `ID` | the arm's power `ID` pin, ADC, **10 kΩ 1 % to `VREF+`** |
| 4 | `SDA` | the arm's controller `SDA`, **4,7 kΩ to 3,3 V** — one pair per controller |
| 5 | `A_SEL` | strapped per the table above |
| 6 | `SCL` | the arm's controller `SCL`, **4,7 kΩ to 3,3 V** |
| 7 | `ALERT` | the arm's `ALERT` pin, **100 kΩ to ground, high = alarm** |
| 8 | `3,3 V` | the `LMR43610`'s 3,3 V — the power board's supply |

**`PWR EXT` is a power body alone**, a source end: no data body.

| pin | signal | on this board |
|---|---|---|
| 1 | `ENABLE` | PD1 `EN_X`, GPIO, boots low, 100 kΩ to ground |
| 2 | `GND` | ground |
| 3 | `ID` | PA0 `ID_X`, ADC, **10 kΩ 1 % to `VREF+`** |
| 4 | `SDA` | PB9, `I2C1_SDA` — the up port's pull-ups |
| 5 | `A_SEL` | **to 3,3 V** — 0x41, the second socket on I2C1 |
| 6 | `SCL` | PB8, `I2C1_SCL` |
| 7 | `ALERT` | PD2 `ALERT_X`, EXTI2, **100 kΩ to ground, high = alarm** |
| 8 | `3,3 V` | the `LMR43610`'s 3,3 V |

### Timers

| timer | bits | kernel | pins | job |
|---|---|---|---|---|
| **TIM4** | 16 | 2²⁷ | CH1 → PB6, CH2 ← PB7 | the ranging turnaround: one-pulse, triggered by the capture on CH2, the return raised on CH1 after `CCR` ticks; PWM-input mode on CH2 measures the incoming pulse's width |
| **TIM2** | 32 | 2²⁷ | CH2 capture ← PB3 | the free-running timebase; the capture of the card's frame start places the round on the grid |
| TIM1 · TIM3 · TIM5 · TIM8 · TIM12 · TIM15 · LPTIM1 · LPTIM2 | | | | free — the arms' USARTs share pins with them and use none |
| **TIM6** | 16 | 2²⁷ ÷ prescaler | none | the RTU frame gaps and the poll rounds on the four arms |
| TIM7 | 16 | | | free |

### Clock tree

| | |
|---|---|
| `OSC_IN` | 2²² = 4,194304 MHz, the data body's `CLK` straight onto the pin — a NOD carries no crystal |
| HSE | bypass |
| PLL1 | M 1 · N 64 · P 2 → VCO 2²⁸ = 268,435456 MHz, `P` 2²⁷ |
| SYSCLK, AHB, APB1/2, timer kernel | **2²⁷ = 134,217728 MHz**, no further division |
| the clock gone | HSI fallback and the degraded flag (`../core/blocks/clocks.md`); the off sequence drops the clock the same way and the returning feed is the wake (`../core/PROTOCOL.md` §7) |

Every timer counts 2²⁷, the grid a card measures on, so a tick here and a tick on the card are the
same tick and a conversion to the 2²³ timebase is a shift of four bits.

### Analogue and reference

**Every filter inductor on this board is a π filter: 10 µF 1206 + 100 nF 0603 before it on the rail and
the same pair at the pins behind it** — the house rule (`../core/POWER.md`, *The filter parts*).

`VDDA` and `VREF+` are one node off the 3,3 V through the shielded 2,2 µH (`SWPA252012S2R2MT`) into
10 µF + 100 nF at the pins, the house filter for an ADC reference on a buck's output
(`../core/POWER.md`, *The filter parts*); `VREF−` and `VSSA` to the ground plane at one point. The eleven `ID` inputs are read single-ended against
`VREF+`, and the 10 kΩ 1 % pull-up of each `ID` resistor hangs on the same `VREF+` node, so the reading is a
ratio and the rail's tolerance drops out. The arms' 12 V and their currents are the four arm power boards' `INA238`s, over I2C3 and I3C1; the
up port's and `PWR EXT`'s are on I2C1 — no divider on this board.

## Rain and snow — two blocks from two instruments

| phase | instrument | in the payload |
|---|---|---|
| rain, hail | **Pluvius**, the weighing gauge — a ModBus module on an arm (`../pluvius/`) | its block: the rain since the last boundary, the last hour and the status word |
| snow | an **80 GHz radar level sensor**, bought to `SENSORS.md` | its block: the range and its echo quality; the depth is the mast height less the range, derived where the archive is read |

Rain is weighed and snow is a height. The water equivalent of snow is the archive's estimate
from a density, never a measurement here; a site without a radar has no snow block.

## RS-485 Sensor Bus

**Four RS-485 Modbus RTU arms, one USART each; the STM32H523 is the master on all four.** No
line side on this board: an arm leaves through a ModBus communication board — always, because the far end of an
arm is a bought sensor or a MOD and never a host, so no crossed cable lands on an arm — and the sensors on it take their 12 V from an arm power board at the
station or the unit board's 12 V terminal on a fed run (`../galvani/README.md`). **The arm connectors sit on this board: four Galvani port positions, one per arm, and a fifth power body on its own, `PWR EXT`** — an arm's position is the same two bodies a card's port carries
(`../galvani/README.md`, *The connectors*): a 12-pin data cable to the communication board, an 8-pin power cable to the power board. **The 12 V is not on the board**: each arm power board taps the 12 V wire on its own terminals, from the tap on this board's terminals and then board to board. The I²C on it (`SDA` ·
`SCL` · `ALERT`) reaches all **six** power sockets — the four arms, `PWR EXT`, and the up port's own,
where the feed board sits on a fed run — but **not on one bus: two sockets to a controller**, the
up port and `PWR EXT` on I2C1, arms 1 and 2 on I2C3, arms 3 and 4 on I3C1. `ENABLE` is one GPIO per
arm, so an arm's 12 V switches on its own.

**A jammed board takes its own pair down and nothing else, and `ALERT` is one pin per socket** —
`ALERT_P` PC8 · `ALERT_1`–`ALERT_4` PC9 · PD10 · PD11 · PC12 · `ALERT_X` PD2, **EXTI 8 · 9 · 10 ·
11 · 12 · 2, six different numbers**, because EXTI is shared by pin number across ports. The
alarm crosses each board's barrier on its `ISO1642`'s channel B, **high on alarm** (`APOL` = 1 at
the `INA238`), so a dark socket reads quiet; hanging six of them on one pin would blind the five
that are working. Six pins buy the attribution. What the alert means does not change: it says
somebody has a threshold crossed, and which one and why comes from the read that follows, on the
ordinary poll round. A socket whose `ALERT` sits high is flagged and polled like the rest.

**`PWR EXT` is a power body and nothing else — the switched supply for Pluvius's pump.**
A source power board plugs into it and is switched by its `ENABLE`, `EN_X` on this board, and
its `INA238` is the pump's ammeter, and the rule that switches it is in the firmware (`FIRMWARE.md` §6, *The `PWR EXT` body*). The board taps the 12 V wire on
its own terminals like every source power board, so its watts never cross this board; on a
remote Palatine they count toward the far end's load, and the feed is picked from the
load table like any run's (`../galvani/README.md`, *Reach*). Everything that leaves the box
crosses a Galvani barrier, and the body already carries the switch, the ammeter and the `ID`, so the port costs two pins.

### Termination and protection

This is the **slow ModBus arm**, not the NodBus. **An arm at 19 200 is not terminated at any
length up to the 50 m cap — the cap is the arm's 12 V, not the data** (`../core/blocks/modbus.md`, *Two rates, one protocol*),
and the bought sensors on it carry no terminator either; an arm run faster than that is
terminated on the Galvani board it leaves through, never here. The clamp likewise sits on the
Galvani board, not on this one.

### The sensors

The universal station and the farmers' build, with a recommended type for each, are `SENSORS.md`.

**What feeds the sensors.** A bought sensor, and a house MOD alike, is powered from **the isolated 12 V an arm power board makes at
the station, or the unit board's 12 V terminal on a fed run** — nominally 12 V, at the station it follows the
pack through the isolator (`../galvani/README.md`). **No sensor rail on this board.**
A sensor that cannot run on a 12 V that wanders with the pack is not the sensor; the wide-input
ModBus class runs to 24–30 V and covers it. **Gating is per sensor, and it follows the part**:
a hungry sensor is switched at its arm power board's `ENABLE` between reads where the part tolerates
it, and some do not — a radar can lose its settling and calibration when cut, some reduce
power instead, some pulse on their own — so the read interval and the gating are
commissioning settings, never a number this document fixes (`../core/blocks/modbus.md`).

### Four arms, and what decides the number

**A ModBus arm carries about sixteen units, and that is why this board has four ports.** The cap
is electrical and it is three things at once: **reach**, because an unterminated RTU run stops
being reliable long before the address space runs out; **the bus's own complexity**, every added
stub loading the pair and lengthening the turnaround; and **the rate**, because a poll round is
serial and sixteen units at a working rate is already a round measured in the hundreds of
milliseconds. Four arms is what a full weather station needs once bought sensors, our own MODs
and a Babel's positions are counted, and a plot wider than four arms carry is a second Palatine
on a fed run, not a fifth arm.
**The fifth body on the board is not an arm**: `PWR EXT` is a power body alone, the switched supply
for Pluvius's pump (*RS-485 Sensor Bus*, above).

**The arms are an electrical division and nothing else.** A **rate is a property of the arm**, a table entry
from commissioning that the handler sets per arm, so a slow sensor needs no tier of its own — it
sits on whichever arm suits its neighbours, and splitting by rate stays what it is, the fallback
for sensors that cannot be moved off their baud (`../core/blocks/modbus.md`). And an arm is
**not an address space**: a NUMBER is unique across the station, so the same type on two arms is
two NUMBERs and the arm never has to disambiguate anything (`../core/PROTOCOL.md` §2).

### The Modbus address map

**Nothing on this bus keeps its factory address.** Ten sensors all answering to 1 cannot be talked
to at all, so every address here is one we put there and the node polls nothing else. The two
halves differ only in how the number gets into the device: ours are **given** it by Palatine's
sweep — a fresh module answers on its type's default NUMBER, 3, that is the byte `TYPE«2 | 3`,
and takes the lowest free number (`../core/PROTOCOL.md` §2) — and bought ones have it
**written into them**. Ours are packed
`TYPE«2 | NUMBER` with TYPE from 4, so they always land at 16 or above and the bench list below
never reaches them.

**Bought — write these in, one sensor at a time:**

| Sensor | Address |
|---|---|
| Air T/RH, 2 m (or the T/RH/P unit) | 0x01 |
| Ground temperature, 5 cm | 0x02 |
| Barometer (free with a T/RH/P unit) | 0x03 |
| Wind speed (or combined unit) | 0x04 |
| Wind direction (free if combined) | 0x05 |
| Pyranometer | 0x06 |
| UV | 0x07 |
| a site's own sensor | 0x08 on |

(**Wind reserves two addresses, 0x04 + 0x05:** a **combined** speed+direction unit uses **0x04** and
leaves **0x05** free; **two separate** units take both. The list stops well short of 0x0F, which is
the whole bought range.)

**Ours — computed, nothing to assign:**

**`TYPE«2 | NUMBER`, TYPE 4..61, NUMBER 0..3** (`../core/PROTOCOL.md` §2):

| Module | type | Address, unit 0 |
|---|---|---|
| Babel, bare — a board with no position fitted (`../babel/MODBUS.md`) | 4 | 0x10 |
| Pluvius (weighing rain gauge) | 5 | 0x14 |
| Ceres (soil moisture + soil temperature) | 6 | 0x18 |
| Sakura (leaf wetness) | 7 | 0x1C |
| a quantity at a position that arrives on a Babel position | **8..61, one per quantity at a position** | claimed when its profile is written |

**Nobody reads these in hex and nobody has to.** The address is computed from the identity, so
what a build keeps is a list of sensor numbers — the wiring is *through this Bifrost, to this
unit*, and no table translates anything.

**One probe at one depth is one Ceres**, so a patrona's depths are separate units with their own
NUMBERs and depth never needs a channel — and each reports both of its depth's quantities, the
moisture on `0x0000` and the soil temperature on `0x0001` (`../ceres/README.md`).

**Babel has no type and no address of its own.** Each of its fitted positions answers as its own
slave on **the type of the quantity it measures**, so a board carrying a thermometer and a
hygrometer spends one address in each of those two types — and `0xFF01 IDENT` is what tells the
master the two are one board (`../babel/MODBUS.md`).

**Only the bought half is ever written.** A fresh bought sensor answers at its factory address;
the commissioning pass finds it on the scan and writes it to its row above, one sensor at a
time. **Ours are never written and never scanned for** — the address falls out of the identity,
so a replaced box comes up on the same one.

---

## The rails, and what the node draws

**One buck and nothing else.** The 12 V comes in on the board's two terminals, an input and a tap — off the
battery wire through its own fuse in the enclosure's fuse field, sized by the construction, off a unit power board's terminals on a fed run —
behind one **`5.0SMDJ14A`** across the input (`../galvani/README.md`, *The rails*). In the box every arm power board
takes its own fuse in the field; on a fed run, where the unit power board limits its own current, the 12 V leaves on the tap
for them, ~2,2 A on the wire with four, none of it across this board. An **`LMR43610` at 3,3 V** feeds everything
on the board: the MCU, the NodBus communication board and the four ModBus communication boards on the arms. This node
carries no analogue front, so there is nothing to split onto an LDO.

| on the 3,3 V | |
|---|---|
| `STM32H523`, LQFP100 | ~100 mA |
| the NodBus communication board — Palatine is its far end | ~18 mA on copper, ~55 mA on glass, 125 mA at most |
| 4× ModBus communication board, one per arm | ~9 mA each listening, ~60 mA asking |
| **total** | **~465 mA of the part's 1 A** at the worst corner — glass at its ceiling and four arms asking at once |

**A ModBus communication board draws far less than its port implies.** An arm at 19 200 is not
terminated (*Termination and protection*); the board takes **~9 mA on the host's 3,3 V listening
and ~60 mA asking**, ~200 mW at the most, as `../galvani/HARDWARE.md` counts it, and only for the
milliseconds of a request.

**Switching an arm off costs this board nothing.** One GPIO per arm reaches the arm's power board
`ENABLE` on the power body and its communication board's `LINE_EN` on the data body, joined on this board:
the `SN6507` takes away the sensors' isolated 12 V and the `SN6505B` the line side in the same
stroke. A switched-off arm draws nothing, so no second buck and no load switch buys anything
here.

### The budget — what the iron allows, and the trip point that is measured

**Palatine's trip point is measured at commissioning, like every run's**: the start-up peak and the
running load, with margin, written into the source board's `INA238` through the card's per-port
register, and rewritten when a sensor is added (`../galvani/README.md`, *The two choices, and they are independent*). There
is no class to declare; the ceiling is the iron:

| | |
|---|---|
| an arm power board, one arm | 0,4 A at ~12 V, **~5 W**; the `SN6507`'s `R_LIM` sits at ~0,7 A, the 0,4 A ceiling is the `INA238`'s |
| four arms at 0,4 A | **~19 W** |
| the node itself, through its buck | ~1,5 W |
| **four arms on their limit and the node** | **~21 W**, inside the 48 V unit power board's ~25 W on a fed run |

An arm asked for more than its cell gives **droops**: the `SN6507` shortens its pulses, the arm's
12 V sags with the current on the limit, and the arm's own `INA238` sees the sag
(`../galvani/README.md`). Nothing on this board is in that path.

**Today's set** uses a third of that:

| | at 12 V |
|---|---|
| 2× air T/RH, and a site's own thermometers where it adds them | 80 mA |
| anemometer | 50 mA |
| pyranometer | 30 mA |
| UV | 20 mA |
| barometer | 20 mA |
| 2× Ceres | 40 mA |
| Pluvius | ~50 mA |
| snow radar, 1,5 W | 125 mA |
| **the sensors** | **~400 mA, 5 W** |
| the node itself, through its buck | ~125 mA, **1,5 W** |
| the sensors through four arm power boards at ~85 % | **+0,9 W** |
| **the node and its four arms** | **~7,4 W** |

**What hangs on `PWR EXT` is not in this table.** Its board taps the 12 V wire and its watts are the
load's own — Pluvius's head is ~12 W for minutes a year, switched on by `EN_X` and off again — and
at a station they never cross this board. On a remote Palatine they are the run's, and the feed
is sized from the load table with them in (`../galvani/README.md`, *Reach*).

## Commissioning the addresses — through the arm, never with an adapter

**Nothing on an arm keeps its factory address**, and the bought sensors all ship at the same one.
They are written **in the learning session**, on the arm they will live on: the head opens it
(`SET PROV arm · rate`), the arm becomes a byte pipe at the sensor's factory rate, the head tunnels
the FC06 writes of address and baud and reads them back at the new settings, and closes the
session (`FIRMWARE.md` §6). One unwritten sensor on the arm at a time. The house MODs are given
their NUMBER by the sweep and are never written by hand. The arm's settings are 19 200 8N1, or the
arm's own rate for a baud-locked sensor; what a register write can and cannot reach is the bus's
(`../core/blocks/modbus.md`, *The register model*). A sensor that goes where it cannot be reached
again — a Ceres in the patrona — is numbered before it goes in (`CONSTRUCTION.md`).

## The payload — blocks

32 B of self-delimiting blocks, `[address][length][data]`, one per ModBus transaction, whole blocks
only, a leading zero address closing a payload the blocks do not fill; decoded by the
module's own address and the profile behind it, where the archive is read (`../core/PROTOCOL.md` §5, `FIRMWARE.md` §7).
Radiation does not pass through here — Quark-Tubes is a mini-NOD on Argus. The arrived feed and the six
sockets' readings ride the `PORTS` frame once a minute and never the payload.

## Temperature

The board's parts are rated to −40 °C and every bought probe is bought to −40 °C. There is no
temperature threshold in the firmware: an arm whose CRC-miss rate passes the `QC` threshold is
switched off and re-fed after a cool-off (`FIRMWARE.md` §9). Burying a remote enclosure for its
thermal mass is the builder's siting.

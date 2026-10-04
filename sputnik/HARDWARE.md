★ N.I.C. ★

# Sputnik — the board

> **Design-stage concept.** The figures are verified against the parts' sheets and the parts
> before a board is made.

**The house NodBus unit on the STM32H523, with a UM980 GNSS receiver on the same board.** The link
leaves as plain logic on the Galvani data body; **there is no 485 part on this board** — the
transceivers, the barrier, the ladder and the termination are the plugged Galvani board's, or the
crossed in-box cable's. The antenna is a bought part named by class, and the tier and wire format
the receiver feeds is `BUS.md`.

---

## Node MCU — STM32H523

Same MCU as every NOD — one toolchain, one firmware ecosystem.

- **Six UARTs** — the NodBus link, its echo receiver `RXD_ECHO`, the clock run to Kronos,
  and the UM980's three COMs (*The processor*, below). No Modbus on this node.
- `CLK` lands on `OSC_IN`, no pin remap (`../core/blocks/nodbus.md`).
- Serves several consecutive node addresses / slots (five, one pool — `BUS.md`); the
  pooled slots and the bandwidth are in `BUS.md`.

## The processor — pins, timers, clock tree

*The record to draw the H523 from and to set CubeMX by. Package LQFP100, `STM32H523VE`; pin
numbers and alternate functions from DS14540 Rev 3, Table 13. Every pin below is either
assigned or listed free — nothing is left to guess.*

### Peripherals

| peripheral | instance | pins | job | mode |
|---|---|---|---|---|
| the NodBus link | **USART1** + **TIM4** | PB6 `TXD` · PB7 `RXD` | the spur to its card, 8N1 at the rung `BUSCFG` hands down — 2²⁰ alone, 2²¹ chained | the pins switch USART → TIM4 for the ranging instant |
| the frame anchor | **TIM2** (32-bit) | PB3 ← `RXD` on CH2 · PA15 ← `PPS` on CH1 | the card's frame start on the grid, and the receiver's PPS against the same counter | never reset, read as differences |
| echo receiver | **USART3**, RX only | PD9 `RXD_ECHO` | hears the board's own frames | |
| the clock run | **USART6** + **TIM3** | PC6 `TXD_T` · PC7 `RXD_T` | the RX/TX + PPS socket to Kronos: NMEA out at 115 200, the PPS on channel B outward | Kronos ranges it; TIM3 in one-pulse mode is the turnaround |
| UM980 COM1 | **USART2** | PA2 · PA3 | the receiver's binary logs — `OBSVMCMP`, `SATSINFO`, the ionosphere models, the HAS and B2b pages | 921 600 8N1 |
| UM980 COM2 | **UART4** | PA0 · PA1 | the lean NMEA the H523 relays onto the clock run | 115 200 |
| UM980 COM3 | **UART5**, `SWAP` set | PB12 · PB13 | the configuration at boot and its answers | 115 200 |
| power body | **I2C1** | PB8 `SCL` · PB9 `SDA`, 4,7 kΩ to 3,3 V | the power board's `INA238` — the arrived voltage; **`A_SEL` strapped to ground on this board**, so it is 0x40, one socket and no processor pin on the strap (`../galvani/README.md`) | 100 kHz; `ALERT` on PC8 |
| ID reads | **ADC1** | PC0 · PC1 · PC2 | one resistor per body, against 10 kΩ | read once at bring-up |
| clock in | **HSE bypass** | PH0 | the data body's `CLK`, 2²² | PH1 unconnected |
| debug | **SWD** | PA13 · PA14 | | no JTAG, no trace |

Six of the seven UARTs, none of them the LPUART. **Two sockets, both the data body**: on the
NodBus socket this board listens on channel B — the card's clock comes in — and `B_DIR` is tied to
ground; on the RX/TX + PPS socket this board drives channel
B — the PPS goes out to Kronos — and `B_DIR` is tied to 3,3 V. No processor pin reads the strap — the socket is the
configuration. The NodBus socket has a power body beside it; the second socket has none, because
the feed travels with the NodBus run.

### The sockets, pin by pin

**Connectors: `NB IN` · `PWR IN` · `TIME OUT` · `12V` · `ANT`** (`../galvani/README.md`, *Connector names*).

**Both sockets are a unit end**: everything on the plugged Galvani board runs whenever Sputnik
does, held by resistors, and no processor pin switches it. In the enclosure each takes a crossed
in-box cable instead — the NodBus socket to a card's port, the time port to Kronos's socket.

*The NodBus socket, data body:*

| pin | signal | on this board |
|---|---|---|
| 1 | `CLK/PPS` | the card's 2²² into PH0, `OSC_IN` in HSE bypass |
| 2 | `GND` | ground |
| 3 | `TXD` | PB6, `USART1_TX` / `TIM4_CH1` |
| 4 | `RXD` | PB7, `USART1_RX` / `TIM4_CH2`, and PB3, `TIM2_CH2` capture, on the same net |
| 5 | `ID` | PC0 `ID_D`, ADC, **10 kΩ 1 % to `VREF+`** |
| 6 | `ID_RET` | **3,32 kΩ to ground** — Sputnik, 0,25, landing on the card's `ID` across a crossed cable |
| 7 | `RXD_ECHO` | PD9, `USART3_RX` — the echo check |
| 8 | `DE` | PE2, GPIO |
| 9 | `B_DIR` | **tied to ground** — this end listens on channel B |
| 10 | `LINE_EN` | **10 kΩ to 3,3 V**, no processor pin |
| 11 | `SD` | PE4, GPIO in, **100 kΩ to ground** |
| 12 | `3,3 V` | the `LMR43610`'s 3,3 V — the communication board's whole supply |

*The NodBus socket, power body* — the unit power board on a fed run, empty in the enclosure:

| pin | signal | on this board |
|---|---|---|
| 1 | `ENABLE` | not connected — the unit power board pulls it to its input |
| 2 | `GND` | ground |
| 3 | `ID` | PC1 `ID_P`, ADC, **10 kΩ 1 % to `VREF+`** |
| 4 | `SDA` | PB9, `I2C1_SDA`, **4,7 kΩ to 3,3 V** |
| 5 | `A_SEL` | **to ground** — 0x40 |
| 6 | `SCL` | PB8, `I2C1_SCL`, **4,7 kΩ to 3,3 V** |
| 7 | `ALERT` | PC8, EXTI, **100 kΩ to ground, high = alarm** |
| 8 | `3,3 V` | the `LMR43610`'s 3,3 V — the power board's supply |

*The time port toward Kronos, data body* — the PPS sender; Sputnik is the unit end of this link
and Kronos the source end:

| pin | signal | on this board |
|---|---|---|
| 1 | `CLK/PPS` | **the UM980's PPS net — the one PA15 captures — drives the pin through 33 Ω**, never through the processor |
| 2 | `GND` | ground |
| 3 | `TXD` | PC6 `TXD_T`, `USART6_TX` / `TIM3_CH1` — the lean `RMC`/`GGA` stream and the ranging return |
| 4 | `RXD` | PC7 `RXD_T`, `USART6_RX` / `TIM3_CH2` — Kronos's heartbeat and its ranging edge |
| 5 | `ID` | PC2 `ID_T`, ADC, **10 kΩ 1 % to `VREF+`**; reads Kronos's `GNSS`, 0,35, across a crossed cable |
| 6 | `ID_RET` | **5,36 kΩ to ground** — `GNSS`, 0,35, the interface this port presents |
| 7 | `RXD_ECHO` | not connected — the time stream is point-to-point and not echo-checked |
| 8 | `DE` | PE5 `DE_T`, GPIO — held low until Kronos's heartbeat arrives |
| 9 | `B_DIR` | **tied to 3,3 V** — this end drives channel B |
| 10 | `LINE_EN` | **10 kΩ to 3,3 V**, no processor pin — the gate on the stream is `DE_T` alone |
| 11 | `SD` | PE7 `SD_T`, GPIO in, **100 kΩ to ground** |
| 12 | `3,3 V` | the `LMR43610`'s 3,3 V |

### Pins

| pin | port | signal on the board | AF / function | dir | note |
|---|---|---|---|---|---|
| 1 | PE2 | `DE` | GPIO | out | the data body's driver enable |
| 2 | PE3 | — | | | free |
| 3 | PE4 | `SD` | GPIO | in | the data body's optical no-light |
| 4 | PE5 | `DE_T` | GPIO | out | the second socket's data body |
| 5 | PE6 | — | | | free |
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
| 16 | PC1 | `ID_P` | `ADC12_INP11` | in | the power body's `ID` |
| 17 | PC2 | `ID_T` | `ADC12_INP12` | in | the second socket, data body |
| 18 | PC3 | — | | | free (ADC) |
| 19 | VSSA | | | | |
| 20 | VREF− | | | | |
| 21 | VREF+ | 3,3 V through the shielded 2,2 µH, 10 µF + 100 nF at the pins — the ADC reference | | | |
| 22 | VDDA | 3,3 V through the shielded 2,2 µH, 10 µF + 100 nF at the pins | | | |
| 23 | PA0 | `TX_COM2` | `UART4_TX` | out | UM980 COM2 — the NMEA the H523 relays to Kronos |
| 24 | PA1 | `RX_COM2` | `UART4_RX` | in | |
| 25 | PA2 | `TX_COM1` | `USART2_TX` | out | UM980 COM1 — the raw observables, the archived stream |
| 26 | PA3 | `RX_COM1` | `USART2_RX` | in | |
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
| 38 | PE7 | `SD_T` | GPIO | in | |
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
| 51 | PB12 | `TX_COM3` | `UART5_TX` (swapped) | out | UM980 COM3 — the configuration |
| 52 | PB13 | `RX_COM3` | `UART5_RX` (swapped) | in | |
| 53 | PB14 | — | | | free |
| 54 | PB15 | — | | | free |
| 55 | PD8 | — | | | free |
| 56 | PD9 | `RXD_ECHO` | `USART3_RX` | in | the echo check on the board's own transmission |
| 57 | PD10 | `GNSS_RST` | GPIO, open-drain | out | the UM980's `RESET_N` |
| 58 | PD11 | — | | | free |
| 59 | PD12 | — | | | free |
| 60 | PD13 | — | | | free |
| 61 | PD14 | `PGOOD` | GPIO | in | the 3,3 V buck's window |
| 62 | PD15 | — | | | free |
| 63 | PC6 | `TXD_T` | `USART6_TX` / `TIM3_CH1` | out | the RX/TX + PPS socket to Kronos — the lean `RMC`/`GGA` stream |
| 64 | PC7 | `RXD_T` | `USART6_RX` / `TIM3_CH2` | in | Kronos ranges this run; TIM3 turns the edge round |
| 65 | PC8 | `ALERT` | GPIO, EXTI | in | the power body's `INA238` — 100 kΩ to ground, high = alarm |
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
| 77 | PA15 | `PPS` | `TIM2_CH1` capture | in | the UM980's PPS — the same net drives the time port's `CLK/PPS` through 33 Ω |
| 78 | PC10 | — | | | free |
| 79 | PC11 | — | | | free |
| 80 | PC12 | — | | | free |
| 81 | PD0 | — | | | free |
| 82 | PD1 | — | | | free |
| 83 | PD2 | — | | | free |
| 84 | PD3 | — | | | free |
| 85 | PD4 | `LED` | GPIO | out | the status LED, fitted — 1 kΩ from the 3,3 V, one 50 ms blink a minute while healthy, two on a fault (`../core/HARDWARE.md`) |
| 86 | PD5 | — | | | free |
| 87 | PD6 | — | | | free |
| 88 | PD7 | — | | | free |
| 89 | PB3 | `RXD` | `TIM2_CH2` capture | in | the same net as PB7 — the timebase captures the start edge of the card's frame |
| 90 | PB4 | — | | | free |
| 91 | PB5 | — | | | free |
| 92 | PB6 | `TXD` | `USART1_TX` / `TIM4_CH1` | out | the NodBus link, to the data body |
| 93 | PB7 | `RXD` | `USART1_RX` / `TIM4_CH2` | in | the pins switch USART → TIM4 for the ranging instant |
| 94 | BOOT0 | 10 kΩ to ground | |  | |
| 95 | PB8 | `SCL` | `I2C1_SCL` | i/o | the power body's `INA238`; the board's I²C parts |
| 96 | PB9 | `SDA` | `I2C1_SDA` | i/o | |
| 97 | PE0 | — | | | free |
| 98 | VCAP | 2× 1 µF 50 V 0805 + 2× 100 nF 50 V 0603 — 2,2 µF, the sheet's 2,2 µF ±20 % | | | |
| 99 | VSS | | | | |
| 100 | VDD | 3,3 V | | | |

**29 GPIO used, 50 free, `PH1` unconnected** (PA4–PA12 · PB0–PB2 · PB4 · PB5 · PB10 · PB14 · PB15 · PC3–PC5 · PC9–PC15 · PD0–PD3 · PD5–PD8 · PD11–PD13 · PD15 · PE0 · PE3 · PE6 · PE8–PE15).

### Timers

| timer | bits | kernel | pins | job |
|---|---|---|---|---|
| **TIM4** | 16 | 2²⁷ | CH1 → PB6, CH2 ← PB7 | the ranging turnaround: one-pulse, triggered by the capture on CH2, the return raised on CH1 after `CCR` ticks; PWM-input mode on CH2 measures the incoming pulse's width |
| **TIM2** | 32 | 2²⁷ | CH2 capture ← PB3, CH1 capture ← PA15 | the free-running timebase; the card's frame start places the round on the grid, and the UM980's PPS captured on the same counter is the receiver's epoch on that grid — what `BUS.md`'s epoch is stamped against |
| **TIM3** | 16 | 2²⁷ | CH1 → PC6, CH2 ← PC7 | the clock run's turnaround: Kronos fires, this board returns the edge after `CCR` ticks |
| TIM1 · TIM5 · TIM8 · TIM12 · TIM15 · LPTIM1 · LPTIM2 | | | | free |
| TIM6 · TIM7 | 16 | | | free — the basic timers |

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

`VDDA` and `VREF+` off the 3,3 V through the shielded 2,2 µH `SWPA252012S2R2MT` each, 10 µF + 100 nF at the pins; `VREF−`
and `VSSA` to the ground plane at one point. The three `ID` inputs are read single-ended against
`VREF+`, and the 10 kΩ 1 % pull-up of each `ID` resistor hangs on `VREF+`, so the reading is a
ratio and the rail's tolerance drops out. The arrived 12 V and the input current are the power
body's `INA238`, over I2C1 — no divider on this board.

---

## GNSS Receiver — Unicore UM980 (the plain UM980 — the "C" is NOT needed)

**The node's primary sensor — one UM980 feeds all of the node's GNSS addresses.**

> **The plain UM980 is sufficient; do NOT pay for the UM980C.** Both the **UM980** and the **UM980C**
> carry the full multi-band signal set **and** the two augmentation services this project logs —
> **Galileo E6-HAS · BeiDou B2b-PPP**. The UM980C's *only* extra is **QZSS L6D CLAS** (a cm-positioning
> service, **Japan only**) and **L-band PPP-AR** (a **paid subscription** positioning service). **NIC
> uses neither** — it does atmospheric sensing (raw observables plus the decoded augmentation pages),
> not high-precision positioning (the station position is surveyed once, static). So the **plain UM980
> does 100 % of what this node needs, cheaper**; the C buys only unused, regional/paid positioning
> features.

> **Required part — not a free-swap (but any UM980-class SKU qualifies).** Unlike the passive / COTS
> parts elsewhere, the receiver is a **hard dependency**: the firmware glue parses its **specific
> message set** (`OBSVMCMP` observations, `SATSINFO`, the ionosphere logs, the HAS and B2b pages — `FIRMWARE.md` §6). A *different vendor's* receiver means rewriting
> the parser (the format is the contract, the chip is swappable with new glue). Within Unicore,
> **UM980 or UM980C both work**; draw the board around the **UM980**.

**What it tracks, from the sheet (Unicore's user manual, R1.9):** GPS L1 C/A · L1C · L2P(Y) · L2C · L5;
BDS B1I · B2I · B3I · B1C · B2a · B2b; GLONASS G1 · G2 · G3; Galileo E1 · E5a · E5b · E6; QZSS L1C/A ·
L1C · L2C · L5 · L6; NavIC L5 — 1408 channels, and three augmentation services named on its front
page, **B2b-PPP · E6-HAS · QZSS L6E MADOCA**. **The three-frequency backbone per constellation is
`BUS.md`'s Tier B**; the raw observables — pseudorange, carrier phase, SNR — are the irreplaceable TEC
measurement, archived; the chip's own corrections are live-only. **What the receiver tracks is a
signal group set by command, and the groups are fixed menus**: group 2 is the one that carries the
whole backbone — and E6 and B2b with it — while the groups that decode **MADOCA** drop GPS L1C,
GLONASS G3, BeiDou B1C and NavIC; so the node runs group 2 and logs HAS and B2b, and MADOCA is not
logged (`FIRMWARE.md` §6, `WHY.md`). **NavIC is tracked on L5 alone**, so its S-band slot in `BUS.md`
stays empty. **SBAS L1C/A is in the group and serves the receiver's own fix; no log carries the SBAS
ionospheric grid**, so it is not a Tier C source.

**The module.** A **54-pin LGA, 22 × 17 × 2,6 mm, soldered to the board** — no carrier, no socket — on
the manual's footprint, its 48 inner pads on a large ground for the heat. **`VCC` 3,0–3,6 V with the
ripple inside that window and ≤ 50 mV of it; 145 mA typical, 180 mA at most, 480 mW**, with an inrush
into its own capacitors at power-on. LVTTL at `VCC` — a high input ≥ 0,7 `VCC`, outputs within 0,45 V
of the rails at 2 mA — so the H523's 3,3 V logic meets it with no translator. Power-on by the sheet:
`VCC` from under 0,4 V, monotonic, no plateau, undershoot under 5 %, and **at least 500 ms below
0,4 V between a power-off and the next power-on** — the buck's soft start is monotonic, and the one
thing that power-cycles this board is the fuse field, never the firmware. `RESET_N` is active low for
5 ms at least. Time pulse accuracy **20 ns RMS**; cold start under 12 s, and no hot start, because the
backup pin is tied to the rail. I²C, SPI and CAN are reserved and not supported by the module's firmware.

| pin | | on this board |
|---|---|---|
| 42 `TXD1` · 43 `RXD1` | COM1, LVTTL | the raw observables — into PA3 `USART2_RX`, from PA2, 921 600 |
| 27 `TXD2` · 26 `RXD2` | COM2 | the lean NMEA — into PA1 `UART4_RX`, from PA0, 115 200 |
| 30 `TXD3` · 31 `RXD3` | COM3 | the configuration at boot and its answers — into PB13 `UART5_RX`, from PB12, 115 200 |
| 53 `PPS` | the time pulse — rising edge on the GPS second, 10 ms wide, by `CONFIG PPS` | **the one net PA15 captures and the time port's `CLK/PPS` takes through 33 Ω** |
| 49 `RESET_N` | active low, ≥ 5 ms | `GNSS_RST` PD10, open-drain, **10 kΩ to the 3,3 V** |
| 36 `V_BCKP` | 2,0–3,6 V; feeds the RTC when `VCC` is gone | **tied to `VCC`** — a fixed station wants no hot start; the sheet forbids ground and open |
| 28 · 29 `BIF` | built-in function | **10 kΩ to the 3,3 V and a test point on each**, nothing else — never ground, supply or data |
| 33 · 34 `VCC` | the supply | the 3,3 V, **3× 10 µF 50 V 1206 + 100 nF at the pins** — the sheet asks ≥ 30 µF |
| 2 `ANT_IN` | 50 Ω; −0,3…6 V, +10 dBm absolute | the feed network below, through its 100 pF |
| 7 `VCC_RF` · 5 `ANT_OFF` · 4 `ANT_DETECT` · 6 `ANT_SHORT_N` | the module's own antenna supply and its detection | **not connected** — the sheet says not to feed the antenna from `VCC_RF`, which has no surge path; the bias is the board's, below |
| 51 `EVENT` | event mark input | not connected |
| 8–11 SPI · 44 · 45 I²C | reserved | not connected |
| 13 · 22 · 23 `RSV` and every `NC` | | open, as the sheet requires |

**The antenna feed — the sheet's figure 3-2, on the board's own bias rail.** The antenna's LNA takes its
supply over the coax from **`ANT_BIAS`, a branch of the 3,3 V behind its own shielded 2,2 µH
`SWPA252012S2R2MT` with the house π pair and a 100 mA polyfuse** — its own rail, so that what the coax
brings in lands on this branch and not on the module's `VCC`. From there: **`L_FEED` 68 nH 0603 RF
inductor** onto the antenna node; **100 nF ∥ 100 pF** at the bias side of the inductor; **`C_BLK` 100 pF**
from the antenna node into `ANT_IN`; on the antenna node an ESD part rated for the GHz line, **`TPD1E05U06`**
(0,5 pF); on the bias a TVS of the rail's class, **`SMF5.0A`**. The kiloampere part of the coax's
protection is the two DC-pass gas arrestors of the cabling section, not these.

- **UART** to the STM32H523 (one enclosure — no RS-485 to the receiver): COM1 the node's own
  GNSS processing, **COM2 the lean time stream, relayed by the H523 onto the clock run** —
  so the H523 owns that run's `RXD`/`TXD` and can turn Kronos's ranging edge round on them —
  COM3 the configuration at boot and its answers; the satellites' azimuths come on COM1 in `SATSINFO`.
- **PPS output** — on a Sputnik station this receiver IS the station's GPS: **the PPS goes to
  Kronos's socket directly**, off the same net that this board captures, through 33 Ω and never
  through the processor; the lean `RMC`/`GGA` stream goes with it on channel A, relayed by the H523
  so that the H523 owns the run's `RXD`/`TXD`. COM1 keeps serving the node's own GNSS processing —
  no contention.
  The wiring and the per-station-type rule are `../core/blocks/gps-pps.md`'s; Kronos
  disciplines and distributes, this is only its source. Both wires cross into the head
  enclosure on the run's **Galvani communication board**, channel A both ways and the PPS on
  channel B, reversed by Kronos's socket (`../core/blocks/gps-pps.md`, *The receiver lives in the enclosure*).

---

## GNSS Antenna — recommendation

A **full-band active GNSS antenna** is required (the whole point is multi-frequency
TEC). Key parameters:

- **Band coverage** 1165–1300 MHz (L5 / L2 / E5 / E6 / B2 / B3) **and** 1525–1610 MHz
  (L1 / E1 / B1 / G1). No S-band element: the UM980 tracks NavIC on L5 alone.
- **Active LNA** ~38–40 dB — **the sheet's optimum is 18–36 dB at `ANT_IN`**, so the coax loss is
  counted in, and a short run with a 40 dB antenna takes a 3–6 dB inline pad at the board — powered
  over the coax from the board's `ANT_BIAS`, 3,3 V (*GNSS Receiver*, the feed network).
- **Phase-centre stability** and **multipath rejection** — the dominant low-elevation
  TEC error; low-elevation satellites are **flagged as lower-quality but not gated out**
  (all satellites are taken — `BUS.md`, *Rate and bandwidth*).

| Tier | Example (or equivalent) | Note |
|---|---|---|
| Research / best | Tallysman VeroStar VSP6037L, Harxon HX-CSX601A | full-GNSS + L-band, excellent multipath rejection |
| Mid | Beitian BT-300 / BT-200 | full multi-band, good value |
| Survey / choke-ring | Harxon HX-CHX600A | best phase-centre stability for fixed installs |

---

## Cabling & connectors — recommendation

- **NIC bus:** UTP Cat 6 (outdoor, UV-stable) through its gland into the communication board's
  terminal, plus the feed's own 2-core cable on its own gland into the power board's terminals. **What each pair
  carries is the plugged Galvani board's**, not this board's: Sputnik ends at the Galvani connector and
  sees logic only (`../galvani/README.md`).
- **Antenna coax:** 50 Ω low-loss — **RG-58** for short runs (< 5 m), **LMR-195/240**
  to ~15 m, **LMR-400** beyond; **SMA or TNC** connectors. Loss before the LNA
  directly costs C/N₀, so keep it short or step up the cable grade.
- **Antenna-line surge: two inline GNSS lightning arrestors** (gas-discharge, **DC-pass** for
  the bias-tee — both of them): one at the antenna, one at the enclosure entry bonded to the
  entry earth. The rule and its reasoning are block-level (`../core/blocks/gps-pps.md`); this is
  the part class.
- **Power:** 12 V on two `DGPS2.5R-5.0` terminals — the battery wire in the box, the unit power board's 12 V remote — and the 3,3 V made on board (*Power*, below).

---

## Two buses in the base, and what standing outside costs

**Sputnik carries two buses, not one.** The first is its **NodBus** link — frames on channel A,
the wire clock on channel B from its card, like any node. The second is what it owes the clock board:
**the lean `RMC`/`GGA` stream plus the PPS**, because on a Sputnik station this receiver *is* the
station's GPS (*GNSS Receiver*, above). Inside the head enclosure that second bus is a cable and
there is nothing to decide.

**The node stands IN the enclosure, and that is the rule, not a preference**
(`../core/blocks/gps-pps.md`, *The receiver lives in the enclosure*). The antenna is sited for the
sky on the station's own mast and only the coax is long; a site whose mast cannot see the sky is
sited wrong. So **the second bus is an in-box cable**: channel A the time stream, channel B the
PPS inward at Kronos, centimetres, nothing to range.

**The remote build is not deleted, it is not the base build.** Both sockets, both straps, the
turnaround timer and Kronos's ranging stay fitted — the bus contract is that every NOD link is the
same link, so the capability is carried everywhere and the deployment rule says where it is
exercised. **Outside, each bus takes a Galvani run of its own**, and the second one runs the
channel reversed: channel A carries the time stream, channel B carries **PPS inward** instead of
the clock outward.

**Sputnik therefore carries two sockets, and they are what configure the boards.** Both are the
data body, both take any board of the family, and each is wired to strap `B_DIR`, the
channel's direction — **this end listens on the NodBus socket (the card's clock in, `B_DIR` tied to
ground) and drives on the RX/TX + PPS socket (the PPS out to Kronos, `B_DIR` to 3,3 V)**. The
plugged board configures its channel B from that strap and no processor pin reads it,
so **there is no jumper on either board and no way to wire the pair backwards**
(`../galvani/README.md`, *The reversed channel*). On glass the reversed run costs a transmitter
at the far end and nothing else — both module sections populated (four fibres), or a BiDi part
(two).

| where the node stands | port boards |
|---|---|
| **in the head enclosure** | the **crossed in-box cable** to a card's port — the link; the 12 V off the wire on the unit's own terminals |
| **remote**, the mast included | a unit power board, and the communication board for the medium — copper or glass (`../galvani/`) |

**The PPS run's delay is subtracted, and that is not optional** — at ~5 ns/m a kilometre is 5 µs
against a ±1 µs contract. **Kronos ranges the run on its channel A**, the house turnaround on
this board: the clock run's `RXD` on CH1 or CH2 and its `TXD` on a compare channel of **one
timer** — two such pairs exist for this board's two runs, USART1 on PB6/PB7 (`TIM4`) for the
NodBus link and USART6 on PC6/PC7 (`TIM3`) for the clock run, the UM980's COMs on the plain
USARTs (`../bifrost/HARDWARE.md`, the pair table) — one-pulse mode, the return after a programmed count of ticks, no CPU in the path
(`../bifrost/HARDWARE.md`, *Ranging*); the PPS on channel B of the same cable rides the
measured route. The run's surveyed length, or the **ranged NodBus spur pulled alongside it**, is
the fallback (`../core/blocks/gps-pps.md`). **On a station whose GPS Sputnik is not, the second run does not
exist** and only the NodBus link leaves the box.

**What pulls the node out is the sky, not the cable.** A wooded or steep site puts the station
where it can be reached and the horizon on a ridge above it, and the antenna coax will not make
that distance — loss ahead of the LNA is C/N₀ directly (*Cabling*, above). Moving the node beats
moving the signal.

---

## Power

**Sputnik takes 12 V on its two terminals — two two-pole `DGPS2.5R-5.0`, an input and a tap — and
makes its one rail from it.** What reaches it is the 12 V — the battery wire tapped on its own
terminals through its own fuse in the enclosure's fuse field, sized by the construction, where it
shares the head's enclosure, the unit power board's 12 V where it stands outside — behind one
**`5.0SMDJ14A`** across the input, and **one `LMR43610` at 3,3 V** on the board feeds everything
(`../galvani/README.md`). No protection ladder: that lives on the Galvani boards, and the 485 begins
there. The buck's `MODE` strap is auto — nothing on this board listens in a band.

**Everything on this board runs off that 3,3 V, the UM980 included.**

**The GNSS module takes the same rail.** It draws 145 mA typical and 180 mA at most, with an inrush
into its own capacitors at power-on, and the `LMR43610` is a 1 A part — enough for the H523, the
module, the antenna's bias and the sockets. The module asks ≤ 50 mV of ripple on its 3,0–3,6 V; the
buck's own ripple into its 10 µF bank and the three 10 µF at the module's pins is millivolts. A linear stage in front
of it would pay ~0,35 W for noise performance the module is not known to need. **The bench
criterion**: the UM980's C/N₀ on the board's 3,3 V against a linear-fed 3,3 V, the same antenna and
sky — equal within the receiver's own reading spread; a second `LMR43610` at 4,0 V into an LDO is the
fallback if it is not.

---

## The NIC bus — logic out, nothing else

**No 485 part on this board and no Modbus.** Data and clock leave on the Galvani connector
as plain logic; the transceivers, the barrier, the ladder and the termination are the plugged
Galvani board's (`../galvani/README.md`).

---

## Enclosure

**The board stands in the station's enclosure; only the antenna is on the mast**, sited for the sky,
on the coax above. Palatine's met on the same site gives the tropospheric correction in place of a
model (`PROCESSING.md`).

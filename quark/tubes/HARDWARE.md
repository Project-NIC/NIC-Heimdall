★ N.I.C. ★

# Quark-Tubes — the counting board

> **Design-stage concept — nothing built.** The figures are verified against the parts' sheets and
> the parts before a board is made.

> **The HV variant's one board.** What the unit is and what stands on it: [`README.md`](README.md);
> the heads it counts — the GM shaper board: [`HEADS.md`](HEADS.md), Helion's He³ tube:
> [`helion/`](helion/), the Gadolin/Rhodion ring: [`gadolin/`](gadolin/); the mini frame: [`BUS.md`](BUS.md); the firmware:
> [`FIRMWARE.md`](FIRMWARE.md). The two low-voltage boards are [`../scintillation/photon/`](../scintillation/photon/) and
> [`../scintillation/positron/`](../scintillation/positron/). Rejected and superseded states are [`../WHY.md`](../WHY.md).

**`Quark-Tubes` is the unit — NodBus mini type 2, the tube counter — and its one board. `Quark`
alone is the radiation part and its folder, never a unit and never a board.** Nothing here carries
a number.

---

## `Quark-Tubes` — the HV counting board

**One H523, and it counts every tube in the assembly.** It is the unit **Quark-Tubes** itself: a
mini-NOD on an Argus segment, publishing four derived channels on the mini frame
([`BUS.md`](BUS.md) owns the layout).

| | |
|---|---|
| processor | **STM32H523** — the burst counting needs it; a slave-class part is too slow |
| **inputs — 18 pins, and no interrupt is shared** | **a build uses 3 + 13 with Gadolin/Rhodion, 3 + 2 with a He³/BF₃ tube.** Three GM tubes on timer inputs, the neutron detector on the rest — the He³ head's two comparators on two timers |
| high voltage | **one potted ~400 V module for every GM tube in the assembly** — Photon's three and the Gadolin/Rhodion ring's thirteen — on one bus with a ~20 µF reservoir at the module and a per-tube RC before `Ra` (*The one 400 V source*, below); the He³ tube and `Neutron`'s photomultiplier take the kV source, a module of the same construction that Helion owns (`helion/HARDWARE.md`). Pot the electronics, leave the tubes bare, because potting attenuates beta. **The module is the kV source's construction with no ladder stage fitted** — the `LT8331` flyback, its winding's 400 V straight out (`helion/HARDWARE.md` §2) |
| **13 tubes on EXTI — K4** | Gadolin/Rhodion, 1 centre + 12 in a ring. **Software interrupts because the merge compares arrival times**, so one particle lit across several tubes counts once |
| **timer inputs** | Photon's three GM tubes of one type — K1 bare, K2 β-stopped, K3 behind Pb (`HEADS.md`) — and a He³/BF₃ tube on K4 and K4A where fitted — the neutron threshold and the LLD — counted without the core noticing |
| thermometers | **five NTCs of one kind, never in the data** — the board's rides the minute's `REPORT` frame: one on this board for the block, one on each head's board — K1 · K2 · K3 on the GM tubes' shaper boards, K4 on the He³ preamplifier — and the correction is applied here, per channel, before the counts are published. Gadolin/Rhodion's ring reads the board's. No digital part and no bus |
| bus | a Galvani data body and a power body to the unit end of its Argus segment (*The sockets*, below); no transceiver on this board — the 485 or the glass is the communication board's |

**The heads it counts carry no MCU.** A tube build is tube, HV and pulse shaping and nothing
else — a shaper board at each GM tube, a charge preamplifier at the He³ tube — and its pulses
arrive here on the wire. That is what makes one counting board serve two units'
tube builds and Gadolin/Rhodion.

**The thermometers sit on the head boards, one per counting channel.** The head is the one
thing physically on the tube, it already carries a shaper or a preamplifier, and the tube, the
head board and `Quark-Tubes` all sit in one stainless block that is one thermal mass — so an SMD
part on the head board reads the tube to a fraction of a degree, reflow-soldered, nothing glued
to a tube. The correction consumes one temperature per channel and no more: `K × (1 + α × (T −
20 °C))` per channel, α small, default 0, so what the sensor owes is repeatability and not
accuracy. **For Gadolin/Rhodion the ring's thirteen tubes merge into the single channel K4**, and
that channel reads the board's sensor — one corrected count cannot carry thirteen temperatures,
and a thermometer per ring tube would be a diagnostic bought with thirteen pins (`../WHY.md`).

**Five NTCs of one kind, on `ADC1`.** The part is **`NCP18XH103F03RB`** — Murata, 0603, 10 kΩ
±1 % at 25 °C, B 3380 K ±1 %, −40…+125 °C, JLCPCB basic `C8545` — on the pattern Gauss and
Quake carry between the coils: a ratiometric divider against a 10 kΩ 1 % fixed resistor, the
divider's top on **`NTC_EN`, PE7**, high for the conversion only, read against `VREF+` so the
rail drops out. `NTC_B` on PC3 is the board's own, the block's ambient; `NTC_1`…`NTC_4` on
PA0–PA3 are the four heads', K1 to K4 in that order, two wires each on the head cable, the
fixed resistor and the station's **3 kΩ + 100 nF at the ADC pin** on this board, so that what a tube's
discharge couples into the pair (~50 pF a metre) lands on 100 nF and decays in a millisecond.
The reading is 0,3–0,5 °C absolute uncalibrated and 0,1 °C after one point; nothing on this
board asks for more. **An open divider reads full scale and is a head without its sensor**, which
then uses the board's — the map of which head has its own is made at bring-up (`FIRMWARE.md`
§A2) and a reading that jumps is dropped, not believed (`FIRMWARE.md` §A6). One GPIO feeds five
20 kΩ dividers at under 1 mA.

**The one 400 V source.** Every GM tube of a build — Photon's K1–K3 and, where the ring is
fitted, Gadolin/Rhodion's thirteen — is the same `SI-22G` class on the same plateau, so one
module feeds them all and Gadolin carries no source of its own. The load never decides it: a GM
pulse is ~50 pC (3,1·10⁸ e⁻), a tube at its 1 000 CPS ceiling draws 50 nA, sixteen tubes all at
the ceiling 0,8 µA — 0,3 mW at 400 V — and the monitor divider and the bleeder are the whole
standing draw at a few µA. What the bus is built for is the sag: **~20 µF at the module** holds
the rail to microvolts a pulse, and the ring's one-particle salvo of thirteen tubes, 0,65 nC, to
under 0,1 mV. **Per tube, on the bus side: an `R_iso` of ~1 MΩ and 100 nF to ground, then `Ra`
per the tube's sheet at the anode** — the RC is what keeps one tube's discharge off the others'
cathodes through their own capacitance, and it stands before `Ra` because anything between
`Ra` and the anode is charge that dumps into the discharge. `HV_EN` and `HV_MON` are the
module's; the bus and the anode strings are wire, terminals and the per-tube parts on the head
boards (`HEADS.md`, `gadolin/HARDWARE.md`).

## The processor — `Quark-Tubes`: pins, timers, clock tree

*The record to draw the H523 from and to set CubeMX by. Package LQFP100, `STM32H523VE`; pin
numbers and alternate functions from DS14540 Rev 3, Table 13. Every pin below is either
assigned or listed free — nothing is left to guess.*

### Peripherals

| peripheral | instance | pins | job | mode |
|---|---|---|---|---|
| the mini link | **USART1** + **TIM4** | PB6 `TXD` · PB7 `RXD` | NodBus mini behind an Argus, type 2 | the pins switch USART → TIM4 for the ranging instant |
| the grid | **TIM2** (32-bit) | PA15 `CLK_SEG` on `ETR` · PB3 ← `RXD` on CH2 | the segment's rung is the counter's clock; Argus's frame start is the anchor | external clock mode 2; a compare per frame latches the four counters |
| echo receiver | **USART3**, RX only | PD9 `RXD_ECHO` | hears the board's own frames | |
| K1 · K2 · K3 | **TIM3** · **TIM8** · **TIM1**, CH1 as external clock | PB4 · PC6 · PE9 | the three GM tubes, counted in hardware — the core never sees a pulse | external clock mode 1, 16-bit counters read every frame and cleared on the second |
| K4 | **TIM15**, CH1 as external clock | PE5 | the He³/BF₃ tube where fitted — the neutron threshold's count | same |
| K4A | **TIM12**, CH1 as external clock | PB14 | the He³ head's LLD count, γ + n — the tube's health beside `K4` (`helion/HARDWARE.md` §4) | same |
| the neutron ring | **EXTI** 0 · 1 · 2 · 3 · 4 · 5 · 6 · 7 · 8 · 9 · 10 · 11 · 12 | PD0 · PD1 · PD2 · PD3 · PC4 · PD5 · PD6 · PD7 · PD8 · PC9 · PD10 · PD11 · PD12 | Gadolin/Rhodion, 1 centre + 12 ring — software interrupts because the merge compares arrival times | rising edge; the handler reads TIM2 for the arrival time |
| high voltage | GPIO · **ADC1** | PE6 `HV_EN` · PC2 `HV_MON` | the ~400 V module | |
| thermometers | **ADC1** | `NTC_B` PC3 the board's; `NTC_1`…`NTC_4` PA0–PA3 the four heads', K1–K4; `NTC_EN` PE7 | five `NCP18XH103F03RB` dividers, corrected here, never in the data — the board's in the `REPORT` frame (*The thermometers sit on the head boards*) | the dividers on for the conversion only; read every 10 s |
| power body | **I2C1** | PB8 · PB9 | the unit power board's `INA238`, alone on its controller, 4,7 kΩ pull-ups to 3,3 V | 100 kHz; `ALERT` on PB13, 100 kΩ to ground, high = alarm |
| ID reads | **ADC1** | PC0 · PC1 | one resistor per body, against 10 kΩ | read once at bring-up |
| clock | **HSI, disciplined by the rung** | no pin — PH0 · PH1 free | there is no crystal on this board (*The rung disciplines the core*, below) | `TIM5` gates the core against the rung, `FRACN` steers the PLL |
| debug | **SWD** | PA13 · PA14 | | no JTAG, no trace |

Eighteen inputs and no interrupt shared: five on timers that count without the core, thirteen
on thirteen distinct EXTI numbers — which is why the ring sits on port D and borrows PC4 and
PC9 for the two numbers port D has spent elsewhere. The `INA238`'s `ALERT` is the fourteenth
EXTI and sits on PB13, above the ring's 0–12: PC8 is EXTI8 and tube 9 is on PD8.

### Pins

| pin | port | signal on the board | AF / function | dir | note |
|---|---|---|---|---|---|
| 1 | PE2 | `DE` | GPIO | out | the data body's driver enable |
| 2 | PE3 | — | | | free |
| 3 | PE4 | `SD` | GPIO | in | the data body's optical no-light — 100 kΩ to ground; a copper board leaves it unpopulated |
| 4 | PE5 | `K4` | `TIM15_CH1`, external clock | in | the He³/BF₃ tube where fitted — the charge preamplifier's discriminated pulse |
| 5 | PE6 | `HV_EN` | GPIO | out | the ~400 V module's enable |
| 6 | VBAT | — | | | tied to `VDD`; no RTC anywhere |
| 7 | PC13 | — | | | free (tamper/RTC pin, unused) |
| 8 | PC14 | — | | | free — no 32 kHz crystal |
| 9 | PC15 | — | | | free |
| 10 | VSS | | | | |
| 11 | VDD | 3,3 V | | | |
| 12 | PH0 | — | | | free — no crystal on this board |
| 13 | PH1 | — | | | free |
| 14 | NRST | reset | |  | 100 nF, no pull beyond the internal |
| 15 | PC0 | `ID_D` | `ADC12_INP10` | in | the data body's `ID` |
| 16 | PC1 | `ID_P` | `ADC12_INP11` | in | the power body's `ID` |
| 17 | PC2 | `HV_MON` | `ADC12_INP12` | in | the module's monitor output, where the part has one |
| 18 | PC3 | `NTC_B` | ADC | in | the board's NTC, on its divider — the block's ambient |
| 19 | VSSA | | | | |
| 20 | VREF− | | | | |
| 21 | VREF+ | 3,3 V through the shielded 2,2 µH, 10 µF + 100 nF at the pins — the ADC reference | | | |
| 22 | VDDA | 3,3 V through the shielded 2,2 µH, 10 µF + 100 nF at the pins | | | |
| 23 | PA0 | `NTC_1` | ADC | in | K1's head NTC, on its divider, RC at the pin — an unfitted head reads the board's |
| 24 | PA1 | `NTC_2` | ADC | in | K2's head NTC, on its divider, RC at the pin — an unfitted head reads the board's |
| 25 | PA2 | `NTC_3` | ADC | in | K3's head NTC, on its divider, RC at the pin — an unfitted head reads the board's |
| 26 | PA3 | `NTC_4` | ADC | in | K4's head NTC, on its divider, RC at the pin — an unfitted head reads the board's |
| 27 | VSS | | | | |
| 28 | VDD | 3,3 V | | | |
| 29 | PA4 | — | | | free (ADC) |
| 30 | PA5 | — | | | free (ADC) |
| 31 | PA6 | — | | | free (ADC) |
| 32 | PA7 | — | | | free (ADC) |
| 33 | PC4 | `T5` | GPIO, EXTI4 | in | Gadolin/Rhodion tube 5, the ring |
| 34 | PC5 | — | | | free (ADC) |
| 35 | PB0 | — | | | free (ADC) |
| 36 | PB1 | — | | | free (ADC) |
| 37 | PB2 | — | | | free |
| 38 | PE7 | `NTC_EN` | GPIO | out | the five dividers' top, high for the conversion only |
| 39 | PE8 | — | | | free |
| 40 | PE9 | `K3` | `TIM1_CH1`, external clock | in | GM tube 3, behind Pb |
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
| 52 | PB13 | `ALERT` | GPIO, EXTI13 | in | the power body's `INA238` — 100 kΩ to ground, high = alarm |
| 53 | PB14 | `K4A` | `TIM12_CH1`, external clock | in | the He³ head's LLD count, γ + n — the tube's health |
| 54 | PB15 | — | | | free |
| 55 | PD8 | `T9` | GPIO, EXTI8 | in | Gadolin/Rhodion tube 9, the ring |
| 56 | PD9 | `RXD_ECHO` | `USART3_RX` | in | the echo check on the board's own transmission |
| 57 | PD10 | `T11` | GPIO, EXTI10 | in | Gadolin/Rhodion tube 11, the ring |
| 58 | PD11 | `T12` | GPIO, EXTI11 | in | Gadolin/Rhodion tube 12, the ring |
| 59 | PD12 | `T13` | GPIO, EXTI12 | in | Gadolin/Rhodion tube 13, the ring |
| 60 | PD13 | — | | | free |
| 61 | PD14 | `PGOOD` | GPIO | in | the 3,3 V buck's window |
| 62 | PD15 | — | | | free |
| 63 | PC6 | `K2` | `TIM8_CH1`, external clock | in | GM tube 2, β-stopped |
| 64 | PC7 | — | | | free |
| 65 | PC8 | — | | | free |
| 66 | PC9 | `T10` | GPIO, EXTI9 | in | Gadolin/Rhodion tube 10, the ring |
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
| 77 | PA15 | `CLK_SEG` | `TIM2_ETR` | in | the segment's rung — the data body's pin 1 `CLK/PPS` into `TIM2_ETR`; the timebase counts it |
| 78 | PC10 | — | | | free |
| 79 | PC11 | — | | | free |
| 80 | PC12 | — | | | free |
| 81 | PD0 | `T1` | GPIO, EXTI0 | in | Gadolin/Rhodion tube 1, the centre — thirteen EXTI lines, no two on one number |
| 82 | PD1 | `T2` | GPIO, EXTI1 | in | Gadolin/Rhodion tube 2, the ring |
| 83 | PD2 | `T3` | GPIO, EXTI2 | in | Gadolin/Rhodion tube 3, the ring |
| 84 | PD3 | `T4` | GPIO, EXTI3 | in | Gadolin/Rhodion tube 4, the ring |
| 85 | PD4 | `LED` | GPIO | out | the status LED, fitted — 1 kΩ from the 3,3 V, one 50 ms blink a minute while healthy, two on a fault (`../../core/HARDWARE.md`) |
| 86 | PD5 | `T6` | GPIO, EXTI5 | in | Gadolin/Rhodion tube 6, the ring |
| 87 | PD6 | `T7` | GPIO, EXTI6 | in | Gadolin/Rhodion tube 7, the ring |
| 88 | PD7 | `T8` | GPIO, EXTI7 | in | Gadolin/Rhodion tube 8, the ring |
| 89 | PB3 | `RXD` | `TIM2_CH2` capture | in | the same net as PB7 — the timebase captures the start edge of Argus's frame |
| 90 | PB4 | `K1` | `TIM3_CH1`, external clock | in | GM tube 1, bare — the shaper's pulse |
| 91 | PB5 | — | | | free |
| 92 | PB6 | `TXD` | `USART1_TX` / `TIM4_CH1` | out | the NodBus mini link, to the data body |
| 93 | PB7 | `RXD` | `USART1_RX` / `TIM4_CH2` | in | the pins switch USART → TIM4 for the ranging instant |
| 94 | BOOT0 | 10 kΩ to ground | |  | |
| 95 | PB8 | `SCL` | `I2C1_SCL` | i/o | the power body's `INA238`, alone on this bus — 4,7 kΩ to 3,3 V |
| 96 | PB9 | `SDA` | `I2C1_SDA` | i/o | 4,7 kΩ to 3,3 V |
| 97 | PE0 | — | | | free |
| 98 | VCAP | 2× 1 µF 50 V 0805 + 2× 100 nF 50 V 0603 — 2,2 µF, the sheet's 2,2 µF ±20 % | | | |
| 99 | VSS | | | | |
| 100 | VDD | 3,3 V | | | |

**42 GPIO used, 38 free** (PA4–PA12 · PB0–PB2 · PB5 · PB10 · PB12 · PB15 · PC5 · PC7–PC8 · PC10–PC15 · PD13 · PD15 · PE0 · PE3 · PE8 · PE10–PE15 · PH0–PH1).

### Timers

| timer | bits | kernel | pins | job |
|---|---|---|---|---|
| **TIM4** | 16 | 2²⁷ | CH1 → PB6, CH2 ← PB7 | the ranging turnaround: one-pulse, triggered by the capture on CH2, the return raised on CH1 after `CCR` ticks; PWM-input mode on CH2 measures the incoming pulse's width |
| **TIM2** | 32 | the rung on `ETR` (PA15), external clock mode 2 | CH2 capture ← PB3; CH3/CH4 compares, no pin | **the grid**: the counter runs on the segment's rung, not on the core, and a compare per frame is the latch instant for the four counters — at 2¹⁹, 4096 ticks is 128 Hz; the capture of Argus's frame start anchors the slot |
| **TIM3** | 16 | `K1` on CH1, external clock mode 1 | | GM tube 1 — the counter is the count |
| **TIM8** | 16 | `K2` on CH1, external clock mode 1 | | GM tube 2 |
| **TIM1** | 16 | `K3` on CH1, external clock mode 1 | | GM tube 3 |
| **TIM15** | 16 | `K4` on CH1, external clock mode 1 | | the He³/BF₃ tube — the neutron threshold |
| **TIM12** | 16 | `K4A` on CH1, external clock mode 1 | | the He³ head's LLD — γ + n, the tube's health |
| **TIM5** | 32 | 2²⁷ | no pin — latched by `TIM2`'s frame compare over the internal trigger | **the discipline loop's gate**: the core counted against the rung, 1 048 576 counts a frame at 0,95 ppm each, or a second's worth at 0,0075 ppm (*The rung disciplines the core*) |
| LPTIM1 · LPTIM2 | | | | free |
| **TIM6** | 16 | 2²⁷ | no pin | **the rung watchdog**: re-armed by every `TIM2` frame compare; two frame periods without one is the rung lost, and the node mutes (`FIRMWARE.md` §A3) |
| TIM7 | 16 | | | free |

A 16-bit counter wraps at 65 535 pulses; a GM tube at its dead-time limit makes a few thousand a
second, and the counter is cleared on every second, long before it wraps.

### Clock tree

| | |
|---|---|
| HSE | **not fitted, and the position is deleted** — the mini tier carries one clock tree, Gauss's and Pascal's: the rung disciplines the HSI (*The rung disciplines the core*, below) |
| the core and the UART | **HSI, disciplined by the rung through the PLL.** `TIM5` gates the core against the rung, the correction goes into `FRACN`, and `HSITRIM` steps one code whenever `FRACN` nears the edge of its window |
| PLL1 | source HSI, `HSIDIV` 1 · **M 8 → an 8 MHz reference** · N 33 + `FRACN` (nominal ratio 33,554432, `FRACN` near 4542) · P 2 → VCO 2²⁸, `P` 2²⁷. **The binary SYSCLK is made by the loop, not by an oscillator's value** |
| SYSCLK, AHB, APB1/2, timer kernel | **2²⁷ = 134,217728 MHz** |
| **the grid** | **TIM2 on the segment's rung**, `ETR` on PA15 — **2¹⁹, and it is not a per-segment choice**: an Argus hands all four segments one number, its role's, and 2¹⁹ is what the mini tier's weakest medium carries (`../../bifrost/HARDWARE.md`, `../../galvani/README.md`) — divided to the sample rate by whole powers of two; the core is out of the time path entirely |
| the rung gone | missing edges on `ETR` are the loss event: the loop freezes on its last code and the node mutes — a lost rung is a lost link, so nothing is held over (`../../core/blocks/nodbus.md`) |

### Analogue and reference

**Every filter inductor on this board is a π filter: 10 µF 1206 + 100 nF 0603 before it on the rail and
the same pair at the pins behind it** — the house rule (`../../core/POWER.md`, *The filter parts*).

`VDDA` and `VREF+` off the 3,3 V through the shielded 2,2 µH `SWPA252012S2R2MT` each, 10 µF + 100 nF at the pins; `VREF−`
and `VSSA` to the ground plane at one point. The two `ID` inputs are read single-ended against
`VREF+`, and the 10 kΩ 1 % pull-up of each `ID` resistor hangs on `VREF+` itself, so the reading is a
ratio and the rail's tolerance drops out. The arrived 12 V and the input current are the power
body's `INA238`, over I2C1 — no divider on this board. The HV monitor on PC2 is read through the module's own divider, against
the same reference. The five NTC dividers hang on `NTC_EN` and are read against `VREF+` the
same way — a ratio, since the GPIO's high level and `VREF+` are both the 3,3 V.

### The rung disciplines the core

**The same loop as Gauss and Pascal, one clock tree across the mini tier.** The grid is `TIM2`
counting the rung and is exact whatever the core does; the core clocks the UART and the ranging
turnaround, and those are what the loop is for.

- **The gate.** `TIM5` counts SYSCLK and `TIM2`'s frame compare latches it over the internal
  trigger — no pin, no interrupt in the path. Two latches must differ by **1 048 576**
  (2²⁷ ÷ 128); one count is **0,95 ppm**, and every 128th latch is a one-second gate at
  0,0075 ppm.
- **The correction.** `FRACN[12:0]` in PLL1, written with the PLL running. The reference is
  **8 MHz** (`M` 8 off the 64 MHz HSI): the pull range is **−1,31 % / +1,68 %** and one step
  **3,6 ppm**; a residual finer than a step is dithered between two adjacent codes.
- **`HSITRIM` lands the HSI inside that window at bring-up, and steps again whenever `FRACN`
  passes a guard band a quarter of the window from either edge** — a land board's HSI drifts
  −2 / +1 % over the year, more than `FRACN` pulls, and the UART sees the 0,24 % step for a few
  frames, a tenth of its tolerance. One step is 0,24 % typical, but the curve goes negative at
  multiples of 32 and as far as −5,2 % at 128, 256 and 384 (DS14540 Rev 3, Table 43), so each
  candidate code is measured on the gate and a code on one of those boundaries is stepped off.
- **A land board sees −40…+60 °C and the loop does not mind**: the reference is half a million
  edges a second and never stops while the node runs, and the HSI's temperature drift is slow
  against a one-frame gate. **There is no holdover table** — unlike a sonde, this board has no
  reason to keep time without the rung: a lost rung is a lost link, the node mutes, and on the
  rejoin the loop closes on the one-frame gate before the first frame is sent.

## The rails

**An H523 on 3,3 V off the 12 V it is handed** — the unit power board's island output, taken on
two two-pole Degson **`DGPS2.5R-5.0`** terminals, an input and a tap, the same node; no 12 V on
either ribbon. The 3,3 V is the house `LMR43610`, the `R3` code, by the
family rule (`../../galvani/HARDWARE.md`, *The buck cell*); the one voltage that is its own is the
potted ~400 V module. **No rail is switched from a pin**: the module sleeps by its own `HV_EN`.

## The sockets — the data body and the power body

**Connectors: `MINI IN` · `PWR IN` · `K1`…`K4` — the heads · `RING` — Gadolin/Rhodion's thirteen lines · `HV GM` — the 400 V bus · `HV He` — the kV source's feed to the He³ head · `12V`** (`../../galvani/README.md`, *Connector names*).

**`Quark-Tubes` is a measuring unit and stands at the U end of its segment, never in the station's
enclosure.** Everything on the Galvani boards behind it runs whenever the board does, held by
resistors: no processor pin switches anything on either body.

**The data body — 12 pins, 2×6, `BX2.54-2xNA`.**

| pin | signal | on this board |
|---|---|---|
| 1 | `CLK/PPS` | the segment's rung, 2¹⁹, into PA15 `TIM2_ETR` |
| 2 | `GND` | ground |
| 3 | `TXD` | PB6 `USART1_TX` / `TIM4_CH1` |
| 4 | `RXD` | PB7 `USART1_RX` / `TIM4_CH2`, and PB3 `TIM2_CH2` on the same net |
| 5 | `ID` | PC0 `ID_D`, 10 kΩ 1 % to `VREF+` |
| 6 | `ID_RET` | not connected — a unit carries no host code |
| 7 | `RXD_ECHO` | PD9 `USART3_RX` |
| 8 | `DE` | PE2, GPIO out |
| 9 | `B_DIR` | **tied to ground** — this end listens on channel B; no processor pin |
| 10 | `LINE_EN` | 10 kΩ to 3,3 V — the line side runs whenever the board does; no processor pin |
| 11 | `SD` | PE4, GPIO in, 100 kΩ to ground |
| 12 | `3,3 V` | the board's 3,3 V, the `LMR43610` |

**The power body — 8 pins, 2×4, `BX2.54-2xNA`.**

| pin | signal | on this board |
|---|---|---|
| 1 | `ENABLE` | not connected — the unit power board pulls it to its input |
| 2 | `GND` | ground |
| 3 | `ID` | PC1 `ID_P`, 10 kΩ 1 % to `VREF+` |
| 4 | `SDA` | PB9 `I2C1_SDA`, 4,7 kΩ to 3,3 V |
| 5 | `A_SEL` | to ground — the `INA238` at 0x40, alone on I2C1 |
| 6 | `SCL` | PB8 `I2C1_SCL`, 4,7 kΩ to 3,3 V |
| 7 | `ALERT` | PB13, EXTI13, 100 kΩ to ground, high = alarm |
| 8 | `3,3 V` | the board's 3,3 V, the `LMR43610` — the power board's supply |

**The 12 V** arrives from the unit power board on the two `DGPS2.5R-5.0` terminals and on neither
ribbon.

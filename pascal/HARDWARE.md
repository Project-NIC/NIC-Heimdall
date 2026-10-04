★ N.I.C. ★

# Pascal — hardware

> **Design-stage concept — nothing built.** The figures are verified against the parts' sheets
> and the parts before a board is made.

**The body is Gauss's sea build** (`../gauss/CONSTRUCTION.md`); what Pascal adds to it is
`CONSTRUCTION.md`. This file is the board.

## What is on it

| part | function |
|---|---|
| **STM32H523**, LQFP100 | the whole unit — the mini link, the grid, the gauge, the payload |
| **the depth gauge** | **TE Connectivity `MS5837-30BA`** (`MS583730BA01-50`), 30 bar, I²C, pressure and temperature, **in a Blue Robotics `Bar30` housing** — an M10 threaded penetrator screwed through a hole in the tube and sealed (`CONSTRUCTION.md`); the gauge's gel face meets the water and the fill stays out of the measurement; its four leads land on I2C3 |
| **`TMP117`** | the wall thermometer, on the same I²C — thermally coupled to the tube wall, the bottom-water record |
| **`LMR43610R3RPER`** at 3,3 V | the one rail, off the 12 V on the board's two terminals — the unit power board's island output at the far end of the run. The house cell: `RT` 7,50 kΩ → 2,08 MHz, auto mode; 4,7 µH shielded, `I_SAT` ≥ 3,5 A; `R_FBT` 28,0 kΩ / `R_FBB` 12,1 kΩ, `C_FF` 22 pF C0G; `C_IN` 4,7 µF + 100 nF; `C_OUT` 3× 10 µF 1206 + 100 nF; `VCC` 1 µF; `BOOT` 100 nF (`../galvani/HARDWARE.md`, *The buck cell*). **The 12 V input bank is 5× 10 µF 50 V 1206 + 100 nF, ceramic only** — a pressure-balanced body takes no hybrid polymer and no electrolytic |
| **no crystal** | this body is oil-filled and pressure-balanced, and a crystal is a sealed gas cavity: the core and the UART run on the **HSI disciplined by the rung through the PLL** (*The rung disciplines the core*) |
| the Galvani **data body** and **power body** | the mini link and the feed's telemetry; the unit-end boards are potted in beside it |

No transceiver, no protection ladder, no 485 on this board: the line side is the plugged
communication board's, the ladder and the island the unit power board's (`../galvani/README.md`).
No rail is switched: the gauge idles in microamps between conversions on its own.

## What it measures, and how

**A change of pressure in the minutes band, from the sea floor.** The gauge's gel face takes the
water's pressure directly; a static offset — the part's, the wall's, the atmosphere's — is harmless,
because the measurement is a change and never an absolute depth. The atmosphere on top of the
column is subtracted downstream from the station's barometer, and nothing on the node does it.
Creep and thermal drift sit in hours; the wave sits in minutes.

**A surface wave reaches down about half its own wavelength**, so how much of it arrives at the
gauge is set by its period, not its height. The bottom attenuation is 1/cosh(2πh/L), and five
seconds of period change it ninetyfold:

| wave | period | wavelength | at 200 m |
|---|---|---|---|
| wind sea | 10 s | ~156 m | **1/1600** — gone |
| long swell from a distant storm | 15 s | ~350 m | **1/18** — alive |
| tsunami | ~10 min | hundreds of km | **1/1** — the whole column moves |

Depth kills the wind sea and leaves the long swell: a 2 m, 15 s swell still lands ~11 mbar on the
bottom, the size of the tsunami itself. **What separates them is the band** — swell is seconds, a
tsunami is minutes — so the gauge is **burst-sampled and averaged over the wave period**, sixteen
conversions a second into a 16 s mean (`FIRMWARE.md` §5). One reading a minute would alias the
swell straight into the tsunami band.

## Rails

```
 12 V on the terminals ─▶ LMR43610 ─▶ 3,3 V ─┬─▶ the H523, the communication board over the data body
                                             └─▶ the MS5837
```

`MODE` auto — nothing on this board listens in a band. **The 12 V input bank is ceramic only, 5× 10 µF
50 V 1206 + 100 nF** — no hybrid polymer and no electrolytic in a pressure-balanced body. **~60–185 mA by the communication
module** — the communication board takes its 3,3 V from this rail; the gauge's own draw is microamps between conversions. Below
the buck every capacitor is a 50 V X7R: 100 nF at every `VDD` of the H523 with 10 µF of bulk,
`VCAP` 2× 1 µF 50 V 0805 + 2× 100 nF 50 V 0603 on each of pins 48 and 98, 2,2 µF; the gauge and the thermometer 100 nF each at
their supply pins.

## The processor — pins, timers, clock tree

*The record to draw the H523 from and to set CubeMX by. Package LQFP100, `STM32H523VE`; pin
numbers and alternate functions from DS14540 Rev 3, Table 13. Every pin below is either
assigned or listed free — nothing is left to guess.*

### Peripherals

| peripheral | instance | pins | job | mode |
|---|---|---|---|---|
| the mini link | **USART1** + **TIM4** | PB6 `TXD` · PB7 `RXD` | NodBus mini behind an Argus, type 3 | the pins switch USART → TIM4 for the ranging instant |
| the grid | **TIM2** (32-bit) | PA15 `CLK_SEG` on `ETR` · PB3 ← `RXD` on CH2 | the segment's rung is the counter's clock; Argus's frame start is the anchor | external clock mode 2; a compare every 2ⁿ ticks is the sample instant |
| echo receiver | **USART3**, RX only | PD9 `RXD_ECHO` | hears the board's own frames | |
| the gauge | **I2C3** | PD6 `SCL` · PD7 `SDA` | the MS5837-30BA, pressure and temperature; the `TMP117` on the wall, read every 10 s | 400 kHz; a conversion is started on the grid and read after its `OSR` time |
| power body | **I2C1** | PB8 `SCL` · PB9 `SDA` | the `INA238` — the arrived voltage and current, sent once a minute in the `REPORT` frame and on `GET REPORT`; the SUPPLY flag in the header outside `VIN_WINDOW` — nothing rides the data payload | 100 kHz, 4,7 kΩ to 3,3 V on both lines; `ALERT` on PC8, 100 kΩ to ground, high = alarm |
| ID reads | **ADC1** | PC0 · PC1 | one resistor per body, against 10 kΩ 1 % to `VREF+` | read once at bring-up |
| clock | **HSI, disciplined by the rung** | no pin — PH0 · PH1 free | there is no crystal on this board (*The rung disciplines the core*) | `TIM5` gates the core against the rung, `FRACN` steers the PLL |
| the holdover's covariates | **ADC1** internal channels · **DTS** | no pin — both are internal | the die's temperature by two routes: `VSENSE` against `VREFINT` in one sequence, and the DTS's own count in `DTS_DR`. Indexed as raw readings, never as a temperature (*The rung disciplines the core*) | the pair in one ADC sequence; the DTS is a register read, no timer |
| debug | **SWD** | PA13 · PA14 | | no JTAG, no trace |

The gauge is on its own I²C, not on the power body's: the two run at different rates and a
sensor read must never wait behind a telemetry read.

### Pins

| pin | port | signal on the board | AF / function | dir | note |
|---|---|---|---|---|---|
| 1 | PE2 | `DE` | GPIO | out | the data body's driver enable |
| 2 | PE3 | — | | | free |
| 3 | PE4 | `SD` | GPIO | in | the data body's optical no-light — 100 kΩ to ground, so an unpopulated `SD` on copper reads quiet |
| 4 | PE5 | — | | | free |
| 5 | PE6 | — | | | free |
| 6 | VBAT | — | | | tied to `VDD`; no RTC anywhere |
| 7 | PC13 | — | | | free (tamper/RTC pin, unused) |
| 8 | PC14 | — | | | free — no 32 kHz crystal |
| 9 | PC15 | — | | | free |
| 10 | VSS | | | | |
| 11 | VDD | 3,3 V | | | |
| 12 | PH0 | — | | | free — no crystal on this board (*Clock tree*) |
| 13 | PH1 | — | | | free |
| 14 | NRST | reset | |  | 100 nF, no pull beyond the internal |
| 15 | PC0 | `ID_D` | `ADC12_INP10` | in | the data body's `ID` — 10 kΩ 1 % to `VREF+` |
| 16 | PC1 | `ID_P` | `ADC12_INP11` | in | the power body's `ID` — 10 kΩ 1 % to `VREF+` |
| 17 | PC2 | — | | | free (ADC) |
| 18 | PC3 | — | | | free (ADC) |
| 19 | VSSA | | | | |
| 20 | VREF− | | | | |
| 21 | VREF+ | the `VDDA` node — the ADC reference, strapped to `VDDA` | | | |
| 22 | VDDA | 3,3 V through the shielded 2,2 µH `SWPA252012S2R2MT`, 10 µF + 100 nF at the pins — a buck's own output feeding an ADC that reads the holdover's temperature covariate, so the filter is the buck-noise inductor of `../core/POWER.md` (*The filter parts*), not a ferrite | | | |
| 23 | PA0 | — | | | free (ADC) |
| 24 | PA1 | — | | | free (ADC) |
| 25 | PA2 | — | | | free (ADC) |
| 26 | PA3 | — | | | free (ADC) |
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
| 56 | PD9 | `RXD_ECHO` | `USART3_RX` | in | the echo check on the board's own transmission |
| 57 | PD10 | — | | | free |
| 58 | PD11 | — | | | free |
| 59 | PD12 | — | | | free |
| 60 | PD13 | — | | | free |
| 61 | PD14 | `PGOOD` | GPIO | in | the 3,3 V buck's window |
| 62 | PD15 | — | | | free |
| 63 | PC6 | — | | | free |
| 64 | PC7 | — | | | free |
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
| 77 | PA15 | `CLK_SEG` | `TIM2_ETR` | in | the segment's rung off the data body's `CLK/PPS`, behind the buffer — the timebase counts it |
| 78 | PC10 | — | | | free |
| 79 | PC11 | — | | | free |
| 80 | PC12 | — | | | free |
| 81 | PD0 | — | | | free |
| 82 | PD1 | — | | | free |
| 83 | PD2 | — | | | free |
| 84 | PD3 | — | | | free |
| 85 | PD4 | `LED` | GPIO | out | the status LED, fitted — 1 kΩ from the 3,3 V, one 50 ms blink a minute while healthy, two on a fault (`../core/HARDWARE.md`) |
| 86 | PD5 | — | | | free |
| 87 | PD6 | `SCL_SENS` | `I2C3_SCL` | i/o | the MS5837-30BA and the `TMP117`, 4,7 kΩ up |
| 88 | PD7 | `SDA_SENS` | `I2C3_SDA` | i/o | |
| 89 | PB3 | `RXD` | `TIM2_CH2` capture | in | the same net as PB7 — the timebase captures the start edge of Argus's frame |
| 90 | PB4 | — | | | free |
| 91 | PB5 | — | | | free |
| 92 | PB6 | `TXD` | `USART1_TX` / `TIM4_CH1` | out | the NodBus mini link, to the data body |
| 93 | PB7 | `RXD` | `USART1_RX` / `TIM4_CH2` | in | the pins switch USART → TIM4 for the ranging instant |
| 94 | BOOT0 | 10 kΩ to ground | |  | |
| 95 | PB8 | `SCL` | `I2C1_SCL` | i/o | the power body's `INA238` — 4,7 kΩ to 3,3 V |
| 96 | PB9 | `SDA` | `I2C1_SDA` | i/o | 4,7 kΩ to 3,3 V |
| 97 | PE0 | — | | | free |
| 98 | VCAP | 2× 1 µF 50 V 0805 + 2× 100 nF 50 V 0603 — 2,2 µF, the sheet's 2,2 µF ±20 % | | | |
| 99 | VSS | | | | |
| 100 | VDD | 3,3 V | | | |

**18 GPIO used, 62 free** (PA0–PA12 · PB0–PB2 · PB4 · PB5 · PB10 · PB12–PB15 · PC2–PC7 · PC9–PC15 · PD0–PD3 · PD5 · PD8 · PD10–PD13 · PD15 · PE0 · PE3 · PE5–PE15 · PH0 · PH1).

### Timers

| timer | bits | kernel | pins | job |
|---|---|---|---|---|
| **TIM4** | 16 | 2²⁷ | CH1 → PB6, CH2 ← PB7 | the ranging turnaround: one-pulse, triggered by the capture on CH2, the return raised on CH1 after `CCR` ticks; PWM-input mode on CH2 measures the incoming pulse's width |
| **TIM2** | 32 | the rung on `ETR` (PA15), external clock mode 2 | CH2 capture ← PB3; CH3/CH4 compares, no pin | **the grid**: the counter runs on the segment's rung — this board has no crystal at all — and a compare every 2ⁿ ticks is the sample instant — at 2¹⁹, 4096 ticks is 128 Hz; the capture of Argus's frame start anchors the slot |
| **TIM5** | 32 | 2²⁷ | no pin — latched by `TIM2`'s frame compare over the internal trigger | **the discipline loop's gate**: the core counted against the rung, 1 048 576 counts a frame at 0,95 ppm each, or a second's worth at 0,0075 ppm (*The rung disciplines the core*) |
| TIM1 · TIM3 · TIM8 · TIM12 · TIM15 · LPTIM1 · LPTIM2 | | | | free |
| **TIM6** | 16 | 2²⁷ | no pin | **the conversion timing**: the compare 18 ms after each D1/D2 command that reads the gauge's result (`FIRMWARE.md` §5) |
| TIM7 | 16 | | | free |

The burst is a compare train on TIM2: the gauge's conversions are started at 2ⁿ-tick intervals
across the wave period and averaged (*What it measures*).

### The rung disciplines the core — no crystal, and a two-stage loop

**There is no crystal and no oscillator part on this board.** A crystal is a sealed gas cavity by
construction — the blank has to vibrate in one — and this body is oil-filled, vacuum-degassed and
pressure-balanced, with no gas anywhere: the same rule that keeps a 1×9 optical module off the
sonde. Pressure pulls a crystal's frequency first and crushes its lid after.

**The sample grid needs no local clock.** The segment's rung — 2¹⁹ Hz, the one number an Argus
hands all four of its segments — arrives on the data body's `CLK/PPS` pin and lands on `TIM2`'s
external clock input, `ETR`. `TIM2` counts the rung directly: a compare every 4096 ticks is the
128 Hz frame, and every 8th of them starts a conversion. Between two rung edges lie 1,907 µs, and
nothing the core does in that time moves the edge; the grid is exact as long as the rung is there,
and what remains is the edge itself — capture, buffer and module delays, ns-class and fixed — which
the card's ranging takes out. The `MS5837` converts on its own time and is read when it is done.

**The rung cannot clock the core directly.** The H523's external-clock input on `OSC_IN` is
specified from 4 MHz and 2¹⁹ is 0,524 MHz, eight times under it. Raising the rung is not open
either: a mini segment on glass runs on the 2 Mb/s module, where a 2¹⁹ square wave is 1,05 Mb/s of
transitions and fits and a 2²² square wave would be four times over. So the core runs on the HSI,
the H523's internal RC oscillator — 64 MHz nominal, 63,7…64,3 MHz as delivered at T_J 30 °C and
drifting −2 / +1 % over the junction range (DS14540 Rev 3, Table 43) — and the firmware disciplines
it against the rung.

**Two things need the core's frequency right, and the grid is not one of them**: the UART's baud,
which binds first — a UART tolerates about 2 %, and a crystal is what every other board buys for
it — and the ranging turnaround, where `TIM4` raises the return a set number of core ticks after the
capture.

**The measurement.** `TIM5`, 32-bit, counts SYSCLK, and `TIM2`'s frame compare latches it over
the internal trigger — no pin, no interrupt, nothing the firmware does can lengthen the interval.
Two consecutive latches differ by what the core did in one frame: at 2²⁷ Hz and 128 frames a
second it must read exactly 2²⁷ ÷ 128 = **1 048 576**, and one count is **0,95 ppm**. Every 128th
latch is a one-second gate, 134 217 728 counts, **0,0075 ppm**. The measurement is taken after the
PLL because what is steered has to be what is measured.

**The correction is a fine control and a coarse one, and both run for the board's life.**

- **Fine: `FRACN`, the PLL's fractional divider.** PLL1 takes the HSI through `M` 8 — an 8 MHz
  reference, half of `f_PLL_IN`'s 16 MHz ceiling, so the HSI's whole spread and drift stay inside
  it, and the rate at which the fractional divider's sigma-delta is characterised (Table 46) — and
  multiplies to the 2²⁸ VCO with `N` 33 plus a 13-bit fraction, nominal ratio 33,554432, so `FRACN`
  sits near 4542 and not near zero. It is written with the PLL running — `PLLFRACEN` cleared,
  `FRACN[12:0]` written, `PLLFRACEN` set — and the output slides to the new value with no dropout.
  The pull range is **−1,31 % / +1,68 %** around the trimmed HSI and one step is **3,6 ppm**. The
  firmware integrates the gate's error into `FRACN` every frame while the residual is still moving
  and every second once it has settled; where the residual is finer than one step the loop
  alternates between the two adjacent codes and the duty carries the average. Nothing downstream
  is phase-sensitive — the grid comes off the rung directly — so the alternation costs nothing: a
  UART byte at 2²⁰ baud is 9,5 µs and 7 ppm of it is 70 ps.
- **Coarse: `HSITRIM`, the HSI's own trim register.** One trim step is 0,24 % typical, 0,32 %
  maximum (Table 43), about 660 `FRACN` steps. At bring-up the first gate sets it so the HSI lands
  inside `FRACN`'s window — the part as delivered may sit 0,47 % off and a cold junction another
  2 % lower, which `FRACN` alone cannot reach. **And it is stepped again whenever `FRACN`
  approaches the edge of its window**: the HSI drifts −2 / +1 % over the junction range, more than
  `FRACN` can pull. When `FRACN` passes a guard band a quarter of the window from either edge, the
  loop steps `HSITRIM` one code toward the centre and lets `FRACN` run back; the UART sees a 0,24 %
  step for a few frames, a tenth of its tolerance, and the grid sees nothing. In a sonde in a water
  column the junction sits in a narrow band and the step may never fire; the loop carries it so
  one firmware serves the mini tier's land boards as well.
- **The trim curve is not monotonic, and the firmware must know it.** Table 43 gives the step as
  *negative* at particular codes — −0,25 % typical at every multiple of 32, −0,8 % at 64, 192, 320
  and 488, and −1,8 % typical and −5,2 % minimum at 128, 256 and 384; that last one is four times
  the whole `FRACN` window. So `HSITRIM` is never binary-searched and never left sitting on a
  multiple of 32: every candidate code is measured on the gate, which costs one frame, and a code
  that lands on one of those boundaries is stepped off it.

**Holdover — the rung gone.** Missing edges on `ETR` are the loss event: the loop freezes on its
last `FRACN` code, the holdover table takes over, and the degraded flag is set until the rung
returns. The table is `FRACN` against the die's temperature, written while the rung is present —
an RC's error is large but smooth and repeatable, which is what a learned table eats; Kronos does
the same with its TCXO. A lost rung is a lost link, so the holdover carries the board to a rejoin
and never carries the station's time. The die's temperature is read two ways, by the H523's own
two sensors, both free:

| | the analogue sensor, on ADC1 | the DTS |
|---|---|---|
| what it gives | a voltage, 2 mV/°C typical, a 12-bit count | a frequency, ~2800 ppm/°C, counted by the block itself into `DTS_DR` |
| resolution | 0,40 °C a count on 3,3 V, averaged by 256× oversampling | as fine as its gate is made long |
| exposed to the rail | yes — 1 % of `VDDA` reads as 3,1 °C, so `VSENSE` is always converted with `VREFINT` in one sequence and the ratio is stored; un-normalised, the holdover would chase the buck with `PGOOD` still high | no |
| exposed to the clock | no | yes — it counts against PCLK, the clock the table corrects: in holdover a 1000 ppm error reads as 0,36 °C, ~28 ppm back through the HSI's coefficient, a 3 % residual that closes in two passes |

Both tables are indexed by the raw reading, never by a temperature, so neither sensor's slope,
offset or linearity ever enters and only repeatability is left; `TS_CAL1` and `TS_CAL2` are read
for reporting, not for the loop. A build ships with one table driving `FRACN` and the other as the
check: two sensors on two physical principles watching one die, and a divergence between them is a
fault nothing else on the board would report. The `TMP117` on the wall is the slow cross-check;
with the body isothermal and the dissipation constant the die-to-wall difference is a constant.

**From cold.** The sonde is powered, the die climbs from the water's temperature to its working
point over minutes, and the HSI moves with it. The loop tracks it on the one-frame gate and
lengthens to the one-second gate once the residual stops moving; the unit enrols as soon as the
baud is inside tolerance and waits for nothing else.

### Clock tree

| | |
|---|---|
| HSE | **not fitted, and the position is deleted rather than left open.** A crystal is a hermetic gas cavity by construction, and **this body is oil-filled, vacuum-degassed and pressure-balanced, with no gas anywhere**: the same rule that keeps a 1×9 optical module off this sonde; pressure pulls the frequency first and crushes the lid after |
| the core and the UART | **HSI, disciplined by the rung through the PLL** — the loop is written out above (*The rung disciplines the core*). `TIM5` gates the core against the rung, the correction goes into `FRACN`, and `HSITRIM` steps one code whenever `FRACN` nears the edge of its window. **The rung cannot clock the core directly** — `OSC_IN`'s external-clock input is specified from 4 MHz and 2¹⁹ is 0,524 MHz, and the PLL's own input floor is 2 MHz. **There is no fallback part**: a clock multiplier off the rung was weighed and dropped on power (`../core/WHY.md`) |
| PLL1 | source HSI, `HSIDIV` 1 · **M 8 → an 8 MHz reference** · N 33 + `FRACN` (nominal ratio 33,554432, so `FRACN` sits near 4542 and is not near zero) · P 2 → VCO 2²⁸, `P` 2²⁷. **The binary SYSCLK is made by the loop, not by an oscillator's value** — the HSI's nominal is 64,000000 MHz, 8,000000 behind `M`, and nothing about it is binary |
| SYSCLK, AHB, APB1/2, timer kernel | **2²⁷ = 134,217728 MHz** |
| **the grid** | **TIM2 on the segment's rung**, `ETR` on PA15 — **2¹⁹, and it is not a per-segment choice**: an Argus hands all four segments one number, its role's, and 2¹⁹ is what the mini tier's weakest medium carries (`../bifrost/HARDWARE.md`, `../galvani/README.md`) — divided to the sample rate by whole powers of two; nothing local is in the time path and there is no crystal to be in it |
| the rung gone | missing edges on `ETR` are the loss event: the loop freezes on its last `FRACN` code, the holdover table takes over, and the degraded flag is set until the rung returns |

### Analogue and reference

**Every filter inductor on this board is a π filter: 10 µF 1206 + 100 nF 0603 before it on the rail and
the same pair at the pins behind it** — the house rule (`../core/POWER.md`, *The filter parts*).

`VDDA` and `VREF+` are one node off the 3,3 V through one shielded 2,2 µH `SWPA252012S2R2MT`,
10 µF + 100 nF at the pins — the rail is the buck's, and the ADC on it reads `VSENSE` for the
holdover table, so the filter is the buck-noise inductor of `../core/POWER.md` and not a
ferrite; `VREF−` and `VSSA` to the ground plane at one point. The two `ID` inputs are read single-ended against
`VREF+`, and the 10 kΩ 1 % pull-up of each `ID` resistor hangs on `VREF+` itself, so the reading
is a ratio and the rail's tolerance drops out. The arrived 12 V and the input current are the power
body's `INA238`, over I2C1 — no divider on this board.

## Terminals and bodies

**Connectors: `MINI IN` · `PWR IN` · `12V`** (`../galvani/README.md`, *Connector names*).

**This is a unit end, and a Pascal never stands in the station enclosure.** Everything on the
plugged Galvani boards runs whenever the sonde does, held by resistors — no processor pin switches
anything on them. Both bodies are `BX2.54-2xNA` shrouded headers; the pin numbering is the
family's (`../galvani/README.md`, *The connectors*). The gauge's four leads run from the `Bar30` housing in the tube wall to pads on this board; the `TMP117` sits on the board against the wall. No cable, no connector.

**The 12 V — two two-pole `DGPS2.5R-5.0` blocks** for 2,5 mm², an input and a
tap, the same node: the unit power board's island output lands on the input. No 12 V rides either
body.

**The data body — 12 pins, 2×6.**

| pin | signal | on this board |
|---|---|---|
| 1 | `CLK/PPS` | into PA15 `TIM2_ETR`, the timer input — the segment's rung, 2¹⁹ |
| 2 | `GND` | ground |
| 3 | `TXD` | PB6 — `USART1_TX`, `TIM4_CH1` for the ranging instant |
| 4 | `RXD` | PB7 — `USART1_RX`, `TIM4_CH2`; the same net on PB3 `TIM2_CH2`, the frame-start capture |
| 5 | `ID` | PC0 `ADC12_INP10` — 10 kΩ 1 % to `VREF+` |
| 6 | `ID_RET` | not connected — a measuring unit never meets a crossed cable |
| 7 | `RXD_ECHO` | PD9 — `USART3_RX`, the echo check |
| 8 | `DE` | PE2 — GPIO, low from reset, up around the sonde's slot and the ranging return |
| 9 | `B_DIR` | **tied to ground** — this end listens on channel B, the rung arrives here; no processor pin |
| 10 | `LINE_EN` | 10 kΩ to 3,3 V — the line side runs whenever the sonde does; no processor pin |
| 11 | `SD` | PE4 — GPIO input, 100 kΩ to ground; unpopulated on copper, so it reads quiet there |
| 12 | `3,3 V` | the `LMR43610`'s 3,3 V — the communication board's whole supply |

**The power body — 8 pins, 2×4.**

| pin | signal | on this board |
|---|---|---|
| 1 | `ENABLE` | not connected — the unit power board pulls it to its own input and runs whenever the feed is there |
| 2 | `GND` | ground |
| 3 | `ID` | PC1 `ADC12_INP11` — 10 kΩ 1 % to `VREF+` |
| 4 | `SDA` | PB9 `I2C1_SDA` — 4,7 kΩ to 3,3 V |
| 5 | `A_SEL` | strapped to ground — one socket on this board, so its `INA238` answers at 0x40 |
| 6 | `SCL` | PB8 `I2C1_SCL` — 4,7 kΩ to 3,3 V |
| 7 | `ALERT` | PC8 — GPIO, EXTI, 100 kΩ to ground; high = alarm |
| 8 | `3,3 V` | the `LMR43610`'s 3,3 V — the power board's supply |

## Bench criteria

- The gauge's pressure reading against a reference at atmosphere, before potting: within the
  part's ±50 mbar at 0–6 bar.
- **The minutes band**: at constant temperature and pressure the burst mean drifts no more than
  **2 mbar in 30 minutes** — ten times under the ~20 mbar wave. The sheet carries no such figure,
  so this is the test that decides the part.
- The grid: with the segment's rung on `CLK_SEG`, the compare train on TIM2 lands the
  conversion starts at 2ⁿ-tick intervals; the payload's frame index advances once per Argus
  frame with no slip over an hour.
- `ID_D` and `ID_P` read the plugged boards' codes within ±0,025.

## Parts

| part | qty | where |
|---|---|---|
| `STM32H523VE`, LQFP100 | 1 | the processor — no crystal |
| 2× 1 µF 50 V 0805 + 2× 100 nF 0603 | 2 sets | `VCAP` |
| 100 nF 50 V | one a supply pin | every `VDD`, the gauge, the `TMP117` |
| 10 µF 50 V | 1 | the processor's bulk |
| 100 nF · 10 kΩ | 1 each | `NRST` · `BOOT0` to ground |
| `MS5837-30BA` (`MS583730BA01-50`) in a Blue Robotics `Bar30` housing | 1 | through the tube wall (`CONSTRUCTION.md`); its leads on I2C3 |
| `TMP117` | 1 | the wall, I2C3 |
| 4,7 kΩ | 2 | I2C3 |
| `LMR43610R3RPER` | 1 | the 3,3 V |
| 4,7 µH shielded, `I_SAT` ≥ 3,5 A | 1 | the buck's inductor |
| 28,0 k · 12,1 k · 22 pF C0G · 7,50 kΩ | 1 set | the divider, `C_FF`, `RT` for 2,08 MHz |
| 4,7 µF 50 V + 100 nF · 1 µF · 100 nF | 1 set | `VIN` · `VCC` · `BOOT` |
| 3× 10 µF 50 V 1206 + 100 nF | 1 set | the 3,3 V node |
| 100 kΩ | 1 | `PG` up, to PD14 |
| 5× 10 µF 50 V 1206 + 100 nF, ceramic | 1 set | the 12 V input |
| 2,2 µH `SWPA252012S2R2MT` + 2× (10 µF + 100 nF) | 1 set | `VDDA` / `VREF+`, π |
| 10 kΩ 1 % | 2 | the `ID` pull-ups to `VREF+` |
| 4,7 kΩ | 2 | I2C1, the power body |
| 100 kΩ | 2 | `SD` and `ALERT` to ground |
| 10 kΩ | 1 | `LINE_EN` to 3,3 V |
| 1 kΩ + LED | 1 | the status LED on PD4 |
| `DGPS2.5R-5.0`, two-pole | 2 | `12V` |
| `BX2.54-2x6NA` · `BX2.54-2x4NA` | 1 each | `MINI IN` · `PWR IN` |
| 6-pin header | 1 | SWD |

The tube, the gland, the fill, the sensor body's hole, the footing and the cable are
`../gauss/CONSTRUCTION.md`'s and `CONSTRUCTION.md`'s.

★ N.I.C. ★

# Gauss — the sensor board

> **Design-stage concept — nothing built.** The figures are verified against the parts' sheets
> and the parts before a board is made.

**Gauss is a magnetometer and its thermometers, nothing else**: the `RM3100` chipset, a wall
`TMP117`/`STS35`, an NTC between the coils, and an H523. It is a **mini-NOD on a NodBus mini
segment behind an Argus**, in every form it has — the stake, the well, the sea — and one board
serves them all.

## One sensor board; the port board is a Galvani

**There is no transceiver on this board, and no build variant.** The line side — transceiver,
isolation, supply and entry protection — is the **unit-end Galvani pair** plugged into its two
bodies and potted with it (`../galvani/README.md`). What crosses between the boards is logic level
(*The Galvani bodies*, below).

Float or isolated, short run or buried spur, is therefore not a choice this board makes: it is
which Galvani boards are plugged in. The sensor board never changes.

## The clock — no crystal, and the rung disciplines the core

**There is no crystal and no oscillator part on this board.** A crystal is a sealed gas cavity by
construction — the blank has to vibrate in one — and this body passes hydrostatic pressure straight
through the fill to the parts, potted on land and oil-filled in the sea alike; the tube wall
protects nothing. Pressure pulls a crystal's frequency first and crushes its lid after. One board
serves both builds, so the land build carries no crystal either.

**The sample grid needs no local clock.** The segment's rung — 2¹⁹ Hz, the one number an Argus
hands all four of its segments — arrives on the data body's `CLK/PPS` pin and lands on `TIM2`'s
external clock input, `ETR`. `TIM2` counts the rung directly, and a compare every 2ⁿ ticks is a
sample instant: 4096 ticks is 128 Hz, 8192 is 64 Hz. Between two rung edges lie 1,907 µs, and
nothing the core does in that time moves the edge; the grid is exact as long as the rung is there,
and what remains is the edge itself — capture, buffer and module delays, ns-class and fixed — which
the card's ranging takes out.

**The rung cannot clock the core directly.** The H523's external-clock input on `OSC_IN` is
specified from 4 MHz and 2¹⁹ is 0,524 MHz, eight times under it. Raising the rung is not open
either: past copper a mini segment runs on the 2 Mb/s glass board, where a 2¹⁹ square wave is
1,05 Mb/s of transitions and fits and a 2²² square wave would be four times over. So the core runs
on the HSI, the H523's internal RC oscillator — 64 MHz nominal, 63,7…64,3 MHz as delivered at
T_J 30 °C and drifting −2 / +1 % over the junction range (DS14540 Rev 3, Table 43) — and the
firmware disciplines it against the rung.

**Three things need the core's frequency right, and the grid is not one of them**: the UART's
baud, which binds first — a UART tolerates about 2 %, and a crystal is what every other board buys
for it; the ranging turnaround, where `TIM4` raises the return a set number of core ticks after the
capture; and the `DRDY` timestamp, where `TIM3` counts core ticks and the offset it converts to the
grid is core time.

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
  `FRACN` can pull, and a pod on a stake in air sees −40…+60 °C over a year, so a trim set once in
  spring would run `FRACN` onto its rail by winter. When `FRACN` passes a guard band a quarter of
  the window from either edge, the loop steps `HSITRIM` one code toward the centre and lets `FRACN`
  run back; the UART sees a 0,24 % step for a few frames, a tenth of its tolerance, and the grid
  sees nothing. In a potted pod under water the junction sits in a narrow band and the step may
  never fire; on land it fires a few times a year.
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
fault nothing else on the board would report. The wall thermometer is the slow cross-check; in a
potted pod with constant dissipation the die-to-wall difference is a constant.

**From cold.** The pod is powered, the die climbs from the water's temperature to its working
point over minutes, and the HSI moves with it. The loop tracks it on the one-frame gate and
lengthens to the one-second gate once the residual stops moving; the unit enrols as soon as the
baud is inside tolerance and waits for nothing else. `DRDY`'s timestamp is only as good as the
loop, so the frames before lock carry `status` 4 WARMUP.

## Block diagram — the sensor board (the Barrel pod carries this sensor board + the unit-end Galvani boards, plugged and potted)

```
 cable (the spur and its feed, 48 V or 300 V as the run asks — potted through on the sea build, a gland on land)
   │
   ├─ feed ─┐
   ├─ data ─┴─ the Galvani boards (supply, transceiver, protection) ── logic level ── H523
   │
   │ [Galvani port board │ buck │ MCU │— quiet gap —│ coils │ tip]
   │ cable end ──────────────────────────────────────────────────► tip end
   └─ every cm between the buck and the coils is 1/r³ of interference (*Placing the RM3100*)
```

## Rails — one buck, four inductor branches

**One `LMR43610R3RPER` to 3,3 V, and every consumer on its own branch through a shielded
inductor — no LDO.** The 12 V is the unit power board's island output on the board's own two
`DGPS2.5R-5.0` terminals; nothing above 12 V reaches this board.

```
 12 V on the terminals ─▶ LMR43610 ─▶ 3,3 V ─┬─ L1 ─▶ the H523 · the communication board · the power board
                                            ├─ L2 ─▶ VDDA / VREF+
                                            ├─ L3 ─▶ the RM3100's AVDD
                                            └─ L4 ─▶ the RM3100's DVDD
```

| position | part and value |
|---|---|
| 12 V input | **5× 10 µF 50 V 1206 + 100 nF, ceramic only** — a pressure-balanced body takes no hybrid polymer and no electrolytic (`../galvani/README.md`, *Pressure boards and land boards*); the pod draws tens of milliamperes and the unit power board's regulated 12 V has no transil clamp to reserve against |
| the buck | the house cell: `RT` 7,50 kΩ → 2,08 MHz, auto mode; 4,7 µH shielded, `I_SAT` ≥ 3,5 A; `C_IN` 4,7 µF 50 V + 100 nF; `C_OUT` 3× 10 µF 50 V 1206 + 100 nF; `VCC` 1 µF; `BOOT` 100 nF; `R_FBT` 28,0 kΩ / `R_FBB` 12,1 kΩ → 3,31 V; `C_FF` 22 pF C0G; `PGOOD` to PD14 |
| `L1`…`L4` | **`SWPA252012S2R2MT`** — 2,2 µH shielded, `DCR` 0,17 Ω, 2,5 × 2,0 × 1,2 mm |
| behind `L1` | 10 µF + 100 nF |
| behind `L2` | 10 µF + 100 nF, then `VDDA` and `VREF+` with 1 µF + 100 nF at the pins |
| behind `L3`, at the cable end | 10 µF + 100 nF |
| behind `L4`, at the cable end | 10 µF + 100 nF — **never more than behind `L3`** |
| at the MagI2C, on each of `AVDD` and `DVDD` | 10 µF + 100 nF, ceramic — no tantalum near the coils |

**No part of this is beside the coils.** The buck and all four inductors sit at the cable end of
the tube with the H523; the RM3100 is at the tip, the MagI2C within 10 cm of its coils, and the
rails reach it on wires the length of the tube. A magnetic part is excluded beside the coils, not
from the board.

**The RM3100 needs no LDO.** It counts oscillator cycles, and its sheet asks the supply for
2,0–3,6 V with at most **50 mV peak-to-peak of ripple** on `AVDD` or `DVDD`. 2,2 µH into 10 µF is
a ~34 kHz corner, ~70 dB down at the buck's 2,08 MHz, with a Q of ~2,8.

**`AVDD` and `DVDD` stay within the sheet's 0,1 V.** The coil drive peaks at no more than
3,3 V / (121 + 30 + 121 Ω) ≈ 12 mA on `AVDD`, 0,17 Ω × 12 mA = 2 mV; the average is 70–260 µA;
`DVDD` draws under 1 mA. The difference is ~2 mV.

**The sensor's input pins are rated to `DVDD` and not above**, so the SPI from the H523 must not
stand higher than the sensor's rail. Both are the same 3,3 V node behind an inductor of 0,17 Ω,
and the difference is fractions of a millivolt — which is why the branches are inductors and not
resistors.

**`DVDD` comes up first or with `AVDD`, never after**: both branches start from one node through
the same inductor, so they rise together on the buck's soft start, and the capacitance behind `L4`
is never larger than behind `L3`, so `DVDD` cannot lag.

**The NTC is on `L1`'s rail and is read against `VREF+` on `L2`'s** — the same node, no DC
between them, so the ratio cancels the rail. The wall thermometer draws microamps on its own I²C.

## Protection

**Nothing on the sensor board protects anything** — protection is the Galvani boards' potted in
beside it.

## The RM3100 — the chipset build and the parts around it

Built from the chipset, as on Quake: the **`13156` ASIC and three `13104` coils**, sequential
along the tube, axes orthogonal — the field is homogeneous over 20 cm, so position is free and
orientation is not.

- **`REXT` 33 kΩ thin-film 0,1 %** — the LR timing reference: its value and tempco enter the
  reading as a gain term.
- **6× 121 Ω thin-film coil series resistors**, PNI's reference circuit — a gain term that cancels
  to first order in the ± difference and is absorbed by the gain calibration and the temperature
  regression; the precision family right, a tighter grade not needed.
- **33 Ω in series on every SPI line.**
- **An NTC thermal-epoxied between the coils** — the chip's own temperature, what the drift
  regression needs — and **a `TMP117`/`STS35` on the tube wall**, fitted by site (*The sensor*,
  below).

## The processor — pins, timers, clock tree

*The record to draw the H523 from and to set CubeMX by. Package LQFP100, `STM32H523VE`; pin
numbers and alternate functions from DS14540 Rev 3, Table 13. Every pin below is either
assigned or listed free — nothing is left to guess.*

### Peripherals

| peripheral | instance | pins | job | mode |
|---|---|---|---|---|
| the mini link | **USART1** + **TIM4** | PB6 `TXD` · PB7 `RXD` | NodBus mini behind an Argus, type 1 | the pins switch USART → TIM4 for the ranging instant |
| the grid | **TIM2** (32-bit) | PA15 `CLK_SEG` on `ETR` · PB3 ← `RXD` on CH2 | the segment's rung is the counter's clock; Argus's frame start is the anchor | external clock mode 2; a compare every 2ⁿ ticks is the sample instant |
| echo receiver | **USART3**, RX only | PD9 `RXD_ECHO` | hears the board's own frames | |
| RM3100 | **SPI1** + `SSN` PA4 | PA5 · PA6 · PA7 | the magnetometer, chipset build | mode 0, ≤ 1 MHz; `DRDY` on PB5, captured on TIM3 |
| thermometers | **I2C2** · **ADC1** | PB10 `SCL_T` · PB12 `SDA_T` · PC2 `NTC` on `ADC1_INP12`, PC3 `NTC_EN` | the TMP117/STS35 on the wall; the NTC between the coils on a divider switched from a GPIO, ratiometric. **Its own controller** — a jammed power board must not hold the tempco's covariate down, and the bus takes four `TMP117` where a build wants more of them | 100 kHz; the divider on for the conversion only |
| the power body | **I2C1** | PB8 · PB9 | the unit power board's `INA238` — **`A_SEL` strapped to ground on this board**, so it is 0x40; one socket here and no processor pin reads the strap (`../galvani/README.md`) | 100 kHz, 4,7 kΩ to 3,3 V on both lines; `ALERT` on PC8, 100 kΩ to ground, high = alarm |
| ID reads | **ADC1** | PC0 · PC1 | one resistor per body, against 10 kΩ 1 % to `VREF+` | read once at bring-up |
| clock | **HSI, disciplined by the rung** | no pin — PH0 · PH1 free | there is no crystal on this board (*The rung disciplines the core*) | `TIM5` gates the core against the rung, `FRACN` steers the PLL |
| the holdover's covariates | **ADC1** internal channels · **DTS** | no pin — both are internal | the die's temperature by two routes: `VSENSE` against `VREFINT` in one sequence, and the DTS's own count in `DTS_DR`. Indexed as raw readings, never as a temperature (*The rung disciplines the core*) | the pair in one ADC sequence; the DTS is a register read, no timer |
| debug | **SWD** | PA13 · PA14 | | no JTAG, no trace |

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
| 12 | PH0 | — | | | free — no crystal on this board |
| 13 | PH1 | — | | | free |
| 14 | NRST | reset | |  | 100 nF, no pull beyond the internal |
| 15 | PC0 | `ID_D` | `ADC12_INP10` | in | the data body's `ID` — 10 kΩ 1 % to `VREF+` |
| 16 | PC1 | `ID_P` | `ADC12_INP11` | in | the power body's `ID` — 10 kΩ 1 % to `VREF+` |
| 17 | PC2 | `NTC` | `ADC1_INP12` | in | the coil NTC's divider midpoint, ratiometric against `VREF+` |
| 18 | PC3 | `NTC_EN` | GPIO | out | the divider's top, high for the conversion only |
| 19 | VSSA | | | | |
| 20 | VREF− | | | | |
| 21 | VREF+ | `L2`'s branch — the ADC reference | | | |
| 22 | VDDA | `L2`'s branch | | | |
| 23 | PA0 | — | | | free (ADC) |
| 24 | PA1 | — | | | free (ADC) |
| 25 | PA2 | — | | | free (ADC) |
| 26 | PA3 | — | | | free (ADC) |
| 27 | VSS | | | | |
| 28 | VDD | 3,3 V | | | |
| 29 | PA4 | `SSN` | GPIO | out | RM3100 `SSN` |
| 30 | PA5 | `SCK` | `SPI1_SCK` | out | RM3100 `SCLK` — 33 Ω in series |
| 31 | PA6 | `MISO` | `SPI1_MISO` | in | RM3100 `MISO` |
| 32 | PA7 | `MOSI` | `SPI1_MOSI` | out | RM3100 `MOSI` |
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
| 47 | PB10 | `SCL_T` | `I2C2_SCL` | i/o | the wall thermometer — its own controller, off the power body's bus |
| 48 | VCAP | 2× 1 µF 50 V 0805 + 2× 100 nF 50 V 0603 — 2,2 µF, the sheet's 2,2 µF ±20 % | | | |
| 49 | VSS | | | | |
| 50 | VDD | 3,3 V | | | |
| 51 | PB12 | `SDA_T` | `I2C2_SDA` | i/o | |
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
| 84 | PD3 | — | | | free — the coil thermometer is an NTC on an ADC pin (`WHY.md`) |
| 85 | PD4 | `LED` | GPIO | out | the status LED, fitted — 1 kΩ from the 3,3 V, one 50 ms blink a minute while healthy, two on a fault (`../core/HARDWARE.md`) |
| 86 | PD5 | — | | | free |
| 87 | PD6 | — | | | free |
| 88 | PD7 | — | | | free |
| 89 | PB3 | `RXD` | `TIM2_CH2` capture | in | the same net as PB7 — the timebase captures the start edge of Argus's frame |
| 90 | PB4 | — | | | free |
| 91 | PB5 | `DRDY` | `TIM3_CH2` capture, EXTI5 | in | RM3100 `DRDY` — captured on the timer so the conversion end is a time, not an interrupt latency |
| 92 | PB6 | `TXD` | `USART1_TX` / `TIM4_CH1` | out | the NodBus mini link, to the data body |
| 93 | PB7 | `RXD` | `USART1_RX` / `TIM4_CH2` | in | the pins switch USART → TIM4 for the ranging instant |
| 94 | BOOT0 | 10 kΩ to ground | |  | |
| 95 | PB8 | `SCL` | `I2C1_SCL` | i/o | the power body's `INA238`, alone on this bus — 4,7 kΩ to 3,3 V |
| 96 | PB9 | `SDA` | `I2C1_SDA` | i/o | 4,7 kΩ to 3,3 V |
| 97 | PE0 | — | | | free |
| 98 | VCAP | 2× 1 µF 50 V 0805 + 2× 100 nF 50 V 0603 — 2,2 µF, the sheet's 2,2 µF ±20 % | | | |
| 99 | VSS | | | | |
| 100 | VDD | 3,3 V | | | |

**25 GPIO used, 55 free** (PA0–PA3 · PA8–PA12 · PB0–PB2 · PB4 · PB13–PB15 · PC4–PC7 · PC9–PC15 · PD0–PD3 · PD5–PD8 · PD10–PD13 · PD15 · PE0 · PE3 · PE5–PE15 · PH0 · PH1).

### Timers

| timer | bits | kernel | pins | job |
|---|---|---|---|---|
| **TIM4** | 16 | 2²⁷ | CH1 → PB6, CH2 ← PB7 | the ranging turnaround: one-pulse, triggered by the capture on CH2, the return raised on CH1 after `CCR` ticks; PWM-input mode on CH2 measures the incoming pulse's width |
| **TIM2** | 32 | the rung on `ETR` (PA15), external clock mode 2 | CH2 capture ← PB3; CH3/CH4 compares, no pin | **the grid**: the counter runs on the segment's rung and a compare every 2ⁿ ticks is the sample instant — at 2¹⁹, 4096 ticks is 128 Hz; the capture of Argus's frame start anchors the slot |
| **TIM3** | 16 | 2²⁷ | CH2 capture ← PB5 | the RM3100's `DRDY` as a time on the grid — the conversion's end, not the interrupt's |
| **TIM5** | 32 | 2²⁷ | no pin — latched by `TIM2`'s frame compare over the internal trigger | **the discipline loop's gate**: the core counted against the rung, 1 048 576 counts a frame at 0,95 ppm each, or a second's worth at 0,0075 ppm (*The rung disciplines the core*) |
| TIM1 · TIM8 · TIM12 · TIM15 · LPTIM1 · LPTIM2 | | | | free |
| TIM6 · TIM7 | 16 | | | free — the basic timers |

The RM3100 is triggered by the compare on TIM2 and its `DRDY` is captured on TIM3, so a sample is
two numbers on one grid: when it was asked for and when it was done.

### Clock tree

| | |
|---|---|
| HSE | **not fitted, and the position is deleted rather than left open.** A crystal is a hermetic gas cavity by construction, and this body passes hydrostatic pressure straight through the fill to the parts in both builds, potted and oil-filled alike; pressure pulls the frequency first and crushes the lid after. **The rung cannot take its place either** — `OSC_IN`'s external-clock input is specified from 4 MHz and 2¹⁹ is 0,524 MHz |
| the core and the UART | **HSI, disciplined by the rung through the PLL** — the loop is written out above (*The rung disciplines the core*). `TIM5` gates the core against the rung, the correction goes into `FRACN`, and `HSITRIM` steps one code whenever `FRACN` nears the edge of its window. **There is no fallback part**: a clock multiplier off the rung was weighed and dropped on power (`../core/WHY.md`) |
| PLL1 | source HSI, `HSIDIV` 1 · **M 8 → an 8 MHz reference** · N 33 + `FRACN` (nominal ratio 33,554432, so `FRACN` sits near 4542 and is not near zero) · P 2 → VCO 2²⁸, `P` 2²⁷. **The binary SYSCLK is made by the loop, not by an oscillator's value** — the HSI's nominal is 64,000000 MHz, 8,000000 behind `M`, and nothing about it is binary |
| SYSCLK, AHB, APB1/2, timer kernel | **2²⁷ = 134,217728 MHz** |
| **the grid** | **TIM2 on the segment's rung**, `ETR` on PA15 — **2¹⁹, and it is not a per-segment choice**: an Argus hands all four segments one number, its role's, and 2¹⁹ is what the mini tier's weakest medium carries (`../bifrost/HARDWARE.md`, `../galvani/README.md`) — divided to the sample rate by whole powers of two; nothing local is in the time path and there is no crystal to be in it |
| the rung gone | missing edges on `ETR` are the loss event: the loop freezes on its last `FRACN` code, the holdover table takes over, and the degraded flag is set until the rung returns |

### Analogue and reference

**Every filter inductor on this board is a π filter: 10 µF 1206 + 100 nF 0603 before it on the rail and
the same pair at the pins behind it** — the house rule (`../core/POWER.md`, *The filter parts*).

`VDDA` and `VREF+` on `L2`'s branch, 1 µF + 100 nF at the pins; `VREF−`
and `VSSA` to the ground plane at one point. The two `ID` inputs are read single-ended against
`VREF+`, and the 10 kΩ 1 % pull-up of each `ID` resistor hangs on `VREF+` itself, so the reading
is a ratio and the rail's tolerance drops out. The arrived 12 V and the input current are the power
body's `INA238`, over I2C1 — no divider on this board.
The RM3100's `REXT` is not on this list: it sets the sensor's own LR clock and the processor
never sees it.

## The Galvani bodies

**Connectors: `MINI IN` · `PWR IN` · `12V`** (`../galvani/README.md`, *Connector names*).

**This is a unit end, and a Gauss never stands in the station enclosure.** Everything on the
plugged Galvani boards runs whenever the pod does, held by resistors — no processor pin switches
anything on them. Both bodies are `BX2.54-2xNA` shrouded headers; the pin numbering is the
family's (`../galvani/README.md`, *The connectors*). The feed's voltage is the run's choice —
the unit power board for 48 V or for 300 V — and nothing on this board changes with it.

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
| 8 | `DE` | PE2 — GPIO, low from reset, up around the pod's slot and the ranging return |
| 9 | `B_DIR` | **tied to ground** — this end listens on channel B; no processor pin |
| 10 | `LINE_EN` | 10 kΩ to 3,3 V — the line side runs whenever the pod does; no processor pin |
| 11 | `SD` | PE4 — GPIO input, 100 kΩ to ground; unpopulated on copper, so it reads quiet there |
| 12 | `3,3 V` | the board's 3,3 V — the buck whose `PGOOD` is PD14 |

**The power body — 8 pins, 2×4.**

| pin | signal | on this board |
|---|---|---|
| 1 | `ENABLE` | not connected — the unit power board pulls it to its own input and runs whenever the feed is there |
| 2 | `GND` | ground |
| 3 | `ID` | PC1 `ADC12_INP11` — 10 kΩ 1 % to `VREF+` |
| 4 | `SDA` | PB9 `I2C1_SDA` — 4,7 kΩ to 3,3 V |
| 5 | `A_SEL` | strapped to ground — the `INA238` answers at 0x40 |
| 6 | `SCL` | PB8 `I2C1_SCL` — 4,7 kΩ to 3,3 V |
| 7 | `ALERT` | PC8 — GPIO, EXTI, 100 kΩ to ground; high = alarm |
| 8 | `3,3 V` | the board's 3,3 V — the power board's supply |

## Bench criteria

1. **The buck-to-coil distance is the tube's length**: the buck and the Galvani boards at the cable
   end, the coils at the tip.
2. **The coil axes are orthogonal on the jig** before potting, and the finished pod is
   tumble-calibrated (`CONSTRUCTION.md`, *Calibration without rotating the site*).
3. **At first power-up the island ground and the bus ground are open** — the power board's barrier
   intact.

## The sensor — the RM3100, and the rules around it

**A three-axis magneto-inductive magnetometer**: very low noise and near-zero offset drift, where a
Hall part has neither.

| parameter | value |
|---|---|
| axes | 3 |
| resolution | ~13 nT at the fitted cycle count — the count sets it |
| field range | ±800 µT |
| interface | SPI, mode 0, ≤ 1 MHz |
| supply | 2,0–3,6 V, at most 50 mV peak-to-peak of ripple on `AVDD` and `DVDD` |
| sampling | `POLL` at 32 Hz, every 4th frame; `DRDY` captured on `TIM3` and the sample interpolated onto the grid (`FIRMWARE.md`) |

**How it measures.** Each coil is a solenoid on a soft-magnetic alloy core and is the inductance of
a relaxation oscillator; the external field biases the core's operating point, so the oscillation
count differs between the two drive polarities, and the difference of the two counts is the field.
It is a counting measurement — no amplifier, no ADC — so the cycle count sets the resolution and
there is no analogue offset drift, and the first-order drive and temperature effects cancel in the
± difference. The core is a permalloy-class alloy and not power ferrite, whose permeability walks
tens of percent over 20 °C.

**Distance is the layout rule, and the pod is built for it.** The chip resolves ~13 nT, and a
switching current loop a centimetre away puts hundreds of nT to µT across it. A current loop's near
field falls as 1/r³, so the buck, the Galvani boards and all four inductors sit at the cable end of
the tube and the coils at the tip, ~80 cm apart on a metre tube; the tube is the thermally stable,
rigid, non-ferrous vault the sensor wants anyway. FR4 and copper shield nothing at geomagnetic
frequencies — only distance and staying out of the switcher's return loop count. Beside the coils
there is nothing but the ceramic capacitors at the MagI2C and the NTC. On solid ground one quiet
location is enough — a crustal or magma change is so large a mass that its field barely differs
over 100 m; the sea is the exception (`ARRAY.md`).

**What hurts and what does not.** A static neighbour — a trace carrying DC, a nickel termination,
tinned copper — makes a constant offset that the tumble calibration removes, which is why the
chipset meets its figure on a crowded board. A **varying current** near the coils makes a varying
field that nothing removes, and **a ferrite or a steel part** at the tip distorts the field and
drifts with temperature. So: a copper and component keep-out around the coils, no dynamic-current
trace under or beside them, and no ferrous hardware — steel screws, nickel-plated parts, inductor
cores — anywhere near the tip.

**Two thermometers at the tip, two jobs.** The RM3100 has no temperature sensor and no
compensation of its own; its offset and sensitivity move with the core's temperature and are
removed by regression outside the chip.

- **The NTC** — 10 kΩ 1 %, on a divider to PC2 `ADC1_INP12`, its top switched from PC3 — is
  thermal-epoxied between the coils and reads the sensor body's own temperature, which the
  regression needs. It is read ratiometrically against `VREF+`, with the divider on for the
  microseconds of the conversion only, so no current stands in the coils' field between reads; a
  1-Wire part's 1,5 mA conversion pulse would be tens of nT at that distance (`WHY.md`). 0,5 °C
  class is enough for the regression.
- **The `TMP117`/`STS35`** on the tube wall — ±0,1 °C absolute, 8 mK resolution, ~3,5 µA, on its
  own I2C2 — is the ground or water thermometer, the mK trend at depth. It is fitted where the pod
  goes into a well, a borehole or the sea, and left empty on a garden stake, where the soil
  temperature is Palatine's and the NTC alone serves the regression.

Both ride the `REPORT` frame once a minute; the payload does not change.

**The compensation is a regression, and the archive keeps the raw.** Pairs of raw field and coil
temperature are logged over quiet periods, a per-axis offset term `a + b·T` is fitted — a gain term
only if the data demands it — and the pod applies it to the live product. The archive stores the
raw field and the raw temperature, never the corrected value, so the correction stays re-derivable
with a better model.

**The measurement window is the anti-alias filter upward; distance is the filter downward.** The
RM3100 has nothing ahead of it, and its millisecond-class integration folds everything above it as
a sinc, so a converter's switching fundamental at hundreds of kHz is tens of dB down. What the
window does not touch is everything slow — the pod's own 128 Hz draw and its harmonics, transmit
bursts, a converter wandering in the geomagnetic band — which is why the budget rests on distance
and not on any frequency, the family's rule for a converter beside a measuring board
(`../galvani/README.md`). What distance leaves sits above the geomagnetic band and is removed
downstream: the archive holds the raw stream, an FFT with the station running against quiet names
the lines — the frame rate and its harmonics, mains, the radio's cadence — and a long low-pass
decimates below them. Averaging kills the white noise; distance kills the drift and the spurs
that averaging cannot touch. **The pod ships the deviation and does no analysis** (`FIRMWARE.md`
§1).

## Parts

| part | qty | where |
|---|---|---|
| `STM32H523VE`, LQFP100 | 1 | the processor — no crystal |
| 2× 1 µF 50 V 0805 + 2× 100 nF 0603 | 2 sets | `VCAP` |
| 100 nF 50 V | one a supply pin | every `VDD` |
| 100 nF · 10 kΩ | 1 each | `NRST` · `BOOT0` to ground |
| `13156` + 3× `13104` | 1 set | the RM3100 chipset, the coils at the tip |
| 33 kΩ thin-film 0,1 % | 1 | `REXT` |
| 121 Ω thin-film | 6 | the coil series resistors |
| 33 Ω | 4 | `SCK` · `MISO` · `MOSI` · `SSN` |
| 10 µF + 100 nF, ceramic | 2 sets | the MagI2C's `AVDD` and `DVDD` |
| NTC 10 kΩ 1 % + 10 kΩ 1 % | 1 | between the coils; the divider switched from PC3 |
| `TMP117` or `STS35` + 2× 4,7 kΩ | 1 | the wall, I2C2, by site |
| `LMR43610R3RPER` | 1 | the 3,3 V |
| 4,7 µH shielded, `I_SAT` ≥ 3,5 A | 1 | the buck's inductor |
| 28,0 k · 12,1 k · 22 pF C0G · 7,50 kΩ | 1 set | the divider, `C_FF`, `RT` for 2,08 MHz |
| 4,7 µF 50 V + 100 nF · 1 µF · 100 nF | 1 set | `VIN` · `VCC` · `BOOT` |
| 3× 10 µF 50 V 1206 + 100 nF | 1 set | the 3,3 V node |
| 100 kΩ | 1 | `PG` up, to PD14 |
| 5× 10 µF 50 V 1206 + 100 nF, ceramic | 1 set | the 12 V input |
| 2,2 µH `SWPA252012S2R2MT` | 4 | L1 · L2 · L3 · L4 |
| 10 µF + 100 nF | 8 sets | before and behind each inductor, π |
| 1 µF + 100 nF | 1 set | `VDDA` / `VREF+` at the pins |
| 10 kΩ 1 % | 2 | the `ID` pull-ups to `VREF+` |
| 4,7 kΩ | 2 | I2C1, the power body |
| 100 kΩ | 2 | `SD` and `ALERT` to ground |
| 10 kΩ | 1 | `LINE_EN` to 3,3 V |
| 1 kΩ + LED | 1 | the status LED on PD4 |
| `DGPS2.5R-5.0`, two-pole | 2 | `12V` |
| `BX2.54-2x6NA` · `BX2.54-2x4NA` | 1 each | `MINI IN` · `PWR IN` |
| 6-pin header | 1 | SWD |

The tube, the gland, the fill and the mount are `CONSTRUCTION.md`'s.

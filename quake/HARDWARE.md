★ N.I.C. ★

# Quake — hardware

> **Design-stage concept.** The figures are verified against the parts' sheets and the parts
> before the board is made.

## The board

**One board, whatever is fitted on it.** An `STM32H523VE` in LQFP100, three seismic sensors on
three SPIs, the magnetometer and the wall thermometer by population, the two Galvani bodies and
the 12 V terminals. An unfitted sensor leaves its footprint and its passives empty and
its `CS` idle; the firmware finds what is there (`FIRMWARE.md` §2).

- **No line part on this board.** Data and clock leave on the data body as logic; the
  transceiver, the barrier, the protection and the feed are the plugged communication and power
  boards'. **A Quake never stands in the enclosure** — it is the unit end of its own spur,
  point-to-point, and nothing on this board changes with the run.
- **Crystal-less.** HSE is the data body's `CLK`, 2²², in bypass; a NOD carries no crystal.
- **No fuse and no protection here** — the power and communication boards carry it.
- **Connectors: `NB IN` · `PWR IN` · `12V`** (`../galvani/README.md`, *Connector names*), and
  SWD on a 6-pin header.

**Every sensor in one orientation.** The PCB places every sensor on the same measurement axes —
by the datasheets' axis drawings, not by the packages — so the per-sensor mounting matrix is the
identity and one levelling matrix serves all of them. The gyro makes it necessary: at rest it
reads zero and has no gravity to be levelled against, so the layout is its calibration. The
residual is placement tolerance, ~1°, under the ADXL355's own 1 % cross-axis figure. The noise
separation of the layout comes first; the common orientation is kept within it.

### Two hookup rules

1. **One buck for every chip.** Each sensor's `VDD` sits on `L_ANA` and its `VDDIO` / `DVIO` on `L_IO`,
   two branches of one node.
   The "IO must not exceed core", "≤ 100 mV apart" and power-sequencing clauses of the MEMS sheets
   all trace to ESD diodes between the two domains; two branches off one buck satisfy all of them
   by construction — they differ only by millivolts across their inductors, and `L_IO` carries no
   more capacitance than `L_ANA`, so an interface never rises before its supply.
2. **33 Ω in series on every SPI line**, as everywhere in the station — it damps ringing.

### The sensors' own pins

- **ADXL355**: `VDDIO` (5) powered, 3,3 V + 100 nF — never floating; `VSSIO` (6) and `VSS` (9) to
  ground; **`V1P8ANA` (10) and `V1P8DIG` (8) 1 µF to ground and nothing else** — internal LDO
  outputs, never loaded; `RESERVED` (7) to ground or open; pin 2 used as `SCLK` selects SPI.
- **SCL3300**: `AVSS`, `DVSS`, `EMC_GND` to ground; **`A_EXTC` (2) and `D_EXTC` (10) a capacitor to
  ground only**; `DVIO` (9) the sensor's 3,3 V; `RESERVED` (3) per the sheet.

## The sensors

### Precision accelerometer — ADXL355 (Analog Devices), SPI1

**The seismic channel.** Clocked from the node's timebase on `EXT_CLK` and phased by `SYNC`, so
its samples sit on the grid (`FIRMWARE.md` §5).

| parameter | value |
|---|---|
| noise density | 25 µg/√Hz |
| range | ±2 / ±4 / ±8 g — **±2 g** in service |
| resolution | 20 bits; 3,81 µg a step at ±2 g |
| package | LCC-14, 6,0 × 6,0 × 2,1 mm, hermetic |
| cross-axis | 1 % |

The hermetic LCC is what holds its zero over years in the ground. **±2 g keeps it noise-limited**:
3,81 µg a step against 25 µg/√Hz; at ±8 g the step is 15,3 µg and eats the margin. Range is the
other accelerometer's job.

### 6-axis IMU — ICM-42688-P (TDK InvenSense), SPI2

**The clip-free accelerometer and the rotation channel.** Its `CLKIN` is a true external clock:
the ODR is derived from the network timebase, not disciplined toward it.

| parameter | value |
|---|---|
| gyro noise | 2,8 mdps/√Hz |
| gyro range | ±15,625 to ±2000 dps — **±15,625 dps** in service |
| gyro drift | ±5 mdps/°C |
| accelerometer noise | 65–70 µg/√Hz |
| accelerometer range | ±2 / ±4 / ±8 / ±16 g — **±8 g** in service |
| external clock | `CLKIN` on pin 9, 31–50 kHz |
| interface | SPI to 24 MHz |
| FIFO | 2 KB |
| supply | 1,71–3,6 V, 0,88 mA (6-axis, low-noise) |
| package | LGA-14, 2,5 × 3,0 × 0,91 mm |

**The two accelerometers run together at different ranges and the head stitches them**: the
ADXL355 while in range, the ICM for the peaks the ADXL355 clipped — at those amplitudes the ICM's
coarser step is a negligible fraction of the signal. **±8 g is twice the strongest ground
acceleration on the instrumental record** (≈ 4,1 g, Iwate-Miyagi 2008); ±16 g doubles it for
nothing worth measuring, so it is a deployment setting. **The gyro takes its most sensitive range**
— seismic rotation is tiny. What would saturate these ranges would already have destroyed the
building, rock or station.

**Without an ADXL355 the ICM's accelerometer is the seismic and levelling source.**

### Inclinometer — SCL3300-D01 (Murata), SPI3

**The tilt and long-term drift reference.** A purpose-built 3-axis inclinometer with its stability
over temperature and lifetime as the primary specification — where a general accelerometer's
offset drift corrupts an angle over time.

| parameter | value |
|---|---|
| modes | 1: ±1,2 g (±90°) · 2: ±2,4 g (±90°) · 3 and 4: ±10°, the fine ones |
| noise density | 0,6 mg/√Hz (Mode 1, 10 Hz bandwidth) |
| long-term stability | < 0,05° over life |
| output | SPI, 32-bit pipelined frames, CRC-8 — acceleration, angle, temperature |
| supply | 3,0–3,6 V |
| temperature | −40 to +85 °C |
| package | LGA-12, 8,6 × 7,6 × 3,3 mm — a footprint of its own |

**It has no clock input, so it cannot be the seismic channel** — it free-runs on its own
oscillator and is read slowly. **Mode 4 or Mode 1, chosen once at calibration** from the resting
tilt: within 10° of level Mode 4, lowest noise, for drift and subsidence; beyond it Mode 1, for a
deliberately tilted install. During a large event it saturates and its values are not used.

### Magnetometer — RM3100, SPI3 by population

**Built from the chipset**: the 13156 ASIC and three 13104 coils, **`REXT` 33 kΩ thin-film 0,1 %**
— the timing reference, the oscillator budget's one precision resistor — and **6× 121 Ω
thin-film** in series with the coils, a gain term that cancels to first order in the ± polarity
difference; the rest is gain calibration and temperature regression. On SPI3 with its own `CS`,
`DRDY` on PD10. Its `AVDD` and `DVDD` are two branches of the board's 3,3 V, their inductors at the
buck and no magnetic part beside the coils (*The supply*).

**An NTC is epoxied between the coils wherever the RM3100 is fitted** — the drift regression's
thermometer travels with the sensor, on a divider switched from `NTC_EN` to an ADC pin; no 1-Wire
part in the coils' field. The layout rules — the far corner from the switcher, a keep-out,
non-ferrous hardware nearby — and the whole measurement, calibration and temperature record are
Gauss's, the same sensor in a sonde of its own (`../gauss/HARDWARE.md`).

### Wall thermometer — TMP117 or STS35, by population

On I3C1 in I²C legacy, PD12/PD13 — the TMP117 at 0x48, the STS35 at 0x4A — coupled to the tube
wall. Compensation never needs it; it is fitted where the ground temperature at depth is wanted
(`CONSTRUCTION.md`, *Depth*).

## The supply

**12 V on the board's two `DGPS2.5R-5.0` terminals, an input and a tap, the same node** — the unit
power board's island output. Nothing above 12 V reaches this board, and no 12 V rides a body.

**One buck, one rail: a `TPS629206` at 3,3 V, in forced PWM, split into branches by inductors.**
The MEMS carry their own internal regulators — the ADXL355's `V1P8ANA`/`V1P8DIG`, the SCL3300's
`A_EXTC`/`D_EXTC` — so no sensor takes an LDO: what each needs is a supply free of the buck's
ripple, and a 2,2 µH into 10 µF is ~76 dB down at the buck's frequency, far below what an LDO
rejects there.

| | |
|---|---|
| the buck | **`TPS629206`**, 600 mA, SOT-5X3, 3–17 V in: its 12 V is the unit power board's regulated output, whose `5.0SMDJ12A` breaks down at 13,3–14,7 V |
| the divider | `R1` 619 kΩ / `R2` 137 kΩ, 1 % → 3,311 V against the 0,6 V reference |
| `MODE/S-CONF` | **9,31 kΩ 1 % to ground** — external feedback, **forced PWM, 2,5 MHz**, no output discharge (the sheet's Smart-CONFIG setting 4; ~2,7 MHz at 12 V in) — the sensors listen, and forced PWM keeps a fixed frequency at any load. The on-time at 14,7 V in is 90 ns against the part's 40 ns minimum |
| the inductor | 2,2 µH shielded, `XGL3530-222` |
| around it | `C_OUT` 3× 10 µF 1206 + 100 nF — the node every branch draws its steps from; `VCC` 1 µF; `BOOT` 100 nF; `PG` to PD14, 100 kΩ to the 3,3 V |

**The branches**, each the station's shielded 2,2 µH `SWPA252012S2R2MT` from the buck's node into
10 µF + 100 nF, with 100 nF more at every pin:

| branch | what sits on it |
|---|---|
| the node itself | the data body's and the power body's `3,3 V` — the communication board and the power board's telemetry, the board's largest draw |
| `L_MCU` | the H523's `VDD` pins, the pull-ups |
| `L_VDDA` | `VDDA` and `VREF+` |
| `L_ANA` | the sensors' supplies — the ADXL355's `VSUPPLY`, the ICM-42688-P's `VDD`, the SCL3300's `VDD` |
| `L_IO` | the sensors' interfaces — the ADXL355's and the ICM's `VDDIO`, the SCL3300's `DVIO` — and the TMP117 |
| `L_RM_A` | the RM3100's `AVDD` |
| `L_RM_D` | the RM3100's `DVDD` — never more capacitance behind it than behind `L_RM_A` |

**The interface is on its own branch so the SPI's edges never reach a sensor's supply.** The
switching current of the pins during a transfer stays on `L_IO`; `L_ANA` carries only the sensors'
steady milliamps, which their own regulators reject.

**The RM3100 is fed as on Gauss.** Its datasheet asks 2,0–3,6 V on `AVDD` and `DVDD` with at most
50 mV peak to peak of ripple and the two within 0,1 V; the coil drive peaks at ~12 mA on `AVDD`,
2 mV across 0,17 Ω. **Both inductors stand at the buck, never beside the coils**; beside the
chipset only 10 µF + 100 nF ceramic on each supply. `DVDD` rises with `AVDD` and never after,
because both start from one node through one inductor type and `L_RM_D` carries no more
capacitance.

**The draw on the 3,3 V**, typical and at the ceiling:

| | typical | ceiling |
|---|---|---|
| the H523 at 2²⁷, VOS2, from flash with the cache on — the sheet gives 13 mA with every peripheral off and 29 mA with every one on, 39 mA at 85 °C | ~20 mA | 40 mA |
| the sensors — ADXL355 0,2 · ICM 0,88 · SCL3300 ~1,2 · RM3100 ~0,5 at 96 axis reads a second, 100 cycle counts · TMP117 µA | ~3 mA | ~4 mA |
| the continuous lines — `CLK` 2²² in, `EXT_CLK` 2²⁰ out, the two `ID` dividers | ~1 mA | ~1 mA |
| the power board's telemetry — `INA238` and `ISO1642` | ~3 mA | ~3 mA |
| the communication board at a far end — copper · glass | 18 · ~55 mA | ~80 · 125 mA |
| **the board** — copper · glass | **~45 · ~85 mA** | **~130 · ~175 mA** |

**8–14 % of the `TPS629206` typical and 22–29 % at the ceiling**, inside the station's 40–50 / 70 %
rule; ~0,2–0,35 W off the 12 V.

**No three-terminal buck module**: its inductor and its light-load behaviour are the vendor's, and
those are what the station's supplies control (`../galvani/README.md`).

**The 12 V input bulk is the house input — a 47 µF 50 V hybrid polymer (105 °C, 10 000 h) + 2×
10 µF 50 V 1206 + 100 nF**, 50 V like every low-rail part in the station (`../core/POWER.md`).

**Capacitors**: NP0/C0G for every small capacitor in an analogue or signal path; X7R is enough
elsewhere — the node sits at a stable 10–12 °C in the ground — rated at more than twice its
working voltage.

## The processor — pins, timers, clock tree

*The record to draw the H523 from and to set CubeMX by. Package LQFP100, `STM32H523VE`; pin
numbers and alternate functions from DS14540 Rev 3, Table 13. Every pin below is either
assigned or listed free — nothing is left to guess.*

### Peripherals

| peripheral | instance | pins | job | mode |
|---|---|---|---|---|
| the NodBus link | **USART1** + **TIM4** | PB6 `TXD` · PB7 `RXD` | the spur to its card, 8N1 at the rung `BUSCFG` hands down — 2²⁰ for a lone unit | the pins switch USART → TIM4 for the ranging instant |
| the frame anchor | **TIM2** (32-bit) | PB3 ← `RXD` on CH2 | the card's frame start on the grid | never reset, read as differences |
| echo receiver | **USART3**, RX only | PD9 `RXD_ECHO` | hears the board's own frames | |
| ADXL355 | **SPI1** + `CS` PA4 | PA5 · PA6 · PA7 | the precision accelerometer | mode 0, ≤ 10 MHz; `INT1` on PB0 |
| ADXL355 clocking | **TIM1** CH1 · **TIM3** CH1 | PE9 `EXT_CLK` · PB4 `SYNC` | full external-clock mode (`EXT_SYNC` 01, `EXT_CLK` 1): 2²⁰ into `INT2`, a 128 Hz pulse into `DRDY` | PWM out |
| ICM-42688-P | **SPI2** + `CS` PB12 | PB13 · PB14 · PB15 | the 6-axis IMU | mode 0, ≤ 24 MHz; `INT1` on PB2 |
| ICM clocking | **TIM15** CH1 | PE5 `CLKIN` | 41,94304 kHz into pin 9 | PWM out, `ARR` 3199 |
| SCL3300 · RM3100 | **SPI3** + `CS` PD0 · PD1 | PC10 · PC11 · PC12 | the inclinometer, and the magnetometer option on the same bus | mode 0; the RM3100's `DRDY` on PD10 |
| thermometers | **I3C1**, in I²C legacy · **ADC1** | PD12 `SCL_T` · PD13 `SDA_T` · PC2 `NTC` on `ADC1_INP12`, PC3 `NTC_EN` | the TMP117/STS35 on the wall; the NTC between the coils on a divider switched from a GPIO, ratiometric. **Its own controller** — a jammed power board must not hold the tempco's covariate down. **Not I2C2**: its only `SCL` pad is PB10 and both `SDA` pads are taken — PB12 by the ICM's `CS`, PB3 by the `RXD` capture — so the second controller is I3C1, which is I²C-capable and has PD12/PD13 to itself | 100 kHz; the divider on for the conversion only |
| the power body | **I2C1** | PB8 · PB9 | the unit power board's `INA238` — **`A_SEL` strapped to ground on this board**, so it is 0x40; one socket here and no processor pin reads the strap (`../galvani/README.md`) | 100 kHz, 4,7 kΩ to 3,3 V on both lines; `ALERT` on PC8, 100 kΩ to ground, high = alarm |
| ID reads | **ADC1** | PC0 · PC1 | one resistor per body, against 10 kΩ 1 % to `VREF+` | read once at bring-up |
| clock in | **HSE bypass** | PH0 | the data body's `CLK`, 2²² | PH1 unconnected |
| debug | **SWD** | PA13 · PA14 | | no JTAG, no trace |

Three sensors on three SPIs so no transfer waits on another; the three data-ready lines are on
three distinct EXTI numbers (0, 2, 10), which is what the bring-up check asks for. Three sensor
clocks on three timers, one per frequency.

### Pins

| pin | port | signal on the board | AF / function | dir | note |
|---|---|---|---|---|---|
| 1 | PE2 | `DE` | GPIO | out | the data body's driver enable |
| 2 | PE3 | — | | | free |
| 3 | PE4 | `SD` | GPIO | in | the data body's optical no-light — 100 kΩ to ground, so an unpopulated `SD` on copper reads quiet |
| 4 | PE5 | `CLKIN_ICM` | `TIM15_CH1` | out | ICM pin 9 `CLKIN` — **41,94304 kHz** = 2²⁷ ÷ 3200 |
| 5 | PE6 | — | | | free |
| 6 | VBAT | — | | | tied to `VDD`; no RTC anywhere |
| 7 | PC13 | — | | | free (tamper/RTC pin, unused) |
| 8 | PC14 | — | | | free — no 32 kHz crystal |
| 9 | PC15 | — | | | free |
| 10 | VSS | | | | |
| 11 | VDD | 3,3 V | | | |
| 12 | PH0 | `CLK_IN` | `OSC_IN`, bypass | in | the data body's pin 1 `CLK/PPS`, 2²² |
| 13 | PH1 | — | |  | unconnected in bypass mode |
| 14 | NRST | reset | |  | 100 nF, no pull beyond the internal |
| 15 | PC0 | `ID_D` | `ADC12_INP10` | in | the data body's `ID` — 10 kΩ 1 % to `VREF+` |
| 16 | PC1 | `ID_P` | `ADC12_INP11` | in | the power body's `ID` — 10 kΩ 1 % to `VREF+` |
| 17 | PC2 | `NTC` | `ADC1_INP12` | in | the coil NTC's divider midpoint, ratiometric against `VREF+` |
| 18 | PC3 | `NTC_EN` | GPIO | out | the divider's top, high for the conversion only |
| 19 | VSSA | | | | |
| 20 | VREF− | | | | |
| 21 | VREF+ | 3,3 V through `L_VDDA` — the ADC reference | | | |
| 22 | VDDA | 3,3 V through `L_VDDA` | | | |
| 23 | PA0 | — | | | free (ADC) |
| 24 | PA1 | — | | | free (ADC) |
| 25 | PA2 | — | | | free (ADC) |
| 26 | PA3 | — | | | free (ADC) |
| 27 | VSS | | | | |
| 28 | VDD | 3,3 V | | | |
| 29 | PA4 | `CS_ADXL` | GPIO | out | ADXL355 pin 1 `CS#` |
| 30 | PA5 | `SCK1` | `SPI1_SCK` | out | ADXL355 pin 2 `SCLK` — 33 Ω in series, as on every SPI line |
| 31 | PA6 | `MISO1` | `SPI1_MISO` | in | ADXL355 pin 4 |
| 32 | PA7 | `MOSI1` | `SPI1_MOSI` | out | ADXL355 pin 3 |
| 33 | PC4 | — | | | free (ADC) |
| 34 | PC5 | — | | | free (ADC) |
| 35 | PB0 | `INT_ADXL` | GPIO, EXTI0 | in | ADXL355 pin 12 `INT1` — data ready |
| 36 | PB1 | — | | | free (ADC) |
| 37 | PB2 | `INT_ICM` | GPIO, EXTI2 | in | ICM pin 4 `INT1` — data ready (pin 9 is the clock) |
| 38 | PE7 | — | | | free |
| 39 | PE8 | — | | | free |
| 40 | PE9 | `EXT_CLK` | `TIM1_CH1` | out | ADXL355 pin 13 `INT2` — **2²⁰ = 1,048576 MHz**, the external clock |
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
| 51 | PB12 | `CS_ICM` | GPIO | out | ICM-42688-P pin 12 `AP_CS` |
| 52 | PB13 | `SCK2` | `SPI2_SCK` | out | ICM pin 13 |
| 53 | PB14 | `MISO2` | `SPI2_MISO` | in | ICM pin 1 `AP_SDO` |
| 54 | PB15 | `MOSI2` | `SPI2_MOSI` | out | ICM pin 14 `AP_SDI` |
| 55 | PD8 | — | | | free |
| 56 | PD9 | `RXD_ECHO` | `USART3_RX` | in | the echo check on the board's own transmission |
| 57 | PD10 | `DRDY_RM` | GPIO, EXTI10 | in | RM3100 `DRDY` |
| 58 | PD11 | — | | | free |
| 59 | PD12 | `SCL_T` | `I3C1_SCL`, I²C legacy | i/o | the wall thermometer — its own controller, off the power body's bus |
| 60 | PD13 | `SDA_T` | `I3C1_SDA`, I²C legacy | i/o | |
| 61 | PD14 | `PGOOD` | GPIO | in | the `TPS629206`'s `PG`, 100 kΩ to the 3,3 V |
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
| 77 | PA15 | — | | | free |
| 78 | PC10 | `SCK3` | `SPI3_SCK` | out | SCL3300 pin 8; the RM3100 shares the bus |
| 79 | PC11 | `MISO3` | `SPI3_MISO` | in | SCL3300 pin 6 |
| 80 | PC12 | `MOSI3` | `SPI3_MOSI` | out | SCL3300 pin 7 |
| 81 | PD0 | `CS_SCL` | GPIO | out | SCL3300 pin 5 `CSB` |
| 82 | PD1 | `CS_RM` | GPIO | out | RM3100 `SSN` — the on-board population option |
| 83 | PD2 | — | | | free |
| 84 | PD3 | — | | | free — the coil thermometer is an NTC on an ADC pin (`../gauss/HARDWARE.md`) |
| 85 | PD4 | `LED` | GPIO | out | the status LED, fitted — 1 kΩ from the 3,3 V, one 50 ms blink a minute while healthy, two on a fault (`../core/HARDWARE.md`) |
| 86 | PD5 | — | | | free |
| 87 | PD6 | — | | | free |
| 88 | PD7 | — | | | free |
| 89 | PB3 | `RXD` | `TIM2_CH2` capture | in | the same net as PB7 — the timebase captures the start edge of the card's frame |
| 90 | PB4 | `SYNC` | `TIM3_CH1` | out | ADXL355 pin 14 `DRDY` — the 128 Hz sync pulse |
| 91 | PB5 | — | | | free |
| 92 | PB6 | `TXD` | `USART1_TX` / `TIM4_CH1` | out | the NodBus link, to the data body |
| 93 | PB7 | `RXD` | `USART1_RX` / `TIM4_CH2` | in | the pins switch USART → TIM4 for the ranging instant |
| 94 | BOOT0 | 10 kΩ to ground | |  | |
| 95 | PB8 | `SCL` | `I2C1_SCL` | i/o | the power body's `INA238`, alone on this bus — 4,7 kΩ to 3,3 V |
| 96 | PB9 | `SDA` | `I2C1_SDA` | i/o | 4,7 kΩ to 3,3 V |
| 97 | PE0 | — | | | free |
| 98 | VCAP | 2× 1 µF 50 V 0805 + 2× 100 nF 50 V 0603 — 2,2 µF, the sheet's 2,2 µF ±20 % | | | |
| 99 | VSS | | | | |
| 100 | VDD | 3,3 V | | | |

**39 GPIO used, 40 free, `PH1` unconnected** (PA0–PA3 · PA8–PA12 · PA15 · PB1 · PB5 · PB10 · PC4–PC7 · PC9 · PC13–PC15 · PD2 · PD3 · PD5–PD8 · PD11 · PD15 · PE0 · PE3 · PE6–PE8 · PE10–PE15).

### Timers

| timer | bits | kernel | pins | job |
|---|---|---|---|---|
| **TIM4** | 16 | 2²⁷ | CH1 → PB6, CH2 ← PB7 | the ranging turnaround: one-pulse, triggered by the capture on CH2, the return raised on CH1 after `CCR` ticks; PWM-input mode on CH2 measures the incoming pulse's width |
| **TIM2** | 32 | 2²⁷ | CH2 capture ← PB3 | the free-running timebase; the capture of the card's frame start places the round on the grid |
| **TIM1** | 16 | 2²⁷ | CH1 → PE9 | `EXT_CLK` for the ADXL355: PWM ÷128, **2²⁰ = 1,048576 MHz**, 50 % |
| **TIM3** | 16 | 2²⁷ ÷ 16 = 2²³ | CH1 → PB4 | `SYNC` for the ADXL355: `ARR` 65535 → **128 Hz** exactly, `CCR` 33 → a 3,9 µs pulse (≥ 4 `EXT_CLK` cycles) |
| **TIM15** | 16 | 2²⁷ | CH1 → PE5 | `CLKIN` for the ICM-42688-P: `ARR` 3199 → **41,94304 kHz**, 50 % |
| TIM5 · TIM8 · TIM12 · LPTIM1 · LPTIM2 | | | | free |
| TIM6 · TIM7 | 16 | | | free — the basic timers |

All three sensor clocks are integer divisions of the one 2²⁷ the card's clock makes, so the
sample instants sit on the grid by construction and the archive's differences need no resampling.

### Clock tree

| | |
|---|---|
| `OSC_IN` | 2²² = 4,194304 MHz, the data body's `CLK` behind the buffer — a NOD carries no crystal |
| HSE | bypass |
| PLL1 | M 1 · N 64 · P 2 → VCO 2²⁸ = 268,435456 MHz, `P` 2²⁷ |
| SYSCLK, AHB, APB1/2, timer kernel | **2²⁷ = 134,217728 MHz**, no further division |
| the clock gone | HSI fallback and the degraded flag (`../core/blocks/clocks.md`); the off sequence drops the clock the same way and the returning feed is the wake (`../core/PROTOCOL.md` §7) |

Every timer counts 2²⁷, the grid a card measures on, so a tick here and a tick on the card are the
same tick and a conversion to the 2²³ timebase is a shift of four bits.

### Analogue and reference

**Every filter inductor on this board is a π filter: 10 µF 1206 + 100 nF 0603 before it on the rail and
the same pair at the pins behind it** — the house rule (`../core/POWER.md`, *The filter parts*).

`VDDA` and `VREF+` on `L_VDDA`, the house 2,2 µH, 10 µF + 100 nF at the pins; `VREF−`
and `VSSA` to the ground plane at one point. The two `ID` inputs are read single-ended against
`VREF+`, and the 10 kΩ 1 % pull-up of each `ID` resistor hangs on `VREF+` itself, so the reading
is a ratio and the rail's tolerance drops out. The arrived 12 V and the input current are the power
body's `INA238`, over I2C1 — no divider on this board. The
processor's ADC measures only the two `ID` ratios and the coil NTC.

## The Galvani bodies and the 12 V

**`NB IN` is the data body, `PWR IN` the power body, `12V` the two terminals.**

**This is a unit end.** Everything on the plugged Galvani boards runs whenever the node does, held
by resistors — no processor pin switches anything on them. Both bodies are `BX2.54-2xNA` shrouded
headers; the pin numbering is the family's (`../galvani/README.md`, *The connectors*).

**The data body — 12 pins, 2×6.**

| pin | signal | on this board |
|---|---|---|
| 1 | `CLK/PPS` | into PH0 `OSC_IN`, HSE bypass — the spur's 2²² |
| 2 | `GND` | ground |
| 3 | `TXD` | PB6 — `USART1_TX`, `TIM4_CH1` for the ranging instant |
| 4 | `RXD` | PB7 — `USART1_RX`, `TIM4_CH2`; the same net on PB3 `TIM2_CH2`, the frame-start capture |
| 5 | `ID` | PC0 `ADC12_INP10` — 10 kΩ 1 % to `VREF+` |
| 6 | `ID_RET` | not connected — a measuring unit never meets a crossed cable |
| 7 | `RXD_ECHO` | PD9 — `USART3_RX`, the echo check |
| 8 | `DE` | PE2 — GPIO, low from reset, up around the node's slot and the ranging return |
| 9 | `B_DIR` | **tied to ground** — this end listens on channel B; no processor pin |
| 10 | `LINE_EN` | 10 kΩ to 3,3 V — the line side runs whenever the node does; no processor pin |
| 11 | `SD` | PE4 — GPIO input, 100 kΩ to ground; unpopulated on copper, so it reads quiet there |
| 12 | `3,3 V` | the buck's 3,3 V node |

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
| 8 | `3,3 V` | the buck's 3,3 V node — the power board's supply |

**The 12 V — two two-pole `DGPS2.5R-5.0` spring blocks**, an input and a tap, the same node: the unit
power board's island output lands on the input, and nothing on this board carries it further. No
12 V rides either body.

## Parts

| part | qty | position |
|---|---|---|
| `STM32H523VE`, LQFP100 | 1 | the processor |
| ADXL355 | 1 | SPI1, the precise accelerometer |
| ICM-42688-P | 1 | SPI2, the IMU |
| SCL3300-D01 | 1 | SPI3, the inclinometer |
| RM3100 chipset — 13156 + 3× 13104 | 1 set | SPI3, by population |
| 33 kΩ thin-film 0,1 % | 1 | the RM3100's `REXT` |
| 121 Ω thin-film | 6 | the RM3100's coil series resistors |
| NTC + switched divider | 1 | between the coils, with the RM3100 |
| TMP117 or STS35 | 1 | the wall thermometer, by population |
| `TPS629206` | 1 | the 3,3 V |
| 2,2 µH shielded, `XGL3530-222` | 1 | the buck's inductor |
| 619 kΩ · 137 kΩ, 1 % | 1 each | the buck's divider |
| 100 kΩ | 1 | `PG` up |
| 2,2 µH `SWPA252012S2R2MT` | 6 | `L_MCU` · `L_VDDA` · `L_ANA` · `L_IO` · `L_RM_A` · `L_RM_D` |
| 10 µF + 100 nF | 8 sets | behind each branch, and at the RM3100 chipset's `AVDD` and `DVDD` |
| 100 nF | one a supply pin | every sensor and processor supply pin |
| 3× 10 µF 1206 + 100 nF · 1 µF · 100 nF | 1 set | the buck's `C_OUT` · `VCC` · `BOOT` |
| 33 Ω | one a SPI line | |
| 10 kΩ 1 % | 2 | the `ID` pull-ups to `VREF+` |
| 4,7 kΩ | 2 | I2C1 |
| 100 kΩ | 2 | `SD` and `ALERT` to ground |
| 10 kΩ | 2 | `LINE_EN` to 3,3 V · BOOT0 to ground |
| 47 µF 50 V hybrid polymer + 2× 10 µF 50 V 1206 + 100 nF | 1 set | the 12 V input |
| 2× 1 µF 50 V 0805 + 2× 100 nF 0603 | 2 sets | `VCAP` |
| `DGPS2.5R-5.0`, two-pole | 2 | `12V` |
| `BX2.54-2x6NA` · `BX2.54-2x4NA` | 1 each | `NB IN` · `PWR IN` |
| 1 kΩ + LED | 1 | the status LED on PD4 |
| 6-pin header | 1 | SWD |

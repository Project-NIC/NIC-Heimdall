★ N.I.C. ★

# Photon — the `Quark-Photon` board

> **Design-stage concept — nothing built.** The figures are verified against the parts' sheets and
> the parts before the board is made. What Photon is and what it publishes is [`README.md`](README.md). The crystal's physics, the
> charge economy and the noise budgets are [`SCINTILLATION.md`](SCINTILLATION.md); the record on
> the wire is [`BUS.md`](BUS.md); the firmware — one image, this board and Positron's — is
> [`FIRMWARE.md`](FIRMWARE.md). **The GM-tube build of Photon is the HV variant and counts on
> `Quark-Tubes`** ([`../../tubes/HEADS.md`](../../tubes/HEADS.md)). Rejected and superseded states are
> [`../../WHY.md`](../../WHY.md).

**This board is also the record of the H7A3 layer the two scintillation boards share** — the
converter's port, the clock tree, the rails, the bias supply — and `Quark-Neutron/Positron` refers
here for them ([`../positron/HARDWARE.md`](../positron/HARDWARE.md)).

---

## Two sensors on `Quark-Photon`

**Both watch the same flash and both deliver charge proportional to the photons that reach
them**, and they must be sampled **simultaneously**. That is the whole reason this build has a
board of its own rather than sharing Positron's.

| sensor | front end | on the board |
|---|---|---|
| **SiPM** | **`THS4551`** alone — the anode on its summing node | converter channel 1 |
| **PIN**, 1–4 segments | **`LTC6268`** → CR + pole-zero 1,5 µs → **`THS4551`** | converter channel 2 |

**The ratio between them is fixed across the range** — about 330 000× — because one photon becomes
~2,8·10⁶ electrons in a SiPM microcell and **one** electron-hole pair in a PIN, with the PIN's
better quantum efficiency and larger share of the split light claiming some of that back. The
hand-over between the two is what gives the channel its span.

**The guard ring is populated on this board and it is populated for the PIN.** ~12 000 electrons
does not swallow surface leakage the way Helion's 1,25 million does, and an outdoor enclosure
condenses water.

## The heads are on the board — its underside, and no head connector

**One PCB. The board lies horizontal in the unit's enclosure and the heads hang under it, windows
down.** The SiPM's and the PIN's pads are on the **bottom layer**; the sealed CsI(Tl) assembly is
bonded to them, its window up against the sensors, and potted. The charge node is the length of
a pad, the guard ring encloses it as drawn, and there is no cable, no head board and no connector
between a sensor and its amplifier — **the one cable out of the unit is the Galvani data body's.**
The crystal's NTC, the leaded `NXFT15XH103FA2B`, is bonded to the assembly beside the sensors.

**The board carries no load.** The assembly is clamped to the enclosure's floor bracket and the
board stands on standoffs off the same bracket, so the bond to the sensors is optical and not
structural. Replacing the assembly is a rework on the bottom layer and nothing else moves.
`Quark-Neutron/Positron` is built the same way, its block and its photomultiplier's socket on the
underside (`../positron/HARDWARE.md`, *The heads are on the board*).

## `Quark-Photon` — SiPM and PIN, both live at once

**The gamma crystal has two sensors and they must be sampled simultaneously**, which is why they
share a board and why this board is not the other one.

| | |
|---|---|
| processor | **STM32H7A3IIT6** — the converter asks for it, not the arithmetic |
| converter | **`AD9251-80`** on the PSSI, interleaved output — **21 × 2²¹ = 44,040192 MSPS a channel, 88,080384 MHz on the pins**, two channels. *(The LFCSP-64 footprint and the register map are the family's, so a build may drop in a sibling: `AD9648`, the same 0,98 LSB of noise at less power; `AD9258` and the 16-bit `AD9268`, ~3–4 dB better SNR at 3–5× the power; `AD9650`, 16-bit, 82 dBFS at 328 mW a channel. Nothing here asks for it — the energy is an area over tens of samples and the resolution is the scintillator's — so the base part stays)* |
| **channel 1 — SiPM** | **`THS4551` and nothing else.** The anode lands on its summing node — the virtual ground a SiPM requires — with **`Cf` 1 nF C0G ∥ `Rf` 1,5 kΩ** in both feedback arms, τ 1,5 µs, and `VOCM` off the converter's own `VCM`. **One microcell is a step of 3,66 LSB** — the size of the ruler, and the reason `Cf` is 1 nF and not more (`SCINTILLATION.md`, *The calibration chain*) |
| **channel 2 — PIN** | **`LTC6268`** charge amplifier (dual: **`LTC6269`**) on each of 1–4 segments → CR + pole-zero at 1,5 µs → **`THS4551`** sum and gain |
| **PIN bias** | off the SiPM's `LT3571` output, **1 MΩ + 100 nF C0G 100 V** at the common cathode — ~25 V (*The SiPM bias supply*) |
| **the guard ring** | **populated, and this is the board that needs it.** The PIN node is picofarads on megohms and its signal is ~12 000 electrons; surface leakage across a PCB in an enclosure where water condenses is the same order. Drawn once, around the `LTC6268` input |
| at the converter's pins | 24,9 Ω ×2 in series + **150 pF differential** (corner 21,2 MHz), 200 Ω ×2 to `VCM` behind 0,1 µF ×2 — the termination and the DC bias in one pair of parts |
| **no shaping amplifier** | shaping is digital, on the H7A3. Nothing is stretched in analogue |

**Why 21,2 MHz is the whole anti-alias filter.** Nyquist is **22,02 MHz** and the fastest
feature in any channel is the ~54 ns single-cell pulse at ~6,5 MHz. The corner sits between the
two, so it passes every signal whole and folds nothing back. **`THS4541` is not needed**: the
150 pF absorbs the sample-and-hold kickback and the series arms only recharge it across the whole
period, so the driver's bandwidth serves the ≤ 6,5 MHz signal and not the encode rate.

### The converter's port — PSSI, and the processor owns both instants

*One port, both boards: `Quark-Neutron/Positron` carries the same converter at the same rate on the
same pins, and everything here is that board's too.*

**The port is the PSSI and the rate is 21 × 2²¹.** The `AD9251` runs its **interleaved output**
(`0x14`, output mux enable): both channels on channel A's pins at twice the encode rate, so
**88,080384 MHz on the pins and 44,040192 MSPS per channel**. Fourteen lines on
`PSSI_D0`–`D13`; `DE` and `RDY` are not used.

**The FMC is not used.** It reads only in bursts, and a burst boundary costs samples on a converter that does not wait.

**One clock, 88,080384 MHz, and the MCU owns both instants.** The `AD9251` carries no oscillator:
`CLK+`/`CLK−` is an encode **input**. **PLL2's `P` output on `MCO2` (PC9) makes 88,080384 MHz** — the
word rate — and a **`74AUC2G34`** tight against the converter hands it out twice: **one gate to the
converter's `CLK+`, the other to `PSSI_PDCK` (PA6)**. Inside the converter the **input clock
divider is set to ÷2** (register `0x0B`), so the part samples at **44,040192 MSPS a channel** off the
same edges the PSSI reads on. The sampling grid is the station's clock tree and nothing else; **the
converter's `DCOA` and `DCOB` are left unconnected.**

**Why not the converter's own `DCO`: the interleaved output is double data rate.** The datasheet's
Figure 3 (*CMOS Interleaved Output Timing*) puts `DCOA` at the encode rate, one period per sample,
with channel A's word on one level of it and channel B's on the other — the data changes on
**both** edges of a 44 MHz clock. The PSSI captures on **one** edge of `PDCK`, so on `DCOA` it would
read one channel and lose the other. Clocking the converter at the word rate and dividing inside
is what turns the part's DDR into the PSSI's SDR, and the part is built for it: an integer divider
1–8, a `SYNC` input that resets it, and a single-ended 1,8 V CMOS clock input good to 200 MHz
(Figure 54). At ÷2 the duty cycle stabiliser is not needed — the sheet asks for it only at ratios
other than 1, 2 and 4.

**`SYNC` gives the divider a defined state; the test pattern says which word is which.** A ÷2
divider has two states, so at start-up the firmware pulses `SYNC` (PG3) once with `0x100` set to
*master sync enable · clock divider sync enable · next sync only*, and the divider resets to its
initial state — the part synchronises the pulse internally, and with one converter no external
alignment to `t_SSYNC`/`t_HSYNC` is needed. What the reset cannot tell the firmware is which word
of its DMA ring is channel A, because the ring's start is asynchronous to the converter; so at
boot channel A's test pattern is set to positive full scale and channel B's to negative (`0x0D`
is a per-channel register through `0x05`), the ring's first words are read, and the parity is
known for as long as the stream runs (`FIRMWARE.md` B2).

| | |
|---|---|
| PC9 `MCO2` (PLL2 `P` ÷ 4) → `74AUC2G34` → converter `CLK+` **and** PA6 `PSSI_PDCK` | **88,080384 MHz** — the word rate on both |
| the converter's divider, `0x0B` | **÷2 → 21 × 2²¹ = 44,040192 MSPS a channel** — the encode, the grid |
| `DCOA` · `DCOB` | unconnected |
| `SYNC` PG3 | one pulse at start; `0x100` = 0x07; the A/B parity from the per-channel test pattern |

**The timing, off the sheet's three numbers.** A ÷2 divider clocks on the input's rising edges, so
both edges of the internal encode — and with them every word change of the interleaved output —
fall on rising edges of the 88 MHz: the data moves **`t_DCO` + `t_SKEW` = 3,1 ns** after a rising
edge and holds until 3,1 ns after the next, 11,35 ns later. **The PSSI samples on the rising edge**
(`PCKPOL`), the one that follows the change: **setup ≈ 8,2 ns against the PSSI's 2 ns, hold ≈ 3,1 ns
against its 1 ns.** What eats into that is the mismatch of the two gates in one package, a few
hundred picoseconds, and the difference of the data lines' traces against `PDCK`'s, which is why
the data lines are routed as a group and `PDCK`'s own trace is kept as short. The converter's `0x17` moves the
eye, delaying the data in steps of 0,56 ns to 4,48 ns.

**The pipeline latency is not a synchronisation problem.** The part presents word *n* nine encode
clocks later; the stream never stops, so that is a constant offset of the sample index and nothing
more. The firmware subtracts it as `LAT` and the first nine words after start-up are discarded
(`FIRMWARE.md` B6).

**The clock ceiling — DS13195 Table 111, PSSI receive:**

| | |
|---|---|
| `PSSI_PDCK` maximum | **100 MHz** |
| `PSSI_PDCK` / `f_HCLK` | ≤ **0,4** — at a 2²⁸ kernel that allows 107 MHz |
| on the pins here | **88,080384 MHz** — 12 % under the first, 18 % under the second |
| data setup / hold at the pin | **2 ns / 1 ns** |

*(Table 110 is PSSI **transmit** and caps `PSSI_PDCK` at 50 MHz. It does not apply: this port
receives.)*

**Parallel CMOS is the only output the part has** — the `AD9251`'s drivers interface 1,8 V to 3,3 V
CMOS logic and there is no LVDS output on the part (the *CMOS/LVDS/LVPECL* in its specification is
the **clock input**). `0x14` carries the output mode and **`10` is the 1,8 V CMOS setting** this
board needs, with the interleaved mux **on** and the data format set there too. `0x0D` stays at
`0x00` in service, the test modes being a bring-up tool.

What CMOS costs is on `DRVDD`, and the datasheet gives the arithmetic:
`I_DRVDD = V_DRVDD × C_LOAD × f_CLK × N`. At 1,8 V, the datasheet's own 5 pF a line, 88,08 MHz on the
pins and the 14 lines interleaving carries, that is **11,1 mA — 20 mW — as an absolute worst case**, every
bit toggling every cycle; the real figure is the average number of bits that move. That current is
what the shielded 2,2 µH, its 10 µF + 100 nF and the bead into 100 nF at `DRVDD`/`IOVDD` exist to hold off the analogue side,
and the datasheet's own instruction is the layout rule this board follows: keep the output lines
short and lightly loaded, because the transients degrade the converter's own dynamic performance.

**The clock is not a question either.** The converter's aperture jitter is **0,1 ps rms**, on a
channel whose fastest feature is at 6,5 MHz; the part's own SNR-against-jitter curve does not start
to bend until picoseconds. The `-80` grade's floor is a **12,5 ns** clock period against the
22,7 ns encode here, and the 9-clock pipeline and the CMOS timing are specified identically across
every grade of the family.

**DCMI and PSSI are one block, and only one runs.** They share the pads — the alternate-function
table writes them as one entry, `DCMI_Dn`/`PSSI_Dn` — share one AHB3 master port, one FIFO and one
DMA request, and `PSSI_CR.ENABLE` with `DCMI_CR.ENABLE` must not both be set to 1. This board runs
the PSSI and carries no camera, so nothing competes.

**The rate is not a power of two, and that is what buys the samples back.** Interleaving costs half
the encode rate, so the binary rung below 2²⁶ would be 2²⁵ and **1,8 samples** across the ~54 ns
single-cell pulse. 21 × 2²¹ is the highest rate that keeps an integer sample count per second
**and** per frame, is reachable by an integer PLL, and leaves the pins under the PSSI's ceiling:
**2,38 samples**, 31 % more. The 9-clock pipeline is **204 ns**.

**Nyquist stays above the anti-alias corner.** 22,02 MHz against the network's 21,2 MHz — 3,7 % of
margin, which is thin but on the right side, so the capacitors do not move. The fastest feature in
any channel is still the ~54 ns pulse at ~6,5 MHz, well below both.

**Nothing else about this port is open.** The bench tests are the converter's own PN 23 (`0x0D` =
`0101`) run through all fourteen lines and the checkerboard (`FIRMWARE.md` B13).

**Tesla's port is the mirror of this one, and what differs is the converter and not a preference.**
There the `ADS127L14` emits `DCLK` and `FSYNC` and the processor follows on SAI, because a
frame-sync data port is the timing master of its own bus. Here the processor emits the encode and
the converter answers. A part with a parallel output can be driven; a part with a serial data port
must be followed (`../../../tesla/HARDWARE.md` §8).

## The processor — `Quark-Photon`: pins, timers, clock tree

*The record to draw the H7A3 from and to set CubeMX by. Package LQFP176, `STM32H7A3IIT6` — the LDO-supply part, no SMPS pins; pin
numbers and alternate functions from DS13195 Rev 8, the LQFP176 column. Every pin below is either
assigned or listed free — nothing is left to guess.*

### Peripherals

| peripheral | instance | pins | job | mode |
|---|---|---|---|---|
| the NodBus link | **USART1** + **TIM4** | PB6 `TXD` · PB7 `RXD` | the spur to its card, 8N1 at the rung `BUSCFG` hands down — 2²⁰ alone, 2²¹ chained, through the fixed-direction translators | the pins switch USART → TIM4 for the ranging instant |
| the frame anchor | **TIM2** (32-bit) | PB3 ← `RXD` on CH2 | the card's frame start on the grid | never reset, read as differences |
| echo receiver | **USART3**, RX only | PB11 `RXD_ECHO` | hears the board's own frames — Marconi's pin, with `PGOOD` and the LED on PH2 and PH3 as there | |
| the converter's data | **PSSI**, 16-bit receive, 14 lines used | `D0`–`D13` on PH9–PH12 · PH14 · PH15 · PI0–PI4 · PI6 · PI7 · PF11, the encode on PA6 | the AD9251's interleaved output — A and B alternating on one 14-bit port at twice the encode rate — into AXI SRAM through DMA | 88,08 M words/s, 44,04 MSPS a channel |
| the converter's clock | `MCO2` on PC9, PLL2's `P` output, through a `74AUC2G34` to the converter's `CLK+` and to `PSSI_PDCK` PA6 | PC9 · PA6 | **88,080384 MHz on both** — the word rate; the converter's own divider ÷2 makes the 44,040192 MSPS encode, the grid; `DCOA`/`DCOB` unconnected | `MCO2` at prescaler 1 — a timer cannot make it: 2²⁸ / (21 × 2²²) = 64/21, no integer divider |
| the converter's registers | **SPI3**, half-duplex + `CSB` PD2 | PC10 `SCLK` · PC12 `SDIO` | the AD9251's three-wire port, written at boot | ≤ 10 MHz; `PDWN` PG2, `SYNC` PG3, `ORA`/`ORB` PG4/PG5, `OEB` tied low, `DCOA`/`DCOB` unconnected |
| SiPM bias | **DAC1** · GPIO · **ADC1** | PA4 `BIAS_SET` · PE5 `BIAS_EN` · PC3 `IBIAS_MON` | the `LT3571`: setpoint, enable, current monitor | the servo's setpoint is `N`, the one-cell step, in raw LSB |
| the amplifiers | GPIO | PG7 · PG8 | the two `THS4551`s' `PD` | |
| thermometers | **ADC1** | `NTC_X` PA0 · `NTC_S` PA1 · `NTC_P` PA2 · `NTC_EN` PF6 | **three NTCs of one kind, never in the data** — the crystal's rides the minute's `REPORT` frame, the other two the `HEALTH` frame: at the **scintillation crystal** — the one the correction uses, its light-yield tempco and the DCR figure — the leaded `NXFT15XH103FA2B` bonded to the crystal; at the **SiPM** and at the **PIN** the 0603 `NCP18XH103F03RB` beside the part, which no correction consumes today and which are there because the parts carry no sensor of their own and a bias law written on temperature is then firmware and not a board (`SCINTILLATION.md`). the station's divider: 10 kΩ from `NTC_EN` to the pin, the NTC from the pin to `VSSA`, **3 kΩ + 100 nF at the ADC pin**, read against `VREF+` so the rail drops out (`../../../tesla/HARDWARE.md` §7) | `NTC_EN` high for the conversion only; no bus, no edges, nothing to blank |
| ID reads | **ADC1** | PC0 · PC1 | one resistor per body, against 10 kΩ 1 % to `VREF+`, the 1,8 V | read once at bring-up |
| PSRAM | **OCTOSPI1**, port 2 | PF0–PF5 · PG0 · PG1 · PG10–PG12 · PF12 | the `APS25608N`, 32 MB octal DDR (*The PSRAM*, below) | 2²⁶, DQS on PF12; **the bus is silent while the front measures** — written only after a burst closes |
| power body | **I2C1** through the `PCA9306` | PB8 · PB9 | the unit power board's `INA238`, alone on its controller — the bus stops at the connector and never reaches the front end, so this read is not blanked | 100 kHz; `ALERT` on PC8 |
| clock in | **HSE bypass** | PH0 | the data body's `CLK`, 2²², through the translator | PH1 unconnected |
| debug | **SWD** | PA13 · PA14 | | no JTAG, no trace |

No regulator is enabled from a pin: every rail runs whenever the 12 V is there, and the parts
that sleep sleep by their own pins — the converter's `PDWN`, the amplifiers' `PD`.

### Pins

| pin | port | signal on the board | AF / function | dir | note |
|---|---|---|---|---|---|
| 1 | PE2 | `DE` | GPIO | out | through the translator — the data body's driver enable |
| 2 | PE3 | — | | | free |
| 3 | PE4 | `SD` | GPIO | in | through the translator — the optical no-light; 100 kΩ to ground on the 3,3 V side |
| 4 | PE5 | `BIAS_EN` | GPIO | out | the bias converter's enable |
| 5 | PE6 | — | | | free |
| 6 | VBAT | — | | | tied to `VDD`; no RTC anywhere |
| 7 | PI8 | — | | | free |
| 8 | PC13 | — | | | free (tamper/RTC pin, unused) |
| 9 | PC14 | — | | | free — no 32 kHz crystal |
| 10 | PC15 | — | | | free |
| 11 | PI9 | — | | | free |
| 12 | PI10 | — | | | free |
| 13 | PI11 | — | | | free |
| 14 | VSS | | | | |
| 15 | VDD | 1,8 V | | | |
| 16 | PF0 | `PS_IO0` | `OCTOSPIM_P2_IO0` | i/o | the `APS25608N`, octal DDR — OCTOSPI1 on port 2 |
| 17 | PF1 | `PS_IO1` | `OCTOSPIM_P2_IO1` | i/o | |
| 18 | PF2 | `PS_IO2` | `OCTOSPIM_P2_IO2` | i/o | |
| 19 | PF3 | `PS_IO3` | `OCTOSPIM_P2_IO3` | i/o | |
| 20 | PF4 | `PS_CLK` | `OCTOSPIM_P2_CLK` | out | 2²⁶ — the pad table's 120 MHz octal-DDR ceiling; 33 Ω at the pad |
| 21 | PF5 | `PS_NCLK` | `OCTOSPIM_P2_NCLK` | out | unused — single-ended clock |
| 22 | VSS | | | | |
| 23 | VDD | 1,8 V | | | |
| 24 | PF6 | `NTC_EN` | GPIO | out | the three NTC dividers' top, high for the conversion only — PF3 is the PSRAM's `IO3`, as on Marconi |
| 25 | PF7 | — | | | free |
| 26 | PF8 | — | | | free |
| 27 | PF9 | — | | | free |
| 28 | PF10 | — | | | free |
| 29 | PH0 | `CLK_IN` | `OSC_IN`, bypass | in | the data body's `CLK`, 2²², through the translator — `V_IH` ≥ 1,26 V at `VDD` 1,8 V |
| 30 | PH1 | — | | | unconnected in bypass mode |
| 31 | NRST | reset | | | 100 nF, no pull beyond the internal |
| 32 | PC0 | `ID_D` | `ADC12_INP10` | in | the data body's `ID` — 10 kΩ 1 % to `VREF+`, the 1,8 V, so the reading is a ratio |
| 33 | PC1 | `ID_P` | `ADC12_INP11` | in | the power body's `ID`, the same way |
| 34 | PC2 | — | | | free (ADC) |
| 35 | PC3 | `IBIAS_MON` | `ADC12_INP13` | in | the bias converter's current monitor — telemetry, a flag and not a measurement. On LQFP176 the pin is `PC3_C`: `INP13` through the `SYSCFG` analog switch, or `ADC2_INP1` direct (DS13195 Table 7) |
| 36 | VDD | 1,8 V | | | |
| 37 | VSSA | | | | |
| 38 | VREF+ | the `VDDA` node — the ADC reference, strapped to `VDDA` | | | |
| 39 | VDDA | 1,8 V through the shielded 2,2 µH `SWPA252012S2R2MT`, 10 µF + 100 nF at the pins, `VREF+` on the same node — the rail is a buck's own, so the filter is the buck-noise inductor of `../../../core/POWER.md`, not a ferrite | | | |
| 40 | PA0 | `NTC_X` | ADC | in | the crystal's NTC, on its divider, 3 kΩ + 100 nF at the pin |
| 41 | PA1 | `NTC_S` | ADC | in | the SiPM's NTC, on its divider, RC at the pin |
| 42 | PA2 | `NTC_P` | ADC | in | the PIN's NTC, on its divider, RC at the pin |
| 43 | PH2 | `PGOOD` | GPIO | in | the 1,8 V `TPS629206`'s `PG` |
| 44 | PH3 | `LED` | GPIO | out | the status LED, fitted — 1 kΩ from the 3,3 V, one 50 ms blink a minute while healthy, two on a fault (`../../../core/HARDWARE.md`) |
| 45 | PH4 | — | | | free |
| 46 | PH5 | — | | | free |
| 47 | PA3 | — | | | free (ADC) |
| 48 | VSS | | | | |
| 49 | VDD | 1,8 V | | | |
| 50 | PA4 | `BIAS_SET` | `DAC1_OUT1` | out | the SiPM bias converter's setpoint — the `LT3571`'s control input; inside the servo, needs no accuracy |
| 51 | PA5 | — | | | free (ADC) |
| 52 | PA6 | `PDCK` | `PSSI_PDCK` | in | **88,080384 MHz** — the read clock, the second gate of the `74AUC2G34` off `MCO2`; a short trace |
| 53 | PA7 | — | | | free (ADC) |
| 54 | PC4 | — | | | free (ADC) |
| 55 | PC5 | — | | | free (ADC) |
| 56 | PB0 | — | | | free (ADC) |
| 57 | PB1 | — | | | free (ADC) |
| 58 | PB2 | — | | | free |
| 59 | PF11 | `D12` | `PSSI_D12` | in |  |
| 60 | PF12 | `PS_DQS` | `OCTOSPIM_P2_DQS` | i/o | |
| 61 | VSS | | | | |
| 62 | VDD | 1,8 V | | | |
| 63 | PF13 | — | | | free (ADC) |
| 64 | PF14 | — | | | free (ADC) |
| 65 | PF15 | — | | | free |
| 66 | PG0 | `PS_IO4` | `OCTOSPIM_P2_IO4` | i/o | |
| 67 | PG1 | `PS_IO5` | `OCTOSPIM_P2_IO5` | i/o | |
| 68 | PE7 | — | | | free |
| 69 | PE8 | — | | | free |
| 70 | PE9 | — | | | free |
| 71 | VSS | | | | |
| 72 | VDD | 1,8 V | | | |
| 73 | PE10 | — | | | free |
| 74 | PE11 | — | | | free |
| 75 | PE12 | — | | | free |
| 76 | PE13 | — | | | free |
| 77 | PE14 | — | | | free |
| 78 | PE15 | — | | | free |
| 79 | PB10 | — | | | free |
| 80 | PB11 | `RXD_ECHO` | `USART3_RX` | in | through the translator — the echo check; 100 kΩ to ground on the 3,3 V side |
| 81 | VCAP | 2× 1 µF 50 V 0805 + 2× 100 nF 50 V 0603 — 2,2 µF, the sheet's 2,2 µF ±20 %, the internal LDO | | | |
| 82 | VDD | 1,8 V | | | |
| 83 | PH6 | — | | | free |
| 84 | PH7 | — | | | free |
| 85 | PH8 | — | | | free |
| 86 | PH9 | `D0` | `PSSI_D0` | in | the converter's data — the interleaved stream, both channels on one port |
| 87 | PH10 | `D1` | `PSSI_D1` | in |  |
| 88 | PH11 | `D2` | `PSSI_D2` | in |  |
| 89 | PH12 | `D3` | `PSSI_D3` | in |  |
| 90 | VSS | | | | |
| 91 | VDD | 1,8 V | | | |
| 92 | PB12 | — | | | free |
| 93 | PB13 | — | | | free |
| 94 | PB14 | — | | | free |
| 95 | PB15 | — | | | free |
| 96 | PD8 | — | | | free |
| 97 | PD9 | — | | | free |
| 98 | PD10 | — | | | free |
| 99 | PD11 | — | | | free |
| 100 | PD12 | — | | | free |
| 101 | PD13 | — | | | free |
| 102 | VSS | | | | |
| 103 | VDD | 1,8 V | | | |
| 104 | PD14 | — | | | free |
| 105 | PD15 | — | | | free |
| 106 | PG2 | `PDWN` | GPIO | out | the AD9251's `PDWN` — the converter asleep |
| 107 | PG3 | `SYNC` | GPIO | out | the AD9251's `SYNC` — resets the ÷2 divider to a defined state; pulsed once at start |
| 108 | PG4 | `ORA` | GPIO | in | channel A overrange — polled per block, not an interrupt |
| 109 | PG5 | `ORB` | GPIO | in | channel B overrange |
| 110 | PG6 | — | | | free |
| 111 | PG7 | `PD_AMP1` | GPIO | out | channel 1's `THS4551` `PD` |
| 112 | PG8 | `PD_AMP2` | GPIO | out | channel 2's `THS4551` `PD`; the `LTC6268`s have no power-down and sit on the clean rail, which is never switched |
| 113 | VSS | | | | |
| 114 | VDD33USB | tied to `VDD` | | | USB unused |
| 115 | PC6 | — | | | free |
| 116 | PC7 | — | | | free |
| 117 | PC8 | `ALERT` | GPIO, EXTI | in | through the translator — the power body's `INA238`; 100 kΩ to ground on the 3,3 V side, high = alarm |
| 118 | PC9 | `ENC` | `MCO2` — PLL2 `P` | out | **88,080384 MHz** — through a `74AUC2G34` tight against the converter, one gate to its `CLK+` (÷2 inside → the 44,04 encode, the grid), one to `PDCK` |
| 119 | PA8 | — | | | free |
| 120 | PA9 | — | | | free |
| 121 | PA10 | — | | | free |
| 122 | PA11 | — | | | free (USB, unused) |
| 123 | PA12 | — | | | free (USB, unused) |
| 124 | PA13 | `SWDIO` | SWD | i/o | |
| 125 | VCAP | 2× 1 µF 50 V 0805 + 2× 100 nF 50 V 0603 — 2,2 µF, the sheet's 2,2 µF ±20 %, the internal LDO | | | |
| 126 | VSS | | | | |
| 127 | VDD | 1,8 V | | | |
| 128 | PH13 | — | | | free |
| 129 | PH14 | `D4` | `PSSI_D4` | in |  |
| 130 | PH15 | `D11` | `PSSI_D11` | in |  |
| 131 | PI0 | `D13` | `PSSI_D13` | in | the top bit |
| 132 | PI1 | `D8` | `PSSI_D8` | in |  |
| 133 | PI2 | `D9` | `PSSI_D9` | in |  |
| 134 | PI3 | `D10` | `PSSI_D10` | in |  |
| 135 | VSS | | | | |
| 136 | VDD | 1,8 V | | | |
| 137 | PA14 | `SWCLK` | SWD | in | |
| 138 | PA15 | — | | | free |
| 139 | PC10 | `SCLK_C` | `SPI3_SCK` | out | the AD9251's `SCLK` — the configuration port |
| 140 | PC11 | — | | | free |
| 141 | PC12 | `SDIO_C` | `SPI3_MOSI`, half-duplex | i/o | the AD9251's `SDIO`, bidirectional |
| 142 | PD0 | — | | | free |
| 143 | PD1 | — | | | free |
| 144 | PD2 | `CSB_C` | GPIO | out | the AD9251's `CSB` |
| 145 | PD3 | — | | | free |
| 146 | PD4 | — | | | free |
| 147 | PD5 | — | | | free |
| 148 | VSS | | | | |
| 149 | VDDMMC | 1,8 V | | | |
| 150 | PD6 | — | | | free |
| 151 | PD7 | — | | | free |
| 152 | PG9 | — | | | free |
| 153 | PG10 | `PS_IO6` | `OCTOSPIM_P2_IO6` | i/o | |
| 154 | PG11 | `PS_IO7` | `OCTOSPIM_P2_IO7` | i/o | |
| 155 | PG12 | `PS_NCS` | `OCTOSPIM_P2_NCS` | out | 10 kΩ to the 1,8 V — deselected while the pins are high-Z at boot |
| 156 | PG13 | — | | | free |
| 157 | PG14 | — | | | free |
| 158 | VSS | | | | |
| 159 | VDD | 1,8 V | | | |
| 160 | PG15 | — | | | free |
| 161 | PB3 | `RXD` | `TIM2_CH2` capture | in | the same net as PB7 — the timebase captures the start edge of the card's frame |
| 162 | PB4 | — | | | free |
| 163 | PB5 | — | | | free |
| 164 | PB6 | `TXD` | `USART1_TX` / `TIM4_CH1` | out | the NodBus link — through the `SN74AXC` translator to the data body |
| 165 | PB7 | `RXD` | `USART1_RX` / `TIM4_CH2` | in | through the translator; the pins switch USART → TIM4 for the ranging instant |
| 166 | BOOT0 | 10 kΩ to ground | | | |
| 167 | PB8 | `SCL` | `I2C1_SCL` | i/o | through the `PCA9306` to the power body — the `INA238`, alone on this bus; 4,7 kΩ on each side |
| 168 | PB9 | `SDA` | `I2C1_SDA` | i/o | through the `PCA9306`; 4,7 kΩ on each side |
| 169 | PE0 | — | | | free |
| 170 | PE1 | — | | | free |
| 171 | PDR_ON | tied to `VDD` | | | the power-down reset stays armed |
| 172 | VDD | 1,8 V | | | |
| 173 | PI4 | `D5` | `PSSI_D5` | in |  |
| 174 | PI5 | — | | | free |
| 175 | PI6 | `D6` | `PSSI_D6` | in |  |
| 176 | PI7 | `D7` | `PSSI_D7` | in |  |

**60 GPIO used, 79 free, `PH1` unconnected** (PA3 · PA5 · PA7–PA12 · PA15 · PB0–PB2 · PB4–PB5 · PB10 · PB12–PB15 · PC2 · PC4–PC7 · PC11 · PC13–PC15 · PD0–PD1 · PD3–PD15 · PE0–PE1 · PE3 · PE6–PE15 · PF7–PF10 · PF13–PF15 · PG6 · PG9 · PG13–PG15 · PH4–PH8 · PH13 · PI5 · PI8–PI11).

### Timers

| timer | bits | kernel | pins | job |
|---|---|---|---|---|
| **TIM4** | 16 | 2²⁸ | CH1 → PB6, CH2 ← PB7 | the ranging turnaround: one-pulse, triggered by the capture on CH2, the return raised on CH1 after `CCR` ticks; PWM-input mode on CH2 measures the incoming pulse's width. 244 µs of range at 2²⁸ — a port past 24 km runs it at ÷4 |
| **TIM2** | 32 | 2²⁸ | CH2 capture ← PB3 | the free-running timebase; the capture of the card's frame start places the round on the grid |
| TIM1 · TIM3 · TIM5 · TIM8 · TIM12–TIM17 · LPTIM1–LPTIM5 | | | | free |
| **TIM6** | 16 | 2²⁸ | no pin | **the housekeeping second**: the ladder's one-second scan and the `IBIAS_MON` read, at the lowest interrupt priority (`FIRMWARE.md`) |
| TIM7 | 16 | | | free |

### The station interface — 3,3 V on translators

**The station interface stays 3,3 V** — the Galvani body is 3,3 V logic on every board in the
station and does not change for a board whose processor runs 1,8 V. **Fixed-direction level
translators** — two `SN74AXC8T245`, one a direction, `DIR` strapped, Marconi's pair (1,2–3,6 V,
push-pull on both sides, ns-class edges) — carry the connector's lines — **2 out** (`TXD`, `DE`), **5 in** (`RXD`, `RXD_ECHO`, `CLK` into
`OSC_IN`, `SD`, `ALERT`); **a `PCA9306`** carries `SDA`/`SCL` to the power board's `ISO1642`,
which does not take 1,8 V, with 4,7 kΩ on each side. `ALERT`, `SD` and `RXD_ECHO` are held low
by 100 kΩ on the 3,3 V side, so an empty or dead line reads quiet. `ID` needs nothing — it is a
ratio against the board's own `VREF+`. The rest are resistors and no processor pin: `LINE_EN`
10 kΩ to 3,3 V, `A_SEL` to ground — one power socket on a unit, so its board is 0x40 — `B_DIR`
tied to ground, and `ID_RET` and `ENABLE` not connected (*The sockets*). **An auto-direction translator is not used on a clock line**: its
one-shot accelerators and pass-gate pull-ups round the edges into series resistance and cable
capacitance.

### The sockets — the data body and the power body

**Connectors: `NB IN` · `PWR IN` · `12V`** (`../../../galvani/README.md`, *Connector names*).

**`Quark-Photon` is a measuring unit and stands at the U end of its spur, never in the station's
enclosure.** Everything on the Galvani boards behind it runs whenever the board does, held by
resistors: no processor pin switches anything on either body. Every logic line crosses the
`SN74AXC` translators or the `PCA9306`; the connector side is 3,3 V.

**The data body — 12 pins, 2×6, `BX2.54-2xNA`.**

| pin | signal | on this board |
|---|---|---|
| 1 | `CLK/PPS` | the spur's 2²², through the `SN74AXC` translator into PH0 `OSC_IN` |
| 2 | `GND` | ground |
| 3 | `TXD` | PB6 `USART1_TX` / `TIM4_CH1`, through the translator |
| 4 | `RXD` | PB7 `USART1_RX` / `TIM4_CH2`, and PB3 `TIM2_CH2` on the same net, through the translator |
| 5 | `ID` | PC0 `ID_D`, 10 kΩ 1 % to `VREF+`, the 1,8 V |
| 6 | `ID_RET` | not connected — a unit carries no host code |
| 7 | `RXD_ECHO` | PB11 `USART3_RX`, through the translator; 100 kΩ to ground on the 3,3 V side |
| 8 | `DE` | PE2, GPIO out, through the translator |
| 9 | `B_DIR` | **tied to ground** — this end listens on channel B; no processor pin |
| 10 | `LINE_EN` | 10 kΩ to 3,3 V — the line side runs whenever the board does; no processor pin |
| 11 | `SD` | PE4, GPIO in, through the translator; 100 kΩ to ground on the 3,3 V side |
| 12 | `3,3 V` | the ordinary 3,3 V, the second `TPS7A2033` (*The rails*) |

**The power body — 8 pins, 2×4, `BX2.54-2xNA`.**

| pin | signal | on this board |
|---|---|---|
| 1 | `ENABLE` | not connected — the unit power board pulls it to its input |
| 2 | `GND` | ground |
| 3 | `ID` | PC1 `ID_P`, 10 kΩ 1 % to `VREF+`, the 1,8 V |
| 4 | `SDA` | PB9 `I2C1_SDA` through the `PCA9306`, 4,7 kΩ on each side |
| 5 | `A_SEL` | to ground — the `INA238` at 0x40, alone on I2C1 |
| 6 | `SCL` | PB8 `I2C1_SCL` through the `PCA9306`, 4,7 kΩ on each side |
| 7 | `ALERT` | PC8, EXTI, through the translator; 100 kΩ to ground on the 3,3 V side, high = alarm |
| 8 | `3,3 V` | the same ordinary 3,3 V — the power board's supply |

**The 12 V** arrives from the unit power board's island output on two two-pole Degson
**`DGPS2.5R-5.0`** terminals, an input and a tap, the same node, and on neither ribbon.

### Clock tree

| | |
|---|---|
| `OSC_IN` | 2²² = 4,194304 MHz, the data body's `CLK` through the translator — **no quartz on this board at all**: the station's clock is what runs it, and the word *crystal* in this project's scintillation text means the **scintillator** |
| HSE | bypass |
| PLL1 | M 1 · N 128 · P 2 → VCO 2²⁹ = 536,870912 MHz, `P` **2²⁸** |
| SYSCLK, AXI, AHB | **2²⁸ = 268,435456 MHz** — VOS0, `VDD` ≥ 1,71 V |
| APB1/2 | ÷2; the timer kernels at 2²⁸ |
| PLL2 | M 1 · N 84 · **P 4** → VCO 352,321536 MHz — inside the wide VCO's 128–560 MHz, the only range a 4,19 MHz input may drive — `P` **21 × 2²² = 88,080384 MHz**, out on `MCO2` (PC9) as the converter's clock and the PSSI's `PDCK`. Integer, off the same 2²² reference; no fractional divider |
| the converter's clock | **88,080384 MHz** on PC9 `MCO2` to the converter's `CLK+` and to `PSSI_PDCK` alike; the converter's ÷2 makes the **44,040192 MSPS** encode — the grid; nothing comes back from the converter |
| OCTOSPI kernel | 2²⁸ ÷ 4 = **2²⁶** — the PSRAM's clock, octal DDR |
| the clock gone | HSI fallback and the degraded flag; the off sequence drops the clock the same way and the returning feed is the wake (`../../../core/PROTOCOL.md` §7) |

Two integer PLLs off the one 2²² reference and no fractional divider anywhere. The encode is not a
power of two: **2²⁸ / fs = 128/21 exactly**, so a time in 2²⁸ ticks becomes a sample index by one
multiply and one shift, with no remainder.

### The PSRAM — a position on the board

**`APS25608N-OBR-BD`, 32 MB octal PSRAM, 1,8 V, on OCTOSPI1 port 2 in octal DDR memory-mapped mode
at 2²⁶** — the drawing carries the position whether or not it is loaded. **32 MB is the smallest octal part made at this speed**;
a smaller one would not be cheaper. The figures are the sheet's (AP Memory rev. 1.2).

| | |
|---|---|
| the part | **`APS25608N-OBR-BD`** — 256 Mb (32 M × 8), octal SPI DDR, 1,62–1,98 V; **mini-BGA 24, 6 × 8 × 1,2 mm, pitch 1,0 mm, ball 0,4 mm**, package code `BD`; the standard grade, `T_C` −40…85 °C (`-OBRX-BD` is the 105 °C grade and is not needed). 200 MHz on the sheet, 2²⁶ here |
| the balls | `CE#` A2 · `CLK` B2 · `DQS/DM` C3 · `ADQ0` D3 · `ADQ1` D2 · `ADQ2` C4 · `ADQ3` D4 · `ADQ4` D5 · `ADQ5` E3 · `ADQ6` E2 · `ADQ7` E1 · `VDD` D1, E4 · `VSS` C1, E5 · `RST#` A3 · `RFU` C2 (a second `CE#`, left open) · the rest `NC`. **`VDDQ` is `VDD` inside the part** — two supply balls, one rail |
| `RESET#` | **left open** — a weak pull-up inside; the firmware resets by the Global Reset command (four clocked `CE#` lows) after the 150 µs power-up phase |
| power-up | `CE#` tracks `VDD` within 200 mV and `CLK` stays low through the first 150 µs — the **10 kΩ pull-up on `NCS`** and the MCU's pins at reset do both; Halfsleep and deep power-down are usable only 1 ms and 500 µs after that |
| the bus | `IO0`–`IO7` PF0–PF2 · PF3 · PG0 · PG1 · PG10 · PG11 · `DQS` PF12 · `CLK` PF4 · `NCS` PG12; `NCLK` PF5 not used, single-ended clock. Source-synchronous: no series part on the data lines or `DQS`, traces ≤ 30 mm and matched to ±5 mm over an unbroken ground — **the sheet allows 15 pF of load**, and the pads are 5–6 pF; **33 Ω at the MCU's `CLK` pad**, the house series part on a fast line. Output drive 25 Ω, the default |
| `NCS` | **10 kΩ to the 1,8 V** — deselected while the MCU's pins are high-Z at boot, and the power-up condition above |
| decoupling | **the sheet's two**: a low-ESR **1 µF** at the supply balls (the house 1 µF 50 V) and a **10 µF** beside the part (the house 10 µF 50 V 1206) for the self-refresh bursts — 25 mA peaks of tens of µs in standby, which the regulator does not see. No filter part: a digital load on a digital rail |
| current | **≤ 20 mA while a record is written** (the sheet: 5 mA at 13 MHz, 19 mA at 133 MHz, 2²⁶ between them); standby **≤ 680 µA at 85 °C**, 90 µA typical at 25 °C; Halfsleep 40 µA typical, deep power-down ≤ 20 µA — none of them worth a mode on a board whose rails are sized for full load |
| clock | 2²⁶ = 67,108864 MHz, the OCTOSPI kernel at 2²⁸ ÷ 4; DDR, so 134 MB/s on the bus — a 120 kB record is under a millisecond. **Write latency code 4 (`MR4[7:5]` = 100b, `F_max` 109 MHz)** — the default code 5 also holds, code 3 stops at 66 MHz and is under the clock. **`CE#` low ≤ 4 µs a burst (`t_CEM`)**: the OCTOSPI's `REFRESH` field caps a memory-mapped access at 268 clocks, so the peripheral breaks long DMA transfers by itself |
| when it runs | **only after a burst closes** (`FIRMWARE.md` §B6): the record is staged in SRAM1/2/4 while the front measures and copied by MDMA afterwards, so the octal bus carries no edges during a measurement |

### Analogue and reference

**Every filter inductor on this board is a π filter: 10 µF 1206 + 100 nF 0603 before it on the rail and
the same pair at the pins behind it** — the house rule (`../../../core/POWER.md`, *The filter parts*).

`VDDA` and `VREF+` on the **1,8 V** through the shielded 2,2 µH `SWPA252012S2R2MT` into 10 µF + 100 nF; `VSSA` to
the ground plane at one point — the LQFP176 bonds `VREF−` to `VSSA` inside. The two `ID` inputs are read
single-ended against `VREF+`, and their 10 kΩ 1 % pull-ups hang on `VREF+` itself, so the reading is a
ratio and the rail's tolerance drops out; the data body's 3,3 V is not used for the `ID` on this
board. The arrived 12 V and the input current are the power body's `INA238`, over I2C1. The bias current monitor on PC3 and the DAC on PA4 are
inside the servo loop, so neither the reference's value nor its drift reaches the measurement.

## The rails — the two H7A3 boards take Marconi's tree

**Both scintillation boards run the same rails, and they are Marconi's** (`../../../marconi/HARDWARE.md`
owns the tree; `../../../galvani/HARDWARE.md`, *The low rails*, owns the parts). **A unit power board
hands the board 12 V and nothing lower**, so every rail below that is made here.

| rail | part | off | what stands on it |
|---|---|---|---|
| **4,0 V** | **`TPS629206`**, forced PWM at 2,5 MHz, `MODE/S-CONF` 9,31 kΩ, `R1` 787 kΩ / `R2` 137 kΩ → 4,047 V; never switched | the 12 V | the two 3,3 V LDOs and the `LT3571` — ~200 mA at the ceiling, 33 % of the part |
| **clean 3,3 V** | **`TPS7A2033`** | the 4,0 V | the `THS4551`s, the `LTC6268` where it is fitted, and the converter's analogue side — its ~50 mA through the `TPS7A2018` the largest share. **Each amplifier takes it through the bead `GZ2012D301TF`, 100 nF on either side of it — the first beside the LDO's output capacitor, the second at its supply pin**: the LDO rejects 45 dB at 1 MHz and less above, and the bead holds what the LDO passes in the tens of MHz where the front works |
| **3,3 V, ordinary** | a second **`TPS7A2033`** | the 4,0 V | the station interface and the plugged communication board |
| **1,8 V** | **`TPS629206`**, forced PWM at 1 MHz, `MODE/S-CONF` 22,1 kΩ, `R1` 274 kΩ / `R2` 137 kΩ → 1,800 V — ~205 mA at the ceiling, 34 % of the part | the 12 V | **the H7A3 entire** — `VDDA`/`VREF+` behind the shielded 2,2 µH `SWPA252012S2R2MT` into 10 µF + 100 nF, the buck-noise inductor of `../../../core/POWER.md`; 12 → 1,8 V at 1 MHz is 150 ns of on-time against the part's 40 ns |
| the converter's `DRVDD` · `IOVDD` | **no regulator** — the 1,8 V through **the shielded 2,2 µH into 10 µF + 100 nF, then the bead `GZ2012D301TF` into 100 nF at the pin** — the bead for the harmonics of the output edges, above the inductor's 69 MHz self-resonance: words at 88 MHz put a data line's fundamental at 44 MHz at most | the 1,8 V | |
| the PSRAM | **no regulator** — the 1,8 V on its two `VDD` balls, 1 µF at the balls and 10 µF beside the part | the 1,8 V | ≤ 20 mA while a record is written, ≤ 680 µA standing (*The PSRAM*) |
| the converter's analogue `VDD` | **`TPS7A2018`** | the clean 3,3 V | a quiet 1,8 V of its own — the `AD9251`'s `AVDD`, 1,7–1,9 V: **~50 mA** for the two channels at 44 MSPS — the sheet gives 49,5 / 52,8 mA typical / maximum for the -40 grade at 40 MSPS and 80,5 / 85,5 for the -80 at 80, and the analogue core's power scales with the clock — so ≤ 57 mA at the ceiling: 19 % of the part, (3,3 − 1,8) V × 57 mA = 86 mW in the LDO. The sheet asks separate supplies for `AVDD` and `DRVDD`, which this board has. The analogue side is the converter's hungry supply and it stands on its regulator: the `TPS7A2018`'s output capacitor, then **the bead `GZ2012D301TF` into 100 nF at every `AVDD` pin** — 57 mA through 0,20 Ω is 11 mV, inside `AVDD`'s 1,7–1,9 V; `DRVDD` behind the bead carries ≤ 11 mA |

**Everything rides 1,8 V on purpose.** `DRVDD` would take 3,3 V and buys nothing for it but a
second rail scheme ([`../../README.md`](../../README.md)). **The station interface stays 3,3 V** through
fixed-direction translators of the `SN74AXC` class, as on Tesla and Marconi.

**`Quark-Tubes` needs none of this** — an H523 on 3,3 V off the 12 V, the house `LMR43610` (`../../tubes/HARDWARE.md`). **No rail on any of the three boards is switched from a pin**; what sleeps, sleeps by its own pin — the converter's `PDWN`, the amplifiers' `PD`.

## The SiPM bias supply — the whole recipe, one per SiPM

**`LT3571`, 16-lead 3 × 3 mm QFN, off the 4,0 V rail** (the part takes 2,7–20 V). One per SiPM:
one on `Quark-Photon`, one on `Quark-Neutron/Positron` with a second position drawn and
unpopulated — the photomultiplier brings its own kV. Boost with an integrated switch, Schottky
and high-side current monitor, output to 75 V; ours makes **~27 V**.

**Its switching frequency is fixed by `R_T` and not moved by the load**, which is the reason a
separate converter exists at all rather than a processor with an internal SMPS: one fixed line is
what the filter to the SiPM is sized against (`SCINTILLATION.md`, *The charge economy*). No
frequency the part offers lies outside the chain's band, so what keeps the line out of the charge
amplifier is the filter and the distance — the family rule — and not the choice of rate.

| position | value | why |
|---|---|---|
| `V_IN` | the **4,0 V** rail | 2,7–20 V window; the rail exists for the LDOs anyway |
| `SHDN` | `BIAS_EN` **through an `SN74AXC` translator** | the pin wants **≥ 1,5 V** and draws **50–65 µA**; a 1,8 V GPIO clears it by 0,25 V and that is not margin worth keeping |
| `R_T` | **12,1 kΩ → 1 MHz** (0,85–1,15 MHz over temperature) | the sheet's characterised point (3571fa, Table 1 and the electrical table). The maximum duty there is 85 % guaranteed over temperature against the 87 % a continuous-conduction boost would need at the window's 30,8 V top — **and it does not bind, because the cell runs discontinuous**: the on-time is set by the energy a cycle delivers, a fraction of the period at a SiPM's microamperes |
| `L` | **4,7 µH, `I_SAT` ≥ 0,6 A**, shielded | **discontinuous conduction**, the sheet's route to low output ripple: `L < D·V_IN / (f·I_LIMIT)` with `D = (V_OUT + 1 − V_IN)/(V_OUT + 1)`. The tight corner is the bottom of the window, 23,9 V (`D` 0,84), at the fast oscillator corner, 1,15 MHz, and the high current limit, 0,57 A: **L < 5,1 µH**. `I_SAT` clears the 0,57 A limit; the 4,0 V rail's own soft start keeps the power-up inrush through the Schottky under it |
| `C_IN` | **1 µF X7R** at `V_IN` | the sheet's value |
| `R1` / `R2` (feedback) | **332 kΩ / 10 kΩ** | `V_OUT = V_CTRL × (1 + R1/R2)` = `V_CTRL × 34,2`. 79 µA of divider current at 27 V, against a 100 nA `FB` bias — 0,13 %, so the divider owns the node |
| `C_OUT` at the `VOUT` pin | **0,22 µF X7R 100 V** | the datasheet's rule: *less* capacitance above 25 V out, for loop stability |
| filter to the SiPM | **2 × 49R9** | **two independent jobs** — switching noise out, and the converter's loop separated from the reservoir below |
| reservoir at the SiPM | **4,7 µF X7R 100 V ∥ 100 nF C0G 100 V** | the charge economy's number. **100 V rated at 27 V applied** — a 50 V part loses ~40 % of its value to DC bias derating and that is directly a gain error |

**The divider sets the window; the DAC sets the point.** `CTRL` accepts **0 to 1 V** and the chip
regulates `FB` to equal it; **above 1,2 V it reverts to its own 1 V reference and the servo loses
the knob**, so `BIAS_SET` is driven, never left floating. With the divider above:

| `CTRL` | bias |
|---|---|
| 0,70 V | 23,9 V |
| **0,79 V** | **27,0 V** — nominal |
| 0,90 V | 30,8 V |

That window covers the **±0,25 V** part-to-part `V_br` spread and the **2,15 V** of tempco over
−40…+60 °C with room at both ends. **One DAC code is 15 mV of bias** — under one kelvin of that
tempco, which is what lets the servo step one code a minute and never hunt
(`FIRMWARE.md`, *The bias servo*).

**`IBIAS_MON` is telemetry and its bands are the part's**: 250 nA – 10 µA needs `V_MONIN` above
10 V, 10 µA – 2,5 mA above 20 V. At 27 V with a SiPM drawing ~1,5 µA the reading sits inside the
first band at both ends. **It is not the servo's sensor** — the servo reads `N`, the one-cell step, out of
the converter's own ring, and this pin answers the bring-up window and the fault only.

**The PIN rides this supply on `Quark-Photon`.** Its common cathode takes the `LT3571`'s output
ahead of the SiPM's 2 × 49R9 filter, through **1 MΩ** and **100 nF C0G 100 V** to ground at the
cathode — ~25 V of reverse bias, no second converter, and the board stays under 30 V. The bias sets
the diode's capacitance and nothing else; the threshold that costs is ~260 keV on the quadrant
against the handover's 500 keV (`SCINTILLATION.md`, *The PIN's bias*).

---

*Physics, windows, charge economy, noise budgets, calibration and the isotope reach tables:
[`SCINTILLATION.md`](SCINTILLATION.md). The tube build: [`../../tubes/`](../../tubes/); the neutron physics:
[`../../NEUTRONS.md`](../../NEUTRONS.md).*

★ N.I.C. ★

# Kronos — the hardware

> **Design-stage concept.** The figures are verified against the parts' sheets and the parts
> before a board is made.

```
   IN                                                  OUT
   ─────────────────────────────────────────           ─────────────────────────────────────────

   TIME IN 1, GNSS — a data body                       MCO1 (PA8) ──▶ DS91C176 ──▶ CLK pair, 2²²
     TXD / RXD ─────────▶ UART4 (PA0/PA1)
     PPS ───────────────▶ TIM5 CH3 (PA2)               TIM2 CH1 (PA5) ──▶ DS91C176 ──▶ PPS pair, PPS_K

   TIME IN 2, Pip — a data body                        I2C1 (PB8/PB9) ◀─▶ SDA · SCL, the label,
     TXD / RXD ─────────▶ USART6 (PC6/PC7)                                  multi-master
     PPS ───────────────▶ TIM5 CH4 (PA3)               3V3 ──▶ ATTN

   the oscillator                                      ONE 10-pin time connector, one ribbon:
     TCXO 2²⁴, ±1,0 ppm, CMOS ──▶ HSE bypass (PH0)     GND · CLK+ · CLK− · GND · PPS+ · PPS− ·
     TMP117 beside it on I2C3                          GND · SDA · SCL · ATTN

   supply: 12 V on two terminals ──▶ LMR43610 ──▶ 3,3 V; no rail from any other board
```

## The oscillator

**`MQF574T33-16.777216-1.0/-40+85`, Mercury — CMOS, 3,3 V, 7,0 × 5,0 × 2,5 mm, the one precise
part in the station.** Its value is **2²⁴**, which keeps every ratio on the board a shift. The
order code carries the ±1,0 ppm and the −40…+85 °C, so nothing is specified separately. The grade
buys **holdover**, not accuracy — 1 ppm is 86 ms a day with no source; accuracy is the loop's. It
drives `OSC_IN` directly in HSE bypass, with no shaper in front of it.

- **Pads**, from the sheet's table: 1 not connected · 2 ground · 3 output · 4 `VDD`.
- **Draw: 21 mA typical at 3,3 V running, 18 mA with the output disabled.** The output enable
  only tri-states the output, so the part has no off and none is added; in the lockdown it keeps
  its ~20 mA, 66 mW, nearly all Kronos draws, and its self-heating stays where the holdover table
  learned it.
- **Its supply**: the 3,3 V through its own shielded 2,2 µH, 10 µF + 100 nF at pad 4. Supply-induced
  jitter on this part is picoseconds against a ±1 µs contract; the local filter is the whole
  treatment.

**The thermometer — `TMP117` on I2C3, its own bus.** The ±1 ppm is mostly the part's
temperature coefficient, so the loop files frequency against temperature while it is locked and
applies the table while it is not (`FIRMWARE.md` §5). **It is placed by its coupling to the
TCXO**: as close to the can as the two footprints allow, on the same side, on the same copper
pour, under the same cover, with nothing that dissipates between them. Absolute accuracy does not
matter — the same sensor writes the table and reads it, so a constant offset, the TCXO's own
self-heating included, cancels. Repeatability and coupling are what the part is bought for; a
sensor further away reads the enclosure, and a table of the enclosure's frequency is worth
nothing.

## The PLL

`OSC_IN` → HSE bypass → PLL1:

| | | |
|---|---|---|
| M ÷2 | F_REF | 8,388608 MHz = 2²³ — inside `PLLRGE` 8–16 MHz |
| N ×32 | VCO | 268,435456 MHz = 2²⁸ — inside the wide range's 192–836 MHz |
| **Q ÷64** | **4,194304 MHz = 2²²** | the time bus's clock → MCO1, prescaler 1 — the spur's own rate |
| **P ÷2** | **134,217728 MHz = 2²⁷** | SYSCLK → APB1 → the capture timers |

**FRACN is the trim the loop moves around the exact ratio**, written live with the PLL running:
`PLLFRACEN` cleared, `FRACN[12:0]` written, `PLLFRACEN` set. **One code is 3,81 ppm** (F_REF /
2¹³ = 1024 Hz on the 2²⁸ VCO), coarser than the correction, so the firmware dithers between two
adjacent codes. No DAC and no varactor.

**×32 and not ×64**: ×64 would put the VCO at 2²⁹, 537 MHz — still inside the window, but it
buys only a halved FRACN step — the dither's same one-tick granularity at a write every ~4 ms
instead of ~2, which no loop notices.

**Both taps hang off one VCO**, so a correction moves what leaves the board — the 2²² on the
ribbon and the 2²⁷ that counts the source pulse and places PPS-K. The measurement is taken after
the PLL for that reason: the board steers exactly what it distributes. There is no divider part
on the board and no 2²³ outside the die; 2²³ is the unit the station counts in.

### P ÷2 — the capture grid

The source PPS is one edge a second and the gate is as long as the loop wants, so the frequency
resolution is `tick / gate` and the tick does not set the loop. **It sets the phase term:**

| P ÷ | SYSCLK = TIMxCLK | tick | phase RMS (`tick/√12`) | core | added to a ~20 ns source PPS |
|---|---|---|---|---|---|
| **÷2** | **2²⁷ = 134,217728 MHz** | 7,45 ns | **2,15 ns** | 11,5 mA | **+0,6 %** |
| ÷4 | 2²⁶ = 67,108864 MHz | 14,9 ns | 4,3 ns | 5,8 mA | +2 % |
| ÷8 | 2²⁵ = 33,554432 MHz | 29,8 ns | 8,6 ns | 2,9 mA | +9 % |
| ÷16 | 2²⁴ = 16,777216 MHz | 59,6 ns | 17,2 ns | 1,4 mA | +32 % |

**÷2, because a card ranges its spurs at 2²⁷**: every timing measurement in the station is then
counted on one grid, and a conversion to the 2²³ timebase is a four-bit shift. The cost is ~6 mA
against the TCXO's 20. **÷32** would be a 119 ns tick, 34 ns RMS — coarser than the pulse it
measures.

**The core runs at 2²⁷ because the timers do.** A timer has no kernel-clock mux of its own —
`TIMxCLK` comes off APB and never exceeds HCLK — so HCLK is what the capture needs. **APB1's
prescaler is 1**: a prescaler above 1 doubles `TIMxCLK` back up and lands it on a different
number. 134,217728 MHz is well inside the part's 250 MHz. The saving is in `WFI`, not in the
clock.

## The time bus

**One 10-pin connector, one flat ribbon of 30–40 cm along the card row, an IDC tap pressed on
for every card and one for the Mayak.** It is Kronos's own body, not a Galvani one: the same
`BX2.54-2xNA` header and `FC-10P` socket as the family, 2,54 mm, 2×5, polarised, and nothing
else in the station is 10 pins wide. Non-isolated; it never leaves the box.

| pin | signal | what it is |
|---|---|---|
| 1 | `GND` | the ribbon's edge |
| 2 | `CLK+` | the clock at 2²², M-LVDS |
| 3 | `CLK−` | |
| 4 | `GND` | between the pairs |
| 5 | `PPS+` | `PPS_K`, the derived second, M-LVDS |
| 6 | `PPS−` | |
| 7 | `GND` | |
| 8 | `SDA` | the label bus |
| 9 | `SCL` | |
| 10 | `ATTN` | tied to 3V3 here; a card carries a pull-down, so a card on this ribbon reads high and is a Bifrost, off it low and an Argus |

**A ground around each pair, and both edges of the ribbon are ground.** `CLK` is the one
conductor that switches continuously, so it lies between two grounds; a ground separates the
pairs; the outer conductor at each end is a ground. **No ground goes between the two conductors
of a pair** — lying side by side, coupled alike to everything around them, is their whole virtue.
`SDA`, `SCL` and `ATTN` take no ground of their own: I²C at 100 kHz against a threshold near a
volt, and a level that never switches. **What the grounds buy is the return path**: with the
return beside its signal the loop is ~500 mm² over 40 cm at the 1,27 mm pitch; with one ground at
the far edge of the ribbon it is ~4600 mm².

**On the board the bus grounds are a thin trace and the supply ground a thick one.** The bus
wants a potential from ground, not a path; a thin trace holds the reference and keeps the
supply's return current off it.

### The drivers

**Two `DS91C176TMAX/NOPB`, SOIC-8, M-LVDS, 3,3 V, 200 Mb/s — one per pair, the only
transceivers on this board.** `MCO1` (PA8) drives the CLK pair's `D`; `TIM2_CH1` (PA5) drives the
PPS pair's `D` — a timer edge, never a GPIO write. Each driver's `DE` is on a GPIO, `DE_CLK` PE8
and `DE_PPS` PE10: the cold-stop drops them. `RE#` high, the receiver section off. Pinout
`R · RE# · DE · D · GND · A · B · VCC`.

### The taps

**On every tap a `THVD1450` receiver in the same footprint and pinout, a centimetre or two from
the tap**, `RE#`, `DE` and `D` tied low; the part pulls `RE#` up inside, so an open pin is a
receiver off. **A card and the Mayak take both pairs.** The Mayak's firmware uses the second —
`PPS_K` and the label — and not the clock; the clock is received there for a build that wants it.
The receiver's failsafe reads high on an open or
idle pair, so a missing ribbon reads a defined level and a stopped clock is what a card's CSS
catches.

### The line block, and the termination on the last node

**Every node — Kronos, each card, the Mayak — carries, per pair, 2× 10 Ω in series at the
connector and behind them a position for 80,6 Ω across the pair.** The legs complete the ~100 Ω
the ribbon wants. **The 80,6 Ω is fitted on Kronos always and on the last node of the ribbon by a
jumper on a 2-pin header**; the ribbon ends at that tap and there is no terminator plug. **No
`SM712`**: ~75 pF a line on six taps would drag the clock's edge out to ~20 ns, and nothing here
leaves the box.

**The levels.** At the driver the 20 Ω carry only the branch to the far node, 5,2 mA, and take
**0,104 V** of the headroom. The ribbon holds **526 mV** and the last node's 80,6 Ω **421 mV**,
against the `THVD1450`'s worst-case thresholds — high above −20 mV, low below −200 mV, the offset
being its failsafe: **2,1× at the worst tap for a low**, far more for a high, and **1,6×** at the
driver's minimum `|VAB|` of 480 mV. The driver sees 80,6 Ω ∥ (20 + 100,6) = **48,3 Ω**, the
sheet's own test load of 50 Ω, so its `ICCD` applies as printed.

### The label bus

**`SDA` and `SCL` are I2C1 on PB8/PB9, 100 kHz, multi-master, pulled up to 3,3 V by 2× 4,7 kΩ on
this board and nowhere else.** Kronos writes the label to every card and the head; the Mayak
writes the seed, the position and a remote change, and Kronos answers as a slave; the cards only
listen. Arbitration is the I²C specification's. At ~150 pF — the ribbon and six taps — 4,7 kΩ
rises in ~0,6 µs against standard mode's 1 µs. Kronos's 3,3 V holds through its Stop, so the
pull-ups hold the bus while it sleeps. The I3C peripheral is a drop-in if the bus ever has to be
faster.

### What the bus draws

`DS91C176`: `ICCD` 20 mA typical, 29,5 mA maximum, into 50 Ω. `THVD1450`: 0,70 mA typical,
0,96 mA maximum, receiving. The terminators are the driver's load, not a line of their own.

| | typ | max |
|---|---|---|
| Kronos — two drivers | 40 mA, 132 mW | 59 mA |
| a card — two receivers | 1,4 mA, 4,6 mW | 1,9 mA |
| four cards | 5,6 mA | 7,7 mA |
| the Mayak — two receivers | 1,4 mA, 4,6 mW | 1,9 mA |
| **the whole bus, twelve parts** | **47 mA, 155 mW** | **69 mA, 226 mW** |

Nothing of that sums on one rail: each board pays for its own parts.

## The sockets

**Two receiver sockets, each a data body and nothing else — the station's RX/TX + PPS sockets.**
`TIME IN 1` takes the GNSS receiver — Polaris in the box, or the in-box cable from a Sputnik;
`TIME IN 2` takes Pip, where one is fitted. **Both are the same circuit**: an NMEA stream on a UART
at 115 200 8N1 and a PPS on a capture channel of the one timer.

**The socket sets the direction, and the direction is inward.** `B_DIR` is **tied to ground** in
both sockets, so channel B carries the far end's PPS in; no processor pin reads it. A receiver
cannot be plugged into a clock port and work the wrong way round, and the state where both ends
drive the channel does not exist.

**Both sockets present `GNSS`, 0,35: 5,36 kΩ from `ID_RET` to ground** — the interface, on both
ends of the link. Polaris, a Sputnik's clock run and a Pip's `TIME OUT` put the same code on their own
`ID`. Which receiver is out there is typed into `0x5C SRC_TYPE` and confirmed by the NMEA that
arrives.

**The data body, per socket:**

| pin | signal | on Kronos |
|---|---|---|
| 1 | `CLK/PPS` | the far end's PPS in → the capture pin (PA2 · PA3) |
| 2 | `GND` | |
| 3 | `TXD` | UART out → `$PNIC,HELLO` every second, `$PNIC,RANGE`; the ranging fire |
| 4 | `RXD` | NMEA in; the ranging return |
| 5 | `ID` | the plugged board's resistor, to an ADC pin with 10 kΩ 1 % to 3,3 V |
| 6 | `ID_RET` | 5,36 kΩ to ground — `GNSS` |
| 7 | `RXD_ECHO` | not connected — the echo check is the far end's |
| 8 | `DE` | GPIO, the communication board's driver enable |
| 9 | `B_DIR` | **tied to ground** — inward |
| 10 | `LINE_EN` | GPIO — the communication board's line side; boots low |
| 11 | `SD` | GPIO in, the optical module's no-light alarm, high on loss; 100 kΩ to ground |
| 12 | `3,3 V` | the board's 3,3 V |

**No power body.** Nothing on a `TIME IN` is fed from here: Polaris takes its 3,3 V off the data body,
and a Pip, or a Sputnik on a run, is fed by its own NodBus run.

**Ranging.** Where the receiver is remoted on a Galvani link, Kronos ranges the run the way a
card ranges a spur: the socket's pins switch from UART to timer for the measuring instant, the
edge leaves at a compare value and the return lands on a capture — `TIM5_CH1`/`CH2` on `TIME IN 1`,
`TIM3_CH1`/`CH2` on `TIME IN 2` — with no CPU in the path. A timer channel on the 2²⁷ grid has no
distance limit.

## The supply

**12 V in on two two-pole `DGPS2.5R-5.0` terminals, an input and a tap, the same node** — the
battery wire through Kronos's own fast fuse in the enclosure's fuse field, sized by the construction (`../daedalus/CONSTRUCTION.md`), and one
**`5.0SMDJ14A`** across the terminals: a reversed pack drives it forward and the fuse clears, an
overvoltage it holds for long enough clears the fuse the same way (`../galvani/README.md`, *The
rails*). **One rail, 3,3 V, off the house buck cell**; no rail comes from the Mayak or a card.

**`LMR43610R3RPER`**: `RT` 7,50 kΩ → 2,08 MHz, auto mode by the `R3` order code; 4,7 µH
shielded, `I_SAT` ≥ 3,5 A; `R_FBT` 28,0 kΩ / `R_FBB` 12,1 kΩ → 3,31 V, `C_FF` 22 pF C0G across
`R_FBT`; `C_IN` 4,7 µF 50 V + 100 nF at `VIN`; `C_OUT` 3× 10 µF 1206 + 100 nF; `VCC` 1 µF;
`BOOT` 100 nF to `SW`; `PGOOD` to PD14 through 100 kΩ to 3,3 V. The 12 V node carries 47 µF 50 V
hybrid polymer + 10 µF 50 V 1206 + 100 nF at the terminals.

**What the board draws**, typical: the TCXO 21 mA, the core at 2²⁷ ~11,5 mA and its peripherals,
the two drivers 40 mA, the `TMP117` microamps — **~80 mA at 3,3 V, ~0,26 W**, ~25 mA from the
12 V.

## The processor — pins, timers, clock tree

**`STM32H523VE`, LQFP100.** Pin numbers and alternate functions from DS14540 Rev 3, Table 13.
Every pin is assigned or listed free.

### Peripherals

| peripheral | instance | pins | job | mode |
|---|---|---|---|---|
| `TIME IN 1` | **UART4** + **TIM5** (32-bit) | PA0 `TXD` · PA1 `RXD` · PA2 `PPS` | NMEA at 115 200 8N1; the PPS on channel B inward; `$PNIC,HELLO` out every second | CH1/CH2 range the run; CH3 captures the PPS |
| `TIME IN 2` | **USART6** + **TIM3** · **TIM5** CH4 | PC6 `TXD` · PC7 `RXD` · PA3 `PPS` | the same | TIM3 CH1/CH2 range the run; TIM5 CH4 captures the PPS |
| PPS-K | **TIM2** (32-bit), slaved to TIM5 | PA5 on CH1 | the derived second onto the PPS pair's driver | output compare, period exactly 2²⁷ ticks |
| the clock out | **MCO1** ← PLL1_Q | PA8 | 2²² onto the CLK pair's driver | prescaler 1 |
| the label | **I2C1**, multi-master | PB8 `SCL` · PB9 `SDA` | the label out; the seed, the position and the registers | 100 kHz, own address 0x3C, wake on address match |
| the thermometer | **I2C3**, master | PD6 `SCL` · PD7 `SDA` | the `TMP117` beside the TCXO | 100 kHz, its own bus |
| `ID` reads | **ADC1** | PC0 · PC2 | one resistor per data body, against 10 kΩ | once at boot |
| clock in | **HSE bypass** | PH0 | the TCXO | PH1 unconnected |
| debug | **SWD** | PA13 · PA14 | | no JTAG, no trace |

**Serial ports: two of seven**, UART4 and USART6. The clock and PPS-K are M-LVDS pairs and the
label is I²C, so none of them spends a serial port.

### Pins

| pin | port | signal on the board | AF / function | dir | note |
|---|---|---|---|---|---|
| 1 | PE2 | `DE_1` | GPIO | out | `TIME IN 1`, data body |
| 2 | PE3 | `LINE_EN_1` | GPIO | out | `TIME IN 1`, data body; boots low |
| 3 | PE4 | `SD_1` | GPIO | in | 100 kΩ to ground |
| 4 | PE5 | `DE_2` | GPIO | out | `TIME IN 2`, data body |
| 5 | PE6 | `LINE_EN_2` | GPIO | out | `TIME IN 2`, data body; boots low |
| 6 | VBAT | — | | | tied to `VDD`; no RTC |
| 7 | PC13 | — | | | free |
| 8 | PC14 | — | | | free — no 32 kHz crystal |
| 9 | PC15 | — | | | free |
| 10 | VSS | | | | |
| 11 | VDD | 3,3 V | | | |
| 12 | PH0 | `TCXO` | `OSC_IN`, bypass | in | the TCXO's output, 2²⁴ |
| 13 | PH1 | — | | | unconnected in bypass |
| 14 | NRST | reset | | | 100 nF, the internal pull-up |
| 15 | PC0 | `ID_1D` | `ADC12_INP10` | in | `TIME IN 1`, data body |
| 16 | PC1 | — | | | free |
| 17 | PC2 | `ID_2D` | `ADC12_INP12` | in | `TIME IN 2`, data body |
| 18 | PC3 | — | | | free |
| 19 | VSSA | | | | |
| 20 | VREF− | | | | |
| 21 | VREF+ | 3,3 V through the shielded 2,2 µH, 10 µF + 100 nF at the pins | | | the `ID` reference |
| 22 | VDDA | 3,3 V through the shielded 2,2 µH, 10 µF + 100 nF at the pins | | | |
| 23 | PA0 | `TXD_1` | `UART4_TX` / `TIM5_CH1` | out | `TIME IN 1`; the pin switches to the timer for the ranging instant |
| 24 | PA1 | `RXD_1` | `UART4_RX` / `TIM5_CH2` | in | |
| 25 | PA2 | `PPS_1` | `TIM5_CH3` capture | in | `TIME IN 1`'s `CLK/PPS`, channel B inward |
| 26 | PA3 | `PPS_2` | `TIM5_CH4` capture | in | `TIME IN 2`'s `CLK/PPS` |
| 27 | VSS | | | | |
| 28 | VDD | 3,3 V | | | |
| 29 | PA4 | — | | | free (ADC) |
| 30 | PA5 | `PPS_K` | `TIM2_CH1` output compare | out | into the PPS pair's `DS91C176` |
| 31 | PA6 | — | | | free (ADC) |
| 32 | PA7 | — | | | free (ADC) |
| 33 | PC4 | — | | | free (ADC) |
| 34 | PC5 | — | | | free (ADC) |
| 35 | PB0 | — | | | free (ADC) |
| 36 | PB1 | — | | | free (ADC) |
| 37 | PB2 | — | | | free |
| 38 | PE7 | `SD_2` | GPIO | in | 100 kΩ to ground |
| 39 | PE8 | `DE_CLK` | GPIO | out | the CLK pair's driver enable |
| 40 | PE9 | — | | | free |
| 41 | PE10 | `DE_PPS` | GPIO | out | the PPS pair's driver enable |
| 42 | PE11 | — | | | free |
| 43 | PE12 | — | | | free |
| 44 | PE13 | — | | | free |
| 45 | PE14 | — | | | free |
| 46 | PE15 | — | | | free |
| 47 | PB10 | — | | | free |
| 48 | VCAP | 2× 1 µF 50 V 0805 + 2× 100 nF 50 V 0603 | | | 2,2 µF, the sheet's 2,2 µF ±20 % |
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
| 61 | PD14 | `PGOOD` | GPIO | in | the `LMR43610`'s window |
| 62 | PD15 | — | | | free |
| 63 | PC6 | `TXD_2` | `USART6_TX` / `TIM3_CH1` | out | `TIME IN 2` |
| 64 | PC7 | `RXD_2` | `USART6_RX` / `TIM3_CH2` | in | |
| 65 | PC8 | — | | | free |
| 66 | PC9 | — | | | free |
| 67 | PA8 | `CLK_OUT` | `MCO1` ← PLL1_Q | out | 2²² into the CLK pair's `DS91C176` |
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
| 84 | PD3 | — | | | free |
| 85 | PD4 | `LED` | GPIO | out | the status LED, 1 kΩ from the 3,3 V — one 50 ms blink a minute while healthy, two on a fault |
| 86 | PD5 | — | | | free |
| 87 | PD6 | `SCL_T` | `I2C3_SCL` | i/o | the `TMP117` |
| 88 | PD7 | `SDA_T` | `I2C3_SDA` | i/o | |
| 89 | PB3 | — | | | free |
| 90 | PB4 | — | | | free |
| 91 | PB5 | — | | | free |
| 92 | PB6 | — | | | free |
| 93 | PB7 | — | | | free |
| 94 | BOOT0 | 10 kΩ to ground | | | |
| 95 | PB8 | `SCL_TB` | `I2C1_SCL` | i/o | the time bus's label |
| 96 | PB9 | `SDA_TB` | `I2C1_SDA` | i/o | |
| 97 | PE0 | — | | | free |
| 98 | VCAP | 2× 1 µF 50 V 0805 + 2× 100 nF 50 V 0603 | | | |
| 99 | VSS | | | | |
| 100 | VDD | 3,3 V | | | |

**27 GPIO used, 52 free, `PH1` unconnected** (PA4 · PA6 · PA7 · PA9–PA12 · PA15 · PB0–PB7 · PB10 · PB12–PB15 · PC1 ·
PC3–PC5 · PC8–PC15 · PD0–PD3 · PD5 · PD8–PD13 · PD15 · PE0 · PE9 · PE11–PE15).

### Timers

| timer | width | clock | channels used | role |
|---|---|---|---|---|
| **TIM5** | 32 | 2²⁷ | CH1 compare → PA0, CH2 capture ← PA1, CH3 capture ← PA2, CH4 capture ← PA3 | the capture timer: both source pulses on one free-running counter, so comparing them is a subtraction; the ranging of `TIME IN 1` on CH1/CH2 |
| **TIM2** | 32 | 2²⁷ | CH1 compare → PA5 | **PPS-K**: period 2²⁷, slaved to TIM5's trigger so the two counters share one phase and a pulse captured on TIM5 places PPS-K on TIM2 by arithmetic |
| **TIM3** | 16 | 2²⁷ | CH1 → PC6, CH2 ← PC7 | the ranging of `TIME IN 2` |
| **TIM6** | 16 | 2²⁷ | no pin | the dither: an interrupt every 2 ms, stepping `FRACN` between two codes |
| TIM1 · TIM4 · TIM7 · TIM8 · TIM12 · TIM15 · LPTIM1 · LPTIM2 | | | | free |

A pulse is read as the difference between two captures a second apart, so the 32-bit wrap never
comes up and nothing is chained.

### Clock tree

| | |
|---|---|
| `OSC_IN` | **2²⁴ = 16,777216 MHz**, the TCXO |
| HSE | bypass; the CSS armed |
| PLL1 | M 2 · N 32 · P 2 · Q 64 → F_REF 2²³, VCO 2²⁸, `P` **2²⁷**, `Q` **2²²** |
| SYSCLK, AHB, APB1, APB2, timer kernel | **2²⁷ = 134,217728 MHz**, every prescaler 1 |
| MCO1 | `MCO1SEL` = PLL1_Q, `MCO1PRE` = 1 → **2²²** on PA8 |
| the discipline | `FRACN[12:0]` rewritten live around the exact ratio |

### Analogue and reference

**Every filter inductor on this board is a π filter: 10 µF 1206 + 100 nF 0603 before it on the rail and
the same pair at the pins behind it** — the house rule (`../core/POWER.md`, *The filter parts*).

`VDDA` and `VREF+` off the 3,3 V through the shielded 2,2 µH `SWPA252012S2R2MT` each, 10 µF + 100 nF at the pins; `VREF−`
and `VSSA` to the ground plane at one point. The two `ID` inputs are read single-ended against
`VREF+`, and each 10 kΩ pull-up hangs on the same 3,3 V, so the reading is a ratio and the rail's
tolerance drops out.

## The parts

| part | qty | where |
|---|---|---|
| `STM32H523VE`, LQFP100 | 1 | the processor |
| `MQF574T33-16.777216-1.0/-40+85`, Mercury | 1 | the TCXO |
| `TMP117` | 1 | beside the TCXO, I2C3 |
| `DS91C176TMAX/NOPB`, SOIC-8 | 2 | the time bus's drivers, CLK and PPS |
| `LMR43610R3RPER` | 1 | 12 V → 3,3 V |
| 4,7 µH shielded, `I_SAT` ≥ 3,5 A | 1 | the buck |
| `BX2.54-2xNA`, 2×5 | 1 | the time connector |
| `BX2.54-2xNA`, 2×6 | 2 | `TIME IN 1`, `TIME IN 2` |
| `DGPS2.5R-5.0`, two-pole | 2 | the 12 V, input and tap |
| `5.0SMDJ14A` | 1 | across the 12 V input |
| 10 Ω, 1 % | 4 | the line block, 2 per pair |
| 80,6 Ω, 1 % | 2 | the termination, fitted — Kronos is always an end |
| 5,36 kΩ, 1 % | 2 | `ID_RET`, `GNSS`, one per socket |
| 10 kΩ, 1 % | 2 | the `ID` pull-ups |
| 100 kΩ | 2 | `SD_1`, `SD_2` to ground |
| 4,7 kΩ | 4 | I2C1 (the label bus), I2C3 — a pair each |
| 10 kΩ | 1 | `BOOT0` to ground |
| 1 kΩ + LED | 1 | the status LED |
| 7,50 kΩ · 28,0 kΩ · 12,1 kΩ, 1 % | 1 each | the buck's `RT` and divider |
| 100 kΩ | 1 | `PGOOD` pull-up |
| 22 pF C0G | 1 | `C_FF` |
| 2,2 µH shielded, `SWPA252012S2R2MT` | 3 | `VDDA`, `VREF+`, the TCXO |
| 47 µF 50 V hybrid polymer | 1 | the 12 V node |
| 10 µF 50 V 1206 | 1 | the 12 V node |
| 4,7 µF 50 V | 1 | the buck's `C_IN` |
| 10 µF 1206 | 3 | the buck's `C_OUT` |
| 10 µF | 4 | the processor's bulk, `VDDA`, `VREF+`, the TCXO |
| 1 µF 50 V 0805 | 4 | `VCAP`, two on each |
| 1 µF | 1 | the buck's `VCC` |
| 100 nF 50 V X7R | ~20 | every `VDD` of the H523, `VDDA`, `VREF+`, `VCAP` two on each, each driver, the `TMP117`, the TCXO, `BOOT`, `NRST`, the buck's input and output, the 12 V node |

Every capacitor below the buck is 50 V X7R; `C_FF` is C0G.

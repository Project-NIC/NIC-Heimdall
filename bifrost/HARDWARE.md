★ N.I.C. ★

# Bifrost — the hardware

> **Design-stage concept.** The figures are verified against the parts' sheets and the parts
> before a board is made.

**Bifrost and Argus are one board, the card, under one firmware.**

## The card, wired

```
   FROM KRONOS — a tap on the 10-pin time bus ribbon: GND · CLK± · GND · PPS± · GND ·
                 SDA · SCL · ATTN — a ground around each pair;
   per pair 2× 10 Ω in series at the connector, a jumpered 80,6 Ω behind them (fitted on the
   last node of the ribbon only), two THVD1450 receivers a centimetre from the tap,
   RE#, DE and D tied low; no SM712 here


      PPS_K ─────────────────────────▶  a capture input
                                        marks the second's boundary, in hardware

      CLK, 2²² ──────────────────────▶  HSE, through a 74AUP1G126 that ATTN enables;
                                        a 74AUP1G125 on the same pin carries the
                                        upstream spur's CLK, which an Argus lives on. ×32 to
                                        the 2²⁷ the card measures on; MCO1 hands the same
                                        2²² to every spur, undivided

      SDA · SCL, the time bus ───────▶  a slave port
                                        names the second the edge just marked

      ATTN ──────────────────────────▶  a GPIO with a pull-down
                                        3V3 on Kronos: high = a Kronos is plugged in
                                        = this card is a BIFROST. Nothing plugged in =
                                        low = this card is an ARGUS. One firmware.

                                        │
                                        ▼
                    ┌─────────────────────────────────────────┐
                    │                                         │
                    │   THE CARD — Bifrost or Argus           │──────▶  port 1
                    │   STM32H523, LQFP100                    │
                    │   6 USARTs of 7: 1 up + its echo, 4 down│──────▶  port 2
   MAYAK  ═════════▶│                                         │
                    │   per port:                             │──────▶  port 3
   the trunk        │     BUSCFG · SYNC · enrollment          │
   one card, one    │     the ranging measurement             │──────▶  port 4
   USART, and that  │                                         │
   IS the card's    │   per unit:                             │
   identity         │     a circular buffer, then the stamp   │
                    │                                         │
                    │   12 V off the wire on two terminals,   │
                    │   none on a port;                       │
                    │   three bucks make the 3,3 V            │
                    │                                         │
                    └─────────────────────────────────────────┘

     ONE CARD PER HEAD TRUNK, up to four, and at most eight units on any
     one of them. Every port position is TWO bodies, a data body for the
     communication board and a power body for the power board; the up port's
     data body carries the trunk over a crossed cable. No 12 V runs across the
     card from its input to any port.

     THE SAME BOARD IS AN ARGUS. The time tap is a body of its own beside the
     upstream connector; an Argus is not pressed on Kronos's ribbon, ATTN reads low,
     and the card takes its clock from channel B of its spur and its time from
     the SYNC header like any unit. That level is the whole difference.

     No line transceiver on this card — the two THVD1450s only receive the
     time bus: the trunk is plain logic to the head, and 485 begins on a
     Galvani board or not at all.
```

**Four ports down and one up, and the card reads what is plugged into each before anything is
powered.** The connector's `ID` is a resistor on the plugged board, read on an ADC pin, so which
Galvani board, which host across an in-box cable, and an empty socket are all different values
with nothing energised (`../galvani/README.md`, *The connectors*). What follows the run is which
boards are fitted: the **485** communication board on copper, **10 km** or **2 Mb/s** on glass,
a **power board** beside it, or the **crossed cable** and no boards at all. The card's own
`ID_RET` on its up port carries its code, **0,15 — 1,78 kΩ**, for the host across such a cable;
its down ports carry none.

## The GPIO budget — LQFP100 holds it

| | pins | what |
|---|---|---|
| four ports down | 4 × 7 = 28 | `TXD` · `RXD` · `DE` · `ID` (ADC) on the data body · `ID` (ADC) on the power body · `SD` · `EN` — one GPIO a port for `LINE_EN` and `ENABLE`; `RXD_ECHO` and `ID_RET` unconnected on a down port |
| the port clock | 5 | `MCO1` on PA8, one output into the quad buffer for all four ports, and **an `OE` a port**, the gate's own GPIO — PE9 · PB14 · PB15 · PD8 |
| the ranging captures | 0 | one timer per port, the capture on the `RXD` pin's own alternate function and the return on `TXD`'s — no extra pin; the five pairs below |
| the port up | 7 | `TXD` · `RXD` · `RXD_ECHO` (its own USART, the sixth) · `DE` · `ID` (ADC) on the data body · `ID` (ADC) on the power body · `SD`; `LINE_EN` is 10 kΩ to 3,3 V in the socket and `ENABLE` is not connected — the card never switches its own up link. `RXD`/`TXD` on channels of one timer, for the return when the card is a unit (an Argus behind a Bifrost) |
| the time bus tap | 4 | `SDA` · `SCL` · `ATTN` · `PPS_K` capture behind its `THVD1450` receiver |
| the clock in | 1 | `OSC_IN` (PH0), the joined outputs of the two clock buffers |
| the power boards, on the port bodies | **11** | **three controllers** — 6 for `SDA`/`SCL` (I2C3 PD6/PD7 · I3C1 PD12/PD13 · I3C2 PC10/PC11) — and **5 `ALERT`s, a pin per port**: PC8 · PD4 · PD5 · PD10 · PD11 |
| the down ports' buck | 1 | `PGOOD` of the `LMR43620` |
| SWD | 2 | `SWDIO` · `SWCLK`; `NRST` and `BOOT0` are dedicated pins |
| **total** | **59** | 20 pins free, `PH1` unconnected — the pin table below |

**One clock in, two doors, and `ATTN` opens one of them.** The time bus's `CLK`, behind its
receiver, and the upstream connector's `CLK` each reach `OSC_IN` through **a single 3-state
buffer**, and the two outputs are joined on the pin:

```
  CLK, time bus (2²²) ──▶ 74AUP1G126 ──┐   OE active HIGH
                            OE ◀─ ATTN  │
                                         ├──▶ OSC_IN (PH0, pin 12)
  CLK, up port   (2²²) ──▶ 74AUP1G125 ──┘   OE active LOW
                            OE ◀─ ATTN
```

**One level, two opposite enables, so exactly one buffer drives and never both** — a Bifrost
(`ATTN` high) passes the time bus, an Argus (`ATTN` low) the up port. It is the wiring that does it,
not the firmware: `ATTN` is tied to 3,3 V on Kronos and pulled down on the card, so it is a defined
level from the moment there is power, and the joined node needs no resistor. **The unselected
source is not looked at at all** — both parts disable their input with their output — so the
level a receiver falls to on an empty tap does not matter — which is what lets the tap carry a
`THVD1450`, whose failsafe reads high. The two sit beside pin 12: millimetres into `OSC_IN`, and the long runs from the
connectors end on inputs of **0,9 pF**.

**A buffer isolates the source and redrives a fresh edge at the pin. Why this family**: `CI` 0,9 pF, `CO` 1,7 pF, `CPD` 4,2 pF at 3,0–3,6 V against the
`74LVC1G` parts' 4 pF and ~19 pF, Schmitt inputs, `IOFF` for partial power-down, and
`tpd` **2,4 ns typical, 3,9 ns maximum at 5 pF, 5,4 ns at 15 pF** over −40…+85 °C (Nexperia,
`74AUP1G125` Rev. 11, `74AUP1G126` Rev. 10). The output is rated ±4 mA, which does not bind:
`OSC_IN` is a CMOS input with no DC load, so the high level sits at the rail against the
**0,7 V_DD** the H523 asks of a bypass clock. The active buffer costs ~0,3 mW at 2²².

**Both roles take 2²² and lock the same PLL, ×32 to 2²⁷**; what differs by role is the source the
`ATTN` level selects ahead of `OSC_IN` and `MCO1`'s prescaler behind it — and nothing else. The buffer's propagation
is fixed, a few nanoseconds, on the clock and never on the second: PPS-K reaches its own capture
pin, and **what the clock's edge carries into a port is inside what that port's ranging
measures**; nothing engineers it away.

**ONE clock output, ONE quad buffer, four sockets — and every port of a card carries the same
clock.** `MCO1` on PA8, sourced from HSE, puts the clock that arrived back out — **prescaler 1 on a
Bifrost, so every spur carries Kronos's own 2²² edge; 8 on an Argus** — and **drives all four gates
of the buffer, their inputs strapped together on the board**; each gate feeds one socket, so a
shorted port costs a gate and not the pin. No timer is spent on it. One package, one net in, four
sockets out, identical, and **each gate's `OE` on a GPIO of its own**, so a port's clock is switched
without its feed.

**The buffer is an `SN74LVC126A`** — quadruple bus buffer gate, 3-state outputs, the house `74LVC`
family (SCAS339U Rev U, July 2024):

| | |
|---|---|
| **why this one and not the `125A`** | **its `OE` is active HIGH** — *"each output is disabled when the associated output-enable (`OE`) input is low"* — so each gate's `OE` sits **on a GPIO of its own, `OE_1`–`OE_4`** (`PE9` · `PB14` · `PB15` · `PD8`), low from reset and held there by its pull-down. The same silicon as the `125A`, whose `OE` is active low; the delay is identical |
| what that buys | **a port's clock is switched on its own** — shut for `PORT_CLK` and while a port is probing with its feed up, shut with the feed for `PORT_PWR` — and a shorted port is killed clock-side by firmware |
| **four pull-downs, and they are not optional** | the sheet: *"to ensure the high-impedance state during power up or power down, `OE` must be tied to GND through a pulldown resistor"*. **100 kΩ from each `OE` to GND** — the house's one pull value; the H523's GPIO sources far more than enough to pull them up against that, and until the processor drives them the ports are Hi-Z rather than beating |
| supply | 1,65–3,6 V; on the card's own 3,3 V, **0,1 µF at the `VCC` pin** as the sheet asks |
| **temperature** | **−40 to +125 °C specified** — the station wants −40…+85, so it is inside with 40 K of margin |
| **delay** | `t_pd` at 3,3 V ± 0,3 V: **2,5 ns typical, 4,5 ns max at 25 °C, 4,7 ns max over −40…+85 °C**. `t_en` from `OE`: 2,5 ns typ, 5,7 ns max over the same range. A port's own ranging measures it out |
| drive | ±24 mA at 3 V, `V_OL` 0,55 V there at 25 °C — **and the sheet's own load rule is ≤ 25 mA an output and ≤ 50 mA for the whole part**, which a clock line into one connector never approaches. It is rated for use to **100 MHz**; 2²² is 4 % of that |
| what it costs the rail | `C_pd` 22 pF a gate enabled at 3,3 V (4 pF disabled), so with ~30 pF of socket and cable on each output, `(22 + 30) pF × 3,3 V × 4,194304 MHz` = **~0,7 mA a port, ~2,9 mA for all four**. Quiescent `I_CC` is 10 µA max at 85 °C |
| inputs | 5,5 V tolerant, `C_i` 4,5 pF. **All four are used** — they are strapped to the one `MCO1` net — so the sheet's never-float rule is met by construction |
| package | **`RGY` VQFN 3,5 × 3,5 mm** the house choice; `BQA` WQFN 3 × 2,5, `PW` TSSOP and `D` SOIC-14 all exist if a build wants a hand-solderable part |

**It sits on the card's own 3,3 V, not on the ports' `LMR43620` branch**, and that does not defeat
the branch split (*The card's supply*, below): a shorted `CLK` pin lands on **the buffer's output**, where
the part's own drive limit is the whole exposure and `OE` takes it off. That is a second reason the
gate is there at all and the connectors are not driven from a processor pin.

**The number is the card's ROLE, and `ATTN` already names it** — the same pin that picks the
source on the way in picks the rung on the way out:

| role | what the ports carry | `MCO1`, off HSE's 2²² |
|---|---|---|
| **Bifrost** | **2²² = 4 194 304 Hz** — a 40 B unit's reception chain is built for it | prescaler **1** — Kronos's own edge |
| **Argus** | **2¹⁹ = 524 288 Hz** — no mini-NOD wants more | prescaler **8** |

Both are exact, and the card needs no table, no `ID` read and no decision: the level on one pin at
boot sets the source, the prescaler, and which of the two the card is.

**An Argus never runs its segments faster and a Bifrost never runs its ports slower.** The unit
behind a 40 B port takes the sync onto **HSE** and its whole reception chain is built for 2²²; a
mini-NOD takes it onto a **timer's external clock input** and divides to its grid by whole powers
of two, where a lower rung costs no accuracy at all — the error is the edge, ns-class and ranged
out. That is why one number a card is enough and why the low one is the Argus's:
`÷4096` of 2¹⁹ is the 128 Hz frame that `Quark-Tubes`' bins are already written on
(`../quark/README.md`), and the 2 Mb/s glass a mini segment may sit on could not
carry 2²² in any case.

## The processor — pins, timers, clock tree

*The record to draw the H523 from and to set CubeMX by. Package LQFP100, `STM32H523VE`; pin
numbers and alternate functions from DS14540 Rev 3, Table 13. Every pin below is either
assigned or listed free — nothing is left to guess.*

### Peripherals

| peripheral | instance | pins | job | mode |
|---|---|---|---|---|
| port 1 down | **USART1** + **TIM4** | PB6 `TXD` · PB7 `RXD` | NodBus spur, 8N1 at the port's data rung, 2²⁰ or 2²¹ | the pins switch USART → TIM4 CH1/CH2 for the ranging instant |
| port 2 down | **USART2** + **TIM15** | PA2 `TXD` · PA3 `RXD` | same | same, TIM15 CH1/CH2 |
| port 3 down | **UART5** + **TIM8**, `SWAP` set | PB12 `TXD` (TIM8_CH3) · PB13 `RXD` (TIM8_CH2) | same | the USART's `SWAP` bit puts the receiver on the trigger-capable channel |
| port 4 down | **USART6** + **TIM3** | PC6 `TXD` · PC7 `RXD` | same | TIM3 CH1/CH2 |
| port up | **UART4** + **TIM5** (32-bit) | PA0 `TXD` · PA1 `RXD` | the trunk to the Mayak, or an Argus's NodBus link | TIM5 CH1/CH2 — the turnaround when the card is a unit |
| echo receiver | **USART3**, RX only | PD9 `RXD_ECHO` | hears the card's own frames on the up port | no timer — it ranges nothing |
| timebase | **TIM2** (32-bit) | PA15 `PPS_K` capture on CH1 | free-running at 2²⁷; CH2–CH4 internal compares for the frame grid | never reset, read as differences |
| the port clock | **MCO1** ← HSE | PA8; the gates' `OE_1`–`OE_4` on PE9 · PB14 · PB15 · PD8 | **one output for all four ports**, into the quad buffer's strapped inputs; the number is the card's role — **2²² on a Bifrost, 2¹⁹ on an Argus**, named by `ATTN`; each port's gate opened by its own `OE` | `MCO1SEL` = HSE, `MCO1PRE` **1** or **8** |
| time bus label | **I2C1**, multi-master | PB8 `SCL` · PB9 `SDA` | Kronos's coarse second and quality, the Mayak's seed | 100 kHz |
| power boards | **I2C3 · I3C1 · I3C2**, master on each — the label keeps I2C1 | I2C3 PD6 `SCL` · PD7 `SDA` · **I3C1 PD12 `SCL` · PD13 `SDA`** · **I3C2 PC10 `SCL` · PC11 `SDA`**, the two I3C in I²C legacy | the five ports' `INA238`s — **at most two sockets to a controller, because `A_SEL` is one bit and two boards on a bus are 0x40 and 0x41**: the up port and down port 1 on I3C1, down ports 2 and 3 on I2C3, down port 4 alone on I3C2. Each port's `A_SEL` is strapped in its socket, the first of a pair low and the second high (`../galvani/README.md`). **Not I2C2**: its only `SCL` pad is PB10 and its usable `SDA` pads are PB12, which is port 3's `TXD`, and PB3, which is `JTDO/TRACESWO` — the I3C pairs cost neither 100 kHz; **4,7 kΩ to 3,3 V on `SDA` and `SCL` of each of the three pairs**; **an `ALERT` pin per port, five of them, no wire-OR, each 100 kΩ to ground**: `ALERT_0` PC8 for the up port, `ALERT_1`–`ALERT_4` on PD4 · PD5 · PD10 · PD11 — **EXTI 8 · 4 · 5 · 10 · 11, five different numbers**, EXTI being shared by pin number across ports. Each rides its board's `ISO1642` channel B and is **high on alarm**, so a dark port reads quiet. A stuck `ALERT` on a wire-OR blinds every other port; a pin each makes it one board's fault |
| ID reads | **ADC1** | ten channels, a data-body and a power-body `ID` per port, below | one resistor per body, against 10 kΩ | read once at bring-up, before a port is fed |
| clock in | **HSE bypass** | PH0 `OSC_IN` | the two joined 3-state buffers, one enabled by `ATTN`: 2²² either way — the time bus's (Bifrost) or the up port's (Argus) | PH1 unconnected |
| debug | **SWD** | PA13 `SWDIO` · PA14 `SWCLK` | | no JTAG, no trace |

### Pins

| pin | port | signal on the board | AF / function | dir | note |
|---|---|---|---|---|---|
| 1 | PE2 | `DE_1` | GPIO | out | port 1 driver enable |
| 2 | PE3 | `EN_1` | GPIO | out | port 1 `ENABLE` + `LINE_EN`, one net; 100 kΩ down |
| 3 | PE4 | `SD_1` | GPIO | in | port 1 optical no-light |
| 4 | PE5 | `DE_2` | GPIO | out | |
| 5 | PE6 | `EN_2` | GPIO | out | port 2 `ENABLE` + `LINE_EN`; 100 kΩ down |
| 6 | VBAT | — | | | tied to `VDD`; no RTC anywhere |
| 7 | PC13 | — | | | free (tamper/RTC pin, unused) |
| 8 | PC14 | — | | | free — no 32 kHz crystal |
| 9 | PC15 | — | | | free |
| 10 | VSS | | | | |
| 11 | VDD | 3,3 V | | | |
| 12 | PH0 | `CLK_IN` | `OSC_IN`, bypass | in | the joined outputs of `74AUP1G126` (time bus) and `74AUP1G125` (up port), 2²² from either |
| 13 | PH1 | — | | | unconnected in bypass mode |
| 14 | NRST | reset | | | 100 nF, no pull beyond the internal |
| 15 | PC0 | `ID_1D` | `ADC12_INP10` | in | port 1 data body `ID` |
| 16 | PC1 | `ID_1P` | `ADC12_INP11` | in | port 1 power body `ID` |
| 17 | PC2 | `ID_2D` | `ADC12_INP12` | in | |
| 18 | PC3 | `ID_2P` | `ADC12_INP13` | in | |
| 19 | VSSA | | | | |
| 20 | VREF− | | | | |
| 21 | VREF+ | 3,3 V through the shielded 2,2 µH, 10 µF + 100 nF at the pins | | | the `ID` reference — the same rail the 10 kΩ pull-ups hang on, so the ratio is supply-free |
| 22 | VDDA | 3,3 V through the shielded 2,2 µH, 10 µF + 100 nF at the pins | | | |
| 23 | PA0 | `TXD_UP` | `UART4_TX` / `TIM5_CH1` | out | the port up |
| 24 | PA1 | `RXD_UP` | `UART4_RX` / `TIM5_CH2` | in | |
| 25 | PA2 | `TXD_2` | `USART2_TX` / `TIM15_CH1` | out | |
| 26 | PA3 | `RXD_2` | `USART2_RX` / `TIM15_CH2` | in | |
| 27 | VSS | | | | |
| 28 | VDD | 3,3 V | | | |
| 29 | PA4 | — | | | free (ADC, DAC) |
| 30 | PA5 | — | | | free (ADC) |
| 31 | PA6 | `ID_4D` | `ADC12_INP3` | in | |
| 32 | PA7 | `ID_4P` | `ADC12_INP7` | in | |
| 33 | PC4 | `ID_3D` | `ADC12_INP4` | in | |
| 34 | PC5 | `ID_3P` | `ADC12_INP8` | in | |
| 35 | PB0 | `ID_UPD` | `ADC12_INP9` | in | the port up, data body |
| 36 | PB1 | `ID_UPP` | `ADC12_INP5` | in | the port up, power body |
| 37 | PB2 | — | | | free |
| 38 | PE7 | `SD_2` | GPIO | in | |
| 39 | PE8 | `DE_3` | GPIO | out | |
| 40 | PE9 | `OE_1` | GPIO | out | port 1's clock gate, the buffer's `1OE`; 100 kΩ down |
| 41 | PE10 | `EN_3` | GPIO | out | port 3 `ENABLE` + `LINE_EN`; 100 kΩ down |
| 42 | PE11 | `SD_3` | GPIO | in | |
| 43 | PE12 | `DE_4` | GPIO | out | |
| 44 | PE13 | `EN_4` | GPIO | out | port 4 `ENABLE` + `LINE_EN`; 100 kΩ down |
| 45 | PE14 | `SD_4` | GPIO | in | |
| 46 | PE15 | `DE_UP` | GPIO | out | |
| 47 | PB10 | — | | | free |
| 48 | VCAP | 2× 1 µF 50 V 0805 + 2× 100 nF 50 V 0603 — 2,2 µF, the sheet's 2,2 µF ±20 % | | | |
| 49 | VSS | | | | |
| 50 | VDD | 3,3 V | | | |
| 51 | PB12 | `TXD_3` | `UART5_TX` (swapped) / `TIM8_CH3` | out | port 3; `SWAP` makes this pin the transmitter |
| 52 | PB13 | `RXD_3` | `UART5_RX` (swapped) / `TIM8_CH2` | in | the receiver on the trigger-capable channel |
| 53 | PB14 | `OE_2` | GPIO | out | port 2's clock gate, `2OE`; 100 kΩ down |
| 54 | PB15 | `OE_3` | GPIO | out | port 3's clock gate, `3OE`; 100 kΩ down |
| 55 | PD8 | `OE_4` | GPIO | out | port 4's clock gate, `4OE`; 100 kΩ down |
| 56 | PD9 | `RXD_ECHO` | `USART3_RX` | in | the up port's echo |
| 57 | PD10 | `ALERT_3` | GPIO, EXTI10 | in | down port 3's power board |
| 58 | PD11 | `ALERT_4` | GPIO, EXTI11 | in | down port 4's power board |
| 59 | PD12 | `SCL_2` | `I3C1_SCL`, I²C legacy | i/o | the up port's and port 1's `INA238`s |
| 60 | PD13 | `SDA_2` | `I3C1_SDA`, I²C legacy | i/o | |
| 61 | PD14 | `PGOOD` | GPIO | in | the `LMR43620`'s window |
| 62 | PD15 | — | | | free |
| 63 | PC6 | `TXD_4` | `USART6_TX` / `TIM3_CH1` | out | |
| 64 | PC7 | `RXD_4` | `USART6_RX` / `TIM3_CH2` | in | |
| 65 | PC8 | `ALERT_0` | GPIO, EXTI8 | in | the up port's power board — **one of five, a pin per port, no wire-OR**; `ALERT_1`–`ALERT_4` on PD4 · PD5 · PD10 · PD11, each off its board's `ISO1642`, high on alarm; 100 kΩ pull-down at the pin |
| 66 | PC9 | — | | | free |
| 67 | PA8 | `CLK_PORT` | `MCO1` ← HSE | out | **the one clock output** — prescaler 1 on a Bifrost (2²²), 8 on an Argus (2¹⁹) — into the quad buffer's four strapped inputs |
| 68 | PA9 | — | | | free |
| 69 | PA10 | — | | | free |
| 70 | PA11 | — | | | free (USB, unused) |
| 71 | PA12 | — | | | free (USB, unused) |
| 72 | PA13 | `SWDIO` | SWD | i/o | |
| 73 | VDDUSB | 3,3 V | | | tied, USB unused |
| 74 | VSS | | | | |
| 75 | VDD | 3,3 V | | | |
| 76 | PA14 | `SWCLK` | SWD | in | |
| 77 | PA15 | `PPS_K` | `TIM2_CH1` capture | in | behind the `THVD1450` receiver on the time bus |
| 78 | PC10 | `SCL_3` | `I3C2_SCL`, I²C legacy | i/o | down port 4's `INA238` — alone on its controller |
| 79 | PC11 | `SDA_3` | `I3C2_SDA`, I²C legacy | i/o | |
| 80 | PC12 | — | | | free |
| 81 | PD0 | — | | | free |
| 82 | PD1 | `SD_UP` | GPIO | in | 100 kΩ down |
| 83 | PD2 | `ATTN` | GPIO | in | high on Kronos's ribbon = Bifrost; pulled down on the card. **Also the `OE` of both clock buffers** |
| 84 | PD3 | — | | | free |
| 85 | PD4 | `ALERT_1` | GPIO, EXTI4 | in | down port 1's power board — high is the alarm, 100 kΩ pull-down at the pin |
| 86 | PD5 | `ALERT_2` | GPIO, EXTI5 | in | down port 2's power board |
| 87 | PD6 | `SCL_PWR` | `I2C3_SCL` | i/o | down ports 2 and 3's `INA238`s |
| 88 | PD7 | `SDA_PWR` | `I2C3_SDA` | i/o | |
| 89 | PB3 | — | | | free |
| 90 | PB4 | — | | | free |
| 91 | PB5 | — | | | free |
| 92 | PB6 | `TXD_1` | `USART1_TX` / `TIM4_CH1` | out | |
| 93 | PB7 | `RXD_1` | `USART1_RX` / `TIM4_CH2` | in | |
| 94 | BOOT0 | 10 kΩ to ground | | | |
| 95 | PB8 | `SCL_TB` | `I2C1_SCL` | i/o | the time bus label |
| 96 | PB9 | `SDA_TB` | `I2C1_SDA` | i/o | |
| 97 | PE0 | — | | | free |
| 98 | VCAP | 2× 1 µF 50 V 0805 + 2× 100 nF 50 V 0603 — 2,2 µF, the sheet's 2,2 µF ±20 % | | | |
| 99 | VSS | | | | |
| 100 | VDD | 3,3 V | | | |

**59 GPIO used, 20 free, `PH1` unconnected** — free: PA4 · PA5 · PA9–PA12 · PB2–PB5 · PB10 · PC9 · PC12–PC15 · PD0 · PD3 · PD15 · PE0.

### Timers

| timer | width | clock | channels used | role |
|---|---|---|---|---|
| **TIM2** | 32 | 2²⁷ | CH1 capture `PPS_K` (PA15); CH2–CH4 compares, no pin | the free-running timebase: the 2²⁷ grid the card measures on, the frame grid, the second's anchor |
| **TIM5** | 32 | 2²⁷ | CH1 compare → PA0, CH2 capture ← PA1 | the port up's ranging turnaround (one-pulse, triggered by CH2) |
| **TIM4** | 16 | 2²⁷ | CH1 → PB6, CH2 ← PB7 | port 1 ranging: fire on CH1, capture the return on CH2 |
| **TIM15** | 16 | 2²⁷ | CH1 → PA2, CH2 ← PA3 | port 2 ranging |
| **TIM8** | 16 | 2²⁷ | CH3 → PB12, CH2 ← PB13 | port 3 ranging |
| **TIM3** | 16 | 2²⁷ | CH1 → PC6, CH2 ← PC7 | port 4 ranging |
| TIM1 · TIM6 · TIM7 · TIM12 · LPTIM1 · LPTIM2 | 16 | | no pin | free — for the firmware's housekeeping if it wants them; the port clock is `MCO1`, not a timer |

A down port's ranging timer measures `2 × route` on 16 bits: 488 µs at 2²⁷, 2 × 49 km. A port
whose route is longer runs its timer at ÷4 (30 ns a tick, 2 ms) — a setting from the run's
length in `BUSCFG`. The up port turns edges round on TIM5 and counts only its programmed offset.

### Clock tree

| | Bifrost | Argus |
|---|---|---|
| `OSC_IN` | 2²² = 4,194304 MHz, the time bus's `CLK` behind its receiver | 2²² = 4,194304 MHz, the up port's `CLK` |
| HSE | bypass | bypass |
| PLL1 | **M 1 · N 64 · P 2** → VCO 2²⁸ = 268,435456 MHz, `P` 2²⁷ — 2²² sits inside **`f_PLL_IN` 2…16 MHz** (DS14540 Rev 3, Table 46) | the same |
| SYSCLK, AHB, APB1/2, timer kernel | **2²⁷ = 134,217728 MHz**, no further division | the same |
| MCO1 | HSE, prescaler 1 → **2²²** on PA8 | HSE, prescaler 8 → **2¹⁹** on PA8 |
| decided by | `ATTN` at boot — high is a Bifrost | |

Every timer counts the same 2²⁷, and the VCO sits inside the part's 192–836 MHz window. Nothing
in the tree changes with the role: the role changes the source ahead of `OSC_IN` and `MCO1`'s
prescaler behind it.

**A builder may run the core slower.** `HPRE` at ÷2 or ÷4 puts SYSCLK at 2²⁶ or 2²⁵ and the
timers with it, so the ranging tick coarsens to 15 or 30 ns — still far inside ±1 µs — and the
port clock, taken off HSE, does not move. It saves some 10 mA per card. The base build does not do it: the
firmware idles in `WFI` between interrupts anyway, and one clock tree is one clock tree.

### Analogue and reference

**Every filter inductor on this board is a π filter: 10 µF 1206 + 100 nF 0603 before it on the rail and
the same pair at the pins behind it** — the house rule (`../core/POWER.md`, *The filter parts*).

`VDDA` and `VREF+` off the 3,3 V through the shielded 2,2 µH `SWPA252012S2R2MT` each, 10 µF + 100 nF at the pins; `VREF−`
and `VSSA` to the ground plane at one point. The ten `ID` inputs are read single-ended against
`VREF+`, and the 10 kΩ pull-up of every `ID` resistor hangs on the same 3,3 V, so the reading is
a ratio and the rail's tolerance drops out.

## The sockets — pin by pin

**Connectors: `MNB/NB IN` · `PWR IN` · `NB/MINI OUT 1`…`4` · `PWR OUT 1`…`4` · `TIME BUS` · `12V` — one board, so the silkscreen carries both roles; which runs in a port is the card's** (`../galvani/README.md`, *Connector names*).

**The up port is a unit end and the four down ports are source ends.** On the up port nothing is
switched by a processor pin: the plugged boards run whenever the card does, held by resistors in
the socket. On a down port one GPIO, `EN_n`, is the port's `LINE_EN` and its `ENABLE`; a second, `OE_n`,
opens its clock gate. Both boot low and carry 100 kΩ to ground, so every down port comes up dark
and unclocked.
Every `ID` is an ADC pin with **10 kΩ 1 % to the `VREF+` rail**, the 3,3 V behind its inductor.
**Each of the three I²C controllers carries one pair of 4,7 kΩ to the processor's 3,3 V**, fitted
once however many sockets share it; every `ALERT` **100 kΩ to ground, high = alarm**.

### `MNB/NB IN` — the up port

**Data body, 12 pins** — its 3,3 V is the up port's `TPS629206`.

| pin | signal | on this board |
|---|---|---|
| 1 | `CLK/PPS` | into the `74AUP1G125` ahead of `OSC_IN` (PH0), enabled by `ATTN` low — the Argus's clock; on a Bifrost the buffer is off and the pin is not looked at |
| 2 | `GND` | ground |
| 3 | `TXD` | PA0, `UART4_TX` / `TIM5_CH1` |
| 4 | `RXD` | PA1, `UART4_RX` / `TIM5_CH2` |
| 5 | `ID` | PB0, `ADC12_INP9`, 10 kΩ 1 % to `VREF+` |
| 6 | `ID_RET` | **1,78 kΩ 1 % to ground — 0,15, a card** |
| 7 | `RXD_ECHO` | PD9, `USART3_RX` |
| 8 | `DE` | PE15, GPIO |
| 9 | `B_DIR` | **tied to ground** — this end listens on channel B |
| 10 | `LINE_EN` | 10 kΩ to 3,3 V, no processor pin |
| 11 | `SD` | PD1, GPIO in, 100 kΩ to ground |
| 12 | `3,3 V` | the up port's `TPS629206` |

**Power body, 8 pins** — on I3C1 with down port 1, the first of the pair.

| pin | signal | on this board |
|---|---|---|
| 1 | `ENABLE` | not connected — the unit power board pulls it to its input |
| 2 | `GND` | ground |
| 3 | `ID` | PB1, `ADC12_INP5`, 10 kΩ 1 % to `VREF+` |
| 4 | `SDA` | PD13, `I3C1_SDA` in I²C legacy — I3C1's pull-up pair, shared with down port 1 |
| 5 | `A_SEL` | to ground — `INA238` at 0x40 |
| 6 | `SCL` | PD12, `I3C1_SCL` in I²C legacy |
| 7 | `ALERT` | PC8, EXTI8, 100 kΩ to ground |
| 8 | `3,3 V` | the up port's `TPS629206` |

### `NB/MINI OUT 1`…`4` — the four down ports

**Data body, 12 pins** — its 3,3 V is the down ports' `LMR43620`.

| pin | signal | common to all four | port 1 | port 2 | port 3 | port 4 |
|---|---|---|---|---|---|---|
| 1 | `CLK/PPS` | the port's `SN74LVC126A` gate output through **33 Ω** in series; all four gate inputs on `MCO1`, PA8; the gate opened by `OE_n` | `1Y`, `OE_1` PE9 | `2Y`, `OE_2` PB14 | `3Y`, `OE_3` PB15 | `4Y`, `OE_4` PD8 |
| 2 | `GND` | ground | | | | |
| 3 | `TXD` | | PB6 `USART1_TX` | PA2 `USART2_TX` | PB12 `UART5_TX` | PC6 `USART6_TX` |
| 4 | `RXD` | | PB7 `USART1_RX` | PA3 `USART2_RX` | PB13 `UART5_RX` | PC7 `USART6_RX` |
| 5 | `ID` | 10 kΩ 1 % to `VREF+` | PC0 `INP10` | PC2 `INP12` | PC4 `INP4` | PA6 `INP3` |
| 6 | `ID_RET` | not connected | | | | |
| 7 | `RXD_ECHO` | not connected — a master never echo-checks | | | | |
| 8 | `DE` | GPIO | PE2 | PE5 | PE8 | PE12 |
| 9 | `B_DIR` | **tied to 3,3 V** — this end drives channel B | | | | |
| 10 | `LINE_EN` | `EN_n`, the same net as `ENABLE`; 100 kΩ to ground | PE3 | PE6 | PE10 | PE13 |
| 11 | `SD` | GPIO in, 100 kΩ to ground | PE4 | PE7 | PE11 | PE14 |
| 12 | `3,3 V` | the `LMR43620` | | | | |

**Power body, 8 pins** — its 3,3 V is the down ports' `LMR43620`.

| pin | signal | common to all four | port 1 | port 2 | port 3 | port 4 |
|---|---|---|---|---|---|---|
| 1 | `ENABLE` | `EN_n`, the same net as `LINE_EN` | PE3 | PE6 | PE10 | PE13 |
| 2 | `GND` | ground | | | | |
| 3 | `ID` | 10 kΩ 1 % to `VREF+` | PC1 `INP11` | PC3 `INP13` | PC5 `INP8` | PA7 `INP7` |
| 4 | `SDA` | the controller's pull-up pair, fitted once | PD13 `I3C1` | PD7 `I2C3` | PD7 `I2C3` | PC11 `I3C2` |
| 5 | `A_SEL` | strapped | 3,3 V — 0x41 | ground — 0x40 | 3,3 V — 0x41 | ground — 0x40 |
| 6 | `SCL` | the controller's pull-up pair, fitted once | PD12 `I3C1` | PD6 `I2C3` | PD6 `I2C3` | PC10 `I3C2` |
| 7 | `ALERT` | EXTI, 100 kΩ to ground | PD4 | PD5 | PD10 | PD11 |
| 8 | `3,3 V` | the `LMR43620` | | | | |

**An Argus is this board**, and these tables are its sockets pin for pin; what differs is the
rung on `CLK/PPS` of the down ports, 2¹⁹ against 2²².

## The trunk — the up port on a Bifrost

**The trunk is a point-to-point MasterNOD link to one Mayak port, over the crossed in-box cable**
— conductors 1–6 of the data body, `TXD`↔`RXD` and `ID`↔`ID_RET` swapped, no Galvani board, no
transceiver: plain 3,3 V logic, the 12 V on each end's own terminals. The physical UART is the
card's identity — one trunk, one card — and nothing in the frame repeats it. **It carries no
clock and no edge**: the time rides it only as the 4 B label on each record, while the network
clock and `PPS_K` are Kronos's own wires, so the trunk's length is irrelevant to timing. **The
four trunk cables are made alike** — same length, construction and connectors — so the links are
interchangeable and nothing is tuned per port. The cards sit within about half a metre of the
Mayak.

**2²¹ ≈ 2,1 Mb/s, 8N1, an ordinary asynchronous UART** — the integer-division rule binds the
NodBus behind the card, not this link. A full card's 8 units × 128 frames × 48 B is 491 520 bit/s
at ten bits a byte, **23 % of the link**; the idle rest is where a ranging pulse and the head's
commands fit. It carries neither a 485 barrier nor a laser, so a build may run it at 2²²: 330 Ω
against ~40 pF is 13 ns, 5,5 % of a 238 ns bit.

**More than one card on a Mayak port is allowed by the hardware and not implemented by the
firmware.** The whole provision is **an unfitted pad for a 330 Ω pull-up on each of the Mayak's trunk
`RXD` lines**, where the cards' outputs meet — no board fits the resistors; the head's `TXD` stays push-pull. The multipoint build is then **each card's
`TXD` switched to open drain in software** and a different cable, 40 cm at most. Two cards on one
port are frequency-locked (Kronos's 2²² on the same ribbon) and epoch-locked (the same `PPS_K`
edge) with the head silent; **the head deals the slot assignment** and checks the cards' stamps
against its own `PPS_K` capture. **At 2²¹ two cards a port is the limit** — 23 % of the wire for one
card, 47 % for two, 70 % for three; **at 2²² four fit**, 47 %. Joined cards free trunk ports at the
head. **The base build keeps one card a port**: a card whose `TXD` sticks low holds the shared
line and silences every card on it, where on its own port it costs only its own units. Their two `ID_RET` resistors read as one parallel value, which the head does
not decode: an `ID` names the interface and the dialogue names the partner.

## The card's supply — 12 V in, three bucks

**The card takes the 12 V on two two-pole terminals of its own, an input and a tap — Degson
`DGPS2.5R-5.0`, 5 mm, 0,75–2,5 mm², 20 A, the same node — through its own fast fuse, sized by the
construction, in the enclosure's fuse field, with one `5.0SMDJ14A` across the terminals.** A
reversed pack drives the transil forward and the fuse clears; an overvoltage it holds long enough
clears the fuse the same way (`../galvani/README.md`, *The rails*). The 12 V is the battery rail at
the station and the unit power board's 12 V at a remote Argus. **None of it crosses the card**: a
source power board on a port takes the 12 V on its own terminals. The 12 V node carries the house
input: **47 µF 50 V hybrid polymer + 10 µF 50 V 1206 + 100 nF** at the terminals.

**Three bucks, all making 3,3 V, and what splits them is a fault.** One rail for everything would
let a short behind any of the five sockets brown out the H523 that has to report it — on a card at
a remote Argus the difference between a bad port and a dead site.

```
  12 V in ─┬──▶ TPS629206 ──▶ 3,3 V   THE PROCESSOR — the H523, the two time-bus receivers, the port-clock buffer
           ├──▶ TPS629206 ──▶ 3,3 V   THE UP PORT — its boards
           └──▶ LMR43620  ──▶ 3,3 V   THE FOUR DOWN PORTS — their boards
```

| rail | part | load | |
|---|---|---|---|
| the processor | **`TPS629206`**, 600 mA | **~61 mA, 203 mW** — the H523 ~60 mA and the two `THVD1450` time-bus receivers at 0,70 mA each (0,96 max); ~10 % of the part | the time input outlives a shorted port |
| the up port | **`TPS629206`**, 600 mA | ~55 mA at an optical end, 125 mA at most; ~18 mA on copper | 21 % of the part at the ceiling |
| the four down ports | **`LMR43620`**, 2 A | ~420 mA with four optical ends, 700 mA, 2,31 W at their ceiling; ~300 mA with four copper communication boards at ~75 mA each; ~520 mA if all four burst at once | 35 % of its ampere at the heaviest, 15 % at the lightest |

**On the 12 V the whole card is ~0,16 A on average and ~0,27 A at its ceiling** — 1,77 and 2,92 W of 3,3 V at the family's 90 % — and 0,33 A at
the 10 V bottom of the pin's window.

**The bucks are fault isolation, not a switch.** None has its `EN` on a pin: every rail runs
whenever the 12 V is there, a shorted port is caught by its own buck's current limit and hiccup,
and **a card's off is a deep sleep of the processor with its down ports dark by their own
`ENABLE`** (`../core/PROTOCOL.md` §7). `MODE` is auto on all three — nothing on this card listens in
a band. A communication board has no terminal and no buck; it takes its 3,3 V over the connector,
and a source power board takes the host side of its isolator from the same pins.

**`TPS629206` — the processor's and the up port's cell**, from its sheet (SLVSGE2): 3–17 V in,
**18 V absolute maximum**, the part the input transil is chosen under.

| position | value |
|---|---|
| `L` | **2,2 µH**, shielded — the sheet's Coilcraft `XGL3530-222` |
| `C_IN` | **4,7 µF 50 V X7R 1206 + 100 nF 50 V** at `VIN` |
| `C_OUT` | **3× 10 µF 50 V X5R 1206** — ~21 µF effective at 3,3 V, the sheet's 22 |
| the divider | **`R1` 619 kΩ / `R2` 137 kΩ, 1 %** → 3,311 V against the 0,6 V reference |
| `MODE/S-CONF` | **17,8 kΩ to ground** — external feedback, 1 MHz, auto PFM/PWM, no AEE: ~93 % at 100 mA |
| `EN` | to `VIN` — the rail runs whenever the 12 V is there |
| `PG` | not connected |

**`LMR43620` — the down ports' cell**, the house cell one size up (`../galvani/HARDWARE.md`, *The
buck cell*): `RT` 7,50 kΩ → 2,08 MHz, auto; **4,7 µH shielded, `I_SAT` ≥
3,5 A**; `R_FBT` 28,0 kΩ / `R_FBB` 12,1 kΩ → 3,31 V, `C_FF` 22 pF C0G; `C_IN` 4,7 µF 50 V + 100 nF;
`C_OUT` 3× 10 µF 1206 + 100 nF; `VCC` 1 µF; `BOOT` 100 nF; `PGOOD` to PD14 through 100 kΩ.

**The time-bus receivers are `THVD1450`, `RE#`, `DE` and `D` tied low**, on the processor's rail.
The part pulls `RE#` up inside, so an open `RE#` is a card with no clock; its failsafe reads high
on an idle pair, which the clock select never looks at (*One clock in, two doors*). On an Argus
the two receivers are fitted and hear nothing; at 0,70 mA each no pin is spent switching them.

**The port kill is one wire, and the port's clock has its own.** `EN_n` is the port's `LINE_EN`
on the data body and its `ENABLE` on the power body: the `SN6505B` on a copper board and the
source converter go off together, and nothing on an optical board is switched. `OE_n` opens and
shuts the port's clock gate by itself. The up port
is a unit end — `LINE_EN` held high by 10 kΩ in the socket, `ENABLE` not connected — so nothing on
the card switches its own up link off. Two kills are commanded by the head and run on their own by
the card's fault ladder (`FIRMWARE.md` §10, §11):

| | what it does | what survives |
|---|---|---|
| **`PORT_CLK`** | `OE_n` low: the port's clock gate shuts; the unit mutes on unnegotiated clock loss and rejoins by itself when it returns | **everything** — the unit was never off |
| **`PORT_PWR`** | `OE_n` and `EN_n` low: the clock, the feed and the line side go | **nothing** — the one reset that clears a part with no reset pin |

## What happens to a frame

```
   OFF THE SPUR — 40 B, carrying only the time a node can honestly know

      ┌────────┬───────┬──────────┬──────┬──────┬────────┬────────────────┬───────┐
      │ unix.0 │ frame │ TYPE|NUM │ slot │ kind │ status │  payload 32 B  │ CRC16 │
      └────────┴───────┴──────────┴──────┴──────┴────────┴────────────────┴───────┘
          │
          └─ unix.0 = the second, mod 256, frame = the sample within it —
             ALL the unit knows of absolute time
                                       │
                                       ▼

   THE CIRCULAR BUFFER — per unit, indexed by frame number, a two-gate FIFO:
   written on arrival, read at the frame's own index + 96 (16 on an Argus)

      the unit held it 32 before sending, so it reaches the head 128 frames,
      one second, after it was measured; a repair — RESEND after a CRC miss —
      lands well inside the delay

      a frame not there at its instant leaves as a FILLER with a reason code,
      so absence is data, not silence
                                     │
                                     ▼

   THE STAMP — the one place the absolute second is put on

        the PPS-K edge marked the boundary
        the Kronos label named it
        the unit's unix.0 must match the low byte of that second — a check
        no delay is subtracted here — the unit placed its grid its route early
                                     │
                                     ▼

   ONTO THE TRUNK — 48 B, and the card PREPENDS; the frame is never moved

      ┌ unix.3 │ unix.2 │ unix.1 ┐ unix.0 │ frame │ TYPE|NUM │ slot │ kind │ status │ payload │ rsvd×5 │ CRC16
      └── written in front ──────┘└──────────── the spur frame where DMA landed it ───────────┘ zeroed   recomputed
        The second reads most-significant-first from the first byte; the five reserve
        bytes overwrite the spur's CRC (already checked) and the new CRC covers all 46.
                                     │
                                     ▼
      a transmit FIFO, 2–4 trunk frames for the whole card
                                     │
                                     ▼
   MAYAK — allocates space in the file and writes it. No time arithmetic at all.
```

## Ranging — the measurement, in time order

**What is being measured is the route, twice**, and the trick that keeps it to twice is that
the far unit is armed by **its own slot** rather than by the arrival of the command.

```
   PERIOD N
   ──────────────────────────────────────────────────────────────────────────

     the card sends the command inside that unit's own block:
     "turn the next edge round"

     its own flight does not matter — it has a whole period to arrive


   PERIOD N+1, INSIDE THAT UNIT'S BLOCK OF FOUR
   ──────────────────────────────────────────────────────────────────────────

     the unit    ├── its DATA frame ──┤
                                      └──▶ THE RETURN IS ARMED HERE
                                           a timer instant both ends know

     the card    ├── hears it arrive ──┤
                                       ├── guard: let the line drain ──┤
                                                                       │
                                              fires the pulse ─────────┘
                                                       │
                                    ├─── the route ───▶│  the unit turns it round
                                                       │
                                    ◀─── the route ────┤  the card captures
                                                       │
                                                       ▼
                        measured  =  2 × the route  +  the turnaround
                        and the turnaround is a constant, known in advance
```

**The turnaround is the unit's timer — capture + compare, no part on any board.** The unit's
timer runs in one-pulse mode triggered by its input: the incoming edge on `RXD` (input capture,
channel 1 or 2 — only those can trigger) starts the counter, and a compare channel on `TXD`
raises the return after `CCR` ticks and drops it at `ARR` — a pulse of programmed delay and
programmed width, in ticks of the unit's grid clock, with no CPU in the path. Before the
measuring frame the processor switches the two pins' alternate functions from the USART to the
timer and raises `DE`; after it, back. The constant is an exact number of ticks, known by
construction; the jitter is the synchroniser's one tick — 7,45 ns on an H523 at 2²⁷, 3,73 ns on an H7A3 at
2²⁸. **The same timer, in PWM-input mode on the same `RXD`, captures both edges of the incoming
pulse**, so the unit reports the width it received as a number in its next frame; the card's
double capture on the return gives the up-path width, and the two together are what a loop
would have given summed — separated by direction instead. No board carries a loop.

**What that asks of a board's pin-out, and nothing else:** `RXD` and `TXD` on channels of the
same timer, `RXD` on CH1 or CH2. Every STM32 in the station has the mode; it is a line in each
`HARDWARE.md`.

**The H523 in LQFP100 has exactly the five pairs the card needs** (DS14540 Rev 3, Table 13;
the USART `SWAP` bit chooses which pin of a pair is the receiver):

| USART | `TXD` | `RXD` | timer | |
|---|---|---|---|---|
| UART4 | PA0 (`TIM5_CH1`) | PA1 (`TIM5_CH2`) | **TIM5**, 32-bit | **the port up** — its route can be the long one |
| USART1 | PB6 (`TIM4_CH1`) | PB7 (`TIM4_CH2`) | TIM4 | a port down |
| USART2 | PA2 (`TIM15_CH1`) | PA3 (`TIM15_CH2`) | TIM15 | a port down |
| UART5 | PB12 (`TIM8_CH3`) | PB13 (`TIM8_CH2`), `SWAP` | TIM8 | a port down |
| USART6 | PC6 (`TIM3_CH1`) | PC7 (`TIM3_CH2`) | TIM3 | a port down |
| USART3 | — | — | none | **the echo receiver** — it ranges nothing |

Five timers, no pin shared, and **TIM2 (32-bit) is left for the 2²⁷ timebase and the `PPS_K`
capture.** The 16-bit timers count 488 µs at 2²⁷ — 2 × 49 km — which covers every 10 km glass
port; a port measuring a 100 km route runs its timer at ÷4 (30 ns a tick, 2 ms of range, still
thirty times inside the contract), set from the run's length the port knows from `BUSCFG`.
**2 ms is about 200 km of round trip and the timer therefore never binds a hop**: the wall is the
glass at 100 km and the 300 V feed at 120 (`../galvani/README.md`, *One hop ends at 100 km*). The
round trip must also fit the frame's idle gap, and 100 km is 1,0 ms against a 7,8 ms frame at
128 Hz.

**An interrupt in that path is not admissible**: entry latency plus whatever was already
running is microseconds of jitter against a microsecond budget, and jitter does not subtract
out the way a constant does.

**It happens on every build, and it is always the data channel** — launched on the card's own
data channel and returned on the units', both always open, so at start it runs in the RC
dialogue and in operation it fits in the gap; the clock channel is never borrowed and never
idle. **And it repeats**: every `RANGE_INTERVAL` seconds per port, with a return of
`RANGE_WIDTH` ticks (`FIRMWARE.md` §4, §10).

**The assumption under it, written down:** the data pairs and the clock pair of one cable, or the
fibres of one cable, share their propagation time to a few per cent, and transceivers of one
class (`ISO145x`) delay the edge alike — the sheet gives the driver 19 ns typical and 41 maximum,
the receiver 36 and 60, over the whole temperature and supply range; each leg carries one of each,
so the typicals cancel in the halving and the two ends' difference is what survives, 23 ns halved at
worst, three ticks. Measuring the data route and applying it to the clock route rests on that.

## The pulse, and the two things that bound it

```
   THE UNIT'S OWN BLOCK — four frame-times, and the measurement lives in three

     ┌────────────┬────────────────────────────────────────────────────────┐
     │    DATA    │   the unit's other three frame-times                   │
     └────────────┴────────────────────────────────────────────────────────┘
                       ├─ pulse ─┤
                       ▲
                       └─ THE LEADING EDGE IS WHAT IS TIMED, so width costs
                          occupancy and never accuracy


     FLOOR     wide enough for the path to pass the edge — the transceiver or
               the 1×9 module, and the cable's own rise time. It is NOT a bit
               time: the return lands on a timer capture, not on a receiver,
               so there is no sampling window and no framing to satisfy.

     CEILING   it fits inside those three frame-times. The scheduler does not
               move for it, and no other unit's block is touched.


   The width is picked from the gap and the route: the pulse plus its round trip
   lands before the next frame is due. The long route carries one unit, so it
   owns the rest of its period — 100 km returns in ~1 ms against ~7 ms idle
   behind a lone unit's frame. Nothing ranges at start-up only.
```

**The card measures at 2²⁷ = 134,217728 MHz — 7,45 ns a tick, exactly 16 sub-ticks per 2²³
tick**, so the measurement enters the time arithmetic as a shift and never as a division.

**The gap is computed and emptied for the pulse.** The card knows the rung and the population, so
it knows to the tick how long the line is idle behind each DATA frame; for the measurement it holds
back everything else that would ride that gap — CONTROL, the tunnel, a resend — and the pulse goes
into a silent line. That is the whole cost: one gap, one unit, nothing rescheduled.

**The glass barely moves; re-ranging is for the electronics.** Single-mode fibre drifts about
**40 ps per kilometre per kelvin** (37–50 by construction), and the loop carries twice the route:

| route | glass in the loop | ΔT | drift |
|---|---|---|---|
| 500 m | 1 km | 5 K | 0,2 ns |
| 2 km | 4 km | 20 K | 3,2 ns |
| 5 km | 10 km | 20 K | 8 ns |

Twenty kelvin across two kilometres is under one tick. **No optical module's sheet specifies its
propagation delay**, and the card and the far unit sit in enclosures rated −40…+85 °C, so the
silicon's drift is the term left unbounded — which is why ranging repeats every `RANGE_INTERVAL`
on every run that leaves the box, and why a module with unspecified delay is admissible on a
ranged spur.

**No ranging on a ModBus arm**: nothing polled slowly enough to sit on ModBus has a use for a
microsecond.

## The parts

| part | qty | where |
|---|---|---|
| `STM32H523VE`, LQFP100 | 1 | the processor |
| `THVD1450`, SOIC-8 | 2 | the time bus receivers, `CLK` and `PPS_K` |
| `74AUP1G126` · `74AUP1G125` | 1 · 1 | the clock select ahead of `OSC_IN` |
| `SN74LVC126A`, `RGY` | 1 | the port-clock buffer, one gate a down port |
| `LMR43620` | 1 | the down ports' 3,3 V |
| `TPS629206` | 2 | the processor's and the up port's 3,3 V |
| 4,7 µH shielded, `I_SAT` ≥ 3,5 A | 1 | the `LMR43620` |
| 2,2 µH shielded, `XGL3530-222` | 2 | the `TPS629206`s |
| `BX2.54-2xNA`, 2×6 | 5 | the data bodies — the up port and four down |
| `BX2.54-2xNA`, 2×4 | 5 | the power bodies |
| `BX2.54-2xNA`, 2×5 | 1 | the time bus tap |
| `DGPS2.5R-5.0`, two-pole | 2 | the 12 V, input and tap |
| `5.0SMDJ14A` | 1 | across the 12 V input |
| 10 Ω, 1 % | 4 | the time bus line block, 2 per pair |
| 80,6 Ω, 1 % + jumper | 2 | the termination, fitted on the last node of the ribbon |
| 33 Ω | 4 | `CLK/PPS` in series, the down ports |
| 1,78 kΩ, 1 % | 1 | `ID_RET` on the up port — 0,15, a card |
| 10 kΩ, 1 % | 10 | the `ID` pull-ups to `VREF+` |
| 10 kΩ | 2 | the up port's `LINE_EN` to 3,3 V · `BOOT0` to ground |
| 4,7 kΩ | 6 | I3C1, I2C3, I3C2 — one pair each |
| 100 kΩ | 20 | `EN_1`–`EN_4` · `OE_1`–`OE_4` · `SD_UP`, `SD_1`–`SD_4` · `ALERT_0`–`ALERT_4` to ground · `ATTN` to ground · `PGOOD` up |
| 7,50 kΩ · 28,0 kΩ · 12,1 kΩ, 1 % · 22 pF C0G | 1 each | the `LMR43620`'s `RT`, divider and `C_FF` |
| 619 kΩ · 137 kΩ, 1 % · 17,8 kΩ | 2 each | the `TPS629206`s' divider and `MODE/S-CONF` |
| 2,2 µH shielded, `SWPA252012S2R2MT` | 2 | `VDDA`, `VREF+` |
| 47 µF 50 V hybrid polymer | 1 | the 12 V node |
| 10 µF 50 V 1206 | 1 | the 12 V node |
| 4,7 µF 50 V 1206 | 3 | the three bucks' `C_IN` |
| 10 µF 50 V X5R 1206 | 9 | the three bucks' `C_OUT`, three each |
| 10 µF | 3 | the processor's bulk, `VDDA`, `VREF+` |
| 1 µF 50 V 0805 | 4 | `VCAP`, two on each |
| 1 µF | 1 | the `LMR43620`'s `VCC` |
| 100 nF 50 V X7R | ~24 | every `VDD` of the H523, `VDDA`, `VREF+`, `VCAP` two on each, `NRST`, each receiver and buffer, the bucks' inputs and outputs, `BOOT`, the 12 V node |

## Bench criteria

- The ranging constant reproduces to ±1 tick over a hundred launches on a 2 m cable; a 500 m
  copper reel ranges within 2 % of its measured length.
- `SYNC`'s start bit lands within ±1 tick of the `PPS_K` edge.
- `ALERT` to `EN_n` low in under 1 ms; a unit power-cycled by the ladder rejoins in under 4 s.
- A short on any down port leaves the processor and the up port running.
- The trunk carries eight units at 128 Hz for 24 h with zero fillers on a bench spur.
- `ATTN` high gives `OSC_IN` the time bus's 2²² and the ports the same edge; `ATTN` low the up port's
  2²² and the ports 2¹⁹.
- `OE_n` low stops one port's clock with its feed and line side still up; `EN_n` low leaves the clock
  running on a port whose gate is open.

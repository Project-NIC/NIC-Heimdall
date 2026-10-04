★ N.I.C. ★

# NIC Core — Hardware (the universal node + station)

> **Design-stage concept.** The figures are verified against the parts' sheets and the parts
> before a board is made.

The hardware that is the **same on every front**: the node's MCU and power chain, the
NodBus front-end, the surge protection, the enclosure, and the data station (master).
A front adds only its **sensors** on top of this — Quake's sensor lineup, for one, is
`../quake/HARDWARE.md`. The detailed, reusable building blocks live in `blocks/`;
this page is how they assemble into a working node and station.

---

## Node — the universal STM32H523 box

| Item | Part | Why |
|---|---|---|
| MCU | **STM32H523** | The house NODE MCU (the full assignment map — H523 nodes, cards and slave MODs, H7A3 DSP boards, ESP32-S31 head — is `../NAMING.md`'s): Cortex-M33 at 2²⁷ Hz, 134 MHz off the timebase, + FPU (the on-node levelling matrix multiply, CRC unit), plenty of SPI/UART/timers, 272 KB SRAM for every front that is not DSP-tier — Tesla, Pip, Marconi and the scintillation boards are the H7A3's; one part / one firmware family across the node boards (no MCU zoo); ~€1 over a value-line H5 at no power penalty; its image updated over the bus by `LOAD` and `APPLY` (`PROTOCOL.md` §7) |
| Bus port | **none on this board — the two Galvani bodies** | **no transceiver sits on a node.** A node is a measuring unit and stands at the unit end of a run: behind it are a **power board and a communication board**, and 485 or glass starts there. The line front, the barrier, the ladder and the feed are those boards'; the node carries the data body and the power body, and the 12 V reaches it on its own terminals and on neither ribbon (`../galvani/README.md`) |
| Quiet-rail LDO | **TPS7A2033** | a clean 3,3 V off a 4,0 V branch, only where a part without a regulator of its own feeds an analogue path (`../galvani/HARDWARE.md`, *The low rails*) |
| True-5 V LDO | **TPS7A4701** | only where a board has a genuine 5 V analogue rail — behind a second `LMR43610` taking the 12 V to 5,5 V on that board (Pluvius), or behind Tesla's `TPS629206` at 5,3 V |
| Rails | **12 V on two terminals; the node makes its own** | the unit power board's island output on the node's own two two-pole Degson `DGPS2.5R-5.0` terminals, an input and a tap — 12 V and nothing lower. The node carries **`LMR43610` → 3,3 V**, and where its LDOs want feedstock a second **`LMR43610` → 4,0 V**, side by side on the 12 V; a true-5 V board adds a second `LMR43610` to 5,5 V and its `TPS7A4701`; an H7A3 board makes its 1,8 V and its analogue feed on two `TPS629206` straight off the 12 V (`../galvani/HARDWARE.md`, *The low rails*, *The buck cell*). The ranging return is the node's timer: `RXD` and `TXD` on channels of one timer, `RXD` on CH1/CH2 (`../bifrost/HARDWARE.md`, *Ranging*) |

The node reaches its sensors only through SPI/UART/GPIO **callbacks** supplied by the
board HAL (the front's glue), so the portable `nic-*` core never touches a register.

```
THE NODE, WIRED — one drawing, because the node itself never changes


   whatever the run turns out to be
   ───────────────────────────────────────────────────────────────────────

     48 V or 300 V, data on copper ──┐
     fibre, the feed beside it       ├────▶  ┌──────────────────────────────────┐
     a short run with its own feed ──┘       │  THE GALVANI BOARDS — a POWER    │
                                             │  board and a COMMUNICATION board │
     THE CABLE LANDS HERE AND NOWHERE ELSE   │                                  │
       the unit end: no tube — the transil   │                                  │
       on the feed, 10 Ω + transil on a pair │                                  │
       the barrier · the island converter    │                                  │
       ISO145x, or the 1×9 seats on glass    │                                  │
                                             └────────────┬─────────────────────┘
                                                          │
        THE DATA BODY, 12 pins:                           │
          1 CLK/PPS (33 Ω at the source) · 2 GND          │
          3 TXD · 4 RXD · 5 ID · 6 ID_RET                 │
          7 RXD_ECHO · 8 DE · 9 B_DIR · 10 LINE_EN        │
          11 SD · 12 3,3 V                                │
        THE POWER BODY, 8 pins:                           │
          1 ENABLE · 2 GND · 3 ID · 4 SDA                 │
          5 A_SEL · 6 SCL · 7 ALERT · 8 3,3 V             │
        THE 12 V on two DGPS2.5R-5.0 terminals, never     │
          on a ribbon — the node makes its own rails:     │
          LMR43610 → 3,3 V · LMR43610 → 4,0 V feedstock   │
                                                          ▼
                                             ┌──────────────────────────────────┐
                                             │      STM32H523, LQFP100          │
                                             │                                  │
                                             │   CLK ─▶ OSC_IN                  │
                                             │   RC at boot, then the wire      │
                                             │   — a NOD carries no crystal     │
                                             │                                  │
                                             │   its own bucks and LDOs         │
                                             └────────────┬─────────────────────┘
                                                          │
                                 SPI / UART / GPIO callbacks — the board HAL
                                                          │
                                                          ▼
                                                the front's sensors


   The node carries NO transceiver and NO port. Which class is plugged in is the run's
   business and never the unit's: Quake, Tesla, Palatine, Sputnik and its PPS, Pip and its
   PPS, Argus and the Atlantis pod all take the same two bodies — and a unit moved from
   copper to glass, or from 48 V to 300 V, changes its Galvani boards and nothing else.
```

## NodBus — RS-485 data + clock, and where it starts

The backbone every node hangs on: outdoor UTP Cat 6, four pairs, into the board through a
**gland onto the Galvani board's field terminals** — there is no inline connector outside the
enclosure (`../galvani/README.md`, *The cable, the ground and the termination*). **All of it lives on the Galvani board.**

- **Inside the enclosure no link carries 485** — a host-to-host link (the Mayak, a card, Palatine,
  Sputnik) is a crossed in-box cable of plain logic, UART and I²C, the clock and PPS as wires
  behind a buffer; Kronos's time bus to the cards is M-LVDS, non-isolated (`../kronos/HARDWARE.md`).
  A unit that shares the enclosure — Palatine, Sputnik — takes such a cable instead of its Galvani
  boards. 485 begins where a Galvani board begins.
- **Data** — an **ISO145x** on the block, always: **ISO1452** where both directions are kept,
  which is every class as normally built; **ISO1450** where a pair really is shared — the clock
  channel and every ModBus leaf; 1×9 modules on glass.
  **The feed travels in a 2-core cable of its own** (2× 1,5 mm² or 2,5 mm²) and on land never on the data
  cable, so the four pairs are data TX · clock · ground · data RX and copper and both glass boards
  run **full duplex**; there is no half-duplex NodBus build. Self-timed TDMA slots: **`DE` is the
  driver enable**, up around the unit's own slot and down after its last stop bit, because the
  units' TX pair carries up to eight `DE`-gated drivers. The power gate is a different pin —
  `ENABLE` on the power body, driven by the source end's host and never by a unit.
- **Clock pair** — the same part, carrying the station's network clock (born on **Kronos**,
  GPSDO-disciplined, fanned out through the Bifrosts — the clock's single source of truth is
  `blocks/nodbus.md`, *The network clock*); the node takes `CLK` off the connector, wired straight to
  **OSC_IN**.
- **Termination** — 10 Ω series on both legs at every copper board, and **80,6 Ω** across the pair
  behind them, put across by a jumper on a 2-pin header on the first and the last board of a
  segment and on no board between; the legs complete the ~100 Ω the UTP wants — 80 + 10 and not 90 + 5, because the larger leg halves the `SM712`'s current before the tube fires. The
  bigger series R buys surge coordination at negligible signal cost on Cat 6.

Full detail: `blocks/nodbus.md`. Bought Modbus sensors ride Palatine's ModBus arms
(`blocks/modbus.md`) — same physics, a polled bus with no clock, and they leave through a Galvani
board like everything else.

## Protection — on the block, on both pairs alike

**The end of the cable decides what is fitted**, never the board and never the run length
(`../galvani/README.md`, *The protection ladder*): every board at a source end carries one
three-electrode tube, `2036-xx-SM`, to the common earthing point, then the series element and
the transil; every board at a unit end carries no tube — the series element and the transil
alone; an optical board carries none. Everything that leaves the enclosure is isolated, and a
remote site is not earthed on purpose. Same on the data and the clock pair, plus
source-side power protection. **None of it is on the node**: the ladder is on the Galvani board
at the enclosure entry, and the board is the sacrificial part. A host's in-box link is a cable
and carries no ladder, because nothing leaves.
Threat model: we protect against nearby induction; a direct strike
vaporises any node-level SPD — a cheap, sacrificial board is the accepted trade
(`../galvani/README.md`, *The protection ladder*).

> **Node-input power is not the node's.** The input stage lives on the plugged Galvani board —
> the island converter (`LT3748` off 48 V, `LT8316` off 300 V, one secondary onto the 12 V)
> behind a fed spur, on the node's own two `DGPS2.5R-5.0` terminals — and the node takes **12 V and
> nothing lower** (`../galvani/README.md`, *The boards*). What is on the
> node is its own bucks — 3,3 V, and 4,0 V where its LDOs want it — the LDOs its quiet loads
> want, and the decoupling around them, in the node's own `HARDWARE.md`.

## Enclosure

A plastic IP68 box (`../daedalus/CONSTRUCTION.md`, *Enclosure*); all connectors on one wall; the PCB on a **thin rigid
double-sided** board (a stiff bond for a seismograph — no foam); the top potted with a
re-enterable **cable gel** (the box seals, the gel protects and stays repairable;
neutral-cure, −40 °C, bubble-free).

## One package — every H523 board is the 100-pin body

The cross-switching pins stayed deleted (DE stays — one pin per port, re-scoped as the
driver's power gate, `blocks/nodbus.md`), and the whole pin budget — six serial
ports, I²C, the capture inputs, MCO, SWD and the service resets — fits **LQFP100 on every H523
board**: Kronos, Bifrost, nodes, Palatine, Quark-Tubes alike. One footprint, one layout block, no
144/64 juggling. **The DSP tier is the same rule one size up: `STM32H7A3IIT6`, LQFP176, on
Tesla, Marconi and both scintillation boards alike** — one footprint for the four boards.

## No FPGA and no CPLD — anywhere

**Two MCU part numbers are the whole programmable inventory of the station.** A gate array adds a
third programmable part, a toolchain, a configuration memory, its own rails and sequencing, and a
second language for work the H7A3 already does in software. It also leaves the station's one
verification discipline: every unit's firmware is a described, host-testable C layer,
and an HDL block is not tested that way.

## Every bus that meets a cable or a connector — the same three parts, in the same order

**From the terminal inward: the transil, the series resistors, the transceiver or the pin.** One
rule for every bus on every board, so nothing is chosen per position:

| the bus | the transil, at the terminal | the series resistors | the termination, where the bus needs one |
|---|---|---|---|
| **485**, every pair | `SM712` — clamps at the pair's −7 / +12 V | 2× 10 Ω, one a line | **80,6 Ω behind a jumper** on the transceiver side of the resistors, so the pair sees its 100 Ω |
| **CAN** | `PESD2CAN` — bidirectional, 24 V stand-off, a few pF | 2× 10 Ω, one a line | **100 Ω behind a jumper** on the transceiver side of the resistors, so the bus sees its 120 Ω |
| **a logic line to a part the builder connects** — SPI, I²C, UART, 1-Wire, GPIO, a clock on a header | one channel of `TPD4E05U06` — 0,5 pF a line, 5,5 V working, IEC 61000-4-2 level 4 | 33 Ω, one a line | — |

**The series resistor on a logic line is 33 Ω and it is on every bus line between a processor and
a part, fixed or plugged** — it slows the edge to what the trace needs, damps the ring, and limits
what a fault on the far end can push into the pin's protection. **The transil is matched to the
line**: the `SM712` is right on a 485 pair and wrong on a 3,3 V logic line, which it would not
clamp before +12 V and would load with tens of pF; the ESD array is right on a logic line and
would not survive a pair. **The order is what makes it work**: the transil takes the hit at the
terminal, the resistor limits what the clamp lets through, and the pin or the transceiver sees
the remainder. The surge path of every run stops on its Galvani boards; what is here is for the
hand, the plugged part and the in-box cable. The count of parts follows the lines fitted and is
never a number in a document.

## One LED a board, fitted, and it blinks — never a standing light, never a DNP footprint

**Every board carries exactly one status LED, fitted, on the pin its table names (`LED`, PD4 on
an H523, the named pin on an H7A3), through 1 kΩ from the 3,3 V rail: ~1,2 mA while lit.** The
firmware owns it and the code is the same everywhere: **one 50 ms blink a minute says alive and
healthy** — 1 µA on average, nothing against a sleeping board's milliwatts; **two blinks say a
fault is flagged in `HEALTH`**; dark is dead or unfed. A standing light is never written: at
2–10 mA it would rival a unit's idle draw. **A bicolour red/green part on a second pin is a
board's own choice** where a builder wants red for the fault; the base is one green.

**No footprint is left DNP for the bench.** A footprint left for later is one nobody ever fits or
ever removes, so a part is either on the board for good or not on it — and the LED count is one,
because a board that allows LEDs collects thirty. The one thing the bench still decides is a
value, not a presence: a flyback's RC snubber (`BRINGUP.md`), fitted from the bench on. A service indicator during a button-gated
service window (the Mayak's) is that board's one LED doing more; diagnostics otherwise travel
the bus and the Handset.

## Data station (the head)

The head of a station is **three named units in one box** — **Mayak** (ESP32-S31: capture /
store / uplink, the roster table), **Kronos** (the clock) and the **Bifrost(s)** (the bridges
the NodBus proper starts behind — the head has no direct multidrop). It is **not**
specific to any front: the same head serves a Quake, a Palatine or a Sputnik station, learning each unit's
type at discovery. The board records:
`../mayak/HARDWARE.md` · `../kronos/HARDWARE.md` · `../bifrost/HARDWARE.md`.

```
THE HEAD — three boards in one box, and every wire between them


   Polaris or a Sputnik's time port — NMEA + PPS — lands on the CLOCK BOARD
                          │
                          │  a data body, B_DIR low at Kronos: channel B carries the PPS in
                          ▼
        ┌───────────────────────────────────┐
        │             KRONOS                │
        │             STM32H523             │
        │                                   │
        │   the TCXO, the GPSDO loop        │        ┌──────────────────────────────┐
        │   and the PLL                     │        │                              │
        │                                   │        │  PPS-K, buffered ────────────┼──▶ every
        │   the station's ONE NMEA parser   │───────▶│  the second as an edge       │   BIFROST
        │                                   │        │                              │
        │   the only precise oscillator     │        │  the network clock, 2²² ─────┼──▶ every
        │   in the whole station            │        │  handed to the spurs         │   BIFROST,
        │                                   │        │  undivided                   │   and the head
        └───────────┬───────────────────────┘        │                              │
                    │              ▲                 │  the time bus, I²C ──────────┼──▶ every
                    │              │                 │  names the second the edge   │   BIFROST
                    │              │                 │  just marked — and the head  │   and the head
                    │              │                 └──────────────────────────────┘
                    │              │
      PPS-K ────────┘              └──────── the time bus, I²C, multi-master:
      to the head's capture pin              the label down once a minute, the
                    │                        seed and the position up
                    │              ▲
                    ▼              │
        ┌───────────────────────────────────┐
        │             MAYAK                 │  UART 0–3  ═══▶ four MasterNOD trunks,
        │             ESP32-S31             │                 one per Bifrost card
        │                                   │
        │   capture · store · uplink        │  LP I²C ─▶ Hermes ───▶ BMS and MPPT,
        │   the roster table                │            their own bus; internal, never gated
        │                                   │
        │   NO direct multidrop             │  I²C ◀─▶ the time bus, with Kronos and the
        │   NO transceiver                  │           cards: the label in, the seed and
        │   NO ModBus of its own            │           the position out
        └───────────────────────────────────┘
              two microSD cards, one a mirror · the modem · Wi-Fi · BLE · USB


   No link in this box is 485. The trunks are plain logic, the clock and PPS on a crossed
   in-box cable are wires behind a buffer, Kronos's time bus to the cards is driven M-LVDS (DS91C176) and received
   by THVD1450 — a 485 part used only as a receiver — non-isolated, and the two I²C buses
   carry names rather than instants.
```

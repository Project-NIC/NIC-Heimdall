★ N.I.C. ★

# Proteus — the board

> **Design-stage concept — nothing built.** The figures are verified against the parts' sheets
> and the parts before a board is made.

**One PCB, four sections, four documents.** Proteus is not boards joined together: it is one
printed board carrying the station's classic structure as traces. The Mayak section is
`../mayak/HARDWARE.md`, Kronos `../kronos/HARDWARE.md`, the Bifrost `../bifrost/HARDWARE.md`,
Hermes `../hermes/HARDWARE.md` — parts, values and pins as written there. **Polaris is not on the
PCB**: it is bought and plugs into Kronos's `TIME IN 1` as in every station
(`../kronos/polaris/HARDWARE.md`). This document is what joins the sections, what is on the PCB
instead of on a plug-in, and what is left off. The processors do not know the board exists.

## What becomes a trace, and what each trace preserves

| link | in a full Heimdall | on Proteus |
|---|---|---|
| **Kronos → the Mayak and the Bifrost: `CLK±`, `PPS±`** | the M-LVDS ribbon, a tap on each | **differential traces from Kronos's `DS91C176` outputs past the Mayak's `THVD1450` receivers to the Bifrost's.** Kronos's 2× 10 Ω legs and its 80,6 Ω fitted as the first node, the Bifrost's as the last; no `TIME BUS` connector, nothing else ever taps the bus |
| **`SDA` · `SCL` · `ATTN`** | the ribbon | traces; Kronos's 4,7 kΩ pair the only pull-ups; `ATTN` a trace to Kronos's 3,3 V with the Bifrost's pull-down fitted — a Bifrost by construction, never an Argus |
| **Mayak trunk 1 → the Bifrost's `MNB/NB IN`** | the crossed in-box cable | **four traces**: Mayak `TXD` (`GPIO54`) → Bifrost `RXD_UP` PA1 and its capture · Bifrost `TXD_UP` PA0 → Mayak `RXD` (`GPIO55`) · the Mayak's 1,10 kΩ `ID_RET` → the Bifrost's `ID` pin (0,10, a Mayak) · the Bifrost's 1,78 kΩ `ID_RET` → the Mayak's port-1 `ID` pin (0,15, a card) · one ground. No pad for a pull-up |
| **the Mayak's LP I²C → Hermes** | the 8-pin power body, a card on it | **Hermes as a section**: `SDA` · `SCL` (`GPIO6`/`7`), `ALERT` (`GPIO1`), its 3,3 V behind the Mayak's 0,15 A polyfuse, its 0,90 `ID` resistor onto `GPIO46`. Never switched: with no unit on its faces it probes once at boot and sleeps them, milliwatts |
| **Kronos's `TIME IN 1` → Polaris** | the carrier on a data body | **the same: `TIME IN 1` fitted on the board edge, Polaris bought and plugged in**, its `GNSS` 0,35 `ID` read on the socket's pin as in every station; `TIME IN 2` not fitted |
| **the modem** | the Mayak's modem position | **an LTE-M / NB-IoT modem on the UART of the Mayak's trunk 2**, free because the board has one trunk; fed from the Mayak's rail, which the backup cell carries (*The rails*, below); its antenna on the board edge |
| **the Bifrost's port 1 → the run** | a data body and a communication board in it | **the copper island on the board** (*The run*, below): the Bifrost's port-1 `TXD` · `RXD` · `RXD_ECHO` · `DE` · `CLK` · `LINE_EN` as traces into the isolators, the port's 3,3 V into the island's `SN6505B` |
| **the Bifrost's `PWR OUT 1`** | a power body | **kept, the one body on the board**: the isolated 12 V cell plugs in and feeds the run; a surge on the feed burns a module and not the board |
| **the 12 V** | pairs of terminals and fuses per board | **one 12 V node behind the brick** (*The feed*, below), one `5.0SMDJ14A`, every section's draw on it; the plug-in cell takes the node on its own terminals as always |

## The run — on the board

The Bifrost's port 1 is the one run, and the communication board's parts sit on the PCB instead:

| ref | part / value | note |
|---|---|---|
| block S | `SN6505B` · `750313734` · 2× `PMEG10020ELR` · the capacitors | the isolated 3,3 V island, ~100 mA, `EN` on the port's `LINE_EN` |
| U2, U3 | `ISO1452` ×2 | data both directions, and the echo-check receiver |
| U4 | `ISO1450` | channel B, the clock out — `B_DIR` tied to 3,3 V on the board: the source end drives |
| C_D | 100 nF 50 V X7R 0603 ×4 | at each isolator's `VCC1` and `VCC2` |
| R_S1 … R_S6 | 6× 10 Ω, pulse-withstanding, 2512 anti-surge or MELF | two per pair |
| TVS_P1 … TVS_P3 | `SM712` ×3 | across each pair |
| R_T1 … R_T3 | 3× 80,6 Ω 1 % 1206, **fitted** | this is the first board of the segment, the unit the last |
| GDT1 | `2036-07-SM`, **fitted**, its centre strapped to the enclosure's ground point | the source end |
| `LINE CU` | `DGPS2.5R-5.0` ×4, 8 poles | the four pairs: data TX · clock · ground · data RX |

The run is the station's copper NodBus run in every figure (`../galvani/README.md`): 2²² on the
clock pair, 500 m on ~100 Ω UTP Cat 6, 3–5 m of hybrid in a vehicle.

## The feed, the brick and the one measurement

- **The board is fed through an isolated DC/DC brick on the PCB, always**: **`REC10K-2412SAW/H2`**
  (RECOM, 10 W, 9–36 V in, 12 V 833 mA out, 86 %, 1,6 kV for a minute, −40…+100 °C with full power
  to 70 °C, 25,4 × 25,4 × 10,2 mm, 15 g), its input on a two-pole `DGPS2.5R-5.0` behind a
  **slow-blow fuse** — the type the sheet asks, its rating sized by the construction
  (`../daedalus/CONSTRUCTION.md`) — and one **`5.0SMDJ33A`**: a 12 V vehicle's network or a site's
  pack, the same path, ~1 W of loss and nothing to switch.
  - **10 W is the board's load with room**: the sections and the modem ~3 W running, ~7 W at their
    coincident peaks; the run's feed — Steinmetz's ~3,3 W through the 12 V cell — ~4 W. **~7 W
    running, ~11 W at the worst instant**, which the brick's current limit, 150 % of its rating in
    hiccup, passes. A run heavier than the cell's declared 5 W takes the 20 W
    `TEN 20-2412WIR` (Traco, EN 50155) on its own footprint.
  - **Its `UVLO` turns it off at 7,0–7,5 V and on again at 8–9 V**: a cranking dip deeper than that
    drops the board, and the Mayak's backup cell writes, reports and lets go.
  - **`CTRL` open** — on; the one control the board does not need. **EMC to EN 55032 class B with
    the sheet's filter**: 3× 10 µF, 33 µH, a 5 µH common-mode choke at the input, 2× 4,7 nF across
    the barrier. The output carries at most 470 µF.
- **The 12 V node behind the brick carries one `INA238`** on the Mayak's LP I²C beside Hermes,
  `A0` high for 0x41, a 20 mΩ shunt: the whole board's voltage and current into `HEALTH`. **Nothing
  is measured ahead of the brick** — the vehicle's side is the vehicle's, a pack's is the BMS's,
  which Hermes reads.
- **The run's feed is the plug-in isolated 12 V cell on `PWR OUT 1`**, its own `INA238` reading
  the run as on any port; the feed pair leaves on the cell's terminals.

## The rails — the Mayak's own, and one for the rest

**The Mayak keeps its own rail and its backup cell exactly as `../mayak/HARDWARE.md` draws them** —
the buck at 3,43 V, the cell behind its `TPS259474LRPW` — and on that rail sit the Mayak, Hermes,
the PHY and the modem: **the same backup core as every Heimdall**. When the 12 V goes the cell
carries those and nothing else; Wi-Fi and the PHY go off, the head writes, the modem sends, the head
lets go (`../mayak/FIRMWARE.md` §6). **The rail's buck is the `LMR43620`**, as the Mayak draws it
where the modem is LTE-M: the modem's ~0,5 A bursts beside the head's 0,6 A pass the `LMR43610`'s
1 A. On the cell, with Wi-Fi off and the modem keyed only after the flush, the draw stays under the
eFuse's 0,85 A floor.

**One `LMR43620`, 3,3 V, 2 A, for the rest**: Kronos, the Bifrost, Polaris through its socket and
the island's `SN6505B`. Kronos's TCXO, PLL and drivers take their 3,3 V through a 2,2 µH shielded
inductor into 10 µF + 100 nF, as its own buck fed them. They go dark with the 12 V, as in a full
Heimdall; nothing holds them up. The further per-section bucks are not laid out: the one port is
isolated on the board and the one plug-in has its own `INA238`, so a short reaches the buck through
nothing this board owns.

## What is fitted, and what is not

- **Fitted**: the brick with its input terminals and fuse; the four data-pair terminals; the Bifrost's
  `PWR OUT 1`; the Mayak's two SD seats, Wi-Fi/BLE antenna, USB-C receptacle and Ethernet port;
  Hermes's 485 and CAN terminals and its TTL and I²C pin headers; Kronos's `TIME IN 1` for
  Polaris; the modem and its antenna connector; the Mayak's backup cell; the `INA238`.
- **Not laid out**: the LoRa module — the modem is LTE-M; the `PWR HERMES` body, every data body,
  the Mayak's `MNB OUT 2`–`4`, the Bifrost's `MNB/NB IN`, `PWR IN` and ports 2–4, Kronos's
  `TIME BUS` and `TIME IN 2`, the other sections' own bucks, and the 12 V terminal pairs a full
  Heimdall gives each unit.

## Ethernet and USB-C

**Ethernet as `../mayak/HARDWARE.md`, *Ethernet*, draws it**: `DP83826E` on RMII, the
`HR911105A` jack, `TPD4E02B04`, on the pins the Mayak's table reserves. **USB-C as
`../mayak/HARDWARE.md`, *USB-C*, draws it**: the receptacle on the board edge beside the jack,
5,1 kΩ on `CC1`/`CC2`, `TPD4E05U06`, the `PMEG10020ELR` from `VBUS`.

## The form

**A 1-DIN radio slot or a small box.** The board is about 160 × 90 mm with the brick, the three
LQFP100s, the module, the modem, the backup cell and the connectors on one long edge — the
terminals, Polaris's `TIME IN 1`, the modem's antenna, the RJ45 and the USB-C — so a 1-DIN sleeve
(178 × 50 mm) takes it with the front open for the connectors. Placement by the documents' rules:
the brick and the bucks at the input corner, Kronos's TCXO and drivers at the far corner, the
island's isolators at the terminals, the module's antenna at the open face, the cell away from the
brick's heat.

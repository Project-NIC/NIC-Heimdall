★ N.I.C. ★

# Mimir — the board

> **Design-stage concept — nothing built.** The figures are verified against the parts' sheets
> and the parts before a board is made.

**One PCB, six sections, five documents.** Mimir is not boards joined together: it is one
printed board, and on it the station's classic structure — the time bus as M-LVDS pairs, the
trunks and the ports as their UARTs — as traces.

**Mimir is a full station made smaller, and its power is a full station's**: a solar panel, the
pack and its BMS, the MPPT, read by Hermes, as `../core/POWER.md` has them, the pack sized for
Mimir's load. **The board takes the pack's 12 V as it stands, with no converter in front**; every
section makes its own 3,3 V with its own bucks as its document draws them — the house `LMR43610`
and its one-size-up `LMR43620` (`../galvani/HARDWARE.md`, *The buck cell*) — as in every station. A converter at the input is Proteus's, which goes into a vehicle. The Mayak section is `../mayak/HARDWARE.md`, Kronos
`../kronos/HARDWARE.md`, both cards `../bifrost/HARDWARE.md` — the same card twice, one on the
time bus and one off it — Palatine `../palatine/HARDWARE.md` and Hermes `../hermes/HARDWARE.md`; parts, values and pins as
written there. This document is only what joins them and what is left off. The processors do not
know the board exists: each sees the pins, the levels and the `ID` codes its document gives it.

## What becomes a trace, and what each trace preserves

| link | in a full Heimdall | on Mimir |
|---|---|---|
| **Kronos → the Mayak and the Bifrost: `CLK±`, `PPS±`** | the M-LVDS ribbon, a tap on each | **the same pairs as differential traces from Kronos's `DS91C176` outputs past the Mayak's `THVD1450` receivers to the Bifrost's, and on to the board's `TIME BUS` connector**, which is the ribbon's first node for a card outside. Kronos's 2× 10 Ω legs and its 80,6 Ω fitted; the Bifrost's legs fitted and its 80,6 Ω on its jumper — **closed when nothing hangs on `TIME BUS`, open when a ribbon does**, so the last node terminates and no other. The Argus does not touch the time bus. The Bifrost keeps its receivers, so its clock input, its failsafe and its CSS behave as its document says |
| **`SDA` · `SCL`** | the label bus on the ribbon | traces to the Mayak and the Bifrost and on to `TIME BUS`; Kronos's 4,7 kΩ pair the only pull-ups |
| **`ATTN`** | 3V3 on Kronos, a pull-down on a Bifrost or an Argus | **a trace to Kronos's 3,3 V at the Bifrost, its pull-down fitted — a Bifrost by construction; nothing at the Argus, its pull-down fitted — an Argus by construction.** The level is the role, as on a ribbon |
| **Mayak trunk 1 → the Bifrost's `MNB/NB IN`** | the crossed in-box cable, conductors 1–6 of the data body | **four traces**: Mayak `TXD` → Bifrost `RXD_UP` PA1 and its capture · Bifrost `TXD_UP` PA0 → Mayak `RXD` · the Mayak's 1,10 kΩ `ID_RET` → the Bifrost's `ID` pin, **so it reads 0,10, a Mayak** · the Bifrost's 1,78 kΩ `ID_RET` → the Mayak's port-1 `ID` pin, **so the Mayak reads 0,15, a card** · one ground. **No pad for a pull-up on either of the Mayak's trunks** — nothing is stacked on a trace, and the spare trunk carries one card on one cable |
| **Bifrost port 1 → Palatine's up port** | the crossed in-box cable | **six traces, 3–4 and 5–6 swapped**: the Bifrost's `1Y` through its 33 Ω → Palatine's `OSC_IN` (PH0) · Bifrost `TXD` PB6 → Palatine `RXD` PB7 and its PB3 capture · Palatine `TXD` PB6 → Bifrost `RXD` PB7 · Palatine's 2,49 kΩ `ID_RET` → the Bifrost's port-1 `ID` PC0, **so the Bifrost reads 0,20, Palatine, exactly as across a cable** · the Bifrost's port-1 `ID_RET` was never connected, so Palatine's `ID` reads open as it does today · one ground. The Bifrost's `OE_1` still gates Palatine's clock, so `PORT_CLK` works; its `EN_1` reaches nothing, as on a cable. Pins 7–12 of both bodies have no conductor and keep their fitted pulls |
| **Bifrost port 2 → the Argus's `MNB/NB IN`** | the crossed in-box cable | **six traces, the same pattern**: the Bifrost's `2Y` through its 33 Ω → the Argus's `OSC_IN` (PH0) — **the Argus locks to the port's 2²² as a remote Argus locks to its spur's** · Bifrost `TXD` → Argus `RXD_UP` PA1 and its capture · Argus `TXD_UP` PA0 → Bifrost `RXD` · the Argus's 1,78 kΩ `ID_RET` → the Bifrost's port-2 `ID`, **so the Bifrost reads 0,15, a card, and asks** · one ground. `OE_2` gates the Argus's clock, `EN_2` reaches nothing |
| **the Mayak's LP I²C → Hermes** | the 8-pin power body, a card on it | **Hermes as a section**: `SDA` · `SCL` (`GPIO6`/`7`), `ALERT` (`GPIO1`) and its 3,3 V behind the Mayak's 0,15 A polyfuse as traces, Hermes's 0,90 `ID` resistor onto the Mayak's `GPIO46`. Never switched: with no unit on its faces it probes once at boot and sleeps them. Its faces — the 485 and CAN terminals, the TTL UART, the I²C out — on the board edge as its document draws them |
| **the 12 V** | five pairs of terminals, five fuses in the field, five transils | **one pair of `DGPS2.5R-5.0`, an input and a tap, one `5.0SMDJ14A`, one fast fuse** ahead of the board, sized by the construction (`../daedalus/CONSTRUCTION.md`) — the six sections draw ~1 A at most together. **No converter at the input**: the 12 V is the station pack's, and every section takes it through its own buck as everywhere else. The arm power boards and any source board on the cards' ports take the 12 V off the wire on their own terminals, as always |
| **the rails** | Kronos's `LMR43610`, each card's `TPS629206` · `TPS629206` · `LMR43620`, Palatine's `LMR43610`, the Mayak's own | **all kept, one per section as drawn** — a short on an arm, a port or a segment browns out its own buck and not the clock; the bucks are fault isolation, and merging them would undo it |

## What is fitted, and what is not

- **Fitted**: Kronos's `TIME IN 1`, `TIME IN 2` and `TIME BUS` — Polaris, bought, plugs into
  `TIME IN 1` as in every station and is not on the PCB; the Bifrost's `NB/MINI OUT 3` and `4` with
  `PWR OUT 3` and `4` — **two complete NodBus ports**; the Argus's `MINI OUT 1`…`4` with
  `PWR OUT 1`…`4` — **four segments**; Palatine's `MB OUT 1`…`4`, `PWR OUT 1`…`4` and `PWR EXT`; the
  Mayak's `MNB OUT 2` — **the spare MasterNOD, a crossed cable to a Bifrost outside, which takes its
  time from `TIME BUS`**; the Mayak's two SD seats, its Wi-Fi, its modem position and the USB-C
  receptacle; **the Mayak's backup cell, on the Mayak's own rail as `../mayak/HARDWARE.md` draws
  it** — it carries the Mayak, Hermes and the modem and nothing else; Hermes's 485 and CAN
  terminals, its TTL UART and I²C headers; the Ethernet port; the one `12V` pair.
- **Not on the PCB**: the `PWR HERMES` body, because Hermes is a section; the Mayak's `MNB OUT 3`
  and `4`; the Bifrost's `MNB/NB IN`, `PWR IN`, `NB/MINI OUT 1`–`2` with `PWR OUT 1`–`2`; the
  Argus's `MNB/NB IN` and `PWR IN`; Palatine's `NB IN` and `PWR IN`; every time bus tap. Their
  footprints are not laid out.

## Ethernet and USB-C

**Ethernet as `../mayak/HARDWARE.md`, *Ethernet*, draws it**: `DP83826E` on RMII, the
`HR911105A` jack, `TPD4E02B04`, on the pins the Mayak's table reserves. **USB-C as
`../mayak/HARDWARE.md`, *USB-C*, draws it**: the receptacle on the board edge beside the jack,
5,1 kΩ on `CC1`/`CC2`, `TPD4E05U06`, the `PMEG10020ELR` from `VBUS`. **The modem position stays as
the Mayak draws it** — LoRa, or an LTE-M modem where there is no gateway, on the UART of trunk 3,
free because its port is not on the PCB — on the rail the backup cell carries.

## Placement

By the documents' own rules: the bucks along the terminal edge; Kronos's TCXO, PLL and drivers at
the far corner with `TIME BUS` beside the Bifrost's receivers; Palatine's arms down one long edge,
the two NodBus ports, the four segments and the spare trunk down the other, Ethernet and USB at
the Mayak's end with its antennas; the time bus's grounds a thin trace, the supply ground a plane,
as `../kronos/HARDWARE.md` says. A 200 × 120 mm card holds the six sections, the
module and the connectors.

## What it buys, and what it costs

**It buys** one PCB where a full Heimdall has a box of separate units: no ribbon taps and no crossed
cables between them, one fuse, one terminal pair and a smaller enclosure.

**It costs** one failure domain: the one fuse and the one PCB take the head, the clock, both
cards, Palatine and Hermes down together, where a full Heimdall loses one unit. **The backup is the
Heimdall's own, unchanged**: the Mayak's cell writes, reports and lets go; everything else goes dark
with the 12 V, and nothing tries to hold it up. It carries Palatine's and the Argus's
silicon whether or not the site measures weather or sondes. A station that outgrows its two spurs
and four segments adds one Bifrost on the spare trunk, on `TIME BUS`; a station that outgrows that is
not a Mimir. **The firmware does not know it exists** — six processors on six sections, each
seeing the pins, the levels and the `ID` codes its document gives it.

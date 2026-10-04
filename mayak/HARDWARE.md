★ N.I.C. ★

# Mayak — the board

> **Design-stage concept.** The figures are verified against the parts' sheets and the parts
> before a board is made.

**One board: a dual-core ESP32-S31 module, its connectors, its 3,3 V, a backup cell and the modem.** The S31
and no other — the S3 has too few UARTs. The clock is Kronos's, a board of its own beside it.

## What is on the board

| | part | notes |
|---|---|---|
| the head | **`ESP32-S31-WROOM-3U-N16R16V`** | 16 MB flash, 16 MB octal PSRAM at 1,8 V; the external-antenna variant |
| 2,4 GHz antenna | one u.FL/SMA connector off the module | Wi-Fi and BLE are one radio on the S31 and share it: where the site runs a Wi-Fi uplink the antenna is the directional one on the mast (`../core/UPLINK_TRANSPORT.md`), and the handheld at the open enclosure works in its back lobe with 20 dB to spare; where it does not, a small 2,4 GHz stub on the box wall. No separate BLE antenna anywhere |
| 12 V in | **two two-pole terminals**, input and tap — `DGPS2.5R-5.0` blocks, two 2-pole — through its own fuse in the enclosure's fuse field, sized by the construction (`../daedalus/CONSTRUCTION.md`), one **`5.0SMDJ14A`** across the terminals, then a **`PMEG10020ELR`** Schottky | the battery rail as it stands, 10–20 V on the pin. A reversed pack drives the transil forward and the fuse clears; the diode stops the backup cell feeding the 12 V wire back through the buck |
| 3,3 V | **one `LMR43610`** set to **3,43 V**, bulk capacitor at the module — **the `LMR43620` on the same footprint where the modem is LTE-M** | sized for the 0,6 A the module sheet asks of its source, and with an LTE-M modem for its ~0,5 A bursts beside it; no 5 V, no LDO. 3,43 V is the backup cell's float; every part on the rail stands 3,6 V. **The rail's bulk is 3× 47 µF 50 V hybrid polymer** beside the buck's own output capacitors — the reserve that carries the rail through the backup cell's takeover |
| backup cell | **one wide-temperature cell, ~3 Ah** — a low-temperature LiFePO4 or a sodium-ion cell, an 18650, a 21700 or a thin prismatic cell (*Power*) — on the 3,3 V rail through one `TPS259474LRPW` eFuse — the hold, the takeover, a 1 A breaker and the undervoltage cut — charged through a `TPS22917` and 10 Ω | carries the head, Hermes and the modem when the 12 V goes: the flush, the BMS read, the modem's report, then the head lets go and the board goes dark (*Power*) |
| ports to the cards | **four 12-pin data bodies**, wired `TXD` · `RXD` · `ID` · `GND` only; the 1,10 kΩ `ID_RET` resistor. **`TXD` is push-pull and carries no resistor; the pad for a 330 Ω pull-up is on `RXD`, unfitted**, which is the wire the cards' outputs share — the hardware that lets more than one card hang on a trunk port, which the firmware does not implement (*The trunk is a spur*, below) | in-box crossed cables to the Bifrosts; no Galvani board ever hangs here |
| time bus tap | **one 10-pin 2×5 IDC** on Kronos's ribbon, **two `THVD1450` receivers, one a pair** (`RE#`, `DE`, `D` tied low), the line block (2× 10 Ω per pair, jumpered 80,6 Ω) | the head takes PPS-K and the I²C label; **the clock is received and routed to `GPIO38`, and the firmware does not use it** — it is there for a build that wants it |
| Hermes | **one 8-pin power body**, a 0,15 A polyfuse in its 3,3 V | the LP I²C, `ALERT`, `ID` |
| enclosure climate | **one `SHT45`** on the board, on the same LP I²C (address 0x44) | temperature and humidity of the box once a minute into SOH — a humidity trend is the one early warning of a seal letting go, and the dew point against board temperature says condensation before corrosion does. Read by the LP core too |
| storage | **two microSD sockets, both 1-bit** — slot 0 on `SD_D0` · `SD_CLK` · `SD_CMD`, slot 1 the mirror on `GPIO35` · `39` · `40` | one mode, one driver, either card can be primary; ~5 MB/s each against the head's 1 MB/s input ceiling. Pull-ups on `CMD` and every `DAT` at both sockets |
| the modem | **one module, chosen per site**, its own antenna connector — LoRa (`SX1262`) where a gateway is in range, `LR1121` for a satellite path, an LTE-M modem or a satellite module on a UART (below) — **always on the rail the backup cell carries** | the thin values frame and the last report; nothing of the archive |
| service | **USB Serial/JTAG on a 1×4 pin header** — `VBUS · D− · D+ · GND`, the motherboard USB-debug header — and a **cable, four pins to a USB-C plug**; ESD array on `D+`/`D−`; **a `PMEG10020ELR` Schottky from `VBUS` into the buck input**, `VBUS` sensed on one GPIO | on the far end it is a virtual COM port, nothing else: a phone with OTG when the software is good, a laptop otherwise. The way into a head whose firmware is down, and **a dead station runs from a phone's OTG or a laptop over the same cable** — service mode, console only, Wi-Fi and BLE off. **No JTAG pads, no `TX0`/`RX0` pads.** **`EFUSE_DIS_USB_JTAG` is never burnt** |
| the button, the status LED | one each | the service window |

**The trunk is a spur, and the hardware keeps it that way — for four resistors.** Every NOD link in
the station is the same link: same framing, same TDMA, same clock discipline. A mini-NOD differs
from a NOD in the width of its payload and in nothing else; a MasterNOD link differs from a spur in
carrying the absolute second.

**What is laid out, and it is the whole provision: a pad for a 330 Ω pull-up to 3,3 V on each trunk
`RXD`, four in all — unfitted.** No build of the station hangs two cards on one port, so the
resistors are not soldered on any board; a build that stacks cards fits four 0603s and nothing else.
Nothing else on this board changes. **The resistor sits where the talkers meet, and that
is the head's `RXD`** — every card's `TXD` lands there, so that is the one wire on the link that
more than one driver ever shares. **The head's own `TXD` takes no resistor and stays push-pull**:
it is the single talker on its line however many cards hang off it, and it drives every card's
`RXD` straight. **The multipoint build is then a software switch on the CARDS** — each card's
`TXD` from push-pull to open-drain — **and a different cable.** The existing wiring survives
unaltered: every card's `RXD` on the head's `TXD`, every card's `TXD` on the head's `RXD`.

**What the pull-ups cost in the base build**: they load a point-to-point line that does not need
them and they draw 10 mA per line while a card holds it low, 40 mA with all four down — **and only
while a card is talking**, because a UART idles high and a released open drain lets the resistor
hold the line with no current in it. That is paid so the board never has to be redrawn.

**The cable is short, and the rung on this link may be raised.** In the multipoint build the cable
is **40 cm at most**; the resistor against the cards' input capacitance is what sets it and nothing
inside the enclosure wants more. **This link is the one in the station with neither a 485 barrier
nor a laser on it**, so its rung is not held at the spur's number: a build that wants it may take
it to **2²² = 4 194 304 Bd**, and 330 Ω against ~40 pF is **13,2 ns of a 238 ns bit — 5,5 %**,
which carries it. A smaller resistor is not the way to buy that: 220 Ω would be 15 mA a line and
the H523's low level is guaranteed at 8 mA, so it spends `V_OL` margin to shorten an edge that is
already short enough.

**In the multipoint build the `ID` resistors fall together, and the head asks anyway.** Two cards
on one port present their `ID` on the same two pins, so what the head reads is one number made of
two resistors in parallel and it is not either card's code. That costs nothing, because **an `ID`
names the interface and never the partner**: the head reads what kind of thing it is facing and
then asks in the dialogue that follows — which is what the sweep does on every other link in the
station. The parallel value is never decoded and no position on the scale is spent on it.

**The head does NOT deal the cards their epoch, and never could do it as well as they already
have it.** Two cards on one trunk port are frequency-locked exactly — both take Kronos's 2²² on
`OSC_IN` off the same ribbon — and epoch-locked, each anchoring its second on the same `PPS_K`
edge from the same pair. They agree on the second boundary to the tick with the head silent.
**What the head deals is the slot assignment**, a message at enrollment; **what its own PPS-K
capture is for is policing** — knowing where the second boundary is, so a card that talks out of
turn is caught. That capture is already fitted (*The time bus tap*, above): nothing is added for
this.

**Two cards to a port, and the limit is the slot budget, not the wire.** The trunk runs 2²¹
8N1 and a card serves up to eight units, so at 128 frames/s one card is 8 × 48 B × 128 = 491 kbit/s
on the wire, **23 %**; two cards **47 %**, three **70 %** — and the deterministic gaps are the
head's injection window and what makes fault detection free. Two leaves that window intact and
takes the station from 32 units to 64. **The rise time is not what binds it**: the master, two
cards with their echo receivers and a short in-box ribbon come to ~40 pF against 330 Ω, 13 ns,
under 3 % of a 477 ns bit.

**At 2²² the same wire carries four**: one card is 12 % of it, two 23 %, four 47 % — the budget two
cards take at 2²¹. **A build that joins cards frees ports**: two cards on each of two ports leave
two trunk UARTs for something else, four on one leave three. **The base build keeps one card a
port, and the reason is the failure domain, not the budget**: a card whose `TXD` sticks low holds
the shared line, the head cannot cut it — a card has no `ENABLE` above it — and every card on that
line goes silent. On its own port a stuck card costs its own eight units; on a shared one it costs
every card behind it, and with four on one it costs the station. **The base firmware does not
implement the multipoint build**; the copper is left able so that a build that wants it writes
software and not a board.

**Whatever sits in the modem position is fed from the 3,3 V rail the backup cell carries** — LoRa, a
satellite module or an LTE-M modem — so the last report leaves when the 12 V goes; a module that
wants another voltage takes it through its own converter from that rail, and a radio fed from
anywhere else is not the modem.

**The modem position — one module, on `SPI2` or on a UART, one of five.** `NSS` · `MOSI` · `SCK` · `MISO` on
the IOMUX pins and `NRESET` · `BUSY` · `DIO1` beside them, the pins of the table below; its own
antenna connector. **The local link is Wi-Fi** — the S31 carries it, and a directional antenna of a
few hundred crowns reaches an access point kilometres off — so the modem position is for what
Wi-Fi cannot reach:

| module | bus | what it buys | where |
|---|---|---|---|
| **`SX1262`** | `SPI2` | LoRa to a gateway, the 16 B values frame | anywhere a gateway is in range — the cheapest |
| **`LR1121`** | `SPI2` | LoRa, and LR-FHSS straight to a satellite that receives it — the values frame from anywhere, a few hundred bytes a day — about six messages, so the heartbeat stretches to ~4 h and the hourly live packet does not fit | remote islands, open sea, no gateway |
| **Iridium `9603N`** | a UART | short-burst data to Iridium, global, billed per message | a site with nothing else, and a trunk UART freed by joining cards |
| **Kinéis `KIM1`** | a UART | Argos-class satellite IoT, markers only | as above |
| **an LTE-M / NB-IoT modem** | a UART | the live tier over an IoT tariff, never the archive (`../core/UPLINK_TRANSPORT.md`) | mobile cover and no gateway — a vehicle, Proteus |

**LTE-M modems to choose from** — the builder's pick; each runs from the backup cell's rail and
none falls back to 2G:

| module | networks | supply (`VBAT`) | note |
|---|---|---|---|
| Quectel **`BG95-M1`** · **`BG95-M2`** | LTE-M · LTE-M + NB-IoT | 2,2–4,35 V | the widest window; the `-M3` only with its 2G off |
| SIMCom **`SIM7080G`** | LTE-M + NB-IoT | 2,7–4,8 V | common on maker boards |

**Not taken**: a module whose RF supply starts above ~2,9 V — Quectel's `BG77` and `BG770A`, whose
`VBAT_RF` starts at 3,1 V, and Nordic's `nRF9160`, which runs from 3,0 V but meets 3GPP only from
3,1 V and its full RF figures from 3,3 V — and any module that keeps a 2G fallback, whose GSM
burst is 2 A.

**The UART modules need a UART this board does not have free**: the four HP UARTs are the
trunks, and the LP UART's `TXD`/`RXD` sit on `GPIO6`/`GPIO7` (module sheet, the pin table), which
are the LP I²C to Hermes. They go on a trunk UART freed by joining cards (*The trunk is a spur*,
above), **or, where all four trunks are used, on the modem position's SPI through an SPI-to-UART
converter on the modem's cable** — the converter belongs to the modem's hookup, not to the board,
and the firmware carries its driver. **Not taken, and the reasons are the budget**: **Starlink**, whose dish draws 15–100 W for
as long as it is on — more than the whole station; **an LTE modem for the archive**, because an
ordinary unlimited mobile tariff costs roughly **600 Kč a month per SIM** — a few hundred stations
make that over a hundred thousand crowns a month — and an IoT tariff is sized in hundreds of
megabytes a month where the archive is gigabytes a day.

**What the satellite positions cost, roughly:** Iridium short-burst data ~**400 Kč a month per
module** for the line, plus ~**4 Kč per message** of up to 50 B — ~80 000 Kč per megabyte; Kinéis
~**25–125 Kč a month per device** for up to 96 messages of 19 B a day; LoRa straight to a
satellite ~**250 Kč a year per device** where the service is priced. All three carry the values
frame, never the archive.

**Not on the board:** any 485 or CAN transceiver, any feed voltage, Ethernet, an RTC, a display.

## The board in the box

```
  the battery, 12 V ──▶ its own terminals, in + tap ──▶ LMR43610 → 3,3 V ─▶ ESP32-S31 head
  (12–13,5 V, the pin      behind a Schottky,                             │
   takes 10–20 V)          PMEG10020ELR                                   └─ 4× UART = MasterNOD
                                                                             links ─▶ Bifrost(s)
                                                                             (capture / store /
                                                                              uplink; the head has
                                                                              NO direct multidrop)

  THE SAME BATTERY, ON THE WIRE, NOT THROUGH THIS BOARD

    every Bifrost card ──▶ its own terminals on the same wire; a station feed cell
                           taps the wire on its own terminals and makes the feed
                           there — 48 or 300 V is a Galvani board's, never the head's, and
                           no board carries the 12 V across itself for another
    KRONOS ─────────────▶ its own input and its own LMR43610 (../kronos/HARDWARE.md)
                           └─ the M-LVDS time bus, one ribbon: clock and PPS-K ─▶ Bifrosts + the head

   GPS module (local) ── PPS + time stream (UART) ──▶ KRONOS (the one NMEA parser);
   Kronos then EMULATES a GPS to the head: the PPS-K wire + the Unix label on the I²C time bus
```

**Input is the station battery as it stands** — a LiFePO4 pack at **12–13,5 V**, the pin rated
**10–20 V** and the design reserve 11–15 V; no fixed rail and no station bus voltage
(`../core/POWER.md` §7, `../galvani/HARDWARE.md`, *The battery*). It arrives on the **two two-pole
terminals every 12 V board carries — `DGPS2.5R-5.0` blocks, two 2-pole, an input and a tap — through
its own fuse in the enclosure's fuse field, sized by the construction, a `5.0SMDJ14A` across them,
then a `PMEG10020ELR` Schottky** — the transil and the fuse every 12 V input carries
(`../galvani/README.md`, *The rails*), the diode the Mayak's own, for its backup cell. Mains is NOT
on this board.

**Isolation is spent where the cable begins — on the two Galvani boards of a port:** the feed
barrier on the power board, the data barrier on the communication board; a run floats from the
station's battery ground (`../galvani/README.md`). **Everything local stays non-isolated** — the ESP, the clock board and their
3,3 V come off plain house bucks on the 12 V. Don't isolate what doesn't leave the box.

## The head — ESP32-S31

- **Module: ESP32-S31-WROOM-3** — **dual-core RISC-V** (the capture-core / comms-core split,
  `FIRMWARE.md` §1), **Wi-Fi 6 + BT 5.4 LE in silicon** (BLE = the button-gated
  commissioning link; Wi-Fi = bulk archive uplink / NTP / OTA), a Gigabit Ethernet MAC that this
  board does not bring out and Proteus and Mimir do, over RMII on the pins the table reserves
  (*Ethernet*, below) — and **4 HP UARTs
  + 1 LP UART**: every serial job gets a native port (budget below) — the S3's three could not,
  which is why no S3 stands anywhere on this board (`WHY.md`). **`ESP32-S31-WROOM-3U-N16R16V`: 16 MB quad flash, 16 MB octal PSRAM** (1,8 V, 200 MHz), the one
  external-antenna variant in the v0.7 ordering table; −40 to 85 °C with the octal PSRAM, no derating.
  The archive's ring buffer + Wi-Fi stack want the 8 MB class, so it clears it. **The chip and the WROOM-3 ship since July 2026; the `-3U` entered the module sheet at v0.6
  (August 2026), and the sheet is still preliminary at v0.7 (2026-09-03)** — every figure here is
  read from that v0.7 and **taken as sufficient**: the design leans only on the sheet's own limits —
  the 3,0–3,6 V window, the 0,6 A source, the pin table — with margin, and a builder re-reads them
  against the first release sheet before ordering. The board is drawn on the
  WROOM-3 footprint, castellated and soldered down, and takes the `-3U`. **Supply 3,0–3,6 V and the
  source must deliver 0,6 A** (sheet §6.2); Wi-Fi TX peaks 295 mA at 17 dBm; external-antenna variant for a sealed enclosure.
- **Two microSD cards** on the SD/MMC host, **both in 1-bit** — slot 0 on the `SD_*` pins, slot 1 a mirror (the store-and-forward buffer) — the card IS the uplink
  buffer; a brownout flush needs the backup cell below.
- **The modem** — an SPI module on `SPI2` (LoRa, `LR1121`) or a UART module on a freed trunk — the 16 B values frame out, and the last report on a lost 12 V.
- **No RS-485 transceiver on the head** — and none on any other board in the box either: the
  trunk UARTs are plain-level MasterNOD links to the Bifrost cards, and 485 starts on a Galvani
  board at a Bifrost port, never before it (`../galvani/README.md`).

### UART budget — S31: every serial job a native port

The S31 carries **4 HP UARTs + 1 LP UART**:

The ports are labelled for a head that **detaches from the NodBus** — each trunk is a
**point-to-point MasterNOD link to a Bifrost**, the NodBus proper starts BEHIND the Bifrosts,
and the unit ceiling is the cards' (eight per card — `../bifrost/README.md`):

| channel | job |
|---|---|
| UART0–3 | **4× MasterNOD link** — one per Bifrost trunk (NodBus framing point-to-point; each Bifrost presents its devices ×N) |
| LP UART | **not used** (`WHY.md`) |
| **LP I²C** | **Hermes, the BMS/MPPT converter**: a small H523 card as an I²C slave on a **power body**, one block read a minute, never gated; its far side is **485 Modbus RTU on `THVD1450`**, a plain 3,3 V UART the second face, I²C out and CAN (`TCAN334`) fitted and asleep until a part wants them; 3,3 V from this board through a 0,15 A polyfuse, the H523 at 4 or 8 MHz with the unused peripherals off (`../hermes/README.md`). **On the LP I²C because the LP core polls it in survival mode** — the LP I²C is a master, which is all this link needs, and `ALERT` is on an LP GPIO so the LP core wakes on it. **The `SHT45` sits on the same bus as a second slave**, on the board, no pin of its own |
| **GPIO: PPS-K → timer capture** | **THE time transfer in — the only one.** The PPS-K edge (off the time bus's PPS pair, through the head's own `THVD1450` receiver), captured by the head, is what marks the second; the bus label only *names* the second the edge already marked. |
| I²C — the time bus | **ESP ↔ Kronos, multi-master, one bus in a star with the cards.** Kronos writes the label once a minute, and on a change of quality, to all; the head writes when it has something — the coarse seed, the position, a remote change — and reads quality and status as registers. Not a time path; nothing time-critical ever rides it (gps-pps). |
| USB-Serial-JTAG | console / service, on the 1×4 header; the same cable powers a dead head (*Power*) |

**No direct multidrop on the ESP's pins**: a branch is a card's spur.

**DMA closes with margin** — the two big names take nothing from the pool:

| consumer | channels | pool |
|---|---|---|
| 4× trunk UART, continuous | 4 TX + 4 RX | GDMA-AHB (5+5) → one pair spare |
| AES/SHA bursts, the uplink's crypto | the spare pair | GDMA-AHB |
| LP I²C → Hermes | none — one block read a minute from the FIFO | — |
| SPI → the modem | 1+1 or none — tens of bytes a transaction | GDMA-AXI (3+3) |
| SD host → the cards | the host's own descriptor DMA | own |
| I²C → Kronos | none — a register read a minute from the FIFO | — |
| Wi-Fi + BLE | none — the MACs have their own DMA outside GDMA | own |

**What the S31 carries, from its v0.7 module sheet:** 4 UART + 1 LP UART · 2 I²C + 1 LP I²C ·
2 PCNT · 2 GP SPI + an SDIO host · 4× 54-bit + 1× 52-bit timers · no I3C · a CAN FD controller,
unused. **The TRNG, the AES/SHA/ECDSA accelerators, the Key Manager and secure boot** carry the
K0/K1 station-key scheme in hardware.

## The connectors and the pin budget

**Connectors: `MNB OUT 1`…`4` · `PWR HERMES` · `TIME BUS` · `12V`** (`../galvani/README.md`, *Connector names*).

**Seven connectors and two terminals.** Four **12-pin data bodies**, one per card, wired to what
the crossed in-box cable carries and nothing more — **a Bifrost sits beside the Mayak in the same
enclosure, and that is a rule of the build**: no Galvani board and no remote card ever hangs on
a head port, so the port carries no line-side pins. A tap on Kronos's **time bus ribbon** — the
same line block as a card: 2× 10 Ω per pair and a jumpered 80,6 Ω for the case the Mayak is the
last node. One **8-pin power body** for Hermes on the LP I²C, its 3,3 V behind a 0,15 A polyfuse. The two 12 V terminals; the USB header, the two SD seats,
the modem and its antenna connector on the board.

| connector | pins wired to the S31 | pins on the socket that land on nothing at the head |
|---|---|---|
| **card data body ×4** | `TXD` **push-pull, no resistor** · `RXD` (UART0–3) **with an unfitted pad for the 330 Ω pull-up** · `ID` (ADC) — **3 per port, 12 in all**; `ID_RET` is the 1,10 kΩ resistor; `GND` (*The card data body*, below) | `CLK/PPS` · `RXD_ECHO` · `DE` · `B_DIR` · `LINE_EN` · `SD` · `3,3 V` — the socket takes only a crossed cable |
| **Kronos time bus tap** | `SDA` · `SCL` (HP I²C 0, multi-master) · `PPS_K` (timer capture, behind its `THVD1450` receiver) · `CLK` (`GPIO38`, behind its own `THVD1450`, unused by the firmware) · `ATTN` (GPIO in) — **5** | — |
| **Hermes, power body** | `SDA` · `SCL` (**LP I²C**, `GPIO7` · `GPIO6`) · `ALERT` (`GPIO1`, an LP GPIO — Hermes's attention, the LP core's wake) · `ID` (ADC, reads 0,90) — **4** (*The Hermes power body*, below) | `ENABLE` — the link is never gated |
| **microSD ×2 — a mirror, both 1-bit** | slot 0: `D0` · `CLK` · `CMD` on three of the module's **dedicated `SD_*` pins** (`SD_D0` · `SD_CLK` · `SD_CMD`, outside the 48) — **0 of the 48**. **Slot 1, the mirror**, on its own IOMUX pins `GPIO35` · `39` · `40` (`SD2_CDATA0` · `SD2_CCLK` · `SD2_CCMD`) — **3**. One mode for both, so either card can be primary. Every record is written to both; a card that fails is reported and the other carries on (`FIRMWARE.md` §6) | `SD_D1`–`D3` (`GPIO21`–`23`) — free |
| **the modem on `SPI2`** — LoRa `SX1262` or `LR1121` | SPI: `SCK` · `MISO` · `MOSI` · `NSS` + `BUSY` · `DIO1` · `NRESET` — **7** | |
| **the button** | BLE gate — **1** | |
| **the status LED** | the station-wide blink in normal running (`../core/HARDWARE.md`), lit through the service window (`FIRMWARE.md` §10) — **1** | |
| **USB, 1×4 pin header** | `USB_DP` · `USB_DM`, the dedicated pair, not GPIO; `VBUS` sense — **1** (`GPIO49`, ADC) | |
| **the backup cell** | hold · charge · on the cell · the cell's current (`GPIO50` · `51` · `52` · `53`, the last an ADC) — **4** | |
| **total** | **38 of the 48 GPIO**, of which **7 ADC-capable** (the four port `ID`s, Hermes's, the `VBUS` sense, the cell's current), plus the six `SD_*` pins | |

### `MNB OUT 1`…`4` — the card data bodies

Four identical sockets, ports 1–4, one per card. **The socket takes only a crossed in-box cable
to a card's up port** — conductors 1–6, with 3–4 and 5–6 swapped — so a pin that the six-way
cable does not carry, or that only a Galvani board would drive, lands on nothing.

| pin | signal | on this board |
|---|---|---|
| 1 | `CLK/PPS` | not connected — the head drives no clock and takes none off a card; its second is `PPS_K` off the time bus |
| 2 | `GND` | ground |
| 3 | `TXD` | the port's UART TX (`GPIO54` · `14` · `16` · `56`), **push-pull, no resistor** — the one talker on its line |
| 4 | `RXD` | the port's UART RX (`GPIO55` · `36` · `17` · `57`), **an unfitted pad for 330 Ω to 3,3 V** — the wire the cards' outputs share |
| 5 | `ID` | ADC (`GPIO42` · `43` · `44` · `45`), **10 kΩ 1 % to the 3,3 V rail**; reads the card's `ID_RET`, 0,15 |
| 6 | `ID_RET` | **1,10 kΩ to ground** — the Mayak, 0,10, landing on the card's `ID` |
| 7 | `RXD_ECHO` | not connected — a master never echo-checks |
| 8 | `DE` | not connected — no Galvani board on a head port, nothing to gate |
| 9 | `B_DIR` | not connected — a crossed cable carries no channel B |
| 10 | `LINE_EN` | not connected — no line side on a crossed cable |
| 11 | `SD` | not connected — no optical module on a crossed cable |
| 12 | `3,3 V` | not connected — the card makes its own rails and nothing on the cable takes power |

### `PWR HERMES` — the Hermes power body

One 8-pin power body, Hermes and nothing else on it.

| pin | signal | on this board |
|---|---|---|
| 1 | `ENABLE` | not connected — the link is never gated; Hermes leaves the pin unpopulated |
| 2 | `GND` | ground |
| 3 | `ID` | ADC (`GPIO46`), **10 kΩ 1 % to the 3,3 V rail**; reads 0,90 |
| 4 | `SDA` | LP I²C `SDA` (`GPIO7`), **4,7 kΩ to 3,3 V** — the bus's one pair of pull-ups, shared with the `SHT45` |
| 5 | `A_SEL` | **strapped to ground** |
| 6 | `SCL` | LP I²C `SCL` (`GPIO6`), **4,7 kΩ to 3,3 V** |
| 7 | `ALERT` | `GPIO1` (`LP_GPIO1`, the LP core's wake), **100 kΩ to ground, high = alarm** — an empty socket reads quiet |
| 8 | `3,3 V` | the board's 3,3 V rail — the `LMR43610` or the backup cell behind it — **through a 0,15 A polyfuse**, so a fault on Hermes cannot take the head's rail |

**What the module has:** 48 IO pins brought out on the `-3U` (`IO0`–`IO19`, `IO33`–`IO40`,
`IO42`–`IO57`, `IO60`, `IO61`, `TX0`, `RX0` — sheet §3), four of them strapping pins (`GPIO36`,
`GPIO37`, `GPIO60`, `GPIO61` — §4) that take only a line whose reset level is harmless, and
six `SD_*` pins beside them. **38 of 48 here, and the first SD on three of its own six.** Ethernet is not brought out on this board — the uplink is Wi-Fi and the modem, and
setting up is the handheld (a phone, a tablet, a laptop) over BLE or Wi-Fi; the RMII pins are reserved for the boards that do. A card's off, and any power
state of a port, is a command over the link, not a pin. **Several cards on one port** — the case
the hardware allows and the firmware does not implement — would put their `ID_RET` resistors in
parallel and make the `ID` read meaningless; a firmware that ever does it treats an off-scale
`ID` as "sweep the port and see who answers", not as a fault. **The pins, one by one** — sheet §3 of the module datasheet (v0.7) for the functions, §4 for
the straps. The four UARTs, the HP I²C and the capture go through the GPIO matrix, which is
what every S3-class UART does anyway and costs nothing at 2 Mb/s (the UARTs run to 5 MBaud);
the modem's SPI sits on `SPI2`'s IOMUX pins and the SD on the `SD_*` pins. **The rule of the
assignment: the LP domain first** — the survival loop needs Hermes, `ALERT` and the button, and
those three take the LP pins first — `GPIO0`, `1`, `6` and `7`; then the non-ADC pins for everything digital,
the ADC block for the five `ID` reads, and the straps left alone.

| GPIO | job | why this pin |
|---|---|---|
| `GPIO0` | **the button** — the BLE gate | `LP_GPIO0`: the LP core reads it in survival mode |
| `GPIO1` | **Hermes `ALERT`** | `LP_GPIO1`: the LP core's wake |
| `GPIO2` | modem `NRESET` | |
| `GPIO3` | modem `BUSY` | |
| `GPIO4` | modem `DIO1` | |
| `GPIO5` | Kronos time bus `ATTN` | read once at boot; the level says a card is a Bifrost, on the head it is only mirrored |
| `GPIO6` | **LP I²C `SCL`** — Hermes and the `SHT45` | `LP_I2C_SCL` — the LP I²C, master only, which is all the link is |
| `GPIO7` | **LP I²C `SDA`** — Hermes and the `SHT45` | `LP_I2C_SDA` |
| `GPIO8` · `GPIO9` | **reserved — RMII `TXD0` · `TXD1`**, unconnected on this board | the Ethernet pins are IOMUX-fixed on the S31 and are left to the boards that bring the MAC out (*Ethernet*, below); the trunks and the modem's SPI go through the matrix and can sit anywhere |
| `GPIO10` · `GPIO11` | **modem `NSS` · `MOSI`** | `SPI2_CS` · `SPI2_D`, IOMUX; `SCK` and `MISO` sit on `GPIO58`/`59` through the matrix, which carries the module's ≤ 16 MHz |
| `GPIO12` · `GPIO13` | **reserved — RMII `TXEN` · `RMII_CLK`**, unconnected on this board | |
| `GPIO14` | **port 2 `TXD`** — UART1 | |
| `GPIO15` | **reserved — RMII `CRS_DV`**, unconnected on this board | |
| `GPIO16` · `GPIO17` | **port 3 `TXD` · `RXD`** — UART2 | |
| `GPIO18` · `GPIO19` | **reserved — RMII `RXD1` · `RXD0`**, unconnected on this board | |
| `SD_D0` · `SD_CLK` · `SD_CMD` | **microSD slot 0, 1-bit** — the primary | `GPIO20` · `24` · `25`, dedicated pins outside the 48 |
| `SD_D1`–`SD_D3` | *free* — `GPIO21`–`23`, dedicated pins outside the 48 | the socket's `D1`–`D3` get their pull-ups on the board and are not routed to the chip |
| `USB_DP` · `USB_DM` | **USB Serial/JTAG** — console and service, on the 1×4 header | the dedicated pair, not GPIO |
| `GPIO33` · `GPIO34` | **time bus `SDA` · `SCL`** — HP I²C 0, multi-master | |
| `GPIO35` · `GPIO39` · `GPIO40` | **microSD slot 1, 1-bit — the mirror**: `D0` · `CLK` · `CMD` | `SD2_CDATA0` · `SD2_CCLK` · `SD2_CCMD`, slot 1's own IOMUX pins. 1-bit like slot 0, and it leaves `GPIO36`/`37` untouched |
| `GPIO38` | **the time bus `CLK`**, 2²², behind its `THVD1450` receiver — **wired and unused by the firmware**: the head's time is PPS-K and the label, and the clock is there for a build that wants to count it | `SD2_CDATA3`, no strap; any MCPWM capture or PCNT unit reaches it through the matrix |
| `GPIO36` | **port 2 `RXD`** — UART1 through the matrix | an input with no pull, so the `VDD_SPI` strap it carries reads its own weak level at reset |
| `GPIO37` | **PHY reset**, on a board with Ethernet; unconnected here | |
| `GPIO36` · `GPIO37` | **the two straps** — `SD2_CDATA1`–`2`, kept off the second SD slot | `GPIO36` straps `VDD_SPI`, the module's internal flash/PSRAM rail, which on the `-N16R16V` **must be 1,8 V** (the `V` is the 1,8 V octal PSRAM) — a pull-up reading high at reset would put 3,3 V on it unless the module's `EFUSE_PMU_FLASH_POWER_SEL_EN` is burnt, which the module datasheet does not say; `GPIO37` straps the JTAG source, harmless either way. **Every IO is 3,3 V regardless** — the strap never touches the IO rail — and SD at 3,3 V signalling is what allows two cards at all (the host's one-card limit is for 1,8 V UHS-I signalling, sheet §5.2.2.10) |
| `GPIO42` · `GPIO43` · `GPIO44` · `GPIO45` | **port 1–4 `ID`** | `ADC1_CH0_N` · `CH0_P` · `CH1_N` · `CH1_P` |
| `GPIO46` | **Hermes `ID`** | `ADC1_CH2_N` |
| `GPIO47` | **`PPS_K` capture** — MCPWM capture through the matrix, behind the head's `THVD1450` receiver | the matrix adds a fixed few ns, and the head's PPS-K is a second mark, not a ranging edge |
| `GPIO48` | **the status LED** | one 50 ms blink a minute in normal running, two on a fault; lit through the button-gated service window |
| `GPIO49` | **`VBUS` sense** — a divider off the USB header | ADC; tells the firmware it is on USB power: service mode, radios off |
| `GPIO50` | **backup hold** — the `TPS259474LRPW`'s `EN/UVLO` through 90,9 kΩ, 56,2 kΩ to ground | high after boot, released after the last report; the 56,2 kΩ keeps the part off while the head is in reset |
| `GPIO51` | **backup charge** — the `TPS22917` on the cell's path in | off below −20 °C on the `SHT45` and while on the cell |
| `GPIO52` | **on the cell** — the `TPS259474LRPW`'s `PG`, 100 kΩ up | high when the cell carries the rail; an interrupt |
| `GPIO53` | **the cell's current** — the `TPS259474LRPW`'s `ILM` | ADC; 0,604 V/A across the 3,32 kΩ, ~0 while the buck runs |
| `GPIO54` · `GPIO55` | **port 1 `TXD` · `RXD`** — UART0 through the matrix | the pad JTAG, not brought out: the board debugs over USB |
| `GPIO56` · `GPIO57` | **port 4 `TXD` · `RXD`** — UART3 | the other two JTAG pads |
| `GPIO58` · `GPIO59` (`TX0` · `RX0`) | **modem `SCK` · `MISO`** through the matrix | the ROM console's pins; the ROM prints there at reset into an SPI module whose `NSS` is high, which ignores it |
| `GPIO60` · `GPIO61` | **`MDC` · `MDIO`** on a board with Ethernet; unconnected here | the boot straps, weak pull-up = SPI boot: `MDIO` carries its own 1,5 kΩ pull-up, the same direction, and `MDC` is this chip's output and idle until after boot |

**48 of 48 on a board that brings the MAC out, 38 on this one** — the seven RMII pins, the PHY
reset and `MDC`/`MDIO` are unconnected here, and the one pin table serves the Mayak, Proteus and
Mimir, so the head's firmware has one map.

### Time distribution — the three signals, separated

1. **PPS (GPS) → Kronos's capture pin only**; the head takes PPS-K off the time bus (gps-pps.md).
   The absolute anchor: the UTC second mark. The H523 disciplines the TCXO/PLL against it; the
   ESP marks its second with PPS-K.
2. **NMEA (GPS) → the CLOCK BOARD (final).** The master's ONE parser lives
   on the H523 — validation where the holdover decision lives. The ESP carries no GPS code;
   it reads labels + quality from the I²C register map (below). Never a time path.
3. **The network clock (Kronos's MCO, 2²²) → the Bifrosts, and received at the head without
   being used.** The Bifrosts stamp the records and count the wire; the head's whole need is **the
   date and the second**, and that is PPS-K + the lazy label. The clock pair has its receiver and
   lands on `GPIO38` so that a build which wants the network clock at the head takes it in
   software and changes no board (all clock numbers: [`nodbus.md`](../core/blocks/nodbus.md), *The network clock*). "The GPS dies —
   where does the head get its seconds?" is answered by PPS-K itself: **it never stops** —
   in holdover Kronos keeps deriving it from the **station TCXO (±1 ppm)**, so the head's
   second keeps arriving on the same wire and the clock-quality SOH flag (locked/holdover)
   marks the data honestly. Between edges the ESP's own crystal drifts µs-class per second —
   orders inside what its housekeeping stamps need; the 1 µs contract lives on the cards.

**The head ↔ Kronos link is the time bus's I²C, multi-master, the wire the label rides**: the head
reads the label, the quality and the position, and writes a coarse seed where Kronos has no source
(`FIRMWARE.md` §5). **Kronos takes its own 12 V and makes its own 3,3 V — this board supplies it
nothing.**

## Power — 12 V in, 3,3 V out, and nothing else on this board

- **This board makes no feed voltage and hands none out.** 48 V and 300 V exist only on a
  Galvani power board, and a station feed cell plugs into **a card's port** and taps its 12 V off the
  wire on its own terminals — never off the head (`../galvani/README.md`,
  *The rails*). The cell itself, its winding and its declared budget are
  the Galvani boards' (`../galvani/HARDWARE.md`); this board's
  concern with it ends at sharing a battery input.
- **Master logic, non-isolated, off the 12 V: one house buck** — the **LMR43610** (the ESP's
  Wi-Fi TX peaks 295 mA, the sheet asks the source for 0,6 A); no 5 V intermediate, no rail, no LDO (the one-converter rule —
  `../galvani/README.md`, *The rails*). Every other host in the box makes
  its own the same way and takes nothing from here.
- **Backup cell — the unannounced loss.** A planned stop is Hermes's: it reads the BMS and the
  head shuts the station down in order before the pack is cut. What is left is the fault the BMS
  takes by itself — a short, an overcurrent trip, a cut wire — and the backup cell covers that:
  the head writes every buffer, reads Hermes for the cause, sends it on the modem, and switches
  itself off.
  - **The cell: one wide-temperature cell, ~3 Ah — twice what the event and the cold ask, and the
    form free.** The chemistry is the condition, not the shape:
    - **a low-temperature LiFePO4** — JYH `LFP18650-1500` cited for its temperature rating, the
      class of chemistry, and not for its size, which is the ~3 Ah above — its sheet giving
      discharge **−40 to 60 °C**, charge **−20 to 45 °C**, 1 C charge and 3 C discharge at most; a
      standard LiFePO4 stops discharging at −20 °C and takes no charge at 0 °C. Its plateau,
      ~3,2–3,3 V, sits inside the module's window across most of its charge;
    - **or a sodium-ion cell** — better in the cold, at its best below zero where LiFePO4 is at its
      worst, but its voltage slopes: floated at 3,43 V and used down to the module's 3,0 V it gives
      the rail only part of its charge, which the event's few percent fits with room.

    **An 18650, a 21700, or a thin prismatic cell** — a flat 3 Ah cell costs a few hundred crowns —
    in a holder or on two terminals on the board. The rail is the cell, with no converter between
    them, so the condition every candidate meets on its sheet is the same: **at −40 °C and 0,6 A it
    stays above 3,0 V for the event**. The head and Hermes on it draw ~0,1 A: **a day at 25 °C, and
    half that still at −40 °C**.
  - **What the cell must carry, the modem on it — the event in two steps, never together.**

    | step | on the cell | for |
    |---|---|---|
    | **the flush** — both cards written | the head and Hermes ~0,1 A, **both cards writing ~0,2 A** — **~0,3 A** | seconds |
    | **the report** — the cause read, the modem sends | the head and Hermes ~0,1 A, **the modem**: LoRa ~0,12 A at 22 dBm · **LTE-M ~0,5 A in its bursts**, ~0,1–0,2 A between them while it attaches — **≤ ~0,6 A at a burst** | LoRa a second; LTE-M up to two or three minutes to attach and send |

    - **The capacity is not the limit.** The worst event — LTE-M attaching for three minutes, five
      minutes in all at ~0,25 A on average — takes **~20 mAh against ~3 Ah: under 1 % at 25 °C,
      ~1–2 % at −40 °C**, and a sodium-ion cell's usable part above 3,0 V still holds it many times. It ages on the calendar, not on cycles.
    - **The cell's current is not the limit.** A 3 Ah cell gives amps; the event asks 0,6 A.
    - **The breaker is a limit, and the order keeps under it.** The cards and an LTE-M burst
      together would be ~0,8 A, at the eFuse's 0,85 A floor; **the modem keys up only once both
      cards are written** (`FIRMWARE.md` §6), so the most the cell gives is ~0,6 A. A burst over
      the floor shorter than the 51 ms blanking passes anyway.
    - **The voltage at −40 °C is the tight figure.** The cell's resistance, ≤ 40–60 mΩ at 25 °C on
      sheets of this class, rises several-fold in the cold, and a 0,6 A burst can pull the rail
      toward the module's 3,0 V floor. **The order is what makes that harmless**: the data is on
      both cards before the modem sends, so a head that browns out at a burst has lost the report,
      never the data. The cell's −40 °C discharge curve at 0,5 C is read before a cell is chosen,
      and the bench measures it (*Bench criteria*).
    - **The modem must run from the cell's rail**: down to ~2,9 V, where the eFuse cuts. LTE-M
      modules of this class take 2,2–2,7 V up (the sheets' `VBAT`), and LoRa 1,8 V; **no 2G
      fallback** — a GSM burst is 2 A. A module that wants another voltage, a satellite module at
      5 V, takes it through its own converter from this rail, its burst inside the same 0,85 A.
  - **The path out: cell → `TPS259474LRPW` → the 3,3 V rail — one eFuse is the hold, the
    takeover and the breaker.** `IN` on the cell, `OUT` on the rail. Its two FETs stand back to
    back, so no current flows from the rail into the cell in any state, on or off.
  - **The hold, and the undervoltage cut — `EN/UVLO` from `GPIO50` through 90,9 kΩ, 56,2 kΩ to
    ground.** The head raises `GPIO50` after boot; while the head is in reset the 56,2 kΩ holds
    `EN` low and the part is off, drawing **4,4 µA** from the cell (28,7 max). The divider is also
    the cut that needs no processor: `GPIO50` sits at the rail, so `EN` follows the rail at 0,382
    of it, and the part turns off when the rail falls to **2,82–2,92 V** (its 1,076–1,116 V
    falling threshold) and on from **3,10–3,20 V** (1,183–1,223 V rising) — below the 3,43 V the
    rail stands at when the head raises the hold. A LiFePO4 cell is nearly empty at the cut, and
    every chemistry here is far above its damage floor. A head that browns out first lets `GPIO50` go high-impedance and the
    56,2 kΩ turns the part off just the same, so a hung or dying head cannot drain the cell.
  - **The takeover — the part's own ideal diode.** While the buck runs the rail stands above the
    cell and the part blocks: it enters reverse blocking when the rail rises **22–36 mV** above
    the cell, within 1 µs, and leaves it only when the cell stands **83–125 mV** above the rail,
    reaching full conduction in **50 µs**. **While the buck runs the cell carries nothing**; a
    board whose layout or schematic lets the rail sag under the cell on a load step lets the cell
    share that step through the part's linear regulation — the bulk at the module and the buck
    beside it are what prevent it. When the buck loses its input the rail sags, and 83–125 mV
    under the cell the part takes it; the takeover follows the rail itself, not the 12 V wire,
    so how slowly the wire decays does not matter.
  - **The breaker — `ILM` 3,32 kΩ: 1,0 A, 0,85–1,15 A.** Above it the part passes the current for
    the `ITIMER` blanking — **100 nF, 51–145 ms, 84 typical** — and above twice it trips in
    500 ns; either way it turns off and, being the `L` variant, **stays off** until `GPIO50` takes
    `EN` below 0,45 V. A short on the rail while it runs on the cell costs the part its conduction,
    not the board the cell's current. The head's load on the cell — Wi-Fi off, both cards writing,
    Hermes, then the modem sending — stays under the 0,85 A floor, because the two never overlap
    (above).
  - **The other pins:** `OVLO` to ground, no overvoltage lockout on a 3,4 V cell and the pin must
    not float · `dVdt` open, the rail being up before the part turns on, so there is no inrush to
    shape · `PGTH` to `OUT`, so `PG` reads the path alone.
  - **`PG` on `GPIO52`, 100 kΩ up to the rail — high when the cell carries the rail.** The part
    holds `PG` low while it blocks and while it is off. An interrupt: the head knows it runs on
    the cell.
  - **`ILM` on `GPIO53`, the ADC — the cell's current, 0,604 V/A** across the 3,32 kΩ
    (182 µA/A); ~0 while the buck runs. The head integrates it through an event and reports the
    charge the cell gave: the one measure of the cell's capacity in the cold. **No capacitor on
    the pin** — its load stays under 50 pF.
  - **The part's own draw from the cell:** 0,43 mA carrying the rail (0,61 max) · 0,19 mA while
    it blocks, which the charge path covers · 4,4 µA off.
  - **The rail stays inside the module's window.** The `ESP32-S31` asks **3,0–3,6 V**, and 3,6 V
    is also its absolute maximum; everything else on the rail goes lower — the SD cards to 2,7 V,
    the `SX1262` to 1,8 V, an LTE-M modem to its sheet's 2,2–2,7 V, the H523 to 1,71 V, the `SHT45` to 1,08 V. The rail is set to
    **3,43 V** — 3,50 V at the top of its tolerance — so the cell floats at ~3,4 V. **The worst
    takeover dip:** 125 mV to the threshold, then 50 µs at 0,6 A out of **3× 47 µF hybrid
    polymer + the buck's 3× 10 µF**, ~170 µF, is 0,18 V more — the rail stays above **3,1 V**.
  - **The path in: the rail → `TPS22917` → 10 Ω → the cell**, `EN` on `GPIO51`. The cell floats
    at the rail's 3,43 V — the LiFePO4 float, and under a sodium-ion cell's ceiling; the resistor
    limits a recharge to ≤ 0,1 A (~0,03 C on 3 Ah) and an event's ~20 mAh comes back in well under an
    hour. The eFuse blocks every current from the
    rail into the cell, so a recharge goes through the resistor and nowhere else; when the 12 V
    returns before the hold is released, the buck's soft start ramps the rail past the cell and
    the part is blocking within 1 µs of the rail passing it by 36 mV. **Charging stops below
    −20 °C**, the low-temperature LiFePO4's charge limit, or at the chosen cell's own: the `SHT45` on the board is the thermometer and
    has no output of its own, so the head drops `GPIO51` on its reading. It drops it too while
    it runs on the cell. The charge current the cell accepts below 0 °C is not on the maker's
    page and is asked of the maker with the full sheet; the float asks ≤ 0,03 C. Off, the
    `TPS22917` blocks the reverse current, so the cell never leaks into a dead rail.
  - **The off is real.** When the head releases `GPIO50`, `EN` falls, the part turns off, the rail
    goes, and the 56,2 kΩ keeps it off; the board stays dark until the 12 V returns and boots it.
    A discharged buck input takes nothing from the cell — the `PMEG10020ELR` stops the 12 V wire
    behind it.
- **Service supply off USB:** `VBUS` from the USB header through a **`PMEG10020ELR`** Schottky
  into the `LMR43610`'s input, beside the 12 V. The diode stands 100 V, so the 12 V node — up to
  20 V on the pin — never reaches the USB host, and the ~0,4 V it drops leaves ≥ 4,5 V against
  the buck's 3,6 V start; the current limit is the host port's. A phone's OTG (~0,5 A) or a
  laptop then runs the head with the station dead: **service mode, console over USB only, Wi-Fi
  and BLE off**, Hermes and the `SHT45` alive on the head's 3,3 V, Kronos and the cards not —
  they have their own bucks off a 12 V that is not there. The firmware knows it is on USB from
  the `VBUS` sense.

### The supply parts

| part | value | position |
|---|---|---|
| `DGPS2.5R-5.0` | 2 | the 12 V input and tap |
| `5.0SMDJ14A` | 1 | across the 12 V input, ahead of the diode |
| `PMEG10020ELR` | 2 | the 12 V input · `VBUS` into the buck input |
| `LMR43610R3RPER` | 1 | the 3,3 V rail, 3,43 V — `LMR43620` on the same footprint where the modem is LTE-M; the house cell (`../galvani/HARDWARE.md`, *The buck cell*) |
| 29,4 kΩ 1 % · 12,1 kΩ 1 % · 22 pF C0G | 1 each | `R_FBT` · `R_FBB` · `C_FF` — 3,43 V |
| 7,50 kΩ | 1 | `RT`, 2,08 MHz |
| 4,7 µH shielded, `I_SAT` ≥ 3,5 A | 1 | the buck's inductor |
| 4,7 µF 50 V X7R 1206 + 100 nF 50 V | 1 + 1 | `C_IN` |
| 3× 10 µF 50 V X5R 1206 + 100 nF 50 V | 3 + 1 | `C_OUT` |
| 1 µF 50 V 0805 · 100 nF 50 V | 1 · 1 | `VCC` · `BOOT` |
| **47 µF 50 V hybrid polymer** | **3** | the rail's bulk at the module |
| `TPS259474LRPW` | 1 | the backup cell's path out — hold, takeover, breaker, cut |
| 3,32 kΩ 1 % | 1 | its `ILM`: 1,0 A, and the current read on `GPIO53` |
| 90,9 kΩ 1 % · 56,2 kΩ 1 % | 1 · 1 | its `EN/UVLO` divider from `GPIO50` |
| 100 nF 50 V | 1 | its `ITIMER`, 84 ms |
| 100 kΩ | 1 | its `PG` up to the rail, into `GPIO52` |
| `TPS22917` | 1 | the backup cell's path in, `EN` on `GPIO51` |
| 10 Ω | 1 | the charge limit, ≤ 0,1 A |
| the backup cell, ~3 Ah, wide-temperature — LiFePO4 or sodium-ion | 1 | 18650, 21700 or a thin prismatic cell; at −40 °C and 0,6 A above 3,0 V on its sheet |
| polyfuse, 0,15 A hold | 1 | Hermes's 3,3 V |

## Bench criteria

- The two 12 V terminals and the `PMEG10020ELR` front: a reversed pack clears the fuse and nothing
  on the board sees it.
- The 3,3 V holds the module's 0,6 A with Wi-Fi transmitting at its 295 mA peak, the bulk at the
  module.
- The backup cell: the takeover with the wire cut at full load, the rail above 3,1 V through it;
  the hold released to a dark board; the cut at 2,82–2,92 V on a bench supply in the cell's place;
  a short on the rail tripping the breaker and the part staying off; the charge path off below
  −20 °C. **At −40 °C, on the cell, the modem sending after the flush**: the rail stays above the
  module's 3,0 V through its bursts, and the cell's current on `ILM` stays under 0,85 A.
- The USB header: the head boots and serves the console from a phone's OTG with the 12 V absent,
  and 20 V on the 12 V node reaches nothing on `VBUS`.

## Ethernet — on the boards that bring the MAC out

**The Mayak board carries no Ethernet; Proteus and Mimir do, on the pins the table above
reserves, and this is the one description of it.** The S31's MAC runs RMII to a PHY:

| part | what |
|---|---|
| **`DP83826E`**, TI | 10/100 PHY, RMII, −40…+105 °C, 25 MHz crystal; it is the RMII clock master and drives the 50 MHz reference into `GPIO13`, so `GPIO35` stays the second SD slot's |
| **`HR911105A`** | RJ45 with integrated 10/100 magnetics, 1,5 kV, two LEDs; Bob Smith termination on the unused pairs' centre taps, 4× 75 Ω into 1 nF 2 kV to the enclosure's ground point |
| **`TPD4E02B04`** | ESD on the four PHY-side line pins |

| RMII signal | GPIO |
|---|---|
| `TXD0` · `TXD1` · `TXEN` | 8 · 9 · 12 |
| `REF_CLK` in, 50 MHz from the PHY | 13 |
| `CRS_DV` · `RXD1` · `RXD0` | 15 · 18 · 19 |
| `MDC` · `MDIO` | 60 · 61 — the boot straps; `MDIO` pulled up 1,5 kΩ, the strap's own direction |
| PHY `RESET_N` | 37 |

The firmware serves the port as it serves Wi-Fi: the archive session, NTP, the command channel,
the live board (`FIRMWARE.md` §9).

## USB-C — on the boards that carry a receptacle

Where a board puts a receptacle on its edge instead of this board's 1×4 header: a 16-pin USB-C
receptacle, **5,1 kΩ to ground on `CC1` and `CC2`** (a device), **`TPD4E05U06`** on `D+` · `D−` ·
`CC1` · `CC2`, and the same `PMEG10020ELR` from `VBUS` into the 3,3 V path as here.

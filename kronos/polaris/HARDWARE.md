★ N.I.C. ★

# Polaris — the hardware

> **Design-stage concept.** The figures are verified against the parts' sheets and the parts
> before a carrier is made.

**Polaris is a bought GNSS module and a bought active antenna on a carrier of ours the size of the
module.** The carrier holds one connector, one pin row and two resistors. Everything it is handed
comes over the connector; everything it hands back goes over the same one.

## The module — bought, one type: `M8N`

**u-blox NEO-M8N on a maker's carrier board with an SMA socket and a pin row.** One type, typed
into Kronos (`0x5C SRC_TYPE` = 1), never detected; Kronos's boot dialogue is written for it. A build
that fits another receiver types it and brings its dialogue; the station carries one.

| | NEO-M8N (data sheet UBX-15031086, FW SPG 3.01) |
|---|---|
| constellations | **GPS + GLONASS concurrent** as shipped; BeiDou in place of GLONASS or Galileo beside them by configuration |
| time pulse | **30 ns RMS, 60 ns 99 %**; as shipped **1 Hz, rising edge, 100 ms**, on pin 3 `TIMEPULSE` — inside ±1 µs by a factor of thirty before Kronos averages it |
| supply | **2,7–3,6 V**, 3,0 V typical |
| draw, tracking at 3 V | **30 mA** GPS + GLONASS, 23 mA GPS alone; **67 mA** peak |
| antenna supply | `VCC_RF` = `VCC` − 0,1 V, **50 mA** at most |
| interface as shipped | UART **9600 8N1**, NMEA 4.0 — `GGA` · `GLL` · `GSA` · `GSV` · `RMC` · `VTG` · `TXT` — and UBX both ways |
| memory | its own flash; Kronos writes nothing to it |
| leap second | `UBX-NAV-TIMELS` from protocol version **18.00**, firmware SPG 3.01 and later; SPG 2.01 is protocol 15.00 and has none (u-blox 8 / M8 Receiver Description UBX-13003221) |

**What the bought board must have — five conditions, read off the listing before buying:**

1. **The time pulse on a pin.** `TIMEPULSE` must come out on the row (`PPS`), not only on a LED.
   A board with a LED alone is not usable and nothing is soldered to its LED resistor.
2. **3,3 V straight in.** The carrier is fed 3,3 V from Kronos and nothing else — there is no 5 V
   on the connector. A board that puts an LDO in front of the module runs it at 3,1–3,2 V on a
   3,3 V feed, which is inside the module's 2,7–3,6 V but wastes the headroom the antenna current
   needs; a board without the LDO is preferred, and one with it has the LDO bridged.
3. **3,3 V logic on `TX`/`RX`**, the module's own level; no level shifter on the board.
4. **An SMA socket with the bias tee on the board** — the module's `VCC_RF` through the board's
   choke onto the centre conductor, so the active antenna is fed from the module's own rail. A
   board with a soldered ceramic patch on IPEX is not it.
5. **A genuine chip on firmware SPG 3.01 or later.** Cheap listings carry clones. Kronos's boot dialogue is the one test the
   station has: a part that does not answer `MON-VER` with the u-blox M8 hardware version
   `00080000` is reported, not configured — a wrong part fails loud and not as a silent time error.
   An M8N still on SPG 2.01 (protocol 15.00) keeps time and cannot announce a leap second; the
   `MON-VER` answer names it and Kronos reports it. A station build buys from a
   distributor; a development bench may buy the listing.

The board's own backup cell and EEPROM, and the module's flash, are not used: Kronos configures the
receiver at every boot and the station keeps no time across a power cut. Leave them fitted; they do nothing we rely on.
A 10 Hz fix rate advertised on such a board is not used either; the dialogue sets Kronos's `NAV_RATE`,
1 Hz as shipped.

**Draw:** the module 30 mA tracking and 67 mA at its peak, the antenna through the bias tee 5–20 mA
(the module's integration manual) — under 100 mA in all, on a connector pin rated 1 A and sized for an optical board's 175 mA.

## The antenna — bought, on an insulating bracket

**MikroTik `ACGPSA`** — an active GPS L1 antenna: SMA male on a fixed ~3 m cable, magnetic base,
IP67, −40…+85 °C, fed 3–5 V over the coax — `VCC_RF`'s 3,2 V is inside it — ≥ 26 dB of LNA. L1 is
all the `M8N` receives, so a single-band antenna is the right one; a multi-band module gets a multi-band
antenna, and the mounting below is the same.

**Mounting.** The antenna stands at the mast top on an **insulating bracket** — the coax shield is
the one conductor on the station that cannot be isolated, so the antenna body must not bond it to
the mast (`../../core/blocks/gps-pps.md`, *The coax, the arresters and the bracket*). The magnetic base needs
steel: a steel disc, ~100 mm, screwed to the plastic bracket, is both what the magnet holds and
the antenna's ground plane.

**The run.** The antenna's own 3 m cable, an in-line DC-pass arrester at its end, the enclosure's
coax down to the entry, the second DC-pass arrester there bonded to the entry earth, and a short
coax to the carrier's SMA — **≤ 4 m in all**, ~20 ns, typed as `CAB`. Both arresters are the
station's, not this carrier's, and both pass DC because the bias-tee current has to reach the LNA
through them.

## The carrier — one connector, one row, two resistors

A board the size of the module. It carries the 12-pin box header `BX2.54-2xNA` for the ribbon from
Kronos, a 5-pin 2,54 mm row the bought module plugs into or solders onto, and two resistors — the
`ID` and the PPS's series resistor.
Nothing is decoupled here — the module board carries its own capacitors — and no other part is
fitted.

| carrier | part |
|---|---|
| connector | `BX2.54-2xNA`, 12-pin 2×6 box header; `FC-12P` on the ribbon |
| module row | 5-pin 2,54 mm socket or pads: `VCC · GND · TX · RX · PPS`, in the order of the bought board |
| `R_ID` | **5,36 kΩ**, 1 %, 0603, pin 5 to ground |
| `R_PPS` | **33 Ω**, 1 %, 0603, the module's `PPS` pin to pin 1 — the channel's series resistor at its source |

**The ribbon** is a 12-way with plugs at both ends, conductors 1–5 and 12 wired and the rest cut
back; Kronos's end sits in its `TIME IN 1` socket.

## The connector — the Galvani data connector, and it is the whole interface

**Connectors: `TIME OUT` · `ANT`** (`../../galvani/README.md`, *Connector names*).

**12 pins, 2×6, `BX2.54-2xNA` on the board, `FC-12P` on the cable** — the family's data connector
(`../../galvani/README.md`), pin for pin. Six pins are wired on this carrier; six are not.

| pin | signal | on this carrier |
|---|---|---|
| **1** | `CLK/PPS` | **the PPS, outward from here and inward at Kronos** — the module's `PPS` pin drives it through `R_PPS`, 33 Ω at the source as on every channel B, 3,3 V logic. Kronos's socket ties `B_DIR` to ground, inward, and this carrier drives the pin unconditionally, so it carries none of a communication board's channel B parts |
| **2** | `GND` | the return; the module's `GND` |
| **3** | `TXD` | host → module: onto the module's `RX` — Kronos's configuration dialogue, 3,3 V logic |
| **4** | `RXD` | module → host: from the module's `TX` — the NMEA stream, 3,3 V logic |
| **5** | `ID` | **`GNSS`, `R_ID` 5,36 kΩ to ground — 0,35**, the same code Kronos presents on `ID_RET` at the other end. It names the **interface**, not this carrier, so Sputnik's clock run and a Pip's `TIME OUT` carry it too. Presence is still proven by the NMEA arriving; the resistor only says what kind of port this is |
| 6 | `ID_RET` | unpopulated — a host's pin, and this carrier is not a host |
| 7 | `RXD_ECHO` | unpopulated — no barrier, no echo receiver |
| 8 | `DE` | unpopulated — no driver to gate |
| 9 | `B_DIR` | unpopulated — the carrier drives channel B whatever the socket says, so it reads no strap |
| 10 | `LINE_EN` | unpopulated — no line side to switch |
| 11 | `SD` | unpopulated — no light to lose |
| **12** | `3,3 V` | **the whole supply, off Kronos over the connector**, onto the module's `VCC` — the module, and the antenna through the module's bias tee. No buck, no terminal, no 12 V and **no power connector** |

**Why no code of its own.** Sputnik, Pip and this carrier all hang on the same two Kronos sockets,
and the code names the **interface**, not the unit. A code apiece would repeat what `GNSS` already
says, and the one thing Kronos still needs — which receiver is out there — is the typed
`0x5C SRC_TYPE` and the NMEA that arrives. **A unit somebody invents later needs no new code**: it
presents `GNSS`, it is served, and it says what it is in the dialogue.

**Why the data connector and not something of its own.** Kronos already carries that socket for a
remoted receiver, the pins it needs are on it, and a second footprint on the clock board would buy
nothing. The carrier is in the same box, centimetres away.

## The delay — two typed terms

| term | value | how |
|---|---|---|
| `CAB` | metres of coax × ~5 ns/m — ≤ 4 m, ≤ 20 ns | typed on Kronos from the run as built |
| `INT` | the antenna's group delay and LNA, the module from antenna port to `PPS` pin, the ribbon | a constant of the type: the module's sheet, or once on a bench against a second receiver on a common antenna; tens of ns |

Nothing here is ranged: the carrier has no processor to turn Kronos's ranging edge round, and at
centimetres of ribbon nothing needs to. Kronos keeps the ranging on both its sockets; Pip's run is
where it is exercised.

## What is NOT here

**No processor.** A carrier that had one would be a Sputnik.

**No feed and no barrier.** The 3,3 V arrives over the connector; there is no 12 V, no isolator,
no transformer and no gas tube. The lightning ladder on this run is the coax's two arresters, and
they are the station's.

**No NodBus, no address, no type code.** Polaris is not a unit. It reports nothing; what Kronos
makes of its stream is Kronos's.

**No bias tee and no RF layout of ours.** Both are the bought board's (`WHY.md`).

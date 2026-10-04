★ N.I.C. ★

# NIC Core — the communication map

**Every exchange in the station, in one table, with the document that owns it.** This file
settles *what talks to what, over what, carrying what*. It settles nothing else: frames,
opcodes, clock rates and register maps belong to the documents named in the last column, and
those win on any question of content.

---

## The map

```
   GNSS ─NMEA─┐                        ┌─ I²C time bus, one, multi-master ─┐
   Pip  ─NMEA─┤                        │  (Kronos labels, the Mayak seeds) │
              ▼                        ▼                                   │
           KRONOS ──PPS-K (wire)────▶ MAYAK ──4× MasterNOD trunk──▶ BIFROST ──4 spurs──▶ units
              │  ──2²² clock (wire)──────────────────────────────────▶ │        (NodBus)     │
              │  ──I²C time bus, the label to all──────────────────────▶ │                     │
                                       │                                                     │
                                       │                                             ModBus arm
                                       ├── LP I²C ─▶ Hermes ─▶ BMS / MPPT                  │
                                       ├── Wi-Fi · the modem ──────────────────────▶ server         COTS · house MOD
                                       └── BLE ──▶ the handset                              · Babel
```

## The links

| link | between | medium | carries | owned by |
|---|---|---|---|---|
| GNSS in | receiver → Kronos | UART + PPS wire | lean NMEA, PPS-GNSS. The station's one NMEA parser is here; the ESP carries no GPS code | `blocks/gps-pps.md` |
| Pip in | Pip → Kronos | UART + PPS wire on Kronos's second RX/TX + PPS socket — a crossed in-box cable, or a communication board on a remote Pip | second time source where fitted: NMEA and `$PNIC` in, PPS on the wire; Kronos's `$PNIC,HELLO` every second is what keeps the socket served. **Pip is also a NodBus unit, type 12, on its other body** — the SID channel goes up the spur | `../tesla/pip/FIRMWARE.md` §8 · `blocks/gps-pps.md` |
| PPS-K | Kronos → Mayak + every Bifrost | the PPS pair of the M-LVDS time bus (`DS91C176` drivers, `THVD1450` receivers), one ribbon | the precise second edge. Never software-forwarded | `blocks/gps-pps.md` |
| network clock | Kronos → every Bifrost and the Mayak | the CLK pair of the M-LVDS time bus (`DS91C176` driver, `THVD1450` receivers) | 2²², the spur's own rate — a Bifrost passes it to its spurs undivided, 2²³ on-board. Kronos is the only source; the Mayak receives it and its firmware does not use it | `blocks/nodbus.md` |
| time bus | Kronos ↔ Mayak ↔ every Bifrost | I²C, one bus in a star, multi-master | Kronos, once a minute and on a change of quality, to all: the label naming the second PPS-K just marked, the date in it; the receiver counts the seconds in between on PPS-K, and PPS-K is the liveness. The Mayak, when it has something: the coarse seed, the position, a change; quality is a register it reads. Cards only listen | `blocks/gps-pps.md` |
| ATTN | Kronos → every card | 3V3 on the time bus ribbon, a pull-down on the card | presence: high = a Bifrost, off the ribbon = an Argus. One firmware | `../bifrost/HARDWARE.md` |
| MasterNOD trunk | Mayak ↔ Bifrost, ×4 | point-to-point full UART, in-box, plain UART levels, over the crossed in-box cable on the data body | 48 B DATA (the card completes the Unix second and recomputes the CRC) and 12 B CONTROL. No multidrop, no arbitration in the normal build; **the trunk is the same bus as a spur, so more cards on one Mayak port is a possibility the hardware allows and the firmware does not implement** (`../bifrost/README.md`) | `../bifrost/README.md` |
| NodBus spur | Bifrost → unit, **4 spur ports per card, ≤ 8 units per card — 4 · 16 · 32 across the station** | 485 or glass behind a Galvani board | 40 B DATA in TDMA slots, 12 B CONTROL on demand, plus the per-spur clock. **Full duplex on every class** — the feed — 48 V, or 300 V — rides its own 2-core cable on land, so the four pairs are data TX · clock · ground · data RX. Glass is point-to-point; copper may chain, and full duplex does not cost that chain — the echo-check receiver carries the read-back, which is where the units past one-per-port come from | `blocks/nodbus.md` |
| ModBus arm | a NOD → its modules | 485 behind a Galvani board, or a crossed in-box cable | plain RTU 8N1, no clock, no termination. Only **Palatine** (its arms) carries one — Argus carries mini segments instead, and Tesla reads its thermometers as NTCs on its own ADC | `blocks/modbus.md` |
| NodBus mini | Argus → its units, **4 segments** (1 upstream + 4, six USARTs in all — the up port's echo receiver is the sixth) | 485 or glass behind a Galvani board | **8 B payload on NodBus framing** — same TDMA, same addressing. The slot is the time, so no marker and no age field. **The sync rung is the card's role, 2¹⁹ on all four segments** — Argus generates it on `MCO1` at prescaler 8 through one quad buffer — and the mini-NOD takes it on a timer capture rather than on HSE — 2¹⁹ on the 2 Mb/s module; the data rung is the same two as everywhere, 2²⁰ alone and 2²¹ chained (`blocks/nodbus.md`). **A segment costs a whole UART**: TDMA needs a continuous ear where a polled arm multiplexes several | `PROTOCOL.md` §2 · `blocks/nodbus.md` |
| the clock board's feed | a **Sputnik** → Kronos | a crossed in-box cable; copper or glass behind a Galvani communication board, **channel B reversed**, where a build stands it outside | the lean `RMC`/`GGA` on channel A, **PPS inward** on channel B. Exists only where the station's GPS is Sputnik's receiver; its run delay is ranged by Kronos on channel A and subtracted like any distance term | `blocks/gps-pps.md` |
| BMS / MPPT | Mayak → Hermes → the power boards | the LP I²C to Hermes, the converter, on a Galvani power body, one point-to-point link — a small H523 card as an I²C slave fed 3,3 V by the head; on its far side **485 Modbus RTU on `THVD1450`** — what an MPPT and a smart BMS speak — with a plain 3,3 V UART as the second face, I²C out and CAN (`TCAN334`) fitted and asleep until a part wants them. The LP UART is not used | Hermes polls both on its own cadence and holds the latest register set; the head reads it as one block. Internal, **never gated** | `../hermes/README.md` |
| uplink | Mayak ↔ server | Wi-Fi · the modem — LoRa, satellite or LTE-M, on the backup cell's rail; LTE-M on Proteus | transport: `UPLINK_TRANSPORT.md` · session HELLO / CURSOR / cumulative ACK: `PROTOCOL.md` §6 | both |
| handset | Mayak ↔ phone | BLE (the S31 is BLE-only) | bring-up and service. Not a served web page | `../mayak/handset/README.md` |

## The frames

| frame | length | direction | where |
|---|---|---|---|
| DATA | **40 B leaving a node (16 B on mini), 48 B reaching the head** — one frame, completed on the way: the card prepends the upper 3 Unix bytes, zeroes five of reserve over the spur's CRC and recomputes. Header `unix.0 · frame · TYPE\|NUM · slot · kind · status`, 32 B payload, CRC | unit → master, one per slot | `PROTOCOL.md` §1 |
| CONTROL | **12 B** — `unix.0 · frame · TYPE\|NUM · SUB · op · arg0 · arg1 · arg2 · rsvd · rsvd · CRC`; `SUB` names the sonde behind an Argus NUMBER, 0 otherwise | either direction, on demand | `PROTOCOL.md` §1 |
| the modem's telemetry | **16 B** values frame | Mayak → server, out only | `PROTOCOL.md` §6 |

Both bus frames are fixed-length, CRC-16-CCITT, delimited by the idle line and told apart by their length — no magic byte. **The address byte is
`TYPE«4 | NUMBER` on NodBus and mini, `TYPE«2 | NUMBER` on ModBus, and means the same thing always** — the path is never in a frame
(`PROTOCOL.md` §2).

## Who says what

`M` = master · `N` = node · `C` = card · `bcast` = 0xFF. The full opcode index is
`PROTOCOL.md` §9; the card's three verbs and the recovery ladder are §10.

| direction | what lives here |
|---|---|
| M→N | DISCOVER (broadcast, RC only; the reply carries the unit's tag) · ASSIGN_ADDR (by tag) · TICK · RESEND · SET |
| N→M | ACK · ERROR, only as the answer to a command · `REPORT` once a minute; a fault is the header's `FAULT` flag, its detail `HEALTH` on `GET` |
| M↔N | GET — everything read or set is a register; the reply comes under the same op |
| M→C | **FLOOR · PORT_PWR · PORT_CLK** — the card holds no policy; it drives a pin and reports |
| bcast | SYNC · BUSCFG · END · HARD_RESET · EVENT |

## The tunnels

- **Modbus inside NodBus.** The Mayak composes the **complete RTU package** (address, function,
  data, its RTU CRC) and sends it down as a DATA frame, `kind` TUNNEL, in the gaps; the bridge
  node (Palatine) is a **byte pipe** — unwrap, stream onto the arm verbatim, wrap the whole
  reply as `kind` TUNNEL_REPLY. One question open per bridge, one answer. No length field, no fragmentation
  (32 B cap), no node retry — timeout is an `ERR`, end-to-end retry is the Mayak's. **The head
  never speaks Modbus onto a spur** and **the mini bus has no tunnel** (`PROTOCOL.md` §1 · §9).
- **Kronos emulates a GPS to the station.** PPS-K plus the label are exactly what a receiver
  gives, so the Mayak's GPS-handling path is unchanged (`blocks/gps-pps.md`).
- **A card is addressed like a unit.** `TYPE«4 | NUMBER` with TYPE 2, the same 12 B CONTROL frame, `SUB` 0 —
  the trunk carries card traffic and proxied unit traffic on one link (`PROTOCOL.md` §10).
- **A polled front's payload describes itself.** **Palatine** ships **self-delimiting blocks** — `[address][length][data]`, one per ModBus transaction —
  the module's address, the length, the registers as they came back — so the Mayak decodes one it has never seen,
  with no schema sent up first. **Argus tiles**: four 8 B mini payloads per 32 B payload at
  enrollment-fixed positions — the sweep's segment table decodes, an absent sonde's position
  rides zeros (`PROTOCOL.md` §5, `../bifrost/argus/README.md`).

## What is deliberately absent

Each of these is a settled decision, not a gap.

- **No multidrop at the head in the normal build.** The four trunks are point-to-point; the
  NodBus starts behind a Bifrost. More Bifrosts on one Mayak port is a possibility, not a build
  (`../bifrost/HARDWARE.md`, *The trunk*).
- **The Mayak carries no ModBus *arm*** — nothing is polled for science on the head. The BMS and
  the MPPT sit behind Hermes on its LP I²C; what the head does not have is a
  sensor bus. It receives the network clock and its firmware uses only PPS-K and the label.
- **No RTC anywhere.** Time comes from above and its absence is reported.
- **A ModBus arm carries no clock and no time.** The host stamps a leaf value when it polls it.
  **There is no exception** — a house unit that needs the clock goes on NodBus mini instead.
  What ModBus keeps is the fan-out: sixteen units on one arm, one USART, which only a polled bus can do.
- **No shared RX from the GNSS receiver** — each consumer has its own link.
- **No burst and no pipelining on any bus.** The house answer is a higher rate and more ports.
- **No tunnel on the mini bus** — a mini-NOD has no arm and no ModBus registers.
- **No subcommutation anywhere** — cancelled; Sputnik's epoch spreading is not it (`WHY.md`).
- **LoRa's and a satellite module's downlink cannot reconfigure anything** — a resend request is the
  whole of it; an LTE-M modem carries the authenticated session as Wi-Fi does (`PROTOCOL.md` §6).
- **An image or a table goes down as a stream** — `LOAD`, the frames on the master's own line, `APPLY` as a trial boot (`PROTOCOL.md` §7); `BOOT0` stays the factory path.
- **The path is never in a frame.** Port 2 is Bifrost 2 by construction and a slot is a time; the
  wiring lives in tables the commissioning sweep builds, and none of them renames anything.

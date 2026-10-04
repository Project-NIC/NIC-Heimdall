★ N.I.C. ★

# NIC Core — Platform Design Decisions (the index)

The decision log of the **universal, front-agnostic NIC platform** has been dissolved into
the documents that own the things — the reasoning moved to where a builder is already
reading, it was not dropped. The protocol and data architecture consolidated into
**[`PROTOCOL.md`](PROTOCOL.md)**; the hardware decisions live with their hardware. What
remains here is the index: the numbers stay valid as handles, and each row says where the
content now lives.

The seismograph front's own numbers — D2–D5, D8, D11, D14–D18, D21, D22 — share the one number
space and are indexed below with the rest; a number is never issued twice.

## Index — every decision, and where it went

| # | Decision | now lives in |
|---|---|---|
| D2 | Quake: one sensor per SPI | `../quake/HARDWARE.md` |
| D3 | Quake: the sensors free-run on their own clocks, configured once | `../quake/FIRMWARE.md` §5 · `../quake/WHY.md` |
| D4 | Quake: 128 Hz by tuning the sensor clocks, not the nominal grid | `../quake/FIRMWARE.md` §5 |
| D5 | Quake: the phase from a common start, the time from the sample index | `../quake/FIRMWARE.md` §4–5 |
| D8 | Quake: the levelling is computed and applied on the node | `../quake/FIRMWARE.md` §7 |
| D11 | Quake: every sensor in one orientation, no per-unit alignment | `../quake/HARDWARE.md` |
| D14 | Quake: 16 bits an axis on the wire | `../quake/BUS.md` |
| D15 | Quake: the SCL3300 as the tilt and drift reference | `../quake/HARDWARE.md` |
| D16 | Quake: the ranges, and the ICM as the seismic fallback | `../quake/HARDWARE.md` |
| D17 | Quake: the SCL3300 read slowly; ranges are deployment settings | `../quake/FIRMWARE.md` §6 |
| D18 | Quake: the tilt angle on the head; compression at storage | `../quake/FIRMWARE.md` §7 · `../quake/BUS.md` |
| D21 | Quake: only the ranges are tunable at run time | `../quake/FIRMWARE.md` §8 |
| D22 | Quake: the supply and the temperatures off the payload | `../quake/BUS.md` · `../quake/WHY.md` |

*(The station-wide numbers follow.)*

| # | Decision | now lives in |
|---|---|---|
| D1 | Modular hardware-agnostic libraries + thin glue | `PROTOCOL.md` §8 |
| D6 | Clock architecture: one master crystal, integer division, MCU-generated sensor clocks | `blocks/nodbus.md` · `../kronos/HARDWARE.md` |
| D7 | Two protocols: a data plane and a control plane | `PROTOCOL.md` §1 |
| D9 | Control protocol: minimal, symmetric, opcode-driven, silence-framed | `PROTOCOL.md` §1 |
| D10 | CRC16 on both protocols | `PROTOCOL.md` §1 |
| D12 | A unit's image is loaded over its own link as a stream and booted as a trial; `BOOT0` the factory path | `PROTOCOL.md` §7 |
| D13 | The comm library is a universal frame bridge; semantics live in the application | `PROTOCOL.md` §1 |
| D19 | Final framing: two lean frames | `PROTOCOL.md` §1 |
| D20 | Runtime setting changes roll over to a new storage table | `PROTOCOL.md` §5 |
| D23 | Node identity is the board's TYPE plus the NUMBER the sweep assigned — not a label on a box and not the MCU UID | `PROTOCOL.md` §2 |
| D24 | Self-running TDMA slots + per-sample status byte | `PROTOCOL.md` §3 |
| D25 | Master uplink: the modem by what it can — LoRa and satellite out-only (+ resend), LTE-M the authenticated session; Wi-Fi optional & rich | `PROTOCOL.md` §6 |
| D26 | Node TYPE: the high nibble of the one-byte `TYPE«4 \| NUMBER` address, learned at discovery | `PROTOCOL.md` §2 |
| D27 | Absolute time: a GPS-disciplined anchor + the sample index | `PROTOCOL.md` §4 |
| D28 | Generic config slots (front-agnostic protocol, front-defined meaning) | `PROTOCOL.md` §5 |
| D29 | Fixed 8-byte frame overhead + Modbus tunnelled in the payload | `PROTOCOL.md` §1 |
| D30 | The archive and the uplink = HMC files per unit, HCC segments; archive and live packets | `archive/HMC.md` · `archive/HCC.md` |
| D31 | Uplink session protocol: HELLO / server CURSOR / cumulative ACK | `PROTOCOL.md` §6 |
| D32 | DATA payload widened 28 → 32 B (power-of-two); frame still fixed | `PROTOCOL.md` §1 |
| D33 | Station position: auto-acquired from the on-board GPS | `PROTOCOL.md` §7 |
| D34 | Node intelligence: a constrained-but-capable worker (the power budget) | `PROTOCOL.md` §7 |
| D35 | Head-end uplink batching: spool the record stream, store-and-forward | `PROTOCOL.md` §6 |
| D36 | Data-priority tiers: what streams, what is queried, what never leaves the node | `PROTOCOL.md` §5 |
| D37 | Subcommutation: mux slow channels on the wire, demux to per-channel storage | `PROTOCOL.md` §5 |
| D38 | Universal per-NODE_TYPE wire schema | `PROTOCOL.md` §5 |
| D39 | Master-side storage: a recording rule per unit selects by bytes; a send rule per unit and link | `archive/HMC.md` · `PROTOCOL.md` §5 |
| D40 | Master split: ESP32 data head + a separate STM32 clock board | `../kronos/README.md` · `../mayak/README.md` |
| D41 | One universal MCU across the whole project: STM32H523 | `../NAMING.md` |
| D42 | A station is a modular master + node-subset bus; freed slots are open extension points | `PROTOCOL.md` §8 |
| D43 | One bus; the sample rate follows the fastest node present | `PROTOCOL.md` §3 |
| D44 | Naming: "Quake" is the seismograph node; the product is the Station | `../NAMING.md` |
| D45 | The master is the universal Station head-end; top-level `mayak/`, not under `quake/` | `../NAMING.md` |
| D46 | Master Modbus cadence on free-running HW timers, never PPS | `PROTOCOL.md` §3 |
| D47 | Leap seconds: acquire on a continuous scale, derive UTC only at the edge | `PROTOCOL.md` §4 |
| D48 | microSD power-loss integrity: FAT kept, self-healing archive, flush on brownout | `PROTOCOL.md` §6 |
| D49 | Clock state-of-health: a low-rate channel, not a bit in every record | `PROTOCOL.md` §4 |
| D50 | Instrument-response gain is a USER setting (mantissa × 10^exp10) — done | `PROTOCOL.md` §5 |
| D51 | Settling / cold-start flag: mark warm-up data, don't drop it | `PROTOCOL.md` §4 |
| D52 | Sensor self-test: the node checks its own sensors in quiet periods | `PROTOCOL.md` §7 |
| D53 | Universal per-sensor diagnostics → master: report WHICH sensor is failing | `PROTOCOL.md` §7 |
| D54 | Mux is an available OPTION per front, not a mandate; seismo runs FULL-RATE | `PROTOCOL.md` §5 |
| D55 | Value-plausibility QC + a per-frame errorflag byte | `PROTOCOL.md` §7 |
| D56 | Link-aware uplink: markers always, full windows on demand | `PROTOCOL.md` §6 |
| D57 | The format is the contract, not the component | `PROTOCOL.md` §6 |
| D58 | The remote architecture: the head detaches, Bifrosts everywhere, per-spur point-to-point NodBuses | `../bifrost/README.md` |
| D59 | The node identity: one byte, `TYPE«4 \| NUMBER`, FINAL | `PROTOCOL.md` §2 |
| D60 | The roster lives on the master, keyed by (bus, TYPE, NUMBER); **the three buses keep separate type spaces**, and the type is the board's name | `PROTOCOL.md` §2 — *Three buses, three type spaces* |
| D61 | the identity is the address and means the same thing always — **`TYPE«4 \| NUMBER` on NodBus and mini, `TYPE«2 \| NUMBER` on ModBus**, which buys types with the two bits it does not need for instances — a type is a quantity at a position; a NUMBER is unique across the station, never per ModBus arm; **the path is not in the frame** — port 2 is Bifrost 2, a slot is a time, the wiring lives in swept tables | `PROTOCOL.md` §2 |
| D62 | Air quality is portable, and has no board of its own | `../palatine/chinook/README.md` |
| D63 | NIC-Pip — NodBus type 12 on Tesla's board under its own image, the SID channel and a longwave time source on Kronos's second socket; the Bifrost formats rather than relays | `../tesla/pip/README.md` · `../bifrost/README.md` |
| D65 | Configuration lives in the MCU's own flash, in fixed cells | `PROTOCOL.md` §7 |
| D66 | One status LED a board, fitted, blinking — never a standing light; start-up time is not a budget | `HARDWARE.md` |
| D68 | Every ModBus address is one we put there; **ours are computed, bought ones are written**. **Babel has no type**: it answers as one slave per fitted sensor position, each on the type of its quantity, and IDENT says they are one board — our own units never multiplex several sensors into one block | `PROTOCOL.md` §2 · `../babel/MODBUS.md` |
| D69 | The feed — 48 V, or 300 V — goes in a cable of its own, so every NodBus class runs full duplex; the one-cable half-duplex build is deleted (`../galvani/WHY.md`) | `../galvani/README.md` — *The doctrine* |

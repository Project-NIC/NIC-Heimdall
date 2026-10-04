★ N.I.C. ★

# NIC — canonical board / front names

> **The rule: a board gets a name.** Anything with its own PCB is a named
> front/module. A chip that rides on another board's PCB is just a module, not a name.
>
> **The name IS the type.** A board is called what it is called, on the wire and in the
> documents alike — the old science labels (`seismo`, `basic`, `iono`, `mag`) are gone, because
> nobody should have to learn a second vocabulary to discover that "basic" meant Palatine.
>
> **The address IS the identity: `TYPE«4 | NUMBER`, one byte.** Quake #2 is `0x52`, Gauss #2 is
> `0x12`, Pluvius #2 on ModBus is `0x16` — three units numbered 2 collide with nothing and **nothing
> is ever translated** (`core/PROTOCOL.md` §2).
>
> **The three buses keep separate type spaces and never share one.** A module's code never appears
> on another bus's frame — a host converts its modules' values into its own payload as records, and
> a record is read in the context of the host that carried it.
>
> | **NodBus** — 40 B, `TYPE«4 \| NUMBER` | | **mini** — 16 B, `TYPE«4 \| NUMBER` | | **ModBus** — `TYPE«2 \| NUMBER` | |
> |---|---|---|---|---|---|
> | 1 | Mayak | **1** | **Gauss** | 4 | **Babel**, bare |
> | 2 | Bifrost | 2 | Quark-Tubes | 5 | Pluvius |
> | 3 | Argus | 3 | Pascal | 6 | Ceres |
> | 4 | **Marconi** | | | 7 | Sakura |
> | 5 | Quake | | | 8..61 | **free — one per quantity at a position** |
> | 6 | Palatine | | | | |
> | 7 | Tesla | | | | |
> | 8 | Sputnik | | | | |
> | 9 | **Photon** | | | | |
> | 10 | *reserved — `Neutron`* | | | | |
> | 11 | **Positron** | | | | |
> | 12 | **Pip** | | | | |
> | 13 | **Steinmetz** | | | | |
>
> **NodBus and mini stop at 14, ModBus at 61**, and for the same reason each: the packed address
> must land above the hand-written `0x01..0x0F` and inside Modbus's legal 1..247. **ModBus buys
> types with the two bits it does not need for instances** — a type is a quantity at a position,
> and a plot carries twenty kinds of sensor and four of any one, where a card's list is short and
> closed. **Gauss is mini type 1 and nothing else** — its ModBus form is gone (`gauss/WHY.md`). **Bought sensors
> get `0x01..0x0F`**, fifteen hand-written addresses against the eight a full weather station fits.
>
> **No house module is hand-written any more, and a NUMBER is unique across the STATION** — never
> per ModBus arm, because Palatine's arms are an electrical division and not an address space.
>
> The station's COMPOSITION lives in one small table on the MASTER, keyed by **(bus, TYPE,
> NUMBER)** — adding a unit type never means touching node firmware. Internal library/symbol
> names (`nq-*`, `nw-*`, `nr-*`) and the science terms (`ionosphere`, `radiation`, …) are
> separate and do **not** change with a board name.

## How a unit attaches — NOD / MOD

Two ways to hang a unit on the network:

| Class | spells | what it is | its bus | examples |
|---|---|---|---|---|
| **NOD** | **Nod**e | a unit that speaks NodBus itself | **NodBus** — RS-485 data + distributed clock (NIC proto, TDMA) | Quake · Palatine · Sputnik · Tesla · Pip · Steinmetz · Marconi · Photon · Positron |
| **MOD** | **Mod**bus | a Modbus slave module, slow — its read interval is a setting, not a fixed rate | **ModBus** — the RS-485 Modbus-RTU leaf bus off a NOD, polled, no clock. **Palatine** carries the arms | Pluvius · Sakura · Ceres · Babel |
| **mini-NOD** | | a unit whose measurement is 8 B and which needs the clock | **NodBus mini** — the same framing, TDMA and clock as NodBus, one payload wide, behind **Argus** | Gauss · Quark-Tubes · Pascal |

**The class is the bus, not the MCU — and the MCU is the H523 either way.** A NOD, a MOD and a
mini-NOD all carry the house H523; what a slow polled leaf does not use of it costs less than a
second firmware base (*MCU assignment*). **No unit appears in two rows** — Gauss is a mini-NOD and
nothing else (`gauss/WHY.md`).

So: a **NOD** sits on the **NodBus**, behind a Bifrost; a **MOD** hangs on a **ModBus arm** — and
**Palatine** carries the ModBus arms — bought sensors and the slow house modules — and **Argus**
carries the NodBus mini segments. The master (**Mayak**) is the head-end — and it drives the NodBus
**through Bifrosts**: each of its four trunk UARTs is a **MasterNOD link** — a point-to-point,
NodBus-framed, in-enclosure link head ↔ Bifrost; the NodBus proper starts behind the Bifrosts.
Everything else is just a board *inside* a unit's enclosure — the passive tube heads inside
**Quark-Tubes** (the GM and proportional tubes, Gadolin/Rhodion included, all counted on
Quark-Tubes' own H523 on the same assembly). It is not a separate bus class.

**Why the ModBus is Modbus.** RS-485 **Modbus RTU** — chosen over I²C / I³C / 1-Wire because a differential pair shrugs off the board-to-board noise that single-ended short-range buses don't, and Modbus is universal (the cost is worth the robustness).

| Board / module | Name | Directory | Senses | Reports as |
|---|---|---|---|---|
| **Master / head-end** | **Mayak** | `mayak/` | — front-agnostic: datalogger + uplink + the roster (the clock is Kronos's) | own board (ESP32-S31 head-end, **NodBus type 1 `Mayak`**) — every unit hangs off it through the Bifrosts |
| **Station timekeeper** | **Kronos** | `kronos/` | — the precision heart: GNSS-disciplined TCXO/GPSDO → the network clock (`core/blocks/nodbus.md`) + PPS + coarse UTC | dedicated **STM32H523** clock board beside the Mayak — pure clock, not a Bifrost |
| **Kronos's GNSS front** | **Polaris** | `kronos/polaris/` | — a time-only GNSS receiver: lean NMEA + PPS to Kronos | **a carrier, not a unit** — a bought u-blox NEO-M8N with its own bias tee, a bought active antenna, and one Galvani data connector into Kronos's RX/TX + PPS socket — it takes 3,3 V over that connector and needs no power connector. No processor, no bus, no address, and **no code of its own**: it presents `GNSS` like the socket facing it, because the code names the interface and not the unit. Fitted where the station has no Sputnik |
| **Longwave carriers — the SID channel, and time** | **Pip** | `tesla/pip/` — **Tesla's board under its own image; held back by coverage, not shelved** | the VLF/LF carrier levels (the D-region SID channel), and from the 34–120 kHz time-code stations and eLoran → PPS + date, for a site where GNSS is not available | **NOD, NodBus type 12 `Pip`** — Tesla's H7A3 board, both data bodies populated: `NB IN` the NodBus link, `TIME OUT` toward Kronos's second RX/TX + PPS socket, where it presents itself as a GNSS receiver (PPS + NMEA) and serves the socket only under Kronos's heartbeat. Tesla and Pip are one board as Bifrost and Argus are one card |
| **Line faults on power lines** | **Steinmetz** | `tesla/steinmetz/` — **Tesla's board under its own image** | the mains-locked impulsive sources of a line — arc · corona · partial discharge — from a vehicle at 80–100 km/h or a static site; Tesla's 4 B event with its own four classes, the floor and the source states | **NOD, NodBus type 13 `Steinmetz`** — Tesla's H7A3 board; in a vehicle behind a Proteus over one hybrid cable |
| **The head, the clock, a Bifrost and Hermes on one small PCB** | **Proteus** | `proteus/` | — four sections on one PCB, Polaris plugged in, an LTE-M modem, one copper NodBus run on the board, an isolated DC/DC brick at the input, Ethernet and USB-C | **one PCB, no type** — ESP32-S31 + 3× STM32H523, each section its own document's processor and firmware |
| **The head, the clock, two cards, Palatine and Hermes on one PCB** | **Mimir** (mini-Heimdall) | `mimir/` | — six sections on one PCB: two NodBus ports, four mini segments, four arms, a spare MasterNOD, Ethernet and USB | **one PCB, no type** — ESP32-S31 + 5× STM32H523 |
| **BMS/MPPT converter** | **Hermes** | `hermes/` | — the bought power boards' bus (485 Modbus RTU, UART, I²C, CAN) → one register block on the Mayak's LP I²C | **house card, no bus type** — **STM32H523** at 4 or 8 MHz on the Mayak's power body, fed 3,3 V by the head, `ID` 0,90; not a NodBus node, not a ModBus slave |
| **Optical bridge card** | **Bifrost** | `bifrost/` | — the light-bridge: station trunk ↔ up to 4 spurs to remote units, copper or fibre (per-spur master, ranging, spur→network time mapping) | **NodBus type 2 `Bifrost`** — house **STM32H523** bridge card — **six USARTs of seven**, one trunk with its echo receiver + four spur ports; every unit hangs behind one. **The same board is Argus** |
| Seismograph node | **Quake** | `quake/` | ground motion (MEMS + precision accel) | **NOD, NodBus type 5 `Quake`** |
| Base / meteo node | **Palatine** | `palatine/` *(was `weather/`)* | temp/RH, pressure, wind, solar, UV, soil | **NOD, NodBus type 6 `Palatine`** |
| Weighing rain gauge | **Pluvius** | `pluvius/` | precipitation by mass (load cell + ADS1235) | **MOD, ModBus type 5 `Pluvius`** — **STM32H523** |
| Leaf-wetness sensor | **Sakura** | `ceres/sakura/` | leaf surface wetness (capacitive) + the temperature at the plate | **MOD, ModBus type 7 `Sakura`** — **STM32H523** |
| Soil-moisture sensor (WMO depths) | **Ceres** | `ceres/` | soil volumetric water (capacitive) + the soil temperature at its depth | **MOD, ModBus type 6 `Ceres`** — **STM32H523** each; seated in the ground stacked in a **patrona** (`palatine/CONSTRUCTION.md`) |
| Magnetometer sonde | **Gauss** | `gauss/` — standalone remote unit (garden / well / borehole / sea) | geomagnetic field (RM3100) + ground temp (TMP117/STS35 on the wall, an NTC between the coils) — magnetometer-only, no tilt (SCL3300 dropped) | **mini-NOD, NodBus mini type 1 `Gauss`** behind an Argus — a sonde is 6 B of measurement, which is a mini payload, so it never needed a 32 B slot or a Bifrost port of its own; the segment carries the clock, which is why the sonde is on it and not on ModBus. **STM32H523** — one sensor board, the port board wired beside it is what changes |
| Universal Modbus bridge | **Babel** | `babel/` | — protocol converter: a sensor's native bus → Modbus RTU | **MOD, and it has NO type of its own** — **STM32H523**; each fitted position answers as its own slave on **the type of its quantity**, and `0xFF01 IDENT` says they are one board; a board with nothing fitted answers once on ModBus type 4, bare (`babel/MODBUS.md`) |
| Carrier node | **Argus** | `bifrost/argus/` | — no sensor of its own: **four NodBus mini segments**, tiled **four 8 B mini payloads to a payload** and announced as **⌈minis/4⌉ NODs** (`bifrost/argus/README.md`). A mini segment costs a whole UART — that is what TDMA charges, and it is why a polled bus keeps the fan-out advantage | **NOD, NodBus type 3 `Argus`** — **the Bifrost board**, **STM32H523**, six USARTs of seven: one NodBus upstream with its echo receiver, and four mini segments. **No LPUART on a bus.** **Four is where the timer captures, the USARTs and the frame index meet** |
| Pressure sonde | **Pascal** | `pascal/` | water column above it — MS5837-30BA + thermometer, in Gauss's oil-filled tube | **mini-NOD, NodBus mini type 3 `Pascal`** — **STM32H523**; the Tier 1 tsunami gauge where a site cannot keep a radar mast (`atlantis/README.md`) |
| HV-tube counting unit | **Quark-Tubes** | `quark/tubes/` | the tube assembly entire — three GM tubes of one type over the deposition plate — bare, β-stopped, behind Pb — and the neutron detector (Gadolin/Rhodion included), all counted on its own H523 | **mini-NOD, NodBus mini type 2 `Quark-Tubes`** — on a mini segment behind Argus. **Counts only, no energy** — a tube delivers none — so raw tubes are normalised and subtracted on the unit into **four published channels: beta · soft γ · hard γ · neutron, one `uint16` each = the whole 8 B payload** (`quark/tubes/BUS.md`). It needs the clock, and a house unit that needs the clock is not a MOD. It carries five NTCs, one on the board and one on each head, but never sends them with the counts: the correction is applied on the counts here, which makes it tier C. **Quark-Tubes is not the scintillation side** — that is **Quark-Scintillation**, Photon and Positron on the NodBus (`quark/scintillation/`); `quark/` keeps the shared radiation physics both build on |
| Universal output block | **Galvani** | `galvani/` — the doctrine home, and the only place that lists the boards | — front-end: the ONE place the network makes a voltage — puts a remote run on **48 V**, or **300 V** where 48 does not carry the load, carrying **the hard-wired unidirectional clock pair + the data pair(s) — duplex follows the feed** (NodBus or ModBus data at the insert point) so one universal module replaces per-case boards | not a sensor — the shared power/protection port board family. **Eleven boards, each named by what it does** — a power board makes the feed or takes it off the cable, a communication board carries the link; the class letters survive only inside a communication board's name (`G-I-N-025` · `G-I-M-005` · `G-O-10-10` · `G-O-2-100` · `G-12-S`; the 300 V unit boards carry their watts, `G-300-U-6` and `G-300-U-40`), and the `L`/`H` split and the ×1/2 series are gone |
| GNSS / ionosphere node | **Sputnik** | `sputnik/` *(was `iono/`)* | multi-GNSS → TEC | **NOD, NodBus type 8 `Sputnik`** |
| **Air-quality reference (Palatine's)** | **Chinook** | `palatine/chinook/` | the bought RS-485 air units — none in the base; particulates and gases fitted per site | **not a board** — the sensor prescription only; the units are bought Modbus slaves on **Palatine's** arms, each reply a block in its payload |
| **HF ionosphere monitor** | **Marconi** | `marconi/` | the F-region through fixed HF transmitters — MUF/foF2 and the passive ionogram, 0,5–16 MHz | **NOD, NodBus type 4 `Marconi`** — one board in one box: a 1 m air-core loop + `LMH5401` and `THS4541` + **`AD9265-80`** direct sampling over the **PSSI** into internal SRAM, and the **STM32H7A3IIT6** that runs the FFT; the Galvani bodies and the two 12 V terminals |
| Lightning node | **Tesla** | `tesla/` | sferics / lightning / TGF | **NOD, NodBus type 7 `Tesla`** — a single board in one box on the antenna frame: **3 ferrite rods** (the converter's fourth channel spare) + THS4551 + ADS127L14 + the **STM32H7A3IIT6** that runs its DSP; the two Galvani bodies and the two 12 V terminals. **One, two or three NUMBERs**: 1 the event records, 2 the source states and the floor, 8 the per-event supplement — **a supplement NOD is its base's NUMBER + 7** (`tesla/README.md`) |
| Gamma unit | **Photon** | `quark/scintillation/photon/` | photons — γ + X-ray, one quantity (they differ by origin, not by energy) | **two builds.** Scintillation: **NOD, NodBus type 9 `Photon`** — CsI(Tl) + SiPM + PIN, sampled simultaneously, on **`Quark-Photon`** (`quark/scintillation/photon/HARDWARE.md`), counts *and* energy. Tube: GM tubes behind graded Pb, feeding **Quark-Tubes'** banded channels |
| Neutron channel, photomultiplier | **Neutron** | `quark/scintillation/neutron/` | neutron | **a channel, type 10 reserved for it.** The two-screen `⁶LiF/ZnS(Ag)` stack read by a photomultiplier on Helion's kV source, on channel 1 of **`Quark-Neutron/Positron`**, published as **two counts, thermal and epithermal, in bytes 8–11 of `Positron`'s record, ahead of its bands** — a screen and a discriminator give a count and no energy, so four bytes carry it (`quark/scintillation/photon/BUS.md`); a build that wants it on a NUMBER of its own takes type 10 |
| He³ neutron head | **Helion** | `quark/tubes/helion/` | neutron | **no MCU** — the He³ / BF₃ proportional tube on **Quark-Tubes'** neutron channel `K4`, and the kV source it owns |
| Beta unit | **Positron** | `quark/scintillation/positron/` | beta, both signs — β⁻ and β⁺ ionise alike | **NOD, NodBus type 11 `Positron`** — a **25 × 25 × 25 mm plastic block** + SiPM on the face, no fibre, on **`Quark-Neutron/Positron`**, **the block only** — the beta the tubes see is `Quark-Tubes`'s derived K1 − K2 (`quark/scintillation/positron/HARDWARE.md`). One box carries the block with `Neutron`'s screens and answers as **ONE** NOD — the beta record with `Neutron`'s two counts in bytes 8–11. **The one eye on Sr-90/Y-90 and Kr-85** — pure beta emitters no gamma head sees |
| Gd/Rh neutron detector | **Gadolin** (Gd) / **Rhodion** (Rh) | `quark/tubes/gadolin/` | neutron — Gd capture → γ, or Rh activation → hard β | **no MCU** — the 13 tubes land on **Quark-Tubes' EXTI** on the same assembly and Quark-Tubes merges and counts them into its neutron channel **K4** |
| **Service companion (software)** | **Handset** | `mayak/handset/` | — the phone at the open enclosure: who enrolled, which sensor is failing by name, GO / NO-GO, and the provisioning commands | **not hardware** — a phone app (Android + iOS), described and not written, on the master's button-gated BLE service |
| **The radiation part** | **Quark** | `quark/` | what is measured and the two ways it is measured — **Quark-Tubes** (`quark/tubes/`) and **Quark-Scintillation** (`quark/scintillation/`) — and the neutron physics both share (`NEUTRONS.md`) | not a unit and not a board — the folder and the name of the whole radiation part |

## Why these names

- **Mayak** — Russian *Маяк*, the lighthouse: the master is the beacon the network steers by.
- **Kronos** — the Titan of time: the station's timekeeper takes time from the heavens (GNSS)
  and hands the whole station its heartbeat (`kronos/`).
- **Polaris** — the pole star, what you take your bearing and your hour from: the card that
  hands Kronos the sky (`kronos/polaris/`).
- **Pip** — *the pips*, the six beeps of the BBC Greenwich Time Signal since 1924, and what
  every transmitter in the band actually sounds like: a notch cut into the carrier once a
  second (`tesla/pip/`). *(Briefly "Marconi" — that name now belongs to the HF monitor, `marconi/`.)*
- **Hermes** — the messenger and interpreter of the gods: the card that carries what the pack
  and the panel say into the head's own registers (`hermes/`).
- **Bifrost** — the Norse bridge of light, guarded by Heimdall: the card that opens the
  light-bridges (optical spurs) from the station to the remote units (`bifrost/`).
- **Quake** — earthquakes.
- **Palatine** — for the *Societas Meteorologica Palatina* (1780), the first standardized international weather network (the front takes the English form of the name).
- **Pluvius** — Latin *rain* (Jupiter Pluvius).
- **Sakura** — the Kyoto cherry-blossom record, one of the longest climate proxies.
- **Ceres** — Roman goddess of the harvest; soil moisture ↔ yield.
- **Gauss** — the **Magnetischer Verein** (1836): the first worldwide magnetometer network with synchronised measurement times — this network's direct ancestor. *(Briefly "Magnes"; renamed.)*
- **Babel** — the Babel fish, the universal translator: it speaks every sensor's tongue and answers in Modbus.
- **Sputnik** — the first satellite.
- **Handset** — what a lineman clips onto a pair to talk to the line; this one clips onto the
  station. It reads *and* sets, which is why it is not called a viewer.
- **Chinook** — the North-American warm wind: the air the station breathes. Not a board — the shared reference for the bought air-quality units.
- **Tesla** — Nikola Tesla: lightning.
- **Marconi** — the 1901 transatlantic transmission worked because of the layer this board
  measures; Kennelly and Heaviside proposed that layer the year after, to explain it (`marconi/`).
- **Photon** — γ and X-ray are photons.
- **Helion** — the He³ nucleus is literally called a helion.
- **Positron** — the β⁺ particle; the unit reads beta of both signs, and the TGF photonuclear
  chain (`¹⁴N(γ,n)¹³N`, β⁺) makes real positrons over thunderstorms.
- **Gadolin** — Johan Gadolin, namesake of gadolinium.
- **Rhodion** — the rhodium-activation variant of the same board (`quark/tubes/gadolin/`).
- **Quark** — neutrons are made of quarks: the radiation part as a whole. **Quark-Tubes** and **Quark-Scintillation** are its two builds.
- **Galvani** — Luigi Galvani: *galvanic isolation* is named after him — the output-board kit (`galvani/`). **There are no class letters, no end digits and no numbers: a board is named by what it does, and the name IS its identity.** **Eleven boards:**

  | board | what it is |
  |---|---|
  | `G-48-S` | **power, source end, 48 V** — the 12 V off the wire on its own terminals |
  | `G-300-S` | **power, source end, 300 V** |
  | `G-48-U` | **power, unit, 48 V** — 12 V out and nothing lower |
  | `G-300-U-6` | **power, unit, 300 V, 6 W** — the pod's board: a 40 mm pipe is the volume |
  | `G-300-U-40` | **power, unit, 300 V, 40 W** — 12 V out; a remote Argus's bus, or any load of that size (a heated unit, a camera — examples) |
  | `G-I-N-025` | **communication, 485 on copper, NodBus** |
  | `G-I-M-005` | **communication, 485 on copper, ModBus** — one transceiver, one pair |
  | `G-O-10-10` | **communication, glass, 10 Mb/s** |
  | `G-O-2-100` | **communication, glass, 2 Mb/s** — 1 m to 100 km |
  | `G-12-S` | **isolated 12 V, source end** — a bought ModBus device a few metres out; no unit-end partner |
  | `G-24-S` | **switched 24 V, source end** — on Palatine's `PWR EXT`, for a load that is not a sensor |

  **The boards carried sheet numbers through two schemes and carry none now** — both schemes and their maps are in `galvani/WHY.md`, and why the station numbers nothing is in `daedalus/WHY.md`.

  **A power board makes the feed or takes it off the cable; a communication board carries the link.** A communication board serves **both ends** — the socket says which. Everything that leaves the enclosure is isolated (`galvani/README.md`).

*(Mechanical codenames, the „Barrel" family: one pipe profile, ~40 mm class, in steps of 25 cm, and the name is the multiple — **Barrel** 25 cm · **DoubleBarrel** 50 cm · **TripleBarrel** 75 cm · **QuadroBarrel** 100 cm. Quake takes a DoubleBarrel, or a QuadroBarrel with the magnetometer; what picks the length and how a body is seated are in `quake/CONSTRUCTION.md`.)*

## Structure of the radiation detectors

**Split by quantity — one quantity, one unit — and the technology is a build**, the
Bifrost/Argus move one level up:

- **`quark/`** — **Quark, the radiation part**: what is measured and by which build, the
  graveyard and `NEUTRONS.md`, the neutron physics the three neutron heads share.
  - **`quark/tubes/`** — **Quark-Tubes, high voltage**: the counting unit **Quark-Tubes** (mini-NOD type 2)
    on its one board, the GM head (`HEADS.md`), **`quark/tubes/helion/`** — **Helion**, the He³/BF₃ head and the kV source — and **`quark/tubes/gadolin/`**
    — **Gadolin** (Gd-capture → γ) and its **Rhodion** (Rh-activation → β) variant, one skeleton.
  - **`quark/scintillation/`** — **Quark-Scintillation, low voltage**: the group README over its three
    folders.
  - **`quark/scintillation/photon/`** — the gamma unit, scintillation NOD (type 9)
    on `Quark-Photon`; also the H7A3 layer both LV boards share, the scintillation physics, the
    record contract and the one firmware image.
  - **`quark/scintillation/positron/`** — the beta unit, scintillation NOD (type 11) on `Quark-Neutron/Positron`;
    **it carries `Neutron`'s two counts too**, and **`quark/scintillation/neutron/`** is that channel — the
    photomultiplier and the two screens, type 10 reserved for it.

## MCU assignment — right-sized

**Two MCU tiers below the head** (the head itself is the ESP32-S31): the **H523** — every node,
card and slave module — and the **H7A3** DSP tier — Tesla, Marconi, and the two
scintillation boards (Photon and Positron). **The H523 is the slave tier too**: the
capacitive sensors read on a synchronous-detector front that wants a timer at 2²⁷ Hz and two
differential ADCs (`ceres/HARDWARE.md`), the ranging return wants a fast timer on every unit
(`bifrost/HARDWARE.md`, *Ranging*), and one part number across the station is one firmware base
and one set of tooling (`core/WHY.md`, *A slave-class MCU*).

- **STM32H523 — the station nodes** (need RAM/buffers/compute; have **CORDIC + FMAC**):
  - **Palatine** — buffer memory for the meteo node and the ModBus master for the bought
    sensors. Radiation is not its job — **Quark-Tubes** counts the detector heads (its own row below).
  - **Quake** — seismic recomputations (filtering / STA-LTA / baseline).
  - **Sputnik** — memory to buffer the GNSS epoch across frames.
  - **Tesla — moved to the H7A3** (the DSP tier, below).
  - **Argus** — six serial ports in the job, which forces the bigger body whichever part is
    chosen; the H523 is that body and is the MCU every other node already runs (its row below).
- **STM32H523 — the sensor modules too, in LQFP100, the house footprint on every H523 (`core/HARDWARE.md`);
  two packages in the station and no more, LQFP100 for the H523 and LQFP176 for the H7A3.** One part covers the whole slave job, and the same one covers the nodes:
  - **Pluvius** — ADS1235 over **SPI** + Modbus over **UART** + drain logic.
  - **Sakura** — leaf wetness: the synchronous-detector capacitive front on the timer, the FDA and
    the two ADCs (`ceres/HARDWARE.md`, the same electronics) + Modbus.
  - **Ceres** — soil moisture, the same front, its own electrode + Modbus.
  - **Pascal** — MS5837-30BA + a thermometer and nothing else.
  - **Hermes** — two faces: I²C slave to the Mayak, Modbus master to the BMS and the MPPT.
  - **Babel** — the protocol converter, one slave per fitted position.
  - **Argus** — six serial ports in the job forced the bigger body before the tier merged;
    now it is simply the house part. Argus's work is unchanged: it relays values that arrive a
    sample a minute.
  - **Gauss** — RM3100 over **SPI**, the thermometers on I²C and the ADC, and one serial port, the
    mini link behind an Argus. The pod does no analysis — a temperature correction and a baseline
    subtraction — and the one non-trivial thing on it, the grid interpolation, costs ~1 % of the
    core. The H523 is there for the family, not for the arithmetic.
- **No MCU — passive tube heads:** the tube builds of **Photon / Helion** are tube + HV +
  pulse-shaping boards. Their pulse outputs go to **Quark-Tubes**, on the same assembly, and land on
  its **hardware timer inputs** — counted without the core noticing. They are **not** in the MCU
  question at all.
- **Own H523 — Quark-Tubes, the HV-tube counting unit (mini-NOD, NodBus mini type 2):** it counts every
  tube and answers on an Argus segment, where the clock arrives with the bus. **The two jobs
  split by hardware:** the
  **13 Gadolin/Rhodion tubes take EXTI**, because merging them means comparing arrival times so
  that one particle lit across several tubes counts once; **everything else takes a timer input**.
  **A slave-class part would be too slow** for the burst counting. The tubes ride Quark-Tubes' four published
  channels — **beta · soft γ · hard γ · neutron** (`quark/tubes/BUS.md` owns the layout).
- **Own H7A3 — TWO scintillation boards**, and the `AD9251-80` on the PSSI is what asks for the
  part on both: **`Quark-Photon`** (SiPM + PIN, sampled simultaneously, guard ring populated) and
  **`Quark-Neutron/Positron`** (one SiPM channel and the photomultiplier's, one front end copied, **no `LTC6268` and no
  guard ring**). `quark/scintillation/photon/HARDWARE.md` owns the board layer. **`Quark` alone is the radiation part and
  its folder — never a unit and never a board**: the tube counter is `Quark-Tubes`, NodBus mini
  type 2, and the scintillation side is `Quark-Scintillation`.
- **Tesla — one board, and its MCU is the STM32H7A3IIT6** (the DSP tier — Tesla, Marconi and
  the two scintillation boards): the sferic receiver (3 ferrite rods + THS4551 + **ADS127L14**) and
  the **H7A3** that runs its DSP are a **single PCB** in one box on the antenna frame. There is
  no inter-board link and no connector but the Galvani socket, so the fastest signals in the
  unit stay on inner layers instead of crossing a cable under the antenna. The NodBus (RS-485 + clock) runs up to the enclosure → the node is
  **bus-powered and network-synced like any other**, no separate bus or sync glue. (Separation is by layout — the digital
  corner behind a ground moat, the Galvani socket at the opposite edge from the front-ends.)

**The H523 covers the slave tier as it covers the nodes** — one part, one firmware base; a slave
uses a corner of it.

## Temperature-sensor policy (station-wide)

The station carries a **pool** of temperature sensors — above-ground and **buried** (e.g. the
**SHT45**, ±0,1 °C, picked as the cheapest *clean* temp source that isn't bundled with extras;
buried units sit at the soil depths). Internally the station holds **~3× more sensor values than it
reports** externally, so temperatures are **selected from the pool** as needed, not duplicated.

**Rule — add a local temperature sensor to a board only when the module needs it *locally* for its
own internal compensation / power-up calibration to output a clean value** (a proper self-calibrating
MOD; re-calibrating it externally would make it "botched", not clean). **Otherwise take temperature
from the pool** — don't add a redundant sensor.

- **Two thermometer parts across the station, and where the sensor sits decides which.** On a
  board, or on a wall the board touches, a **digital part on I²C — `TMP117` or `STS35`, two
  makers, one fitted** (TMP117: ±0,1 °C max over −20…+50 °C, ±0,15 to +70, 16 bit, 3,5 µA at
  1 Hz, WSON 2×2; STS35: ±0,1 °C typical over 20…60 °C, 0,01 °C, −40…+125 °C, DFN-8; addresses
  0x48 and 0x4A, so one driver tells them apart) — Quake's and Gauss's wall, Kronos's TCXO,
  Pascal's tube wall, Pluvius's cell — and Ceres's and
  Sakura's plate, on the board under the glass, because there the temperature is a product and the front end
  is off while it is read. **Inside a sensing volume, or at the end of a lead that only
  corrects, an NTC on the board's ADC** — 10 kΩ, 1 %, ratiometric against the ADC's own reference so the reference
  cancels, the divider switched from a GPIO for the conversion only: Tesla's rods, between
  Gauss's and Quake's coils, in Pluvius's vessel, **on `Quark-Tubes`
  and its four heads, five of one kind** — a board that spends its pins on inputs and whose
  temperature only corrects (`quark/tubes/HARDWARE.md`) — **and on every H7A3 measuring board:
  Photon's crystal, SiPM and PIN, Positron's block and photomultiplier base, Marconi's board,
  Tesla's rods**, where nothing asks for a digital part's 0,1 °C and a resistor puts no bus edge
  beside a front end (`quark/WHY.md`). Two order codes: `NCP18XH103F03RB` on a board,
  `NXFT15XH103FA2B` at the end of a lead; 3 kΩ + 100 nF at the ADC pin. It is
  0,3–0,5 °C absolute uncalibrated and 0,1 °C after one point, which every correction in the
  station is content with, and it puts no current pulse and no bus edge where a sensor would
  hear it — a 1-Wire part's 1,5 mA conversion is tens of nT between a magnetometer's coils. **No
  `DS18B20` anywhere** (`core/WHY.md`). Neither is a bought unit's job.
- **Sakura** (leaf wetness) — **a `TMP117`/`STS35` beside the plate**: the compensation and the
  input calibration of the wetness read, and the temperature at the plate reported beside it on
  `0x0001` — one unit, two values.
- **Ceres** (soil moisture) — **a `TMP117`/`STS35` beside the electrode, same reason as Sakura**
  (the capacitive read needs temperature compensation at the probe — water permittivity is
  temperature-dependent), and **Ceres is the soil thermometer of its depth**: the temperature
  rides `0x0001` beside the moisture, one unit, one address, the shape a bought T/RH pod has —
  always fitted, always reported; no bought soil thermometer in the base, and a thermometer at
  another depth or height is the site's own bought unit on an arm (`ceres/README.md`).
- **Gadolin** — **none**: Geiger-plateau counting is temperature-stable; classic dosimeters omit
  it, correctly.
- **Photon's GM tubes and Helion** — **one NTC on each head's board**, K1 · K2 · K3 on the GM
  shaper boards and K4 on the He³ preamplifier, read on `Quark-Tubes` and applied there per channel,
  never transmitted (`quark/tubes/HARDWARE.md`). Helion's pulse-height discrimination (the 764 keV
  peak against gamma) feels a gain drift even in a sealed tube, and the correction is digital.
- **Modbus environmental sensors** (pressure / humidity / VOC) — **none extra**: temperature is built
  into the sensor IC and self-calibrated, so Modbus already delivers a clean value.

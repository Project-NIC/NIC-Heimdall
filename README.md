<p align="center">
  <img src="NIC-Heimdall.svg" width="200"/>
</p>

★ N.I.C. ★

# NIC-Heimdall

**The hardware side of NIC: one self-contained, multi-phenomenon measuring station — the sensing
fronts that watch the solid Earth, the atmosphere and the ionosphere, and the head, clock, cards,
power and cabling that carry them. Commodity parts, light enough to process on a home machine.**

[![License: MIT](https://img.shields.io/badge/License-MIT-red.svg)](https://opensource.org/licenses/MIT)
[![License: CERN-OHL-S v2](https://img.shields.io/badge/License-CERN--OHL--S%20v2-red.svg)](https://ohwr.org/cern_ohl_s_v2.txt)
![Version: 0.2 — concept](https://img.shields.io/badge/version-0.2%20concept-blue.svg)

> ⚠️ **Design-stage concept — NOT hardware-validated.** The architecture, the wire protocol and the
> archive are worked in depth, and the archive's reference codec is **host-tested**; **nothing has
> been built or run on real hardware** — no board fabricated, no firmware written, no sensor calibrated.
> Written for **experienced builders**: it gives the topology and the reasoning, not a tutorial.
> **A concept this size WILL contain mistakes** — consistency between documents is not correctness.
> Found one? That is the system working — check it against the record and fix the record.

> **Part numbers on our own boards are worked reference points, not a mandate.** The documents fix
> the topology and the target values; the exact vendor part is whoever builds it's choice.
> **Bought sensors are never named by type at all** — a named part is the one a builder in another
> country cannot get. Every bought unit is specified by what it must meet (*What is bought*, below).

> **There is no schematic yet — the description is the schematic.** A concept this size moves in
> the documents, not in EDA: every board's `HARDWARE.md` carries its parts, values, pins and the
> arithmetic behind them. Schematics belong to the step from concept to build, and
> [`schematics/`](schematics/) is waiting for them. Whoever draws the first board starts from a
> finished description, not a blank page — and if that is you, the folder is yours. There is no
> station-wide numbering; a board is found under its project by its name.

*(The archive — HMC, the container, and HCC, the codec — is the station's own, in
[`core/archive/`](core/archive/HMC.md).)*

## Start here

| if you want to know | read |
|---|---|
| **what it measures** | [*The units*](#the-units--what-it-measures-and-what-it-measures-it-with), below — one row a unit, each linking to its folder |
| **how a station fits together** | [*What a station is*](#what-a-station-is), then [`core/`](core/README.md) and [`core/COMMS.md`](core/COMMS.md) — the bus, the clock, every link in one table |
| **how the data is kept and shared** | [`core/archive/HMC.md`](core/archive/HMC.md), [`EXPORTERS.md`](core/archive/EXPORTERS.md) and [`core/INTEROP.md`](core/INTEROP.md) — the archive, its readers, and who takes the data |
| **how it is built** | [`daedalus/`](daedalus/README.md) for the station as a structure, each board's `HARDWARE.md` for the board, [`schematics/`](schematics/README.md) for the drawings to come |
| **why it is the way it is** | the `WHY.md` in every folder — what was tried, what was dropped, and the reason |
| **how to help, and how to cite it** | [`CONTRIBUTING.md`](CONTRIBUTING.md) · [`CITATION.cff`](CITATION.cff) |

## What a station is

**A station is one enclosure and whatever fronts a site needs.** Inside the enclosure: the head
(Mayak), the clock (Kronos), one or more cards (Bifrost / Argus), the BMS/MPPT converter (Hermes), a
battery and a solar charger. Every cable that leaves the enclosure crosses a Galvani board —
isolation, surge protection and the feed. Outside: the units, each its own board in its own box,
hanging on a point-to-point spur (copper to 500 m, glass beyond), plus the bought sensors on
Palatine's ModBus arms. **Swap the fronts and it is a different instrument on the same bus and the
same code.** Membership of a network is a property of deployment, not of the station: data is
handed to the networks that already exist (`core/INTEROP.md`).

```
   sky
    │
    ▼
 ┌────────┐   PPS + label   ┌────────┐   4 trunks   ┌──────────────┐   4 spurs   ┌─────────┐
 │ KRONOS │────────────────▶│ MAYAK  │─────────────▶│ BIFROST ×4   │────────────▶│  UNITS  │
 │        │  network clock  │ store  │  point-to-   │ an ARGUS     │             │  Quake  │
 │ the    │                 │ uplink │  point       │ behind a port│             │  Gauss  │
 │ second │                 │        │              │ 1 up + 4 down│             │  Tesla… │
 └────────┘                 └───┬────┘              └──────────────┘             └─────────┘
      ▲                         │
   POLARIS or SPUTNIK           ▼  LP I²C        BIFROST and ARGUS are ONE BOARD, one firmware — a
   (the GNSS receiver)       HERMES ──▶ BMS · MPPT · battery · panel
                                                  card pressed on Kronos's time bus ribbon is a Bifrost

   Every cable that leaves the enclosure crosses a GALVANI board — isolation, protection and
   the spur's feed. Inside the enclosure there is no 485 at all and a unit's link is a CABLE; Kronos's time
   bus to the cards is M-LVDS (kronos/HARDWARE.md).
```

**Time comes from one place.** Kronos disciplines the network clock to GNSS and every node counts
*that*, never its own crystal. Remote units hang behind a card on point-to-point spurs, each spur
its own ranged timing domain; the delivered precision is **±1 µs** across the station.

## The three buses

| bus | frame | who carries it | who hangs on it |
|---|---|---|---|
| **NodBus** | 40 B frame, 32 B payload, TDMA, clocked | a **Bifrost** card: one trunk up, four spur ports down | the full-size units — Quake, Tesla, Marconi, Sputnik, Palatine, Photon, Positron, Pip, Steinmetz, Argus |
| **NodBus mini** | 16 B frame, 8 B payload, TDMA, clocked | an **Argus** card: one link up, four segments down | the small clocked units — Gauss, Quark-Tubes, Pascal |
| **ModBus** | Modbus RTU, polled, 9 600 or 19 200 | **Palatine**, four isolated arms and `PWR EXT`, one rate per arm | every bought sensor, and the house MODs — Pluvius, Ceres, Sakura, Babel |

**Identity is the address**: `TYPE«4 | NUMBER` on NodBus and mini, `TYPE«2 | NUMBER` on ModBus, one
byte, self-describing, never translated. NodBus types: 1 Mayak · 2 Bifrost · 3 Argus · 4 Marconi ·
5 Quake · 6 Palatine · 7 Tesla · 8 Sputnik · 9 Photon · 10 reserved for `Neutron` · 11 Positron · 12 Pip · 13 Steinmetz. Mini: 1 Gauss ·
2 Quark-Tubes · 3 Pascal. ModBus: 4 Babel, bare · 5 Pluvius · 6 Ceres · 7 Sakura, then one type per bought quantity at a
position (`core/PROTOCOL.md`).

## The head, and what hangs off it

| | | MCU |
|---|---|---|
| **[Mayak](mayak/)** | the head — datalogger, uplink (the modem · Wi-Fi · BLE to the handset), the four trunk links every card hangs off; no bus ports of its own | ESP32-S31 |
| **[Kronos](kronos/)** | the timekeeper — TCXO disciplined to GNSS, the network clock at 2²³ Hz, PPS and the second to every card on one M-LVDS ribbon | STM32H523 |
| **[Polaris](kronos/polaris/)** | Kronos's GNSS front, bought — a time-only receiver module and an active antenna on a carrier with one connector and two resistors; no processor. Fitted when the station has no Sputnik | — |
| **[Bifrost](bifrost/)** · **[Argus](bifrost/argus/)** | **one card, one firmware**: on Kronos's ribbon it is a Bifrost and formats 40 B NodBus spurs with the absolute second; off it, an Argus carrying four 16 B mini segments | STM32H523 |
| **[Hermes](hermes/)** | the BMS/MPPT converter — polls the pack and the panel on whatever bus they come with (485 Modbus RTU, TTL, I²C, CAN), hands the head one register block on I²C | STM32H523 |
| **[Galvani](galvani/)** | the transport layer — **eleven boards**, no populations: `G-I-N-025` (485, NodBus) · `G-I-M-005` (485, ModBus arm) · `G-O-10-10` (glass 10 Mb/s, 10 km) · `G-O-2-100` (glass 2 Mb/s, 1 m to 100 km) · `G-12-S` (isolated 12 V for a bought device) · `G-24-S` (the switched 24 V on Palatine's `PWR EXT`, for a load that is not a sensor — a pump) · `G-48-S` · `G-300-S` (the station's feed cells) · `G-48-U` · `G-300-U-6` · `G-300-U-40` (the unit end, 12 V out). Everything that leaves the enclosure crosses one | — |
| **[Daedalus](daedalus/)** | the station as a structure — mast, seat, vault, earthing, enclosure, cable routing; a build guide | — |
| **[Gaia](gaia/)** | the siting atlas — where stations go on the planet | — |
| **[Handset](mayak/handset/)** | the commissioning app — the phone at the open enclosure, over BLE | — |
| **[Mimir](mimir/)** (mini-Heimdall) | Mayak, Kronos, a Bifrost, an Argus, Palatine and Hermes on one PCB, each section as its document draws it, the ribbon taps and the crossed cables as traces; two NodBus ports, four mini segments, four arms, a spare MasterNOD, Ethernet and USB | ESP32-S31 + 5× STM32H523 |
| **[Proteus](proteus/)** | Mayak, Kronos, a Bifrost and Hermes on one small PCB, Polaris plugged in, an LTE-M modem on the backup cell, one copper NodBus run on the board, an isolated DC/DC brick at the input, Ethernet and USB-C — a whole station head in a 1-DIN slot | ESP32-S31 + 3× STM32H523 |
| **[Atlantis](atlantis/)** | the water build — the Galvani boards in their pressure build and a cable chosen for the environment; no unit of its own. Tier 1 (Pascal and Gauss a few hundred metres out) is built from the Galvani family; Tier 2 (a magnetometer 50–100 km out on 300 V) is **shelved** | — |

## The units — what it measures, and what it measures it with

**NodBus units** (40 B frame, 32 B payload, behind a Bifrost):

| | measures | the sensing part | MCU |
|---|---|---|---|
| **[Quake](quake/)** | ground motion and local events, plus tilt; optionally the slow magnetic field | MEMS `ADXL355` + `ICM-42688-P` on independent SPIs, `SCL3300` for inclination, an `RM3100` population — strong-motion, not an observatory seismometer; the *DoubleBarrel* tube for burial | STM32H523 |
| **[Tesla](tesla/)** | VLF sferics — lightning, the fast B-field, TGF timing; the same board is Pip and Steinmetz under their own images | **three ferrite rods** at 120° on a frustum, a differential transimpedance front end, a quad 24-bit ΔΣ converter (`ADS127L14`) at 2²⁰ SPS, its own DSP | STM32H7A3 |
| **[Pip](tesla/pip/)** | the D-region — VLF/LF carrier levels (SID), and longwave time for a station without GNSS | **Tesla's board under its own image**; NodBus type 12. Held back by transmitter coverage, finished as a description | STM32H7A3 |
| **[Steinmetz](tesla/steinmetz/)** | line faults on power lines — arc, corona, partial discharge — from a vehicle at 80–100 km/h or a site | **Tesla's board under its own image**; NodBus type 13. In a vehicle it hangs on a Proteus in the cabin over one hybrid cable | STM32H7A3 |
| **[Marconi](marconi/)** | the F-region — HF carrier levels 0,5–16 MHz, MUF / foF2, a passive ionogram | one vertical air-core loop into direct sampling, `AD9265` at 2²⁶ | STM32H7A3 |
| **[Sputnik](sputnik/)** | the integrated ionosphere — Total Electron Content; and the station's time when fitted | `UM980` multi-band GNSS receiver; five node slots on one board | STM32H523 |
| **[Palatine](palatine/)** | the meteo base — air T/RH at 2 m and 5 cm above the grass, pressure, wind, solar radiation, UV, snow depth, rain, soil moisture and temperature, leaf wetness; and **[Chinook](palatine/chinook/)**, what the air is made of — bought air units on the same arms | **no sensor of its own** — four isolated ModBus arms, bought RS-485 probes specified by what they must meet, and the house MODs below | STM32H523 |
| **[Photon](quark/scintillation/photon/)** | γ / X-ray — count, energy sum, largest event, twelve energy bands, every frame, accumulated over the second | `Quark-Photon` board: CsI(Tl) cube + SiPM and a PIN diode as one measurement switched by energy | STM32H7A3 |
| **[Positron](quark/scintillation/positron/)** | beta, both signs — and the two neutron counts of the same box | `Quark-Neutron/Positron` board: a 25 mm plastic scintillator block + SiPM; [`Neutron`](quark/scintillation/neutron/)'s `⁶LiF/ZnS(Ag)` screens read by a photomultiplier as its first channel — type 10 reserved for it | STM32H7A3 |
| **[Argus](bifrost/argus/)** | nothing — it carries four mini segments and tiles their 8 B records into its own | the card (above) | STM32H523 |

**NodBus mini units** (16 B frame, 8 B payload, behind an Argus):

| | measures | the sensing part | MCU |
|---|---|---|---|
| **[Gauss](gauss/)** | the slow geomagnetic field — the standalone form, when a station runs no Quake or one without the magnetometer | `RM3100` in a sealed tube, oil-filled in the sea build, the Barrel family's pipe | STM32H523 |
| **[Quark-Tubes](quark/tubes/)** | radiation by tubes — every tube channel counted on one board, counts only | `Quark-Tubes` board: the counting H523; the tube heads (Photon's GM tubes behind graded lead, Helion's He³/BF₃ tube, **Gadolin**'s Gd capture + GM tubes with **Rhodion**, Rh activation) carry no MCU and feed it pulses | STM32H523 |
| **[Pascal](pascal/)** | the water column above it — a tsunami gauge on the sea floor | a 30 bar piezoresistive depth gauge, potted in an oil-filled tube | STM32H523 |

**ModBus house MODs** (on a Palatine arm, fed the arm's isolated 12 V, own `THVD1450` front):

| | measures | the sensing part | MCU |
|---|---|---|---|
| **[Pluvius](pluvius/)** | precipitation, weighed — grams in and grams drained | a 200 cm² catch into a vessel on a **30 kg load cell**, `ADS1235`, a counted 24 V peristaltic drain on Palatine's switched `EXT` body | STM32H523 |
| **[Ceres](ceres/)** | soil volumetric water content and the soil temperature at its depth, one unit per depth in a *patrona* | a comb reading through borosilicate glass, the board potted in a borosilicate bowl — built because no bought coating survives the dirt | STM32H523 |
| **[Sakura](ceres/sakura/)** | leaf wetness and the temperature at the plate | Ceres's board in the same bowl, hung in the canopy at 45°, glass to the sky | STM32H523 |
| **[Babel](babel/)** | any sensor not sold as RS-485 Modbus — converted at the sensor, so no foreign bus ever rides the station | I²C · SPI · UART · 1-Wire in, Modbus RTU out; up to four positions, each answering as its quantity's type | STM32H523 |

**Radiation, in two builds:** **[Quark](quark/)** is the radiation part, measuring the same quantities two ways — **[Quark-Tubes](quark/tubes/)**, high voltage, every tube counted on one board (Photon's GM tubes, **[Helion](quark/tubes/helion/)** and **[Gadolin](quark/tubes/gadolin/)** are tube heads on it, not units of their own), and **[Quark-Scintillation](quark/scintillation/)**, low voltage — Photon, Positron and its `Neutron` channel.

**MCUs, and there are three.** `ESP32-S31` at the head · `STM32H523` (LQFP100, the one footprint) on nodes, Kronos, the card and every slave module · `STM32H7A3IIT6` (LQFP176) on the DSP tier — Tesla/Pip, Marconi and the two scintillation boards. **The four H7A3 boards share one digital half**: the same LQFP176, the `APS25608N-OBR-BD` position on OCTOSPI1 port 2 on the same eleven pins (fitted where the image wants it — Marconi always, Tesla for the classifier, Photon for the burst records, Positron open), the same Galvani output. What differs is the analogue front and the converter's data path — the `ADS127L14`'s frame-sync port on SAI on Tesla/Pip, a parallel port on PSSI on the other three.

## What is bought

**The rule: a finished unit costs a few dollars more than the bare sensor inside it, and the
sensing element is the same part at every price — so nothing that can be bought is built.** The
station's own boards are the ones the market does not sell in a form that lasts: the head, the
clock, the card, the Galvani family, the units above. Everything else is a purchase, and it is
specified by what it must meet; where a document names a type, it is a recommendation a builder can start from, never a requirement.

**Bought sensors — on Palatine's ModBus arms** (`palatine/SENSORS.md`). All of them **RS-485
Modbus RTU, 9 600 or 19 200 8N1, powered from the arm's 12 V (10–30 V input), rated to −40 °C**,
in one housing with a cable:

| quantity | qty | what the unit must meet |
|---|---|---|
| air temperature + humidity | 2 — at 2 m in a shield, at 5 cm above the grass without one | ≤ ±0,3 °C, ≤ ±2 % RH, stainless IP6x probe, in a radiation shield |
| soil temperature | the Ceres depths | **Ceres reports it**, always — the second value of every Ceres; a thermometer at any other depth or height is the site's own bought RS-485 unit on an arm, not the base |
| pressure | 1 | absolute 300–1100 hPa, ≤ ±1 hPa; on an arm like everything else |
| wind | 1 | speed + direction; mechanical or ultrasonic by climate |
| solar radiation | 1 | pyranometer 0–2000 W/m² |
| UV | 1 | UV index, 290–390 nm; UVA + UVB in W/m² where the unit gives them; no UVC |
| snow depth | 1 | 80 GHz FMCW radar level sensor, ≤ 3° beam, ±1 mm, blanking under ~10 cm, IP67, solids-rated, −40 °C standard |
| soil moisture | 2, two depths | **Ceres is the house build** — the moisture and the soil temperature from one unit; a bought capacitive RS-485 probe only where nobody builds it |
| air quality (optional) | as wanted | none in the base — a site fits by setting (forest CO at fire level, city PM/NO₂/O₃, industry its own gas), every one a finished RTU unit and a consumable (`palatine/chinook/`) |

**Bought parts elsewhere in the station:**

| what | where | what it must meet |
|---|---|---|
| GNSS receiver, time-only | Polaris, on Kronos's socket | a bought u-blox NEO-M8N, the one typed kind, with an active antenna on the mast; typed, never detected, and configured by Kronos at every boot |
| GNSS receiver, TEC + time | Sputnik | `UM980`, multi-band |
| battery | the enclosure | LiFePO4, 2P4S of 314 Ah, 8,0 kWh — the ceiling; charged between 20 and 80 %, ~5 days of the full load — the station does not save power |
| BMS and MPPT | behind Hermes | any pair that speaks 485 Modbus RTU, TTL, I²C or CAN; the BMS must switch itself back on when the pack recharges |
| solar panel | the mast | sized by the site (`gaia/`) |
| data cable | every fed run | outdoor UTP Cat 6, all four pairs: data TX · clock · ground · data RX — full duplex |
| power cable | every fed run | 2-core, 2× 1,5 or 2,5 mm², 300/500 V class, on its own on land — a hybrid cable the option, the data cable's own jacket under water |
| optical modules | `G-O-10-10` · `G-O-2-100` | the 10 Mb/s DC-coupled class to 10 km; `OPT2-55A03STR` (1550 nm, 1 m to 100 km) on the 2 Mb/s board |
| transformers | the Galvani feed cells | six catalogue windings — `750310988` · `750311607` · `750310349` · `11328-T078` · `11338-T195` · `750311592` |
| enclosure, mast, earthing | Daedalus | `daedalus/CONSTRUCTION.md` |

## Power, in one paragraph

The battery rail is the station's 12 V, a star from the enclosure's fuse field, one fuse a board;
every board makes its own low rails from it. A fed run leaves the enclosure at **48 V**, or **300 V** where 48 does not carry
the load that far, made by a Galvani source cell and converted back to 12 V by the unit board at the
far end. A run's trip point is measured at commissioning and programmed into an `INA238` at the
source end, under the cell's ceiling — ~26 W at 48 V, ~47 W at 300 V. Copper to 500 m, glass beyond.
**The concept is built on what is normally on sale**: stock outdoor Cat 6, not a specialist cable
(`galvani/README.md`).

## Why it matters

- **Seismic** — local ground motion and event detection; a dense grid is the basis of
  earthquake early-warning-style alerting.
- **Ionospheric TEC** — multi-frequency GNSS measures Total Electron Content directly: space
  weather, HF propagation, positioning correction, and the transients that shock the upper
  atmosphere.
- **Radiation & lightning** — cheap relative monitors: the statistics of many nodes, not one
  precise instrument.
- **Cross-coupling** — a big seismic event sends acoustic-gravity waves up into the ionosphere
  minutes later; running a Quake and a Sputnik at one site captures both ends.
- **Density is the instrument.** The satellites move and the air moves, so a network samples an
  evolving **4-D volume** rather than a set of static points — mesoscale spacing already resolves
  what a sparse net cannot.

## Why the elaborate node is the frugal one

The tempting "simple" station streams its raw signal out and lets a server think. It is a false
economy on the one budget that binds an unattended solar node: **power.** DSP at the edge costs a
few milliamps on a processor that is running anyway; streaming raw keeps the **radio** busy, and the
radio is the glutton. **So the complex node is the low-power node** — and power is not a battery-life
number, it is the price of the station, because consumption scales solar and storage linearly.

The same doctrine on the data: **store the change, not the raw** · **detection, not metrology** —
knowing *that* something happened is one threshold, absolute flux is where cost multiplies. **The
network is the instrument.** And because the edge already produces the finished per-station product,
joining an existing aggregator is a trivial insertion on their side, not a new pipeline.

## What it costs

> **Back-of-envelope, on purpose.** Order-of-magnitude figures meant to show a *ratio*. Station
> costs vary ±2×; the point survives the error bars by a wide margin.

**One rough model, parts only, and every other figure in the project is read against it:**

| what | about |
|---|---|
| the base — Mayak, Kronos, a card, Palatine, Hermes; the 8 kWh pack and the 570 W panel; the enclosure, the vault, the mast and the earthing; a bought weather set | **~$3–4 k**, half of it the pack, the panel and the structure |
| a DSP-tier unit — Tesla, Marconi, a Quark-Scintillation board — with its Galvani pair and its cable | **~$0,5–0,8 k** each |
| an H523 unit — Quake, Gauss, Pascal, Pluvius — with its Galvani pair and its cable | **~$0,2–0,5 k** each |
| **a full station**, the base and five or six units | **~$7–9 k** |

| scope | stations | spacing | @ ~$4 k (base) … ~$8 k (full) | = of global military spend |
|---|---|---|---|---|
| one country (ČR) | 100 | ~28 km | ~$0,4–0,8 M | **~5–10 seconds** |
| Baltic–Adriatic corridor | 1 000 | ~28 km | ~$4–8 M | ~1–1,5 minutes |
| **all of Eurasia** | 65 535 | ~29 km | **~$0,26–0,52 B** | **~0,8–1,7 hours** |
| whole planet, mesoscale | ~178 000 | ~29 km | ~$0,7–1,4 B all-in | ~2,3–4,6 hours |

That Eurasia row is not a typo: a 2-byte station domain at mesoscale spacing blankets essentially
the whole landmass, putting **~2,7 million simultaneous rays** through the atmosphere at once. A
planet-wide civil geophysical instrument is **one large infrastructure project** — a metro line, a
long bridge — not a moonshot. *(Global military spending ≈ $2,7 T/year, SIPRI public figures.
A yardstick, not a political position.)*

**Not money, and not physics.** At continental scale the binding constraint is **cross-border
coordination**. NIC is architected to grow bottom-up: every station is autonomous, the network
assembles piece by piece, and you do not need permission from a continent to start with a hundred
stations in one.

## Code

**No firmware is written yet, and none is in this repository.** A unit's firmware is described in
its `FIRMWARE.md` — boot, states, the bus, time, registers, faults — and is written against that
description once the board's `HARDWARE.md` has settled: the board first, the firmware second.

The code that is here is of two kinds, both plain Python 3:

| | |
|---|---|
| the archive's reference codec | [`core/archive/ref/`](core/archive/ref/) — HCC as `HCC.md` fixes it, the Steim-2 it is measured against, and its tests (`python3 tests/test_hcc.py`, and `tests/test_damaged.py` for streams that are not whole) |
| the calculations behind the documents | [`gaia/tools/`](gaia/tools/) the siting maps · [`gauss/models/`](gauss/models/) the shallow-water siting model · [`marconi/models/`](marconi/models/) the loop's pointing · [`tesla/design/`](tesla/design/) the rod worksheet, its field solve and the chain's clip and floor |

## The tree

Every project is a folder, and a part that belongs to another sits in a subfolder of it. The right-hand
column is the bus and the type.

```
NIC-Heimdall/
├── mayak/              Mayak — the head: datalogger and uplink           NodBus 1
│   └── handset/        Handset — the commissioning app, over BLE
├── kronos/             Kronos — the clock
│   └── polaris/        Polaris — Kronos's GNSS front, bought
├── hermes/             Hermes — the BMS/MPPT converter
├── bifrost/            Bifrost — the card, NodBus master                 NodBus 2
│   └── argus/          Argus — the same card, NodBus mini master         NodBus 3
├── galvani/            Galvani — the eleven transport boards
├── mimir/              Mimir — mini-Heimdall, the station on one board
├── proteus/            Proteus — the smallest whole station head
├── quake/              Quake — seismograph                               NodBus 5
├── tesla/              Tesla — lightning and sferics                     NodBus 7
│   ├── pip/            Pip — longwave carriers: SID and time             NodBus 12
│   └── steinmetz/      Steinmetz — line faults on power lines            NodBus 13
├── marconi/            Marconi — the HF ionosphere                       NodBus 4
├── sputnik/            Sputnik — GNSS TEC, and the station's time        NodBus 8
├── palatine/           Palatine — the meteo base, ModBus master          NodBus 6
│   └── chinook/        Chinook — air quality, bought units
├── quark/              Quark — the radiation part
│   ├── tubes/          Quark-Tubes — high voltage                        mini 2
│   │   ├── helion/     Helion — He³ / BF₃ neutron head
│   │   └── gadolin/    Gadolin — Gd neutron head, with Rhodion
│   └── scintillation/  Quark-Scintillation — low voltage
│       ├── photon/     Photon — γ / X-ray                                NodBus 9
│       ├── positron/   Positron — beta                                   NodBus 11
│       └── neutron/    Neutron — the photomultiplier channel             type 10 reserved
├── gauss/              Gauss — the magnetometer sonde                    mini 1
├── pascal/             Pascal — the pressure sonde, tsunami              mini 3
├── pluvius/            Pluvius — the weighing rain gauge                 ModBus 5
├── ceres/              Ceres — soil moisture                             ModBus 6
│   └── sakura/         Sakura — leaf wetness                             ModBus 7
├── babel/              Babel — any sensor → Modbus                       ModBus 4 bare
├── atlantis/           Atlantis — the water build
├── daedalus/           Daedalus — the station as a structure
├── gaia/               Gaia — the siting atlas
├── schematics/         the drawings to come, one folder a board
└── core/               the node core — bus, frame, clock, archive
```

## Where the rest lives

| | |
|---|---|
| the node core, the bus, the frame, the clock | **[`core/`](core/README.md)** |
| what talks to what, over what | **[`core/COMMS.md`](core/COMMS.md)** |
| names, MCU assignment, who is a NOD and who a MOD | **[`NAMING.md`](NAMING.md)** |
| who takes our data and in what format | **[`core/INTEROP.md`](core/INTEROP.md)** |
| the archive — HMC, the container, and HCC, the codec | **[`core/archive/`](core/archive/HMC.md)** |
| the exporters — every format the world takes, written from the archive | **[`core/archive/EXPORTERS.md`](core/archive/EXPORTERS.md)** |

## Open

Hardware under **CERN-OHL-S v2**, software under **MIT**. Build it, change it, ship it — how to
help is [`CONTRIBUTING.md`](CONTRIBUTING.md), and how to cite it [`CITATION.cff`](CITATION.cff).

★ Viva La Resistánce ★

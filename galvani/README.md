<p align="center">
  <img src="NIC-Galvani.svg" width="200"/>
</p>

★ N.I.C. ★

# Galvani — the transport boards

> **Design-stage concept — nothing drawn, nothing built.** This file is the doctrine and the
> function of every board; the parts, values and calculations are [`HARDWARE.md`](HARDWARE.md),
> the bill of materials [`BOM.md`](BOM.md), the rejected alternatives [`WHY.md`](WHY.md). **It is a
> concept: before a schematic is drawn, every figure here is verified against the parts' sheets
> and the parts themselves.**

**Galvani is the station's transport layer, built as a kit of small boards.** A **power board**
makes the feed or takes it off the cable; a **communication board** carries the link. A host —
a card, a unit, Palatine — talks to either over a short ribbon and neither knows nor cares what
transport hangs on the far side. Everything that leaves an enclosure is isolated, the mast runs
included; inside an enclosure a link is a cable and there is no supply board at all.

**Eleven boards, named by what they do.** Communication, named by what is on the cable, the
rate and the reach: **`G-I-N-025`** 485 on copper for NodBus · **`G-I-M-005`** 485 on copper for
ModBus · **`G-O-10-10`** glass, 10 Mb/s to 10 km · **`G-O-2-100`** glass, 2 Mb/s, one module from a metre
to 100 km. Power, named by voltage and end: **`G-48-S`** and **`G-300-S`** make the feed
at the source end from the 12 V they tap off the wire; **`G-48-U`**, **`G-300-U-6`** and
**`G-300-U-40`** take it off the cable at the unit end and deliver **12 V and nothing else**;
**`G-12-S`** is the isolated 12 V for a bought ModBus device; **`G-24-S`** is a switched 24 V for
a load that is not a sensor (a pump). **`S` is the SOURCE end — where the run is fed and the master
stands, at the station or at a remote Argus alike; `U` is the UNIT end.** A board has a name and
no number; the name is the whole identity.

> ## PRESSURE BOARDS AND LAND BOARDS — TWO BUILDS, NOT ONE
>
> **PRESSURE: `G-48-U`, `G-300-U-6`, `G-O-2-100` with the pressure module, and `G-I-N-025` at
> a unit end** — no tube fitted, the capacitors ceramic; the land build of the same board is as it
> stands. Built for an oil-filled, pressure-balanced pod to
> **350–500 bar**. **Solid parts only**: MLCC ceramic, resistors, plastic-moulded semiconductors,
> wound magnetics the oil soaks. **No film, no electrolytic, no hybrid polymer, no air-cavity part**
> — relay, crystal, MEMS, a module with a lid. A pressure board is a land board too; the reverse is
> never true. **The pod is filled under vacuum, which draws the air out of every winding with the
> rest**, so a transformer the fill has not wholly soaked is no fault.
>
> **LAND ONLY: `G-48-S` · `G-300-S` · `G-300-U-40` · `G-12-S` · `G-24-S` · `G-I-N-025` at a
> source end · `G-I-M-005` · `G-O-10-10` · `G-O-2-100` with the catalogue module.** They carry film, hybrid polymer or cavity parts.
>
> **`G-O-2-100` is land-only as long as its modules are catalogue parts** — every 1×9 module
> has an air cavity behind its lid. A pressure link is built only with a pressure-tolerant module
> obtained from the maker, and that is sourced before the link is drawn.
> **Every board outside the enclosure is the outdoor build, and the cable follows the ground it
> lies in.** On land any cable made for burial serves. **In water the cable is built for water** —
> its insulation, a water-blocked core, and armour that takes the laying tension to the depth it is
> laid at — copper and glass alike. The interface in a good housing is not the question; the cable
> is (*Under water*, below).

## The doctrine

**One threshold, and everything hangs off it: does the run leave the enclosure?**

| | inside the enclosure | **anything that leaves — the mast included** |
|---|---|---|
| barrier | none | **isolated, always** |
| what it runs on | **the battery rail as it stands, handed on as the 12 V** | **48 V**, or **300 V** where 48 does not carry the load that far; a bought ModBus device takes **12 V** off the unit board's island, or off `G-12-S` at the station |
| what is on the wire | **UART · I²C · the clock and PPS as wires behind a buffer; Kronos's time bus on M-LVDS** — no 485 at all | 485 or glass, and it starts on the board |
| the board | **none — a cable** | **a power board and a communication board at each end** |

Not computed — decided by lightning, and the criterion is **inside-or-outside, never
distance**: the risk is what a cable is routed *past*. A strike raises the mast by hundreds of
kilovolts in a microsecond, and coupling into a cable routed through it is `I = C·dV/dt` across
the cable-to-mast capacitance — ten picofarads against 10¹¹ V/s is an ampere, so three metres
up the mast is a worse position than five hundred buried. The battery rail never travels: the
moment a run leaves, it leaves through a board.

**48 V is the number.** The station makes its own feed, so there is no band to accept and no
outside source to qualify; feeding a spur from the battery rail dies at 600 m on a 3 W load.
**300 V is the higher feed and it is picked by watts × distance, not by a class**; it is one
nominal, not a range.

**One deliberate exception: the GNSS antenna coax.** RF plus bias cannot be isolated, so it leaves
the enclosure without a barrier and pays entry treatment instead — its arresters
(`../core/blocks/gps-pps.md`). Everything else that leaves is a bus or a feed, and goes through a
board. A radiation head's pulse leads stay inside its assembly: the head counts on its own
processor and answers on a bus, so what leaves is an ordinary bus behind an ordinary board.

### The in-box link — a cable, not a board

What an in-box link carries is plain logic between two host boards: no transceiver, no
barrier, no ladder, no termination. UART runs against ground, so a common ground is the whole
requirement; the `CLK` series resistor sits at its source, on the host board.

**Every socket in the station has the same pinout, so the in-box cable is CROSSED**: `TXD` to
`RXD`, `ID` to `ID_RET`; `CLK/PPS` and `GND` straight. Everything that is an output on both hosts
(`DE`, `LINE_EN`, `B_DIR`) and everything that has no partner on a host (`RXD_ECHO`, `SD`,
`3,3 V`) is not in the cable. **Six wires — conductors 1 to 6 of the data connector, `CLK/PPS` ·
`GND` · `TXD` · `RXD` · `ID` · `ID_RET` — and the crossing is two adjacent pairs, 3–4 and 5–6,
swapped at one end**: a six-way ribbon pressed straight into one `FC-12P` at pins 1–6 and, at the
other, split between conductors 2–3, 4–5 and 6 and re-pressed with each of the two pairs turned
over. The 12 V comes off the wire on each host's own terminals; the power connector has no part
in an in-box link, because there is no power board in the box.

**For the builder: two cables that look alike and are not.** **Host to a Galvani board — straight**,
pin 1 to pin 1, the ribbon pressed through. **Host to host inside the box — crossed**, 3–4 and 5–6
turned over at one end. It is the oldest arrangement in serial links — RS-232's DTE and DCE: unlike
devices on a straight cable, two alike on a null-modem — and the Galvani board is the unlike side. **The
crossed cable is made from a ribbon of another colour and carries an `X` label at both ends**, so it
is told apart on sight; a straight ribbon between two hosts, or a crossed one to a Galvani board,
joins two outputs or two inputs and the `ID` read reports it before anything is fed.

**Each host reads the other's number on `ID`** (*`ID` — one scale*), so both sides know before
they are powered that there is no run leaving the enclosure and no feed to gate, and a host
plugged where it does not belong reads as a fault before anything is fed. A cable cannot be used
to reach past a gland: there is no barrier on it and nothing that could be strapped to add one.

### The rails — every host makes its own, and the 12 V is a wire

**There is no supply board in the box.** The battery is the 12 V node, 10–20 V, and **it is a
wire, not a board**: every board that makes its own rails takes it on two two-pole terminals, an
input and a tap, the same node (*The connectors*). **In the enclosure the 12 V leaves the pack
through a fuse field, one fuse to a board** — the Mayak, Kronos, every card, Palatine, Sputnik
and every source power board each on its own fuse; the tap carries the wire on only where the
source limits its own current, a remote Argus's `G-300-U-40`. No board carries 12 V across itself
for another board: a card's port is two ribbons and nothing else, a source power board takes the
12 V on its own terminals.

**Every 12 V input is a `5.0SMDJ14A` across the terminals behind that fuse.** It stands off 14 V and
starts to conduct at 15,6–17,2 V, above the pack's charge stop and below the 18 V absolute maximum
of the lowest-rated part on a 12 V node, the `TPS629206`. **The fuse is what the transil protects
against**: a pack connected the wrong way round drives the transil forward and the fuse clears; an
overvoltage the transil clamps for longer than a surge heats it until the fuse clears. Either way
the fuse is lost and the board is not. The fuse is fast-acting and **sized by the construction, not
by the board**: per install, on the branch's input power, the pack's fault current and the thinnest
wire to the fuse field (`../daedalus/CONSTRUCTION.md`) — no rating is written on a board. The field
is the enclosure's, not a board's.

**The two ends of a run are not alike on the 12 V, and the current is why.** A unit power board
*hands out* 12 V on its terminals — 0,3 A to a unit at 3 W, 3,33 A off a `G-300-U-40`. A source
power board *takes in* the 12 V it makes the feed from: `G-48-S` takes 28,4 W in, 2,4 A at a 12 V
pack and 2,6 A at the 11 V floor; `G-300-S` takes 50,6 W in, 4,6 A at the same floor; `G-24-S` ~3 A
while its load runs; `G-12-S` 0,5 A. None of it is a ribbon current, which is why no board of the
family puts 12 V on a ribbon at all.

**Every host makes its own low rails from the 12 V** (`../core/POWER.md`), and nothing in the box
regulates for anyone else. A unit power board hands out 12 V and nothing lower — no buck on any
Galvani board; a communication board has no terminal and no buck, it takes 3,3 V from its host
over the connector.

**The 12 V a bought device wants is a unit power board's island output on its tap** — a 10–30 V,
12–24 V or 5–24 V window is the common supply spec of a bought part, and 12 V serves all three —
or, a few metres from the station, `G-12-S`: the battery across a barrier and straight onto a
short run, one board where a feed cell and a unit board would be two. Where the problem is reach
rather than voltage, the host goes out there on a fed run and its leaves stay short behind it.

### The two choices, and they are independent

**The feed is decided by watts × distance; the communication board by medium × distance.** A
300 V run with a `G-I-N-025` at two hundred metres is a legal build and so is a 48 V run with a
`G-O-2-100`. Two cross-sections and no more on the feed cable: **2× 1,5 mm² and 2× 2,5 mm²**.

**A run's trip point is measured, and the cell is the ceiling.** At commissioning the far end's
start-up peak and running load are measured and the source board's `INA238` is programmed to
them with margin — the host's per-port register, rewritten when a sensor is added. There is no
load class between the measurement and the cell: a far end at 3 W gets a trip near 4 W, one at
21 W near 24. **What the cell delivers is the wall**: `G-48-S` ~26 W and `G-48-U` ~25 W on the far
side, so a 48 V run carries up to ~25 W at its far end; `G-300-S` ~47 W, of which `G-300-U-40`
delivers 40 W and `G-300-U-6` 6 W. **300 V buys reach, not watts**: a far end further out than
the 48 V table carries goes to 300 V; a load past ~25 W at any distance needs a different cell.

**One source board may feed several unit boards.** Every unit board takes the feed in on one
terminal pair and hands it on, straight through, on the other, so units chain on one feed cable.
The chain is one far end to the source: its loads and every unit board's loss are summed, the
source cell's ceiling is shared, the `INA238` threshold is programmed to the sum, and the reach
tables are read with the whole sum at the last unit. One `ENABLE` switches the whole chain; a
unit that must be switched alone takes a run of its own. **A NodBus mini segment takes one source
board and its units chain on it**: they already share the segment's pair, so a unit that holds the
bus takes the segment down whatever feeds it, and a feed of their own each would buy no failure
domain the data does not already lose. A unit board shorted at its input — a transil gone short
after a surge — takes its segment dark until it is replaced; one shorted behind its converter
does not, because the unit board regulates its own output current.

```
   A RUN, END TO END — four boards and two cables

   SOURCE END                                            UNIT END

   ┌────────────┐                                        ┌────────────┐
   │  the host  │◀── 12 V, its own terminals             │ the unit's │
   │            │                                        │ own board  │── its own bucks
   └──────┬─────┘                                        └─────┬──────┘
          │ two ribbons                                        │ two ribbons
          │ 3,3 V · logic — the power board taps the 12 V wire │ 12 V from the power board's terminals
      ┌───┴────┐                                          ┌────┴───┐
   ┌──────┐ ┌──────┐                                  ┌──────┐ ┌──────┐
   │ COMM │ │POWER │══ 2-core feed cable ════════════▶│POWER │ │ COMM │
   │ 485/ │ │  S   │   (its own cable on land; in a   │  U   │ │ 485/ │
   │glass │ │      │    hybrid or under water, cores  │      │ │glass │
   │      │ │      │    in the data cable's jacket)   │      │ │      │
   └──┬───┘ └──────┘                                  └──────┘ └───┬──┘
      │                                                            │
      └═══════ 485 pairs or fibre ═════════════════════════════════┘

   …and where the link never leaves the box there is no board at all: the same data socket takes
   the crossed cable to the other host, and the 12 V comes off the wire on the unit's own terminals.
```

## The common part — what every board shares

### The connectors

**A port is two ribbon connectors and, on a board that carries 12 V, two terminals — split by what
the wire carries.** The **data connector** runs between a host and a communication board, and
between two hosts in the box; the **power connector** between a host and a power board. The pinout
is the same in every socket, so a board plugs straight in and only the host-to-host cable is
crossed. Neither connector carries a rail that can hurt anything, so a cross-plug damages nothing
and is reported by the `ID` read.

**The data connector — 12 pins, 2×6.** IDC numbering: odd pins on one row, even on the other,
and ribbon conductor *n* lands on pin *n*.

| pin | signal | direction | what it is |
|---|---|---|---|
| **1** | `CLK/PPS` | host → board, or board → host | **one pin for channel B.** The 2²² (or 2¹⁹) wire clock outward, or a PPS edge inward — which, the socket decides (*The reversed channel*). On the ribbon's edge with its ground beside it; the 33 Ω series resistor at its source |
| **2** | `GND` | — | the return, one of two |
| **3** | `TXD` | host → board | channel A, the logic side is always full-duplex UART |
| **4** | `RXD` | board → host | channel A back |
| **5** | `ID` | board → host | **one resistor to ground on the plugged board**, read on an ADC pin of the host against 10 kΩ 1 % to the host's analogue rail (*`ID` — one scale*) |
| **6** | `ID_RET` | host → the far host | **a host's own number**, the same kind of resistor, for the host across a crossed cable; unconnected on a Galvani board and on a host that never meets a crossed cable |
| 7 | `RXD_ECHO` | board → host | the echo-check receiver's output — the second `ISO1452` on copper, the receive-only tap on glass; every unit hears its own transmission (`../core/blocks/nodbus.md`). Unconnected on a master's socket: a master never echo-checks |
| 8 | `DE` | host → board | the driver enable, around the unit's slot; on glass nothing to gate, the module is dark when idle |
| 9 | `B_DIR` | socket → board | **tied in the socket, no processor pin**: to 3,3 V where this end drives channel B, to ground where it listens; the board carries no pull (*The reversed channel*) |
| 10 | `LINE_EN` | host → board | switches the isolated line side — the `SN6505B`'s `EN` on a copper board; **not populated on glass**, nothing switches a module. Pulled down by 100 kΩ on the board. **At a source end it is the host's GPIO, one net with the port's `ENABLE`; at a unit end it is 10 kΩ to 3,3 V in the socket and no processor pin** — the line side runs whenever the unit does |
| 11 | `SD` | board → host | the optical module's no-light alarm, high on loss; unpopulated on copper, so the host pulls it down by 100 kΩ. Beside `3,3 V` so the likeliest short reads as a permanent alarm |
| 12 | `3,3 V` | host → board | the host's rail — the communication board's whole supply; an optical board's 175 mA at a master port is the heaviest and one pin carries it |

`RE#` is not a pin: the receiver is held active on the board. `TXD`/`RXD` on 3–4 and `ID`/`ID_RET`
on 5–6 sit side by side so the in-box cable is a six-way ribbon with two adjacent pairs swapped.

**The power connector — 8 pins, 2×4.**

| pin | signal | direction | what it is |
|---|---|---|---|
| **1** | `ENABLE` | host → board | into the converter's `EN`, same ground, no opto; **pulled down at the receiving end**, so a reset, a cut ribbon and a dark port leave the port off — **a source board's converter is off until its host switches it on**. A source power board's pin, driven by the host's GPIO; at the unit end the same pull goes to the input on the board, the board runs whenever the feed is there, and the host's pin is unconnected. Its one ribbon neighbour is ground |
| **2** | `GND` | — | the return |
| 3 | `ID` | board → host | the board's resistor to ground, as on the data connector — a power board answers with a resistor before its `INA238` is programmed, and when it does not answer |
| 4 | `SDA` | ↔ | the `INA238`'s I²C, across the board's `ISO1642`; **4,7 kΩ to 3,3 V on the host**, one pair per controller |
| 5 | `A_SEL` | socket → board | **strapped low or high in the host's wiring, no processor pin**; crosses the `ISO1642` onto the `INA238`'s `A0`, `A1` grounded on the board: **0x40 low or unwired, 0x41 high** — two identical boards on one bus are two addresses, and a controller carries at most two power sockets. Stands between `SDA` and `SCL` |
| 6 | `SCL` | host → board | the I²C clock |
| 7 | `ALERT` | board → host | the `INA238`'s alarm, **high on alarm, a pin per socket on the host, never a wire-OR** — one stuck alarm must not blind the rest; **100 kΩ to ground on the host**, so an empty socket reads quiet. Beside `3,3 V` so a short there is a permanent alarm |
| 8 | `3,3 V` | host → board | the host side of the `ISO1642`, a few milliamps |

**The same connector carries Hermes, the head's BMS/MPPT converter**, on the Mayak: an I²C slave,
3,3 V over the connector through a 0,15 A polyfuse, `ID` at 0,90, `ENABLE` not populated
(`../hermes/README.md`). It is the one board on this connector that is not a power board.

**The part — one connector type across the whole station, in three widths.**

| | part | 2×4 · 2×5 · 2×6 |
|---|---|---|
| **on the board** | **`BX2.54-2xNA`** — 2,54 mm dual-inline shrouded box header, THT, polarising cut-out, no ejectors | overall 17,78 · 20,32 · 22,86 mm; 3 A, 550 V AC/min, −55…+105 °C |
| **on the cable** | **`FC-xP`** — IDC socket for 1,27 mm ribbon, with its strain-relief cover | `FC-8P` · `FC-10P` · `FC-12P`; 1 A, 250 V, −40…+105 °C, 20 mΩ |

The pair's rating is the socket's: 1 A · 250 V · −40…+105 °C. **Widths are what separate the
bodies — 12 the data connector, 8 the power connector, 10 Kronos's time bus** (its own connector,
`../kronos/HARDWARE.md`, the same part family). No second pitch, no keyed variant, no pin-removal
key; nothing latches — the socket's strain relief and a tie hold the ribbon. Within one width a
wrong plug costs nothing and is reported by the `ID` read; the 12 V, the one place a wrong plug
destroys something, is on a different part. **The board says what it is in silkscreen** — its name
and its `ID` value beside the connector, and every socket says what it expects.

**One terminal block in the station — Degson `DGPS2.5R-5.0`, order code `10060008518`**: a PCB
PUSH-SNAP spring block, two poles, 5 mm pitch, 0,75–2,5 mm² solid or flexible (18–14 AWG) without
a ferrule, strip 9 mm, 20 A, 400 V (III/2), 4 kV rated surge, −40…+105 °C, PA66 UL94 V-0,
tin-plated. The spring is delivered open: the wire is pushed to the end, the button snaps, and the
click is the check. **It stands on every terminal position of the family**: the two 12 V terminals
of every board that carries 12 V at all — an input and a tap, the same node — the two pairs of
every feed position, 48 V or 300 V, out of a source board and in and out on a unit board, the data
pairs of the communication boards, a MOD's four wires and a bought sensor's lead. A position of
more than two poles is that many two-pole blocks side by side. A field conductor enters facing the
gland.

**Left to right, input to output, on every board.** The 12 V terminals stand at the left edge and
whatever the board puts out — a feed, the 24 V, the arm's 12 V — at the right; a technician
opening the enclosure reads a card as the battery arriving on the left and the converter's output
leaving on the right, and never has to read a label to know which end is which.

**One pole to a net.** A block has one pole per electrical net and no IN/OUT pair: a run that
carries on to the next unit is twisted into the pole it shares — the two conductors stripped
together and pushed in as one.

| position | block | poles | what |
|---|---|---|---|
| `G-I-N-025` | 4× `DGPS2.5R-5.0` | **8** | the data cable's four pairs |
| `G-I-M-005` | 4× `DGPS2.5R-5.0` | **8** | the arm's pair on four positions — 4× `A`, 4× `B` — the star of up to four sensor cables |
| `G-12-S` | 4× `DGPS2.5R-5.0` | **8** | the arm's 12 V on four positions — 4× +, 4× − |
| every board that carries 12 V | 2× `DGPS2.5R-5.0` | **4** | in and tap: + and − each way |
| a feed position | 2× `DGPS2.5R-5.0` | **4** | two pairs, 48 V or 300 V |
| `G-24-S`'s output | `DGPS2.5R-5.0` | **2** | the load's 2-core |
| a MOD on an arm | 2× `DGPS2.5R-5.0` | **4** | `A` · `B` · `GND` · 12 V, in that pole order |

What is communication between boards goes on the ribbon connectors, never on a terminal; the
terminals carry field wire and power. A card carries four poles and nothing else: its 12 V in and
its tap.

**The earth leaves a source board on a stud, never on the block.** The tube's centre is a kA path
for 20 µs, and a spring block is not a kA part: at 10 kA the constriction of the current in the
contact spot repels the two surfaces with a force of the same order as the spring's own
pressure, and the joule or two released in the spot in that time pits or welds it. A bolted joint
has neither problem. The stud is an **M4 plated through-hole** with a pad of at least 10 mm on
both layers and a ring of stitching vias around it, and the stack is the electrician's — **screw ·
plain washer · the board · plain washer · the strap's crimped ring lug · plain washer · spring
washer · nut, all brass**. The strap itself is 2,5 mm² and a few centimetres long: the pulse heats
it by about one kelvin, so the cross-section is not the question, and the voltage across it is
the inductance of its length (`../daedalus/CONSTRUCTION.md`). At a remote site, where nothing is
earthed, the hole stays empty.

### Connector names — one scheme across the station

**Every connector is named by what it carries and which way, on the silkscreen and in every
document; no connector is a J-number.** A name is the bus or the kind, the direction, and a number
where there are several:

| part | meaning |
|---|---|
| `NB` · `MNB` · `MINI` · `MB` · `TIME` | a data body, by its bus: NodBus · MasterNodBus (the Mayak's trunk) · NodBus mini · ModBus · the receiver's RX/TX + PPS |
| `PWR` | a power body |
| `12V` | the two 12 V terminals — one node, two blocks in parallel, no direction |
| `FEED` | the power on the cable — 48 V, 300 V, an arm's isolated 12 V, the switched 24 V — on a Galvani power board only |
| `LINE CU` · `LINE FO` | the data on the cable, copper pairs or fibre — on a Galvani communication board only |
| `DATA` | a Galvani communication board's own ribbon socket |
| `OUT` | the board drives or feeds there — the master's side; **a Galvani `S` board sits in it** |
| `IN` | the board listens or is fed there — the unit's side; **a Galvani `U` board sits in it** |

**A connector's name says what it is, never what plugs into it**; the description beside it may
say what typically hangs there.

| board | connectors |
|---|---|
| Mayak | `MNB OUT 1`…`4` · `PWR HERMES` · `TIME BUS` · `12V` |
| Hermes | `PWR IN` · `MB OUT` · `UART` · `CAN` · `I2C` |
| Kronos | `TIME IN 1` · `TIME IN 2` · `TIME BUS` · `12V` |
| the card, Bifrost / Argus | `MNB/NB IN` · `PWR IN` · `NB/MINI OUT 1`…`4` · `PWR OUT 1`…`4` · `TIME BUS` · `12V` |
| Palatine | `NB IN` · `PWR IN` · `MB OUT 1`…`4` · `PWR OUT 1`…`4` · `PWR EXT` · `12V` |
| Sputnik | `NB IN` · `PWR IN` · `TIME OUT` · `12V` · `ANT` |
| Polaris | `TIME OUT` · `ANT` |
| Tesla / Pip | `NB IN` · `PWR IN` · `TIME OUT` · `ROD X` · `ROD Y` · `ROD Z` · `12V` |
| Marconi | `NB IN` · `PWR IN` · `LOOP` · `12V` |
| Quake · Quark-Photon | `NB IN` · `PWR IN` · `12V` |
| Quark-Neutron/Positron | `NB IN` · `PWR IN` · `HV` · `12V` |
| Gauss · Pascal | `MINI IN` · `PWR IN` · `12V` |
| Quark-Tubes | `MINI IN` · `PWR IN` · `K1`…`K4` · `RING` · `HV GM` · `HV He` · `12V` |
| a MOD — Babel, Pluvius | `MB IN`, the arm's four wires; its own sensor connectors by their sensor |
| a MOD potted in glass — Ceres, Sakura | none: the four-wire cable is soldered to the board and stripped into the arm board's terminals |
| `G-I-N-025` · `G-I-M-005` | `DATA` · `LINE CU` |
| `G-O-10-10` · `G-O-2-100` | `DATA` · `LINE FO` |
| `G-48-S` · `G-300-S` · `G-12-S` · `G-24-S` | `PWR` · `12V` · `FEED OUT` |
| `G-48-U` · `G-300-U-6` · `G-300-U-40` | `PWR` · `12V` · `FEED IN` |

**The chain reads the same everywhere**: a host's `… OUT` → a Galvani `S` board → `FEED OUT` /
`LINE` → the cable → `FEED IN` / `LINE` → a Galvani `U` board → the unit's `… IN`. In the box the
crossed cable runs from `… OUT` straight into `… IN`, no board between.

### `ID` — one scale

**A board says what it is before it is powered.** One resistor from `ID` to ground on the board,
read once at bring-up on an ADC pin of the host against 10 kΩ (1 %) to the host's analogue rail —
3,3 V or 1,8 V, the table is in ratios. Twenty windows of 0,05 with margins of ±0,025; the E96 grid
lands every ratio within 0,006 of nominal and 1 % parts move it by ±0,005 at most. It is the one
analogue read in the contract, and it is passive on purpose: a driven pin would need the far board
alive, and then a dead board and an empty socket read the same.

| reads | resistor, E96 | what is on the other end | channel B | ranging |
|---|---|---|---|---|
| **0,00** | a short | **a fault** — no board and no host takes this code | — | — |
| 0,05 | 523 Ω | **free, kept free** — margin beside the short | — | — |
| **0,10** | 1,10 kΩ | **the Mayak** — across an in-box cable | the wire | — |
| **0,15** | 1,78 kΩ | **a card**, Bifrost or Argus — one code for both; the card says which it is in the announce that follows | the wire | yes |
| **0,20** | 2,49 kΩ | **Palatine** — across an in-box cable | the wire | yes |
| **0,25** | 3,32 kΩ | **Sputnik** — across an in-box cable | the wire | yes |
| **0,30** | 4,32 kΩ | **`G-24-S`** — the switched 24 V | — | — |
| **0,35** | 5,36 kΩ | **`GNSS` — the interface, on BOTH ends of a time link**: every port that carries the NMEA time stream presents it, Kronos on `ID_RET` at its receiver sockets, and whatever hangs there on its own `ID` | PPS, inward | yes |
| **0,40** | 6,65 kΩ | **`G-I-N-025`** — 485, two pairs, full duplex | 2²² | yes |
| **0,45** | 8,25 kΩ | **`G-I-M-005`** — 485, one pair, half duplex | — | none — a polled arm is not ranged |
| **0,50** | 10,0 kΩ | **`G-O-10-10`** — glass, 10 Mb/s | 2²² | yes |
| **0,55** | 12,1 kΩ | **`G-O-2-100`** — glass, 2 Mb/s | 2¹⁹ | yes |
| **0,60** | 15,0 kΩ | **`G-48-S`** | — | — |
| **0,65** | 18,7 kΩ | **`G-300-S`** | — | — |
| **0,70** | 23,2 kΩ | **`G-48-U`** | — | — |
| **0,75** | 30,1 kΩ | **`G-300-U-6`** | — | — |
| **0,80** | 40,2 kΩ | **`G-300-U-40`** | — | — |
| **0,85** | 56,2 kΩ | **`G-12-S`** | — | — |
| **0,90** | 90,9 kΩ | **Hermes** — on the Mayak's power connector | — | — |
| 0,95 | 191 kΩ | **free, kept free** — margin beside the open | — | — |
| **1,00** | open | **nothing plugged in** | — | — |
| anything else | outside every window | **a fault**, reported as one | — | — |

**An `ID` names the interface, never the partner — and the host asks anyway.** Below 0,40 is a
host across an in-box cable: no feed to gate, no board to enable. 0,40 to 0,55 is a
communication board, and the value says the medium and the rung it can carry — a check that the
board seated can carry what the card is sending, not what picks the clock; a card hands all
four of its ports one rung by its role (`../bifrost/HARDWARE.md`). 0,60 and above, and 0,30, is a
power board: the unit end learns which cell it hangs on, the source end which cell it is
driving and therefore which `INA238` thresholds to programme — and it answers when the `INA238`
does not. Who is on the other side comes from the dialogue that follows, even where the code
narrows it to one thing: a resistor says something is plugged in, only an answer says something
is alive. One code serves every unit on a time link, including one somebody invents later.

**`ID_RET` is a host's own number.** Every host carries the resistor of its code from `ID_RET` to
ground — the Mayak's 1,10 kΩ like anyone else's, because 0,00 is the fault code — and the crossed
cable lands it on the other host's `ID`. A Galvani board carries a resistor on `ID` only;
identification off a board is one-way, outward, because a board has no processor to read with.
A host that only ever stands behind a Galvani board carries no number and never meets the cable.

### The telemetry — `INA238` and `ISO1642` on every power board

**One front on every power board, and it is mandatory: an `INA238` with a low-side shunt, and an `ISO1642` carrying
its I²C both ways and `ALERT` up across the barrier; `A_SEL` crosses it down onto `A0`.** The host
reads bus voltage and current over the connector and programs the thresholds from what `ID` said
was seated. **The measuring side runs on an island the board makes for itself from the host's
3,3 V on the power connector** — `SN6505B` + `750313734` + 2× `PMEG10020ELR`, the same cell as a
communication board's line-side supply and the same 5 kV winding, always on and never behind
`ENABLE`, so a dark port reads as 0 V and not as silence. Every board of the family therefore has
one supply, the host's 3,3 V, and makes its own isolated copy of it where it needs one.
**The shunt is a 1210 metal-element sense resistor on the ±40,96 mV range** — 50 mΩ on 48 V and
12 V, 200 mΩ on 300 V, 20 mΩ on 24 V — and **`VBUS` is a divider with a 100 kΩ bottom leg on 48 V
(90,9 kΩ top) and 300 V (3× 422 kΩ), the pin straight on 12 V and 24 V**, 10 nF on the pin
everywhere; conversion 4,12 ms × 128, 16 bits without noise, `ALERT` within 4 ms. **`G-300-S`
alone carries a second `INA238`, the leak watch**, reading the tube's centre through 8,84 MΩ and
7,84 MΩ of ordinary 1206 resistors — the source end of a 300 V run is the one place it can be
measured, and the one place it must be.

| board | the shunt sits | what it reports |
|---|---|---|
| **the source boards** — `G-48-S` · `G-300-S` · `G-24-S` · `G-12-S` | **on the OUTPUT, on the isolated side** — the run is on the far side of the transformer and what the part must see is the run drooping: an overloaded cell shortens its pulses and the output sags while the current sits on the limit | the run's voltage and current; **over** the programmed load is a fault, `ALERT` → the host drops `ENABLE`; **under** the minimum is a dry run or a cut cable |
| **the unit boards** — `G-48-U` · `G-300-U-6` · `G-300-U-40` | **on the INPUT** — the feed as it arrives | what arrives at the far end; the host at the unit end reads it, and the 48 V leak watch is the two ends compared |
| **`G-300-S`, the second `INA238`** | between the run and earth | **the leak watch, mandatory on 300 V**: an insulation break on a floating run draws no current and trips nothing until the second break, or a person, closes the circuit; the part reads the leakage current from the source end every minute. **On a sea return the run is not floating and the watch has nothing to read**; the run is watched by its current, as a 48 V one |

**A 48 V run is watched by subtraction** — what leaves the station and does not arrive has leaked;
no part is added. A 12 V or 24 V run is not watched: within 30 V DC, the limit SELV keeps even
immersed. Every threshold is a programmed `INA238` register, never a converter property; **there
is no fuse anywhere in the family** — a polyfuse re-closes into the same arc and reports nothing,
where `ALERT` and `ENABLE` clear the fault and say so.

### `ENABLE`, `LINE_EN` and the three states

**`ENABLE` is a source power board's pin and nothing else's.** The host that owns the port drives
it from one GPIO; the pull-down at the receiving end means an unplugged or dead host leaves the
port off. At the unit end the pull goes to the input: the board runs from the moment the feed
arrives, never cuts its own supply, and resets itself through its watchdogs. **`LINE_EN` on the
data connector switches the isolated line side of a copper board and is not populated on glass**;
on a host a port's `ENABLE` and `LINE_EN` are one GPIO, so the feed and the line side go off
together and there is no state in which a communication board runs into a dark feed. **Nothing
above the port drives `ENABLE`**: a remote host is switched off, reset or put to sleep by a command
over the bus, executed by the processor that owns the thing.

| state | how | what happens |
|---|---|---|
| **running** | nothing at all | the station does not save power, and that is the design: every rail was sized for its own board's load, so there is no policy, no duty cycling and no threshold anywhere |
| **off, behind a port** | `END` on the bus, then the clock off, then **`ENABLE`** down at the source power board (`../core/PROTOCOL.md` §7, §10) | the feed goes and the far end draws nothing at all; what is left at the source end is the cell's own quiescent. It is also the one reset that clears a part with no reset pin |
| **off, in the enclosure** | a deep sleep the Mayak commands | there is no `ENABLE` above an in-box board and none is wanted — the battery is a wire, every board taps it; off costs milliwatts and each board wakes on the link it already has |

**A port comes up OFF and is switched on after the run is wired, never before.** Every host's
`ENABLE` GPIO boots low. The family's converters are no-opto flybacks: they read the output through
the transformer on the primary side, which is what deletes the optocoupler and the fourth wire
across the barrier — and it means no loop holds an unloaded output down: below the cell's minimum
load the rail climbs until the transil conducts and sits at its knee. Nothing is destroyed, the
far end's parts are rated against that clamp, but the rail is no longer the voltage on the label
and a device plugged onto an already-running empty port meets the risen one. **So: wire the run,
establish what is on the far end, then enable the port.** In service a port is either running with
its full load or dark with its converter stopped; the empty enabled port is the one state where
the operator, and not the design, keeps the rail at its rating.

**No board switches its own processor's rail and no rail on any board has its `EN` on a pin**;
what a board switches off, it switches at its parts. **Nothing switches an optical module**: a
killed run takes the far end's module with it, and at the near end a laser that stays lit costs
less than the part that would switch it.

### The reversed channel — a property of the socket

**Channel A carries the data, always. Channel B is a one-way line whose direction the socket
sets** — the clock outward from a card, a PPS inward to Kronos, or anything else a remote build
wants to send one way. No Galvani board carries the setting and no processor pin reads or drives
it: **the socket ties `B_DIR` hard — to 3,3 V where the host drives channel B, to ground where it
listens** — and the board carries no pull. High at every `NB/MINI OUT` and every `TIME OUT`; ground at every `… IN`
and every `TIME IN`. A host knows which socket it owns by construction, and the state
where both ends drive the channel cannot be expressed.

- **Copper, `G-I-N-025`: no part at all.** The channel B `ISO1450` has `D` and `R` both on the
  `CLK/PPS` pin, and `DE` and `RE#` both on `B_DIR`. High: the driver is on and `R` is at high
  impedance, so the host's edge goes out. Low: the driver is off and `R` drives the pin to the host.
  Two outputs never meet on the pin.
- **Glass, `G-O-10-10` and `G-O-2-100`: two gates.** A module's `RD` cannot go to high impedance,
  so the pin is joined through a **`74AUP1G126`** from the pin to `TD` (`OE` active high) and a
  **`74AUP1G125`** from `RD` to the pin (`OE` active low), and **the same `B_DIR` net carries both
  `OE`s, the `TPS22917`'s `ON` on `VccT` and the `TPS22917L`'s on `VccR`** — four inputs, one level:
  high feeds and connects the transmitter, low the receiver. Each gate regenerates the edge and
  isolates the pin (`CI` 0,9 pF), and with `IOFF` an unfed section's pin loads nothing. 100 kΩ to
  ground on `TD` and on the `125`'s input, so neither floats while its gate is shut.
- **Neither medium turns a laser round.** On glass each end has a transmitter and a receiver on
  channel B — four fibres a link, two on BiDi — and the direction decides which laser talks.
- **The uses never mix.** A fitted pair is a bus link or a reversed-channel link, never both; what
  rides channel B is the firmware's business.

### The protection ladder

**One cell at every voltage, and only the grades move. Reading from the cable: gas tube →
series element → transil.** The series element sits *between* the tube and the clamp: it drops the
tube's let-through across an impedance so the transil holds only the remainder — 22 µH chokes on
a feed, 10 Ω on a data pair, one per leg, no coupled winding. **The END OF THE CABLE decides what
is fitted, not which board it is:**

| | gas tube | series | transil |
|---|---|---|---|
| **every board at a SOURCE end** | **yes** — three-electrode, to the common earthing point | yes | yes |
| **every board at a UNIT end** | **none** | yes | yes |
| **an optical board, either end** | — | — | — nothing electrical arrives on a fibre |
| **a MOD on a ModBus arm** | none | 2× 10 Ω | `SM712` on the pair, 2× `5.0SMDJ18A` on the 12 V |

**A unit end carries no tube because there is nothing for one to discharge into.** A twisted pair
leaves almost no loop, so the surge is common-mode, and common-mode at a floating end has no
path; the three-electrode part degenerates into two gaps in series the moment its centre floats.
What is left is the differential residue, and that is the transil's. The far end clamps no
common mode at all, on purpose: the run's own impedance limits it (~110 Ω, 600 µH and 24 Ω a
kilometre) and the barrier stops it. **A remote site is not earthed and is not going to be** — a
bad earth is worse than none; a source board there carries its tube's footprint and leaves the
strap unconnected, and branch spacing stops one strike taking all four branches
(`../daedalus/CONSTRUCTION.md`).

**The parts — one tube family, six transil codes, one series pair.** The tube is Bourns's
three-electrode `2036-xx-SM`: **`2036-07-SM`** on 48 V and every data pair, **`2036-30-SM`** on
300 V. **A three-electrode tube does not strike both gaps at once** — one goes, the other follows,
and for a few hundred nanoseconds the whole difference stands across the pair; that is what the
series element and the transil are for, and both are fitted at every position. The transils are
the `5.0SMDJ` family, two in parallel at every position that meets a cable: **`5.0SMDJ54A`** on 48 V (clamp ~87 V,
which every downstream rating follows) · **`5.0SMDJ350A`** on 300 V (~565 V) · **`5.0SMDJ18A`** on
the unregulated 12 V (`G-12-S`'s output, a MOD's input) · **one `5.0SMDJ12A`** on a unit board's
regulated 12 V output, which meets no cable · **`5.0SMDJ28A`** on 24 V · **one `5.0SMDJ14A`** across every 12 V input in the enclosure, behind its fuse (*The rails*). A data pair takes **2× 10 Ω and an `SM712`**, on every copper board
and on every MOD. **The 75 V tube quenches itself on a 48 V feed; the 300 V tube does not** — there
a fired tube is a short across the feed until the converter's current limit holds it and the
source end takes the feed away: the `INA238` sees the collapse, raises `ALERT`, the host drops
`ENABLE`. That sequence is mandatory, because a struck tube held in a continuous arc is a dead
tube.

**The board is the sacrificial part.** The line cable lands on the board, the ladder lives on the
board, and at a source end the tube dumps to the common earthing point through a mm²-class strap a
few centimetres long — at 8/20 µs kA edges every metre of wire (~1 µH) adds kilovolts of
`L·di/dt`, so the board mounts at the enclosure entry next to the earthing point. Lightning eats
the board; the host and the ribbon live, and the board is replaced from the spares box.

### The cable, the ground and the termination

**The data cable is UTP Cat 6, outdoors in a UV-resistant PE jacket — a stock item.** Category 6
to ISO/IEC 11801 or TIA-568, **solid bare copper, 23 AWG** — **never copper-clad aluminium (CCA)**,
which is sold under the same label at half the price, carries about half again the loop resistance
the reach tables are computed on, and corrodes at every termination. The jacket is polyethylene
with a UV stabiliser, black or marked for outdoor use; a PVC indoor jacket cracks in a few summers.
485 is a
bus: A to A, B to B, nothing ever crosses; the white-striped core of a pair is A, the solid core B.
**The feed travels in a 2-core cable of its own — 2× 1,5 mm² or 2× 2,5 mm², 300/500 V class, its
own gland**, solid = +, striped = return. That is what leaves all four pairs to the data and makes
every NodBus run full duplex; on land two cables go into every fed run. **A hybrid cable is the option** —
four pairs and two power cores in one jacket, the pairs still all four to data — and a builder takes
whichever the market offers; the base build is the two cables.

| pair | carries |
|---|---|
| orange | **data TX A/B** — the units' slots, up to eight `DE`-gated drivers |
| blue | **clock A/B** |
| green | **signal ground** — both cores; every line-side island ties its GND here |
| brown | **data RX A/B** — the master's channel, one driver, never turns |

**The ground is its own pair, both cores, and carries no feed current, ever.** TIA-485's −7…+12 V
common-mode window is defined against the transceiver's local ground, so an isolated island must
have that reference brought to it, and a reference that carries supply current is no reference.
**The ground bonds to the feed return at exactly ONE point — the source end.** A second bond
anywhere turns the ground into a parallel feed return; a unit-end island ties its line-side GND to
the green pair and to nothing else. On a ModBus arm a bought sensor bonds its data reference to its
supply minus internally — that is the far bond, so there the station island is the end that
follows. On glass the two fibres run full duplex by construction and the module is dark when idle.

**A ModBus arm is a different cable: four wires in one jacket — `A`, `B`, 12 V, GND** — because a
bought sensor comes with that cable already on it. At the station it is stripped and split into
two boards: the pair onto `G-I-M-005`, the +/− onto `G-12-S`, each with four positions, so an arm
is a star of up to four sensor cables at the boards and more than four chain at a sensor. The
sensors' own 0,35–0,5 mm² cables give tens of metres; **50 m is the cap the arm's unregulated 12 V
sets** (`../core/blocks/modbus.md`). Classic ModBus stays on copper inside an arm; anything that
has to stand far away is a NodBus or NodBus mini unit behind a spur, and a plot that would want a
longer arm is served by the host going out to it on a fed run.

**Termination.** Every 485 pair on every copper board carries **2× 10 Ω in series, always** — they
sit in front of the `SM712` and halve its current before the tube fires, and the receive input is
≥ 12 kΩ, so 20 Ω of series costs a quarter of the amplitude against a margin of several times the
receiver threshold. **The terminating resistor, 80,6 Ω across the pair behind the 10 Ω, is fitted
on the first and the last board of a segment and on no board between** — point-to-point that is
both ends, on a chain the board at the source end and the last unit's. 80,6 + 2 × 10 is the ~100 Ω
the UTP wants; not 120 Ω, which belongs to dedicated 485 cable this project does not use. **The
resistor is on the board and a JUMPER on a 2-pin 2,54 mm header puts it across the pair — one
header per pair**: three on `G-I-N-025`, one on `G-I-M-005`. The jumper is set at commissioning
with the cable, and a lost jumper fails to the harmless side — an unterminated short segment
usually still runs, where a resistor left on a middle board puts a third ~100 Ω across the line and
nothing in the field shows it on a meter. On a ModBus arm at 19 200 Bd, to its 50 m cap, the
jumper is left open at the source end and the far end is the bought sensor's own affair; only an
arm run faster than that closes it (`../core/blocks/modbus.md`). **No inline connector outside the
enclosure** — nothing on the wet side has contacts; the glands follow the cables, two where the
board chains, one where it ends. A potted sonde has no terminal, is always the last unit on its
segment, and carries its termination inside.

### Under water

**Under water the feed rides in the data cable's own jacket** — one cable to lay, not two, because in
the sea the laying is the cost. **300 V wherever the sea is the return**, because the
return's own volts — the electrodes' polarisation, the sea's telluric field — stay a small part of
it only at a few milliamperes; **on two conductors over a copper run 48 V serves as well**. Two
ways to carry it:

| | **two conductors** | **one potential, the return through the sea** |
|---|---|---|
| where | fresh or salt water | **salt water only** — fresh water conducts too little to be a return |
| the feed | two insulated cores | one insulated conductor — a core, a screen, a sheath, a stainless tube: whatever the cable carries insulated |
| the return | the second core | an electrode at each end: a titanium cathode at the pod, an anode ashore — MMO or high-silicon iron, never plain steel — in ground that stays wet |
| polarity | as on land | **the conductor negative**, so a breach of its insulation is cathodic and dissolves nothing |

**The data is copper for a near sonde and glass beyond.** Copper is the four pairs in the same
jacket, **Cat 5e or Cat 6 and nothing below**, on a mini segment to 1 km (*Reach*); glass is the 2 Mb/s module's fibres in the pressure
build, to 100 km. The ground pair and its one bond are as on land; with the sea as the return, the
pod's feed return is its electrode.

**The electrode stands off the pod**: hydrogen forms on the cathode, and the pod keeps hydrogen out
by construction. **Beside a magnetometer the current is a field** — `μ₀I/2πr`, 2 nT at 1 m for
10 mA — so the cable and the electrode stay at the cable end of the pod, the tube's length away
from the coils. **A breach reads as current**: on either method the seawater adds a path — on a sea return the
only sign, because that run does not float and `G-300-S`'s leak watch has nothing to read — and the source
board's `INA238` sees the draw rise past its programmed load; a smaller leak shows as the
difference between the source end and the sum of the unit boards on the run. **With the sea as the
return, the source boards of several runs may share one anode**: the conductor is the negative,
the shunt sits in it, and each board's `INA238` reads its own run's current alone.

### Reach

**Copper carries a run to 500 m; past that it is fibre.** The reach on copper is the clock
channel's: at 2²² Hz its fundamental lands at the receiver threshold at 500 m on Cat 6, which makes
**250 m the recommendation and 500 m the maximum** — `G-I-N-025` is named for the first. **A NodBus
mini segment reaches twice as far**: its clock is 2¹⁹ and its data 2²⁰, both with a fundamental of
~0,52 MHz, where the pair loses ~1,5 dB/100 m against ~4 at 4,19 MHz — the same budget lands at the
threshold at ~1,3 km, so **500 m the recommendation and 1 km the maximum** — for a lone unit: a
chained segment runs its data at 2²¹, ~1,05 MHz, and these figures do not cover it. **A ModBus
arm is 50 m**, capped by its 12 V and not by the 485. **On glass the recommended distance is half
the computed one**, and a mixed run is capped by its shortest branch.

**The far end is one load, and the builder adds it up**: a measuring unit is 2 to 3 W by what is
fitted, an optical communication board about 0,5 W, a bought device on the 12 V tap whatever it
draws, a host with four ModBus arms ~7 W with an ordinary set of sensors and up to ~21 W with the
arms on their limit. **The tables say what the unit board delivers at 12 V**, with the source
cell's ceiling, the copper's `I²R` and the unit board's efficiency already inside. The method: the
source cell supplies `P_s` at `V₀`, draws `I = P_s/V₀`, the copper takes `I²R`; where the cable is
long enough that a constant-power load cannot be fed at all, the criterion `V_end = 75 % V₀`
binds instead and `P = 0,1875·V₀²/R`; the run delivers the smaller of the two less the unit board's
loss. Loop resistance, both cores at 20 °C: **22,93 Ω/km on 1,5 mm², 13,76 Ω/km on 2,5 mm²**.

**48 V** — `G-48-S` at ~26 W into `G-48-U` at ~90 %:

| length | 1,5 mm² | 2,5 mm² |
|---|---|---|
| 100 m | **22,8 W** | 23,3 W |
| 250 m | **21,9 W** | 22,5 W |
| 500 m | **20,4 W** | **21,6 W** |
| 1 km | 17,0 W | 19,8 W |
| 2 km | 8,5 W | 14,1 W |
| 5 km | 3,4 W | 5,7 W |

**300 V** — `G-300-S` at ~47 W into a `G-300-U` at ~91,5 %:

| length | 1,5 mm² | 2,5 mm² |
|---|---|---|
| 1 km | **42,5 W** | 42,7 W |
| 5 km | 40,4 W | 41,5 W |
| 10 km | 37,9 W | 39,9 W |
| 20 km | 32,7 W | 36,9 W |
| 30 km | 22,4 W | 33,8 W |
| 50 km | 13,5 W | 22,4 W |
| **100 km — the ceiling** | **6,7 W** | **11,2 W** |

**The load follows the distance.** The 40 W a `G-300-U-40` declares is a near-field figure and the
table is what arrives: at 5 km on 2,5 mm² the run still carries it, at 20 km it is 37 W, at 50 km
22. When a run is extended, the far end's declared watts are re-checked against the new length.
On 48 V the copper 485 caps the run at 500 m long before the feed does, so the rows past it are for
an all-glass build. **One hop ends at 100 km, and the laser is what ends it**: `G-O-2-100`'s long
module is a 100 km catalogue part, and there is no longer one. The feed has margin past that point
and it is not used; the ranging never binds, a down port's timer counting 2 ms, about 200 km of
round trip. Past 100 km the answer is a remote Argus re-transforming.

**Where the watts go.** Two fractions hold at every voltage and cross-section: at the maximum
distance (`V_end` = 75 % `V₀`) the cable drops 25 % of `V₀` and burns 33 % of what the far end gets;
at the recommended distance, half of it, 10,4 % and 11,7 %. A 300 V run on 2,5 mm² at 10 km,
worked: battery → `G-300-S` 50,6 W in, 47,0 out; the copper, 138 Ω loop at 0,157 A, 3,4 W;
`G-300-U-40` → 12 V, 39,9 W out — 79 % battery to the far end's 12 V. The same watts at the same
distance on 48 V would need 39× the copper. When the distance is not enough, the answers in order
are a bigger cross-section, then 300 V, then a battery on site — never a bigger island.

**Which boards a run takes — three questions, in order:**

| question | answer |
|---|---|
| does it leave the enclosure? | **no** → the crossed cable, the 12 V off the wire · **yes** → a power board and a communication board at each end |
| how far, and on what? | **copper to 500 m, a mini segment to 1 km** → `G-I-N-025` · **past that** → glass: **a NodBus spur on `G-O-10-10`, to 10 km and no further** — its 2²² clock does not fit the 2 Mb/s module · **a mini segment on `G-O-2-100`**, a metre to 100 km |
| does 48 V carry the load over that distance? | **yes** → `G-48-S` + `G-48-U` · **no** → `G-300-S` + `G-300-U-40`, or `G-300-U-6` in a pod |

| the far end | communication | power |
|---|---|---|
| a measuring unit at ordinary distance | `G-I-N-025` | 48 V |
| the same past 500 m, to 10 km | `G-O-10-10` | 48 V; 300 V where 48 does not carry the load |
| a mini-NOD segment that outgrows copper | `G-O-2-100` | 48 V; past what 48 V carries, `G-300-U-6` |
| a ModBus arm — bought sensors, a MOD | `G-I-M-005` at the source end; the sensor brings its own 485 | the arm's 12 V: `G-12-S` at the station, a unit board's tap on a fed run |
| a load that is not a sensor (a pump) | none — it is a load, not a unit | `G-24-S` on the host's dedicated power socket, up to ~33 W |
| a remote Argus, its segments optical | `G-O-10-10` | 300 V — the loadout asks for it, not the distance |
| a clock run to Kronos — a remote Sputnik, Pip | a communication board with channel B at each end, inward at Kronos | none — the far end is fed by its own NodBus run |
| a pod under water, near — to 1 km | `G-I-N-025`, a unit end and so no tube — the pressure build — the pairs in the sea cable | 48 V on two conductors, `G-48-U`; 300 V, `G-300-U-6`, on a sea return — *Under water* |
| a pod under water, far | `G-O-2-100`, the pressure module | 300 V, `G-300-U-6` — *Under water* |

**Read the load at the far end, not the unit's name.** A remote host with a full plot of sensors
and a remote measuring unit are the same problem to this family.

### A remote Argus — the far end that is a source again

**A remote Argus with optical segments cannot pass power down them**, so every far unit is its own
site and the remote Argus's budget is its five near ends plus whatever its outgoing runs carry.
It re-transforms: `G-300-S` at the station feeds `G-300-U-40` at the remote Argus, whose 12 V
terminals are the site's wire — the Argus card's terminals, then four `G-300-S` for the outgoing
runs, tap after tap, 3,33 A on the wire and never on a ribbon; reach resets at every such site. Four optical ports alone
draw ~2,6 W (4 × 578 mW on 3,3 V ÷ 0,9), a tenth of `G-48-S`'s ~26 W, and the whole card ~3,2 W —
before its segments' units are counted — **a remote Argus is a 300 V run by its sum, not by a rule.**

The site's load by module class, the card and four ports, back to the battery:

| module | 12 V bus | `G-300-U-40` | `G-300-S` | from the battery |
|---|---|---|---|---|
| 100 mA | 3,2 W | 3,8 W | 5,1 W | 6,0 W |
| 150 mA | 4,8 W | 5,6 W | 7,5 W | 8,8 W |
| 300 mA | 9,3 W | 10,9 W | 14,6 W | 17,2 W |

The 100 mA row is `../bifrost/argus/HARDWARE.md`'s own count; the 150 and 300 mA rows scale it by
the module's draw and are estimates. The 300 mA row is the 52/84 Mb/s class, fitted nowhere; it is
what the two boards are rated to.
**Nothing at a remote site is earthed** — the four `G-300-S` are fitted as at a station with their
straps unconnected — and **branch spacing** stops one strike taking all four branches, not the
strike. Where a battery can be had on site it is still the cheaper answer.

## The boards

*Every board carries the connectors that are its own and the common part above; what follows is
what each one adds. In every figure the barrier is marked. The parts and their values are
`HARDWARE.md`; the full list is `BOM.md`.*

### `G-48-S` — power, source, 48 V

Makes the 48 V feed for one run from the 12 V it taps off the wire.

```
 12 V off the wire, two terminals ─▶ EMI choke ─▶ LT3748 · ISC165N15NM6 ─▶ 750310988 ─▶ V5N22 ─┐
                                   boundary mode, no burst    1:4,42               │ THE BARRIER
                                   R_SENSE 9,1 mΩ                                  ▼
                                                              2× 5.0SMDJ54A ─▶ chokes 22 µH ═════════════▶ the feed, 2-core
                                                                                                2036-07-SM ─▶ ⏚ common earthing point
 INA238 + ISO1642 on the output · the host side on the port's 3,3 V · ENABLE drives the LT3748's EN
```

- **in:** 12 V on its terminals; 3,3 V and `ENABLE` off the power connector · **out:** 48 V on a 2-core cable; `SDA · SCL · ALERT` back
- **rating:** **~26 W delivered** at the pack's working corner; 28,4 W in. The ceiling is the winding's 15 A of saturation: in boundary mode the inductance cancels and only the reflected voltage moves the watts per ampere, and this is the one 12 V → 48 V winding in the catalogue. 41 kHz at full load; no burst mode, so a minimum load of 0,65 W
- **parts:** `5.0SMDJ14A` across the 12 V input · `LT3748` · `ISC165N15NM6` · `750310988` · `V5N22-M3` · `US3M`, the blocking diode · `INA238` · `ISO1642` · the island `SN6505B` · `750313734` · 2× `PMEG10020ELR` · 2× `5.0SMDJ54A` · `2036-07-SM` · 2× 22 µH chokes · the EMI choke and the bulk
- **`ID`:** 15,0 kΩ, 0,60

### `G-300-S` — power, source, 300 V

Makes the 300 V feed for one run: the same cell as `G-48-S` with the 300 V winding, rectifier and
ladder, and the leak watch. A board of its own, because a different winding, rectifier and clamp
are a different drawing.

```
 12 V off the wire, two terminals ─▶ EMI choke ─▶ LT3748 · ISC165N15NM6 ─▶ 750310349 ─▶ US3M ─┐
                                   R_SENSE 5,5 mΩ             1:10                │ THE BARRIER
                                                                                  ▼
                                                              2× 5.0SMDJ350A ─▶ chokes 22 µH ═════════════▶ the feed, 2-core
                                                                                                 2036-30-SM ─▶ ⏚ common earthing point
 INA238 + ISO1642 on the output · a second INA238, the run against earth — the leak watch · ENABLE drives the LT3748's EN
```

- **in:** 12 V on its terminals — 4,2 A at 12 V, 4,6 A at the 11 V floor; 3,3 V and `ENABLE` off the power connector · **out:** 300 V on a 2-core cable; `SDA · SCL · ALERT` back
- **rating:** **~47 W delivered, one number**, 50,6 W in — the same board at the station and at a remote Argus. The shunt allows ~66 W and the `INA238` threshold holds the 47; 145 kHz at full load, no burst mode, so a minimum load of 0,63 W. The winding's 1000 V AC insulation is accepted for the concept; a production run orders a custom-insulated winding
- **parts:** `5.0SMDJ14A` across the 12 V input · `LT3748` · `ISC165N15NM6` · `750310349` · 2× `US3M`, the rectifier and the blocking diode · 2× `INA238` · `ISO1642` · the island `SN6505B` · `750313734` · 2× `PMEG10020ELR` · 2× `5.0SMDJ350A` · `2036-30-SM` · 2× 22 µH chokes · the EMI choke and the bulk
- **`ID`:** 18,7 kΩ, 0,65

### `G-48-U` — power, unit, 48 V

Takes 48 V off the cable and delivers **12 V** on its two terminals — an input and a tap, the same
node; a bought device takes it off the tap. Its second feed pair carries the feed on, behind this
board's ladder, to the next unit on a chained segment.

```
 48 V ══▶ 2× 5.0SMDJ54A ─▶ bulk ─▶ LT3748 · ISC165N15NM6 ─▶ 750311607 ─▶ V10P10 ─┐
  through   across the pair          R_SENSE 15 mΩ            2,5:1, 14 µH   100 V     │ THE BARRIER
  the gland no tube and no chokes — a unit end floats                                     ▼
                                                                                              12 V ─▶ 5.0SMDJ12A ─┬─▶ the two terminals ─▶ the unit
                                                                                                                   └─▶ the tap — a bought device, ≤ 0,5 A
 INA238 + ISO1642 on the input · the feed straight through on the second pair, so units chain
```

- **in:** 48 V through the gland · **out:** 12 V on the two terminals; `SDA · SCL · ALERT` on the connector; the host's 3,3 V feeds the isolator's host side
- **rating:** **~25 W** at 12 V; what arrives is capped by the source cell's ~26 W, not by this board, and the shunt allows ~50 W. 455 kHz at full load, clamped at 1,05 MHz below about 10 W; minimum load 0,24 W. Nothing to switch: the unit makes its own rails from the 12 V
- **a pressure board** — ceramic and solid parts only
- **parts:** `LT3748` · `ISC165N15NM6` · `750311607` · `V10P10-M3` · `INA238` · `ISO1642` · the island `SN6505B` · `750313734` · 2× `PMEG10020ELR` · 2× `5.0SMDJ54A` · `5.0SMDJ12A` on the 12 V output · the ceramic bulk
- **`ID`:** 23,2 kΩ, 0,70

### `G-300-U-6` · `G-300-U-40` — power, unit, 300 V

**Two boards, and what splits them is the enclosure, not the watts**: `G-300-U-6` is a pod's
board, where a 40 mm pipe is the whole volume and its winding is 8,6 mm tall against 23,5;
`G-300-U-40` is a 12 V bus of that size anywhere. Everything else is common — the same `LT8316`,
the same `FCD260N65S3`, the same 300 V ladder, the same connectors, the same telemetry. They
differ in the winding, in `R_SNS` and in the rectifier.

```
 300 V ══▶ 2× 5.0SMDJ350A ─▶ the ~2 µF bank ─────────────────────▶ LT8316 · FCD260N65S3 ─▶ [ the winding ] ─▶ [ the diode ] ─┐
   through   across the pair                                          quasi-resonant                                          │ THE BARRIER
   the gland no tube and no chokes — a unit end floats                                                                           ▼
                                                                                                                             12 V ─▶ 5.0SMDJ12A ─┬─▶ the two terminals ─▶ a unit
                                                                                                                                                  └─▶ the tap — a device, or a remote site's wire
 INA238 + ISO1642 on the input
```

| | **`G-300-U-6`** | **`G-300-U-40`** |
|---|---|---|
| the winding | **`11338-T195`** — CEEH178, 14:1:1,7, 1000 µH, 19 × 17,4 × 8,6 mm SMD, basic 3000 V AC | **`11328-T078`** — PQ2620, 8:1:1, 670 µH, 31 × 28,5 × 23,5 mm PIN, reinforced 3 kV |
| `R_SNS` | **130 mΩ** | **58 mΩ** |
| rated output | **6 W** at 12 V | **40 W** at 12 V — the source's ~47 W less this board's 3,7 W of loss, declared under the 11 V corner |
| the rectifier | **`V10P10-M3`**, 100 V / 10 A — 33 V of reverse stress, 0,5 A | **`V10P10-M3`**, 100 V / 10 A — 50 V of reverse stress, 3,7 A, two thirds of the board's loss |
| `f` at full load | 140 kHz, the part's clamp — discontinuous | 91 kHz |
| on the FET's drain | 472 V of its 650; **550 V on its RCD clamp** — `US3M`, 100 nF 1 kV, 2× 38,3 kΩ — fitted, for the winding's 34 µH of leakage | 402 V of its 650 |
| **the bulk bank** | **5× 1 µF 500 V X7R 2220** — ceramic, **a pressure board** | **4× 1 µF 630 V polypropylene film + 100 nF 1 kV X7R 1812** — **land only**; a pressure build takes `G-300-U-6`'s bank and recalculates |
| **`ID`** | **30,1 kΩ**, 0,75 | **40,2 kΩ**, 0,80 — the one place the two boards differ outside the power train, and they must, because a host may not ask a 6 W board for 40 W |

- **in:** 300 V through the gland · **out:** 12 V on the two terminals; `SDA · SCL · ALERT` on the connector
- **standing loss:** none worth the name — `LT8316` draws 12 µA on `V_IN` off and 75 µA on `BIAS` in burst, milliwatts at 300 V, and the island its 1,5 mA on the host's 3,3 V; it bursts down to 3,5 kHz, so the minimum load is ~1 % of the rating
- **parts, `G-300-U-6`:** `LT8316` · `FCD260N65S3` · `11338-T195` · `V10P10-M3` · the RCD clamp, `US3M` · `INA238` · `ISO1642` · the island `SN6505B` · `750313734` · 2× `PMEG10020ELR` · 2× `5.0SMDJ350A` · `5.0SMDJ12A` on the 12 V output · the ceramic bank
- **parts, `G-300-U-40`:** `LT8316` · `FCD260N65S3` · `11328-T078` · `V10P10-M3` · `INA238` · `ISO1642` · the island `SN6505B` · `750313734` · 2× `PMEG10020ELR` · 2× `5.0SMDJ350A` · `5.0SMDJ12A` on the 12 V output · the film bank

### `G-12-S` — the isolated 12 V, source

**The 12 V across a barrier and straight onto a short run — the supply a bought ModBus device a few
metres out plugs into**, on the same terminal block as its pair. No unit-end partner: what stands
at the far end is the device. It taps the 12 V wire on its own terminals, 0,4 A out and 0,5 A in,
and takes 3,3 V and `ENABLE` off the power connector like every source board. **Its scope is a
sensor's supply**: about 5 W at the working corner, unregulated, following the pack. A load that
wants watts is a feed at 48 V or 300 V, never this board at a lower one.

```
 12 V off the wire, two terminals ─▶ EMI choke ─▶ SN6507 push-pull ─▶ 750319691 ─▶ 2× PMEG10020ELR ─┐
                                   1,26 MHz, R_LIM 34,8 kΩ → ~0,7 A   N = 1,13          │ THE BARRIER
                                   ENABLE drives its EN                                 ▼
                                                                        2× 5.0SMDJ18A ─▶ chokes ─▶ 2036-07-SM ─▶ ⏚ common earthing point
                                                                                                    ══▶ ± ~12 V, unregulated, on eight poles
                                    INA238 + ISO1642 on the output — the host side on the port's 3,3 V
```

- **in:** 12 V off the wire on two terminals; `ENABLE` and 3,3 V off the power connector · **out:** ~12 V on eight poles, `DGPS2.5R-5.0` × 4 — four positions, 4× + and 4× −, beside `G-I-M-005`'s four positions of the pair
- **rating:** **0,4 A**, about 5 W. `R_LIM` 34,8 kΩ → ~0,7 A (0,5–0,9 A over the part's spread): 0,4 A out is ~0,45 A through the switches, and a 0,5 A limit would droop the rated load on a low part. **The overload is the `INA238`'s, not the OCP's** — the part's OCP shortens pulses rather than switching off and would hold an overload until thermal shutdown, so `ALERT` past 0,4 A takes `ENABLE` away; the OCP is left the start into a sensor's input capacitors and a short
- **parts:** `5.0SMDJ14A` across the 12 V input · `SN6507` · `750319691` · 2× `PMEG10020ELR` · `INA238` · `ISO1642` · the island `SN6505B` · `750313734` · 2× `PMEG10020ELR` · 2× `5.0SMDJ18A` · `2036-07-SM` · the chokes, the EMI choke and the bulk
- **`ID`:** 56,2 kΩ, 0,85

### `G-24-S` — power, source, 24 V, switched

**A switched 24 V across a barrier for a load that is not a sensor (a pump), on the host's dedicated power socket.** No unit-end partner: the load takes the 24 V straight, on a
2-core cable of its own, and nothing regulates after the barrier but the cell itself. It taps the
12 V wire on its own terminals and takes 3,3 V and `ENABLE` off the power connector — **and
`ENABLE` is the switch**: the host raises it for the duration of the load's run, and otherwise the
cell stands off with the `LT3748`'s `EN/UVLO` held low and the board draws nothing.

```
 12 V off the wire, two terminals ─▶ LT3748 · ISC165N15NM6 ─▶ 750311592 ─▶ V10P10-M3 ─┐
                                    R_SENSE 10 mΩ            1:1, 8 µH     100 V      │ THE BARRIER
                                    ENABLE drives its EN/UVLO                          ▼
                                                              2× 5.0SMDJ28A ─▶ chokes 22 µH ─▶ 2036-07-SM ─▶ ⏚ common earthing point
                                                                                                ══▶ 24 V, two poles — the load's 2-core cable
                                    INA238 + ISO1642 on the output — the host side on the port's 3,3 V
```

- **in:** 12 V off the wire on two terminals, ~3 A while the load runs (36 W in at the working corner); `ENABLE` and 3,3 V off the power connector · **out:** 24 V on one two-pole `DGPS2.5R-5.0`; `SDA · SCL · ALERT` back
- **rating:** **~33 W delivered** at the pack's working corner — `I_pk` 9,0 A at 10 mΩ, 112 kHz at full load. What the port declares is the load's, a programmed `INA238` threshold: **over** the limit is a stalled load, **under** the minimum a load running dry or a cut cable — two thresholds the host programs. The current limit is the cell's own `R_SENSE`; a stalled motor is delivered into at the limit and never fused
- **the ladder:** the source end's, like `G-12-S` — the three-electrode tube, the 22 µH chokes and 2× `5.0SMDJ28A` on the 24 V. What stands at the load's end is the load's own affair: a bare motor takes two leaded transils on its lugs, a controlled one a small board of its own
- **parts:** `5.0SMDJ14A` across the 12 V input · `LT3748` · `ISC165N15NM6` · `750311592` · `V10P10-M3` · `US3M`, the blocking diode · `INA238` · `ISO1642` · the island `SN6505B` · `750313734` · 2× `PMEG10020ELR` · 2× `5.0SMDJ28A` · `2036-07-SM` · 2× 22 µH chokes · the bulk
- **`ID`:** 4,32 kΩ, 0,30

### `G-I-N-025` — communication, 485 on copper, NodBus

Carries the NodBus over copper, **both ends of the run, one board**: data full duplex, the echo
receiver, and the clock or the PPS on channel B. The socket sets which end it stands at — `B_DIR`
and `LINE_EN` strapped or driven there — and a source end fits the tube and its strap.

```
 the host ── data connector ──▶ 3,3 V from the host ─┬─▶ SN6505B ─▶ 750313734 ─▶ PMEG10020ELR ─▶ isolated 3,3 V
              LINE_EN drives the SN6505B's EN │             420 kHz      1:1,1          THE BARRIER    │
                                              │                                                        ▼
                                              └── the ID resistor · B_DIR strap read       ISO1452 ── data, both directions
                                                                                           ISO1452 ── the echo-check receiver, driver tied off
                                                                                           ISO1450 ── channel B: the clock out, or PPS in
                                                                                                     D and R both on the CLK/PPS pin;
                                                                                                     DE and RE# on B_DIR
                                                                                                     │
                                                                                SM712 ─▶ 2× 10 Ω ══ 80,6 Ω behind a jumper ══▶ TX · clock · RX
                                                                                                 2036-07-SM ─▶ ⏚ common earthing point, at the source end only
 four two-pole DGPS2.5R-5.0, eight poles, one pole to a net · three termination jumpers, one per pair
```

- **in:** 3,3 V and logic from the host · **out:** three 485 pairs on the data cable
- **draw, on the host's 3,3 V:** ~75 mA at a source end, ~18 mA at a unit end, ~130 mA mid-burst; the line side alone 57 / 5 mA
- **at a source end:** the tube and its earth strap fitted; at a unit end left off — the same board
- **parts:** `SN6505B` · `750313734` · `PMEG10020ELR` · 2× `ISO1452` · `ISO1450` · 3× `SM712` · 6× 10 Ω · 3× 80,6 Ω with their headers and jumpers · `2036-07-SM` · `DGPS2.5R-5.0` × 4
- **`ID`:** 6,65 kΩ, 0,40

### `G-I-M-005` — communication, 485 on copper, ModBus

`G-I-N-025` stripped to a ModBus arm: **one transceiver, one pair, half duplex** — Modbus RTU
turns the line, and this is the one half-duplex board in the family — the same barrier and the
same isolated supply. **50 m** is the reach in the name: the arm's 12 V sets it, not the 485.

**No end letter, because the board has one end.** A half-duplex pair is symmetric, `DE` from the
host turns it frame by frame, there is no clock channel and `B_DIR` is unconnected. At the far end
of an arm there is no board at all: a bought sensor and a MOD bring their own 485.

```
 the host ── data connector ──▶ 3,3 V from the host ──▶ SN6505B ─▶ 750313734 ─▶ PMEG10020ELR ─▶ isolated 3,3 V ─▶ ISO1450 ── A/B, DE from the host
              LINE_EN drives the SN6505B's EN                           THE BARRIER                              │
                                                                                               SM712 ─▶ 2× 10 Ω ══ 80,6 Ω behind a jumper ══▶ the pair
                                                                                                            2036-07-SM ─▶ ⏚ common earthing point
```

- **in:** 3,3 V and logic from the host · **out:** the arm's pair on eight poles, `DGPS2.5R-5.0` × 4 — four positions, 4× `A` and 4× `B`
- **draw, on the host's 3,3 V:** ~9 mA listening, ~60 mA asking
- **parts:** `SN6505B` · `750313734` · `PMEG10020ELR` · `ISO1450` · `SM712` · 2× 10 Ω · 80,6 Ω with its header and jumper · `2036-07-SM` · `DGPS2.5R-5.0` × 4
- **`ID`:** 8,25 kΩ, 0,45

### `G-O-10-10` — communication, glass, 10 Mb/s

Carries the NodBus over fibre to 10 km with the 10 Mb/s module — the 2²² clock rides it.

```
 the host ── data connector ──▶ 3,3 V from the host ──────────────────────┬─▶ π filter ─▶ 1×9 seat: channel A, data, both sections
                                                                          └─▶ π filter ─▶ 1×9 seat: channel B, the clock — VccT at the sending end,
                                                                                                                              VccR at the receiving end
                                                                          74AUP1G126 pin → TD, 74AUP1G125 RD → pin; their OEs and the two
                                                                          load switches on B_DIR
 local bulk at each seat · the ID resistor · NO barrier, NO ladder — the fibre isolates by construction
```

- **in:** 3,3 V and logic from the host · **out:** four fibres
- **draw:** ~105 mA at a source end and ~55 mA at a unit end on average, 175 and 125 mA at most; every module this project fits reads 100 mA, the receiver ~25 and the laser ~75 of it
- **module:** `OPT10-31103STR` / `-PTR` — 10 Mb/s, 10 km, duplex only. DC-coupled, TTL, idle mapped to laser-dark; `DE` has no laser to gate. Reach 0–10 km, no attenuator at any length
- **parts:** the two modules · `74AUP1G126` · `74AUP1G125` · `TPS22917` on `VccT` and `TPS22917L` on `VccR` · 2× 100 kΩ · the π filters and the bulk — `B_DIR` a static select from the socket, not a switch
- **`ID`:** 10,0 kΩ, 0,50

### `G-O-2-100` — communication, glass, 2 Mb/s

The same board with the 2 Mb/s module in the seat — **one part, and it reaches from a metre of
fibre to 100 km**: its receiver takes −39 to 0 dBm. Channel B runs the lower rung, 2¹⁹, enough for
a mini-NOD, and turns round like `G-O-10-10`'s. A board of its own because the module is a different
drawing.

**The module has no minimum length but no margin at zero**: it sits exactly on its overload
ceiling with no fibre in front of it, so on a short run a fixed optical attenuator is fitted
(`HARDWARE.md`, *The minimum length*).

- **in / out / draw:** as `G-O-10-10`
- **module:** `OPT2-55A03STR` duplex, 1550 nm, or the BiDi pair `OTB2-35A03STR` / `OTB2-53A03STR`
- **parts:** as `G-O-10-10`, with this module
- **the pressure build** is the same part obtained from the maker in a pressure-tolerant form against the same specification; the catalogue modules are land parts
- **`ID`:** 12,1 kΩ, 0,55

## Converters and measuring boards — distance, not frequency

**No switching frequency this family can buy is outside every measuring band**, and the island
flybacks run in boundary mode, so their frequency moves with the load and cannot be parked. The
rule is mechanical: the converter as far from the front end as the enclosure allows — the near
field falls with the cube of the distance, doubling it is −18 dB; shielded inductors, a solid
ground plane, a small hot loop; `MODE` per board, forced PWM where a band listens and auto where
nothing does; and what leaves a board on a cable is filtered — on a source board the bulk and the
series chokes stand between the converter and the terminal, so the converter's own spectrum is
not a criterion. One converter per board where a band is measured: two free-running boundary
converters make a difference frequency that sweeps the band all day. The criterion is the
measuring board's own bench figure, and the `INA238` threshold guards the number afterwards.

## Files

| File | Contents |
|---|---|
| [`HARDWARE.md`](HARDWARE.md) | the component layer — every cell, every part and value, board by board; the record to draw from |
| [`BOM.md`](BOM.md) | the bill of materials, board by board; `HARDWARE.md` wins |
| [`EXAMPLE.md`](EXAMPLE.md) | an example build — one Bifrost in a box |
| [`WHY.md`](WHY.md) | the graveyard — rejected alternatives and superseded states, with the reason |

## Licence

Hardware: CERN-OHL-S v2 (`../LICENSE-HW`) · Software: MIT (`../LICENSE`) — Copyright (c) 2026 NIC — Native Intellect Community

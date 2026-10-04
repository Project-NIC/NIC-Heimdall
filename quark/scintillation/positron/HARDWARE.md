★ N.I.C. ★

# Positron — the block and the SiPM, and the board they land on

> **Design-stage concept — nothing built.** The figures are verified against the parts' sheets and
> the parts before the board is made. What Positron is and why the unit exists is [`README.md`](README.md). The block and its window
> are below; the isotope reach tables, the calibration chain and the deployment geometry are
> [`../photon/SCINTILLATION.md`](../photon/SCINTILLATION.md). The board, `Quark-Neutron/Positron`,
> is below too, and the H7A3 layer it shares with `Quark-Photon` is
> [`../photon/HARDWARE.md`](../photon/HARDWARE.md).

**Positron is one quantity — beta, both signs — and one build, on the low-voltage board.** It
carries no MCU of its own and no high voltage; on a station that runs tubes the beta is the bare K1
tube against the β-stopped K2, 0,5–1 m over the plate, and the plate is in every build
(`../../tubes/HEADS.md`).

| part | what it is | lands on |
|---|---|---|
| **low voltage** | 25 mm plastic block + SiPM on the face | **`Quark-Neutron/Positron`** |

---

## One SiPM channel on `Quark-Neutron/Positron`

**A 25 × 25 × 25 mm plastic block — `EJ-240`, the SLOW plastic** — PTFE-wrapped, with the **SiPM
directly on one face** and into **`THS4551`** alone — the anode on the amplifier's summing node,
which is the virtual ground a SiPM requires. **There is no wavelength-shifting fibre**: a fibre
exists to collect from a thin tile's edge, and a 25 mm face takes a 6 × 6 mm SiPM directly. The
block's own 430 nm therefore reaches the SiPM's PDE peak with no shift to green to pay for.

**`EJ-240` and not the fast `EJ-200`, and the reason is cell saturation** (`WHY.md`). 6 300
photons/MeV, **decay constant 285 ns**, rise 19,5 ns, −60…+60 °C with the light output flat from
−60 to +20 °C and 95 % of it at +60 °C. A fast plastic hands the SiPM all its light inside one
microcell recovery, so the cell count is a hard limit; 285 ns spreads it, the cells recycle, and
**Rh-106 — the hardest release beta at 3 541 keV — falls from 28,5 % of the cells to 3,1 %**.

**Thickness is what makes this a spectrometer instead of a counter.** Plastic is 1,023 g/cm³, so
25 mm is **2,56 g/cm²**, and the electron ranges decide everything:

| | range | in 25 mm |
|---|---|---|
| Sr-90, 0,546 MeV | 0,18 g/cm² — 1,8 mm | contained |
| Kr-85, 0,687 MeV | 0,28 g/cm² — 2,8 mm | contained |
| **Y-90, 2,28 MeV** — the hardest beta this unit is built for | **1,1 g/cm² — 10,7 mm** | **contained** |
| full containment holds to | ~2,5 g/cm² | **~5 MeV** |

**A relativistic electron is minimum-ionising, ~2 MeV per g/cm²**, so anything that does *not* stop
leaves the same deposit whatever its energy — which is what a thin tile does to every beta above
its containment limit. This block has no such limit inside the band that matters.

**The cosmic muon is the ruler, and 25 mm is what separates it.** A minimum-ionising particle
crossing 2,56 g/cm² leaves **~5,1 MeV**, cleanly a factor of two above Y-90's 2,28 MeV endpoint, so
the muon peak stands on its own and serves both as an energy marker and as a clean veto. On a fast plastic
that deposit sat near 41 % cell occupancy, past the ~30 % where the response begins to bend; on
`EJ-240` it is near **4,5 %**, and a fully contained **Y-90** beta near **2 %** (`WHY.md`,
`../photon/SCINTILLATION.md`), so the muon peak is a linearity reference here and not only a
position marker.

**What the thickness costs, and it is the only cost: gamma.** At 662 keV a 2,56 g/cm² block
interacts with **19,8 %** of the photons that cross it, and in low-Z plastic almost all of it is Compton, a smeared
continuum with no lines, lying right across the beta band. **It is subtracted and not vetoed**:
`Quark-Photon` looks down at the same 1 m × 1 m plate from the same height on the same clock, so
the gamma rate and its spectrum are measured beside this channel, second by second, and taken off
statistically. That is a handle a standalone beta counter does not have, and it is why the
thickness is affordable here and would not be elsewhere.

**The charge stage's tail is this board's own, set by `Rf · Cf` for the block and not inherited
from the gamma head** — this board has no PIN and no crystal, and `EJ-240` gives its light in
285 ns (`WHY.md`).

**The two ends are set by different things and are two independent knobs**: the **rise** by the
light collection plus whatever RC is added deliberately, the **tail** by `Rf · Cf`. What couples
them is the **ballistic deficit** — the tail must outlast the collection or the peak under-reads
the charge:

| `τ_tail / τ_rise` | ballistic deficit |
|---|---|
| **10** | under 1 % — the safe choice |
| **5** | 2–3 %, calibrated out |
| under 3 | amplitude stops meaning energy |

**`τ` everywhere below is a decay constant, not a duration.** At `1τ` an exponential is still at
37 % and has delivered 63 % of its charge; where a length is meant, this document says *to 95 %*
and gives the number. Both the amplifier's 600 ns and the scintillator's 285 ns are constants of
that kind.

| | value | |
|---|---|---|
| **`Cf`** | **1,5 nF** C0G, E12 | the ruler — one microcell a step of **2,44 LSB**; the range — `Q/Cf` puts **2,2 MeV** at full scale, and the slow light's peak is 0,51 of `Q/Cf`, so Rh-106's 3 541 keV lands at 83 % of it |
| **`Rf`** | **402 Ω**, E96 | `τ = Rf·Cf` = **603 ns** |
| **window, events** | **1 375 ns** | the optimum of `signal/√T` against the scintillator's own 285 ns |

**`Cf` is set by the ruler and `Rf` is the tail — two knobs, both set here.** The one-cell step
`Q/Cf` must stand well above the converter's 1,29 LSB of noise for the dark stream's cumulants to
be read (`../photon/SCINTILLATION.md`, *The calibration chain*); 1,5 nF puts it at 2,44 LSB and
the ruler at 2–3 % a second, 1 % on the running mean. The ruler has no window: it is a statistic of
the stream and resolves no cell. What 1,5 nF costs is the noise gain against the SiPM's 3,4 nF,
3,3 — the channel's floor 1,35 LSB — and the range above 2,2 MeV at `Q/Cf`, which `EJ-240`'s
285 ns light gives back by halving the peak; the noise on this channel is still the converter's and
not the amplifier's.

**`THRESH` is set for `EJ-240`'s slow light.** The light arrives over 285 ns, so the same charge makes a pulse whose **peak is 49 % lower** than a
fast plastic's; the area is untouched, convolution preserves it, but the trigger fires on amplitude
and a threshold set for a fast plastic would count from twice the energy.

**No pulse shape is read on this channel**: `EJ-240` carries no discriminating slow component and
the energy is the area of the digitised pulse, so the tail is set for the ballistic deficit alone
(`WHY.md`).

**Low Z is kept, and it is not only about gamma.** Electrons backscatter off a surface in
proportion to Z — about 5 % off plastic against 40–50 % off a high-Z scintillator — so a crystal
would throw away half the betas before measuring any of them. Plastic thick is right; a crystal of
any thickness is not (`WHY.md`).

**It shares its board with the `Neutron` channel, on one topology with its own feedback values.** The
photomultiplier's anode and the block's SiPM both put a charge into a summing node, and the quantity
is a scale factor in firmware; both channels are charge stages — the block's read for its area, the screens' for each rise's peak (*`Quark-Neutron/Positron`*, below). **One box carries both and answers as ONE NOD** —
`Positron`, type 11, with `Neutron`'s two counts, thermal and epithermal, in bytes 8–11
of the record (`../photon/BUS.md`): the screens and the peak sort give counts and no
energy, so four bytes are the whole channel. **Type 10 is reserved for `Neutron`.**

**No `LTC6268` and no guard ring on this board.** There is no PIN here, and the block's channel is
the same order as Helion's 1,25 million electrons, which swallows surface leakage whole
(*`Quark-Neutron/Positron`*, below).

**The head looks down at the 1 m × 1 m plastic deposition plate**, window down, 0,5–1 m above it. The window budget
(≤ 30 mg/cm²) and the isotope reach are `../photon/SCINTILLATION.md`'s.

## The entrance window — two builds, and the trade is service life, not physics

Beta range in a low-Z material is set by **areal density alone**, which is why a window is
specified in mg/cm² and never in µm. Aluminium is 2,70 g/cm³; a dried conformal coat is 1,0–1,2
g/cm³, and it is specified by its mass per area too.

| window | mg/cm² | betas cut below |
|---|---|---|
| aluminised Mylar | **0,8** | 22 keV |
| **50 µm aluminium** | **13,5** | 100 keV |
| 50 µm Al + coat 2 mg/cm² | **15,5** | 108 keV |
| 50 µm Al + coat 5 mg/cm² | **18,5** | 121 keV |
| 50 µm Al + coat 11 mg/cm² | **24,5** | 143 keV |

**Every build meets the ≤ 30 mg/cm² budget**, so the choice is service life. The Mylar reaches down
to 22 keV and is a **consumable**; the coated aluminium loses the band below ~150 keV and is
likely permanent. Nothing this instrument measures lives down there — Cs-137's 514 keV beta ranges
169 mg/cm², K-40's 1,31 MeV 576, Bi-214's 3,27 MeV 1614, all of them straight through. Only C-14
falls off (156 keV endpoint, 28 mg/cm², and its continuous spectrum averages a third of that), and
tritium was never in reach. **For an unattended outdoor head, take the robust window.**

50 µm settles light-tightness in the same part: aluminium foil below ~10 µm has pinholes and 50 µm
does not, so no second layer is needed.

### The coat — a class and a set of properties, not a product

**No product is named, because the station is built where it stands** and a coat bought in one
country is not sold in the next. The coat is any conformal coating of the classes below, in the
international class letters of IPC-CC-830 and IEC 61086, that meets every line of the table; the
builder takes what the local market carries and checks it against the table.

| class | | |
|---|---|---|
| **SR**, silicone | **the first choice** | the widest temperature range, elastic far below −40 °C, UV-stable |
| **UR**, polyurethane — **aliphatic only** | accepted | tough and flexible; an aromatic polyurethane yellows and chalks in sunlight and is not accepted |
| **AR**, acrylic | accepted | UV-stable and easy to strip and redo; harder when cold, so the bend test below decides it |
| ER, epoxy | not accepted | brittle on a 50 µm foil that flexes with every temperature swing, and chalks in UV |
| XY, parylene | not accepted | vapour-deposited in a vacuum chamber, not a builder's process |

| property | requirement | how the builder checks it |
|---|---|---|
| mass per area | **2–11 mg/cm²** dried — 2 closes the foil's pinholes and stops Po-212, 11 keeps the window at 24,5 mg/cm² | a 10 × 10 cm piece of the same foil weighed before and after coating on a 0,01 g scale: 1 mg/cm² is 100 mg |
| temperature | rated over a range that contains **−40 … +80 °C** | the product sheet |
| sunlight | rated for outdoor exposure or UV-stable | the product sheet |
| flexibility | no crack when the coated foil is bent round a **3 mm mandrel at −40 °C** | ISO 1519, on the weighed piece after a night in a freezer at that temperature |
| adhesion | **class 0 or 1** in a cross-cut test on the foil | ISO 2409 |
| cure | **nothing acid released while curing** — an acetoxy silicone gives off acetic acid, which corrodes aluminium; neutral or alkoxy cure, or a solvent-borne coat | the product sheet; a vinegar smell while it cures rules it out |

**Condensation is kept out by bonding, not by the coat's chemistry.** Water vapour passes through
every polymer and leaves as freely as it came; what traps it is a patch where the coat does not
hold to the foil. So the foil is degreased and dry when it is coated, the coat goes on in two thin
crossed passes rather than one thick one, and it runs **continuously over the foil's edge onto the
frame**, so there is no open edge for water to creep under.

*(Ranges from the CSDA fit `R = 0.412·E^(1.265−0.0954·lnE)` g/cm², valid 0,01–3 MeV — computed,
not quoted.)*

## The beta head is a block, and nothing in it is glued

**A monolithic 25 × 25 × 25 mm block, PTFE-wrapped, the SiPM on one face** — the wrap white,
unsintered PTFE thread-seal tape with no adhesive, at least 0,5 mm in total, the SiPM's face bare
and the light-tight layer over the wrap (`../photon/SCINTILLATION.md`). Nothing is glued, so
nothing delaminates over −40/+60 °C, which is a real failure mode for a scintillator bonded to a
window. 2,56 g/cm² contains every beta the unit is built for — Y-90 ranges 1,1 g/cm² — and the
thickness is the point (`WHY.md`, *The 25 × 25 × 2 mm tile*).

---

## The heads are on the board — its underside, and no head connector

**One PCB, horizontal, the heads under it, windows down** — the same construction as `Quark-Photon`
(`../photon/HARDWARE.md`, *The heads are on the board*). The block's SiPM is on the **bottom
layer** and the PTFE-wrapped block stands under it, its aluminium window toward the plate; the
block is held by the enclosure's floor bracket, the board on standoffs off the same bracket, so
the sensor bond carries no load. Its NTC is the 0603 beside the SiPM.

**The photomultiplier's socket is on the bottom layer too** — `FE2019` for the `XP3462`, the
`E678-14A` for the `R1307-01` — with **the whole base around it: the divider chain and the dynode
decoupling** (`../neutron/HARDWARE.md`, *The base*), in an **HV corner of the board**: milled slots
between the −HV nodes and everything else, the corner conformal-coated, no copper pour under it.
The tube hangs from the socket photocathode down, the stack against the plate, **inside its
mu-metal shield, and the shield is clamped to the enclosure** — 200 g of tube and the shield never
hang on the pins, and the 3 cm lead shell around the head stands on the floor bracket
(`../neutron/HARDWARE.md`, *The lead shield*); the board's standoffs come off the same bracket so the pins meet the socket.
**The kV source stays its own board** (`../../tubes/helion/HARDWARE.md` §2): two wires, −HV and its
return, from its terminal to a terminal at the divider. Nothing else joins the boards, and the one
cable out of the unit is the Galvani data body's.

## `Quark-Neutron/Positron` — one channel Neutron, one channel Positron

**One front end, drawn once and copied — the topology; the feedback values are the channel's.**
The photomultiplier's anode and the block's SiPM put the same kind of charge into the same summing
node, a `THS4551` with `Rf` ∥ `Cf` in both arms, so the quantity is a scale factor in firmware. Both channels are charge stages; what differs is the time constant, the
scale and the reading — the block's **area**, the screens' **peak**: the neutron channel follows the photomultiplier's train with **`Rf` 10 kΩ ∥ `Cf` 10 pF, τ ≈ 100 ns**,
every rise an event read by its peak, the tube's gain set by its HV (`../neutron/HARDWARE.md`, *The
readout*); the beta block's channel is `Cf` 1,5 nF ∥ `Rf` 402 Ω, τ 603 ns. Two channels, one
topology, one BOM line for the amplifier; a build that later wants them on two boards moves a
copy and changes nothing else.

| | |
|---|---|
| processor | **STM32H7A3IIT6**, same part, same converter |
| converter | **`AD9251-80`** on the PSSI, interleaved — **44,040192 MSPS** a channel, two channels |
| **channel 1 — Neutron** | **`THS4551`** — the photomultiplier's anode, the `⁶LiF/ZnS(Ag)` screen read by the tube on Helion's kV source (`../neutron/HARDWARE.md`) — the two-screen stack, thermal and epithermal; charge stage, `Rf` 10 kΩ ∥ `Cf` 10 pF, τ ≈ 100 ns — a threshold in photoelectrons takes the gamma out and two height windows tell epithermal and thermal apart |
| **channel 2 — Positron** | **`THS4551`** — the 25 mm plastic block, SiPM straight on the face, no fibre; charge stage, `Cf` 1,5 nF ∥ `Rf` 402 Ω, τ 603 ns — one cell 2,44 LSB |
| **no `LTC6268`** | there is no PIN on this board. **The part is absent, not unpopulated** |
| **no guard ring** | nothing here needs one: the photomultiplier's anode pulse and the block's are both of the order of a He³ capture's 1,25 million electrons, which swallow surface leakage whole |
| at the converter's pins | the same RC network as `Quark-Photon`, unchanged |

**One box, ONE NOD.** A fitted board answers as **`Positron`, type 11**, and `Neutron`'s two counts,
thermal and epithermal, are bytes 8–11 of that record, ahead of its ten bands — the two-screen stack
and a threshold and two height windows give counts and no energy, so four bytes carry the channel
and an address of its own would have held 28 empty ones (`../photon/BUS.md`). **NodBus type 10 is
reserved for `Neutron`.** **Channel 1 is `Neutron`'s position**, drawn and kept: a build that
reworks the radiation side later plugs into it and does not redraw the board.

---

## The processor — `Quark-Neutron/Positron`: pins, timers, clock tree

*The record to draw the H7A3 from and to set CubeMX by. Package LQFP176, `STM32H7A3IIT6` — the LDO-supply part, no SMPS pins; pin
numbers and alternate functions from DS13195 Rev 8, the LQFP176 column. Every pin below is either
assigned or listed free — nothing is left to guess.*

### Peripherals

| peripheral | instance | pins | job | mode |
|---|---|---|---|---|
| the NodBus link | **USART1** + **TIM4** | PB6 `TXD` · PB7 `RXD` | the spur to its card, 8N1 at the rung `BUSCFG` hands down — 2²⁰ alone, 2²¹ chained, through the fixed-direction translators | the pins switch USART → TIM4 for the ranging instant |
| the frame anchor | **TIM2** (32-bit) | PB3 ← `RXD` on CH2 | the card's frame start on the grid | never reset, read as differences |
| echo receiver | **USART3**, RX only | PB11 `RXD_ECHO` | hears the board's own frames — Marconi's pin, with `PGOOD` and the LED on PH2 and PH3 as there | |
| the converter's data | **PSSI**, 16-bit receive, 14 lines used | `D0`–`D13` on PH9–PH12 · PH14 · PH15 · PI0–PI4 · PI6 · PI7 · PF11, the encode on PA6 | the AD9251's interleaved output — A and B alternating on one 14-bit port at twice the encode rate — into AXI SRAM through DMA | 88,08 M words/s, 44,04 MSPS a channel |
| the converter's clock | `MCO2` on PC9, PLL2's `P` output, through a `74AUC2G34` to the converter's `CLK+` and to `PSSI_PDCK` PA6 | PC9 · PA6 | **88,080384 MHz on both** — the word rate; the converter's own divider ÷2 makes the 44,040192 MSPS encode, the grid; `DCOA`/`DCOB` unconnected (`../photon/HARDWARE.md`, *The converter's port*) | `MCO2` at prescaler 1 — a timer cannot make it: 2²⁸ / (21 × 2²²) = 64/21, no integer divider |
| the converter's registers | **SPI3**, half-duplex + `CSB` PD2 | PC10 `SCLK` · PC12 `SDIO` | the AD9251's three-wire port, written at boot | ≤ 10 MHz; `PDWN` PG2, `SYNC` PG3, `ORA`/`ORB` PG4/PG5, `OEB` tied low, `DCOA`/`DCOB` unconnected |
| SiPM bias | **DAC1** ×2 · GPIO ×2 · **ADC1** ×2 | PA4/PA5 `BIAS_SET` · PE5/PE6 `BIAS_EN` · PC3/PC4 `IBIAS_MON` | one `LT3571` fitted, Positron's; channel 1's position is drawn and unpopulated — the photomultiplier brings its own kV | the servo's setpoint is `N`, the one-cell step, in raw LSB |
| the amplifiers | GPIO | PG7 · PG8 | the two `THS4551`s' `PD` | |
| thermometers | **ADC1** | `NTC_S` PA0 · `NTC_P` PA1 · `NTC_EN` PF6 | **two NTCs of one kind, never in the data** — the SiPM's rides the minute's `REPORT` frame, the photomultiplier's the `HEALTH` frame: the 0603 `NCP18XH103F03RB` beside the **block's SiPM**, the leaded `NXFT15XH103FA2B` at the **photomultiplier's base**, on the head, its two wires in the anode's cable — no correction consumes them today; they are there because the parts carry no sensor of their own and a bias law written on temperature is then firmware and not a board (`../photon/SCINTILLATION.md`). the station's divider: 10 kΩ from `NTC_EN` to the pin, the NTC from the pin to `VSSA`, **3 kΩ + 100 nF at the ADC pin**, read against `VREF+` so the rail drops out (`../../../tesla/HARDWARE.md` §7) | `NTC_EN` high for the conversion only; no bus, no edges, nothing to blank |
| ID reads | **ADC1** | PC0 · PC1 | one resistor per body, against 10 kΩ 1 % to `VREF+`, the 1,8 V | read once at bring-up |
| PSRAM | **OCTOSPI1**, port 2 | PF0–PF5 · PG0 · PG1 · PG10–PG12 · PF12 | the `APS25608N`, 32 MB octal DDR (*The PSRAM*, below) | 2²⁶, DQS on PF12; **the bus is silent while the front measures** — unloaded on this board |
| power body | **I2C1** through the `PCA9306` | PB8 · PB9 | the unit power board's `INA238`, alone on its controller — the bus stops at the connector and never reaches the front end, so this read is not blanked | 100 kHz; `ALERT` on PC8 |
| clock in | **HSE bypass** | PH0 | the data body's `CLK`, 2²², through the translator | PH1 unconnected |
| debug | **SWD** | PA13 · PA14 | | no JTAG, no trace |

No regulator is enabled from a pin: every rail runs whenever the 12 V is there, and the parts
that sleep sleep by their own pins — the converter's `PDWN`, the amplifiers' `PD`.

### Pins

| pin | port | signal on the board | AF / function | dir | note |
|---|---|---|---|---|---|
| 1 | PE2 | `DE` | GPIO | out | through the translator — the data body's driver enable |
| 2 | PE3 | — | | | free |
| 3 | PE4 | `SD` | GPIO | in | through the translator — the optical no-light; 100 kΩ to ground on the 3,3 V side |
| 4 | PE5 | `BIAS_EN1` | GPIO | out | |
| 5 | PE6 | `BIAS_EN2` | GPIO | out | |
| 6 | VBAT | — | | | tied to `VDD`; no RTC anywhere |
| 7 | PI8 | — | | | free |
| 8 | PC13 | — | | | free (tamper/RTC pin, unused) |
| 9 | PC14 | — | | | free — no 32 kHz crystal |
| 10 | PC15 | — | | | free |
| 11 | PI9 | — | | | free |
| 12 | PI10 | — | | | free |
| 13 | PI11 | — | | | free |
| 14 | VSS | | | | |
| 15 | VDD | 1,8 V | | | |
| 16 | PF0 | `PS_IO0` | `OCTOSPIM_P2_IO0` | i/o | the `APS25608N`, octal DDR — OCTOSPI1 on port 2 |
| 17 | PF1 | `PS_IO1` | `OCTOSPIM_P2_IO1` | i/o | |
| 18 | PF2 | `PS_IO2` | `OCTOSPIM_P2_IO2` | i/o | |
| 19 | PF3 | `PS_IO3` | `OCTOSPIM_P2_IO3` | i/o | |
| 20 | PF4 | `PS_CLK` | `OCTOSPIM_P2_CLK` | out | 2²⁶ — the pad table's 120 MHz octal-DDR ceiling; 33 Ω at the pad |
| 21 | PF5 | `PS_NCLK` | `OCTOSPIM_P2_NCLK` | out | unused — single-ended clock |
| 22 | VSS | | | | |
| 23 | VDD | 1,8 V | | | |
| 24 | PF6 | `NTC_EN` | GPIO | out | the two NTC dividers' top, high for the conversion only — PF3 is the PSRAM's `IO3`, as on Marconi |
| 25 | PF7 | — | | | free |
| 26 | PF8 | — | | | free |
| 27 | PF9 | — | | | free |
| 28 | PF10 | — | | | free |
| 29 | PH0 | `CLK_IN` | `OSC_IN`, bypass | in | the data body's `CLK`, 2²², through the translator — `V_IH` ≥ 1,26 V at `VDD` 1,8 V |
| 30 | PH1 | — | | | unconnected in bypass mode |
| 31 | NRST | reset | | | 100 nF, no pull beyond the internal |
| 32 | PC0 | `ID_D` | `ADC12_INP10` | in | the data body's `ID` — 10 kΩ 1 % to `VREF+`, the 1,8 V, so the reading is a ratio |
| 33 | PC1 | `ID_P` | `ADC12_INP11` | in | the power body's `ID`, the same way |
| 34 | PC2 | — | | | free (ADC) |
| 35 | PC3 | `IBIAS_MON1` | `ADC12_INP13` | in | channel 1's bias current — telemetry. The pin is `PC3_C`: `INP13` through the `SYSCFG` analog switch, or `ADC2_INP1` direct |
| 36 | VDD | 1,8 V | | | |
| 37 | VSSA | | | | |
| 38 | VREF+ | the `VDDA` node — the ADC reference, strapped to `VDDA` | | | |
| 39 | VDDA | 1,8 V through the shielded 2,2 µH `SWPA252012S2R2MT`, 10 µF + 100 nF at the pins, `VREF+` on the same node — the rail is a buck's own, so the filter is the buck-noise inductor of `../../../core/POWER.md`, not a ferrite | | | |
| 40 | PA0 | `NTC_S` | ADC | in | the block's SiPM NTC, on its divider, 3 kΩ + 100 nF at the pin |
| 41 | PA1 | `NTC_P` | ADC | in | the photomultiplier's base NTC, on its divider, RC at the pin |
| 42 | PA2 | — | | | free (ADC) |
| 43 | PH2 | `PGOOD` | GPIO | in | the 1,8 V `TPS629206`'s `PG` |
| 44 | PH3 | `LED` | GPIO | out | the status LED, fitted — 1 kΩ from the 3,3 V, one 50 ms blink a minute while healthy, two on a fault (`../../../core/HARDWARE.md`) |
| 45 | PH4 | — | | | free |
| 46 | PH5 | — | | | free |
| 47 | PA3 | — | | | free (ADC) |
| 48 | VSS | | | | |
| 49 | VDD | 1,8 V | | | |
| 50 | PA4 | `BIAS_SET1` | `DAC1_OUT1` | out | channel 1's bias converter setpoint — the position kept, unpopulated with the photomultiplier head |
| 51 | PA5 | `BIAS_SET2` | `DAC1_OUT2` | out | channel 2's — Positron's SiPM |
| 52 | PA6 | `PDCK` | `PSSI_PDCK` | in | **88,080384 MHz** — the read clock, the second gate of the `74AUC2G34` off `MCO2`; a short trace |
| 53 | PA7 | — | | | free (ADC) |
| 54 | PC4 | `IBIAS_MON2` | `ADC12_INP4` | in | channel 2's |
| 55 | PC5 | — | | | free (ADC) |
| 56 | PB0 | — | | | free (ADC) |
| 57 | PB1 | — | | | free (ADC) |
| 58 | PB2 | — | | | free |
| 59 | PF11 | `D12` | `PSSI_D12` | in |  |
| 60 | PF12 | `PS_DQS` | `OCTOSPIM_P2_DQS` | i/o | |
| 61 | VSS | | | | |
| 62 | VDD | 1,8 V | | | |
| 63 | PF13 | — | | | free (ADC) |
| 64 | PF14 | — | | | free (ADC) |
| 65 | PF15 | — | | | free |
| 66 | PG0 | `PS_IO4` | `OCTOSPIM_P2_IO4` | i/o | |
| 67 | PG1 | `PS_IO5` | `OCTOSPIM_P2_IO5` | i/o | |
| 68 | PE7 | — | | | free |
| 69 | PE8 | — | | | free |
| 70 | PE9 | — | | | free |
| 71 | VSS | | | | |
| 72 | VDD | 1,8 V | | | |
| 73 | PE10 | — | | | free |
| 74 | PE11 | — | | | free |
| 75 | PE12 | — | | | free |
| 76 | PE13 | — | | | free |
| 77 | PE14 | — | | | free |
| 78 | PE15 | — | | | free |
| 79 | PB10 | — | | | free |
| 80 | PB11 | `RXD_ECHO` | `USART3_RX` | in | through the translator — the echo check; 100 kΩ to ground on the 3,3 V side |
| 81 | VCAP | 2× 1 µF 50 V 0805 + 2× 100 nF 50 V 0603 — 2,2 µF, the sheet's 2,2 µF ±20 %, the internal LDO | | | |
| 82 | VDD | 1,8 V | | | |
| 83 | PH6 | — | | | free |
| 84 | PH7 | — | | | free |
| 85 | PH8 | — | | | free |
| 86 | PH9 | `D0` | `PSSI_D0` | in | the converter's data — the interleaved stream, both channels on one port |
| 87 | PH10 | `D1` | `PSSI_D1` | in |  |
| 88 | PH11 | `D2` | `PSSI_D2` | in |  |
| 89 | PH12 | `D3` | `PSSI_D3` | in |  |
| 90 | VSS | | | | |
| 91 | VDD | 1,8 V | | | |
| 92 | PB12 | — | | | free |
| 93 | PB13 | — | | | free |
| 94 | PB14 | — | | | free |
| 95 | PB15 | — | | | free |
| 96 | PD8 | — | | | free |
| 97 | PD9 | — | | | free |
| 98 | PD10 | — | | | free |
| 99 | PD11 | — | | | free |
| 100 | PD12 | — | | | free |
| 101 | PD13 | — | | | free |
| 102 | VSS | | | | |
| 103 | VDD | 1,8 V | | | |
| 104 | PD14 | — | | | free |
| 105 | PD15 | — | | | free |
| 106 | PG2 | `PDWN` | GPIO | out | the AD9251's `PDWN` — the converter asleep |
| 107 | PG3 | `SYNC` | GPIO | out | the AD9251's `SYNC` — resets the ÷2 divider to a defined state; pulsed once at start |
| 108 | PG4 | `ORA` | GPIO | in | channel A overrange — polled per block, not an interrupt |
| 109 | PG5 | `ORB` | GPIO | in | channel B overrange |
| 110 | PG6 | — | | | free |
| 111 | PG7 | `PD_AMP1` | GPIO | out | channel 1's `THS4551` `PD` |
| 112 | PG8 | `PD_AMP2` | GPIO | out | channel 2's |
| 113 | VSS | | | | |
| 114 | VDD33USB | tied to `VDD` | | | USB unused |
| 115 | PC6 | — | | | free |
| 116 | PC7 | — | | | free |
| 117 | PC8 | `ALERT` | GPIO, EXTI | in | through the translator — the power body's `INA238`; 100 kΩ to ground on the 3,3 V side, high = alarm |
| 118 | PC9 | `ENC` | `MCO2` — PLL2 `P` | out | **88,080384 MHz** — through a `74AUC2G34` tight against the converter, one gate to its `CLK+` (÷2 inside → the 44,04 encode, the grid), one to `PDCK` |
| 119 | PA8 | — | | | free |
| 120 | PA9 | — | | | free |
| 121 | PA10 | — | | | free |
| 122 | PA11 | — | | | free (USB, unused) |
| 123 | PA12 | — | | | free (USB, unused) |
| 124 | PA13 | `SWDIO` | SWD | i/o | |
| 125 | VCAP | 2× 1 µF 50 V 0805 + 2× 100 nF 50 V 0603 — 2,2 µF, the sheet's 2,2 µF ±20 %, the internal LDO | | | |
| 126 | VSS | | | | |
| 127 | VDD | 1,8 V | | | |
| 128 | PH13 | — | | | free |
| 129 | PH14 | `D4` | `PSSI_D4` | in |  |
| 130 | PH15 | `D11` | `PSSI_D11` | in |  |
| 131 | PI0 | `D13` | `PSSI_D13` | in | the top bit |
| 132 | PI1 | `D8` | `PSSI_D8` | in |  |
| 133 | PI2 | `D9` | `PSSI_D9` | in |  |
| 134 | PI3 | `D10` | `PSSI_D10` | in |  |
| 135 | VSS | | | | |
| 136 | VDD | 1,8 V | | | |
| 137 | PA14 | `SWCLK` | SWD | in | |
| 138 | PA15 | — | | | free |
| 139 | PC10 | `SCLK_C` | `SPI3_SCK` | out | the AD9251's `SCLK` — the configuration port |
| 140 | PC11 | — | | | free |
| 141 | PC12 | `SDIO_C` | `SPI3_MOSI`, half-duplex | i/o | the AD9251's `SDIO`, bidirectional |
| 142 | PD0 | — | | | free |
| 143 | PD1 | — | | | free |
| 144 | PD2 | `CSB_C` | GPIO | out | the AD9251's `CSB` |
| 145 | PD3 | — | | | free |
| 146 | PD4 | — | | | free |
| 147 | PD5 | — | | | free |
| 148 | VSS | | | | |
| 149 | VDDMMC | 1,8 V | | | |
| 150 | PD6 | — | | | free |
| 151 | PD7 | — | | | free |
| 152 | PG9 | — | | | free |
| 153 | PG10 | `PS_IO6` | `OCTOSPIM_P2_IO6` | i/o | |
| 154 | PG11 | `PS_IO7` | `OCTOSPIM_P2_IO7` | i/o | |
| 155 | PG12 | `PS_NCS` | `OCTOSPIM_P2_NCS` | out | 10 kΩ to the 1,8 V — deselected while the pins are high-Z at boot |
| 156 | PG13 | — | | | free |
| 157 | PG14 | — | | | free |
| 158 | VSS | | | | |
| 159 | VDD | 1,8 V | | | |
| 160 | PG15 | — | | | free |
| 161 | PB3 | `RXD` | `TIM2_CH2` capture | in | the same net as PB7 — the timebase captures the start edge of the card's frame |
| 162 | PB4 | — | | | free |
| 163 | PB5 | — | | | free |
| 164 | PB6 | `TXD` | `USART1_TX` / `TIM4_CH1` | out | the NodBus link — through the `SN74AXC` translator to the data body |
| 165 | PB7 | `RXD` | `USART1_RX` / `TIM4_CH2` | in | through the translator; the pins switch USART → TIM4 for the ranging instant |
| 166 | BOOT0 | 10 kΩ to ground | | | |
| 167 | PB8 | `SCL` | `I2C1_SCL` | i/o | through the `PCA9306` to the power body — the `INA238`, alone on this bus; 4,7 kΩ on each side |
| 168 | PB9 | `SDA` | `I2C1_SDA` | i/o | through the `PCA9306`; 4,7 kΩ on each side |
| 169 | PE0 | — | | | free |
| 170 | PE1 | — | | | free |
| 171 | PDR_ON | tied to `VDD` | | | the power-down reset stays armed |
| 172 | VDD | 1,8 V | | | |
| 173 | PI4 | `D5` | `PSSI_D5` | in |  |
| 174 | PI5 | — | | | free |
| 175 | PI6 | `D6` | `PSSI_D6` | in |  |
| 176 | PI7 | `D7` | `PSSI_D7` | in |  |

**62 GPIO used, 77 free, `PH1` unconnected** (PA2–PA3 · PA7–PA12 · PA15 · PB0–PB2 · PB4–PB5 · PB10 · PB12–PB15 · PC2 · PC5–PC7 · PC11 · PC13–PC15 · PD0–PD1 · PD3–PD15 · PE0–PE1 · PE3 · PE7–PE15 · PF7–PF10 · PF13–PF15 · PG6 · PG9 · PG13–PG15 · PH4–PH8 · PH13 · PI5 · PI8–PI11).

### Timers

| timer | bits | kernel | pins | job |
|---|---|---|---|---|
| **TIM4** | 16 | 2²⁸ | CH1 → PB6, CH2 ← PB7 | the ranging turnaround: one-pulse, triggered by the capture on CH2, the return raised on CH1 after `CCR` ticks; PWM-input mode on CH2 measures the incoming pulse's width. 244 µs of range at 2²⁸ — a port past 24 km runs it at ÷4 |
| **TIM2** | 32 | 2²⁸ | CH2 capture ← PB3 | the free-running timebase; the capture of the card's frame start places the round on the grid |
| TIM1 · TIM3 · TIM5 · TIM8 · TIM12–TIM17 · LPTIM1–LPTIM5 | | | | free |
| **TIM6** | 16 | 2²⁸ | no pin | **the housekeeping second**: the ladder's one-second scan and the `IBIAS_MON` read, at the lowest interrupt priority (`../photon/FIRMWARE.md`) |
| TIM7 | 16 | | | free |

### The station interface — 3,3 V on translators

**The station interface stays 3,3 V** — the Galvani body is 3,3 V logic on every board in the
station and does not change for a board whose processor runs 1,8 V. **Fixed-direction level
translators** — two `SN74AXC8T245`, one a direction, `DIR` strapped, Marconi's pair (1,2–3,6 V,
push-pull on both sides, ns-class edges) — carry the connector's lines — **2 out** (`TXD`, `DE`), **5 in** (`RXD`, `RXD_ECHO`, `CLK` into
`OSC_IN`, `SD`, `ALERT`); **a `PCA9306`** carries `SDA`/`SCL` to the power board's `ISO1642`,
which does not take 1,8 V, with 4,7 kΩ on each side. `ALERT`, `SD` and `RXD_ECHO` are held low
by 100 kΩ on the 3,3 V side, so an empty or dead line reads quiet. `ID` needs nothing — it is a
ratio against the board's own `VREF+`. The rest are resistors and no processor pin: `LINE_EN`
10 kΩ to 3,3 V, `A_SEL` to ground — one power socket on a unit, so its board is 0x40 — `B_DIR`
tied to ground, and `ID_RET` and `ENABLE` not connected (*The sockets*). **An auto-direction translator is not used on a clock line**: its
one-shot accelerators and pass-gate pull-ups round the edges into series resistance and cable
capacitance.

### The sockets — the data body and the power body

**Connectors: `NB IN` · `PWR IN` · `HV` · `12V`** (`../../../galvani/README.md`, *Connector names*).

**`Quark-Neutron/Positron` is a measuring unit and stands at the U end of its spur, never in the station's
enclosure.** Everything on the Galvani boards behind it runs whenever the board does, held by
resistors: no processor pin switches anything on either body. Every logic line crosses the
`SN74AXC` translators or the `PCA9306`; the connector side is 3,3 V.

**The data body — 12 pins, 2×6, `BX2.54-2xNA`.**

| pin | signal | on this board |
|---|---|---|
| 1 | `CLK/PPS` | the spur's 2²², through the `SN74AXC` translator into PH0 `OSC_IN` |
| 2 | `GND` | ground |
| 3 | `TXD` | PB6 `USART1_TX` / `TIM4_CH1`, through the translator |
| 4 | `RXD` | PB7 `USART1_RX` / `TIM4_CH2`, and PB3 `TIM2_CH2` on the same net, through the translator |
| 5 | `ID` | PC0 `ID_D`, 10 kΩ 1 % to `VREF+`, the 1,8 V |
| 6 | `ID_RET` | not connected — a unit carries no host code |
| 7 | `RXD_ECHO` | PB11 `USART3_RX`, through the translator; 100 kΩ to ground on the 3,3 V side |
| 8 | `DE` | PE2, GPIO out, through the translator |
| 9 | `B_DIR` | **tied to ground** — this end listens on channel B; no processor pin |
| 10 | `LINE_EN` | 10 kΩ to 3,3 V — the line side runs whenever the board does; no processor pin |
| 11 | `SD` | PE4, GPIO in, through the translator; 100 kΩ to ground on the 3,3 V side |
| 12 | `3,3 V` | the ordinary 3,3 V — the **`TPS7A2033`** off the 4,0 V `TPS629206`, the rail the station interface and the plugged communication board stand on |

**The power body — 8 pins, 2×4, `BX2.54-2xNA`.**

| pin | signal | on this board |
|---|---|---|
| 1 | `ENABLE` | not connected — the unit power board pulls it to its input |
| 2 | `GND` | ground |
| 3 | `ID` | PC1 `ID_P`, 10 kΩ 1 % to `VREF+`, the 1,8 V |
| 4 | `SDA` | PB9 `I2C1_SDA` through the `PCA9306`, 4,7 kΩ on each side |
| 5 | `A_SEL` | to ground — the `INA238` at 0x40, alone on I2C1 |
| 6 | `SCL` | PB8 `I2C1_SCL` through the `PCA9306`, 4,7 kΩ on each side |
| 7 | `ALERT` | PC8, EXTI, through the translator; 100 kΩ to ground on the 3,3 V side, high = alarm |
| 8 | `3,3 V` | the same ordinary 3,3 V — the power board's supply |

**The 12 V** arrives from the unit power board's island output on two two-pole Degson
**`DGPS2.5R-5.0`** terminals, an input and a tap, the same node, and on neither ribbon.

### Clock tree

| | |
|---|---|
| `OSC_IN` | 2²² = 4,194304 MHz, the data body's `CLK` through the translator — **no quartz on this board at all**: the station's clock is what runs it, and the word *crystal* in this project's scintillation text means the **scintillator** |
| HSE | bypass |
| PLL1 | M 1 · N 128 · P 2 → VCO 2²⁹ = 536,870912 MHz, `P` **2²⁸** |
| SYSCLK, AXI, AHB | **2²⁸ = 268,435456 MHz** — VOS0, `VDD` ≥ 1,71 V |
| APB1/2 | ÷2; the timer kernels at 2²⁸ |
| PLL2 | M 1 · N 84 · **P 4** → VCO 352,321536 MHz — inside the wide VCO's 128–560 MHz, the only range a 4,19 MHz input may drive — `P` **21 × 2²² = 88,080384 MHz**, out on `MCO2` (PC9) as the converter's clock and the PSSI's `PDCK`. Integer, off the same 2²² reference; no fractional divider |
| the converter's clock | **88,080384 MHz** on PC9 `MCO2` to the converter's `CLK+` and to `PSSI_PDCK` alike; the converter's ÷2 makes the **44,040192 MSPS** encode — the grid; nothing comes back from the converter |
| OCTOSPI kernel | 2²⁸ ÷ 4 = **2²⁶** — the PSRAM's clock, octal DDR |
| the clock gone | HSI fallback and the degraded flag; the off sequence drops the clock the same way and the returning feed is the wake (`../../../core/PROTOCOL.md` §7) |

Two integer PLLs off the one 2²² reference and no fractional divider anywhere. The encode is not a
power of two: **2²⁸ / fs = 128/21 exactly**, so a time in 2²⁸ ticks becomes a sample index by one
multiply and one shift, with no remainder.

### The PSRAM — a position on the board

**`APS25608N-OBR-BD`, 32 MB octal PSRAM, 1,8 V, on OCTOSPI1 port 2 in octal DDR memory-mapped mode
at 2²⁶** — the drawing carries the position whether or not it is loaded. **32 MB is the smallest octal part made at this speed**;
a smaller one would not be cheaper. The figures are the sheet's (AP Memory rev. 1.2).

| | |
|---|---|
| the part | **`APS25608N-OBR-BD`** — 256 Mb (32 M × 8), octal SPI DDR, 1,62–1,98 V; **mini-BGA 24, 6 × 8 × 1,2 mm, pitch 1,0 mm, ball 0,4 mm**, package code `BD`; the standard grade, `T_C` −40…85 °C (`-OBRX-BD` is the 105 °C grade and is not needed). 200 MHz on the sheet, 2²⁶ here |
| the balls | `CE#` A2 · `CLK` B2 · `DQS/DM` C3 · `ADQ0` D3 · `ADQ1` D2 · `ADQ2` C4 · `ADQ3` D4 · `ADQ4` D5 · `ADQ5` E3 · `ADQ6` E2 · `ADQ7` E1 · `VDD` D1, E4 · `VSS` C1, E5 · `RST#` A3 · `RFU` C2 (a second `CE#`, left open) · the rest `NC`. **`VDDQ` is `VDD` inside the part** — two supply balls, one rail |
| `RESET#` | **left open** — a weak pull-up inside; the firmware resets by the Global Reset command (four clocked `CE#` lows) after the 150 µs power-up phase |
| power-up | `CE#` tracks `VDD` within 200 mV and `CLK` stays low through the first 150 µs — the **10 kΩ pull-up on `NCS`** and the MCU's pins at reset do both; Halfsleep and deep power-down are usable only 1 ms and 500 µs after that |
| the bus | `IO0`–`IO7` PF0–PF2 · PF3 · PG0 · PG1 · PG10 · PG11 · `DQS` PF12 · `CLK` PF4 · `NCS` PG12; `NCLK` PF5 not used, single-ended clock. Source-synchronous: no series part on the data lines or `DQS`, traces ≤ 30 mm and matched to ±5 mm over an unbroken ground — **the sheet allows 15 pF of load**, and the pads are 5–6 pF; **33 Ω at the MCU's `CLK` pad**, the house series part on a fast line. Output drive 25 Ω, the default |
| `NCS` | **10 kΩ to the 1,8 V** — deselected while the MCU's pins are high-Z at boot, and the power-up condition above |
| decoupling | **the sheet's two**: a low-ESR **1 µF** at the supply balls (the house 1 µF 50 V) and a **10 µF** beside the part (the house 10 µF 50 V 1206) for the self-refresh bursts — 25 mA peaks of tens of µs in standby, which the regulator does not see. No filter part: a digital load on a digital rail |
| current | **≤ 20 mA while a record is written** (the sheet: 5 mA at 13 MHz, 19 mA at 133 MHz, 2²⁶ between them); standby **≤ 680 µA at 85 °C**, 90 µA typical at 25 °C; Halfsleep 40 µA typical, deep power-down ≤ 20 µA — none of them worth a mode on a board whose rails are sized for full load |
| clock | 2²⁶ = 67,108864 MHz, the OCTOSPI kernel at 2²⁸ ÷ 4; DDR, so 134 MB/s on the bus — a 120 kB record is under a millisecond. **Write latency code 4 (`MR4[7:5]` = 100b, `F_max` 109 MHz)** — the default code 5 also holds, code 3 stops at 66 MHz and is under the clock. **`CE#` low ≤ 4 µs a burst (`t_CEM`)**: the OCTOSPI's `REFRESH` field caps a memory-mapped access at 268 clocks, so the peripheral breaks long DMA transfers by itself |
| when it runs | **not in the base build** — the position is loaded on `Quark-Photon` for the burst records (`../photon/FIRMWARE.md` §B6); this board's image writes nothing there and the footprint stays open |

### Analogue and reference

**Every filter inductor on this board is a π filter: 10 µF 1206 + 100 nF 0603 before it on the rail and
the same pair at the pins behind it** — the house rule (`../../../core/POWER.md`, *The filter parts*).

**The converter's `DRVDD` takes the 1,8 V through the shielded 2,2 µH into 10 µF + 100 nF, then the bead `GZ2012D301TF` into 100 nF at the pin** — the `AD9251`'s words at 88 MHz put a data line's fundamental at 44 MHz at most, under the inductor's 69 MHz self-resonance, and the edges' harmonics above it, where the bead absorbs them. `VDDA` and `VREF+` are one node on the **1,8 V** through one shielded 2,2 µH `SWPA252012S2R2MT`, 10 µF + 100 nF at the pins — the rail is a buck's own, so the filter is the buck-noise inductor of `../../../core/POWER.md` and not a ferrite; `VSSA` to
the ground plane at one point — the LQFP176 bonds `VREF−` to `VSSA` inside. The two `ID` inputs are read
single-ended against `VREF+`, and their 10 kΩ 1 % pull-ups hang on `VREF+` itself, so the reading is a
ratio and the rail's tolerance drops out; the data body's 3,3 V is not used for the `ID` on this
board. The arrived 12 V and the input current are the power body's `INA238`, over I2C1. The fitted `LT3571`'s monitor and DAC are inside its
servo loop, so the reference's value reaches no measurement.

## What this board takes from `Quark-Photon`

The converter's port and its timing, the clock tree, the rails, the SiPM bias supply's recipe,
the station interface and the processor weighed against it are the H7A3 layer the two boards
share, written once on Photon's board ([`../photon/HARDWARE.md`](../photon/HARDWARE.md)); the
firmware is one image on both ([`../photon/FIRMWARE.md`](../photon/FIRMWARE.md)).

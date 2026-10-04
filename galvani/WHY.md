★ N.I.C. ★

# Galvani — the graveyard

Rejected alternatives and superseded states, with the reason. Nothing returns from here. Board
and sheet numbers below are the retired schemes' (*The sheet numbers — both schemes*); read a
number by the entry it sits in.

## The ejector-latch header — weighed and dropped

`NJ2.54-2xNA`, the same family with ejector towers, was the first pick. Its catches start at 16
pins and the station's bodies are 8, 10 and 12: **the bulk without the function**, 126 mm of edge on
an eleven-body card. The `FC` socket's strain-relief cover and a tie hold the ribbon.

## Mechanical keying between the connector bodies — weighed and dropped

Weighed when the 10-pin time bus made a third width. **Pin removal**: ten signals plus a key is
twelve, the data body's width — the collision it prevents. **A second pitch**: a second stock line
for one connector. **A keyed shroud family**: part numbers differing only in a key, which somebody
orders wrong. Width, silkscreen and the `ID` read tell the bodies apart.

## Taxonomy

**The class letters, the `L`/`H` split, the thirteen-board family and the 416-up renumbering** — a
taxonomy that no longer exists; the family splits by job, power board or communication board. The
pod's optical front is still not a 1×9; the island is still the cap, and the voltage the way past it.

**Two connector widths as an interlock** (`Galvani 1`, `Galvani 2`) — the feed never rode them, so
a cross-plug could damage nothing.

**Eleven sheets → seven.** `G-48-12` was `G-48-U`'s cell to the feedback resistor, once the island
delivers 12 V; `G-300-U-H` was `G-300-U` with a 12 V secondary, `LT8316`'s ~1 % minimum load putting
a node at 11 % of 28 W; `G-12-I` was `G-48-U` without the island; `G-A-2` the 1×9 seat with the
slower module; the two station cells share `LT3748` and the FET. Added: the ModBus sibling of
`G-I-2-025`, one `ISO1450` — a Palatine arm used one transceiver of three. Sheets 413, 415, 416,
419, 421 and 422 retired, never reused.

**The 6 V rail, `EN_6V`, `PG_6V` and the `NX3008NBKT` gate** — only for three boards' true 5 V
behind an LDO, 5,5 V being in dropout; gone with the 12 V island, those boards bucking to 5,5 V ahead
of their `TPS7A4701`.

**The 12 V board's 250 mA** — a 15 W class held to the ≤ 3 W spur budget for a load never stated;
the island's rating is the ceiling.

**Two port boards back to back** to feed a foreign device — never drawn; the unit board's 12 V
terminal does it.

**The radiation heads' exemption from isolation** — for pulse leads co-stamped at 1/128 s, which
no bus could carry; the head now counts on Quark-Tubes' H523 and answers on a mini segment.

**"Rejected parts are footnotes, reasoning in git history."** Withdrawn: the reasoning lives here.

**Ceres and Sakura with their own line front** — their sensing draws less than an idling
transceiver and a second board doubled the build; on fibre, the exception bought nothing.

**"The GDT sits on the station-end power board only"** — from when both ends were one port board;
the ladder is populated by sheet.

**Communication boards with their own battery terminal, fuse and buck** — replaced by the host's
rail over the data connector (the card's `LMR43620`). The optical boards' 4,0 V buck and
`TPS7A2033` went too: the module asks only a π filter, and the receiver has its own limiting
amplifier.

## Windings

**The shopping targets that predate the catalogue parts.** The method stands; three of five columns
did not. `G-300-S`'s ~10 µH was half the answer, the volt-second product wanting 5 µH at 1:10
(`750310349`); `G-48-U`'s ≥ 3,5 A belonged to a higher rating, `I_LIM` being 1,33 A; `G-48-12`'s
≥ 315 µH came from a vanished design; `G-300-U` was missing. Only `G-48-S`'s row survived.

## The house buck

**`MPQ4321` — superseded on one axis.** It won on every other (−40…+150 °C, 42 V load dump,
output-referred `PG`, one `R_FREQ`, $0,45) and lost on **±10 % spread spectrum that no pin, bit or
order code turns off**: a discrete line can be notched or stepped around, a smear cannot, and under
Tesla (5–512 kHz) and Marconi (0,5–16 MHz) every harmonic to 16 MHz is in band.

**`LMR43610` locked to 2²¹ by its `SYNC` pin — dropped with the pin.** 680 dB where 80 dB was three
orders past the requirement, a clock the boards lack, the frequency baked into the part number; an
`RT` resistor parking on the decimator's broad zeros does the same.

**A low-frequency buck — rejected.** The decimator attenuates only above `f_DATA`/2: 100 kHz lands
at −0,4 dB and 400 kHz at −6,3 dB, against −79 dB near 2·`f_DATA`.

## Duplex — the reversal record

Half duplex while both feed pairs were paralleled; full when the feed took one pair; half when the
ground proved a conductor and claimed it back; full when the feed left the data cable and long
copper went to glass. Each reason lost its premise; every NodBus run is full duplex.

## The feed

**The 200–300 V band** — withdrawn; it made every reach figure ambiguous by 2,25 and bought nothing.
300 V is one nominal.

**Feeding a spur from the battery rail.** A constant-power load settles at
`V_end = (V₀ + √(V₀² − 4·P·R_loop)) / 2` and collapses when V₀² < 4·P·R. On 2× 2,5 mm²
(~13,8 Ω/km loop) at 75 % V₀ the 3 W spur reaches 10 km at 48 V and **0,63 km at 12 V** — inside a
spur's range.

**The tapped trunk** — tapping a 300 V cable instead of re-transforming at the remote hub (72 %):
three chained 8 W sites need 24 W plus cable at the head, not 38 W. It costs point-to-point — reach
accumulates, each branch needs its own current limit and a high-side 300 V kill. Recorded as the
alternative; the Frozen table is not written to it.

**`G-300-U` at 3,57 W with `15364-T008`** — merged into the 28 W board: one winding (`11328-T078`),
one shunt, a node at 11 %.

**`G-A-2-20` and `G-A-2-100` as names** — the reach in the name; one board, the module by
population.

**The 70 kHz noise criterion, and parking the converter on a decimator zero.** The rule could not
be written for Marconi (0,5–16 MHz), and a boundary-mode island flyback follows the load (335 kHz at
3 W, inside Tesla's band). **Every frequency this family can buy lands in somebody's band**: the
defence is distance, layout, forced PWM where a band listens, and the bench. Its arithmetic went
with it.

**The feed on the data cable's fourth pair** — the ground needed its own pair, so every run was half
duplex. A 2-core of its own (0,73 V of drop at 500 m for the 3 W spur, against ~5 V down one AWG24
pair) freed it; the echo-check receiver took over what half duplex gave free. Cat 6, kept for its
100 V conductor-to-conductor isolation, is now simply the cable.

**A hybrid opto-power cable as the feed's carrier** — one way of pulling the 2-core, not the
doctrine.

**Two 485s plus optocouplers, an isolated DC-DC, biasing and monostables** — rejected for the
integrated isolated transceiver (`ISO145x`, 50 Mbps, the clock pair too).

**"4,0 V exists for a receiver"** — argued from an LDO beside the optical module's RX; gone with
that LDO, 4,0 V staying as feedstock for a unit's own LDOs.

**Monolithic against external FET.** An external FET doubles the line items (switch, `R_SENSE`,
gate network, a bigger snubber and core). `G-300-S-H` merged into `G-300-S` as identical to the
shunt; `G-300-U-H` into `G-300-U` once only the winding differed.

**PoE and phantom powering** were never options — both need centre-tapped magnetics and a 485
transceiver has none.

## The connector contract

**Three strap levels (`DUPLEX` · `MED1` · `MED0`) → one `ID` resistor** — three GPIO per port on a
five-port card, against one ADC read of 1 % parts.

**`RE#` off the connector** — the receiver stays on for the echo check, listen-only was never used,
and transceiver sleep is `ENABLE`. Tied on the board.

**Two `ENABLE`s per port → one GPIO into both bodies.** No state ever wanted the communication
board without the feed or the feed without the board.

**The hierarchical `ENABLE` and the wake wire — deleted.** A wire Mayak → card → Argus, so one switch
folded the station. The owner never asked for it, and a remote hub is killed by a NodBus command,
the only thing reaching it over glass. **The one-board rule finished it**: Bifrost and Argus are one
board under one firmware, in the box or at a run's end, so a wired enable or wake would mean two
things by position. Off stays software, at the cost of a sleep floor — Stop, never Standby.

**`ALERT` one per source → wire-OR.** A host's power boards share one I²C: the pin says something,
the poll says which.

**`RXD_ECHO` added** — the echo-check receiver had a part and no pin.

**`G-48-U`'s `EN/UVLO` on a host GPIO** — a unit power board has no host in front of it and would
never start. Tied to the input.

**A unit switching its own link or supply off — declined.** It would drop the board on every reset;
a unit resets by watchdog and cold-restarts by the station's `PORT_PWR`.

**The clock-channel return gate (`OE_CLK`) — gone.** Start-up ranging moved to the data channel, one
gate per unit board, assuming a cable's pairs share their delay to a few per cent; on BiDi glass the
1490/1550 nm difference over 10 km is nanoseconds.

**Optical boards populate every section as standard** — one-way by depopulation made the reversed
socket a second build.

**Three connector bodies → one.** The 24-pin data, 10-pin control and 14-pin output bodies put
300 mm of header on a 44 cm perimeter. Once unit boards hand out 12 V only and every host makes its
own rails, what the 10 and the 14 carried (4,0 V, 3,3 V, `ENABLE`) fits the 24's ten spare pins.
(Superseded by two bodies, below.)

**Mirrored sockets → a crossed cable.** Mirroring put a Galvani board's `ID` resistor on both `ID`
and `ID_RET`; one pinout and a crossed host-to-host cable put it on one pin.

**`ID_RET` as a plain ground → a host's number.** Grounded, every host read 0 Ω. A resistor there
makes a wrong plug at the Mayak a fault before power; the Galvani values moved above the host range,
superseding the old six-value table (0 Ω · 2,2 k · 4,7 k · 10 k · 22 k).

**The ranging-return gate (`74LVC1G125`, `OE`) — gone; the return is the unit's timer.** A one-pulse
timer needs no part, pin or CPU, and adds a delay, so the return waits for the line to clear, and a
measured incoming width, so the two paths' distortions come back separately. The cost: `RXD` and
`TXD` on one timer, `RXD` on CH1/CH2; jitter ±4 ns against ±1 µs.

**Sheets by population → sheets by board** — two populations are two schematics in the EDA anyway;
the merge saved nothing. Renamed then: `G-I-2-025` → `G-I-N-025`, the ModBus board `G-I-M`,
`G-A-2` → `G-O-2-100`.

**The buck pair leaves the unit boards, and the in-box supply population with it.** For eight remote
units, ~22 bucks against ~19 with every host carrying its own — a wash. **What decided it**: the
300 V unit board is only the converter, the box has no supply board, and every host reuses one buck
worksheet. `EN_4V`, `PG_4V`, `PG_3V3` left the connector.

**The 12 V into a station power board over the connector, and Fastons for the battery.** Three IDC
pins and cores at 1,7 A — superseded (*The station feed cell's 12 V on the ribbon*, *The card as the
12 V distributor*). Fastons: a spade has no key, and a swapped pair is the classic field mistake.

**Seven digitally controlled power chips — weighed against the plain bucks and dropped.** I²C on a
buck is sold for core-voltage scaling (0,6–2 V, input ≤ 5,5 V), not for 3,3 V from 12 V:

| part | why not |
|---|---|
| `TPS65263` | I²C moves only 0,68–1,95 V; 600 kHz, in Marconi's band |
| `MP8862` | 65 % at 10 mA; a buck-boost nothing needs |
| `BQ2579x` | a charger, needing firmware from the first millisecond |
| `MPQ7920` | 2,7–5,5 V in — the phone-PMIC class |
| `MPQ70240/1` | the standard OTP starts buck 3 at 1,2 V and the H7A3 on it never boots to rewrite it; a custom code is an FAE |
| `MAX5073` | non-synchronous, 45 % at 0,2 A; no I²C |
| `TPS62932/3` | ±6 % spread spectrum always on; the FPWM code trades `PG` for `SS` and has no PFM. Reserve if price decides |

The wish — enable, mode, power-good, current per rail — the station has in the house buck's `MODE`,
`EN`, `PGOOD` and the `INA238`. It left a `TPS62821` second stage, since superseded (*`TPS62821` as
the H7A3 boards' second stage*). An outside suggestion of 1 µH and a `VBIAS` pin on the `LMR43610`:
1,35 A of ripple, and no such pin.

## The station feed cell's 12 V on the ribbon — superseded by a keyed lead

When the three bodies became one, both power boards took the 12 V on three ribbon pins: 0,3 A out of
a unit board, but 1,7 A into `G-48-S` and 4,3 A into `G-300-S` — **not a ribbon current**. The cells
moved to a keyed two-pin lead, a second ribbon per port costing too much card; itself superseded
(*One body → two*).

## `G-B-U` — an isolated 12 V unit board, never a sheet

The far-end mirror of `G-M-S`, feeding a unit 12 V over a short run. **It breaks the rule that the
battery rail never travels**: 12 V at a pump's current is worse than a 3 W spur dying at 600 m. The
drain head that raised it has its own 24 V source board (`../pluvius/HARDWARE.md`).

## The 48 V pair, re-rated on the catalogue — what left

**`750311456` on `G-48-U`** — a 2,4 A part for 12 V / 1 A, capping the run at 14,2 W whatever the
shunt; `750311607` (2,5:1, 14 µH, `I_SAT` 9,5 A) clears the whole run, `750311424` (3 A, ~20 W) did
not. **Run backwards on `G-48-S`** it would have capped the cell at a third of `750310988`.

**The 60 V / 3 A Schottky on `G-48-U`** → `SS310`, 100 V at 32 V of stress: the grade buys leakage,
which doubles every 10 °C. (`SS310` retired below.)

**`G-48-S`'s 8,0 W — withdrawn as unreconstructable.** A worked budget replaced it, 28,4 W in at
`I_pk` 9,90 A, ~26 W out, retiring the claim that a four-port optical remote Argus at 6,47 W sat at
19 % of margin.

## `G-M-S`'s 1,02 A current limit — retired

`R_LIM` 30,1 kΩ was set against the rectifier's rating, but the `SN6507` meets overcurrent only by
shortening pulses — no hiccup, no latch — so **a limit above the switches' own 0,5 A holds twice
their rating until the die overheats**. The limit became 0,5 A, declared 0,4 A, with no `INA238`
(fitted later, *`G-M-S` → `G-12-S`*).

## The 300 V unit board split in two, and `L`/`H` did not come back

**What splits `G-300-U-6` from `G-300-U-40` is the enclosure**: a pod's 40 mm pipe against a 23,5 mm
winding. `L`/`H` differed only in watts, the big island covering both; here the small sheet fits
where the big one cannot, and is not there for efficiency. `11338-T195` (8,6 mm) was dropped for
basic insulation on a submerged 300 V pod, and taken later (*`00399-T239` on `G-300-U-6`*).

## The rectifier ladder — three Vishay grades, and what left

`SS310` retired (3 A cannot carry 4,2 A), `V2F22` left `G-48-S` (five amperes cost nothing there),
`PMEG10030ELP` the 300 V island; the ladder is `V5N22-M3` · `V3PM10-M3` · `V10P10-M3` · `US1M` ·
`PMEG10020ELR`. The grade buys leakage; an ultrafast PN part costs a volt, 1 W on a 24 W island. No
snubber diode is owed: the RC snubber is DNP until the bench, 402 V of 650 at the drain.

## `G-300-S`'s 8,6 W and 37 W — one number, and the shunt sat on saturation

Both were the day's loads, not the board's. Beneath them, 6,2 mΩ put the peak at 17,7 A against
`750310349`'s 18 A — **98 %**. `R_SENSE` went to 7,6 mΩ, ~47 W out (superseded again, *The
volt-second screen*).

## `G-300-U-50` became `G-300-U-40`

The 50 W was capability, never a deliverable: `G-300-S` delivers ~47 W, the unit board loses 3,7 W,
the far end sees 41–43 W — **40 W declared**, `R_SNS` staying at 58 mΩ. Two thirds of the loss is
the rectifier (2,41 W in `V10P10-M3`); a bigger grade buys 0,4 W for a stock number. The cost:
`P-24-S` wants 45,6 W for Pluvius's head, so at a 300 V run's far end the head runs low in its
20–45 W range.

## One body → two, and the 12 V left both — the merge is superseded

A port is a 12-pin data body, a power body and two keyed 12 V poles. The merge had won on card area;
the split costs 22 pins against 24, the 12 V going to terminals at once. The merge had put amperes
beside a clock edge on one ribbon at ±1 µs, **and could not carry `G-300-U-40`'s 3,33 A or
`G-300-S`'s 4,6 A on three 1 A pins**.

**`FAULT` deleted from the contract** — no source; the alarms are the `INA238`'s `ALERT` and a
communication board's `SD`. **`ID` added to the power body** — a board known only by its `INA238`
vanished when the part was unpopulated or asleep; a resistor answers when the board is dead.

## `G-M-S` → `G-12-S`, and the board gets its `INA238`

A power board (`SN6507`, no transceiver) wore a communication board's letter; `G-12-S` is 12 V,
source end, and there is no `G-12-U`. "Telemetry: not fitted" was reversed: **the failure is
invisible without it** — overloaded, the `SN6507` shortens its pulses, the arm droops and every
sensor reads wrong, like a broken sensor in Palatine's polls.

## `G-I-M` → `G-M-S` — the rename that was only half done

`G-M-S` came off the power board and for a day nobody wore it. The 485 arm board, still `G-I-M`,
took it for its one end — the far side of a ModBus arm is never a Galvani board — and `S` says there
is no `G-M-U`. (Renamed again, *`G-M-S` → `G-I-M-005`*.)

## The two growth pins on the power body — retired the day after

A 2×5 power body with two unassigned pins went to 2×4: a growth pin costs footprint, ribbon and an
open pin everywhere for a signal nobody could name, and with nothing drawn a later pin is a sheet
edit. (Corrected below: the odd one is reserve.)

## `G-12-S` measuring on the host's own ground — withdrawn the day after

The `INA238` on the primary saved an `ISO1641` and an `EL357ND` but missed its failure: **the
drooping arm is a secondary voltage**, the primary's 12 V never moves, and a primary shunt mixes in
the converter's losses in a load-dependent share. The shunt sits on the arm and the board carries
both isolators.

## Galvani's own sleep model, and `ENABLE` as one name for two jobs

This file said sleep was the clock alone and a sleep packet wakes nothing — true of a packet, false
of the line it arrives on; the protocol owns sleep. A wake wire: no spare pin among six in-box
wires. A switch on a processor's rail: the idling buck costs microamps, and cutting it removes the
ear that wakes it. `ENABLE` named both the port's kill and the `SN6505B`'s `EN`; the data body's
became `LINE_EN`, kept though only copper uses it because eleven pins is not a dual-inline body.

## The optical load switch — deleted while still open

Both optical sheets owed "a load switch behind `ENABLE`". A killed run takes the far module with the
feed, and a near laser left lit costs less than the switch and its drop.

## Saving power at all — no policy, by decision

Every rail is sized for its board's full load, so duty cycling reclaims nothing; with nothing
measuring during sleep, no board needs a quiet-mode strap.

## Two `ID` scales, and the Bifrost's duplicate code

The data body coded 0,10 for a card and 0,20 for an Argus up port, but a host has one `ID_RET`
resistor and knows its ports by construction: one code. The bodies' independent scales were sound
and dropped: **a number meaning two things costs a person time**. One scale, windows 0,05 ± 0,025,
1 % parts within ±0,005. 0,00 became the fault code, the Mayak moving to 1,10 kΩ lest a dead board on
a Bifrost's up port read as the Mayak.

## `SN6505B` has no fault output

Its sheet (SLLSEP9I) brings out none of thermal shutdown, the 1,75 A clamp or UVLO, so a `FAULT` by
an optocoupler shorting `ID` has no source on the most-built board short of a supervisor. The opto's
LED in series with `ID` **fails on ordering**: `ID` is read before `LINE_EN`, so every board would
read absent at bring-up.

## The optocoupler, the photoMOS, and why the sensing had to move

A phototransistor lands `ID` at 0,03–0,06, not zero, and 0,00 matters only because nothing else
produces it. A photoMOS dies with the isolated 3,3 V it would report; a normally-closed one fails on
ordering. **So the sensing moves to the primary**, on the `SN6505B`'s `VCC` — 1,56 mA running,
0,1 µA stopped, 1,75 A into a shorted rectifier — where a window comparator catches both, missing
only a slow drift behind the barrier. Its parts are not bought.

## "Neither body carries a spare pin" — wrong

A dual-inline count is even, so deleting one pin saves nothing. The odd one is reserve: the power
body kept its eighth. (Taken by `A_SEL` since.)

## `TLP172A` as the `ID` short — retired, and the retirement corrected

Toshiba photorelay, `R_ON` 1 Ω against the 10 kΩ pull-up — a true zero. But it is normally open and
the fault leaves nothing to light its LED, a Form B part fails on ordering, and it is Not Recommended
for New Design and rated only from −20 °C. **The correction**: wherever the isolated side is alive —
a real fault output, the `INA238`'s `ALERT` — a photorelay lands `ID` on the true zero a
phototransistor cannot.

## Crossing `ID`/`ID_RET` by pin position instead of in the cable — cannot be done

Mirrored positions on a straight ribbon need two socket pinouts, and a Galvani board would not know
which it sat in: **one pinout is what lets any board fit any port**. A third body for the in-box
link costs a footprint on every host to save one cable per station. **Crossing by the socket's
role** fails too: a card's up port takes the Mayak's trunk in the box and a Galvani board at a
remote Argus. The crossed cable is RS-232's null-modem; Auto-MDIX in firmware is not built (below).

## Mechanical keying against a wrong plug — not bought beyond the two widths

Within one width a wrong plug damages nothing and the `ID` read reports it. Keying is spent where a
wrong plug destroys something — the 12 V, already keyed; the rest gets silkscreen.

## The sheet numbers — both schemes

EDA sheet numbers from the 400 series — 401 `G-48-S` · 402 `G-48-U` · 403 `G-300-S` · 404
`G-300-U-6` · 405 `G-300-U-40` · 406 `G-12-S` · 412 `G-I-N-025` · 413 `G-M-S` · 414 `G-O-10-10` ·
415 `G-O-2-100` — the second scheme, from the first by 411 → 401, 412 → 402, 424 → 403, 414 → 405,
420 → 406, 417 → 412, 423 → 413, 418 → 414, 425 → 415. Retired with the station-wide numbering: the
name is the identity (`../daedalus/WHY.md`). Nothing was drawn under either.

## The card as the 12 V distributor — superseded

After the connector split the card took the battery on a keyed two-pin and carried the 12 V to an
output beside each port. Palatine's board size pushed the 12 V onto wire, and the family followed:
**the 12 V is a wire, not a board** — an input and a tap terminal per board. The port outputs, the
"12 V lead beside the ribbons" and the "one terminal per board" left with it.

## Channel B joined by gates, and `B_DIR` read back by the processor — superseded

A buffer each way on the one `CLK/PPS` pin, `B_DIR` read on a host GPIO. One `SN74LVC1G3157`
replaced them: **a dead module's `TD` or `RD` on a driven node is a diode into an unpowered rail**,
which a gate feeds and an open switch does not. `B_DIR` lost its processor pin: a host knows its
socket by construction, and a GPIO driving it brings back the both-ends-drive state. (The switch
superseded too, *Channel B through an `SN74LVC1G3157`*.)

## Shed load — the unit's own rail enables, dropped

The doctrine's second state: a unit cut its sensor rails at the regulators' `EN` pins. It bought a
power cycle for a part with no reset pin, and cost a pin and a pull per regulator, a doctrine across
sixteen documents and a hidden fault — **a shed unit fell below its island's 1–2 % minimum load**.
The station's `ENABLE` cold-restarts every remote unit. Babel keeps it as the builder's option for a
foreign chip whose only reset is a power cycle.

## The one-cable half-duplex build of `G-I-N-025` — deleted

A hand strap putting TX and RX on one pair, `ID` at 8,25 kΩ, the brown pair to the 48 V — to save a
second cable on a short run. It cost two pair maps, two meanings for one `ID`, a duplex argument in
every bus document, a second ranging case and a bursting mechanism, **for a site never named**.
`G-I-M-005` stays half duplex, ModBus RTU turning the line by its own rule.

## `G-M-S` → `G-I-M-005` — the end letter goes, and the name joins the formula

The board has no end: the half-duplex pair is symmetric, `DE` turns it at master and slave alike and
`B_DIR` is unconnected, so an end letter would say something false. The name also sat outside the
formula `G-X-r-d` (cable · rate · reach). Ceres and Sakura keep a bare `THVD1450`: the dish has no
room for a body.

## A 500 m ModBus arm — superseded by 50 m

The arm carries a `G-12-S`'s unregulated ~12 V at 0,4 A: 500 m of 2,5 mm² drops 2,8 V, **under the
10 V floor of half the sensors**; 50 m drops 0,28 V. Further out is a fed run.

## A MOD with a data body on `G-I-M` at both ends — one day

No MOD carries a data body. Pluvius sits behind the arm's `G-I-M-005` with its own `THVD1450` and
takes a 48 V feed on a power body. (For an afternoon it carried a `G-I-N-025`, which bound the unit
to one port, `../pluvius/WHY.md`.)

## The Phoenix plug-in pair on the 12 V — `CCA 2,5/ 2-G-5,08 P26THR` + `FKC 2,5/ 2-ST-5,08`

A THR header and a push-in plug per lead. ModBus sensors are stripped into a bare-conductor block
anyway, so **a second family for the 12 V is an order code for nothing**; the Degson block also
takes the 300 V positions, which the plug pair could not.

## The 48 V pair "recalculated toward ~40 W" — a premise that was wrong

The cell's watts were read as ½·L·I_pk²·f, so a lower-inductance winding looked like more watts.
**In boundary mode the inductance cancels**, leaving `P = ½·I_pk·(V_in·V_refl)/(V_in + V_refl)`:
the reflected voltage sets the watts per ampere — 2,85 W/A for `750310988` at 1:4,42, 4,29 for
`750310349` at 1:10, which is why `G-300-S` makes 47 W from the same 12 V. Forty watts on 48 V wants
1:1,6 or lower, ≥ 8 µH for the `LT3748`'s 400 ns window, `I_SAT` ≥ 15 A and 1,5 kV; nothing in the
catalogue does it (`750311592` needs 11,7 µH; step-down parts reversed leave 2–3,6 µH; the Sumida
PQ2620 parts are high-voltage primaries). The cell stays at ~26 W; beyond it is the 300 V pair or a
switched `G-24-S`, where `750311592` went.

## One terminal block on `G-I-N-025`, IN and OUT on the same poles — dropped

The argument — the terminal as the junction, so a pulled board does not open the segment — is void:
**the block leaves with the board**. Two solid 23 AWG cores in one cage are not a joint: a second
ferrule size and a crimp at commissioning. A two-entry push-in block was a second family for a
position that chains on one board in three.

## The terminator as a latching two-pin housing — dropped

A latch adds a connector family against the harmless failure, a lost shunt; the harmful one, a shunt
left on a middle board, no part prevents — it is a commissioning step.

## The Degson blocks that came and went

- **`DG235-5.0`** — 0,5–1,5 mm², above a sensor's 0,35 mm² lead and the data cable's solid core, so
  a ferrule on everything; 80 mm of edge for sixteen poles.
- **`DG250-3.5`** — 45° entry, where a field conductor enters parallel to the board. **`DG228-3.5`**
  — the ferrule stays, PC UL94 V-2 against the family's PA66 V-0, −40…+85 °C with no margin over
  the enclosure.
- **`DG235-3.81`** — 30,5 mm against 40, barely moving the edge.
- **`DG243-10.0`** on the 12 V and the feeds, 1000 V at 10 mm — `DG245-5.0` halves the edge, its
  450 V a creepage class: it holds 2,5 kV for a minute against the transil's ~565 V.
- **`DG241-2.5`** — 0,2–0,5 mm², below a field terminal's class, 6 A / 130 V starving the arm's
  supply. **The doubled pole went with it**: a run that carries on is twisted into the pole it
  shares, as a field terminal is wired everywhere.
- **A 2,5 mm² block at 90°** does not exist — Degson's 90° family stops at 1,5 mm² — so `DG245-5.0`
  at 45° was the exception. (All superseded, *Two Degson blocks … superseded by one*.)

## Four poles on an arm board, and the chain that came with them

For one pass the arm boards had four poles and arms chained, sixteen devices being too many to star
at one board. But each bought sensor has one cable, and only what is past the fourth chains:
**sixteen devices was never sixteen cables at one board**. Eight poles, four positions.

## 3,3 V on the boards that carry the lower rail — not taken

1,8 V there is the consumption optimum and the converter's `IOVDD` takes no other level; 3,3 V
converters cost dynamic range. A 1,8 V station would be worse: the line, the lasers and the Galvani
body are 3,3 V parts.

## The address strapped on the board — could not have worked

Power boards of a class are one board, so two on one controller strapped one address; the `INA238`
has no software address. **The socket names the board**: the spare pin became `A_SEL`, strapped in
the host's wiring and crossing on the `ISO1642`'s channel A to `A0` — 0x40 or 0x41, two power
sockets per controller. Not taken: a photorelay per board, a host I²C mux (the barrier onto the
host), `ISO1644` (a 10-pin power body).

## `FAULT` on the power body's spare pin — deleted twice

Neither the `SN6507` nor the `SN6505B` drives it, its position went to `A_SEL`, and a substituted
driver with a fault output reports over the I²C already there.

## The two-electrode tube across the pair — retired, and with it the coordination analysis

`2027-A-07-SM` and `2027-A-42-SM` were the differential tier. **A twisted pair leaves almost no
loop**, so a surge is common-mode, its differential residue the transil's, and the three-electrode
part covers both — at a unit end a floating one is two gaps in series anyway. The coordination the
pair forced passed and is kept so it is not reopened: `2036-07-SM` 250 V against `2027-A-07-SM`
300 V on 48 V, `2036-30-SM` 500 V against `2027-A-42-SM` 675 V on 300 V, earth first both;
`2027-A-35-SM` (297,5 V, 1,19×) and `2027-A-40-SM` were the grades weighed. Clearance follows
`2036-30-SM`'s 650 V; the 300 V boards' under-mask figure fell to 0,40 mm.

**The chokes at a unit end — removed**: no crowbar to work against, and the run's ~600 µH a
kilometre dwarfs 22 µH. **The per-port polyfuse — removed**: seconds to trip, re-closes into the arc,
reports nothing; the current limit, `ALERT` and `ENABLE` do its job.

## The three states — normal, sleep, kill — the middle one gone

The sleep saved a fraction of a watt where the whole branch could go dark. The states are *running*,
*off behind a port* and *off in the enclosure*; a port is at full load or dark, so no rail is held
into milliwatts and no bleeder is bought.

## A relay, then no switch, for the switched load on a unit board's tap

A FET on a 12 V wire leaving the board wants a gate driver and a ladder; a micro-relay replaced it.
Then the tap went: **every branch is switched at its supply**, the port's `ENABLE`, and the one
switched load has its own `G-24-S` — leaving the pressure boards no cavity part.

## Tube and chokes left standing at a unit end after the ladder changed

`G-300-U`'s bulk was derived from a pair tube and chokes at a 20:1 winding it no longer used, and
`G-I-N-025` had the tube at both ends. **At a unit end the run is the limiter** (~110 Ω, ~600 µH a
kilometre): the residue lifts the bank ~19 V, the FET sits at 402 V of 650, the tube at the source
end only.

## A fuse per port on the station power board, and no fuse box — superseded

Fuses are the installation's — PV → MPPT → pack → BMS, and a fuse box behind the BMS. A board's
protection is its supply's current limit, the `INA238` and `ENABLE`.

## Bulk capacitors weighed and dropped

Aluminium electrolytics — dropped for one part set on the surface and under water. Film (PET 10 µF,
PP 1 µF 1 kV) — size and price. A 250 V or 200 V ceramic on 48 V — twice the cost per effective
microfarad. SMD hybrids stop at 80 V. **The 450 V film bank on `G-300-U`** (9,5 µF) became ~2 µF:
the residue's 180 µC over the 90 V to the transil's `V_BR` is all a bank needs; ceramic on
`G-300-U-6`, wound film having voids nobody rates for 350–500 bar. **`C_CL` as its own part**
(100 nF 250 V) became the bulk ceramic, `G-48-S`'s clamp moving from 80 V to 50 V to keep the 100 V
part at 1,75×, for 0,06 W.

## The buck cell's per-rail passives, one `VCAP` part and the 10 kΩ pull-downs — superseded

Per-rail dividers and inductors became one cell everywhere, only `R_FBT` moving (`R_FBB` 12,1 k,
4,7 µH on `R3`, 10 µH on `MB3`); 3× 10 µF 50 V 1206 keep ~21–25 µF where one 22 µF 10 V part kept
half its marking, and the 3,74 V rail went. `VCAP`'s 2,2 µF, used nowhere else, became 2× 1 µF. The
10 kΩ pull-downs became the house's 100 kΩ: at 10 kΩ each driven pin burned 0,33 mA.

## Loose ends in the parts, closed

**"Chokes" without a value** — one 22 µH / 5 A part bought to 650 V on the four station boards; the
`LT3748` cells' 1,5 µH, `G-12-S`'s house 2,2 µH. **Values off the standard series** (`Rs` 25 Ω,
`Cf1` 60 pF, `Cin` 3,2 nF, `Cf` 3,07 nF, `R_CLK` 9,6 kΩ, 80 kΩ) moved within tolerance to 24,9 Ω,
62 pF, 3,3 nF, 3,3 nF with 182 Ω, 9,53 kΩ and 100 kΩ. **"Comb-clean by construction" on the
`SN6505B`** — from before *distance, not frequency*, and untrue: on its own oscillator the part
spreads its spectrum. **`G-12-S`'s limit at
0,5 A** — missed the turns ratio and the ±30 % spread, so a low part drooped at rated load; `R_LIM`
34,8 kΩ, ~0,7 A. **`G-12-S` at 9,53 kΩ** — 7,5 Vµs checked at the typical rate, not the lowest;
`R_CLK` 7,87 kΩ.

## The two load classes, the word "hub", and "station" for the S end — superseded

A *measuring unit* at ≤ 3 W and a *hub* at ≤ 24 W were a third number between draw and cell, a
trap for the first 6 W unit; **the cell is the only ceiling**, the `INA238` threshold set at
commissioning. *Hub* named a load class after a device. *Station* for the S end was wrong at a remote Argus, whose
`G-300-S` sources its run: `S` is source, `U` unit. Also dropped: *host*, *switch*, watt-named
classes, master/slave.

## "ModBus over glass" — a phrase, not a build

It meant one of our own units as a ModBus slave, Pascal the proof; Pascal is NodBus mini type 3, so
classic ModBus stays on copper inside an arm's 50 m.

## The measuring side fed "from the measured feed through its own small supply" — never a part

A linear regulator from 48 V and a depletion-FET source from 300 V were rejected the same day: every
power board carries the communication board's always-on island (`SN6505B` + `750313734` + 2×
`PMEG10020ELR`), a milliamp-class supply that reads a dark port too. `760390011` at 2,5 kV left with
it: one 5 kV winding on every island.

## Concepts that left `HARDWARE.md`

The island *keeper* and *the kit*; `G-48-U`'s in-box population; the pod that could not take a 1×9
and fitted a hermetic TO-can (the pressure build is the same module, asked of the maker); the old O
tier's burst-length bulk; gigabit BiDi with Manchester or 8b10b; the oil-filled deep sonde's optics;
the island's D+Z clamp (the cells carry an RCD); "terminator plug" and "shunt" for the termination
jumper, *shunt* being the `INA238`'s resistor alone.

## The 20 km 2 Mb/s module, and two transils on the regulated 12 V — superseded

`OPT2-31203STR` (1310 nm, 20 km) sat on its overload ceiling with a 15 dB window; the 1550 nm part
takes −39 to 0 dBm, so **a shorter part buys nothing**. Two `5.0SMDJ12A` on a unit board's 12 V
output followed the rule of two, meant for positions that meet a cable; behind the barrier one does
it.

## Channel B through an `SN74LVC1G3157`, and `G-O-2-100` one-way — superseded

Not needed on copper — the `ISO1450`'s `R` is high-impedance while its driver is on — and on glass
it passes the edge with ~17 pF where a buffer regenerates it. The card's `74AUP1G126` /
`74AUP1G125` with `IOFF` replaced it, on `G-O-2-100` too; channel B is one-way in either direction.
`B_DIR`'s 10 kΩ pull-down went once every socket tied the pin.

## Two cables into every fed run, without exception — superseded

Still the base on land; under water the feed rides in the data cable's jacket, on two conductors or
one with the sea return, because **there the laying is the cost**.

## `TPS62821` as the H7A3 boards' second stage, `LMR43610MB3RPER` and `TPS629203` — superseded

12 → 1,8 V in one step was taken to be at the edge of an on-time, so the H7A3 boards cascaded an
`LMR43610MB3RPER` and a `TPS62821`, the small rails a `TPS629203`. The `TPS629206`'s 40 ns minimum
against 150 ns at 1 MHz, and a cascade gaining ~1 point, made the station's bucks three parts.

## Six diode codes — four

`US1Y` does not exist — the US1 series ends at `US1M`. The blocking diode takes the bulk emptying
into a fired tube, ~29 A for ~13 µs on `G-300-S`, plus follow current until `ENABLE` acts; `US1M`
met it only on its 30 A surge figure, `US3M` (100 A) with a third used, and takes the rectifier too.
`V3PM10-M3` went into `V10P10-M3`, ~0,4 W less on `G-48-U`.

## The volt-second screen, 7,6 mΩ on `G-300-S` and 30 mΩ on `G-48-U` — superseded

The screen multiplied inductance by the winding's saturation current instead of the shunt's limit,
and took `G-48-U` at 3:1. Read as the `LT3748` sheet states it — **a minimum `L_PRI` rising with
`R_SENSE`** — `G-300-S` at 7,6 mΩ needed 6,1 µH against 4,5, `G-48-U` at 30 mΩ 25 µH against 12,6.
The shunts went to 5,5 mΩ and 15 mΩ, the largest allowed, delivering past the declared figure the
`INA238` holds. `750310349`'s 18 A was thermal, saturation 25 A typ. Not taken: `LT3751` (an
optocoupler and an `LT4430`), Coilcraft `GA3459-BL` and `DA2034-AL` (7–9 W of leakage in the
clamp), a custom winding.

## The `INA238` as a population, and one source board to one unit board — superseded

Mandatory: the shunts allow more than declared, the threshold holding the figure, and on 300 V its
`ALERT` takes the feed from a struck tube. A source board may feed a chain of unit boards;
`G-300-U-40`'s "up to 2,33 A" is its 3,33 A less what taps it.

## The automatic swap — possible, not built

The USART's `SWAP` bit, the S31's GPIO matrix and both `ID` legs on ADC pins would let a host turn
itself round on a straight cable, Ethernet's Auto-MDIX. Finding the way round faces two logic
outputs until one side decides, which a 3,3 V pin does not stand; **a crossed cable costs nothing**.

## `00399-T239` on `G-300-U-6` — superseded

Sumida's table gives it 6 : 1 : 0,7, so **`BIAS` sits at 8,6 V, under the `LT8316`'s 10–30 V**.
`11338-T195` replaced it (14:1:1,7, `BIAS` 20,9 V, 8,6 mm against 11,8), its 34 µH of leakage adding
an RCD clamp. Lifting the output to 14,3 V was dropped: a thin margin, a non-12 V rail.

## Two Degson blocks, `DG245-5.0` and `DG236-3.81` — superseded by one

The 45° and 90° blocks gave way to one PUSH-SNAP block on every position, `DGPS2.5R-5.0` (order
`10060008518`; 0,75–2,5 mm², 20 A, 400 V, 4 kV surge): one block on every terminal, the feed's clamp
inside its rating. The cost: data-cable cores under 0,75 mm² sit below its listed range.

## Fuse ratings written on the boards — superseded: sized by the construction

The README fixed 1 A for every host, 5 A for `G-48-S` and `G-24-S`, 10 A for `G-300-S`. **A fuse
protects the cable as much as the load**, and its fault current comes from the pack, the BMS trip
point and the cross-section to the fuse field — so it is sized per install, as Daedalus has it, and
no rating is written on a board.

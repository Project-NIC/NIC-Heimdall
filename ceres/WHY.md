★ N.I.C. ★

# Ceres — the graveyard

Rejected alternatives and superseded states, with the reason. Nothing returns from here and
nothing is deleted from it — the board's and both units' build, Sakura's own entries aside
(`sakura/WHY.md`).

## The line-front exception — retired

Sensing draws ~5 mW, the idling transceiver ~17 mW — the case for leaving the barrier off. **A shorter leaf answered it, not a cheaper front**: a Palatine on fibre at the plot puts the
sensors metres away. The line front is on the arm's Galvani board like everywhere else.

## The read method — what lost to the synchronous detector

Every alternative read one number where the medium has two, conductance and capacitance:

- **the U073's `TSC`** — ions read as water, drifts with salinity; the U073 left the slave tier
  with it;
- **an oscillator** (`555`, LC) — magnitude only, `G` shifts it like `C`, plus its own drift;
- **a diode peak detector** — magnitude only, drifts with temperature;
- **a phase detector** — phase only;
- **sampling voltage and current directly** — the H523's 2,5 MSPS ADC cannot reach 25 MHz and has
  no sample-and-hold to undersample;
- **an `AD835`-class multiplier** — the switch's job, for dollars and tens of mA.

A board per frequency or per project: three frequencies are three `ARR` values on one timer, and
Ceres and Sakura differ only in the enclosure. The H503: one ADC, no differential mode, so `I` and
`Q` read in turn and the sign is lost.

## The timer at 200 MHz and the reads at 1 / 5 / 25 MHz — superseded

`ARR` 199 · 39 · 7 off a 200 MHz PLL gave way to 2²⁷ Hz and 2²⁰ / 2²² / 2²⁴ off the house 2²⁴
crystal: **every clock in the station is a power of two**, a 16× spread separates water, ions and
coating as well as 25×, and the gate drives 100 pF more easily at 16,8 MHz.

## The second slave on the soil-temperature type — superseded

An NTC, plus a `TMP117`/`STS35` answering as a second slave where a soil-temperature position
matched the depth. **A bought T/RH pod is one slave**, and a two-slave Ceres was the one unit
shaped unlike its bought equivalent. The digital part is always fitted; soil temperature rides
`0x0001`.

## The electrode's cover and the enclosure — what went before the dish

- **PTFE over the electrode** — beads dew on Sakura; borosilicate takes no water, wets like a
  leaf, does not age in sun.
- **Epoxy as the dielectric** — 1–3 % water, drifted for weeks, sun-attacked; it now meets the
  outside only in the channel, outside the field.
- **The thermometer on a lead** — one more part and seal; on the board at the comb's centroid it
  reads the same plate.
- **A test tube with a rod electrode** — electronics 5–10 cm off on a connector, the seal that
  fails on a buried unit.
- **A four-layer FR4 leaf** (PHYTOS) — the mask chalks in sun in 1–3 years, FR4 takes
  water at the measured surface.
- **A head on a connector** — every connector on a wet unit is a seal.
- **Hot-melt potting** — polyamide takes 1–3 % water, polyolefin does not bond to glass.
- **A 60 mm dish** — a quarter of a 90 mm board carries the electronics and triples the comb.

## The dish's back — superseded states

- **The dish as the bowl and the lid over it** — back glass 15 mm above the board: 110 g of epoxy
  or an air cavity. The lid is the bowl.
- **Ceres filled without a back glass** — an epoxy back face hydrating in wet soil. A ring and a
  disc put glass on both faces.
- **A stop disc in the patrona** — unneeded with a solid fill.
- **Sakura's back the Petri dish itself, inverted as a bell** — with ring and disc on both, the
  second half has no job.
- **Air under Sakura's disc** — condensate forms there in a sealed unit. Both are solid.

## The comb on both copper layers, the internal RC, the follower's rail — rejected

The top face faces the fill, not the medium: **added copper is constant stray** beside the buck's
node. The internal RC's ±2 % goes straight into `C` and sits at the edge of RTU's baud tolerance;
the crystal's driver is off in Stop. An unpowered `THS4541` still driven by the switches takes the
drive through its input protection, so it is switched at `PD`.

## The supply — a `TPS629203`, an LDO behind the buck, the filters around it — superseded

The buck was a `TPS629203`, 17 V in, which the input transils' ~29 V clamp kills; then an
`LMR43610` at 3,7 then 4,0 V into a `TPS7A2033` at 3,3 V, an LC filter per consumer and a
one-height rule. Replaced by one `LMR43610` at 3,3 V, no LDO: **what costs accuracy is a switching
node beside the front end, not a percent of ripple**, and one rail leaves no mixed-voltage
interface. The 100 V input ceramic gave way to a 47 µF hybrid polymer with the house 50 V
ceramics; the ring follows the tallest part.

## Values of this board alone — superseded

2,2 kΩ on the thermometer's I²C (house: 4,7 kΩ); `VDDA` at 1 µH into 1 µF, then 2,2 µH into 4,7 µF
(house: 2,2 µH into 10 µF + 100 nF, like every buck-fed `VDDA`/`VREF+` in the station).

## Epoxy as the one fill — superseded by the condition

The fill was named epoxy; the build needs one that is inert and takes up little water. Low-uptake
epoxy is the reference, and any compound meeting both serves.

## The 47 µF hybrid polymer on the 12 V input — superseded by ceramics

The family's land-board bulk was copied onto this board. It is an 8 × 10,2 mm can, and the bowl
leaves ~4 mm above the board under the back — the can does not fit, and Sakura's thin block even
less. Four 10 µF 50 V 1206 leave ~16–20 µF at 12 V after the bias derating, which a board drawing
~50 mA through a read does not miss.

★ N.I.C. ★

# Palatine — the graveyard

Rejected alternatives and superseded states, with the reason. Nothing returns from here and
nothing is deleted from it.

## The first firmware's decisions, W1–W23 — superseded

W3 (the node's load-cell front) is Pluvius's; W4 (bench-writing bought sensors through a USB
adapter) went with the learning session; W6/W12 (the input-voltage byte) and W9/W11 (the fixed
payload map) with the blocks; W22 (mean and gust at the edge) because the node computes nothing;
W16/W23 (light sleep, deep-sleeping nodes) because nothing in the station is parked — an idle
board is ended and switched off. W21, no field calibration, is the station's
(`../core/INTEROP.md`).

## Two T/RH heights, a separate barometer and the 10 m wind — superseded

Air T/RH at two heights, a barometer apart from any T/H/P probe, wind at 10 m. A national network
takes T/RH at 2 m only (the second thermometer is the ground minimum at 5 cm, unshielded), may
house the barometer in the 2 m unit, and takes wind at 2 m, where evapotranspiration is computed;
nobody stands a 10 m mast on a field.

## The arms as a slow tier and a fast tier — dropped

The arms split by sensor speed, slow scalars on one pair, the rest on the other. **A rate is a
per-arm commissioning entry**, so an arm runs at what its occupants take and a slow sensor costs
only its own poll. Rate-splitting stays the fallback for a sensor stuck on its baud
(`../core/blocks/modbus.md`); the arms divide the electricals — reach, bus complexity, poll-round
length.

## The rain gauge — the standing vessel

A vessel standing on a ~10 kg cell over a ~1 dm² catch, drained by a gear pump through its
clearances (theory in `THEORY.md`). Replaced by Pluvius's suspended vessel on a 30 kg cell over
200 cm², emptied by a peristaltic head that counts what it removes (`../pluvius/`). Rain is
weighed, snow is a height, its water equivalent a downstream estimate.

## The fixed 32-byte map, then four tagged records — superseded by the blocks

A fixed 32 B payload (24 B of meteo at fixed offsets, 3 B reserve, a 4 B block of per-frame gamma
and neutron counts, input voltage last, big-endian, schema announced at discovery), then four
tagged 8 B records (address, channel, age in frames, 4 B value). **A fixed map fits one loadout, a
tagged record one value**; the self-delimiting block (`../core/PROTOCOL.md` §5) carries whatever a
sensor answered. Radiation never passes through Palatine (Quark-Tubes is a mini-NOD on Argus);
`Vin` went with every unit's supply streaming.

## The barometer on I²C — retired

An I²C barometer on the board (PD6/PD7, I2C3), kept inside for its −20 °C floor. **Palatine's one
input is the ModBus arm**: barometers are bought as RS-485 units rated to −40 °C; anything not sold
as RS-485 goes through Babel.

## Bought sensors named by type — superseded twice

The lists named parts (an SHT45 probe, a Chirp soil sensor, a radar by model). **A named part is
the one a builder abroad cannot get**; a hundred RS-485 units meet the same line, and a finished
RTU unit costs a few dollars over the bare sensor, the element the same at every price. The lists
went to quantity and requirement; `SENSORS.md` later added an indicative type and price, the
requirement still deciding. Soil T/RH went too — a humidity nobody reads — so soil positions are
thermometers and moisture is Ceres, as no bought coating survives the dirt.

## A fifth ModBus arm on the LPUART — dropped

A fifth arm on LPUART1 (a polled RTU line is the one bus an LPUART carries without loss) for a
separated sensor position, at five power bodies, twelve `ID` channels and three pin moves. **A
plot wider than four arms is a second Palatine on a fed run**, and the separated position guessed
at a site nobody had. The real need, a switched supply for Pluvius's head, is a power body alone —
`EXT`, two pins against six. No LPUART on a TDMA bus stands.

## The cold-protection chapter — the component table, the burial tables, the data-quality shutdown, the bimetallic latch — withdrawn

`THEORY.md` carried an operating-temperature chapter: a −40 °C component table, soil-at-1-m tables
for burying a remote enclosure, an "effective range" from self-heating, a firmware shutdown on CRC
rate and value drift, a sub-−40 °C variant (PT1000 probes, LiSOCl₂ cells), and a bimetallic latch
— a −40 °C bimetal on the battery latching two series MOSFETs in the station's supply until it
closed. It assumed a small battery and a supply that could not stand the cold.

The premise went: the pack was then a 300 Ah LiFeYPO₄ rated −45…+85 °C (now `../core/POWER.md` §3), the
supplies −55 °C, and the station's dissipation holds an insulated enclosure tens of kelvin above
the air — **the thermal question is summer's**, met by mass and ground coupling. The firmware never
had a temperature shutdown; its one thermal rule is the arm's CRC-miss rate (`FIRMWARE.md` §9).
The latch failed on its own: **no unit switches the station's supply** — off is `END` and the
port's feed, and only the BMS's processor-free lockdown takes the spine (`../core/POWER.md`, *The
lockdown*); a latch would be a second switch on that rail.

## Writing bought sensors with a USB adapter — superseded

Addresses and bauds were set on a bench through a USB→RS-485 adapter, one sensor at a time. The
learning session made it redundant: the head opens the arm as a byte pipe at the factory rate and
tunnels the same FC06 writes (`FIRMWARE.md` §6), so a sensor is written in place, one unwritten
sensor per arm at a time. `HARDWARE.md`'s GPIO budget (≈ 44 pins, against a pin table using 53)
also counted a buck `EN` pin the doctrine forbids; the pin table is the one count.

## The up port's `LINE_EN` on a GPIO, the arms on an in-box cable, the `INA238`s on I2C1 — superseded

`LINE_EN` sat on PE3, pulled to run and set high at boot; a unit end holds its Galvani boards
running with resistors, not a processor pin, so it is a 10 kΩ pull-up in the socket and PE3 is
free. The arms could stay inside on an in-box cable, the boot's `ID` read accepting one on an
arm's data body; an arm's far end is a bought sensor or a MOD, never a host, so every arm leaves
through a `G-I-M-005`. The arms' `INA238`s, listed on I2C1 with the up port's, are on I2C3 and
I3C1, two per controller; an arm's threshold "class" is its load measured at commissioning.

## Radiation on a Palatine arm, and lightning on a ModBus leaf — rejected

Radiation once rode a clocked ModBus arm on a fifth port, and `SENSORS.md` had `Photon` as an
MCU-less GM tube on Quark-Tubes' timer inputs. Radiation is Quark's and the scintillation units'
(NodBus and mini), Gauss a mini-NOD behind Argus — neither on an arm. The AS3935 was judged
inadequate, and a sferic detector on a ModBus leaf would break the self-contained frame; lightning
is Tesla, a NOD.

## The snow depth computed on the node — superseded

The node took N readings, a median, mast height less range with temperature compensation, and a
quality flag holding the last good value; `HARDWARE.md` later had the head subtract. **Palatine
computes nothing and the head interprets no payload**: the block carries range and echo quality,
and depth is derived where the archive is read.

## A missed sensor's last block shipped with `SENSOR` set — corrected

On a miss the ring kept the slot, so a silent sensor went up with its last value and `status` 2
SENSOR. `../core/PROTOCOL.md` §5 forbids it: **with no age field, a stale value looks fresh**. The
slot now empties on a miss until the next good reply.

## Mode B — what it was not made of

Computing everywhere: dropped — a station is entitled to raw values, so mode A stays the base and
the WMO tables are a second mode. A new payload: a table row is a ModBus block from an address
ModBus never assigns, handled like any bought sensor's. A per-row validity bitmap: `0x8000`, the
house map's absent value, marks it per column. Sunshine duration, dew point, sea-level pressure on
the board: each needs only the rows (sunshine by the reader's method); Palatine computes only
means, extremes, sums, vector means and the gust.

## A louvred screen — dropped for the passive triple-cylinder shield

WMO (Vol. I, chapter 2, §2.5) gives naturally ventilated screens and shields the same fault — up
to +2,5 K in sun and calm, −0,5 K on a clear calm night — and the same cure, ventilation. A screen
pays in volume (double louvres, roof and snow floor, a stand, repainting every two years, a door
nobody at an automatic station opens); **three concentric plastic pipes on polyamide rods pay with
a chimney**, for a fraction of the material, and are WMO's own named alternative (§2.5.2). A black
inner shell went: at ε ~0,95 the wall passes the element, by long-wave exchange, every tenth of a
degree it sits off the air. The mast mount went: shaded whenever the sun passed behind the pipe,
and within side-flash range of the strike terminal; the shield has its own post, poleward of the
mast (`../daedalus/CONSTRUCTION.md`).

## The barometer in the enclosure — superseded

A sealed IP68 box in a vault under a polystyrene lid is a pressure vessel with its own
temperature; WMO puts closed-room errors above 1 hPa and wind pumping at 2–3 hPa, and wants a
static head in open air. The barometer sits in the radiation shield with the T/RH probe, or is the
T/RH/P unit there.

## The monthly precipitation classes — Tab. 3 as printed

Kožnarová and Klabzuba (2002), Tab. 3, misprints one cell in each of six months; `CLIMATE.md`
once replaced the monthly rows with a computed table. The rows are carried, each misprint resolved
by the table's rule — adjacent classes meet — or, where two cells could be wrong, by the gamma
shape of the row's own 2 % and 98 % bounds: **IV** 60…130 then 141…160 — 2 % and 98 % at 20 and
260 give CV ~0,6, 75 % quantile 131: *wet* 131…160 · **XII** 60…130 then 141…180 — likewise,
131…180 · **VIII** 20…39, 30…49, 70…130 — CV ~0,5 puts the 10 % and 25 % quantiles at 44 and 63:
*dry* 40…69 · **IX** 10…29, 20…39, 50…140 — CV ~0,65 puts them at 31 and 52: *dry* 30…49 ·
**X** 10…19, 40…59, 40…140 — the one-cell reading is *dry* 20…39 · **XI** < 10 then 20…39 —
CV ~0,58 puts the 2 % at ~20: *extraordinary dry* < 20. The 1987 methodical instruction, the
article's source, would settle them and is not expected to surface. The computed table stays for a
country that publishes none.

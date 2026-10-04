★ N.I.C. ★

# Chinook — the air units

> **Design-stage concept — nothing built.** The figures are verified against the parts' sheets and
> the parts before a board is made.

What to buy when a site fits air chemistry, and what each unit must meet. **Every air unit is a
finished RS-485 ModBus module**, bought like the T/RH probe and the pyranometer: Palatine reads its
registers and ships them as a block, with no correction and no second processor. A part whose
datasheet says I²C inside is still bought as a finished ModBus unit — the sensing element is the
same part at every price, and the housing with its processor and 485 interface costs a few dollars
more than the bare element. **The electronics are trivial; the hard parts are temperature,
humidity and the inlet.**

**No product is named, and no register map is written here.** The same sensing element is sold
in a hundred housings on every continent and on every marketplace, the housing decides the map,
and the map comes on the sheet with the unit. A unit is bought by its **sensing principle** — the
tables below — and by the lines under *What every unit must meet*; its register run is read from
its sheet and entered in the arm's roster at commissioning, like the T/RH probe's
(`../FIRMWARE.md` §7). Where a sheet does not carry a figure, the unit is not bought.

## What to buy

**Optical first, where an optical part exists; a cell only where none does.** Prices are
approximate, for a finished unit in its housing:

```
OPTICAL — nothing consumed; a counter's fan and chamber want cleaning
  PM2.5 / PM10                laser scattering                       ~75–150 USD
  CO₂                         NDIR, 400–5000 ppm                     ~50–200 USD
  CH₄                         NDIR, from ~50 ppm                     ~30–100 USD
  CO, at fire level           NDIR, ppm resolution                   smoke, not the ambient background
  VOC, measured               PID, 10,6 eV lamp                      ~200–500 USD

ELECTROCHEMICAL — the cell is a consumable, ½–2 years
  CO, ambient                 the standard method                    ~100 USD
  NO₂ · O₃ · SO₂ · H₂S · NH₃  one cell per gas                       ~100 USD each

BY SETTING — what a setting tends to want, not a loadout
  forest, wildland            CO — the fire signal, co-timed with a Tesla strike      not PM (the dust is the forest), not CO₂
  field, agriculture          PM10 / PM2.5 in season — tillage, harvest, drift        not CO₂, not O₃
  livestock, manure, biogas   CH₄ by NDIR; NH₃ and H₂S by cell — the site's own gases
  town, city                  PM2.5 · PM10 · NO₂ · O₃ · CO — traffic and heating       not CO₂ (a room number, not a street number)
  industry, a plant, a road   the known hazard — SO₂, H₂S, NH₃, VOC, PM               not a general panel
```

**CO is the one gas the optics do not buy cheaply.** It absorbs weakly and sits at tenths of a ppm
in clean air, so a cheap NDIR path resolves smoke and not the background; ambient CO by optics is an
analyser at thousands of dollars, and the standard method is the cell. A forest watching for fire
takes NDIR; a town watching traffic takes the cell.

**Detection, not metrology.** The station's product is a change against the site's own running
background, timed on the station clock; absolute ambient chemistry to agency grade is the agency's
fixed stations'.

## The classes — what each principle is, gives, and dies of

The figures are the class's, read across the makers' sheets; a unit is checked against them on its
own sheet before it is bought.

| principle | what it measures and how | what it gives | range · resolution | response | what wears | operating window |
|---|---|---|---|---|---|---|
| **electrochemical cell** | an amperometric cell: the gas diffuses through a membrane to a working electrode and is oxidised or reduced; the current is proportional to concentration. **One cell, one gas** | **an absolute concentration of that gas**, ppb to ppm | CO 0–100 or 0–1000 ppm at 0,1–0,5 ppm · NO₂, O₃, SO₂ 0–20 ppm at ~20 ppb · H₂S 0–100 ppm at 0,1–1 ppm · NH₃ 0–100 ppm at 0,5–1 ppm | t₉₀ 15–60 s; NH₃ to 150 s | **the cell** — its electrolyte dries and its electrode consumes the target; > 2 years in air on the sheet, ½–2 outdoors | −20 … +50 °C (the better cells −30 … +55); 15–95 % RH, non-condensing |
| **NDIR** | infrared absorption on the gas's own band across a sealed optical path, against a reference channel | **CO₂ in ppm, measured**; CH₄ from tens of ppm | CO₂ 400–5000 ppm at ±(30–50 ppm + 3 %) · CH₄ from ~50 ppm, or 0–100 % LEL | ~30 s | **nothing** — a lamp rated for years, no moving part, no consumable | −10 … +50 °C on most; some sheets to −40 |
| **laser scattering** | particles cross a laser beam in a fan-drawn chamber; a photodiode counts the scattered pulses and sizes them by intensity | PM1 · PM2.5 · PM4 · PM10 in µg/m³ from 0,3 µm, and on some units number concentrations in bins from 0,5 to 10 µm | ±(5–15 µg/m³ + 5–15 %) at 25 °C and 50 % RH on the fine fraction; the coarse fraction ±25 % | 1 s, averaged by the unit | **the fan**, and the open chamber, which fouls with the dust it counts, condenses in fog and takes insects; the sheet's 8–10 years are a bench figure, outdoors a season between cleanings | −10 … +60 °C; 0–95 % RH, non-condensing — and over-reads above ~60 % RH (below) |
| **PID** | a 10,6 eV ultraviolet lamp ionises the organic molecules in the gas; the ion current is the total | **a measured VOC total**, calibrated in isobutylene equivalents; a response factor per compound, never one compound out of a mix | 1 ppb – 40 ppm on the ppb cell, 0,5 – 10 000 ppm on the ppm cell | seconds | **the lamp window**, 10 000–15 000 h, cleaned and then replaced; the electrode stack with it | −20 … +50 °C; 0–99 % RH, non-condensing, on the humidity-resistant cells |
| **MOX** | a heated metal-oxide element whose resistance moves with reducing gases | a **relative** VOC and NOx index, re-baselined by the unit against its own running background | an index, not a concentration | seconds | the element drifts; 1–2 years of usable index | −10 … +50 °C |

**What the classes do not do, and a buyer must know:**

- **A MOX "eCO₂" is not CO₂**: it is estimated from the VOC signal. Real ppm is NDIR, and nothing
  else is shipped as CO₂. **The MOX index is relative** and is used as that; an absolute VOC figure
  is the PID's, calibrated in a laboratory the station does not carry.
- **An ozone cell reads NO₂ too.** Every low-cost O₃ cell responds to NO₂ nearly one for one; a unit
  sold as ozone is an **oxidant** reading (O₃ + NO₂) unless it carries an NO₂ cell with an ozone
  filter beside it and subtracts. A town fits the pair, or ships the sum as what it is.
- **A biased cell wants continuous power.** NO₂ and O₃ cells run under a bias voltage and settle for
  minutes to hours after it is applied, and their baseline wanders by tens of ppb over days. **No
  gating between reads on an electrochemical unit**; the arm's power board leaves it on. CO₂ by
  NDIR and a PID warm up in a minute and may be gated; a particle counter runs its fan for ~30 s
  before a reading and may be gated at that cost.
- **A cell's zero moves with temperature**, by ppb per kelvin on the toxic-gas cells; a unit that
  does not carry a temperature compensation on its sheet reads the weather.
- **NDIR CO₂ assumes clean air once a week.** Its automatic baseline correction pins the lowest
  reading of the last days to ~400 ppm; outdoors that is true and the correction holds, in a
  closed shelter it is not and the correction is switched off on the unit.
- **NDIR methane does not see the ambient 2 ppm**: its floor is tens of ppm, which is why it is a
  livestock, manure and biogas instrument and not a background one.

## Particulates — the humidity, the fan, the asymmetry

- **The accuracy is asymmetric**: PM1 and PM2.5 are counted, PM4 and PM10 inferred from the
  distribution's tail — a property of the class, not of a unit.
- **Damp air over-reads.** Hygroscopic particles swell above ~60 % RH, so fog reads as smog. The
  correction is a κ-Köhler growth factor from the station's own RH, made downstream, never on the
  node; without it PM in humid air means nothing. A unit with a **heated inlet holding the sample
  under 60 % RH** measures dry mass directly and is the one worth paying for at a humid site.
- **The fan is a moving part**: it clogs with dust, takes moisture and wears. Buy a unit with an
  automatic fan-clean cycle. The module is a consumable — replaced, never recalibrated.

## Temperature, humidity, the inlet

- **Temperature is the hard one.** Nearly every air unit runs ~0–50 °C against the station's
  −40/+60 °C; in winter an outdoor unit condenses, its fan freezes and its sensor leaves its range.
  **The air set is a warm-season or town fit.** A heated sensor chamber that holds the units above
  0 °C is a site's cold-climate option and burns power continuously; it is never the base. **Buy on
  the operating temperature, not on the IP rating** — they are two figures.
- **Humidity**: the PM correction downstream; condensation kept off the optics and the cells.
- **The inlet** lets outdoor air reach the units and keeps rain, spray, sun and insects out; it is
  the real mechanical design of an air set and it is `CONSTRUCTION.md` — an open-bottomed white
  chimney with a cap, a coarse mesh and a baffle, the membranes the units' own.

## What every unit must meet

| | requirement |
|---|---|
| interface | **RS-485, Modbus RTU**, 9 600 or 19 200 Bd, 8N1, a settable slave address — the arm's one rate for every unit on it |
| supply | **10–30 V DC** off the arm's 12 V, the running and the start-up current on the sheet |
| output | the quantity **in a stated unit and scale** on the sheet — ppm, ppb, µg/m³, or a raw count with its factor; a unit whose sheet gives no scale is not bought |
| the figures | **range, resolution, t₉₀, operating temperature and humidity, and the consumable's life on the sheet**, each inside the class's row above |
| calibration | the cell or the optics **factory-calibrated, with the date**; a cell older than six months on the shelf has spent part of its life |
| housing | outdoor, the inlet as above; the IP figure is the housing's and says nothing about the operating window |
| documents | the **register map and the sheet with the unit** — they are the roster entry, and no unit is enrolled without them |

## What reaches the archive

**An absolute value where the unit measures one** — CO₂ in ppm, PM in µg/m³, a cell's ppb, a
PID's isobutylene-equivalent ppb — **and a relative index where it does not**, the MOX VOC and
NOx. Each unit's reply rides Palatine's payload as its own block, in the shape the unit returned it
(`../FIRMWARE.md` §7).

★ N.I.C. ★

# Palatine — mode B, the WMO tables

> **Design-stage concept — nothing built.** The figures are verified against the parts' sheets and
> the parts before a board is made.

**Palatine runs in one of two modes, set by its `MODE` register.** **Mode A is the base**: it ships
every sensor's reply as a raw block and computes nothing (`README.md`, `FIRMWARE.md`). **Mode B**
does what an automatic weather station's logger does: it samples its sensors, computes the WMO
statistics over fixed intervals, and ships each interval as one row of a table — a minute, ten
minutes, an hour, a day. A station that wants raw values runs A; one that is to stand beside a
national network, or send a day's weather over a thin link, runs B. Nothing else in the station
changes between the two.

## The rows are blocks

**A table row is a ModBus block from a module address ModBus never uses.** Addresses 248–255 are
reserved by ModBus; Palatine takes four of them for its tables:

| address | table | a row closes | carries |
|---|---|---|---|
| `0xF8` | `MIN` | every minute | the minute's means, its gust, its sums |
| `0xF9` | `TEN` | every 10 minutes | the ten-minute means, extremes, gust and sums |
| `0xFA` | `HOUR` | every hour | the values at the hour, the hour's extremes and sums |
| `0xFB` | `DAY` | at `DAY_END` | the day's extremes, means and sums |

**A row is `address · 30 · fifteen registers`, big-endian, exactly 32 B — one payload.** It is a
block like any other, so the head stores it, the profile decodes it and the reader reads it with
the machinery a bought sensor already uses: **each table is a profile**, fifteen registers with a
quantity, a scale and a name, held in the head's mirror and shipped with the archive. **Every
register is a signed 16-bit value and `0x8000` is absent** — a quantity the station has no sensor
for, or a statistic with too few samples behind it; nothing else marks a gap.

**A row goes up once**, in the first slot after its interval closes — rows that close together, a
minute, its ten minutes and its hour, in consecutive slots, the shortest table first — and belongs to **the last
closed interval of its table** at the frame's second; a reader needs no time inside the row.
**The blocks of the sensors the tables consume are not shipped in mode B**; every other roster
entry — Ceres, Sakura, Babel's positions, Chinook's air units — ships its block as in mode A.

## The tables

Units: temperature 0,1 °C · humidity 0,1 % · pressure 0,1 hPa, at the station · wind 0,1 m/s ·
direction 1°, **1–360 with 360 north and 0 calm** — WMO's convention: a speed at or under 0,2 m/s is
calm and its direction is 0 · precipitation 0,1 mm · irradiance 1 W/m² · radiant exposure 1 kJ/m² an hour,
10 kJ/m² a day · UV index 0,1 · snow depth 1 mm.

**`MIN` — the minute**

| # | quantity | statistic |
|---|---|---|
| 0 | air temperature, 2 m | mean |
| 1 | relative humidity, 2 m | mean |
| 2 | pressure | mean |
| 3 | ground temperature, 5 cm | mean |
| 4 | wind speed | mean — a one-minute wind is, in WMO's words, a long gust; the wind is the ten-minute value |
| 5 | wind direction | vector mean |
| 6 | gust | the largest 3 s running mean |
| 7 | precipitation | sum |
| 8 | global irradiance | mean |
| 9 | UV index | mean |
| 10 | snow depth | the latest |
| 11–14 | — | absent |

**`TEN` — ten minutes, closing at :00, :10 … :50**

| # | quantity | statistic |
|---|---|---|
| 0 | air temperature | mean |
| 1 | air temperature | the lowest minute mean |
| 2 | air temperature | the highest minute mean |
| 3 | relative humidity | mean |
| 4 | pressure | mean |
| 5 | ground temperature | mean |
| 6 | wind speed | mean |
| 7 | wind direction | vector mean |
| 8 | gust | the largest 3 s running mean |
| 9 | gust direction | the vector mean over that 3 s |
| 10 | precipitation | sum |
| 11 | global irradiance | mean |
| 12 | UV index | mean |
| 13 | wind speed | standard deviation over the ten minutes |
| 14 | wind direction | standard deviation over the ten minutes, each sample first brought within ±180° of its predecessor |

**`HOUR` — the hour, closing at :00**

| # | quantity | statistic |
|---|---|---|
| 0 | air temperature | the last minute's mean — the value at the hour |
| 1 | air temperature | the highest minute mean of the hour |
| 2 | air temperature | the lowest minute mean of the hour |
| 3 | relative humidity | the last minute's mean |
| 4 | pressure | the last minute's mean |
| 5 | ground temperature | the lowest minute mean of the hour |
| 6 | wind speed | the mean of the last ten minutes |
| 7 | wind direction | the vector mean of the last ten minutes |
| 8 | gust | the largest 3 s running mean of the hour |
| 9 | gust direction | the vector mean over that 3 s |
| 10 | precipitation | sum |
| 11 | global radiant exposure | sum, kJ/m² |
| 12 | UV index | the highest minute mean |
| 13 | snow depth | the latest |
| 14 | — | absent |

**`DAY` — the day, closing at `DAY_END`**

| # | quantity | statistic |
|---|---|---|
| 0 | air temperature | the highest minute mean |
| 1 | air temperature | the lowest minute mean |
| 2 | air temperature | the mean of the minute means |
| 3 | relative humidity | mean |
| 4 | relative humidity | the lowest minute mean |
| 5 | pressure | mean |
| 6 | ground temperature | the lowest minute mean — the ground frost |
| 7 | wind speed | mean |
| 8 | gust | the largest 3 s running mean of the day |
| 9 | gust direction | the vector mean over that 3 s |
| 10 | precipitation | sum |
| 11 | global radiant exposure | sum, 10 kJ/m² |
| 12 | UV index | the highest minute mean |
| 13 | snow depth | the latest at `SNOW_HOUR` |
| 14 | — | absent |

**What the tables leave to the reader**, because it needs nothing but the rows: the dew point from
temperature and humidity, the pressure reduced to sea level from the station's elevation, the
three-hour pressure tendency from three `HOUR` rows, the sunshine duration from the `MIN`
irradiance by whatever pyranometric method the reader takes, the snow's water equivalent, and the
period classification of `CLIMATE.md` from the `DAY` rows.

## How the statistics are made

**The WMO conventions** (WMO-No. 8, Vol. I, Annex 1.A and chapter 5; Vol. V, chapters 2 and 3):
temperature, humidity and pressure are reported as one-minute means; the wind as a ten-minute mean
with its direction as a vector mean — the components averaged, Vol. V §3.6 — and **the standard
deviations of speed and direction over the same ten minutes**, which CIMO recommends beside the
mean and the gust; the gust as the largest 3 s running mean in the interval, **sampled at 4 Hz and
averaged over the last twelve samples, overlapping, every sample** — the design chapter 5 §5.8.3
gives as its first system; precipitation and radiation as sums. **Every longer statistic is built
from the minute** — a ten-minute temperature is the mean of its ten minute means, an hour's extreme
is the extreme of its sixty — except the gust and the wind's standard deviations, which are always
computed over the raw wind samples.

**The sampling.** The roster stays the roster; mode B gives each quantity its interval:

| quantity | read every |
|---|---|
| air temperature and humidity, pressure, ground temperature | 5 s |
| global irradiance, UV | 5 s |
| wind speed and direction | **0,25 s** — four a second, what a 3 s running mean needs; a unit that holds its own 3 s gust in a register is read every 5 s and its gust register taken instead |
| precipitation — Pluvius's `RATE` | 10 s, as in mode A |
| snow depth | every 600 s **six reads over a minute, the median** — WMO asks a 1 cm result averaged over a minute, and one radar echo is not that |

**The wind's 0,25 s is the one interval below a second**, and only a wind entry takes it: four
reads a second on its arm, ~100 ms of the arm's second at 19 200, inside the schedule's 800 ms cell.

**Why these intervals** (Vol. V §2.4.2): samples for an average are taken no further apart than the
sensor's time constant, and samples for an extreme four times as often. Annex 1.A gives the
constants — temperature 20 s, humidity 40 s, radiation 20 s, snow depth under 10 s — so 5 s holds
for all of them. **Pressure is the one exception**: its constant is 2 s and the read is 5 s, held
under the annex's third criterion — twelve samples a minute on a quantity whose minute-to-minute
change is far under the 0,1 hPa reported — and the wind pumping WMO warns of is the shield's job,
not the sampler's (`SITING.md`).

**The rules of a statistic:**

- **A statistic with fewer than half its samples is absent.** It covers a sensor that stopped
  answering, a missed poll run, and the first rows after a boot, whose intervals started before
  the board did — the sums in progress are lost with a reset and the row says so by being absent.
- **A vector mean** is the direction of the mean of the wind vectors, speed times the unit vector
  of the direction, sample by sample.
- **A precipitation sum** is the rise of Pluvius's `RATE` over the interval; a fall is a Pluvius
  boundary, and the reading after it counts from zero.
- **The intervals are UTC.** Minutes, ten-minutes and hours close on UTC seconds divisible by 60,
  600 and 3600; the day at `DAY_END`, a UTC hour.

## What mode B adds to the board

**The time.** Palatine counts frames from `SYNC` and holds `unix.0`; a UTC interval needs the whole
second. In mode B the head writes **`UNIX`** — the UTC second of a named frame: the station's second, which is
GPS time and leap-blind, less the GPS − UTC offset Kronos holds (`../core/PROTOCOL.md` §4) — after
`SYNC`, once a day and after a leap second, and Palatine counts on from it.

**The roles.** Each roster entry that feeds a table carries a **role** — which quantity it is, which
register of its block, its sign, and the scale that turns the sensor's raw register into the
table's unit (`physical = (raw + add) × mantissa × 10^exp10`, the profile's own form). The head
writes the role from the profile it already holds. An entry with no role ships its block as in
mode A. **Mode B converts units and mode A never does** — that is the whole difference in what the
board computes.

**The registers**, beside `FIRMWARE.md` §8:

| register | r/w | what |
|---|---|---|
| `0x0018 MODE` | r/w | 0 mode A · 1 mode B; a change marks the frame `status` 5 CHANGED |
| `0x0019 UNIX` | w | the UTC Unix second of the frame named in the value — the station's second less the GPS − UTC offset — `kind` 5 |
| `0x001A ROLE` | r/w | per roster index: role · register in the block · signed · mantissa · exp10 · add, `kind` 5 |
| `0x001B DAY_END` | r/w | the UTC hour the day closes at, default 0; **set at commissioning to the UTC hour nearest local solar midnight** — WMO refers radiation totals to local apparent time (Vol. I, §7.1.3.4), and the day's radiant exposure is this row's |
| `0x001C SNOW_HOUR` | r/w | the UTC hour of the day's snow depth, default 6 |

**The memory**: per quantity the minute's running sums and extremes, the ten-minute and hour sums
built from the minute values, the day's, the wind's last twelve samples for the 3 s running mean and
its ten-minute sums of squares for the standard deviations — a few kilobytes of the 272 KB. The CPU adds a few hundred additions a second.

## In the archive and on the link

**The head stores each table as a series of its own**, address `6n` and the table's address, under
the default rule `CHANGE` per block, which keeps every row once (`../core/archive/HMC.md`). The
other blocks of a mode-B Palatine go into its ordinary series as in mode A.

**The `HOUR` and `DAY` rows are what a thin link carries.** A station with no Wi-Fi sends its live
packet over the head's modem once an hour — the `HOUR` row, and the `DAY` row once a day — a 32 B
block each, enough for the day's temperatures and precipitation beside the national network's
stations; its cards hold everything else until they are read.

**The `HOUR` and `TEN` rows are the content of the WMO exchange templates** — BUFR 307096 for hourly
surface data and 307092 for ten-minute data; the encoding into BUFR is the server's, from the
archive.

★ N.I.C. ★

# Palatine — describing a period against the normal

> **Design-stage concept — nothing built.** The figures are verified against the parts' sheets and
> the parts before a board is made.

What a finished month, half-year or year is called against the climate of the place — the
vocabulary a period of Palatine's data is labelled with; a mode-B Palatine's `DAY` rows are the
input (`WMO.md`). The boundary tables are configuration;
nothing here is a firmware constant.

## Describing a period — the WMO classification

**A finished month, half-year or year is classified against the normal, and the vocabulary is
fixed.** WMO adopted the recommendation in **1983** to stop "above normal" and "below normal"
being used by opinion; ČHMÚ issued the Czech methodology from it in **1987**. Source:
Kožnarová V., Klabzuba J., *Doporučení WMO pro popis meteorologických, resp. klimatologických
podmínek definovaného období*, Rostlinná výroba 48 (2002) 4: 190–192.

**Two definitions that are not interchangeable:**

- **Standard climatological long-term average** — computed from **at least three complete,
  consecutive, finished decades**. Any series may be used, but **the period must always be
  stated** with the number.
- **Standard climatological normal** — the special case: the mean of the **last completed
  30-year period, the thirty-year blocks counted from 1. 1. 1901**. **The current normal is
  1991–2020**; the next becomes 2021–2050. **Calling a long-term average from any other period
  "the normal" is not permitted** — the convention exists to eliminate multi-year oscillation
  and long-period aperiodic change, and it only does that if everyone uses the same block.

**Seven classes, defined by probability — this is the part that carries anywhere:**

| class | exceedance probability | recurrence |
|---|---|---|
| extraordinary above normal | **< 2 %** | < 50 years |
| very above normal | 2,0–9,9 % | < 10 years |
| above normal | 10,0–24,9 % | < 4 years |
| **normal** | **25,0–75,0 %** | > 2 years |
| below normal | 75,1–90,0 % | < 4 years |
| very below normal | 90,1–98,0 % | < 10 years |
| extraordinary below normal | **> 98 %** | < 50 years |

For temperature the same classes are named *extraordinary warm … extraordinary cold*; for
precipitation *extraordinary wet … extraordinary dry*.

**The classes travel; the numbers do not.** The seven probability bands above are the WMO
recommendation and are the same everywhere. **The boundary values that map a measurement into
them are per country** — they are a national meteorological service's fit of those probabilities
to its own climate, and a station takes the tables of the country it stands in. Anything
published with a class attached should say which country's tables produced it, the same way a
long-term average has to state its period.

**How a value reaches a class depends on the element's sign range, and there are only two rules:**

- **An element that can go negative** — air temperature — is classified by its **deviation from
  the normal**, in kelvins, and plotted as a line.
- **An element that is zero or positive** — precipitation total, sunshine duration — is
  classified by its **percentage of the normal**, and plotted as bars.

**The periods are month, half-year and year.** The half-years are **IV–IX** and **X–III**, not
calendar halves.

**"Extreme" is not one of the classes and must not be used as one.** *Extremely dry*,
*extremely above normal* and the like are explicitly discouraged: an extreme is the highest or
lowest value in a data set, a different statistic kept for a different purpose.

### The boundary tables — Czech, and only the aggregate rows are trustworthy as printed

The probability table above is the recommendation. The **number** tables are ČHMÚ's fit to Czech
data and are valid **for the Czech Republic only** — a station elsewhere takes its own country's
tables, fitted to the same seven probabilities. Nothing in the firmware should carry these
numbers as constants.

| period | **temperature**, K from the normal | **precipitation**, % of the normal |
|---|---|---|
| **IV–IX** | < −2,5 · −2,5…−1,6 · −1,5…−1,1 · **−1,0…1,0** · 1,1…1,5 · 1,6…2,5 · > 2,5 | < 30 · 30…59 · 60…69 · **70…120** · 121…150 · 151…180 · > 180 |
| **X–III** | < −1,5 · −1,5…−1,1 · −1,0…−0,6 · **−0,5…0,5** · 0,6…1,0 · 1,1…1,5 · > 1,5 | < 35 · 35…59 · 60…79 · **80…120** · 121…130 · 131…160 · > 160 |
| **I–XII** | < −1,2 · −1,2…−0,8 · −0,7…−0,6 · **−0,5…0,5** · 0,6…1,2 · 1,3…1,5 · > 1,5 | < 60 · 60…79 · 80…89 · **90…110** · 111…130 · 131…140 · > 140 |

**The monthly temperature rows are carried — they are contiguous as printed**, the one slip in
January (two classes meeting at 3,5) read as 3,6:

| month | K from the normal: extraordinary cold · very cold · cold · **normal** · warm · very warm · extraordinary warm |
|---|---|
| I | < −8,5 · −8,5…−4,6 · −4,5…−2,1 · **−2,0…2,0** · 2,1…3,5 · 3,6…5,0 · > 5,0 |
| II | < −9,5 · −9,5…−5,6 · −5,5…−1,1 · **−1,0…2,5** · 2,6…3,0 · 3,1…4,0 · > 4,0 |
| III | < −4,0 · −4,0…−3,6 · −3,5…−2,1 · **−2,0…2,0** · 2,1…3,0 · 3,1…4,0 · > 4,0 |
| IV | < −3,5 · −3,5…−3,1 · −3,0…−1,6 · **−1,5…1,5** · 1,6…2,5 · 2,6…3,5 · > 3,5 |
| V | < −3,5 · −3,5…−2,6 · −2,5…−1,6 · **−1,5…1,5** · 1,6…2,5 · 2,6…3,5 · > 3,5 |
| VI | < −2,5 · −2,5…−2,1 · −2,0…−1,1 · **−1,0…1,0** · 1,1…2,0 · 2,1…2,5 · > 2,5 |
| VII | < −2,5 · −2,5…−1,6 · −1,5…−0,6 · **−0,5…1,0** · 1,1…1,5 · 1,6…2,5 · > 2,5 |
| VIII | < −3,0 · −3,0…−1,6 · −1,5…−0,6 · **−0,5…1,0** · 1,1…1,5 · 1,6…2,5 · > 2,5 |
| IX | < −4,0 · −4,0…−2,6 · −2,5…−1,1 · **−1,0…1,0** · 1,1…2,0 · 2,1…3,5 · > 3,5 |
| X | < −3,0 · −3,0…−2,6 · −2,5…−1,1 · **−1,0…1,0** · 1,1…2,0 · 2,1…2,5 · > 2,5 |
| XI | < −3,5 · −3,5…−2,1 · −2,0…−1,1 · **−1,0…1,0** · 1,1…1,5 · 1,6…3,0 · > 3,0 |
| XII | < −5,0 · −5,0…−4,1 · −4,0…−1,6 · **−1,5…1,5** · 1,6…2,5 · 2,6…5,0 · > 5,0 |

**The Czech daily mean is a term mean, and the tables stand on it.** ČHMÚ computes the daily mean
air temperature for climatology from the dry-bulb readings at the three climatological terms as
**(T07 + T14 + 2·T21) / 4**, the terms at **07, 14 and 21 h local mean solar time** — the
station's longitude corrected from the 15° E meridian at 4 minutes a degree, one hour later under
summer time (ČHMÚ Metodický předpis č. 13, *Návod pro pozorovatele meteorologických stanic*, 2003,
§1.2 and §6.1.1). The Czech normals and the boundary tables above were fitted to months of that
mean, and a month's deviation is only comparable to them when it is formed the same way: the
server takes the three minute values at the station's own term instants from the archived `MIN`
rows and forms the term mean, not the day's 24-hour mean of the `DAY` row — the two differ
systematically, and the X–III classes are 0,5 K wide.

**The monthly precipitation rows are carried, read for contiguity.** As printed (Kožnarová and
Klabzuba 2002, Tab. 3) six months overlap or leave a gap, each by one misprinted cell; the cell
is read so that the classes meet, and where two readings would, the one the gamma shape of the
row's own 2 % and 98 % boundaries supports (*The monthly boundaries*, below): IV and XII *wet*
from 131, not 141 · VIII *dry* 40…69, not 30…49 · IX *dry* 30…49, not 20…39 · X *dry* 20…39,
not 40…59 · XI *extraordinary dry* < 20, not < 10 (`WHY.md`).

| month | % of the normal: extraordinary dry · very dry · dry · **normal** · wet · very wet · extraordinary wet |
|---|---|
| I | < 30 · 30…49 · 50…69 · **70…120** · 121…160 · 161…230 · > 230 |
| II | < 10 · 10…29 · 30…59 · **60…140** · 141…180 · 181…240 · > 240 |
| III | < 20 · 20…29 · 30…49 · **50…140** · 141…220 · 221…270 · > 270 |
| IV | < 20 · 20…39 · 40…59 · **60…130** · 131…160 · 161…260 · > 260 |
| V | < 20 · 20…49 · 50…59 · **60…130** · 131…180 · 181…230 · > 230 |
| VI | < 20 · 20…49 · 50…69 · **70…120** · 121…170 · 171…210 · > 210 |
| VII | < 20 · 20…39 · 40…59 · **60…130** · 131…170 · 171…230 · > 230 |
| VIII | < 20 · 20…39 · 40…69 · **70…130** · 131…180 · 181…220 · > 220 |
| IX | < 10 · 10…29 · 30…49 · **50…140** · 141…210 · 211…270 · > 270 |
| X | < 10 · 10…19 · 20…39 · **40…140** · 141…210 · 211…280 · > 280 |
| XI | < 20 · 20…39 · 40…59 · **60…130** · 131…180 · 181…250 · > 250 |
| XII | < 20 · 20…39 · 40…59 · **60…130** · 131…180 · 181…250 · > 250 |

### The monthly boundaries — computed per place where a country publishes none

**A national table is a service's fit of the seven probabilities to its climate; where there is
none, the station makes the same fit from the same data it already needs.** A percentage of the
normal presupposes the place's monthly normals, so the thirty monthly totals of the normal period
are in hand for every month, and the boundaries follow from them:

1. **The series**: the twelve months' totals over the **last completed normal period, 1991–2020**,
   thirty values a month — the place's own record where it has them, else the national service's
   gridded series for the cell the station stands in; the source and the period are stated with
   the table, as a normal's are.
2. **The distribution**: a two-parameter gamma, the one WMO fits to monthly precipitation for the
   Standardized Precipitation Index (WMO-No. 1090, *Standardized Precipitation Index User Guide*,
   2012), with Thom's estimator: `A = ln x̄ − (Σ ln x)/n`, `k = (1 + √(1 + 4A/3)) / 4A`,
   `θ = x̄/k`. A month with zero totals in the series takes the SPI's mixed form,
   `H(x) = q + (1 − q)·G(x)`, `q` the fraction of zero months.
3. **The boundaries**: the quantiles at **2 · 10 · 25 · 75 · 90 · 98 %** divided by the mean,
   rounded to whole percent, the classes written contiguous as ČHMÚ writes them — `< b₁ · b₁…b₂−1
   · … · > b₆`.
4. **The half-years and the year** the same way, on the half-year and annual totals.

**The shape is the national tables' shape, which is why the computed table is trusted.** Fitted
to the three Czech aggregate rows above, a gamma reproduces them to within their rounding:

| period | ČHMÚ's row | gamma, fitted | CV | rms |
|---|---|---|---|---|
| IV–IX | 30 · 60 · 70 · 120 · 150 · 180 | 41 · 58 · 74 · 121 · 147 · 186 | 0,36 | 6 % |
| X–III | 35 · 60 · 80 · 120 · 130 · 160 | 50 · 66 · 80 · 117 · 138 · 167 | 0,29 | 8 % |
| I–XII | 60 · 80 · 90 · 110 · 130 · 140 | 65 · 77 · 87 · 112 · 125 · 143 | 0,19 | 4 % |

A month's boundaries are a function of its coefficient of variation alone, and a month scatters
far more than a half-year — in central Europe a station's month runs at CV 0,4–0,7 — which is
why the monthly rows are wider than the aggregate rows and cannot be read off them:

| CV | 2 % | 10 % | 25 % | 75 % | 90 % | 98 % |
|---|---|---|---|---|---|---|
| 0,15 | 72 | 81 | 90 | 110 | 120 | 133 |
| 0,20 | 63 | 75 | 86 | 113 | 126 | 145 |
| 0,30 | 48 | 64 | 78 | 118 | 140 | 171 |
| 0,40 | 36 | 53 | 71 | 123 | 153 | 198 |
| 0,50 | 25 | 44 | 63 | 128 | 167 | 227 |
| 0,60 | 17 | 35 | 56 | 131 | 180 | 258 |
| 0,70 | 11 | 27 | 49 | 134 | 194 | 289 |

ČHMÚ's monthly rows above sit where the gamma puts CV 0,45–0,7 — the normal band 70…130 in
August at ~0,5, 40…140 in October at ~0,7 — so a computed month lands where a national one does.
**Where a national monthly table exists it wins**, being the country's published word, and a
class is reported with the table that made it.

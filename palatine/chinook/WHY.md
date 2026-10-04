★ N.I.C. ★

# Chinook — the graveyard

Rejected alternatives and superseded states, with the reason. Nothing returns from here and
nothing is deleted from it.

## A default loadout of CO₂ + laser PM + VOC/NOx index — superseded

`SENSORS.md` locked NDIR CO₂, a laser particle counter and a MOX VOC/NOx index as durable basics.
**Two are not**: the counter's fan and optics foul outdoors, the MOX drifts; and open-country CO₂
barely varies by site. The default went to one base channel, then to none.

## CO by NDIR as the one base channel — superseded; the base is empty

`README.md` then made CO by NDIR the single base channel: sealed optics, a combustion marker
co-timed with a strike on Tesla. That suits a forest, not a base. **No air-chemistry quantity is
wanted everywhere** — a field wants dust, a city the traffic quartet — and **every such unit is a
consumable**: a cell lasts half a year to two, a MOX element a year or two, a counter a season
between cleanings, at about a thousand crowns a block; a hundred stations make a standing budget and
a maintenance round. **A station potted and left for years may carry nothing on its base that needs
a hand every season.** The site chooses from a menu by setting.

## CO by NDIR as the ambient method — superseded

The menu named NDIR for CO. **CO absorbs weakly and sits at tenths of a ppm in clean air**: cheap
NDIR resolves smoke, not the background; ambient CO by optics is an analyser at thousands of
dollars. NDIR stays for fire; ambient CO is the electrochemical cell.

## Chinook at the repository root — superseded

It stood there while meant to be a board. **It is neither board nor unit** — bought Modbus units on
Palatine's arms — so it is `palatine/chinook/`.

## The units named by product — superseded by the sensing principle

`SENSORS.md` named classes — `IOT-S300AQ` / `SEM227`, `SenseCAP SOLO CO2 5000` / `SEN0659`,
`SPS30` — and owed their register maps per setting. **Every market houses the same element
differently**, and the map is the housing's, entered in the arm's roster at commissioning. Replaced
by the principle — amperometric cell, NDIR, laser scattering, PID, MOX — and what every unit must
meet. The PM humidity-growth correction is made downstream from the station's RH, never on the
station.

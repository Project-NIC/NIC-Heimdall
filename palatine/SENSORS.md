★ N.I.C. ★

# Palatine — the sensors on the arms

> **Design-stage concept — nothing built.** The figures are verified against the parts' sheets and
> the parts before a board is made.

**Every sensor of Palatine's is a ModBus RTU slave on one of its arms** — Palatine has no other
input. **Palatine is the weather; what the air is made of is Chinook's** (`chinook/`), on the same
arms. Most units are bought; the house MODs are **Pluvius** (the weighing rain gauge), **Ceres**
(soil moisture and temperature), **Sakura** (leaf wetness) and **Babel** (a sensor not sold as
ModBus, converted at the sensor), built because the market does not sell them in a form that lasts
in the dirt. Every unit hands over a finished, compensated value, so Palatine's logic is one and
the same for every sensor.

**Two builds, and the types are examples.** A bought sensor is named by a recommended type and an
approximate price: what counts is the row it must meet, and a builder buys what the country sells.
A sensor not sold as an RS-485 ModBus unit goes through **Babel** (`../babel/`), never onto
another bus.

```
UNIVERSAL — any station, town or field, alongside a national network
  air temperature + humidity   2 m, in a radiation shield       an SHT45 probe on RS-485, ~50 USD
  pressure                     at the station                   an RS-485 barometer, 300–1100 hPa, ±1 hPa
    — or the three in one:     2 m, in the shield               a T/RH/P unit, S-THP-01A class, ~66 GBP
  ground temperature           5 cm above the grass, no shield  an SHT45 probe on RS-485, ~50 USD —
                                                                the night minimum counts, the day is sun
  wind speed + direction       2 m                              cups and vane in one, ~70 USD,
                                                                or a small 2D ultrasonic, ~100–130 USD
  global radiation             levelled, open horizon           a silicon pyranometer, PYR20 class, ~60 USD
  UV                           beside the pyranometer           a UV-index unit, 290–390 nm, ~60 USD
  precipitation                beside the station               Pluvius, ~1000 USD built
  snow depth                   on the mast, looking down        an 80 GHz radar level sensor (below)
  soil moisture + temperature  −10 and −50 cm                   Ceres ×2, ~30–40 USD of parts each

FOR FARMERS — the universal station, and
  the soil profile             −10 · −20 · −50 · −100 cm        Ceres ×4 in one patrona, ~30–40 USD each:
                                                                where the water is along the roots, how deep
                                                                a rain soaks, when the subsoil dries
  another field                where the soil changes           another patrona of four
  leaf wetness                 at the top of the canopy         Sakura, ~30–40 USD of parts — vineyard,
                                                                orchard, potatoes, cereals, sunflower;
                                                                raised with the crop
```

**Two SHT45s, at 2 m and at 5 cm, because humidity differs by height.** On a calm night the grass
cools to the dew point and the air over it saturates while 2 m stays drier, so the pair gives dew
and ground frost where they form, and in wind, when the two agree, each checks the other; with
Sakura at the canopy it is the humidity profile an orchard sprays against mould by.

**Every unit is RS-485 ModBus on the arm's 12 V and rated to −40 °C.** The house MODs' prices are
the parts, estimated. The depths are the standard series — 5, 10, 20, 50 and 100 cm, the national
networks' and the agricultural networks' alike; the soil type is recorded for the site, never
worked into the depths. The water balance of a field — rain in, evapotranspiration out — is
computed downstream from temperature, humidity, the 2 m wind and the radiation; it is not a sensor.

## Wind — the two principles

**Any RS-485 ModBus variant; the bus interface is the only hard requirement.** Pick the principle
by climate and budget:

| option | ~cost | supply | moving parts | in cold and snow | |
|---|---|---|---|---|---|
| mechanical cup and vane, **two units** | 60–70 USD | 12 V | bearings | can ice up and stall | the cheapest; **two** addresses |
| mechanical, **combined** | 70–80 USD | 12 V | bearings | ices up | one address, one mount |
| **ultrasonic 2D**, low-cost | 150–300 USD | usually 10–30 V | none | good; better heated, at more power | continuous transducer power |
| **ultrasonic, low-power** (Calypso, Gill, FT class) | 300–1500 EUR | 3,3–18 V | none | the best | the lowest power, the highest price |

A hot-wire anemometer (speed only, a heater) and Doppler or lidar wind (profiling) are not for this
station. **The default is a 12 V mechanical unit**; an ultrasonic is justified where icing or
access makes moving parts a liability.

## Pyranometer — the two classes

| option | spectrum, class | range | ~cost | |
|---|---|---|---|---|
| **silicon cell, RS-485** (PYR20 class) | 400–1100 nm, no ISO class | 0–2000 W/m² | 50–100 USD | 5–24 V; a spectral error from the silicon band |
| **thermopile, ISO 9060** (Hukseflux SR05, Apogee class) | the full solar spectrum | 0–2000 W/m² | hundreds of EUR | reference grade |

**The default is the silicon cell**; a thermopile where radiation accuracy is the site's purpose.

## Precipitation — Pluvius

**Palatine sees three registers of it: `RATE · HOUR · STATUS`**, the running total since the last
boundary with its `UNSETTLED` bit, the last complete hour, and the status word that carries the
drain's request, an 8 B block. The drain itself is the unit's own, and its commands and counts ride
the tunnel as registers, never the payload; the gauge, its cell and its drain are `../pluvius/`, its registers
`../pluvius/MODBUS.md`. Pluvius is built rather than bought because it owns its drain pump, and
each drain records a fresh zero that takes out the cell's drift and creep. Its pump's 24 V is
`PWR EXT` (`FIRMWARE.md` §6).

## Snow — an 80 GHz radar level sensor, bought

**Snow depth is a down-looking range from the mast**: the depth is the mast height less the range,
derived where the archive is read; Palatine ships the range and the unit's echo-quality register as
one block.

**What the unit must meet:** an 80 GHz FMCW radar level meter — **≤ 3° beam**, **±1 mm**, a
blanking zone under ~10 cm, **−40 °C operating**, IP67, 12–30 V, **RS-485 ModBus**, **rated for
solids**, because a snow surface is a rough, weakly reflecting solid and a solids unit's algorithm
expects that return. Any unit that meets it will do; it costs more than a −20 °C water-level unit
and is still the buy.

**Why radar.** The binding figure is the operating temperature, not the IP rating:

| principle | class | operating | in snow |
|---|---|---|---|
| ultrasonic | cheap RS-485 module | ~−20 °C | powder absorbs the ping; the speed of sound drifts with the air |
| laser ToF | cheap RS-485 module | −20…+70 °C | needs an optical surface; bright sun and blowing snow blind it |
| **80 GHz radar** | **low-cost FMCW level meter** | **−40 °C as standard** | reads the surface echo; dry powder is a weak return |
| snow-rated purpose-built | snow sensor | −45 / −40 °C | 50–100× the radar's price |

Snow and ice are nearly transparent at 80 GHz, so the beam reads the surface where a laser scatters
and a ping is absorbed, and no speed-of-sound correction exists to make. The 3° beam lights a small
spot clear of the mast and the vegetation; the dead zone is ~8 cm.

**Icing.** Dry snow and frost on the lens cost nothing; a wet film of ice or water attenuates and
detunes. The lens is the unit's own PTFE — no grease or coating, which collects dust and does not
stop rime. Where hard rime is the site's, a **heated-lens** variant: the lens only, a few watts,
thermostatic. A weak or absent echo is in the unit's quality register, and a reader holds the last
good depth through it. The shroud the radar is mounted in is `CONSTRUCTION.md`.

**Gating follows the part.** Whether the radar is cut between reads is its own business — some lose
their settling when unpowered and are never cut, some sleep on command, some pulse on their own.
Pick the part with that in mind; one that can be gated sits on a gated arm (`FIRMWARE.md` §6).

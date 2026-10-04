<p align="center">
  <img src="NIC-Pascal.svg" width="200"/>
</p>

★ N.I.C. ★

# Pascal — tlaková sonda

[English](README.md) · **Čeština** · [Русский](README.ru.md)

> **Koncept ve fázi návrhu — nic není postaveno.** Čísla se ověřují proti katalogovým listům a součástkám, než vznikne deska.

**Pascal čte ze dna moře vodní sloupec nad sebou: NodBus mini typ 3, mini-NOD na segmentu Argus.**
Je to tsunamiměr Tier 1 — tam, kde pobřežní lokalita nemůže udržet radarový stožár, měří Pascal
tutéž vlnu zespodu (`../atlantis/README.md`). Jeho adresa je ta vypočtená, `TYPE«4 | NUMBER`,
**0x31** pro jednotku 1; v každém rámci posílá 4 B tlaku, průměr přes periodu vlny, a jednou za
minutu své teploty a napájení, jak k němu dorazilo (`FIRMWARE.md`).

**Tělo je mořská sestava jednotky Gauss s jedním otvorem navíc**: trubka PVDF, lepená průchodka
dole, podélně vodotěsný kabel, plnění olejem nebo vazelínou, žádný vak (`../gauss/CONSTRUCTION.md`),
a skrz stěnu trubky hloubkoměr ve svém pouzdře `Bar30`, jehož gelová plocha je tou plochou ve vodě.
Výplň chrání elektroniku a do měření nikdy nevstupuje. Statický offset je neškodný — Pascal měří
*změnu*, nikdy absolutní hloubku; tečení a tepelný drift se odehrávají v hodinách, vlna v minutách
(`HARDWARE.md`, *What it measures*).

| | |
|---|---|
| MCU | `STM32H523VE`, LQFP100 — bez krystalu, jádro na HSI, které disciplinuje stupeň hodin segmentu (`HARDWARE.md`, *The rung disciplines the core*) |
| čidlo | **TE Connectivity `MS5837-30BA`**, objednací kód `MS583730BA01-50` — piezorezistivní tlakoměr 30 bar ≈ 300 m, I²C — **v pouzdře Blue Robotics `Bar30`**: průchodka se závitem M10, gelová plocha tlakoměru ve vodě, skrz stěnu trubky |
| rozlišení | **0,2 mbar** RMS při nejvyšším převzorkování ≈ 2 mm vody |
| absolutní přesnost, 0–40 °C | ±50 mbar (0–6 bar) · ±100 mbar (0–20 bar) · **±200 mbar (0–30 bar)** — v pracovní hloubce ~21 bar je součástka ve třídě ±2 m a měřenou veličinou je změna |
| teploměr | **`TMP117` na stěně trubky**, v oleji — teplota vody u dna, absolutně 0,1 °C, trend v mK; vlastní teplota tlakoměru zůstává jeho kompenzací |
| součástky | `STM32H523VE` · hloubkoměr · `TMP117` · `LMR43610` |
| sběrnice | **NodBus mini** — jeden datový konektor Galvani a jeden napájecí konektor, segment za kartou Argus; mini nese hodiny, a proto je sonda právě na ní |
| desky | napájecí a komunikační deska v tlakovém provedení rodiny — jen pevné součástky, žádná vzduchová dutina, zalité do oleje spolu se zbytkem; modul, který odpovídá trase (`../galvani/README.md`, *Pressure boards and land boards*) |

## Kde leží

**150–200 m vody**: dost hluboko, aby byla pod světem hladiny, a dost mělko, aby mohla ležet blízko
u strmého boku ostrova. Ve 200 m má tsunami zhruba **20 cm ≈ 20 mbar** (Greenův zákon z ~10 cm
v hlubokém oceánu) a šíří se rychlostí √(g·h) ≈ **44 m/s**. Sondy stojí v prstenci po 4, 8
nebo 12 kolem ostrova, stejně jako sondy Gauss — prstenec a jeho strop viz `../gauss/ARRAY.md`. Pouzdro je ukotvené k základu a jeho
kabel je k němu přichycený, protože pouzdro, které se zvedne o 10 cm, ohlásí 10 mbar, které se
nikdy nestaly (`CONSTRUCTION.md`).

**Hloubka utlumí větrové vlny a nechá dlouhé vlnobití**, a vlnobití má velikost samotné tsunami;
odděluje je frekvenční pásmo, sekundy proti minutám, takže Pascal vzorkuje v dávkách a průměruje
přes periodu vlny (`HARDWARE.md`, *What it measures*).

## Blokové schéma

```
   an ARGUS segment ══ the feed + the 2¹⁹ rung ══════▶ ┌───────────────────────────────────────────┐
   the family's pressure boards, feed and medium       │ PASCAL — the oil-filled tube, mini type 3 │
                                                       │ H523 · LMR43610 · the depth gauge through │  pressure, 4 B a frame · the temperatures
                                                       │ the wall · TMP117 on the wall             │  and the feed once a minute in the REPORT frame
                                                       └───────────────────────────────────────────┘
```

## Soubory

| soubor | obsah |
|---|---|
| [`HARDWARE.md`](HARDWARE.md) | deska — co měří a jak, napájecí sběrnice, hodiny a jejich disciplinace, každý pin, konektory, součástky, kritéria zkoušky na stole |
| [`CONSTRUCTION.md`](CONSTRUCTION.md) | co Pascal přidává k mořské sestavě jednotky Gauss — otvor a tělo čidla, základ, kabel |
| [`FIRMWARE.md`](FIRMWARE.md) | popis firmwaru — řetězec převodu, průměr přes periodu vlny, alarm, payload, registry, co je otestováno |
| [`WHY.md`](WHY.md) | hřbitov — co se zkusilo, co padlo a proč |

## Licence

Hardware: CERN-OHL-S v2 (`../LICENSE-HW`) · Software: MIT (`../LICENSE`) — Copyright (c) 2026 NIC — Native Intellect Community

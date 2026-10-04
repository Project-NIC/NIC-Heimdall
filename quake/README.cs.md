<p align="center">
  <img src="NIC-Quake.svg" width="200"/>
</p>

★ N.I.C. ★

# Quake — seismograf

[English](README.md) · **Čeština** · [Русский](README.ru.md)

> **Koncept ve fázi návrhu — nic není postaveno.** Čísla se ověřují proti katalogovým listům a součástkám, než vznikne uzel.

**Quake je seismograf stanice: NodBus typ 5, NOD.** Utěsněná trubice usazená v otvoru ve skále
nebo v zemině, ponechaná léta bez zásahu. Měří zrychlení půdy dvěma akcelerometry s různými
rozsahy, rotaci gyroskopem, náklon a drift inklinometrem a — podle osazení — magnetické pole
a teplotu půdy v hloubce. 128 vzorků za sekundu, 40 B na rámec, horizontace přímo na uzlu.

**MEMS, ne geofony.** Zkušební hmota MEMS rezonuje v oblasti kHz, vysoko nad seismickým pásmem
pod 20 Hz; součástka je utěsněná na jednom čipu, kalibrovaná ve výrobě, digitální, tři osy v jednom
pouzdře — jeden čip tam, kde sestava s geofony potřebuje tři cívky.

## Co vidí jeden uzel

**Přístroj pro silné pohyby a lokální události, ne observatorní seismometr.** Šumové dno MEMS —
~25 µg/√Hz u přesné součástky — leží výrazně nad šumem geofonu, takže jeden uzel zachytí:

- pocítěná místní zemětřesení, zhruba **M ≥ 3 do ~50 km**;
- větší regionální události, zhruba **M ≥ 4,5 do několika set km**;
- ne však malou vzdálenou událost, kterou by zachytil geofon — M3 ve 150 km je pod šumovým dnem.

**Přístrojem je síť**: mnoho uzlů v husté mřížce získá to, co jeden nedokáže, a sdílené hardwarové
hodiny je sladí na **±1 µs**, kde hobby seismograf na NTP drží ±10 ms — a právě to umožňuje práci
s časy příchodu vln a s porovnáním mezi uzly.

## Jak je připojen ke stanici

**Každý Quake je sám na vlastní odbočce Bifrostu — bod–bod, plný duplex, konec trasy na straně
jednotky.** Uzel nenese žádnou linkovou součást: napájecí deska pro jeho napájení a komunikační
deska pro jeho médium sedí v trubici na konci kabelu a trasa — měď nebo sklo, 48 V nebo 300 V — na
vlastní desce Quake nic nemění. Hodiny jsou Kronosovy, přenášené hodinovým párem odbočky; uzel se na
ně zavěsí, taktuje z nich své senzory a vysílá svůj rámec ve svém slotu. **Lze ho instalovat pod
libovolným úhlem do ~10° od svislice**: jeden kalibrační příkaz a uzel se sám vyrovná podle vektoru
gravitace.

```
   a Bifrost spur ══ the feed + the data pairs ══▶ power board + communication board ──┐
                                                                                       │ NB IN · PWR IN · 12V
                                                        ┌──────────────────────────────┴──────┐
                                                        │ QUAKE — H523, type 5, 40 B a frame  │
                                                        │ ADXL355 · ICM-42688-P · SCL3300     │
                                                        │ RM3100 and TMP117 by population     │
                                                        │ an NTC between the coils            │
                                                        └─────────────────────────────────────┘
```

![Prioritní pásy a města](coverage.svg)

> **Kde se mřížka vyplatí** — prioritní pásy a města podle pravidel umísťování v
> [`../gaia/SITING.md`](../gaia/SITING.md); vrstva je
> [`../gaia/priority-earthquake.geojson`](../gaia/priority-earthquake.geojson).

## Soubory

| soubor | obsah |
|---|---|
| [`HARDWARE.md`](HARDWARE.md) | deska — senzory, napájení, piny procesoru, časovače a strom hodin, konektory, součástky |
| [`FIRMWARE.md`](FIRMWARE.md) | popis firmwaru — start, sběrnice, čas, sběr dat, kalibrace, registry, poruchy |
| [`BUS.md`](BUS.md) | payload (užitečná data), 16bitový formát na vodiči, rámec hlášení, ukládání |
| [`CONSTRUCTION.md`](CONSTRUCTION.md) | usazení v zemi, rodina Barrel a její ohybové módy, hloubka, kabel |
| [`WHY.md`](WHY.md) | hřbitov — zamítnuté alternativy a překonané stavy |

## Licence

Hardware: CERN-OHL-S v2 (`../LICENSE-HW`) · Software: MIT (`../LICENSE`) — Copyright (c) 2026 NIC — Native Intellect Community

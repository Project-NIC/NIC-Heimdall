<p align="center">
  <img src="NIC-Gaia.svg" width="200"/>
</p>

★ N.I.C. ★

# Gaia — kam stanice patří

[English](README.md) · **Čeština** · [Русский](README.ru.md)

> **Koncept ve fázi návrhu — nic není postaveno.** Čísla se ověřují proti katalogovým listům a součástkám, než vznikne deska. **Všechny souřadnice jsou přibližné a orientační — vodítko pro umístění, ne geodetická data.**

**Gaia je mapa teček: kam po celé Zemi stanice patří, v jaké hustotě a v návaznosti na to, co už
existuje.** Jednotkou návrhu je jedna stanice; členství v síti je vlastností nasazení a data jdou do
sítí, které existují (`../core/INTEROP.md`). Jak se stanice staví a usazuje, popisuje
`../daedalus/CONSTRUCTION.md`; co jde do vody, popisuje `../atlantis/README.md`.

**Mezera, kterou vyplňuje.** Světové pozemní sítě jsou malé a nevyvážené: GSN ~150 seismických stanic,
INTERMAGNET ~120 magnetických observatoří, IGS ~530 referenčních stanic GNSS, DART 74 tsunami bójí —
a z přibližně 10–15 k aktivních profesionálních seismických stanic stojí třetina v Japonsku a USA,
zatímco celé obydlené pásy ohrožení — Indie, Turecko, Írán, Indonésie, východní Afrika — jsou téměř
holé. Ne další stanice tam, kde jich je mnoho: **první stanice tam, kde nejsou žádné.**

```
   GeoNames towns × the hazard lines ──▶ siting rules 1–6 ──▶ the first wave, ordered by global blindness
                                                           ──▶ COMPLEMENT where nothing is in reach
                                                           ──▶ DENSIFY where the spacing is ~100 km
   the loadout follows the site; the density follows the field
```

## Mapy

- **Jeden soubor po druhém** — GitHub vykreslí kterýkoli `.geojson` níže jako interaktivní mapu.
- **Všechny vrstvy dohromady** — `index.html` načte všechny vrstvy do jedné mapy Leaflet s přepínačem
  vrstev; běží na GitHub Pages (Settings → Pages → Deploy from `main`, root) na adrese
  `https://<org>.github.io/<repo>/gaia/`. Leaflet je přibalený ve `vendor/`, takže online se stahují
  jen mapové dlaždice.
- **Statické snímky** vedle jednotek, které je používají — `tesla/pip/coverage.svg`,
  `quake/coverage.svg`, `gauss/coverage.svg` — generované skriptem `tools/gen_coverage_svg.py`;
  autoritou je GeoJSON a po změně vrstvy se skript spustí znovu.

Stylování se řídí [simplestyle-spec](https://github.com/mapbox/simplestyle-spec), takže se soubory
vykreslí stejně v prohlížeči GitHubu i v kombinované mapě.

## Soubory

| soubor | obsah |
|---|---|
| [`SITING.md`](SITING.md) | šest pravidel umístění, dvě role nasazení, dvě úrovně (tiers), hustota a výbava, vypočtená první vlna, pravidlo „nejdřív ostrůvek“, proč lze síť rozmístit |
| [`DATA.md`](DATA.md) | veřejné datové sady, na kterých mapy stojí |
| [`WHY.md`](WHY.md) | hřbitov — co se zkusilo, co padlo a proč |
| [`proposed-network.geojson`](proposed-network.geojson) | **nasazení** — vypočtená první vlna, značka po značce, každá se svou výbavou a počtem jednotek; pobřežní značky nesou azimut hrozby, délku kabelu k patě svahu a hloubku paty a přichytí se k pojmenovanému ostrůvku tam, kde nějaký leží na svahu směrem k moři |
| [`existing-systems.geojson`](existing-systems.geojson) | reprezentativní výběr dnešních globálních sítí — DART · GSN · INTERMAGNET · IGS · CTBTO |
| [`priority-tsunami.geojson`](priority-tsunami.geojson) | hlavní subdukční příkopy a lokality polí sond na ostrovních obloucích, každá se sektorem, na který míří její vějíř |
| [`priority-earthquake.geojson`](priority-earthquake.geojson) | kontinentální seismické pásy a silně exponovaná, přístrojově nedostatečně pokrytá města |
| [`priority-tec.geojson`](priority-tec.geojson) | kde se GNSS-TEC vyplatí nejvíc — hřebeny rovníkové anomálie a polární ovály |
| [`priority-radiation.geojson`](priority-radiation.geojson) | nejlepší místa pro neutronové hlavice jako monitor kosmického záření — nízká geomagnetická prahová rigidita, vysoká nadmořská výška; nejslabší prioritní geografie |
| [`longwave-transmitters.geojson`](longwave-transmitters.geojson) | služby 34–120 kHz, které Pip slyší — modře stanice časového kódu (DCF77 · MSF · WWVB · RBU · BPC · JJY), úplné datum jednou za minutu na nosné odvozené od cesiového etalonu; oranžově eLoran na 100 kHz, čas na ~100 ns a bez kalendáře. Kruhy jsou nominální dosah ve volném poli; rozhoduje okolí přijímače (`SITING.md`, pravidlo 6). Většina planety žádný dlouhovlnný čas nemá |
| [`bathy-per-site.csv`](bathy-per-site.csv) | naměřená batymetrie pro 121 ze 168 pobřežních lokalit (ETOPO); 47 jsou schematické linie příkopů příliš daleko od skutečné pevniny a řešením je Slab2 (`DATA.md`) |
| `tools/` | `gen_network.py` · `gen_priority.py` · `gen_bathy.py` · `gen_coverage_svg.py` — skripty, které vrstvy počítají |
| `index.html` · `vendor/` | kombinovaná mapa |

## Licence

Hardware: CERN-OHL-S v2 (`../LICENSE-HW`) · Software: MIT (`../LICENSE`) — Copyright (c) 2026 NIC — Native Intellect Community

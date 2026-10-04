<p align="center">
  <img src="NIC-Palatine.svg" width="200"/>
</p>

★ N.I.C. ★

# Palatine — meteorologická základna

[English](README.md) · **Čeština** · [Русский](README.ru.md)

> **Koncept ve fázi návrhu — nic není postaveno.** Čísla se ověřují proti katalogovým listům a součástkám, než vznikne deska.

**Palatine je hostitel ModBus celé stanice.** Jeden `STM32H523`, který je masterem čtyř větví ModBus RTU
(arm) a jednotkou NodBus směrem ke své kartě: dotazuje se kupovaných čidel a vlastních MODů, které
uvádí jeho soupis, každého v jeho vlastním intervalu a rozloženě v rámci hodiny, aby žádná sekunda
nebyla přeplněná, každou odpověď odesílá jako samoohraničený blok ve svém 32 B payloadu (užitečná data)
a tunelové balíčky centrály předává na větve doslovně. **V režimu A, základním, s hodnotou nic
nepočítá** — žádný průměr, žádné nárazy větru, žádný převod jednotek — a nedrží žádnou politiku kromě
soupisu, který do něj zapsala centrála; **v režimu B je meteorologickým zapisovačem stanice**, počítá
statistiky WMO a odesílá řádek za minutu, za deset minut, za hodinu a za den (`WMO.md`). **Počasí patří
Palatine; to, z čeho se skládá vzduch, patří sadě Chinook** (`chinook/`), jejíž kupované jednotky visí
na týchž větvích.

| | |
|---|---|
| třída | **NOD** — NodBus **typ 6**, jeden slot, adresa `0x6n`; za odbočkou Bifrost |
| MCU | `STM32H523VE`, LQFP100, standardní H523 projektu |
| větve | **čtyři větve ModBus RTU, každá na vlastním USART** (USART2 · USART6 · UART4 · UART5), shodné; **standardně osazené dvě, strop jsou čtyři**. Každá opouští skříň na komunikační desce a napájí svá čidla z napájecí desky, 50 m, zhruba šestnáct jednotek na větev. Plocha širší, než pokryjí čtyři větve, znamená druhou Palatine, nikdy pátou větev |
| `PWR EXT` | **jeden samostatný napájecí konektor**, ne větev: zdrojová napájecí deska spínaná signálem `EN_X` pro čerpadlo jednotky Pluvius — její `INA238` je ampérmetr zátěže (`FIRMWARE.md` §6) |
| napájení | 12 V na jejích dvou svorkách — vodič od baterie ve skříni, na napájené trase svorky napájecí desky jednotky — jeden `LMR43610` na 3,3 V pro MCU a komunikační desky; **na této desce není žádná napájecí větev pro čidla**, čidlo si bere izolovaných 12 V z větve |
| zátěž | ~7 W s dnešními čidly, **~21 W** se čtyřmi větvemi na jejich limitu; vypínací práh trasy se měří při uvádění do provozu — špička při rozběhu a provozní zátěž, s rezervou (`HARDWARE.md`, *The budget*) |
| payload | 32 B **samoohraničených bloků**, `[address][length][data]`, jeden na každou transakci ModBus, jen celé bloky; dekódují se podle vlastní adresy modulu a jeho profilu tam, kde se čte archiv (`../core/PROTOCOL.md` §5) |
| bez dotazu | `PORTS` jednou za minutu — jejích šest zásuvek, nejprve port směrem nahoru, pak čtyři větve a `PWR EXT`; příznaky v hlavičce v každém rámci; `ACK` |
| firmware | čtyři mastery ModBus a jedna jednotka NodBus: soupis, obchůzka vlastních MODů, kruhový buffer bloků, tunel, pravidlo `PWR EXT` (`FIRMWARE.md`) |

## Co visí na větvích

**Vlastní MODy** — naše vlastní desky, každá jeden Modbus slave, na 12 V z větve:

- **Pluvius** (`../pluvius/`) — vážicí srážkoměr: tenzometrický snímač pod stíněnou nádobou, odpovídá v gramech, sám řídí své vypouštění; 24 V pro jeho hlavici čerpadla přichází z `PWR EXT`
- **Ceres** (`../ceres/`) — vlhkost a teplota půdy v jedné hloubce, jedna jednotka na každou hloubku; v −10 a −50 cm; profil pro farmu −10 · −20 · −50 · −100 cm
- **Sakura** (`../ceres/sakura/`) — ovlhčení listů
- **Babel** (`../babel/`) — čidlo, které se neprodává jako Modbus, převedené přímo u čidla; jeden slave na každou osazenou pozici
- **Vzduchové jednotky Chinook** (`chinook/`) — sada pro plyny a částice, kupovaná, na větvi jako všechno ostatní

**Kupovaná čidla RS-485** — `SENSORS.md` uvádí univerzální stanici a sestavu pro zemědělce, pro každé čidlo doporučený typ; co se neprodává jako RS-485, jde přes Babel:

- teplota a vlhkost vzduchu ve 2 m v radiačním krytu a přízemní teplota v 5 cm (`SITING.md`)
- vítr, rychlost a směr ve 2 m — mechanický nebo ultrazvukový podle klimatu a rozpočtu
- pyranometr, 0–2000 W/m²
- UV — index, UVA/UVB tam, kde je jednotka poskytuje
- barometr — absolutní, 300–1100 hPa, ≤ ±1 hPa, do −40 °C; nebo jedna jednotka T/RH/P ve 2 m
- výška sněhu — radarový hladinoměr 80 GHz, bezkontaktní, ±1 mm

**Jedna adresa není jedno čidlo**: jednotka může pod jednou adresou nést několik veličin a blok nese
všechny registry, na které odpověděla. **Déšť se váží, sníh měří radar**; kroupy padají do
srážkoměru, roztají a zváží se.

## Kde stojí

**Standardně ve skříni centrály** — odbočuje si vodič 12 V na vlastních svorkách a její větve
opouštějí skříň na deskách Galvani. Velká stanice postaví druhou Palatine na plochu s čidly nebo
k druhému stožáru desítky metrů daleko: tatáž deska za deskami Galvani. Vzdálená skříň zakopaná
kvůli tepelné setrvačnosti je volbou umístění, kterou dělá stavitel.

Každá větev je jedna sběrnice za jedním transceiverem; větve existují proto, aby se trasy držely
odděleně — vzdálený přístroj sám na své větvi, hrstka teploměrů na další — ne aby se násobila
zařízení. Součástky jsou dimenzované do −40 °C a každé kupované čidlo se kupuje do −40 °C; větev,
jejíž míra chyb CRC překročí práh, se vypne a po ochranné pauze se jí znovu připojí napájení
(`FIRMWARE.md` §9).

## Blokové schéma

```
   a BIFROST spur ──40 B NodBus, type 6──▶ ┌────────────────────────────────┐ ──arm 1..4, ModBus RTU──▶ bought sensors · Pluvius · Ceres · Sakura · Babel · Chinook's units
                                           │ PALATINE — H523 · LMR43610     │ ──EXT, one power body, EN_X──▶ 24 V, switched, to Pluvius's head
                                           │ polls the roster, packs blocks │
                                           │ into one 32 B record a frame   │
                                           └────────────────────────────────┘
```

## Strom

```
palatine/                     Palatine — the meteo base and the ModBus master  NodBus 6
└── chinook/                  Chinook — air quality: bought units on the arms, not a board
```

## Soubory

| soubor | obsah |
|---|---|
| [`HARDWARE.md`](HARDWARE.md) | deska — H523 a každý pin, větve, konektory, napájecí sběrnice, rozpočet, mapa adres Modbus, přidělování adres při uvádění do provozu |
| [`CONSTRUCTION.md`](CONSTRUCTION.md) | půdní sloupec — patrona — a kryt sněhového radaru |
| [`FIRMWARE.md`](FIRMWARE.md) | popis firmwaru — větve, soupis a obchůzka, dotazování, kruhový buffer bloků a tunel, pravidlo `PWR EXT`, registry, co je otestováno |
| [`SENSORS.md`](SENSORS.md) | co visí na větvích — dvě sestavy a co musí kupovaná jednotka splňovat, veličinu po veličině |
| [`SITING.md`](SITING.md) | radiační kryt a expozice každého přístroje |
| [`WMO.md`](WMO.md) | režim B — tabulky WMO, jejich sloupce, jak vznikají statistiky, registry, které přidává |
| [`CLIMATE.md`](CLIMATE.md) | popis uzavřeného období vůči normálu — třídy WMO a tabulky hranic |
| [`chinook/`](chinook/) | Chinook — z čeho se skládá vzduch: kupované vzduchové jednotky, které si lokalita osadí na tyto větve, co koupit a co musí každá splňovat |
| [`WHY.md`](WHY.md) | hřbitov — co se zkusilo, co padlo a proč |

## Licence

Hardware: CERN-OHL-S v2 (`../LICENSE-HW`) · Software: MIT (`../LICENSE`) — Copyright (c) 2026 NIC — Native Intellect Community

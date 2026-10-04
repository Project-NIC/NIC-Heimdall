<p align="center">
  <img src="NIC-Heimdall.svg" width="200"/>
</p>

★ N.I.C. ★

# NIC-Heimdall

[English](README.md) · **Čeština** · [Русский](README.ru.md)

**Hardwarová strana NIC: jedna soběstačná stanice měřící více druhů jevů — senzorové frontendy, které
sledují pevnou Zemi, atmosféru a ionosféru, a centrála, hodiny, karty, napájení a kabeláž, které je
nesou. Běžně dostupné součástky a data dost lehká na zpracování na domácím počítači.**

[![License: MIT](https://img.shields.io/badge/License-MIT-red.svg)](https://opensource.org/licenses/MIT)
[![License: CERN-OHL-S v2](https://img.shields.io/badge/License-CERN--OHL--S%20v2-red.svg)](https://ohwr.org/cern_ohl_s_v2.txt)
![Version: 0.2 — concept](https://img.shields.io/badge/version-0.2%20concept-blue.svg)

> ⚠️ **Koncept ve fázi návrhu — NEOVĚŘENO na hardwaru.** Architektura, přenosový protokol a archiv
> jsou propracované do hloubky a referenční kodek archivu je **otestovaný na hostitelském počítači**;
> **nic nebylo postaveno ani spuštěno na skutečném hardwaru** — žádná deska nebyla vyrobena, žádný
> firmware napsán, žádný senzor kalibrován. Psáno pro **zkušené stavitele**: dává topologii
> a zdůvodnění, ne návod. **Koncept této velikosti BUDE obsahovat chyby** — konzistence mezi
> dokumenty není totéž co správnost. Našli jste nějakou? To systém funguje — ověřte ji proti
> záznamu a záznam opravte.

> **Katalogová čísla na našich vlastních deskách jsou propracované referenční body, ne povinnost.**
> Dokumenty určují topologii a cílové hodnoty; konkrétní součástku konkrétního výrobce volí ten, kdo
> desku staví. **Kupované senzory se typem nejmenují vůbec** — pojmenovaná součástka je přesně ta,
> kterou stavitel v jiné zemi nesežene. Každá kupovaná jednotka je specifikována tím, co musí
> splnit (*Co se kupuje*, níže).

> **Schéma zatím neexistuje — schématem je popis.** Koncept této velikosti se vyvíjí v dokumentech,
> ne v EDA: `HARDWARE.md` každé desky nese její součástky, hodnoty, piny a výpočty za nimi. Schémata
> patří ke kroku od konceptu ke stavbě a [`schematics/`](schematics/) na ně čeká. Kdo nakreslí první
> desku, začíná z hotového popisu, ne z prázdné stránky — a jste-li to vy, složka je vaše.
> Číslování v rámci celé stanice neexistuje; desku najdete pod jejím projektem podle jména.

*(Archiv — HMC, kontejner, a HCC, kodek — je vlastní součástí stanice, v
[`core/archive/`](core/archive/HMC.md).)*

## Začněte zde

| chcete-li vědět | čtěte |
|---|---|
| **co měří** | [*Jednotky*](#jednotky--co-měří-a-čím-to-měří), níže — jeden řádek na jednotku, každý s odkazem na její složku |
| **jak stanice drží pohromadě** | [*Co je stanice*](#co-je-stanice), pak [`core/`](core/README.md) a [`core/COMMS.md`](core/COMMS.md) — sběrnice, hodiny, každé spojení v jedné tabulce |
| **jak se data uchovávají a sdílejí** | [`core/archive/HMC.md`](core/archive/HMC.md), [`EXPORTERS.md`](core/archive/EXPORTERS.md) a [`core/INTEROP.md`](core/INTEROP.md) — archiv, jeho čtečky a kdo data přebírá |
| **jak se staví** | [`daedalus/`](daedalus/README.md) pro stanici jako konstrukci, `HARDWARE.md` každé desky pro desku, [`schematics/`](schematics/README.md) pro budoucí výkresy |
| **proč je taková, jaká je** | `WHY.md` v každé složce — co se zkusilo, co padlo a proč |
| **jak pomoci a jak citovat** | [`CONTRIBUTING.md`](CONTRIBUTING.md) · [`CITATION.cff`](CITATION.cff) |

## Co je stanice

**Stanice je jedna skříň a ty frontendy, které daná lokalita potřebuje.** Uvnitř skříně: centrála
(Mayak), hodiny (Kronos), jedna nebo více karet (Bifrost / Argus), převodník BMS/MPPT (Hermes),
baterie a solární nabíječ. Každý kabel, který opouští skříň, prochází deskou Galvani — izolace,
přepěťová ochrana a napájení. Venku: jednotky, každá vlastní deska ve vlastní krabici, zavěšené na
odbočce bod–bod (měď do 500 m, dál sklo), plus kupované senzory na větvích ModBus (arm) Palatine.
**Vyměňte frontendy a je to jiný přístroj na stejné sběrnici a se stejným kódem.** Členství v síti je
vlastností nasazení, ne stanice: data se předávají sítím, které už existují (`core/INTEROP.md`).

```
   sky
    │
    ▼
 ┌────────┐   PPS + label   ┌────────┐   4 trunks   ┌──────────────┐   4 spurs   ┌─────────┐
 │ KRONOS │────────────────▶│ MAYAK  │─────────────▶│ BIFROST ×4   │────────────▶│  UNITS  │
 │        │  network clock  │ store  │  point-to-   │ an ARGUS     │             │  Quake  │
 │ the    │                 │ uplink │  point       │ behind a port│             │  Gauss  │
 │ second │                 │        │              │ 1 up + 4 down│             │  Tesla… │
 └────────┘                 └───┬────┘              └──────────────┘             └─────────┘
      ▲                         │
   POLARIS or SPUTNIK           ▼  LP I²C        BIFROST and ARGUS are ONE BOARD, one firmware — a
   (the GNSS receiver)       HERMES ──▶ BMS · MPPT · battery · panel
                                                  card pressed on Kronos's time bus ribbon is a Bifrost

   Every cable that leaves the enclosure crosses a GALVANI board — isolation, protection and
   the spur's feed. Inside the enclosure there is no 485 at all and a unit's link is a CABLE; Kronos's time
   bus to the cards is M-LVDS (kronos/HARDWARE.md).
```

**Čas pochází z jednoho místa.** Kronos disciplinuje síťové hodiny na GNSS a každý uzel počítá
*ty*, nikdy svůj vlastní krystal. Vzdálené jednotky visí za kartou na odbočkách bod–bod, každá
odbočka je vlastní časovou doménou s měřením zpoždění (ranging); dodaná přesnost je **±1 µs** v celé
stanici.

## Tři sběrnice

| sběrnice | rámec | kdo ji nese | kdo na ní visí |
|---|---|---|---|
| **NodBus** | rámec 40 B, payload (užitečná data) 32 B, TDMA, taktovaná | karta **Bifrost**: jedna kmenová linka nahoru, čtyři porty odboček dolů | plné jednotky — Quake, Tesla, Marconi, Sputnik, Palatine, Photon, Positron, Pip, Steinmetz, Argus |
| **NodBus mini** | rámec 16 B, payload 8 B, TDMA, taktovaná | karta **Argus**: jedno spojení nahoru, čtyři segmenty dolů | malé taktované jednotky — Gauss, Quark-Tubes, Pascal |
| **ModBus** | Modbus RTU, dotazovaná (polling), 9 600 nebo 19 200 | **Palatine**, čtyři izolované větve a `PWR EXT`, jedna rychlost na větev | každý kupovaný senzor a vlastní MODy — Pluvius, Ceres, Sakura, Babel |

**Identita je adresa**: `TYPE«4 | NUMBER` na NodBus a mini, `TYPE«2 | NUMBER` na ModBus, jeden
bajt, samopopisná, nikdy se nepřekládá. Typy NodBus: 1 Mayak · 2 Bifrost · 3 Argus · 4 Marconi ·
5 Quake · 6 Palatine · 7 Tesla · 8 Sputnik · 9 Photon · 10 rezervováno pro `Neutron` · 11 Positron ·
12 Pip · 13 Steinmetz. Mini: 1 Gauss · 2 Quark-Tubes · 3 Pascal. ModBus: 4 Babel, holý · 5 Pluvius ·
6 Ceres · 7 Sakura, dále jeden typ na každou kupovanou veličinu na pozici (`core/PROTOCOL.md`).

## Centrála a co na ní visí

| | | MCU |
|---|---|---|
| **[Mayak](mayak/)** | centrála — datalogger, uplink (modem · Wi-Fi · BLE k Handsetu), čtyři kmenová spojení, na kterých visí každá karta; žádné vlastní sběrnicové porty | ESP32-S31 |
| **[Kronos](kronos/)** | strážce času — TCXO disciplinovaný na GNSS, síťové hodiny na 2²³ Hz, PPS a sekunda ke každé kartě po jednom plochém kabelu M-LVDS | STM32H523 |
| **[Polaris](kronos/polaris/)** | GNSS frontend Kronosu, kupovaný — modul přijímače jen pro čas a aktivní anténa na nosné desce s jedním konektorem a dvěma rezistory; žádný procesor. Osazuje se, když stanice nemá Sputnik | — |
| **[Bifrost](bifrost/)** · **[Argus](bifrost/argus/)** | **jedna karta, jeden firmware**: na plochém kabelu Kronosu je to Bifrost a formátuje odbočky NodBus s rámcem 40 B s absolutní sekundou; mimo něj je to Argus nesoucí čtyři segmenty mini s rámcem 16 B | STM32H523 |
| **[Hermes](hermes/)** | převodník BMS/MPPT — dotazuje akumulátor a panel na jakékoli sběrnici, se kterou přijdou (485 Modbus RTU, TTL, I²C, CAN), a centrále předává jeden blok registrů po I²C | STM32H523 |
| **[Galvani](galvani/)** | transportní vrstva — **jedenáct desek**, žádné varianty osazení: `G-I-N-025` (485, NodBus) · `G-I-M-005` (485, větev ModBus) · `G-O-10-10` (sklo 10 Mb/s, 10 km) · `G-O-2-100` (sklo 2 Mb/s, 1 m až 100 km) · `G-12-S` (izolovaných 12 V pro kupované zařízení) · `G-24-S` (spínaných 24 V na `PWR EXT` Palatine, pro zátěž, která není senzorem — čerpadlo) · `G-48-S` · `G-300-S` (napájecí buňky stanice) · `G-48-U` · `G-300-U-6` · `G-300-U-40` (strana jednotky, výstup 12 V). Každý kabel opouštějící skříň prochází jednou z nich | — |
| **[Daedalus](daedalus/)** | stanice jako konstrukce — stožár, základ, šachta, uzemnění, skříň, vedení kabelů; stavební průvodce | — |
| **[Gaia](gaia/)** | atlas umístění — kam na planetě stanice patří | — |
| **[Handset](mayak/handset/)** | aplikace pro uvádění do provozu — telefon u otevřené skříně, přes BLE | — |
| **[Mimir](mimir/)** (mini-Heimdall) | Mayak, Kronos, Bifrost, Argus, Palatine a Hermes na jedné DPS, každá sekce tak, jak ji kreslí její dokument, odbočky z plochého kabelu a křížené kabely jako spoje na desce; dva porty NodBus, čtyři segmenty mini, čtyři větve, záložní MasterNOD, Ethernet a USB | ESP32-S31 + 5× STM32H523 |
| **[Proteus](proteus/)** | Mayak, Kronos, Bifrost a Hermes na jedné malé DPS, zasunutý Polaris, modem LTE-M na záložním článku, jedna měděná trasa NodBus na desce, izolovaný modul DC/DC na vstupu, Ethernet a USB-C — celá centrála stanice v pozici 1-DIN | ESP32-S31 + 3× STM32H523 |
| **[Atlantis](atlantis/)** | vodní provedení — desky Galvani v tlakovém provedení a kabel zvolený pro prostředí; žádná vlastní jednotka. Úroveň 1 (Pascal a Gauss pár set metrů od břehu) se staví z rodiny Galvani; úroveň 2 (magnetometr 50–100 km od břehu na 300 V) je **odložena** | — |

## Jednotky — co měří a čím to měří

**Jednotky NodBus** (rámec 40 B, payload 32 B, za Bifrostem):

| | měří | senzorová část | MCU |
|---|---|---|---|
| **[Quake](quake/)** | pohyb půdy a lokální události, plus náklon; volitelně pomalé magnetické pole | MEMS `ADXL355` + `ICM-42688-P` na nezávislých SPI, `SCL3300` pro náklon, varianta osazení s `RM3100` — silné otřesy (strong-motion), ne observatorní seismometr; trubka *DoubleBarrel* pro zakopání | STM32H523 |
| **[Tesla](tesla/)** | VLF sferiky — blesky, rychlé pole B, časování TGF; tatáž deska je Pip a Steinmetz s vlastním obrazem firmwaru | **tři feritové tyče** po 120° na komolém kuželu, diferenciální transimpedanční vstup, čtyřkanálový 24bitový převodník ΔΣ (`ADS127L14`) na 2²⁰ SPS, vlastní DSP | STM32H7A3 |
| **[Pip](tesla/pip/)** | oblast D — úrovně nosných VLF/LF (SID) a dlouhovlnný čas pro stanici bez GNSS | **deska Tesly s vlastním obrazem firmwaru**; typ NodBus 12. Pozdržen kvůli pokrytí vysílači, dokončen jako popis | STM32H7A3 |
| **[Steinmetz](tesla/steinmetz/)** | poruchy na elektrických vedeních — oblouk, koróna, částečné výboje — z vozidla při 80–100 km/h nebo z pevného místa | **deska Tesly s vlastním obrazem firmwaru**; typ NodBus 13. Ve vozidle visí na Proteu v kabině přes jeden hybridní kabel | STM32H7A3 |
| **[Marconi](marconi/)** | oblast F — úrovně nosných KV 0,5–16 MHz, MUF / foF2, pasivní ionogram | jedna svislá smyčka bez jádra do přímého vzorkování, `AD9265` na 2²⁶ | STM32H7A3 |
| **[Sputnik](sputnik/)** | integrální ionosféra — celkový obsah elektronů (TEC); a čas stanice, je-li osazen | vícepásmový přijímač GNSS `UM980`; pět slotů uzlů na jedné desce | STM32H523 |
| **[Palatine](palatine/)** | meteorologický základ — teplota/vlhkost vzduchu ve 2 m a 5 cm nad trávou, tlak, vítr, sluneční záření, UV, výška sněhu, srážky, vlhkost a teplota půdy, ovlhčení listů; a **[Chinook](palatine/chinook/)**, z čeho se skládá vzduch — kupované jednotky na týchž větvích | **žádný vlastní senzor** — čtyři izolované větve ModBus, kupované sondy RS-485 specifikované tím, co musí splnit, a vlastní MODy níže | STM32H523 |
| **[Photon](quark/scintillation/photon/)** | γ / rentgenové záření — počet, součet energií, největší událost, dvanáct energetických pásem, v každém rámci, akumulováno za sekundu | deska `Quark-Photon`: krychle CsI(Tl) + SiPM a PIN dioda jako jedno měření přepínané podle energie | STM32H7A3 |
| **[Positron](quark/scintillation/positron/)** | beta, obou znamének — a dva neutronové počty z téže krabice | deska `Quark-Neutron/Positron`: blok plastového scintilátoru 25 mm + SiPM; stínítka `⁶LiF/ZnS(Ag)` od [`Neutron`](quark/scintillation/neutron/) čtená fotonásobičem jako jeho první kanál — je pro něj rezervován typ 10 | STM32H7A3 |
| **[Argus](bifrost/argus/)** | nic — nese čtyři segmenty mini a skládá jejich záznamy 8 B do svého vlastního | karta (výše) | STM32H523 |

**Jednotky NodBus mini** (rámec 16 B, payload 8 B, za Argusem):

| | měří | senzorová část | MCU |
|---|---|---|---|
| **[Gauss](gauss/)** | pomalé geomagnetické pole — samostatná podoba, když stanice nemá Quake nebo má Quake bez magnetometru | `RM3100` v utěsněné trubce, v mořském provedení plněné olejem, trubka rodiny Barrel | STM32H523 |
| **[Quark-Tubes](quark/tubes/)** | záření pomocí trubic — každý kanál trubice se počítá na jedné desce, jen počty | deska `Quark-Tubes`: počítací H523; hlavice s trubicemi (GM trubice Photonu za odstupňovaným olovem, trubice He³/BF₃ Helionu, záchyt na Gd + GM trubice **Gadolinu** s **Rhodionem**, aktivace Rh) nemají MCU a dodávají jí impulzy | STM32H523 |
| **[Pascal](pascal/)** | vodní sloupec nad ním — měřič tsunami na mořském dně | piezorezistivní hloubkoměr na 30 bar, zalitý v trubce plněné olejem | STM32H523 |

**Vlastní MODy ModBus** (na větvi Palatine, napájené izolovanými 12 V větve, s vlastním vstupem `THVD1450`):

| | měří | senzorová část | MCU |
|---|---|---|---|
| **[Pluvius](pluvius/)** | srážky, vážením — gramy přitékající a gramy vypuštěné | záchytná plocha 200 cm² do nádoby na **tenzometrickém snímači 30 kg**, `ADS1235`, počítané peristaltické vypouštění 24 V na spínaném konektoru `EXT` Palatine | STM32H523 |
| **[Ceres](ceres/)** | objemový obsah vody v půdě a teplota půdy v její hloubce, jedna jednotka na hloubku v *patroně* | hřeben měřící přes borosilikátové sklo, deska zalitá v borosilikátové misce — postavena, protože žádný kupovaný povlak nepřežije hlínu | STM32H523 |
| **[Sakura](ceres/sakura/)** | ovlhčení listů a teplota na destičce | deska Ceres v téže misce, zavěšená v korunách pod 45°, sklem k obloze | STM32H523 |
| **[Babel](babel/)** | jakýkoli senzor, který se neprodává jako RS-485 Modbus — převedený přímo u senzoru, takže po stanici nikdy nejede cizí sběrnice | vstup I²C · SPI · UART · 1-Wire, výstup Modbus RTU; až čtyři pozice, každá odpovídá jako typ své veličiny | STM32H523 |

**Záření ve dvou provedeních:** **[Quark](quark/)** je radiační část, která měří tytéž veličiny dvěma způsoby — **[Quark-Tubes](quark/tubes/)**, vysoké napětí, každá trubice počítaná na jedné desce (GM trubice Photonu, **[Helion](quark/tubes/helion/)** a **[Gadolin](quark/tubes/gadolin/)** jsou na ní hlavice s trubicemi, ne samostatné jednotky), a **[Quark-Scintillation](quark/scintillation/)**, nízké napětí — Photon, Positron a jeho kanál `Neutron`.

**MCU, a jsou tři.** `ESP32-S31` v centrále · `STM32H523` (LQFP100, jediný footprint) na uzlech, Kronosu, kartě a každém podřízeném modulu · `STM32H7A3IIT6` (LQFP176) na úrovni DSP — Tesla/Pip, Marconi a obě scintilační desky. **Čtyři desky H7A3 sdílejí jednu digitální polovinu**: stejné LQFP176, pozici `APS25608N-OBR-BD` na portu 2 OCTOSPI1 na týchž jedenácti pinech (osazenou tam, kde ji obraz firmwaru chce — Marconi vždy, Tesla pro klasifikátor, Photon pro záznamy záblesků, u Positronu otevřené), stejný výstup Galvani. Liší se analogový vstup a datová cesta převodníku — port frame-sync převodníku `ADS127L14` na SAI u Tesly/Pipu, paralelní port na PSSI u ostatních tří.

## Co se kupuje

**Pravidlo: hotová jednotka stojí o pár dolarů víc než holý senzor uvnitř ní a snímací prvek je za
každou cenu tatáž součástka — takže nic, co se dá koupit, se nestaví.** Vlastní desky stanice jsou
ty, které trh neprodává v podobě, která vydrží: centrála, hodiny, karta, rodina Galvani, jednotky
výše. Všechno ostatní je nákup a je specifikováno tím, co musí splnit; kde dokument jmenuje typ, je
to doporučení, od kterého může stavitel začít, nikdy požadavek.

**Kupované senzory — na větvích ModBus Palatine** (`palatine/SENSORS.md`). Všechny **RS-485
Modbus RTU, 9 600 nebo 19 200 8N1, napájené z 12 V větve (vstup 10–30 V), dimenzované do −40 °C**,
v jednom pouzdru s kabelem:

| veličina | počet | co jednotka musí splnit |
|---|---|---|
| teplota + vlhkost vzduchu | 2 — ve 2 m v radiačním krytu, v 5 cm nad trávou bez něj | ≤ ±0,3 °C, ≤ ±2 % RH, nerezová sonda IP6x, v radiačním krytu |
| teplota půdy | hloubky Ceres | **hlásí ji Ceres**, vždy — druhá hodnota každé jednotky Ceres; teploměr v jakékoli jiné hloubce či výšce je vlastní kupovaná jednotka RS-485 dané lokality na větvi, ne součást základu |
| tlak | 1 | absolutní 300–1100 hPa, ≤ ±1 hPa; na větvi jako všechno ostatní |
| vítr | 1 | rychlost + směr; mechanický nebo ultrazvukový podle klimatu |
| sluneční záření | 1 | pyranometr 0–2000 W/m² |
| UV | 1 | UV index, 290–390 nm; UVA + UVB ve W/m², pokud je jednotka poskytuje; žádné UVC |
| výška sněhu | 1 | radarový hladinoměr FMCW 80 GHz, svazek ≤ 3°, ±1 mm, mrtvá zóna pod ~10 cm, IP67, pro sypké materiály, standardně −40 °C |
| vlhkost půdy | 2, dvě hloubky | **Ceres je vlastní provedení** — vlhkost i teplota půdy z jedné jednotky; kupovaná kapacitní sonda RS-485 jen tam, kde ji nikdo nepostaví |
| kvalita vzduchu (volitelně) | dle potřeby | v základu žádná — lokalita osazuje podle prostředí (les CO na úrovni požáru, město PM/NO₂/O₃, průmysl svůj vlastní plyn), každá z nich hotová jednotka RTU a spotřební materiál (`palatine/chinook/`) |

**Kupované díly jinde ve stanici:**

| co | kde | co musí splnit |
|---|---|---|
| přijímač GNSS, jen čas | Polaris, v patici Kronosu | kupovaný u-blox NEO-M8N, jediný typově určený druh, s aktivní anténou na stožáru; typ je pevně daný, nikdy se nedetekuje a Kronos ho konfiguruje při každém startu |
| přijímač GNSS, TEC + čas | Sputnik | `UM980`, vícepásmový |
| baterie | skříň | LiFePO4, 2P4S po 314 Ah, 8,0 kWh — strop; nabíjená mezi 20 a 80 %, ~5 dní plného zatížení — stanice energií nešetří |
| BMS a MPPT | za Hermem | jakýkoli pár, který mluví 485 Modbus RTU, TTL, I²C nebo CAN; BMS se musí sama znovu zapnout, když se akumulátor dobije |
| solární panel | stožár | dimenzovaný podle lokality (`gaia/`) |
| datový kabel | každá napájená trasa | venkovní UTP Cat 6, všechny čtyři páry: data TX · hodiny · zem · data RX — plný duplex |
| napájecí kabel | každá napájená trasa | dvoužilový, 2× 1,5 nebo 2,5 mm², třída 300/500 V, na souši samostatně — hybridní kabel jako možnost, pod vodou vlastní plášť datového kabelu |
| optické moduly | `G-O-10-10` · `G-O-2-100` | třída 10 Mb/s se stejnosměrnou vazbou do 10 km; `OPT2-55A03STR` (1550 nm, 1 m až 100 km) na desce 2 Mb/s |
| transformátory | napájecí buňky Galvani | šest katalogových vinutí — `750310988` · `750311607` · `750310349` · `11328-T078` · `11338-T195` · `750311592` |
| skříň, stožár, uzemnění | Daedalus | `daedalus/CONSTRUCTION.md` |

## Napájení v jednom odstavci

Bateriový rozvod je 12 V stanice, hvězda z pojistkového pole skříně, jedna pojistka na desku; každá
deska si z něj dělá vlastní nízká napětí. Napájená trasa opouští skříň na **48 V**, nebo na
**300 V** tam, kde 48 V neunese zátěž tak daleko; vyrábí je zdrojová buňka Galvani a deska jednotky
na vzdáleném konci je převádí zpět na 12 V. Vypínací bod trasy se měří při uvádění do provozu
a programuje do `INA238` na straně zdroje, pod stropem buňky — ~26 W při 48 V, ~47 W při 300 V.
Měď do 500 m, dál sklo. **Koncept stojí na tom, co se běžně prodává**: běžný venkovní Cat 6, ne
speciální kabel (`galvani/README.md`).

## Proč na tom záleží

- **Seismika** — lokální pohyb půdy a detekce událostí; hustá síť je základem výstrah typu
  včasného varování před zemětřesením.
- **Ionosférický TEC** — vícefrekvenční GNSS měří celkový obsah elektronů přímo: kosmické počasí,
  šíření KV, korekce určování polohy a přechodové jevy, které otřásají horní atmosférou.
- **Záření a blesky** — levné relativní monitory: statistika mnoha uzlů, ne jeden přesný přístroj.
- **Vzájemná vazba** — velká seismická událost vyšle o minuty později akusticko-gravitační vlny
  nahoru do ionosféry; Quake a Sputnik v jedné lokalitě zachytí oba konce.
- **Přístrojem je hustota.** Satelity se pohybují a vzduch se pohybuje, takže síť vzorkuje
  vyvíjející se **4D objem**, nikoli soubor statických bodů — rozestup v mezoměřítku už rozliší to,
  co řídká síť nedokáže.

## Proč je propracovaný uzel ten úsporný

Lákavá „jednoduchá“ stanice posílá svůj surový signál ven a přemýšlení nechá na serveru. Je to
falešná úspora na tom jediném rozpočtu, který svazuje bezobslužný solární uzel: **energii.** DSP na
okraji stojí pár miliampér na procesoru, který stejně běží; streamování surových dat drží
zaměstnané **rádio**, a rádio je žrout. **Takže složitý uzel je ten s nízkou spotřebou** — a spotřeba
není číslo o výdrži baterie, je to cena stanice, protože spotřeba lineárně škáluje solár
i úložiště.

Stejná doktrína platí pro data: **ukládej změnu, ne surová data** · **detekce, ne metrologie** —
vědět, *že* se něco stalo, je jeden práh, absolutní tok je místo, kde se náklady násobí. **Přístrojem
je síť.** A protože okraj už vyrábí hotový produkt za každou stanici, připojení k existujícímu
agregátoru je na jeho straně triviální vložení, ne nová pipeline.

## Kolik to stojí

> **Odhad od oka, záměrně.** Řádová čísla, která mají ukázat *poměr*. Náklady na stanici se liší
> ±2×; pointa přežije chybové úsečky s velkou rezervou.

**Jeden hrubý model, jen součástky, a každé další číslo v projektu se čte proti němu:**

| co | zhruba |
|---|---|
| základ — Mayak, Kronos, karta, Palatine, Hermes; akumulátor 8 kWh a panel 570 W; skříň, šachta, stožár a uzemnění; kupovaná meteorologická sada | **~$3–4 k**, polovina z toho akumulátor, panel a konstrukce |
| jednotka úrovně DSP — Tesla, Marconi, deska Quark-Scintillation — se svým párem Galvani a kabelem | **~$0,5–0,8 k** za kus |
| jednotka H523 — Quake, Gauss, Pascal, Pluvius — se svým párem Galvani a kabelem | **~$0,2–0,5 k** za kus |
| **plná stanice**, základ a pět nebo šest jednotek | **~$7–9 k** |

| rozsah | stanic | rozestup | @ ~$4 k (základ) … ~$8 k (plná) | = světových vojenských výdajů |
|---|---|---|---|---|
| jedna země (ČR) | 100 | ~28 km | ~$0,4–0,8 M | **~5–10 sekund** |
| koridor Baltské moře–Jadran | 1 000 | ~28 km | ~$4–8 M | ~1–1,5 minuty |
| **celá Eurasie** | 65 535 | ~29 km | **~$0,26–0,52 B** | **~0,8–1,7 hodiny** |
| celá planeta, mezoměřítko | ~178 000 | ~29 km | ~$0,7–1,4 B celkem | ~2,3–4,6 hodiny |

Ten řádek pro Eurasii není překlep: 2bajtová doména stanic s rozestupem v mezoměřítku pokryje
v podstatě celou pevninu a pošle atmosférou naráz **~2,7 milionu současných paprsků**.
Celoplanetární civilní geofyzikální přístroj je **jeden velký infrastrukturní projekt** — linka
metra, dlouhý most — ne let na Měsíc. *(Světové vojenské výdaje ≈ $2,7 T/rok, veřejná čísla SIPRI.
Měřítko, ne politický postoj.)*

**Nejde o peníze a nejde o fyziku.** V kontinentálním měřítku je svazujícím omezením **přeshraniční
koordinace**. NIC je navržen tak, aby rostl zdola: každá stanice je autonomní, síť se skládá kousek
po kousku a k tomu, abyste začali se stovkou stanic v jedné zemi, nepotřebujete svolení celého
kontinentu.

## Kód

**Zatím není napsán žádný firmware a žádný není v tomto repozitáři.** Firmware jednotky je popsán
v jejím `FIRMWARE.md` — start, stavy, sběrnice, čas, registry, poruchy — a píše se podle tohoto
popisu, jakmile se `HARDWARE.md` desky ustálí: nejdřív deska, potom firmware.

Kód, který tu je, je dvojího druhu, obojí čistý Python 3:

| | |
|---|---|
| referenční kodek archivu | [`core/archive/ref/`](core/archive/ref/) — HCC tak, jak ho určuje `HCC.md`, Steim-2, se kterým se porovnává, a jeho testy (`python3 tests/test_hcc.py` a `tests/test_damaged.py` pro proudy, které nejsou celé) |
| výpočty za dokumenty | [`gaia/tools/`](gaia/tools/) mapy umístění · [`gauss/models/`](gauss/models/) model umístění v mělké vodě · [`marconi/models/`](marconi/models/) směrování smyčky · [`tesla/design/`](tesla/design/) pracovní list tyče, výpočet jejího pole a limitace a šumové dno řetězce |

## Strom

Každý projekt je složka a díl, který patří k jinému, sedí v jeho podsložce. Pravý sloupec je
sběrnice a typ.

```
NIC-Heimdall/
├── mayak/              Mayak — the head: datalogger and uplink           NodBus 1
│   └── handset/        Handset — the commissioning app, over BLE
├── kronos/             Kronos — the clock
│   └── polaris/        Polaris — Kronos's GNSS front, bought
├── hermes/             Hermes — the BMS/MPPT converter
├── bifrost/            Bifrost — the card, NodBus master                 NodBus 2
│   └── argus/          Argus — the same card, NodBus mini master         NodBus 3
├── galvani/            Galvani — the eleven transport boards
├── mimir/              Mimir — mini-Heimdall, the station on one board
├── proteus/            Proteus — the smallest whole station head
├── quake/              Quake — seismograph                               NodBus 5
├── tesla/              Tesla — lightning and sferics                     NodBus 7
│   ├── pip/            Pip — longwave carriers: SID and time             NodBus 12
│   └── steinmetz/      Steinmetz — line faults on power lines            NodBus 13
├── marconi/            Marconi — the HF ionosphere                       NodBus 4
├── sputnik/            Sputnik — GNSS TEC, and the station's time        NodBus 8
├── palatine/           Palatine — the meteo base, ModBus master          NodBus 6
│   └── chinook/        Chinook — air quality, bought units
├── quark/              Quark — the radiation part
│   ├── tubes/          Quark-Tubes — high voltage                        mini 2
│   │   ├── helion/     Helion — He³ / BF₃ neutron head
│   │   └── gadolin/    Gadolin — Gd neutron head, with Rhodion
│   └── scintillation/  Quark-Scintillation — low voltage
│       ├── photon/     Photon — γ / X-ray                                NodBus 9
│       ├── positron/   Positron — beta                                   NodBus 11
│       └── neutron/    Neutron — the photomultiplier channel             type 10 reserved
├── gauss/              Gauss — the magnetometer sonde                    mini 1
├── pascal/             Pascal — the pressure sonde, tsunami              mini 3
├── pluvius/            Pluvius — the weighing rain gauge                 ModBus 5
├── ceres/              Ceres — soil moisture                             ModBus 6
│   └── sakura/         Sakura — leaf wetness                             ModBus 7
├── babel/              Babel — any sensor → Modbus                       ModBus 4 bare
├── atlantis/           Atlantis — the water build
├── daedalus/           Daedalus — the station as a structure
├── gaia/               Gaia — the siting atlas
├── schematics/         the drawings to come, one folder a board
└── core/               the node core — bus, frame, clock, archive
```

## Kde je zbytek

| | |
|---|---|
| jádro uzlu, sběrnice, rámec, hodiny | **[`core/`](core/README.md)** |
| co mluví s čím a po čem | **[`core/COMMS.md`](core/COMMS.md)** |
| názvy, přiřazení MCU, kdo je NOD a kdo MOD | **[`NAMING.md`](NAMING.md)** |
| kdo přebírá naše data a v jakém formátu | **[`core/INTEROP.md`](core/INTEROP.md)** |
| archiv — HMC, kontejner, a HCC, kodek | **[`core/archive/`](core/archive/HMC.md)** |
| exportéry — každý formát, který svět přijímá, zapsaný z archivu | **[`core/archive/EXPORTERS.md`](core/archive/EXPORTERS.md)** |

## Otevřenost

Hardware pod **CERN-OHL-S v2**, software pod **MIT**. Postavte to, změňte to, rozšiřujte to — jak
pomoci, je v [`CONTRIBUTING.md`](CONTRIBUTING.md), a jak citovat, v [`CITATION.cff`](CITATION.cff).

★ Viva La Resistánce ★

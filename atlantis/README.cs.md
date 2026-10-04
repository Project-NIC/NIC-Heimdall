<p align="center">
  <img src="NIC-Atlantis.svg" width="200"/>
</p>

★ N.I.C. ★

# Atlantis — všechno, co jde do vody

[English](README.md) · **Čeština** · [Русский](README.ru.md)

> **Koncept ve fázi návrhu — nic není postaveno.** Čísla se ověřují proti katalogovým listům a součástkám, než vznikne deska.

**Atlantis je vodní provedení vlastních jednotek stanice: desky Galvani v tlakovém provedení, tělo
kapsle a kabel zvolený pro prostředí — nic víc.** Není to jednotka a žádnou nepřidává: kapsle ve
vodě jsou Pascal a Gauss a desky jsou ty z `../galvani/`.

**Dvě úrovně sdílejí tělo kapsle, recept na kabel a doktrínu zalévání**; liší se jen délka
a výbava:

| | **Úroveň 1 — stavěná** | **Úroveň 2 — odložená** |
|---|---|---|
| co | Pascal a Gauss pár set metrů od strmého ostrůvku, v prstenci | magnetometr na úpatí ostrovního svahu |
| kabel | **jeden mořský kabel**, napájení v plášti datového kabelu — dva vodiče, nebo jeden se zpětnou cestou přes moře (`../galvani/README.md`, *Under water*); měděné páry do 1 km, dál sklo | **50–100 km** pancéřovaného hybridního kabelu, pokládaného z lodi — 100 km je strop, daný modulem (*Spojení*) |
| napájení | **300 V** se zpětnou cestou přes moře; na dvou vodičích po měděné trase stačí 48 V | **300 V** |
| desky | desky Galvani v tlakovém provedení, modul, který se pro trasu hodí | tytéž |
| hloubka | 150–200 m ≈ 15–20 bar | umístění v 1,7–4,7 km ≈ 170–470 bar; provedení dimenzované na 8 km ≈ 800 bar |

**Úroveň 2 je odložena kvůli nákladům, ne kvůli fyzice.** Elektronikou je pár 300 V z rodiny
a skleněný modul 2 Mb/s; **projektem je kabel** — 50–100 km pancéřovaného hybridu pokládaného
z lodi je položka v řádu milionů, jakmile se započte pokládka, průzkum a povolení, o řády víc než
cokoli jiného, co NIC staví. **Optika na mokrém konci je jediná část, kterou nelze koupit
z regálu**: kapsle bere tlakově odolnou podobu modulu, vyžádanou od výrobce. Koncept je
propracovaný až do výpočtů níže, aby se, pokud se tato úroveň někdy probudí, nic neodvozovalo
dvakrát.

```
   SHORE STATION                                                          THE PODS, 150–200 m, a ring round the islet
   300 V source · the module that suits the run  ══ one sea cable ══▶    PASCAL and GAUSS on their pressure boards, oil or jelly in PVDF

   TIER 2, SHELVED:  300 V source · the 2 Mb/s glass  ══ 50–100 km armoured hybrid, ship-laid ══▶  a Gauss pod at 1,7–4,7 km
```

## Mořský hladinoměr — tlaková sonda místo radarového stožáru

Pobřežní lokality pro tsunami nesou **dolů hledící radarový mořský hladinoměr** a ten potřebuje
stožár, který město dokáže hostit a udržovat (`../gaia/SITING.md`, pravidlo 4). **Hloubkoměr na
mokrém konci měří totéž zespodu**: tlakoměr na 30 bar, `MS5837-30BA`, ~300 m vody, daleko nad
sloupcem ~30 m, který lokalita pro tsunami potřebuje. Tím odpadá stožár, a ten tvoří většinu
nákladů na údržbu pobřežní stanice. **Je to Pascal** (`../pascal/README.md`), typ NodBus mini 3 —
jak je ukotven, jak hluboko jde a proč se vzorkuje v dávkách, najdete tam.

## Prstenec

**Čtyři, osm, dvanáct — a strop určuje pokládka.** Sondy úrovně 1 stojí v prstenci kolem
ostrůvku, Pascal i Gauss, 150–200 m dolů po jeho strmém svahu. Hluboké sondy úrovně 2 na úpatí
svahu místo toho stojí jako vějíř 4–5 sond v sektoru obráceném ke zdroji a prstenec osmi si
ponechávají jen tam, kde k ostrovu dosahují dva oblouky (`../gaia/SITING.md`, pravidlo 4):

| sondy | rozestup | co potká čelo vlny z libovolného směru |
|---|---|---|
| **4** — minimum | 90° | jednu sondu čelně a dvě vedle ní na bocích, částečně; ta v závětří je reference |
| **8** — rekonstrukce | 45° | dvě nebo více čelně a boky po obou stranách — směr i obtékání kolem ostrova jsou rozlišeny |
| **12** — strop | 30° | totéž, s jednou sondou, kterou lze ztratit |

**Dvanáct určuje kabel, ne senzor.** Každá sonda je samostatná trasa položená od břehu a kabel
s pokládkou stojí mnohonásobně víc než sonda; nad dvanáct lokalita kupuje kabel, ne informaci.

## Tělo kapsle — malé, monolitické, plněné olejem

Na souši je klasická krabice v pořádku. **Ve vodě je tlak celým omezením** a provedení jde opačnou
cestou: **malé, monolitické, jedna deska v oleji**. Magnetometr jsou cívky, ne MEMS, takže tlak
snáší přímo — olej ho přenáší a není tu žádná nádoba, kterou by rozdrtil. **Celá kapsle je
zalitá, včetně optického transceiveru, a vlákno je vyvedeno přímo ven, svařené, utěsněné uvnitř
výplně.**

**Nepřítelem v hloubce je uzavřená plynová bublina** — stlačuje se, posouvá a namáhá zálivku —
proto je provedení **trubka z PVDF, olej nebo vazelína, odplyněné ve vakuu: nikde žádný plyn
a žádný vak.** Při 800 bar, na které je provedení dimenzováno, se výplň stlačí o ~4–5 % objemu
a PVDF o ~2,7 %, a chladné dno přidá ~0,5 %; stěna pojme rozdíl ~2,5–3 % tím, že se prohne
dovnitř — kruhová stěna Ø 40 o tloušťce 2 mm při obvodovém přetvoření ~1 %, výplň ~20 bar pod
tlakem moře — a stěna zploštělá za tepla (PVDF se tvaruje při ~150–160 °C) to pojme ještě
měkčeji. **Vodík je vyloučen konstrukcí**: žádná obětovaná anoda u kapsle, vlákno v hermetické
trubičce, výplň odplyněná, žádná vysokopevnostní ocel. Teploměr pak čte skutečnou izotermní
teplotu dutiny. **Mělká kapsle hladinoměru a hluboká sonda se liší tloušťkou stěny a výbavou, ne
konstrukcí.**

**Vyloučeny jsou jen součástky se vzduchovou dutinou a kapsle žádné nemá.** Budič laseru,
přijímač a procesor jsou integrované obvody s plným tělem a snášejí tlak jako zbytek zálivky;
senzor náklonu z Gausse právě proto odešel a zmizel i krystal — HSI kapsle se řídí stupněm hodin
přicházejícím po spojení (`../core/blocks/clocks.md`, *The two sondes carry no crystal at all*).

## Optický frontend, odolný vůči oleji a tlaku

**Modul 1×9 nemůže do hloubky**: je to pouzdro se vzduchovou dutinou a zásuvkou (receptacle)
a plynová dutina za víčkem takové plochy je to jediné, co kapsle zakazuje; zalití zvenčí dutinu
uvnitř pouzdra nevyřeší. **Do kapsle jde tlakově odolná podoba téhož modulu, vyžádaná od
výrobce** — **hermetický laser a fotodioda s vláknovým pigtailem, koaxiální TO-can Ø 5,6 mm**
s vláknem vystupujícím axiálně: čip a jeho čočka jsou zatavené za sklem uvnitř kovového pouzdra,
olej se dotýká jen inertního kovového a skleněného povrchu (inertní výplň, silikonový olej)
a hydrostatický tlak vyrovnává olej, takže jedinou namáhanou dutinou je malé okénko pouzdra.
**Návrhovou obálkou je hluboké provedení**: ~470 bar v nejhlubším umístění v rámci dosahu 100 km,
170–400 bar tam, kde sedí většina sond (`../gaia/bathy-per-site.csv`). Pigtail je svařen
s vláknem kabelu a vyveden zalitou podmořskou průchodkou pro vlákno; holé sklo je odolné vůči
oleji i tlaku.

**Co výrobce nesmí ztratit, ať je pouzdro jakékoli:** stejnosměrnou vazbu s **klidovým stavem
mapovaným na zhasnutý laser** — pravidlo, na kterém jsou optické desky postaveny
(`../galvani/README.md`) — taktovací kmitočet segmentu a úrovně TTL. Požadavek se zadává
v hertzích, ne v Mb/s. Stejnosměrná vazba přivádí klidovou úroveň přímo na laser a UART je v klidu
ve vysoké úrovni, takže při špatné polaritě laser trvale svítí a vypálí se.

## Kabel — kupovaný, pro moře

**Trasa úrovně 1** je jeden mořský kabel s napájením v plášti (`../galvani/README.md`, *Under
water*). **Trasa úrovně 2** je **hybrid**: jednovidové vlákno pro segment NodBus mini a měď pro
napájení 300 V v jednom plášti.

- **Počet vláken je téměř zadarmo.** U pancéřovaného kabelu stojí peníze pancíř a pokládka, ne
  sklo, takže se táhnou rezervní vlákna: dvě nebo tři v provozu — BiDi nebo duplex — zbytek
  studené rezervy.
- **Plášť a pancíř pro moře**: podélně vodotěsné jádro (gel nebo bobtnavá páska, aby poškozený
  plášť nemohl nasávat vodu podél kabelu), pancíř z ocelových drátů proti oděru a proti nebezpečí
  kotev a rybolovu na mělčinách, vnější plášť stabilní vůči UV. Běžný podmořský recept, kupovaný.
- **Pokládka**: po mořském dně mimo místa oděru, **zahrabaný v pásmu příboje a kotvení**, na
  tvrdém dně v hloubce, s **rezervou na každém konci** a svorkou u sondy a na břehu, aby vlny
  a proudy nikdy netahaly za průchodku ani za svár.
- **Do kapsle**: vlákno přes zalitou podmořskou průchodku pro vlákno, měď za vlastní vývodkou a při
  zpětné cestě přes moře katoda na druhé. Průchodky jsou jediné hranice, které je třeba utěsnit.
- **Na břehu**: kabel přistává v pobřežní stanici, běžné pozemní stanici, jejíž brána nese jediný
  uplink ven za celý klastr (`../core/UPLINK_TRANSPORT.md`).
- **V moři žádné aktivní vybavení** — žádný opakovač uprostřed trasy, žádné napájení na mořském
  dně, žádné pole hydrofonů.

## Napájení — dvě varianty

**Jedna kapsle, jedna sada desek, jedno vlákno, 300 V.** Liší se zpětná cesta:

| | **A — dvojitá izolace** | **B — zpětná cesta přes moře** |
|---|---|---|
| zpětná cesta | druhý vodič; nic neopouští kabel | moře |
| referenční kabel | **Type 3744** (dok. 11906565 rev. F): 4× SM G.657.B2 + 4× MM v ocelové trubičce, **7× 1,0 mm², ≤ 20,4 Ω/km**, 3 kV, 10,5 mm, 6 000 m, plášť PU | **OCC-SC500**: 17 mm LW, 8 000 m, > 70 kN, 18 kV DC, pancéřovaná provedení do 200 m; jeden vodič, třída 1–1,6 Ω/km |
| kapsle | plovoucí potenciál | titanová katoda na druhé průchodce |
| břeh | zdrojová deska jako u každé trasy | vodič na záporném pólu; anoda (MMO nebo vysokokřemíková litina, nikdy obyčejná ocel) v zemi, která zůstává mokrá a vodivá celý rok |
| požaduje se od výrobce | pancíř, pevnost při přetržení, souvislá délka | odpor vodiče, cena |

**Volitelně u A — vložka hlídající průnik.** Tenká kovová objímka uvnitř skořepiny z PVDF,
s odstupem od desky a připojená k jednomu napájecímu vodiči: mořská voda prasklou skořepinou
spojí tento vodič s mořem a hlídání úniku ve stanici to okamžitě zachytí. Poslouží jakýkoli kov,
který v mořské vodě vydrží, dokud se kapsle nevyzvedne — hliník, nerez.

**Vedení stojí na plném V₀ dřív, než kapsle začne odebírat**, takže restart do vybitého zásobního
kondenzátoru je jediný okamžik, kdy zátěž neodebírá konstantní výkon: napájecí deska jednotky
omezí nárazový proud, jinak vedení samo sebe stáhne dolů.

## Co kapsle odebírá

**Počítá se s 1 W, dimenzuje se na 1,5 W.** Modul je `OPT2-55A03STR`, `I_TX + I_RX` 100 mA
v katalogovém listu jako jedno číslo; kanál hodin osazuje jen `VccR` a datový vysílač je mezi
dávkami zhasnutý:

| | při 3,3 V |
|---|---|
| datový modul, obě sekce, TX v dávkách | ~80–100 mA |
| modul hodin, jen přijímací sekce | ~20 mA |
| `STM32H523`, jeden UART a SPI | ~30–60 mA |
| `RM3100`, `INA238` + `ISO1642`, zbytek | ~10 mA |
| **na 3,3 V** | **~0,5–0,6 W** |
| **na kabelu**, přes snižující měnič a izolovaný ostrov při ~85 % | **~0,6–0,7 W** |

**`VccT` a `VccR` jsou oddělená napájení s oddělenými zeměmi**, takže jednosměrné spojení hodin
nepotřebuje žádnou jednosměrnou součástku: kapsle osazuje jen `VccR` a centrála jen `VccT`.
Spojení hodin nemůže běžet v dávkách — hodiny jsou spojité — takže tento modul odebírá vždy plný
proud.

**Napájení je 300 V, protože 48 V to neunese**: na 105 km potřebuje i 1 W ~130 V na
2× 1,5 mm² a 48 V dodá ~0,17 W.

**Zásobní kondenzátor v kapsli se dimenzuje podle dávky a dávka podle protokolu.** Kabel dodává
konstantní výkon a zátěž kapsle je dávková, takže ji zásobní kondenzátor vyrovnává:
`C = 2·P_burst·T / (V₁² − V₂²)`. Délka dávky klesá se stupněm a s velikostí bloku a kondenzátor
dlouhou dávku nezachrání, takže pravidlo zní **posílej často a po malých kouscích** — dávka dost
krátká na to, aby ji kondenzátor pokryl, měnič dimenzovaný na průměr.

## Dosah

Zákon konstantního výkonu jako všude — `V_end = (V₀ + √(V₀² − 4·P·R_loop)) / 2`, kolaps při
V₀² < 4·P·R. Nejhorší případ sčítá rezervu trasy, maximální odpor vodiče a žádný bonus za
studenou vodu: **105 km, 20 °C, 300 V**. „Použitelné“ je domácí kritérium `V_end` ≥ 75 % `V₀`,
`P = 0,1875·V₀²/R`, před měničem kapsle:

| kabel | smyčka | kolaps | použitelné |
|---|---|---|---|
| hybrid 2× 1,5 mm² | 2541 Ω | 8,9 W | 6,6 W |
| **Type 3744, 2 vodiče** | 4284 Ω | 5,3 W | **3,9 W** |
| Type 3744, 3 + 3 | 1428 Ω | 15,8 W | 11,8 W |
| **OCC-SC500 + moře**, 1,6 Ω/km | ~200 Ω včetně zemniče | ~110 W | ~84 W |

**Každý řádek nese rezervu 1,5 W**; nejmenší kabel, který to zvládne, je ten, který lokalita
dovolí.

## Spojení

**Dva přenosy, každý jiný.** **Datové** spojení je jeden modul provozovaný **plně duplexně** —
BiDi na jednom vlákně nebo duplexní součástka na dvou, podle volby stavitele — takže se nepřepíná
žádný směr a na optické straně není žádné `DE`. **Hodiny** jsou jednosměrné: vysílač v centrále,
přijímací sekce v kapsli. **Dvě vlákna s párem BiDi, čtyři s duplexními součástkami.**

**Kapsle je mini-NOD, takže její segment běží na stupni mini**, a tam trh součástky má:
**TTL transceivery 1×9 se stejnosměrnou vazbou, specifikované od nuly do 2 Mb/s na 100 km**,
jednovidové 1550 nm, průmyslové provedení — **`OPT2-55A03STR`** (3,3 V, SC, 1550 nm, **rozpočet
33 dB**), nebo **sladěný pár BiDi `OTB2-35A03STR` + `OTB2-53A03STR`**, jehož dva konce nejsou
zaměnitelné. **Průmyslové provedení −40 až +85 °C**, protože pobřežní přistání je venkovní skříň
jako každá jiná. **Hodiny 2¹⁹ — 524,288 kHz, ekvivalent 1,05 Mb/s** — se do nich vejdou; data jsou
~400 B/s a do volby vůbec nevstupují. Hodiny 2²² sběrnice s rámcem 40 B odpovídají 8,39 Mb/s a nic
se stejnosměrnou vazbou je nepřenese dál než zhruba 40 km — proto je dlouhé spojení segmentem mini.

## Jak daleko a jak hluboko — rozpočet kabelu a hloubky

**Na úrovni konceptu: tato čísla vybírají třídu kabelu, nejsou průzkumem**; skutečné nasazení si
změří vlastní lokalitu. Kde sonda sedí, určuje magnetometr — plný transportní signál je na **úpatí
vlastního svahu ostrova** a plošina za ním přidá ~0 % (`../gauss/ARRAY.md`) — takže délka kabelu
je dána tím, kde podle místní batymetrie leží úpatí. Skutečná batymetrie (ETOPO) pro 121 ze 168
pobřežních lokalit je v `../gaia/bathy-per-site.csv`:

| | |
|---|---|
| kabel k úpatí s plným signálem | **medián 120 km** (min 32, max 258) — do 100 km u 40 lokalit |
| co koupí 20 km kabelu | medián **195 m** vody — jen 15 strmých lokalit dosáhne ≥ 1 000 m |
| co koupí 50 km | medián **1 141 m** — 64 lokalit ≥ 1 000 m, 27 ≥ 2 000 m |
| **co koupí 100 km — dosah** | medián **2 940 m** — 102 lokalit ≥ 1 000 m, 85 ≥ 2 000 m; mediánová lokalita na ~80 % hloubky svého úpatí |
| šelfové lokality, ve 100 km stále méně než 300 m vody | **13** |
| tlak na sondu, typicky | 1,7–4,0 km ≈ **170–400 bar** (~470 bar v nejhlubší) |
| kabel na lokalitu, maximum | **500 km** — 4–5 sond, každá vlastní trasou k úpatí nebo na 100 km (800 km u prstence se dvěma oblouky); medián 430 km |

**Dosah je 100 km** — dosah modulu (*Spojení*), s výkonem propočteným na 105 km kvůli rezervě
trasy (*Dosah*). **Úroveň 2 je výběr, stavěný lokalitu po lokalitě**, takže rozpočet je na
lokalitu, nikdy součet za celou síť. Do 20 km se nevejde nic. **Páky dosahu**, v tomto pořadí, tam,
kde úpatí leží za ním:

1. **Nejdřív ostrůvek** — pobřežní stanice na ostrůvku na svahu (`../gaia/SITING.md`).
2. **Umístění na zlom svahu** — ~85 % signálu leží těsně pod úpatím, za zlomek kabelu.
3. **Jeden svár uprostřed trasy.**

**Ve vodě je médiem jeden modul: 2 Mb/s, od metru do 100 km**, v jeho tlakově odolné podobě.
Modul 10 Mb/s je 1×9 se vzduchovou dutinou a zůstává na souši. **Hloubka se platí, neobchází
se** — úpatí sedí ve 3–4,7 km, 300–470 bar, a právě to vede k provedení plněnému olejem; ostrůvek
zkrátí kabel, nikdy tlak.

## Minimální ostrovní uzel — jedna věc, dobře udělaná

**Vodotěsná mikrostanice na malém, často neobydleném ostrově či atolu, postavená k tomu, aby
sledovala moře a nedělala nic, co nepotřebuje.** Není to meteostanice; nikdo tam nežije, kdo by
chtěl data o dešti nebo půdě.

- **Smyslem je radar** — dolů hledící **mořský hladinoměr 80 GHz**, táž součástka, kterou Palatine
  používá pro sníh (`../palatine/SENSORS.md`), vzorkovaný v dávkách a průměrovaný přes periodu vln,
  takže tsunami zůstane a větrné vlny se zprůměrují. Vzdutí nebo tsunami stoupá během minut
  a obyčejný radar to vidí.
- **Magnetometr je hluboké rozšíření** — sonda na úpatí ostrovního svahu (`../gauss/ARRAY.md`),
  kde dno padá strmě a blízko — i tak je úpatí s plným transportem 32 km kabelu u nejstrmější
  pobřežní lokality a 120 u mediánu (*Jak daleko*, výše).
- **Seismometr v trubce** — vrtná sonda DN40 od Quake (`../quake/CONSTRUCTION.md`), spuštěná do
  vyvrtané trubky ve skále a zaklínovaná. Je to **nejčasnější varování ze všech tří**: vlna P
  přichází minuty před vodou.
- **Provozní měření (housekeeping) jsou drobná** — barometrický tlak, teplota a náklon. Tlak je za
  pár centů skutečné rozlišovací kritérium pro meteotsunami a bouřkové vzdutí; teplota a náklon
  říkají, že platforma žije a stojí rovně. Žádný srážkoměr, žádný větrný stožár, žádné půdní sondy.

**Dva druhy senzorů mají opačné náklady na usazení:**

| | radar | sondy — magnetometr na dně, seismometr ve skále |
|---|---|---|
| žije | nad vodou, v pásmu příboje | v klidu, na hlubokém dně nebo dole ve vyvrtané trubce |
| potřebuje | tuhou konstrukci, která přežije lámající se vlny, tříšť a nárazy | spustit a navázat — žádná konstrukce v pásmu příboje |
| **náklady na usazení** | **ta drahá polovina** | **ta levná polovina** |

**Elektronika a kabel jsou vyřešené a levné; usazení — kotva, stožár v pásmu příboje, koroze
a obrůstání — stojí víc než všechno ostatní dohromady.** Tam jde úsporné úsilí: jeden levný,
odolný, standardizovaný úchyt a žádná pozlacená elektronika, která by měla ušetřit úchyt, jenž se
stejně musí postavit. **Rozpočet je ~$50–100 k včetně mořského usazení**, oproti ~$4–9 k za pozemní
stanici — stále třikrát až pětkrát méně než profesionální bóje (kotvení DART stojí ~$250–500 k,
většinou za samotné kotvení).

## Soubory

| soubor | obsah |
|---|---|
| [`WHY.md`](WHY.md) | hřbitov — co se zkusilo, co padlo a proč |

## Licence

Hardware: CERN-OHL-S v2 (`../LICENSE-HW`) · Software: MIT (`../LICENSE`) — Copyright (c) 2026 NIC — Native Intellect Community

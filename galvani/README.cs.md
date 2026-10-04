<p align="center">
  <img src="NIC-Galvani.svg" width="200"/>
</p>

★ N.I.C. ★

# Galvani — přenosové desky

[English](README.md) · **Čeština** · [Русский](README.ru.md)

> **Koncept ve fázi návrhu — nic není nakresleno, nic není postaveno.** Tento soubor je doktrína a
> funkce každé desky; součástky, hodnoty a výpočty jsou v [`HARDWARE.md`](HARDWARE.md),
> kusovník v [`BOM.md`](BOM.md), zamítnuté alternativy ve [`WHY.md`](WHY.md). **Je to
> koncept: než se nakreslí schéma, každé číslo zde se ověří proti katalogovým listům součástek
> a proti součástkám samotným.**

**Galvani je přenosová vrstva stanice, postavená jako stavebnice malých desek.** **Napájecí deska**
vyrábí napájení, nebo ho odebírá z kabelu; **komunikační deska** nese spoj. Hostitel — karta,
jednotka, Palatine — mluví s kteroukoli z nich přes krátký plochý kabel a ani jeden neví a neřeší,
jaký přenos visí na druhé straně. Všechno, co opouští skříň, je galvanicky oddělené, trasy po
stožáru nevyjímaje; uvnitř skříně je spoj prostě kabel a deska zdroje tam není vůbec.

**Jedenáct desek, pojmenovaných podle toho, co dělají.** Komunikační — podle toho, co je na kabelu,
podle rychlosti a dosahu: **`G-I-N-025`** 485 po mědi pro NodBus · **`G-I-M-005`** 485 po mědi pro
ModBus · **`G-O-10-10`** sklo, 10 Mb/s do 10 km · **`G-O-2-100`** sklo, 2 Mb/s, jeden modul od metru
do 100 km. Napájecí — podle napětí a konce: **`G-48-S`** a **`G-300-S`** vyrábějí napájení na
zdrojovém konci z 12 V, které odebírají z vodiče; **`G-48-U`**, **`G-300-U-6`** a **`G-300-U-40`**
ho odebírají z kabelu na konci jednotky a dodávají **12 V a nic jiného**; **`G-12-S`** je
galvanicky oddělených 12 V pro kupované zařízení ModBus; **`G-24-S`** je spínaných 24 V pro zátěž,
která není senzorem (čerpadlo). **`S` je ZDROJOVÝ konec — tam, kde se trasa napájí a kde stojí
master, na stanici i u vzdáleného Argusu; `U` je konec JEDNOTKY.** Deska má název a žádné číslo;
název je celá její identita.

> ## TLAKOVÉ DESKY A POZEMNÍ DESKY — DVĚ PROVEDENÍ, NE JEDNO
>
> **TLAKOVÉ: `G-48-U`, `G-300-U-6`, `G-O-2-100` s tlakovým modulem a `G-I-N-025` na konci
> jednotky** — bez osazené výbojky, kondenzátory keramické; pozemní provedení téže desky je takové,
> jaké je. Postavené pro olejem plněné, tlakově vyrovnané pouzdro do
> **350–500 bar**. **Jen plné součástky**: keramické MLCC, rezistory, polovodiče zalisované v plastu,
> vinuté magnetické součástky, které olej prosákne. **Žádný film, žádný elektrolyt, žádný hybridní
> polymer, žádná součástka se vzduchovou dutinou** — relé, krystal, MEMS, modul s víčkem. Tlaková
> deska je zároveň pozemní deskou; opak neplatí nikdy. **Pouzdro se plní pod vakuem, které vytáhne
> vzduch z každého vinutí spolu se vším ostatním**, takže transformátor, který náplň neprosákla
> úplně, není vada.
>
> **JEN POZEMNÍ: `G-48-S` · `G-300-S` · `G-300-U-40` · `G-12-S` · `G-24-S` · `G-I-N-025` na
> zdrojovém konci · `G-I-M-005` · `G-O-10-10` · `G-O-2-100` s katalogovým modulem.** Nesou filmové, hybridně polymerové nebo dutinové součástky.
>
> **`G-O-2-100` je jen pozemní, dokud jsou jeho moduly katalogové součástky** — každý modul 1×9
> má za víčkem vzduchovou dutinu. Tlakový spoj se staví jen s modulem odolným vůči tlaku, získaným
> od výrobce, a ten se zajistí dřív, než se spoj nakreslí.
> **Každá deska mimo skříň je venkovní provedení a kabel se řídí prostředím, ve kterém leží.** Na
> souši poslouží jakýkoli kabel určený k uložení do země. **Ve vodě je kabel stavěný pro vodu** —
> jeho izolace, podélně vodotěsné jádro a pancíř, který unese tah při pokládce do hloubky, v níž
> leží — měděný i optický. Otázkou není rozhraní v dobrém pouzdře; otázkou je kabel (*Pod vodou*,
> níže).

## Doktrína

**Jeden práh a všechno visí na něm: opouští trasa skříň?**

| | uvnitř skříně | **cokoli, co odchází — stožár nevyjímaje** |
|---|---|---|
| bariéra | žádná | **galvanické oddělení, vždy** |
| na čem běží | **bateriová sběrnice tak, jak je, předaná dál jako 12 V** | **48 V**, nebo **300 V** tam, kde 48 neunese zátěž tak daleko; kupované zařízení ModBus bere **12 V** z ostrova desky jednotky, nebo z `G-12-S` na stanici |
| co je na vodiči | **UART · I²C · hodiny a PPS jako vodiče za budičem; časová sběrnice hodin Kronos na M-LVDS** — žádné 485 vůbec | 485 nebo sklo, a začíná to na desce |
| deska | **žádná — kabel** | **napájecí deska a komunikační deska na každém konci** |

Nepočítá se to — rozhoduje blesk a kritériem je **uvnitř, nebo venku, nikdy vzdálenost**: riziko
je to, *kolem* čeho kabel vede. Úder zvedne stožár o stovky kilovoltů za mikrosekundu a vazba do
kabelu vedeného po něm je `I = C·dV/dt` přes kapacitu mezi kabelem a stožárem — deset pikofaradů
proti 10¹¹ V/s dává ampér, takže tři metry nahoru po stožáru jsou horší poloha než pět set metrů
v zemi. Bateriová sběrnice nikdy necestuje: ve chvíli, kdy trasa odchází, odchází přes desku.

**48 V je to číslo.** Stanice si vyrábí vlastní napájení, takže není žádné pásmo, které by se
muselo přijmout, ani vnější zdroj, který by se musel kvalifikovat; napájení odbočky z bateriové
sběrnice končí na 600 m při zátěži 3 W. **300 V je vyšší napájení a volí se podle wattů ×
vzdálenosti, ne podle třídy**; je to jedna jmenovitá hodnota, ne rozsah.

**Jedna záměrná výjimka: koaxiál antény GNSS.** VF signál s předpětím nelze galvanicky oddělit,
takže opouští skříň bez bariéry a platí za to ošetřením na vstupu — svými svodiči
(`../core/blocks/gps-pps.md`). Všechno ostatní, co odchází, je sběrnice nebo napájení a jde přes
desku. Pulzní vodiče radiační hlavice zůstávají uvnitř její sestavy: hlavice počítá na vlastním
procesoru a odpovídá po sběrnici, takže to, co odchází, je obyčejná sběrnice za obyčejnou deskou.

### Spoj uvnitř skříně — kabel, ne deska

Spoj uvnitř skříně nese prostou logiku mezi dvěma hostitelskými deskami: žádný transceiver, žádná
bariéra, žádná kaskáda, žádné zakončení. UART běží proti zemi, takže společná zem je celý
požadavek; sériový rezistor `CLK` sedí u svého zdroje, na hostitelské desce.

**Každá patice ve stanici má stejné zapojení vývodů, takže kabel uvnitř skříně je KŘÍŽENÝ**: `TXD`
na `RXD`, `ID` na `ID_RET`; `CLK/PPS` a `GND` rovně. Vše, co je výstupem na obou hostitelích
(`DE`, `LINE_EN`, `B_DIR`), a vše, co na hostiteli nemá protějšek (`RXD_ECHO`, `SD`, `3,3 V`),
v kabelu není. **Šest vodičů — žíly 1 až 6 datového konektoru, `CLK/PPS` · `GND` · `TXD` · `RXD` ·
`ID` · `ID_RET` — a křížení tvoří dva sousední páry, 3–4 a 5–6, prohozené na jednom konci**:
šestižilový plochý kabel zalisovaný rovně do jednoho `FC-12P` na piny 1–6 a na druhém konci
rozdělený mezi žílami 2–3, 4–5 a 6 a znovu zalisovaný s každým z obou párů otočeným. 12 V se bere
z vodiče na vlastních svorkách každého hostitele; napájecí konektor ve spoji uvnitř skříně nehraje
žádnou roli, protože ve skříni žádná napájecí deska není.

**Pro stavitele: dva kabely, které vypadají stejně, a nejsou.** **Hostitel k desce Galvani —
rovný**, pin 1 na pin 1, plochý kabel zalisovaný skrz. **Hostitel k hostiteli uvnitř skříně —
křížený**, 3–4 a 5–6 otočené na jednom konci. Je to nejstarší uspořádání sériových spojů — DTE a
DCE u RS-232: nestejná zařízení na rovném kabelu, dvě stejná na null-modemu — a deska Galvani je ta
nestejná strana. **Křížený kabel se dělá z plochého kabelu jiné barvy a nese na obou koncích štítek
`X`**, aby se rozeznal na první pohled; rovný plochý kabel mezi dvěma hostiteli nebo křížený k desce
Galvani spojí dva výstupy nebo dva vstupy a čtení `ID` to ohlásí dřív, než se cokoli napájí.

**Každý hostitel čte na `ID` číslo toho druhého** (*`ID` — jedna stupnice*), takže obě strany ještě
před zapnutím vědí, že žádná trasa skříň neopouští a že není žádné napájení ke spínání, a hostitel
zapojený tam, kam nepatří, se přečte jako porucha dřív, než se cokoli napájí. Kabelem nelze dosáhnout
za průchodku: není na něm žádná bariéra a nic, čím by se dala propojkou přidat.

### Napájecí sběrnice — každý hostitel si dělá vlastní a 12 V je vodič

**Ve skříni není žádná deska zdroje.** Akumulátor je uzel 12 V, 10–20 V, a **je to vodič, ne
deska**: každá deska, která si dělá vlastní napájecí sběrnice, ho přijímá na dvou dvoupólových
svorkách, vstupu a vývodu, téhož uzlu (*Konektory*). **Ve skříni opouští 12 V akumulátor přes pole
pojistek, jedna pojistka na desku** — Mayak, Kronos, každá karta, Palatine, Sputnik a každá zdrojová
napájecí deska, každá na vlastní pojistce; vývod vede vodič dál jen tam, kde zdroj sám omezuje svůj
proud, u `G-300-U-40` vzdáleného Argusu. Žádná deska nevede 12 V přes sebe pro jinou desku: port
karty jsou dva ploché kabely a nic jiného, zdrojová napájecí deska bere 12 V na vlastních svorkách.

**Každý vstup 12 V má na svorkách za svou pojistkou `5.0SMDJ14A`.** Snese 14 V a začne vést při
15,6–17,2 V, nad koncem nabíjení akumulátoru a pod absolutním maximem 18 V nejméně odolné
součástky na uzlu 12 V, `TPS629206`. **Pojistka je to, co chrání transil**: obráceně
připojený akumulátor otevře transil v propustném směru a pojistka vypadne; přepětí, které transil
omezuje déle než rázový impulz, ho zahřívá, dokud pojistka nevypadne. Tak či onak přijde vniveč
pojistka, a ne deska. Pojistka je rychlá a **dimenzuje se podle konstrukce, ne podle desky**: pro
každou instalaci, podle vstupního příkonu jištěného obvodu, zkratového proudu akumulátoru a
nejtenčího vodiče k poli pojistek (`../daedalus/CONSTRUCTION.md`) — na desce není napsána žádná
hodnota. Pole patří skříni, ne desce.

**Oba konce trasy nejsou na 12 V stejné a důvodem je proud.** Napájecí deska jednotky 12 V na
svých svorkách *rozdává* — 0,3 A jednotce při 3 W, 3,33 A z `G-300-U-40`. Zdrojová napájecí deska
12 V, ze kterých vyrábí napájení, *přijímá*: `G-48-S` bere 28,4 W, 2,4 A při akumulátoru 12 V a
2,6 A na spodní hranici 11 V; `G-300-S` bere 50,6 W, 4,6 A na téže hranici; `G-24-S` ~3 A, dokud
běží jeho zátěž; `G-12-S` 0,5 A. Nic z toho není proud pro plochý kabel, a proto žádná deska rodiny
nedává 12 V na plochý kabel vůbec.

**Každý hostitel si vyrábí vlastní nízká napětí z 12 V** (`../core/POWER.md`) a nic ve skříni
nereguluje pro nikoho jiného. Napájecí deska jednotky rozdává 12 V a nic nižšího — na žádné desce
Galvani není snižující měnič; komunikační deska nemá svorku ani snižující měnič, bere 3,3 V od
svého hostitele přes konektor.

**12 V, které chce kupované zařízení, je ostrovní výstup napájecí desky jednotky na jejím vývodu**
— okno 10–30 V, 12–24 V nebo 5–24 V je běžná specifikace napájení kupované součástky a 12 V vyhoví
všem třem — nebo, pár metrů od stanice, `G-12-S`: akumulátor přes bariéru a rovnou na krátkou
trasu, jedna deska tam, kde by napájecí měnič a deska jednotky byly dvě. Kde je problémem dosah, a
ne napětí, jde hostitel ven na napájené trase a jeho vlastní vedení za ním zůstávají krátká.

### Dvě volby, a jsou nezávislé

**Napájení se rozhoduje podle wattů × vzdálenosti; komunikační deska podle média × vzdálenosti.**
Trasa 300 V s `G-I-N-025` na dvě stě metrů je přípustná stavba, a stejně tak trasa 48 V
s `G-O-2-100`. Na napájecím kabelu dva průřezy a víc ne: **2× 1,5 mm² a 2× 2,5 mm²**.

**Vypínací bod trasy se měří a měnič je strop.** Při uvádění do provozu se změří rozběhová špička
a provozní zátěž vzdáleného konce a `INA238` zdrojové desky se na ně naprogramuje s rezervou —
registr hostitele pro daný port, přepsaný, když se přidá senzor. Mezi měřením a měničem není žádná
třída zátěže: vzdálený konec při 3 W dostane vypínací bod blízko 4 W, konec při 21 W blízko 24.
**To, co měnič dodá, je zeď**: `G-48-S` ~26 W a `G-48-U` ~25 W na vzdálené straně, takže trasa
48 V nese na vzdáleném konci až ~25 W; `G-300-S` ~47 W, z nichž `G-300-U-40` dodá 40 W a
`G-300-U-6` 6 W. **300 V kupuje dosah, ne watty**: vzdálený konec dál, než kam sahá tabulka pro
48 V, jde na 300 V; zátěž nad ~25 W v jakékoli vzdálenosti potřebuje jiný měnič.

**Jedna zdrojová deska může napájet několik desek jednotek.** Každá deska jednotky přijímá napájení
na jednom páru svorek a předává ho přímo skrz na druhém, takže jednotky se řetězí na jednom
napájecím kabelu. Řetěz je pro zdroj jeden vzdálený konec: jeho zátěže a ztráty každé desky
jednotky se sčítají, strop zdrojového měniče se sdílí, práh `INA238` se programuje na součet a
tabulky dosahu se čtou s celým součtem na poslední jednotce. Jeden `ENABLE` spíná celý řetěz;
jednotka, která se musí spínat samostatně, dostane vlastní trasu. **Segment NodBus mini bere jednu
zdrojovou desku a jeho jednotky se na ní řetězí**: už sdílejí pár segmentu, takže jednotka, která
drží sběrnici, shodí segment bez ohledu na to, co ji napájí, a vlastní napájení pro každou z nich by
nekoupilo žádnou poruchovou doménu, kterou data už beztak neztrácejí. Deska jednotky zkratovaná na
vstupu — transil, který se po rázu prorazil nakrátko — nechá svůj segment potemnělý, dokud se
nevymění; zkrat za jejím měničem ne, protože deska jednotky reguluje svůj vlastní výstupní proud.

```
   A RUN, END TO END — four boards and two cables

   SOURCE END                                            UNIT END

   ┌────────────┐                                        ┌────────────┐
   │  the host  │◀── 12 V, its own terminals             │ the unit's │
   │            │                                        │ own board  │── its own bucks
   └──────┬─────┘                                        └─────┬──────┘
          │ two ribbons                                        │ two ribbons
          │ 3,3 V · logic — the power board taps the 12 V wire │ 12 V from the power board's terminals
      ┌───┴────┐                                          ┌────┴───┐
   ┌──────┐ ┌──────┐                                  ┌──────┐ ┌──────┐
   │ COMM │ │POWER │══ 2-core feed cable ════════════▶│POWER │ │ COMM │
   │ 485/ │ │  S   │   (its own cable on land; in a   │  U   │ │ 485/ │
   │glass │ │      │    hybrid or under water, cores  │      │ │glass │
   │      │ │      │    in the data cable's jacket)   │      │ │      │
   └──┬───┘ └──────┘                                  └──────┘ └───┬──┘
      │                                                            │
      └═══════ 485 pairs or fibre ═════════════════════════════════┘

   …and where the link never leaves the box there is no board at all: the same data socket takes
   the crossed cable to the other host, and the 12 V comes off the wire on the unit's own terminals.
```

## Společná část — co sdílí každá deska

### Konektory

**Port tvoří dva konektory plochého kabelu a na desce, která nese 12 V, dvě svorky — rozdělené podle
toho, co vodič nese.** **Datový konektor** vede mezi hostitelem a komunikační deskou a mezi dvěma
hostiteli ve skříni; **napájecí konektor** mezi hostitelem a napájecí deskou. Zapojení vývodů je
v každé patici stejné, takže deska se zapojí rovně a křížený je jen kabel mezi hostiteli. Ani
jeden konektor nenese napětí, které by mohlo cokoli poškodit, takže záměna konektorů nic nezničí a
ohlásí ji čtení `ID`.

**Datový konektor — 12 pinů, 2×6.** Číslování IDC: liché piny v jedné řadě, sudé ve druhé, a žíla
plochého kabelu *n* přistane na pinu *n*.

| pin | signál | směr | co to je |
|---|---|---|---|
| **1** | `CLK/PPS` | hostitel → deska, nebo deska → hostitel | **jeden pin pro kanál B.** Drátové hodiny 2²² (nebo 2¹⁹) ven, nebo hrana PPS dovnitř — co z toho, rozhoduje patice (*Obrácený kanál*). Na okraji plochého kabelu se zemí vedle sebe; sériový rezistor 33 Ω u svého zdroje |
| **2** | `GND` | — | zpětný vodič, jeden ze dvou |
| **3** | `TXD` | hostitel → deska | kanál A, logická strana je vždy plně duplexní UART |
| **4** | `RXD` | deska → hostitel | kanál A zpět |
| **5** | `ID` | deska → hostitel | **jeden rezistor proti zemi na zapojené desce**, čtený na pinu ADC hostitele proti 10 kΩ 1 % k analogovému napájení hostitele (*`ID` — jedna stupnice*) |
| **6** | `ID_RET` | hostitel → vzdálený hostitel | **vlastní číslo hostitele**, rezistor téhož druhu, pro hostitele na druhé straně kříženého kabelu; nezapojený na desce Galvani a na hostiteli, který se s kříženým kabelem nikdy nesetká |
| 7 | `RXD_ECHO` | deska → hostitel | výstup přijímače kontroly ozvěny — druhý `ISO1452` na mědi, odbočka jen pro příjem na skle; každá jednotka slyší vlastní vysílání (`../core/blocks/nodbus.md`). Nezapojený v patici mastera: master ozvěnu nikdy nekontroluje |
| 8 | `DE` | hostitel → deska | povolení budiče, kolem slotu jednotky; na skle není co spínat, modul je v klidu potemnělý |
| 9 | `B_DIR` | patice → deska | **pevně svázaný v patici, žádný pin procesoru**: na 3,3 V tam, kde tento konec budí kanál B, na zem tam, kde poslouchá; deska nenese žádný pull rezistor (*Obrácený kanál*) |
| 10 | `LINE_EN` | hostitel → deska | spíná galvanicky oddělenou linkovou stranu — `EN` obvodu `SN6505B` na měděné desce; **na skle neosazený**, modul nic nespíná. Na desce stažený 100 kΩ dolů. **Na zdrojovém konci je to GPIO hostitele, jedna síť s `ENABLE` portu; na konci jednotky je to 10 kΩ na 3,3 V v patici a žádný pin procesoru** — linková strana běží, kdykoli běží jednotka |
| 11 | `SD` | deska → hostitel | alarm ztráty světla optického modulu, při ztrátě v H; na mědi neosazený, takže ho hostitel stahuje 100 kΩ dolů. Vedle `3,3 V`, aby se nejpravděpodobnější zkrat četl jako trvalý alarm |
| 12 | `3,3 V` | hostitel → deska | napájení hostitele — celé napájení komunikační desky; 175 mA optické desky v portu mastera je nejvíc a nese ho jeden pin |

`RE#` není pin: přijímač je na desce držen aktivní. `TXD`/`RXD` na 3–4 a `ID`/`ID_RET` na 5–6 leží
vedle sebe, takže kabel uvnitř skříně je šestižilový plochý kabel se dvěma prohozenými sousedními
páry.

**Napájecí konektor — 8 pinů, 2×4.**

| pin | signál | směr | co to je |
|---|---|---|---|
| **1** | `ENABLE` | hostitel → deska | do `EN` měniče, stejná zem, žádný optočlen; **stažený dolů na přijímajícím konci**, takže reset, přeříznutý plochý kabel a potemnělý port nechají port vypnutý — **měnič zdrojové desky je vypnutý, dokud ho hostitel nezapne**. Pin zdrojové napájecí desky, buzený GPIO hostitele; na konci jednotky vede tentýž rezistor na desce ke vstupu, deska běží, kdykoli je napájení přítomné, a pin hostitele je nezapojený. Jeho jediný soused na plochém kabelu je zem |
| **2** | `GND` | — | zpětný vodič |
| 3 | `ID` | deska → hostitel | rezistor desky proti zemi, jako na datovém konektoru — napájecí deska odpoví rezistorem dřív, než je její `INA238` naprogramovaný, i tehdy, když neodpovídá |
| 4 | `SDA` | ↔ | I²C obvodu `INA238` přes `ISO1642` desky; **4,7 kΩ na 3,3 V na hostiteli**, jeden pár na řadič |
| 5 | `A_SEL` | patice → deska | **propojený na L nebo H v zapojení hostitele, žádný pin procesoru**; přechází přes `ISO1642` na `A0` obvodu `INA238`, `A1` uzemněný na desce: **0x40 při L nebo nezapojeném, 0x41 při H** — dvě stejné desky na jedné sběrnici jsou dvě adresy a řadič nese nejvýše dvě napájecí patice. Leží mezi `SDA` a `SCL` |
| 6 | `SCL` | hostitel → deska | hodiny I²C |
| 7 | `ALERT` | deska → hostitel | alarm `INA238`, **při alarmu v H, jeden pin na patici na hostiteli, nikdy wired-OR** — jeden zaseknutý alarm nesmí oslepit ostatní; **100 kΩ proti zemi na hostiteli**, takže prázdná patice se čte jako klidová. Vedle `3,3 V`, aby zkrat na tom místě byl trvalým alarmem |
| 8 | `3,3 V` | hostitel → deska | hostitelská strana `ISO1642`, pár miliampérů |

**Tentýž konektor nese na Mayaku Hermes, měnič BMS/MPPT centrály**: I²C slave, 3,3 V přes
konektor přes vratnou pojistku 0,15 A, `ID` na 0,90, `ENABLE` neosazený (`../hermes/README.md`).
Je to jediná deska na tomto konektoru, která není napájecí deskou.

**Součástka — jeden typ konektoru v celé stanici, ve třech šířkách.**

| | součástka | 2×4 · 2×5 · 2×6 |
|---|---|---|
| **na desce** | **`BX2.54-2xNA`** — dvouřadá stíněná kolíková lišta s krytem, rozteč 2,54 mm, THT, polarizační výřez, bez vyhazovačů | celkem 17,78 · 20,32 · 22,86 mm; 3 A, 550 V AC/min, −55…+105 °C |
| **na kabelu** | **`FC-xP`** — zásuvka IDC pro plochý kabel 1,27 mm, s krytem odlehčení tahu | `FC-8P` · `FC-10P` · `FC-12P`; 1 A, 250 V, −40…+105 °C, 20 mΩ |

Parametry dvojice jsou parametry zásuvky: 1 A · 250 V · −40…+105 °C. **Těla od sebe odlišují šířky —
12 datový konektor, 8 napájecí konektor, 10 časová sběrnice hodin Kronos** (její vlastní konektor,
`../kronos/HARDWARE.md`, táž řada součástek). Žádná druhá rozteč, žádná klíčovaná varianta, žádný
klíč s vyjmutým pinem; nic nezapadá do zámku — plochý kabel drží odlehčení tahu zásuvky a stahovací
pásek. V rámci jedné šířky nestojí špatné zapojení nic a ohlásí ho čtení `ID`; 12 V, jediné místo,
kde špatné zapojení něco zničí, je na jiné součástce. **Deska říká, co je, potiskem** — svůj název
a hodnotu `ID` vedle konektoru, a každá patice říká, co očekává.

**Jedna svorkovnice v celé stanici — Degson `DGPS2.5R-5.0`, objednací kód `10060008518`**: pružinová
svorkovnice do DPS PUSH-SNAP, dva póly, rozteč 5 mm, 0,75–2,5 mm² plný nebo lanovaný vodič
(18–14 AWG) bez dutinky, odizolování 9 mm, 20 A, 400 V (III/2), jmenovité rázové napětí 4 kV,
−40…+105 °C, PA66 UL94 V-0, cínováno. Pružina se dodává otevřená: vodič se zasune na doraz, tlačítko
zaklapne a cvaknutí je kontrolou. **Stojí na každé svorkové pozici rodiny**: dvě svorky 12 V na
každé desce, která vůbec nese 12 V — vstup a vývod, týž uzel — dva páry každé napájecí pozice,
48 V nebo 300 V, ven ze zdrojové desky a dovnitř i ven na desce jednotky, datové páry
komunikačních desek, čtyři vodiče MOD a přívod kupovaného senzoru. Pozice s více než dvěma póly
je tolik dvoupólových svorkovnic vedle sebe. Vodič z terénu vstupuje čelem k průchodce.

**Zleva doprava, ze vstupu na výstup, na každé desce.** Svorky 12 V stojí u levého okraje a cokoli
deska dává ven — napájení, 24 V, 12 V pro větev — vpravo; technik, který otevře skříň, čte kartu
tak, že akumulátor přichází zleva a výstup měniče odchází vpravo, a nikdy nemusí číst štítek, aby
věděl, který konec je který.

**Jeden pól na síť.** Svorkovnice má jeden pól na každou elektrickou síť a žádnou dvojici IN/OUT:
trasa, která pokračuje k další jednotce, se zkroutí do pólu, který sdílí — oba vodiče se odizolují
spolu a zasunou jako jeden.

| pozice | svorkovnice | póly | co |
|---|---|---|---|
| `G-I-N-025` | 4× `DGPS2.5R-5.0` | **8** | čtyři páry datového kabelu |
| `G-I-M-005` | 4× `DGPS2.5R-5.0` | **8** | pár větve na čtyřech pozicích — 4× `A`, 4× `B` — hvězda až čtyř senzorových kabelů |
| `G-12-S` | 4× `DGPS2.5R-5.0` | **8** | 12 V větve na čtyřech pozicích — 4× +, 4× − |
| každá deska, která nese 12 V | 2× `DGPS2.5R-5.0` | **4** | vstup a vývod: + a − v každém směru |
| napájecí pozice | 2× `DGPS2.5R-5.0` | **4** | dva páry, 48 V nebo 300 V |
| výstup `G-24-S` | `DGPS2.5R-5.0` | **2** | dvoužilový kabel zátěže |
| MOD na větvi | 2× `DGPS2.5R-5.0` | **4** | `A` · `B` · `GND` · 12 V, v tomto pořadí pólů |

Komunikace mezi deskami jde po konektorech plochých kabelů, nikdy po svorce; svorky nesou vodiče
z terénu a napájení. Karta nese čtyři póly a nic jiného: svůj vstup 12 V a svůj vývod.

**Zem opouští zdrojovou desku na svorníku, nikdy na svorkovnici.** Střed výbojky je cesta pro kA po
dobu 20 µs a pružinová svorkovnice není součástka pro kA: při 10 kA odpuzuje zúžení proudu
v kontaktní ploše obě plochy silou téhož řádu jako vlastní přítlak pružiny a joule nebo dva
uvolněné v té ploše za tu dobu ji vypálí důlky nebo svaří. Šroubový spoj nemá ani jeden z těch
problémů. Svorník je **pokovený průchozí otvor M4** s ploškou nejméně 10 mm na obou vrstvách a
věncem prošívacích prokovů kolem a skladba je elektrikářská — **šroub · plochá podložka · deska ·
plochá podložka · krimpované kroužkové oko pásku · plochá podložka · pružná podložka · matice, vše
mosazné**. Samotný pásek má 2,5 mm² a délku pár centimetrů: impulz ho ohřeje asi o jeden kelvin,
takže průřez není otázkou, a napětí na něm je dané indukčností jeho délky
(`../daedalus/CONSTRUCTION.md`). Na vzdáleném stanovišti, kde nic není uzemněné, zůstává otvor
prázdný.

### Názvy konektorů — jedno schéma pro celou stanici

**Každý konektor je pojmenován podle toho, co nese a kterým směrem, v potisku i v každém dokumentu;
žádný konektor není J-číslo.** Název je sběrnice nebo druh, směr a číslo tam, kde jich je více:

| část | význam |
|---|---|
| `NB` · `MNB` · `MINI` · `MB` · `TIME` | datové tělo podle své sběrnice: NodBus · MasterNodBus (páteř Mayaku) · NodBus mini · ModBus · RX/TX přijímače + PPS |
| `PWR` | napájecí tělo |
| `12V` | dvě svorky 12 V — jeden uzel, dvě svorkovnice paralelně, žádný směr |
| `FEED` | napájení na kabelu — 48 V, 300 V, galvanicky oddělených 12 V větve, spínaných 24 V — jen na napájecí desce Galvani |
| `LINE CU` · `LINE FO` | data na kabelu, měděné páry nebo vlákno — jen na komunikační desce Galvani |
| `DATA` | vlastní patice plochého kabelu komunikační desky Galvani |
| `OUT` | deska tam budí nebo napájí — strana mastera; **sedí v ní deska Galvani `S`** |
| `IN` | deska tam poslouchá nebo je napájena — strana jednotky; **sedí v ní deska Galvani `U`** |

**Název konektoru říká, co konektor je, nikdy co se do něj zapojuje**; popis vedle něj může říct,
co tam obvykle visí.

| deska | konektory |
|---|---|
| Mayak | `MNB OUT 1`…`4` · `PWR HERMES` · `TIME BUS` · `12V` |
| Hermes | `PWR IN` · `MB OUT` · `UART` · `CAN` · `I2C` |
| Kronos | `TIME IN 1` · `TIME IN 2` · `TIME BUS` · `12V` |
| karta, Bifrost / Argus | `MNB/NB IN` · `PWR IN` · `NB/MINI OUT 1`…`4` · `PWR OUT 1`…`4` · `TIME BUS` · `12V` |
| Palatine | `NB IN` · `PWR IN` · `MB OUT 1`…`4` · `PWR OUT 1`…`4` · `PWR EXT` · `12V` |
| Sputnik | `NB IN` · `PWR IN` · `TIME OUT` · `12V` · `ANT` |
| Polaris | `TIME OUT` · `ANT` |
| Tesla / Pip | `NB IN` · `PWR IN` · `TIME OUT` · `ROD X` · `ROD Y` · `ROD Z` · `12V` |
| Marconi | `NB IN` · `PWR IN` · `LOOP` · `12V` |
| Quake · Quark-Photon | `NB IN` · `PWR IN` · `12V` |
| Quark-Neutron/Positron | `NB IN` · `PWR IN` · `HV` · `12V` |
| Gauss · Pascal | `MINI IN` · `PWR IN` · `12V` |
| Quark-Tubes | `MINI IN` · `PWR IN` · `K1`…`K4` · `RING` · `HV GM` · `HV He` · `12V` |
| MOD — Babel, Pluvius | `MB IN`, čtyři vodiče větve; vlastní konektory senzorů podle svého senzoru |
| MOD zalitý ve skle — Ceres, Sakura | žádné: čtyřžilový kabel je připájený k desce a odizolovaný do svorek desky větve |
| `G-I-N-025` · `G-I-M-005` | `DATA` · `LINE CU` |
| `G-O-10-10` · `G-O-2-100` | `DATA` · `LINE FO` |
| `G-48-S` · `G-300-S` · `G-12-S` · `G-24-S` | `PWR` · `12V` · `FEED OUT` |
| `G-48-U` · `G-300-U-6` · `G-300-U-40` | `PWR` · `12V` · `FEED IN` |

**Řetěz se všude čte stejně**: `… OUT` hostitele → deska Galvani `S` → `FEED OUT` / `LINE` →
kabel → `FEED IN` / `LINE` → deska Galvani `U` → `… IN` jednotky. Ve skříni vede křížený kabel
z `… OUT` rovnou do `… IN`, bez desky mezi nimi.

### `ID` — jedna stupnice

**Deska řekne, co je, dřív, než je napájena.** Jeden rezistor z `ID` proti zemi na desce, přečtený
jednou při oživení na pinu ADC hostitele proti 10 kΩ (1 %) k analogovému napájení hostitele —
3,3 V nebo 1,8 V, tabulka je v poměrech. Dvacet oken po 0,05 s rezervami ±0,025; řada E96 umístí
každý poměr do 0,006 od jmenovité hodnoty a součástky 1 % ho posunou nejvýše o ±0,005. Je to jediné
analogové čtení v celém kontraktu a je záměrně pasivní: buzený pin by potřeboval živou vzdálenou
desku, a pak by se mrtvá deska a prázdná patice četly stejně.

| čtení | rezistor, E96 | co je na druhém konci | kanál B | měření zpoždění |
|---|---|---|---|---|
| **0,00** | zkrat | **porucha** — tento kód nedostane žádná deska ani žádný hostitel | — | — |
| 0,05 | 523 Ω | **volné, drží se volné** — rezerva vedle zkratu | — | — |
| **0,10** | 1,10 kΩ | **Mayak** — přes kabel uvnitř skříně | drát | — |
| **0,15** | 1,78 kΩ | **karta**, Bifrost nebo Argus — jeden kód pro obě; karta řekne, která je, v ohlášení, které následuje | drát | ano |
| **0,20** | 2,49 kΩ | **Palatine** — přes kabel uvnitř skříně | drát | ano |
| **0,25** | 3,32 kΩ | **Sputnik** — přes kabel uvnitř skříně | drát | ano |
| **0,30** | 4,32 kΩ | **`G-24-S`** — spínaných 24 V | — | — |
| **0,35** | 5,36 kΩ | **`GNSS` — rozhraní, na OBOU koncích časového spoje**: každý port, který nese časový proud NMEA, ho předkládá, Kronos na `ID_RET` u svých patic přijímače, a cokoli tam visí, na svém vlastním `ID` | PPS, dovnitř | ano |
| **0,40** | 6,65 kΩ | **`G-I-N-025`** — 485, dva páry, plný duplex | 2²² | ano |
| **0,45** | 8,25 kΩ | **`G-I-M-005`** — 485, jeden pár, poloviční duplex | — | žádné — u dotazované větve se zpoždění neměří |
| **0,50** | 10,0 kΩ | **`G-O-10-10`** — sklo, 10 Mb/s | 2²² | ano |
| **0,55** | 12,1 kΩ | **`G-O-2-100`** — sklo, 2 Mb/s | 2¹⁹ | ano |
| **0,60** | 15,0 kΩ | **`G-48-S`** | — | — |
| **0,65** | 18,7 kΩ | **`G-300-S`** | — | — |
| **0,70** | 23,2 kΩ | **`G-48-U`** | — | — |
| **0,75** | 30,1 kΩ | **`G-300-U-6`** | — | — |
| **0,80** | 40,2 kΩ | **`G-300-U-40`** | — | — |
| **0,85** | 56,2 kΩ | **`G-12-S`** | — | — |
| **0,90** | 90,9 kΩ | **Hermes** — na napájecím konektoru Mayaku | — | — |
| 0,95 | 191 kΩ | **volné, drží se volné** — rezerva vedle rozpojení | — | — |
| **1,00** | rozpojeno | **nic není zapojeno** | — | — |
| cokoli jiného | mimo všechna okna | **porucha**, hlášená jako porucha | — | — |

**`ID` pojmenovává rozhraní, nikdy protějšek — a hostitel se stejně ptá.** Pod 0,40 je hostitel
přes kabel uvnitř skříně: žádné napájení ke spínání, žádná deska k povolení. 0,40 až 0,55 je
komunikační deska a hodnota říká médium a stupeň, který dokáže nést — kontrola, že usazená deska
unese to, co karta posílá, nikoli to, co vybírá hodiny; karta přidělí všem svým čtyřem portům jeden
stupeň podle své role (`../bifrost/HARDWARE.md`). 0,60 a výš a 0,30 je napájecí deska: konec
jednotky se dozví, na jakém měniči visí, zdrojový konec, který měnič budí, a tedy které prahy
`INA238` naprogramovat — a odpoví i tehdy, když `INA238` neodpovídá. Kdo je na druhé straně, vyplyne
z dialogu, který následuje, i tam, kde kód zužuje možnosti na jedinou: rezistor říká, že je něco
zapojeno, jen odpověď říká, že je něco živé. Jeden kód slouží každé jednotce na časovém spoji,
včetně té, kterou někdo vymyslí později.

**`ID_RET` je vlastní číslo hostitele.** Každý hostitel nese rezistor svého kódu z `ID_RET` proti
zemi — Mayak svých 1,10 kΩ jako kdokoli jiný, protože 0,00 je kód poruchy — a křížený kabel ho
přivede na `ID` druhého hostitele. Deska Galvani nese rezistor jen na `ID`; identifikace z desky je
jednosměrná, ven, protože deska nemá procesor, kterým by četla. Hostitel, který vždy stojí jen za
deskou Galvani, nenese žádné číslo a s kabelem se nikdy nesetká.

### Telemetrie — `INA238` a `ISO1642` na každé napájecí desce

**Jeden měřicí obvod na každé napájecí desce, a je povinný: `INA238` s bočníkem na straně země a
`ISO1642`, který přenáší jeho I²C oběma směry a `ALERT` nahoru přes bariéru; `A_SEL` jím prochází
dolů na `A0`.** Hostitel čte napětí sběrnice a proud přes konektor a programuje prahy podle toho, co
`ID` řeklo o usazené desce. **Měřicí strana běží na ostrově, který si deska sama vyrábí z 3,3 V
hostitele na napájecím konektoru** — `SN6505B` + `750313734` + 2× `PMEG10020ELR`, tentýž měnič jako
napájení linkové strany komunikační desky a totéž vinutí 5 kV, vždy zapnutý a nikdy za `ENABLE`,
takže potemnělý port se čte jako 0 V, a ne jako ticho. Každá deska rodiny tedy má jedno napájení,
3,3 V hostitele, a tam, kde ji potřebuje, si z něj dělá vlastní galvanicky oddělenou kopii.
**Bočník je snímací rezistor 1210 s kovovým odporovým prvkem na rozsahu ±40,96 mV** — 50 mΩ na 48 V
a 12 V, 200 mΩ na 300 V, 20 mΩ na 24 V — a **`VBUS` je dělič se spodní větví 100 kΩ na 48 V (horní
90,9 kΩ) a 300 V (3× 422 kΩ), na 12 V a 24 V pin přímo**, všude 10 nF na pinu; převod 4,12 ms ×
128, 16 bitů bez šumu, `ALERT` do 4 ms. **Jen `G-300-S` nese druhý `INA238`, hlídač svodu**, který
čte střed výbojky přes 8,84 MΩ a 7,84 MΩ obyčejných rezistorů 1206 — zdrojový konec trasy 300 V je
jediné místo, kde se to dá měřit, a jediné místo, kde se to měřit musí.

| deska | kde sedí bočník | co hlásí |
|---|---|---|
| **zdrojové desky** — `G-48-S` · `G-300-S` · `G-24-S` · `G-12-S` | **na VÝSTUPU, na galvanicky oddělené straně** — trasa je na druhé straně transformátoru a součástka musí vidět, jak trasa klesá: přetížený měnič zkracuje své impulzy a výstup se propadá, zatímco proud sedí na limitu | napětí a proud trasy; **nad** naprogramovanou zátěží je porucha, `ALERT` → hostitel shodí `ENABLE`; **pod** minimem je suchá trasa nebo přeříznutý kabel |
| **desky jednotek** — `G-48-U` · `G-300-U-6` · `G-300-U-40` | **na VSTUPU** — napájení tak, jak přichází | co přichází na vzdálený konec; čte to hostitel na konci jednotky a hlídání svodu na 48 V je porovnání obou konců |
| **`G-300-S`, druhý `INA238`** | mezi trasou a zemí | **hlídač svodu, na 300 V povinný**: porušení izolace na plovoucí trase neodebírá žádný proud a nic nevybaví, dokud obvod neuzavře druhé porušení nebo člověk; součástka čte svodový proud ze zdrojového konce každou minutu. **Při zpětném vedení mořem trasa neplave a hlídač nemá co číst**; trasa se hlídá podle svého proudu, jako trasa 48 V |

**Trasa 48 V se hlídá odečtem** — co opustí stanici a nedorazí, uniklo; žádná součástka se
nepřidává. Trasa 12 V nebo 24 V se nehlídá: je v mezích 30 V DC, limitu, který SELV dodrží i při
ponoření. Každý práh je naprogramovaný registr `INA238`, nikdy vlastnost měniče; **v celé rodině
není nikde žádná pojistka** — vratná pojistka se znovu sepne do téhož oblouku a nic nehlásí, kdežto
`ALERT` a `ENABLE` poruchu odstraní a řeknou to.

### `ENABLE`, `LINE_EN` a tři stavy

**`ENABLE` je pin zdrojové napájecí desky a ničí jiný.** Hostitel, kterému port patří, ho budí
jedním GPIO; rezistor stahující dolů na přijímajícím konci znamená, že odpojený nebo mrtvý hostitel
nechá port vypnutý. Na konci jednotky vede tento rezistor ke vstupu: deska běží od chvíle, kdy
napájení dorazí, nikdy si neodpojí vlastní napájení a resetuje se sama přes své watchdogy.
**`LINE_EN` na datovém konektoru spíná galvanicky oddělenou linkovou stranu měděné desky a na skle
není osazený**; na hostiteli jsou `ENABLE` a `LINE_EN` portu jedno GPIO, takže napájení a linková
strana se vypínají spolu a neexistuje stav, ve kterém by komunikační deska běžela do potemnělého
napájení. **Nic nad portem `ENABLE` nebudí**: vzdálený hostitel se vypíná, resetuje nebo uspává
povelem po sběrnici, který vykoná procesor, jemuž ta věc patří.

| stav | jak | co se stane |
|---|---|---|
| **běží** | vůbec nic | stanice nešetří energii, a to je záměr návrhu: každé napájení bylo dimenzováno na zátěž vlastní desky, takže nikde není žádná politika, žádné střídání pracovního cyklu a žádný práh |
| **vypnuto, za portem** | `END` na sběrnici, pak vypnuté hodiny, pak **`ENABLE`** dolů na zdrojové napájecí desce (`../core/PROTOCOL.md` §7, §10) | napájení zmizí a vzdálený konec neodebírá vůbec nic; na zdrojovém konci zbývá klidový odběr samotného měniče. Je to také jediný reset, který vyčistí součástku bez resetovacího pinu |
| **vypnuto, ve skříni** | hluboký spánek, který povelem nařídí Mayak | nad deskou uvnitř skříně není žádný `ENABLE` a žádný se nechce — akumulátor je vodič, každá deska z něj odebírá; vypnutí stojí miliwatty a každá deska se probudí po spoji, který už má |

**Port naběhne VYPNUTÝ a zapne se až po zapojení trasy, nikdy dřív.** GPIO `ENABLE` každého
hostitele startuje v L. Měniče rodiny jsou flybacky bez optočlenu: čtou výstup přes transformátor
na primární straně, což je to, co ruší optočlen a čtvrtý vodič přes bariéru — a znamená to, že
nezatížený výstup nedrží dole žádná smyčka: pod minimální zátěží měniče napětí stoupá, dokud
transil nezačne vést, a sedí na jeho koleni. Nic se nezničí, součástky vzdáleného konce jsou na
toto omezení dimenzované, ale napětí už není to ze štítku a zařízení zapojené do již běžícího
prázdného portu se setká se zvýšeným. **Tedy: zapojit trasu, zjistit, co je na vzdáleném konci, a
pak povolit port.** V provozu port buď běží s plnou zátěží, nebo je potemnělý se zastaveným
měničem; prázdný povolený port je jediný stav, kdy napětí na jmenovité hodnotě drží obsluha, a ne
návrh.

**Žádná deska nespíná napájení vlastního procesoru a žádné napájení na žádné desce nemá svůj `EN`
na pinu**; co deska vypíná, vypíná na svých součástkách. **Nic nespíná optický modul**: zrušená trasa
vezme s sebou modul vzdáleného konce a na blízkém konci stojí laser, který zůstane svítit, méně než
součástka, která by ho spínala.

### Obrácený kanál — vlastnost patice

**Kanál A nese data, vždy. Kanál B je jednosměrná linka, jejíž směr nastavuje patice** — hodiny ven
z karty, PPS dovnitř do hodin Kronos, nebo cokoli jiného, co chce vzdálená sestava poslat jedním
směrem. Žádná deska Galvani toto nastavení nenese a žádný pin procesoru ho nečte ani nebudí:
**patice pevně váže `B_DIR` — na 3,3 V tam, kde hostitel budí kanál B, na zem tam, kde poslouchá** —
a deska nenese žádný pull rezistor. H u každého `NB/MINI OUT` a každého `TIME OUT`; zem u každého
`… IN` a každého `TIME IN`. Hostitel ví, kterou patici vlastní, už svou konstrukcí, a stav, kdy
kanál budí oba konce, nelze vůbec vyjádřit.

- **Měď, `G-I-N-025`: žádná součástka navíc.** `ISO1450` kanálu B má `D` i `R` na pinu `CLK/PPS`
  a `DE` i `RE#` na `B_DIR`. H: budič je zapnutý a `R` je ve vysoké impedanci, takže hrana hostitele
  jde ven. L: budič je vypnutý a `R` budí pin k hostiteli. Dva výstupy se na pinu nikdy nesetkají.
- **Sklo, `G-O-10-10` a `G-O-2-100`: dvě hradla.** `RD` modulu nemůže přejít do vysoké impedance,
  takže pin je připojen přes **`74AUP1G126`** z pinu na `TD` (`OE` aktivní v H) a **`74AUP1G125`**
  z `RD` na pin (`OE` aktivní v L) a **táž síť `B_DIR` nese obě `OE`, `ON` obvodu `TPS22917` na `VccT`
  a obvodu `TPS22917L` na `VccR`** — čtyři vstupy, jedna úroveň: H napájí a připojuje vysílač, L
  přijímač. Každé hradlo obnoví hranu a oddělí pin (`CI` 0,9 pF) a s `IOFF` pin nenapájené sekce nic
  nezatěžuje. 100 kΩ proti zemi na `TD` a na vstupu `125`, aby ani jeden neplaval, když je jeho
  hradlo zavřené.
- **Ani jedno médium laser neotáčí.** Na skle má každý konec na kanálu B vysílač i přijímač — čtyři
  vlákna na spoj, dvě u BiDi — a směr rozhoduje, který laser mluví.
- **Použití se nikdy nemíchají.** Osazený pár je spoj sběrnice, nebo spoj obráceného kanálu, nikdy
  obojí; co jede po kanálu B, je věcí firmwaru.

### Ochranná kaskáda

**Jedna buňka na každém napětí a mění se jen stupně. Od kabelu: plynová výbojka → sériový prvek →
transil.** Sériový prvek sedí *mezi* výbojkou a omezovačem: propustný zbytek výbojky nechá padnout
na impedanci, takže transil drží jen zbytek — tlumivky 22 µH na napájení, 10 Ω na datovém páru,
jeden na každou žílu, žádné vázané vinutí. **O tom, co se osadí, rozhoduje KONEC KABELU, ne to, o
jakou desku jde:**

| | plynová výbojka | sériový prvek | transil |
|---|---|---|---|
| **každá deska na ZDROJOVÉM konci** | **ano** — tříelektrodová, na společný zemnicí bod | ano | ano |
| **každá deska na konci JEDNOTKY** | **žádná** | ano | ano |
| **optická deska, na kterémkoli konci** | — | — | — po vlákně nic elektrického nepřichází |
| **MOD na větvi ModBus** | žádná | 2× 10 Ω | `SM712` na páru, 2× `5.0SMDJ18A` na 12 V |

**Konec jednotky nenese výbojku, protože není nic, do čeho by se mohla vybít.** Kroucený pár
nenechává téměř žádnou smyčku, takže ráz je souhlasný a souhlasný ráz na plovoucím konci nemá kudy
téct; tříelektrodová součástka degeneruje na dvě jiskřiště v sérii, jakmile její střed plave. Co
zbývá, je rozdílový zbytek, a ten patří transilu. Vzdálený konec záměrně neomezuje souhlasné napětí
vůbec: omezuje ho vlastní impedance trasy (~110 Ω, 600 µH a 24 Ω na kilometr) a zastaví ho bariéra.
**Vzdálené stanoviště uzemněné není a nebude** — špatná zem je horší než žádná; zdrojová deska tam
nese stopu pro výbojku a pásek nechává nezapojený, a rozestup ramen brání tomu, aby jeden úder
vzal všechna čtyři ramena (`../daedalus/CONSTRUCTION.md`).

**Součástky — jedna řada výbojek, šest kódů transilů, jedna sériová dvojice.** Výbojka je
tříelektrodová `2036-xx-SM` od Bournsu: **`2036-07-SM`** na 48 V a na každém datovém páru,
**`2036-30-SM`** na 300 V. **Tříelektrodová výbojka nezapálí obě jiskřiště najednou** — jedno
zapálí, druhé následuje, a několik set nanosekund stojí celý rozdíl napříč párem; právě na to jsou
sériový prvek a transil a oba jsou osazené na každé pozici. Transily jsou z řady `5.0SMDJ`, dva
paralelně na každé pozici, která se setkává s kabelem: **`5.0SMDJ54A`** na 48 V (omezení ~87 V,
kterému se přizpůsobují všechny navazující dimenzace) · **`5.0SMDJ350A`** na 300 V (~565 V) ·
**`5.0SMDJ18A`** na neregulovaných 12 V (výstup `G-12-S`, vstup MOD) · **jeden `5.0SMDJ12A`** na
regulovaném výstupu 12 V desky jednotky, který se s žádným kabelem nesetkává · **`5.0SMDJ28A`** na
24 V · **jeden `5.0SMDJ14A`** na každém vstupu 12 V ve skříni, za jeho pojistkou (*Napájecí
sběrnice*). Datový pár dostává **2× 10 Ω a `SM712`**, na každé měděné desce a na každém MOD.
**Výbojka 75 V na napájení 48 V sama zhasne; výbojka 300 V ne** — tam je zapálená výbojka zkratem
napříč napájením, dokud ji nepodrží proudový limit měniče a zdrojový konec napájení neodebere:
`INA238` vidí propad, zvedne `ALERT`, hostitel shodí `ENABLE`. Tento sled je povinný, protože
výbojka udržovaná v trvalém oblouku je mrtvá výbojka.

**Deska je obětovaná součástka.** Linkový kabel končí na desce, kaskáda žije na desce a na
zdrojovém konci výbojka svádí na společný zemnicí bod páskem třídy mm², dlouhým pár centimetrů — při
hranách kA tvaru 8/20 µs přidává každý metr vodiče (~1 µH) kilovolty `L·di/dt`, takže deska se
montuje u vstupu do skříně vedle zemnicího bodu. Blesk sní desku; hostitel a plochý kabel přežijí a
deska se vymění z krabice s náhradními díly.

### Kabel, zem a zakončení

**Datový kabel je UTP Cat 6, venku v plášti z PE odolném proti UV — skladová položka.** Kategorie 6
podle ISO/IEC 11801 nebo TIA-568, **plná holá měď, 23 AWG** — **nikdy hliník plátovaný mědí (CCA)**,
který se prodává pod stejným označením za poloviční cenu, má asi o polovinu větší odpor smyčky, než
s jakým jsou počítány tabulky dosahu, a koroduje na každém zakončení. Plášť je polyetylen
s UV stabilizátorem, černý nebo značený pro venkovní použití; vnitřní plášť z PVC za pár let
popraská.
485 je
sběrnice: A na A, B na B, nikdy se nic nekříží; žíla páru s bílým pruhem je A, plná žíla B.
**Napájení vede vlastní dvoužilový kabel — 2× 1,5 mm² nebo 2× 2,5 mm², třída 300/500 V, vlastní
průchodka**, plná = +, pruhovaná = zpětný vodič. Díky tomu zůstanou všechny čtyři páry datům a
každá trasa NodBus je plně duplexní; na souši vedou do každé napájené trasy dva kabely. **Hybridní
kabel je volitelný** — čtyři páry a dvě napájecí žíly v jednom plášti, páry pořád všechny čtyři pro
data — a stavitel vezme to, co trh nabízí; základní sestavou jsou dva kabely.

| pár | nese |
|---|---|
| oranžový | **data TX A/B** — sloty jednotek, až osm budičů řízených `DE` |
| modrý | **hodiny A/B** |
| zelený | **signálová zem** — obě žíly; každý ostrov linkové strany tu váže svou GND |
| hnědý | **data RX A/B** — kanál mastera, jeden budič, nikdy se neotáčí |

**Zem je vlastní pár, obě žíly, a nikdy nenese proud napájení.** Souhlasné okno −7…+12 V podle
TIA-485 je definováno vůči místní zemi transceiveru, takže galvanicky oddělený ostrov musí mít tuto
referenci přivedenou, a reference, která nese napájecí proud, žádnou referencí není. **Zem se
spojuje se zpětným vodičem napájení přesně v JEDNOM bodě — na zdrojovém konci.** Druhé spojení
kdekoli udělá ze země paralelní zpětný vodič napájení; ostrov na konci jednotky váže GND své linkové
strany na zelený pár a na nic jiného. Na větvi ModBus spojuje kupovaný senzor svou datovou referenci
s mínusem svého napájení interně — to je vzdálené spojení, takže tam je ostrov stanice ten konec,
který se přizpůsobuje. Na skle běží obě vlákna plně duplexně už konstrukcí a modul je v klidu
potemnělý.

**Větev ModBus je jiný kabel: čtyři vodiče v jednom plášti — `A`, `B`, 12 V, GND** — protože
kupovaný senzor přichází s tímto kabelem už nasazeným. Na stanici se odizoluje a rozdělí na dvě
desky: pár na `G-I-M-005`, +/− na `G-12-S`, každá se čtyřmi pozicemi, takže větev je u desek hvězda
až čtyř senzorových kabelů a víc než čtyři se řetězí u senzoru. Vlastní kabely senzorů 0,35–0,5 mm²
dávají desítky metrů; **50 m je strop, který stanovuje neregulovaných 12 V větve**
(`../core/blocks/modbus.md`). Klasický ModBus zůstává uvnitř větve na mědi; cokoli, co musí stát
daleko, je jednotka NodBus nebo NodBus mini za odbočkou, a pozemek, který by chtěl delší větev,
obslouží hostitel, který k němu jde ven na napájené trase.

**Zakončení.** Každý pár 485 na každé měděné desce nese **2× 10 Ω v sérii, vždy** — sedí před
`SM712` a půlí jeho proud, než výbojka zapálí, a přijímací vstup má ≥ 12 kΩ, takže 20 Ω v sérii stojí
čtvrtinu amplitudy proti rezervě několikanásobku prahu přijímače. **Zakončovací rezistor, 80,6 Ω
napříč párem za 10 Ω, se osazuje na první a poslední desce segmentu a na žádné desce mezi nimi** —
u spoje bod–bod jsou to oba konce, u řetězu deska na zdrojovém konci a deska poslední jednotky.
80,6 + 2 × 10 je těch ~100 Ω, které chce UTP; ne 120 Ω, které patří vyhrazenému kabelu pro 485,
jejž tento projekt nepoužívá. **Rezistor je na desce a JUMPER na dvoupinové liště 2,54 mm ho
připojí napříč párem — jedna lišta na pár**: tři na `G-I-N-025`, jedna na `G-I-M-005`. Jumper se
nastavuje při uvádění do provozu spolu s kabelem a ztracený jumper selže na neškodnou stranu —
nezakončený krátký segment obvykle pořád běží, kdežto rezistor ponechaný na prostřední desce dá na
linku třetích ~100 Ω a nic v terénu to na měřáku neukáže. Na větvi ModBus při 19 200 Bd, do jejího
stropu 50 m, zůstává jumper na zdrojovém konci rozpojený a vzdálený konec je věcí kupovaného
senzoru; jen větev provozovaná rychleji ho zapojí (`../core/blocks/modbus.md`). **Žádný průběžný
konektor mimo skříň** — nic na mokré straně nemá kontakty; průchodky následují kabely, dvě tam, kde
deska řetězí, jedna tam, kde končí. Zalitá sonda nemá svorku, je vždy poslední jednotkou na svém
segmentu a zakončení nese uvnitř.

### Pod vodou

**Pod vodou jede napájení ve vlastním plášti datového kabelu** — jeden kabel k pokládce, ne dva,
protože v moři je nákladem pokládka. **300 V všude, kde je zpětným vodičem moře**, protože vlastní
volty zpětné cesty — polarizace elektrod, telurické pole moře — zůstávají jeho malou částí jen při
několika miliampérech; **na dvou vodičích po měděné trase poslouží stejně dobře 48 V**. Dva způsoby,
jak ho vést:

| | **dva vodiče** | **jeden potenciál, zpětná cesta mořem** |
|---|---|---|
| kde | sladká nebo slaná voda | **jen slaná voda** — sladká voda vede příliš málo, aby byla zpětnou cestou |
| napájení | dvě izolované žíly | jeden izolovaný vodič — žíla, stínění, plášť, nerezová trubka: cokoli, co kabel nese izolované |
| zpětná cesta | druhá žíla | elektroda na každém konci: titanová katoda u pouzdra, anoda na břehu — MMO nebo vysokokřemičitá litina, nikdy obyčejná ocel — v půdě, která zůstává vlhká |
| polarita | jako na souši | **vodič záporný**, takže porušení jeho izolace je katodické a nic nerozpouští |

**Data jsou na mědi pro blízkou sondu a na skle dál.** Měď jsou čtyři páry v témže plášti, **Cat 5e
nebo Cat 6 a nic horšího**, na segmentu mini do 1 km (*Dosah*); sklo jsou vlákna modulu 2 Mb/s
v tlakovém provedení, do 100 km. Zemní pár a jeho jediné spojení jsou jako na souši; s mořem jako
zpětnou cestou je zpětným vodičem napájení pouzdra jeho elektroda.

**Elektroda stojí stranou od pouzdra**: na katodě vzniká vodík a pouzdro vodík drží venku už svou
konstrukcí. **Vedle magnetometru je proud polem** — `μ₀I/2πr`, 2 nT na 1 m pro 10 mA — takže kabel a
elektroda zůstávají na kabelovém konci pouzdra, délku trubky daleko od cívek. **Porušení se čte
jako proud**: u obou způsobů přidá mořská voda cestu — při zpětné cestě mořem jediná známka, protože
taková trasa neplave a hlídač svodu `G-300-S` nemá co číst — a `INA238` zdrojové desky vidí, jak
odběr roste nad naprogramovanou zátěž; menší svod se ukáže jako rozdíl mezi zdrojovým koncem a
součtem desek jednotek na trase. **S mořem jako zpětnou cestou mohou zdrojové desky několika tras
sdílet jednu anodu**: vodič je záporný, bočník sedí v něm a `INA238` každé desky čte jen proud
vlastní trasy.

### Dosah

**Měď nese trasu do 500 m; dál je to vlákno.** Dosah na mědi je dosahem hodinového kanálu: při
2²² Hz dopadne jeho základní harmonická na práh přijímače ve 500 m na Cat 6, což dává **250 m jako
doporučení a 500 m jako maximum** — `G-I-N-025` je pojmenovaná podle prvního. **Segment NodBus mini
dosáhne dvakrát dál**: jeho hodiny jsou 2¹⁹ a data 2²⁰, obojí se základní harmonickou ~0,52 MHz, kde
pár ztrácí ~1,5 dB/100 m proti ~4 při 4,19 MHz — tentýž rozpočet dopadne na práh při ~1,3 km, takže
**500 m doporučení a 1 km maximum** — pro osamocenou jednotku: řetězený segment provozuje svá data
na 2²¹, ~1,05 MHz, a na ten se tato čísla nevztahují. **Větev ModBus má 50 m**, omezená svými 12 V,
ne 485. **Na skle je doporučená vzdálenost polovinou vypočtené** a smíšená trasa je omezena svou
nejkratším ramenem.

**Vzdálený konec je jedna zátěž a stavitel ji sečte**: měřicí jednotka má 2 až 3 W podle toho, co
je osazeno, optická komunikační deska asi 0,5 W, kupované zařízení na vývodu 12 V tolik, kolik
odebírá, hostitel se čtyřmi větvemi ModBus ~7 W s běžnou sadou senzorů a až ~21 W s větvemi na
jejich limitu. **Tabulky říkají, co deska jednotky dodá na 12 V**, se stropem zdrojového měniče,
`I²R` mědi a účinností desky jednotky už v sobě. Metoda: zdrojový měnič dodává `P_s` při `V₀`,
odebírá `I = P_s/V₀`, měď si vezme `I²R`; tam, kde je kabel tak dlouhý, že zátěž s konstantním
příkonem nelze napájet vůbec, platí místo toho kritérium `V_end = 75 % V₀` a `P = 0,1875·V₀²/R`;
trasa dodá menší z obou hodnot minus ztrátu desky jednotky. Odpor smyčky, obě žíly při 20 °C:
**22,93 Ω/km na 1,5 mm², 13,76 Ω/km na 2,5 mm²**.

**48 V** — `G-48-S` při ~26 W do `G-48-U` při ~90 %:

| délka | 1,5 mm² | 2,5 mm² |
|---|---|---|
| 100 m | **22,8 W** | 23,3 W |
| 250 m | **21,9 W** | 22,5 W |
| 500 m | **20,4 W** | **21,6 W** |
| 1 km | 17,0 W | 19,8 W |
| 2 km | 8,5 W | 14,1 W |
| 5 km | 3,4 W | 5,7 W |

**300 V** — `G-300-S` při ~47 W do `G-300-U` při ~91,5 %:

| délka | 1,5 mm² | 2,5 mm² |
|---|---|---|
| 1 km | **42,5 W** | 42,7 W |
| 5 km | 40,4 W | 41,5 W |
| 10 km | 37,9 W | 39,9 W |
| 20 km | 32,7 W | 36,9 W |
| 30 km | 22,4 W | 33,8 W |
| 50 km | 13,5 W | 22,4 W |
| **100 km — strop** | **6,7 W** | **11,2 W** |

**Zátěž se řídí vzdáleností.** 40 W, které deklaruje `G-300-U-40`, je hodnota pro blízké pole a
tabulka je to, co dorazí: na 5 km na 2,5 mm² je trasa pořád unese, na 20 km je to 37 W, na 50 km
22. Když se trasa prodlouží, deklarované watty vzdáleného konce se znovu ověří proti nové délce.
Na 48 V omezí měděné 485 trasu na 500 m dávno předtím, než ji omezí napájení, takže řádky za tím
platí pro čistě optickou sestavu. **Jeden skok končí na 100 km a končí ho laser**: dlouhý modul
`G-O-2-100` je katalogová součástka na 100 km a delší není. Napájení má za tímto bodem rezervu a ta
se nevyužívá; měření zpoždění nikdy neomezuje, časovač neaktivního portu počítá 2 ms, asi 200 km
cesty tam a zpět. Za 100 km je odpovědí vzdálený Argus, který napájení znovu transformuje.

**Kam jdou watty.** Dva podíly platí na každém napětí a průřezu: na maximální vzdálenosti
(`V_end` = 75 % `V₀`) kabel ztratí 25 % `V₀` a spálí 33 % toho, co dostane vzdálený konec; na
doporučené vzdálenosti, poloviční, 10,4 % a 11,7 %. Trasa 300 V na 2,5 mm² na 10 km, propočtená:
akumulátor → `G-300-S` 50,6 W dovnitř, 47,0 ven; měď, smyčka 138 Ω při 0,157 A, 3,4 W;
`G-300-U-40` → 12 V, 39,9 W ven — 79 % z akumulátoru na 12 V vzdáleného konce. Tytéž watty na
stejnou vzdálenost by na 48 V potřebovaly 39× víc mědi. Když vzdálenost nestačí, odpovědi v pořadí
jsou větší průřez, pak 300 V, pak akumulátor na místě — nikdy větší ostrov.

**Které desky trasa potřebuje — tři otázky, v pořadí:**

| otázka | odpověď |
|---|---|
| opouští skříň? | **ne** → křížený kabel, 12 V z vodiče · **ano** → napájecí deska a komunikační deska na každém konci |
| jak daleko a po čem? | **měď do 500 m, segment mini do 1 km** → `G-I-N-025` · **dál** → sklo: **odbočka NodBus na `G-O-10-10`, do 10 km a ne dál** — její hodiny 2²² se do modulu 2 Mb/s nevejdou · **segment mini na `G-O-2-100`**, od metru do 100 km |
| unese 48 V zátěž na tu vzdálenost? | **ano** → `G-48-S` + `G-48-U` · **ne** → `G-300-S` + `G-300-U-40`, nebo `G-300-U-6` v pouzdře |

| vzdálený konec | komunikace | napájení |
|---|---|---|
| měřicí jednotka v běžné vzdálenosti | `G-I-N-025` | 48 V |
| totéž za 500 m, do 10 km | `G-O-10-10` | 48 V; 300 V tam, kde 48 zátěž neunese |
| segment mini-NOD, který přeroste měď | `G-O-2-100` | 48 V; za hranicí toho, co unese 48 V, `G-300-U-6` |
| větev ModBus — kupované senzory, MOD | `G-I-M-005` na zdrojovém konci; senzor si přináší vlastní 485 | 12 V větve: `G-12-S` na stanici, vývod desky jednotky na napájené trase |
| zátěž, která není senzorem (čerpadlo) | žádná — je to zátěž, ne jednotka | `G-24-S` na vyhrazené napájecí patici hostitele, až ~33 W |
| vzdálený Argus, jeho segmenty optické | `G-O-10-10` | 300 V — žádá si ho vybavení, ne vzdálenost |
| hodinová trasa ke Kronosu — vzdálený Sputnik, Pip | komunikační deska s kanálem B na každém konci, dovnitř u Kronosu | žádné — vzdálený konec je napájen vlastní trasou NodBus |
| pouzdro pod vodou, blízko — do 1 km | `G-I-N-025`, konec jednotky, a tedy bez výbojky — tlakové provedení — páry v mořském kabelu | 48 V na dvou vodičích, `G-48-U`; 300 V, `G-300-U-6`, se zpětnou cestou mořem — *Pod vodou* |
| pouzdro pod vodou, daleko | `G-O-2-100`, tlakový modul | 300 V, `G-300-U-6` — *Pod vodou* |

**Čtěte zátěž na vzdáleném konci, ne název jednotky.** Vzdálený hostitel s plným pozemkem senzorů
a vzdálená měřicí jednotka jsou pro tuto rodinu tentýž problém.

### Vzdálený Argus — vzdálený konec, který je opět zdrojem

**Vzdálený Argus s optickými segmenty po nich nemůže předávat napájení**, takže každá vzdálená
jednotka je vlastní stanoviště a rozpočet vzdáleného Argusu je jeho pět blízkých konců plus cokoli,
co nesou jeho odchozí trasy. Napájení znovu transformuje: `G-300-S` na stanici napájí `G-300-U-40`
u vzdáleného Argusu, jehož svorky 12 V jsou vodičem stanoviště — svorky karty Argus, pak čtyři
`G-300-S` pro odchozí trasy, vývod za vývodem, 3,33 A na vodiči a nikdy na plochém kabelu; dosah se
na každém takovém stanovišti nuluje. Samotné čtyři optické porty odebírají ~2,6 W (4 × 578 mW na
3,3 V ÷ 0,9), desetinu z ~26 W `G-48-S`, a celá karta ~3,2 W — ještě než se započítají jednotky
jejích segmentů — **vzdálený Argus je trasa 300 V podle svého součtu, ne podle pravidla.**

Zátěž stanoviště podle třídy modulu, karta a čtyři porty, zpět k akumulátoru:

| modul | sběrnice 12 V | `G-300-U-40` | `G-300-S` | z akumulátoru |
|---|---|---|---|---|
| 100 mA | 3,2 W | 3,8 W | 5,1 W | 6,0 W |
| 150 mA | 4,8 W | 5,6 W | 7,5 W | 8,8 W |
| 300 mA | 9,3 W | 10,9 W | 14,6 W | 17,2 W |

Řádek 100 mA je vlastní propočet z `../bifrost/argus/HARDWARE.md`; řádky 150 a 300 mA ho škálují
podle odběru modulu a jsou odhady. Řádek 300 mA je třída 52/84 Mb/s, nikde neosazená; je to hodnota,
na kterou jsou obě desky dimenzované.
**Na vzdáleném stanovišti není nic uzemněné** — čtyři `G-300-S` se osazují jako na stanici, s pásky
nezapojenými — a **rozestup ramen** brání tomu, aby jeden úder vzal všechna čtyři ramena, nikoli
úderu samotnému. Kde lze mít na místě akumulátor, je to pořád levnější odpověď.

## Desky

*Každá deska nese konektory, které jsou její vlastní, a výše popsanou společnou část; dále následuje
to, co každá z nich přidává. V každém obrázku je vyznačena bariéra. Součástky a jejich hodnoty jsou
v `HARDWARE.md`; úplný seznam v `BOM.md`.*

### `G-48-S` — napájecí, zdrojový konec, 48 V

Vyrábí napájení 48 V pro jednu trasu z 12 V, které odebírá z vodiče.

```
 12 V off the wire, two terminals ─▶ EMI choke ─▶ LT3748 · ISC165N15NM6 ─▶ 750310988 ─▶ V5N22 ─┐
                                   boundary mode, no burst    1:4,42               │ THE BARRIER
                                   R_SENSE 9,1 mΩ                                  ▼
                                                              2× 5.0SMDJ54A ─▶ chokes 22 µH ═════════════▶ the feed, 2-core
                                                                                                2036-07-SM ─▶ ⏚ common earthing point
 INA238 + ISO1642 on the output · the host side on the port's 3,3 V · ENABLE drives the LT3748's EN
```

- **vstup:** 12 V na svých svorkách; 3,3 V a `ENABLE` z napájecího konektoru · **výstup:** 48 V na dvoužilový kabel; `SDA · SCL · ALERT` zpět
- **dimenzace:** **~26 W dodaných** v pracovním bodě akumulátoru; 28,4 W příkon. Stropem je 15 A saturace vinutí: v hraničním režimu se indukčnost vykrátí a watty na ampér posouvá jen odražené napětí, a toto je jediné vinutí 12 V → 48 V v katalogu. 41 kHz při plné zátěži; žádný burst režim, takže minimální zátěž 0,65 W
- **součástky:** `5.0SMDJ14A` na vstupu 12 V · `LT3748` · `ISC165N15NM6` · `750310988` · `V5N22-M3` · `US3M`, blokovací dioda · `INA238` · `ISO1642` · ostrovní `SN6505B` · `750313734` · 2× `PMEG10020ELR` · 2× `5.0SMDJ54A` · `2036-07-SM` · 2× tlumivka 22 µH · odrušovací tlumivka a vyhlazovací kondenzátory
- **`ID`:** 15,0 kΩ, 0,60

### `G-300-S` — napájecí, zdrojový konec, 300 V

Vyrábí napájení 300 V pro jednu trasu: tentýž měnič jako `G-48-S` s vinutím, usměrňovačem a
kaskádou pro 300 V a s hlídačem svodu. Samostatná deska, protože jiné vinutí, usměrňovač a omezovač
jsou jiný výkres.

```
 12 V off the wire, two terminals ─▶ EMI choke ─▶ LT3748 · ISC165N15NM6 ─▶ 750310349 ─▶ US3M ─┐
                                   R_SENSE 5,5 mΩ             1:10                │ THE BARRIER
                                                                                  ▼
                                                              2× 5.0SMDJ350A ─▶ chokes 22 µH ═════════════▶ the feed, 2-core
                                                                                                 2036-30-SM ─▶ ⏚ common earthing point
 INA238 + ISO1642 on the output · a second INA238, the run against earth — the leak watch · ENABLE drives the LT3748's EN
```

- **vstup:** 12 V na svých svorkách — 4,2 A při 12 V, 4,6 A na spodní hranici 11 V; 3,3 V a `ENABLE` z napájecího konektoru · **výstup:** 300 V na dvoužilový kabel; `SDA · SCL · ALERT` zpět
- **dimenzace:** **~47 W dodaných, jedno číslo**, 50,6 W příkon — tatáž deska na stanici i u vzdáleného Argusu. Bočník dovolí ~66 W a práh `INA238` drží těch 47; 145 kHz při plné zátěži, žádný burst režim, takže minimální zátěž 0,63 W. Izolace vinutí 1000 V AC je pro koncept přijata; sériová výroba objedná vinutí se zakázkovou izolací
- **součástky:** `5.0SMDJ14A` na vstupu 12 V · `LT3748` · `ISC165N15NM6` · `750310349` · 2× `US3M`, usměrňovač a blokovací dioda · 2× `INA238` · `ISO1642` · ostrovní `SN6505B` · `750313734` · 2× `PMEG10020ELR` · 2× `5.0SMDJ350A` · `2036-30-SM` · 2× tlumivka 22 µH · odrušovací tlumivka a vyhlazovací kondenzátory
- **`ID`:** 18,7 kΩ, 0,65

### `G-48-U` — napájecí, konec jednotky, 48 V

Odebírá 48 V z kabelu a dodává **12 V** na své dvě svorky — vstup a vývod, týž uzel; kupované
zařízení si je bere z vývodu. Její druhý napájecí pár vede napájení dál, za kaskádou této desky,
k další jednotce na řetězeném segmentu.

```
 48 V ══▶ 2× 5.0SMDJ54A ─▶ bulk ─▶ LT3748 · ISC165N15NM6 ─▶ 750311607 ─▶ V10P10 ─┐
  through   across the pair          R_SENSE 15 mΩ            2,5:1, 14 µH   100 V     │ THE BARRIER
  the gland no tube and no chokes — a unit end floats                                     ▼
                                                                                              12 V ─▶ 5.0SMDJ12A ─┬─▶ the two terminals ─▶ the unit
                                                                                                                   └─▶ the tap — a bought device, ≤ 0,5 A
 INA238 + ISO1642 on the input · the feed straight through on the second pair, so units chain
```

- **vstup:** 48 V průchodkou · **výstup:** 12 V na dvě svorky; `SDA · SCL · ALERT` na konektoru; 3,3 V hostitele napájí hostitelskou stranu izolátoru
- **dimenzace:** **~25 W** na 12 V; to, co dorazí, omezuje ~26 W zdrojového měniče, ne tato deska, a bočník dovolí ~50 W. 455 kHz při plné zátěži, omezeno na 1,05 MHz pod asi 10 W; minimální zátěž 0,24 W. Nic ke spínání: jednotka si z 12 V vyrábí vlastní napájení
- **tlaková deska** — jen keramika a plné součástky
- **součástky:** `LT3748` · `ISC165N15NM6` · `750311607` · `V10P10-M3` · `INA238` · `ISO1642` · ostrovní `SN6505B` · `750313734` · 2× `PMEG10020ELR` · 2× `5.0SMDJ54A` · `5.0SMDJ12A` na výstupu 12 V · keramické vyhlazovací kondenzátory
- **`ID`:** 23,2 kΩ, 0,70

### `G-300-U-6` · `G-300-U-40` — napájecí, konec jednotky, 300 V

**Dvě desky a to, co je rozděluje, je skříň, ne watty**: `G-300-U-6` je deska pro pouzdro, kde je
40mm trubka celým objemem a její vinutí je vysoké 8,6 mm proti 23,5; `G-300-U-40` je sběrnice 12 V
této velikosti kdekoli. Vše ostatní je společné — tentýž `LT8316`, tentýž `FCD260N65S3`, tatáž
kaskáda pro 300 V, tytéž konektory, tatáž telemetrie. Liší se vinutím, `R_SNS` a usměrňovačem.

```
 300 V ══▶ 2× 5.0SMDJ350A ─▶ the ~2 µF bank ─────────────────────▶ LT8316 · FCD260N65S3 ─▶ [ the winding ] ─▶ [ the diode ] ─┐
   through   across the pair                                          quasi-resonant                                          │ THE BARRIER
   the gland no tube and no chokes — a unit end floats                                                                           ▼
                                                                                                                             12 V ─▶ 5.0SMDJ12A ─┬─▶ the two terminals ─▶ a unit
                                                                                                                                                  └─▶ the tap — a device, or a remote site's wire
 INA238 + ISO1642 on the input
```

| | **`G-300-U-6`** | **`G-300-U-40`** |
|---|---|---|
| vinutí | **`11338-T195`** — CEEH178, 14:1:1,7, 1000 µH, 19 × 17,4 × 8,6 mm SMD, základní izolace 3000 V AC | **`11328-T078`** — PQ2620, 8:1:1, 670 µH, 31 × 28,5 × 23,5 mm PIN, zesílená izolace 3 kV |
| `R_SNS` | **130 mΩ** | **58 mΩ** |
| jmenovitý výstup | **6 W** na 12 V | **40 W** na 12 V — ~47 W zdroje minus 3,7 W ztrát této desky, deklarováno pod pracovním bodem 11 V |
| usměrňovač | **`V10P10-M3`**, 100 V / 10 A — 33 V závěrného namáhání, 0,5 A | **`V10P10-M3`**, 100 V / 10 A — 50 V závěrného namáhání, 3,7 A, dvě třetiny ztrát desky |
| `f` při plné zátěži | 140 kHz, omezení součástky — přerušovaný režim | 91 kHz |
| na drainu FET | 472 V z jeho 650; **550 V na jeho omezovači RCD** — `US3M`, 100 nF 1 kV, 2× 38,3 kΩ — osazeném kvůli 34 µH rozptylové indukčnosti vinutí | 402 V z jeho 650 |
| **vyhlazovací banka** | **5× 1 µF 500 V X7R 2220** — keramika, **tlaková deska** | **4× 1 µF 630 V polypropylenový film + 100 nF 1 kV X7R 1812** — **jen pozemní**; tlakové provedení převezme banku `G-300-U-6` a přepočítá se |
| **`ID`** | **30,1 kΩ**, 0,75 | **40,2 kΩ**, 0,80 — jediné místo, kde se obě desky liší mimo výkonovou část, a musí, protože hostitel nesmí chtít po desce na 6 W 40 W |

- **vstup:** 300 V průchodkou · **výstup:** 12 V na dvě svorky; `SDA · SCL · ALERT` na konektoru
- **klidová ztráta:** žádná, která by stála za řeč — `LT8316` odebírá 12 µA na `V_IN` ve vypnutém stavu a 75 µA na `BIAS` v burst režimu, miliwatty při 300 V, a ostrov svých 1,5 mA z 3,3 V hostitele; v burst režimu jde dolů až na 3,5 kHz, takže minimální zátěž je ~1 % jmenovité
- **součástky, `G-300-U-6`:** `LT8316` · `FCD260N65S3` · `11338-T195` · `V10P10-M3` · omezovač RCD, `US3M` · `INA238` · `ISO1642` · ostrovní `SN6505B` · `750313734` · 2× `PMEG10020ELR` · 2× `5.0SMDJ350A` · `5.0SMDJ12A` na výstupu 12 V · keramická banka
- **součástky, `G-300-U-40`:** `LT8316` · `FCD260N65S3` · `11328-T078` · `V10P10-M3` · `INA238` · `ISO1642` · ostrovní `SN6505B` · `750313734` · 2× `PMEG10020ELR` · 2× `5.0SMDJ350A` · `5.0SMDJ12A` na výstupu 12 V · filmová banka

### `G-12-S` — galvanicky oddělených 12 V, zdrojový konec

**12 V přes bariéru a rovnou na krátkou trasu — napájení, do kterého se zapojuje kupované zařízení
ModBus pár metrů od stanice**, na téže svorkovnici jako jeho pár. Žádný protějšek na konci jednotky:
na vzdáleném konci stojí samo zařízení. Odebírá 12 V z vodiče na vlastních svorkách, 0,4 A ven a
0,5 A dovnitř, a bere 3,3 V a `ENABLE` z napájecího konektoru jako každá zdrojová deska. **Jejím
rozsahem je napájení senzoru**: asi 5 W v pracovním bodě, neregulované, sledující akumulátor.
Zátěž, která chce watty, je napájení na 48 V nebo 300 V, nikdy tato deska na nižším napětí.

```
 12 V off the wire, two terminals ─▶ EMI choke ─▶ SN6507 push-pull ─▶ 750319691 ─▶ 2× PMEG10020ELR ─┐
                                   1,26 MHz, R_LIM 34,8 kΩ → ~0,7 A   N = 1,13          │ THE BARRIER
                                   ENABLE drives its EN                                 ▼
                                                                        2× 5.0SMDJ18A ─▶ chokes ─▶ 2036-07-SM ─▶ ⏚ common earthing point
                                                                                                    ══▶ ± ~12 V, unregulated, on eight poles
                                    INA238 + ISO1642 on the output — the host side on the port's 3,3 V
```

- **vstup:** 12 V z vodiče na dvou svorkách; `ENABLE` a 3,3 V z napájecího konektoru · **výstup:** ~12 V na osmi pólech, `DGPS2.5R-5.0` × 4 — čtyři pozice, 4× + a 4× −, vedle čtyř pozic páru na `G-I-M-005`
- **dimenzace:** **0,4 A**, asi 5 W. `R_LIM` 34,8 kΩ → ~0,7 A (0,5–0,9 A přes rozptyl součástky): 0,4 A ven je ~0,45 A přes spínače a limit 0,5 A by u slabšího kusu nechal jmenovitou zátěž poklesnout. **Přetížení řeší `INA238`, ne OCP** — OCP součástky zkracuje impulzy, místo aby vypnula, a přetížení by držela až do tepelného vypnutí, takže `ALERT` nad 0,4 A odebere `ENABLE`; OCP zbývá start do vstupních kondenzátorů senzoru a zkrat
- **součástky:** `5.0SMDJ14A` na vstupu 12 V · `SN6507` · `750319691` · 2× `PMEG10020ELR` · `INA238` · `ISO1642` · ostrovní `SN6505B` · `750313734` · 2× `PMEG10020ELR` · 2× `5.0SMDJ18A` · `2036-07-SM` · tlumivky, odrušovací tlumivka a vyhlazovací kondenzátory
- **`ID`:** 56,2 kΩ, 0,85

### `G-24-S` — napájecí, zdrojový konec, 24 V, spínané

**Spínaných 24 V přes bariéru pro zátěž, která není senzorem (čerpadlo), na vyhrazené napájecí patici hostitele.** Žádný protějšek na konci jednotky: zátěž bere 24 V přímo, po
vlastním dvoužilovém kabelu, a za bariérou nereguluje nic kromě samotného měniče. Odebírá 12 V
z vodiče na vlastních svorkách a bere 3,3 V a `ENABLE` z napájecího konektoru — **a `ENABLE` je
spínač**: hostitel ho zvedne na dobu chodu zátěže a jinak měnič stojí s `EN/UVLO` obvodu `LT3748`
drženým v L a deska neodebírá nic.

```
 12 V off the wire, two terminals ─▶ LT3748 · ISC165N15NM6 ─▶ 750311592 ─▶ V10P10-M3 ─┐
                                    R_SENSE 10 mΩ            1:1, 8 µH     100 V      │ THE BARRIER
                                    ENABLE drives its EN/UVLO                          ▼
                                                              2× 5.0SMDJ28A ─▶ chokes 22 µH ─▶ 2036-07-SM ─▶ ⏚ common earthing point
                                                                                                ══▶ 24 V, two poles — the load's 2-core cable
                                    INA238 + ISO1642 on the output — the host side on the port's 3,3 V
```

- **vstup:** 12 V z vodiče na dvou svorkách, ~3 A, dokud zátěž běží (36 W příkon v pracovním bodě); `ENABLE` a 3,3 V z napájecího konektoru · **výstup:** 24 V na jedné dvoupólové `DGPS2.5R-5.0`; `SDA · SCL · ALERT` zpět
- **dimenzace:** **~33 W dodaných** v pracovním bodě akumulátoru — `I_pk` 9,0 A při 10 mΩ, 112 kHz při plné zátěži. To, co port deklaruje, patří zátěži, naprogramovaný práh `INA238`: **nad** limitem je zablokovaná zátěž, **pod** minimem zátěž běžící nasucho nebo přeříznutý kabel — dva prahy, které hostitel programuje. Proudový limit je vlastní `R_SENSE` měniče; do zablokovaného motoru se dodává na limitu a nikdy se nejistí pojistkou
- **kaskáda:** zdrojového konce, jako u `G-12-S` — tříelektrodová výbojka, tlumivky 22 µH a 2× `5.0SMDJ28A` na 24 V. Co stojí na konci zátěže, je věcí zátěže samotné: holý motor dostane dva vývodové transily na svá oka, řízený vlastní malou desku
- **součástky:** `5.0SMDJ14A` na vstupu 12 V · `LT3748` · `ISC165N15NM6` · `750311592` · `V10P10-M3` · `US3M`, blokovací dioda · `INA238` · `ISO1642` · ostrovní `SN6505B` · `750313734` · 2× `PMEG10020ELR` · 2× `5.0SMDJ28A` · `2036-07-SM` · 2× tlumivka 22 µH · vyhlazovací kondenzátory
- **`ID`:** 4,32 kΩ, 0,30

### `G-I-N-025` — komunikační, 485 po mědi, NodBus

Nese NodBus po mědi, **oba konce trasy, jedna deska**: data plně duplexně, přijímač ozvěny a hodiny
nebo PPS na kanálu B. Na kterém konci stojí, nastavuje patice — `B_DIR` a `LINE_EN` tam propojené
nebo buzené — a zdrojový konec osazuje výbojku a její pásek.

```
 the host ── data connector ──▶ 3,3 V from the host ─┬─▶ SN6505B ─▶ 750313734 ─▶ PMEG10020ELR ─▶ isolated 3,3 V
              LINE_EN drives the SN6505B's EN │             420 kHz      1:1,1          THE BARRIER    │
                                              │                                                        ▼
                                              └── the ID resistor · B_DIR strap read       ISO1452 ── data, both directions
                                                                                           ISO1452 ── the echo-check receiver, driver tied off
                                                                                           ISO1450 ── channel B: the clock out, or PPS in
                                                                                                     D and R both on the CLK/PPS pin;
                                                                                                     DE and RE# on B_DIR
                                                                                                     │
                                                                                SM712 ─▶ 2× 10 Ω ══ 80,6 Ω behind a jumper ══▶ TX · clock · RX
                                                                                                 2036-07-SM ─▶ ⏚ common earthing point, at the source end only
 four two-pole DGPS2.5R-5.0, eight poles, one pole to a net · three termination jumpers, one per pair
```

- **vstup:** 3,3 V a logika od hostitele · **výstup:** tři páry 485 na datovém kabelu
- **odběr, z 3,3 V hostitele:** ~75 mA na zdrojovém konci, ~18 mA na konci jednotky, ~130 mA uprostřed burstu; samotná linková strana 57 / 5 mA
- **na zdrojovém konci:** výbojka a její zemnicí pásek osazené; na konci jednotky vynechané — tatáž deska
- **součástky:** `SN6505B` · `750313734` · `PMEG10020ELR` · 2× `ISO1452` · `ISO1450` · 3× `SM712` · 6× 10 Ω · 3× 80,6 Ω s lištami a jumpery · `2036-07-SM` · `DGPS2.5R-5.0` × 4
- **`ID`:** 6,65 kΩ, 0,40

### `G-I-M-005` — komunikační, 485 po mědi, ModBus

`G-I-N-025` osekaná na větev ModBus: **jeden transceiver, jeden pár, poloviční duplex** — Modbus RTU
otáčí linku a toto je jediná poloduplexní deska v rodině — tatáž bariéra a totéž galvanicky
oddělené napájení. **50 m** je dosah v názvu: stanovuje ho 12 V větve, ne 485.

**Žádné písmeno konce, protože deska má jen jeden konec.** Poloduplexní pár je symetrický, `DE`
od hostitele ho otáčí rámec po rámci, není žádný hodinový kanál a `B_DIR` je nezapojený. Na
vzdáleném konci větve není deska vůbec: kupovaný senzor i MOD si přinášejí vlastní 485.

```
 the host ── data connector ──▶ 3,3 V from the host ──▶ SN6505B ─▶ 750313734 ─▶ PMEG10020ELR ─▶ isolated 3,3 V ─▶ ISO1450 ── A/B, DE from the host
              LINE_EN drives the SN6505B's EN                           THE BARRIER                              │
                                                                                               SM712 ─▶ 2× 10 Ω ══ 80,6 Ω behind a jumper ══▶ the pair
                                                                                                            2036-07-SM ─▶ ⏚ common earthing point
```

- **vstup:** 3,3 V a logika od hostitele · **výstup:** pár větve na osmi pólech, `DGPS2.5R-5.0` × 4 — čtyři pozice, 4× `A` a 4× `B`
- **odběr, z 3,3 V hostitele:** ~9 mA při poslechu, ~60 mA při dotazu
- **součástky:** `SN6505B` · `750313734` · `PMEG10020ELR` · `ISO1450` · `SM712` · 2× 10 Ω · 80,6 Ω s lištou a jumperem · `2036-07-SM` · `DGPS2.5R-5.0` × 4
- **`ID`:** 8,25 kΩ, 0,45

### `G-O-10-10` — komunikační, sklo, 10 Mb/s

Nese NodBus po vlákně do 10 km s modulem 10 Mb/s — hodiny 2²² po něm projdou.

```
 the host ── data connector ──▶ 3,3 V from the host ──────────────────────┬─▶ π filter ─▶ 1×9 seat: channel A, data, both sections
                                                                          └─▶ π filter ─▶ 1×9 seat: channel B, the clock — VccT at the sending end,
                                                                                                                              VccR at the receiving end
                                                                          74AUP1G126 pin → TD, 74AUP1G125 RD → pin; their OEs and the two
                                                                          load switches on B_DIR
 local bulk at each seat · the ID resistor · NO barrier, NO ladder — the fibre isolates by construction
```

- **vstup:** 3,3 V a logika od hostitele · **výstup:** čtyři vlákna
- **odběr:** v průměru ~105 mA na zdrojovém konci a ~55 mA na konci jednotky, nejvýše 175 a 125 mA; každý modul, který tento projekt osazuje, uvádí 100 mA, z toho přijímač ~25 a laser ~75
- **modul:** `OPT10-31103STR` / `-PTR` — 10 Mb/s, 10 km, jen duplex. Stejnosměrně vázaný, TTL, klid mapovaný na zhasnutý laser; `DE` nemá laser, který by spínal. Dosah 0–10 km, v žádné délce bez útlumového článku
- **součástky:** oba moduly · `74AUP1G126` · `74AUP1G125` · `TPS22917` na `VccT` a `TPS22917L` na `VccR` · 2× 100 kΩ · π filtry a vyhlazovací kondenzátory — `B_DIR` je statická volba z patice, ne spínač
- **`ID`:** 10,0 kΩ, 0,50

### `G-O-2-100` — komunikační, sklo, 2 Mb/s

Tatáž deska s modulem 2 Mb/s v patici — **jedna součástka, a dosáhne od metru vlákna do 100 km**:
její přijímač bere −39 až 0 dBm. Kanál B běží na nižším stupni, 2¹⁹, což na mini-NOD stačí, a otáčí
se jako u `G-O-10-10`. Samostatná deska, protože modul je jiný výkres.

**Modul nemá minimální délku, ale nemá ani rezervu na nule**: bez vlákna před sebou sedí přesně na
svém stropu přebuzení, takže na krátké trase se osazuje pevný optický útlumový článek
(`HARDWARE.md`, *The minimum length*).

- **vstup / výstup / odběr:** jako `G-O-10-10`
- **modul:** `OPT2-55A03STR` duplex, 1550 nm, nebo pár BiDi `OTB2-35A03STR` / `OTB2-53A03STR`
- **součástky:** jako `G-O-10-10`, s tímto modulem
- **tlakové provedení** je tatáž součástka, získaná od výrobce ve tlakově odolné podobě podle téže specifikace; katalogové moduly jsou pozemní součástky
- **`ID`:** 12,1 kΩ, 0,55

## Měniče a měřicí desky — vzdálenost, ne frekvence

**Žádná spínací frekvence, kterou si tato rodina může koupit, neleží mimo všechna měřicí pásma**, a
ostrovní flybacky běží v hraničním režimu, takže jejich frekvence se hýbe se zátěží a nedá se
zaparkovat. Pravidlo je mechanické: měnič tak daleko od vstupní části, jak to skříň dovolí — blízké
pole klesá s třetí mocninou vzdálenosti, její zdvojnásobení je −18 dB; stíněné indukčnosti, souvislá
zemní plocha, malá horká smyčka; `MODE` podle desky, vynucené PWM tam, kde pásmo poslouchá, a auto
tam, kde nic; a co opouští desku po kabelu, je filtrované — na zdrojové desce stojí mezi měničem a
svorkou vyhlazovací kondenzátory a sériové tlumivky, takže vlastní spektrum měniče není kritériem.
Jeden měnič na desku tam, kde se měří pásmo: dva volně běžící hraniční měniče vytvoří rozdílovou
frekvenci, která pásmo přejíždí celý den. Kritériem je vlastní laboratorní hodnota měřicí desky a
práh `INA238` pak to číslo hlídá.

## Soubory

| Soubor | Obsah |
|---|---|
| [`HARDWARE.md`](HARDWARE.md) | vrstva součástek — každý měnič, každá součástka a hodnota, deska po desce; záznam, podle kterého se kreslí |
| [`BOM.md`](BOM.md) | kusovník, deska po desce; platí `HARDWARE.md` |
| [`EXAMPLE.md`](EXAMPLE.md) | příklad sestavy — jeden Bifrost ve skříni |
| [`WHY.md`](WHY.md) | hřbitov — co se zkusilo, co padlo a proč: zamítnuté alternativy a překonané stavy, s důvodem |

## Licence

Hardware: CERN-OHL-S v2 (`../LICENSE-HW`) · Software: MIT (`../LICENSE`) — Copyright (c) 2026 NIC — Native Intellect Community

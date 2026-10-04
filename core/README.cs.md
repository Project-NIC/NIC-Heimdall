★ N.I.C. ★

# NIC Core — univerzální platforma

[English](README.md) · **Čeština** · [Русский](README.ru.md)

> **Koncept ve fázi návrhu — nic není postaveno.** Čísla se ověřují proti katalogovým listům a součástkám, než vznikne deska.

Toto je **na frontendu nezávislá** polovina NIC: knihovny a hardwarové bloky, které každý produktový
frontend (Quake, Palatine, Sputnik, …) používá znovu *beze změny*. Core neví nic o seismometrech,
srážkoměrech ani přijímačích GNSS — přesouvá bajty, rozvádí hodiny, udržuje čas, plánuje sloty
a mluví s hlavním uplinkem. Veškerý produktový význam žije v propojovacím kódu (glue) frontendu,
nikdy tady.

> **Pojistka.** Pokud vás při zapojování frontendu láká sáhnout do knihovny jádra a přidat
> produktově specifický význam, zastavte se: ten význam patří do propojovacího kódu frontendu. Jádro
> zůstává obecné, aby nový frontend stál nula změn v jádru (konfigurace je obecná — viz
> `PROTOCOL.md` §5).

---

## Nosný princip — uzel závisí jen na NodBus

**Uzel je nezávislý na hardwaru. Nezávisí na ničem kromě protokolu NodBus — proto běží kdekoli,
s čímkoli a připojený k čemukoli.**

Právě proto je platforma univerzální, a říká se to tady **jednou**:

- Uzel není „seismograf“ ani „meteostanice“. Je to *věc, která mluví NodBus*. Co snímá, je detail
  propojovacího kódu jeho frontendu.
- Nepředpokládá nic o tom, kdo jsou jeho sousedé, co je master, co měří ostatní uzly ani na jaké
  kabeláži visí. Nastartuje, ohlásí svůj **typ** (jaký je to druh), převezme **číslo**, které mu
  přidělí obchůzka (sweep), zabere slot TDMA a streamuje rámce.
- Proto jakýkoli uzel běží na jakékoli sběrnici NIC, volně smíchaný s jakýmikoli jinými typy uzlů,
  za jakýmkoli masterem — protože jedinou smlouvou mezi nimi je protokol NodBus (rámce `nic-link`,
  řídicí opkódy, sdílené hodiny a samoběžné sloty).

Všechno ostatní v `core/` existuje proto, aby tato smlouva byla malá, robustní a přenositelná.

### NodBus v jednom odstavci

**NodBus** je páteř dat + hodin, na které visí uzly, na měděném 485 nebo na skle za deskou Galvani.
Data UART jedou v samočasovaných slotech TDMA — **duplex se řídí napájením a každá třída teď bere
napájení ve vlastním kabelu, takže každá třída je plně duplexní** (`G-I-N-025` na mědi, `G-O-10-10` /
`G-O-2-100` na skle); poloduplexní provedení NodBus neexistuje. Další pár nese síťové hodiny
(zrozené na Kronosu, jediným zdrojem definice je `blocks/nodbus.md`, *The network clock*),
regenerované v každém transceiveru, takže přežijí i dlouhý kabel. Vlastní NodBus začíná za mosty
Bifrost stanice — centrála s ním mluví přes spojení bod–bod MasterNOD. Kupované senzory Modbus
a pomalé vlastní moduly sedí na větvích ModBus (arm) Palatine — stejná fyzika, dotazovaná sběrnice
bez hodin. Viz `blocks/nodbus.md` (NodBus) a `blocks/modbus.md` (ModBus).

## Druhý nosný princip — jednotka přežije rušení, aniž by věděla, co to je

**Žádná jednotka ve stanici nikdy neidentifikuje zdroj rušení a žádná to nepotřebuje.** Úder blesku,
jiskřící vedení, spínaný měnič, radar, svářeč na vedlejším poli, EMP, něco, co nikdo
necharakterizoval — pojmenovat zdroj je otázka pro další zpracování a žádný vstupní obvod na ni
neumí odpovědět. **Od každé jednotky se požadují tytéž čtyři věci a jsou podmínkou návrhu, ne volbou
jednotlivé desky:**

1. **Detekuje, že její vstup už není signál** — proti vlastnímu průběžnému pozadí, nikdy proti
   prahu zakompilovanému do kódu. Každá jednotka už toto pozadí počítá pro své vlastní měření,
   takže to nestojí nic nového.
2. **Degraduje předvídatelně.** Saturovaný obvod se zotaví v omezeném čase, smyčka přečká výpadek
   setrvačností, filtr nezůstane doznívat do dalšího rámce. Nic se nezablokuje, nic neuteče
   a **žádná jednotka nepotřebuje k návratu vypnutí a zapnutí napájení.**
3. **Označí rámec** — `status` 6 CLIPPED tam, kde hodnota narazila na svůj rozsah, `status` 7
   DISTURBED tam, kde neořízla, ale vstup nebyl signál, a zasažený podíl okna tam, kde jednotka
   nějaké okno měří (`PROTOCOL.md` §1).
4. **Nikdy nepublikuje rušené číslo, jako by bylo čisté.** Archiv, který nedokáže odlišit rušené
   čtení od dobrého, je horší než mezera: mezera je poctivá.

**Obrana, která musí nejdřív rozpoznat zdroj, selže na prvním zdroji, který nikdo
necharakterizoval** — a to je přesně ta třída, kvůli které tato stanice existuje (typ UFO u Tesly,
`../tesla/DETECTION.md`). Opatření jsou proto už konstrukcí nezávislá na zdroji: zatemnění
(blanking) podle obálky, zotavení s omezenou časovou konstantou, příznak v rámci. **Doba zotavení
je laboratorní kritérium každé jednotky, která saturuje**, zapsané v jejím vlastním `HARDWARE.md`:
právě ona mění zatemněný podíl na chybu úrovně.

---

## Hardwarové bloky (`blocks/`)

Znovupoužitelné hardwarové bloky sdílené všemi uzly a mastery — blok zkopírujte, nekreslete ho znovu:

| Blok | Co |
|---|---|
| [`nodbus.md`](blocks/nodbus.md) | NodBus: datové páry + pár síťových hodin, měď nebo sklo (**jeho první oddíl je TA definice hodin**), zakončení |
| [`clocks.md`](blocks/clocks.md) | **Která deska nese který oscilátor, úroveň po úrovni** — jediná přesná součástka na Kronosu, nic na kartách a jednotkách NodBus, HSI disciplinovaný stupněm na jednotkách mini, běžný krystal na MODech ModBus a binární pravidlo, díky kterému se dá koupit ze skladu |
| [`modbus.md`](blocks/modbus.md) | Koncová sběrnice ModBus (leaf) — větve Palatine, kupované senzory a vlastní MODy na nich a vlastní mapa registrů |
| [`gps-pps.md`](blocks/gps-pps.md) | Doktrína GPS PPS / časování + rozvod času uvnitř skříně (Kronos) |

---

## Blok

```
   KRONOS ──the M-LVDS time bus, 2²² + PPS_K──▶ the CARDS ──spurs, 2²², 40 B TDMA──▶ NODs: Quake · Palatine · Sputnik · Tesla · Marconi · Photon · Positron
      │                                          │
   PPS-K + label ──▶ MAYAK ◀──trunks, 48 B────────┘        ARGUS ──segments, 2¹⁹, 16 B─▶ mini-NODs: Gauss · Quark-Tubes · Pascal
                       │                                   PALATINE ──ModBus RTU arms──▶ MODs and bought sensors
                    the uplink
   the core: framing · addressing · TDMA · the clock · the node contract        a front: one sensor and its glue, nothing of the core changed
```

## Strom

```
core/                    the node core — the bus, the frame, the clock, the uplink
├── archive/             HMC, the container · HCC, the codec · the exporters · ref/, the reference library
└── blocks/              the hardware blocks, one file each
```

## Soubory

| Soubor | Obsah |
|---|---|
| [`PROTOCOL.md`](PROTOCOL.md) | rámce, adresování, TDMA, model času, datové úrovně, uplink, smlouva uzlu |
| [`COMMS.md`](COMMS.md) | **komunikační mapa** — každé spojení ve stanici v jedné tabulce, každý řádek jmenuje dokument, který ho vlastní |
| [`HARDWARE.md`](HARDWARE.md) | jak se bloky skládají do stanice — univerzální uzel a centrála |
| [`POWER.md`](POWER.md) | zdroj, úložiště, rozvod, tepelné poměry a záchranná pojistka BMS |
| [`DESIGN.md`](DESIGN.md) | rejstřík rozhodnutí — čísla D, každý řádek jmenuje, kde obsah žije |
| [`BRINGUP.md`](BRINGUP.md) | oživení v pořadí, od krabice po zelená data, a brána studeného návratu |
| [`COMMISSIONING.md`](COMMISSIONING.md) | uvedení do provozu — laboratoř, pak terén, pak oživení |
| [`UPLINK_TRANSPORT.md`](UPLINK_TRANSPORT.md) | jak se stanice dostane k serveru |
| [`INTEROP.md`](INTEROP.md) | kdo přebírá naše data a v jakém formátu |
| [`WHY.md`](WHY.md) | hřbitov — co se zkusilo, co padlo a proč |
| [`blocks/`](blocks/) | hardwarové bloky, každý v jednom souboru — tabulka výše |
| [`archive/HMC.md`](archive/HMC.md) | archiv: kontejner — řada souborů na jednotku, pravidla záznamu a odesílání, data zepředu a popis zezadu |
| [`archive/HCC.md`](archive/HCC.md) | archiv: kodek — uchované záznamy jedné adresy, sloupec po sloupci, bezztrátově; žádná tabulka, ale pravidlo |
| [`archive/EXPORTERS.md`](archive/EXPORTERS.md) | čtečky archivu — miniSEED, IAGA-2002, RINEX, BUFR, CWOP, IOC, kvalita vzduchu, Blitzortung, CSV, SQL, prohlížeč; každý jedna šablona |
| [`archive/ref/`](archive/ref/README.md) | referenční kód, Python — kodér a dekodér HCC, Steim-2, měření |

---

## Licence

Hardware: CERN-OHL-S v2 (`../LICENSE-HW`) · Software: MIT (`../LICENSE`) — Copyright (c) 2026 NIC — Native Intellect Community

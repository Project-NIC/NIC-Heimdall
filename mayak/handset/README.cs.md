<p align="center">
  <img src="NIC-Handset.svg" width="200"/>
</p>

★ N.I.C. ★

# Handset — pomocník stanice pro oživení a servis (BLE)

[English](README.md) · **Čeština** · [Русский](README.ru.md)

Pomocník pro oživení bez notebooku pro **Mayak**, centrálu stanice (`../FIRMWARE.md` §10, servisní okno). Sestavíte stanici, stisknete na Mayaku tlačítko uvedení do provozu, otevřete tuto aplikaci a ta ukáže, **kdo se zaregistroval** a **který senzor je nezřízený / selhává, jménem** — **GO / NO-GO**, které přečtete na telefonu. Umí také posílat příkazy **zřízení / opětovného uvedení do provozu** (provision / re-commission).

> **Koncept ve fázi návrhu — žádná aplikace není napsána.** Tento soubor je popisem aplikace a GATT kontraktu, který Mayak dodržuje; aplikace se postaví podle něj, jakmile firmware Mayaku zpřístupní níže uvedený kontrakt.

**Stack — doporučení, ne příkaz:** jedna kódová základna pro Android **i** iOS — Flutter s `flutter_blue_plus` jako BLE central je propracovaná volba; React Native s `react-native-ble-plx` nebo nativní Kotlin a Swift udělají totéž. BLE, ne klasický BT: nízká spotřeba, a ESP32-S31 v Mayaku stejně umí jen BLE (BT 5.4 LE).

---

## GATT kontrakt — firmware Mayaku mu odpovídá

Mnemotechnická pomůcka: bajty `4e 49 43 20` v bázi služby tvoří „NIC “.

| Role | UUID | Vlastnost |
|---|---|---|
| Služba | `9a1e0001-4e49-4320-b000-000000000000` | — |
| **Stav** | `9a1e0002-4e49-4320-b000-000000000000` | **notify** |
| **Příkaz** | `9a1e0003-4e49-4320-b000-000000000000` | **write** |

**Advertising je podmíněn tlačítkem.** Mayak tuto službu ohlašuje **jen** tehdy, když je aktivní jeho tlačítko uvedení do provozu / časový limit (v běžném provozu je rádio vypnuté — spotřeba + bezpečnost). Sken, který nic nenajde, znamená „stiskněte tlačítko na skříni“.

### Stav (notify) — JSON oddělený novými řádky

Jeden úplný dokument na řádek; dokument **může zabrat několik notifikací** (skládejte podle `\n`). Zrcadlí HEALTH/SOH, které Mayak už agreguje:

```json
{"go":false,"nodes":[
  {"n":1,"bus":"nodbus","type":5,"label":"Quake","ok":false,"sensors":[
    {"id":0,"name":"ICM","h":0},
    {"id":1,"name":"ADXL355","h":2},
    {"id":2,"name":"SCL3300","h":0},
    {"id":3,"name":"RM3100","h":0}
  ]},
  {"n":2,"bus":"nodbus","type":6,"label":"Palatine","ok":true,"sensors":[
    {"id":1,"name":"AIR1","h":0},{"id":9,"name":"SOIL1","h":0}
  ]}
]}
```

- `go` — celkové GO/NO-GO (všechny osazené senzory OK).
- `n` NUMBER uzlu, `type` TYPE uzlu — **kód z vlastní tabulky typů dané sběrnice** (`../../core/PROTOCOL.md` §2: NodBus 1 Mayak · 2 Bifrost · 3 Argus · 4 Marconi · 5 Quake · 6 Palatine · 7 Tesla · 8 Sputnik · 9 Photon · 10 rezervováno (Neutron) · 11 Positron · 12 Pip · 13 Steinmetz; mini 1 Gauss · 2 Quark-Tubes · 3 Pascal; ModBus 4..61, jeden na veličinu na pozici) a `bus` říká, o kterou tabulku jde. Volitelné pole `label` je jen laskavost pro ladění — identitou je kód, nikdy jméno.
- pro každý senzor: `id` = lokální číslo senzoru v uzlu (adresa v soupisu Modbus nebo index senzoru SPI), `name` popisek pro člověka, `h` = **zdravotní kód senzoru**: `0` OK, `1` SELFTEST_FAIL, `2` NO_RESPONSE, `3` DEGRADED.

### Tabulka korekcí — zpoždění, která nic nedokáže změřit

**Všechno, co stanice odečítá, je v této tabulce, a všechno to patří Kronosu.** Jeden člen, který stanice změřila sama — trasa hodin vzdáleného přijímače, vyměřená Kronosem — se vrací **jen pro čtení**, aby operátor viděl, co se zjistilo, a nemusel hádat. Zbytek změřit nelze a musí se zadat ručně, a jsou to **dvě čísla**: anténní kabel v metrech a konstanta antény a přijímače v nanosekundách z katalogového listu přijímače nebo z jednoho měření na stole — tvar, který používá konvence CGGTTS ze světa časování (`../../kronos/FIRMWARE.md` §7, tabulka zpoždění). Jsou to **pevné odchylky**, takže se odečítají, místo aby se tolerovaly. Telefon je místo, kde je zadává člověk, protože člověk je na místě jediný, kdo ví, kolik kabelu se skutečně natáhlo.

**Kronos drží vlastní kopii ve flash a nabízí ji při startu; autoritou je centrála.** Aplikace tedy ukazuje, čemu stanice věří, a zápis to nahradí — vyměněný Kronos dostane správná čísla, aniž by je kdokoli zadával dvakrát.

Čtení (ve stavovém dokumentu):

```json
{"corr":{"total_ns":13150,"terms":[
  {"id":"cab","label":"antenna cable","m":20,"ns_per_m":5,"ns":100,"src":"typed"},
  {"id":"int","label":"antenna + receiver","ns":50,"src":"typed"},
  {"id":"route","label":"remoted receiver, clock run","ns":13000,"src":"ranged"}
]}}
```

- **`src` říká, komu číslo patří:** `typed` je editovatelné, `ranged` je to, co změřila stanice, a je jen pro čtení — zápis do něj je odmítnut, ne potichu zahozen.
- `m` je to, co zadá operátor; `ns` je to, co z toho spočítala stanice. Člen, který sestava nemá, chybí, není nulový — `route` existuje jen tam, kde je přijímač vysunut na lince Galvani (`../../galvani/README.md`, *The reversed channel*).
- Zadané členy se na Kronosu sečtou a zaokrouhlí na celé záchytné cykly, 7,45 ns (`../../kronos/FIRMWARE.md` §7, tabulka zpoždění); vyměřený člen se aplikuje tamtéž.
- **Trasa odbočky není korekce a v této tabulce není.** Karta ji vyměří a zapíše do jednotky, která na ni umístí svou vlastní mřížku (`../../core/PROTOCOL.md` §7); trasy se čtou z karet a na telefonu se ukazují jako diagnostika, jedna na slot.
- **`route` je člen, který rozhoduje, zda vysunutá sestava vůbec splní kontrakt.** Sklo má ~5 ns/m, takže kilometr je ~5 µs proti dodávané přesnosti 1 µs. Kronos ho vyměří; vysunutý přijímač, jehož trasa nebyla vyměřena, je v aplikaci **NO-GO**, stejně jako nezřízený senzor.

### Klimatologické tabulky — nastavené podle země, a země je součástí odpovědi

**Třída normality nemá smysl bez tabulek, které ji vytvořily.** Sedm pravděpodobnostních pásem je dáno WMO a všude stejné; **hraniční hodnoty jsou fitem národní služby na její vlastní klima**, takže stanice nese tabulky země, ve které stojí (`../../palatine/CLIMATE.md`). Nic z toho není konstanta firmwaru — je to konfigurace stanice, zadaná jednou tady.

Co aplikace nastavuje:

- **zemi a označení služby, jejíž tabulky to jsou** — uložené spolu s nimi, protože cokoli publikovaného s připojenou třídou musí říct, čí tabulky ji vytvořily;
- **platný normál** — 30letý blok počítaný od roku 1901, takže dnes **1991–2020**. Uvádí se spolu s čísly stejně, jako musí dlouhodobý průměr uvádět své období;
- **samotné hraniční tabulky** — teplota v K od normálu, srážky v % normálu, po měsících, po pololetích (**IV–IX** a **X–III**) a za rok.

**Drží je Mayak; telefon je jen místo, kde se zadávají.** Je to největší tabulka, kterou centrála nese, a jediná, která se *netýká* vlastního hardwaru stanice — všechno ostatní v tomto kontraktu popisuje skříň, tohle popisuje, kde skříň stojí.

**Stanice bez nahraných tabulek stále měří a stále archivuje.** Jen neklasifikuje: třída je interpretace přidaná nad hodnotu, nikdy podmínka jejího zaznamenání. Chybějící tabulka tedy není NO-GO, na rozdíl od nevyměřeného `route`.

### Příkaz (write) — JSON ukončený novým řádkem

```json
{"cmd":"refresh"} // push a fresh status snapshot now
{"cmd":"recommission"} // re-run the whole commissioning pass
{"cmd":"provision","node":1} // (re)provision one node's sensors
{"cmd":"corr","set":[{"id":"cab","m":25}]}     // write one TYPED term; the station recomputes ns
                                               // a ranged term is read-only and a write is refused
```

> JSON byl zvolen kvůli čitelnosti a protože uvádění do provozu je řídké / příležitostné. Pokud ho později budete chtít úspornější, přepněte obě strany na zhuštěný TLV — aplikace se ho dotýká jen ve svém parseru stavu a ve své BLE vrstvě.

---

## Co potřebuje sestavení pro telefon

**Oprávnění BLE**: na Androidu `BLUETOOTH_SCAN` (s `neverForLocation`) a `BLUETOOTH_CONNECT` a na API < 31 `ACCESS_FINE_LOCATION`; na iOS `NSBluetoothAlwaysUsageDescription` (krátký řetězec „used to commission NIC stations“). BLE potřebuje skutečný telefon, ne emulátor.

Aplikace má čtyři části: GATT kontrakt (UUID výše), vrstvu BLE (sken, připojení, odběr stavu, zápis příkazu), model stavu (stanice · uzel · senzor, parsování JSON, slovník zdravotních stavů) a jedinou obrazovku.

## Design — Volkov Commander

**Vzhled Norton / Volkov Commanderu:** DOSově modré pozadí, neproporcionální písmo, azurový panel s rámečkem z čar a typický **spodní pruh funkčních kláves** (F2 Refresh · F3 Connect · F5 Recomm · F10 Quit). Žádná moderní barevná okrasa — barva je **16barevná DOSová paleta použitá funkčně**: zdraví senzorů (zelená / žlutá / červená), inverzní zvýrazňovací pruh GO / NO-GO. Působí jako diagnostický nástroj, ne jako hračka.

## Licence

Software: MIT (`../../LICENSE`) — Copyright (c) 2026 NIC — Native Intellect Community

<p align="center">
  <img src="NIC-Mayak.svg" width="200"/>
</p>

★ N.I.C. ★

# Mayak — centrála stanice

[English](README.md) · **Čeština** · [Русский](README.ru.md)

> **Koncept ve fázi návrhu — nic není postaveno.** Čísla se ověřují proti katalogovým listům a součástkám, než vznikne deska.

**Mayak zaznamenává stanici a je jejími jedinými dveřmi do světa.** Jedna deska s ESP32-S31: uvádí
karty do fáze FLOOR, přijímá od nich hotové záznamy s časovou značkou, uzavírá každou sekundu, z každé
jednotky si ponechá to, co vybere její pravidlo záznamu, a zapisuje to na dvě karty microSD jako HMC,
řadu souborů pro každou jednotku, drží zrcadlo konfigurace stanice a posílá data domů — Wi-Fi pro
archiv, modem pro tenký rámec hodnot, BLE pro telefon u otevřené skříně. **Neinterpretuje payload
(užitečná data) žádného frontendu**: seismický vzorek i blok dat GNSS jsou bajty pod určitým TYPE,
které ponechá nebo zahodí pravidlo porovnávající bajty, a čte je až server. Jednou za minutu čte
přes Hermes stav akumulátoru a je jediným prvkem stanice, který vypíná jednotky v daném pořadí dřív,
než je vyřadí deficit.

## Jak je připojen ke stanici

- **Dolů: čtyři páteřní linky, na každé jedna karta.** Spoje MasterNOD bod–bod na 2²¹, prosté úrovně
  UART na kříženém kabelu uvnitř skříně; vlastní NodBus začíná až za kartami. Centrála mluví s kartou
  třemi slovesy — spusť svůj FLOOR, napájej port, taktuj port — a pak naslouchá. Neosazená ploška
  pro pull-up 330 Ω na `RXD` každé páteřní linky je celou hardwarovou přípravou na druhou kartu na
  jednom portu, kterou firmware neimplementuje a kterou žádná deska neosazuje.
- **Čas: odběr z plochého kabelu Kronosu.** Čas centrály je hrana `PPS_K`, zachycená hardwarově,
  a štítek na I²C časové sběrnice; hodinový pár se přijímá a nepoužívá. Centrála neopatřuje časovou
  značkou nic, co se dostane do archivu — každý záznam nese sekundu karty.
- **Napájení: Hermes na LP I²C**, převodník k BMS a MPPT, nikdy neodpojovaný; v režimu přežití ho
  čte jen samotné jádro LP. Záložní článek udrží centrálu při výpadku 12 V dost dlouho na to, aby
  zapsala všechny buffery a nahlásila příčinu.
- **Ven: Wi-Fi, pozice pro modem, BLE a USB.** Rámec hodnot jednou za hodinu a při značce; archiv
  přes Wi-Fi tam, kde ji lokalita má; telefon v okně otevíraném tlačítkem; konzole USB v každém
  stavu a napájení pro centrálu mrtvé stanice.

```
   KRONOS ──PPS-K + the label──▶ ┌──────────────────────────────┐ ──trunk 1..4, 48 B MasterNOD, point-to-point──▶ the BIFROST / ARGUS cards
                                 │ MAYAK — ESP32-S31            │
   HERMES ◀──LP I²C + ALERT──────│ the store · the mirror ·     │ ──▶ modem · Wi-Fi · BLE · USB
                                 │ the uplink                   │
                                 └──────────────────────────────┘
```

## Strom

```
mayak/        Mayak — the head: datalogger and uplink  NodBus 1
└── handset/  Handset — the commissioning app, over BLE
```

## Soubory

| soubor | obsah |
|---|---|
| [`HARDWARE.md`](HARDWARE.md) | deska — modul S31, konektory a každý pin, páteřní linka jako odbočka, odběr z časové sběrnice, rozpočet sériových linek a DMA, vstup 12 V, napájecí linka, záložní článek, servisní napájení z USB, součástky, zkoušky na stole |
| [`FIRMWARE.md`](FIRMWARE.md) | popis firmwaru — páteřní linky, čas na centrále, příjem dat a archiv, strom, čísla a zrcadlo konfigurace, řídicí rovina, uplink, servisní okno, napájení a přežití, vyhodnocení událostí, co je otestováno |
| [`WHY.md`](WHY.md) | hřbitov — zamítnuté alternativy a překonané stavy |
| [`handset/`](handset/) | popis aplikace pro telefon a kontrakt GATT, který dodržuje s centrálou |

## Licence

Hardware: CERN-OHL-S v2 (`../LICENSE-HW`) · Software: MIT (`../LICENSE`) — Copyright (c) 2026 NIC — Native Intellect Community

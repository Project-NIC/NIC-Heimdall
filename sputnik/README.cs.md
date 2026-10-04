<p align="center">
  <img src="NIC-Sputnik.svg" width="200"/>
</p>

★ N.I.C. ★

# Sputnik — ionosférický frontend GNSS a čas stanice

[English](README.md) · **Čeština** · [Русский](README.ru.md)

> **Koncept ve fázi návrhu — nic není postaveno.** Čísla se ověřují proti katalogovým listům a součástkám, než vznikne deska.

**Sputnik čte ionosféru z GNSS: NodBus typ 8, NOD.** Jeden vícesystémový, vícefrekvenční přijímač
Unicore **UM980** a jeden STM32H523 na jedné desce. Posílá proudem surové observace každé sledované
družice na třech frekvencích — pseudovzdálenost, fázi nosné, SNR — s frekvencí 5 Hz, z nichž se
počítá **celkový elektronový obsah** každé dráhy nad stanicí a spolu s přízemní meteorologií Palatine
**srážitelná vodní pára**.

**Tentýž přijímač je zdrojem času GNSS pro stanici.** Jeho PPS a úsporný proud NMEA jdou do
Kronosu přes druhou zásuvku desky; na stanici se Sputnikem se nic jiného neosazuje a stanice bez
něj osadí Polaris (`../kronos/polaris/`). Kronos zůstává autoritou, která disciplinuje a
rozvádí — UM980 je jen jeho zdroj.

## Jak je připojen ke stanici

- **Ve skříni, vždy** — anténa je umístěna s výhledem na oblohu na vlastním stožáru stanice a dlouhý
  je jen koaxiální kabel (`../core/blocks/gps-pps.md`, *The receiver lives in the enclosure*). Obě
  zásuvky pak přijímají křížený kabel uvnitř skříně: zásuvka NodBus k portu karty, časový port
  k přijímačové zásuvce Kronosu. Obě jsou také postaveny tak, aby každá přijala jednu trasu Galvani,
  časová trasa obráceně — PPS dovnitř u Kronosu — a Kronos u ní změří vzdálenost.
- **Nahoru: NodBus, pět NUMBERů, pět slotů** — jedna fronta, kterou H523 plní bloky po 32 B z epoch
  o délce 200 ms; master je skládá podle indexu rámce a pořadí slotů a PRN každého záznamu udává jeho
  družicový systém.
- **Do strany: časový port ke Kronosu** — PPS přijímače na kanálu B přímo z jeho pinu, proud
  `RMC`/`GGA` přeposílaný na kanálu A, obsluhovaný, dokud odpovídá heartbeat Kronosu. **`END`
  ukončuje měřicí stranu, nikdy ne přeposílání**: zdroj času stanice odchází jen s úplným uzamčením.

```
   the antenna ──coax──▶ ┌────────────────────────────┐ ── NodBus, type 8, five NUMBERs ──▶ BIFROST
                         │ SPUTNIK — H523 · UM980     │
                         │ raw observables, 5 Hz      │ ── PPS + NMEA, the time port ──▶ KRONOS
                         └────────────────────────────┘
```

## Soubory

| soubor | obsah |
|---|---|
| [`HARDWARE.md`](HARDWARE.md) | deska — piny H523, dvě zásuvky, UM980, anténa a její koaxiální kabel, napájecí linka |
| [`BUS.md`](BUS.md) | dvě vrstvy, epocha, záznam Tier B, záznam geometrie a frekvenční páteř, fond slotů, rychlost a šířka pásma, co se archivuje |
| [`PROCESSING.md`](PROCESSING.md) | TEC a PWV a kde se počítají — stanice, okrsková skříň nebo server, jedna pozorovaná veličina |
| [`FIRMWARE.md`](FIRMWARE.md) | popis firmwaru — konfigurace přijímače, epocha a proud bloků, pět slotů, přeposílání do Kronosu, registry, co je otestováno |
| [`WHY.md`](WHY.md) | hřbitov — zamítnuté alternativy a překonané stavy |

## Licence

Hardware: CERN-OHL-S v2 (`../LICENSE-HW`) · Software: MIT (`../LICENSE`) — Copyright (c) 2026 NIC — Native Intellect Community

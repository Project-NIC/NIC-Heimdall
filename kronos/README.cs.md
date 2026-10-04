<p align="center">
  <img src="NIC-Kronos.svg" width="200"/>
</p>

★ N.I.C. ★

# Kronos — hodiny stanice

[English](README.md) · **Čeština** · [Русский](README.ru.md)

> **Koncept ve fázi návrhu — nic není postaveno.** Čísla se ověřují proti katalogovým listům a součástkám, než vznikne deska.

**Samostatná deska s STM32H523, která nese jedinou přesnou součástku ve stanici: TCXO 2²⁴
disciplinovaný přijímaným pulzem.** Kronos přijímá PPS a NMEA z přijímače GNSS nebo z Pip, na dvou
zásuvkách. Řídí svou PLL podle pulzu, jednou odečte zpoždění přijímacího řetězce a předává stanici
hodinový signál a pojmenovanou sekundu po jedné časové sběrnici. Všechny ostatní desky tento hodinový
signál počítají a žádná si nevede vlastní čas.

```
   POLARIS or SPUTNIK ──NMEA + PPS, a data body──▶ ┌────────────────────────────┐
   PIP, where fitted  ──NMEA + PPS, a data body──▶ │ KRONOS — STM32H523         │
                                                   │ TCXO 2²⁴ · PLL1, FRACN     │
                                                   │ steered to the pulse       │
                                                   │ the delay table, once      │
                                                   └──────────────┬─────────────┘
                                                                  │ the time bus, one 10-pin ribbon:
                                                                  │ CLK 2²² · PPS_K on M-LVDS,
                                                                  │ the label on I²C, ATTN high
                                   ┌──────────────┬───────────────┼──────────────┬──────────────┐
                                   ▼              ▼               ▼              ▼              ▼
                                card 1         card 2          card 3         card 4        MAYAK
                               (ATTN high → a Bifrost)                                  the whole bus
```

**Co rozdává.** `CLK` na 2²², vlastní rychlosti odbočky, a `PPS_K`, odvozenou sekundu, obojí dělené
z jednoho řízeného VCO, takže sekunda má přesně 2²² period hodin a nikdy neskočí. **Štítek** — celá
unixová sekunda a kvalita — jde na I²C časové sběrnice jednou za minutu a při každé změně kvality.
Přijatý pulz řídí smyčku a nikdy neopustí desku.

**Co přijímá.** Přijímač GNSS jednoho ze dvou pojmenovaných typů, zadaný typem a nikdy
nedetekovaný — `M8N` na Polaris, který Kronos při každém startu kontroluje a konfiguruje, nebo
`UM980` na Sputniku — na první zásuvce; Pip na druhé.
Bez obou nastaví Mayak po sběrnici hrubou počáteční sekundu a čas se hlásí jako nastavený zvenčí.
Když zdroj zmizí, nese stanici TCXO — 1 ppm je 86 ms za den — a kvalita hlásí holdover.

**Čím přispívá k chybě: nanosekundami.** Stanice dodává ±1 µs; mřížka zachytávání je zde 7,45 ns
a tabulka zpoždění se odečítá na této mřížce.

**Kde stojí.** Ve skříni, na vodiči 12 V jako každý hostitel, vedle Mayaku a řady karet. Nemá nad
sebou žádný `ENABLE`; vypnutí je Stop, který nařídí Mayak, a probuzení je shoda adresy na časové
sběrnici.

## Strom

```
kronos/         Kronos — the station's clock
└── polaris/    Polaris — its GNSS front, bought: a receiver and an antenna on a carrier
```

## Soubory

| soubor | obsah |
|---|---|
| [`HARDWARE.md`](HARDWARE.md) | deska: TCXO a PLL, časová sběrnice, zásuvky, piny procesoru, časovače a strom hodin, napájení, součástky |
| [`FIRMWARE.md`](FIRMWARE.md) | popis firmwaru: start, stavy, zdroje, disciplinační smyčka, PPS-K, tabulka zpoždění, štítek a registry, poruchy, perzistence, testy |
| [`WHY.md`](WHY.md) | hřbitov: zamítnuté alternativy a překonané stavy, s odůvodněním |
| [`polaris/`](polaris/) | Polaris — čistě časový frontend GNSS, který stanice osadí na `TIME IN 1`, když nemá Sputnik |

## Licence

Hardware: CERN-OHL-S v2 (`../LICENSE-HW`) · Software: MIT (`../LICENSE`) — Copyright (c) 2026 NIC — Native Intellect Community

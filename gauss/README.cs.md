<p align="center">
  <img src="NIC-Gauss.svg" width="200"/>
</p>

★ N.I.C. ★

# Gauss — magnetometrická sonda

[English](README.md) · **Čeština** · [Русский](README.ru.md)

> **Koncept ve fázi návrhu — nic není postaveno.** Čísla se ověřují proti katalogovým listům a součástkám, než vznikne deska.

**Gauss sleduje pomalé magnetické pole** — bouře, náhlé začátky bouří, pulzace, dB/dt a pod mořem
signál pohybové indukce tsunami. Je to tříosý magnetoinduktivní magnetometr (`RM3100`) se dvěma
teploměry a H523 v utěsněné trubce, který hlásí **odchylku pole od uložené základní hodnoty**.
Tesla poslouchá rychlé pole, Gauss pomalé — dvě jednotky téže veličiny.

**Osazuje se tehdy, když žádné jiné čidlo v sestavě nenese magnetometr.** Tam, kde čip nese Quake,
je to magnetometr stanice a žádný Gauss se nestaví.

| | |
|---|---|
| třída | **mini-NOD — NodBus mini typ 1 `Gauss`**, za kartou Argus; adresa `TYPE«4 \| NUMBER`, **0x11** pro jednotku 1 |
| čidlo | `RM3100` — tři magnetoinduktivní cívky, třída ~13 nT, rozlišení je výsledkem layoutu a filtrace (`HARDWARE.md`) |
| teploměry | `TMP117` na stěně trubky — teplota země nebo vody v hloubce, trend v mK; NTC mezi cívkami — pro regresi driftu |
| MCU | `STM32H523`, LQFP100 — bez krystalu: jádro běží na HSI, které disciplinuje stupeň hodin segmentu |
| payload, 8 B | bajty **0–5** tři osy, odchylka od základní hodnoty jako `int16`; **6–7** rezerva. Spouštěčem je příznak **ALARM** v hlavičce, napájení hlásí příznak **SUPPLY** |
| jednou za minutu | **rámec `REPORT`** ve čtvrtém rámcovém čase slotu: `VBUS` · `CURRENT` · `TMP117` na stěně · NTC cívek |
| pouzdro | trubka ~40 mm, **dvě sestavy**: pozemní — trubka PPR nebo HT, jedna vícestupňová průchodka, žádný konektor, víčka svařená; mořská — trubka PVDF, olej nebo vazelína, lepená průchodka dole, do ~250 m (`CONSTRUCTION.md`); hlubinná sestava patří k Atlantis |
| napájení | 12 V z napájecí desky jednotky na konci u pouzdra; trasa 48 V nebo 300 V podle své délky a komunikační modul, který jí odpovídá — pod vodou tlaková provedení desek rodiny |
| odběr | ~0,5–1 W podle komunikačního modulu |

**Pouzdro nedělá žádnou analýzu.** Vzorkuje, koriguje podle vlastní kalibrace a teploty,
interpoluje na mřížku rámců a odečítá základní hodnotu; spektra, dlouhé filtry a slučování dat
z pole sond běží o úroveň výš.

## Blokové schéma

```
   an ARGUS segment ══ the feed + the 2¹⁹ rung ══▶ ┌────────────────────────────────────────────────┐
   the unit power board and the communication      │ GAUSS — the Barrel pod, mini type 1            │
   board at the pod end                            │ Galvani boards │ H523 │ the gap │ RM3100 coils │ tip
                                                   └────────────────────────────────────────────────┘
   8 B a frame: the field; the wall TMP117 and the coil NTC once a minute
```

## Soubory

| Soubor | Obsah |
|---|---|
| [`HARDWARE.md`](HARDWARE.md) | deska čidla — napájecí sběrnice, hodiny a jejich disciplinace, piny, čidlo a pravidla kolem něj |
| [`CONSTRUCTION.md`](CONSTRUCTION.md) | dvě sestavy — pozemní trubka a mořská trubka z PVDF, výplň, kde stojí, kalibrace bez otáčení instalace |
| [`ARRAY.md`](ARRAY.md) | pobřežní pole sond pro tsunami a oceánské proudění — Tier 2, odložený koncept |
| [`FIRMWARE.md`](FIRMWARE.md) | popis firmwaru — dotazování, časová značka `DRDY` a interpolace, kalibrace a základní hodnota, payload, registry, co je otestováno |
| [`WHY.md`](WHY.md) | hřbitov — co se zkusilo, co padlo a proč |
| [`models/`](models/) | model umístění v mělké vodě, na kterém stojí hloubkové pravidlo v `ARRAY.md` |

## Licence

Hardware: CERN-OHL-S v2 (`../LICENSE-HW`) · Software: MIT (`../LICENSE`) — Copyright (c) 2026 NIC — Native Intellect Community

★ N.I.C. ★

# Polaris — frontend GNSS pro Kronos, kupovaný

[English](README.md) · **Čeština** · [Русский](README.ru.md)

> **Koncept ve fázi návrhu — nic není postaveno.** Čísla se ověřují proti katalogovým listům a součástkám, než vznikne nosič.

**Kupovaný modul přijímače, kupovaná aktivní anténa a nosič velikosti modulu, který je připojí na
datový konektor Kronosu. Žádný procesor, žádná sběrnice, žádná adresa.** Polaris je to, co stanice
osadí, když nemá Sputnik: čistě časový přijímač GNSS na zásuvce `TIME IN 1` Kronosu, který mu předává
úsporný proud NMEA a hranu PPS. Kronos disciplinuje a rozvádí; tohle je jen jeho zdroj.

```
   ACGPSA, active antenna, mast-top, on an insulating bracket
        │ coax ≤ 4 m, the station's two DC-pass arresters on it
        ▼ SMA
   ┌────────────────────────────────────┐
   │ POLARIS — the carrier              │
   │   the bought module: NEO-M8N,      │      one Galvani data connector, 12 pins
   │   its bias tee, SMA                │─────────────────────────────────────────▶ KRONOS
   │   ID = GNSS, 33 Ω on the PPS       │      CLK/PPS · GND · TXD · RXD · ID · 3,3 V
   └────────────────────────────────────┘
```

**Přijímač nikdy neopouští skříň** (`../../core/blocks/gps-pps.md`). Dlouhý je jen koaxiální kabel
a RF je to jedno. Polaris tedy nemá žádnou bariéru k překročení, žádné napájení k zajištění a nic
k měření vzdálenosti: je to nosič, ne deska Galvani a ne jednotka.

**Jeden typ, `M8N`, zadaný do Kronosu a nikdy nedetekovaný.** Kronos ho kontroluje při každém
startu — `MON-VER` — a konfiguruje na jeden profil, takže modul čerstvě z krabice i modul, který
někdo překonfiguroval, naběhnou stejně. **`UM980` je součástka Sputniku a zde se neobjevuje** —
stanice se Sputnikem už svůj zdroj času má.

**Jeho zpoždění jsou dvě zadaná čísla a žádné měření.** `CAB` je koaxiální kabel, metry × ~5 ns/m;
`INT` je anténa a modul jako jedna konstanta typu. Obě se odečítají na Kronosu, jednou.

| soubor | co obsahuje |
|---|---|
| [`HARDWARE.md`](HARDWARE.md) | co se kupuje a co to musí mít, nosič, konektor pin po pinu, anténa a její montáž, složky zpoždění |
| [`WHY.md`](WHY.md) | hřbitov — vlastní karta s vlastním bias tee, přijímač na vrcholu stožáru, vlastní kód, čtyři typy |

## Licence

Hardware: CERN-OHL-S v2 (`../../LICENSE-HW`) · Software: MIT (`../../LICENSE`) — Copyright (c) 2026 NIC — Native Intellect Community

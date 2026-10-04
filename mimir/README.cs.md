<p align="center">
  <img src="NIC-Mimir.svg" width="200"/>
</p>

★ N.I.C. ★

# Mimir (mini-Heimdall) — centrála, hodiny, dvě karty a Palatine na jedné desce

[English](README.md) · **Čeština** · [Русский](README.ru.md)

> **Koncept ve fázi návrhu — nic není postaveno.** Čísla se ověřují proti katalogovým listům a součástkám, než vznikne deska.

**Mimir je jedna DPS nesoucí šest sekcí stanice — Mayak, Kronos, Bifrost, Argus, Palatine a Hermes —
každou rozvrženou tak, jak ji kreslí její vlastní dokument, s nezměněnými piny a nezměněným
firmwarem.** Deska odstraňuje to, co je ve stanici spojovalo: odběr Bifrostu z plochého kabelu
časové sběrnice, křížené kabely uvnitř skříně z páteřní linky Mayaku k Bifrostu a z portů Bifrostu
k Palatine a Argusu a čtyři z pěti párů svorek 12 V s jejich pojistkami. Navenek má **dva porty
NodBus, čtyři segmenty mini, čtyři větve ModBus (arm) s `PWR EXT`, jeden volný MasterNOD pro vlastní
Bifrost, Ethernet a USB**. Je to skříň stanice, která měří počasí, s několika odbočkami a několika
sondami, a dá se rozšířit o jednu další kartu.

Pojmenován po **Mímirovi**, jehož studna leží pod kořenem stromu a chová Heimdallův roh.

```
            ┌──────────────────────────────────────────────────────────────────────────────┐
 12V in ────┤  KRONOS ── CLK · PPS_K · SDA · SCL · ATTN as traces ──▶ BIFROST              │
  + tap     │   TIME IN 1 (Polaris) · TIME IN 2 · TIME BUS ──▶ ribbon  │  port 1 traces ──▶ PALATINE ── MB OUT 1…4 · PWR OUT 1…4 · PWR EXT
 one fuse   │                                                          │  port 2 traces ──▶ ARGUS ──── MINI OUT 1…4 · PWR OUT 1…4
            │  MAYAK ── MasterNOD 1 as traces ─────────────────────────┘  NB/MINI OUT 3, 4 ──── two spurs
            │    │   MNB OUT 2 ───────────────────────────────────────────────────────────── a further card
            │    │   SD · modem · Wi-Fi · BLE · LP I²C ──▶ HERMES ── 485 · CAN · UART · I²C out
            └────┼─────────────────────────────────────────────────────────────────────────┘
                 └── USB · Ethernet
```

## Soubory

| soubor | obsah |
|---|---|
| [`HARDWARE.md`](HARDWARE.md) | šest sekcí, co se stane vodivou cestou a co každá cesta zachovává, co je osazeno, Ethernet, 12 V, rozmístění, co přináší a co stojí |
| [`WHY.md`](WHY.md) | hřbitov |

## Licence

Hardware: CERN-OHL-S v2 (`../LICENSE-HW`) · Software: MIT (`../LICENSE`) — Copyright (c) 2026 NIC — Native Intellect Community

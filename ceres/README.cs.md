<p align="center">
  <img src="NIC-Ceres.svg" width="200"/>
</p>

★ N.I.C. ★

# Ceres — čidlo vlhkosti půdy

[English](README.md) · **Čeština** · [Русский](README.ru.md)

> **Koncept ve fázi návrhu — nic není postaveno.** Čísla se ověřují proti katalogovým listům a součástkám, než vznikne deska.

**Ceres měří objemový obsah vody v půdě v referenční hloubce WMO a teplotu půdy v téže hloubce:
ModBus typ 6, MOD na větvi ModBus (arm) Palatine.** Hřebenová elektroda na spodní straně desky měří
skrz 1 mm borosilikátového skla — deska je zalitá v borosilikátové misce hřebenem dolů, se sklem na
obou stranách — a čte se jako admitance na třech frekvencích: kvadraturní složka je voda, složka ve
fázi jsou ionty a frekvence obojí oddělí. Jednotka odpovídá hotovou, kompenzovanou hodnotou.
Několik jednotek leží v zemi v **patroně**, trubce naplněné na pracovním stole a zatlučené do země
jako jeden celek (`../palatine/CONSTRUCTION.md`, *The soil column*), a právě to zajišťuje, že je
hloubka mezi stanicemi opakovatelná.

| | |
|---|---|
| třída | **vlastní MOD** — Modbus RTU slave na větvi Palatine, **ModBus typ 6**, adresa `6«2 \| NUMBER`, 0x18 pro jednotku 0; bez hodin, údaj označí časem hostitel, když se dotazuje |
| co hlásí | obsah vody na `0x0000`, teplotu půdy v dané hloubce na `0x0001` — jedna jednotka, jedna adresa, dvě hodnoty (`MODBUS.md`) |
| snímací prvek | vlastní hřeben desky pod sklem misky, vodorovně v udusaném písku patrony, sklem dolů |
| čtení | synchronní detektor na 2²⁰ / 2²² / 2²⁴ Hz — 1,05 / 4,19 / 16,8 MHz — proti referenčnímu kondenzátoru přes tentýž řetězec (`HARDWARE.md`) |
| součástky | `STM32H523VE` · `THS4541` · 3× `74LVC1G3157` · `74LVC1G17` · `TMP117` nebo `STS35` · `THVD1450` · `LMR43610` |
| hloubky | standardní řada 5 · 10 · 20 · 50 · 100 cm — dvojice pro stanici −10 a −50 cm, profil pro farmu −10 · −20 · −50 · −100 cm; které hloubky stanice nese, je rozhodnutí podle plodiny a lokality |
| napájení | izolovaných 12 V z větve na čtyřvodičovém kabelu, jako každé čidlo na větvi; deska si vyrábí vlastních 3,3 V |
| kabel | plochý, čtyři vodiče, připájený k desce a zalitý skrz kanálek misky; na jednotce žádný konektor |

**Sakura je tatáž deska v téže misce, zavěšená v porostu** — `sakura/`.

## Blokové schéma

```
   a Palatine arm ── A · B · 12 V · GND ── the flat cable, up the patrona
        │
   ┌────┴─────────────────────────────────────────────────────────────────────────┐
   │ CERES — the board potted in a borosilicate bowl                              │
   │  H523 · THVD1450 · the detector chain                                        │
   │  the comb on the underside ── 1 mm of glass ── the packed sand at the depth  │
   │  the thermometer on the board ── the soil temperature there                  │
   └──────────────────────────────────────────────────────────────────────────────┘
```

## Strom

```
ceres/       Ceres — soil moisture and the soil temperature at its depth  ModBus 6
└── sakura/  Sakura — leaf wetness, Ceres's board in the same bowl        ModBus 7
```

## Soubory

| soubor | obsah |
|---|---|
| [`HARDWARE.md`](HARDWARE.md) | deska a miska — měření, řetězec, napájecí sběrnice, zalévání, každý pin, součástky, kritéria zkoušky na stole; jediná společná deska pro Ceres a Sakura |
| [`FIRMWARE.md`](FIRMWARE.md) | popis firmwaru — cyklus čtení na třech frekvencích, výpočet admitance, křivka a kompenzace, co je otestováno |
| [`MODBUS.md`](MODBUS.md) | kontrakt ModBus obou jednotek |
| [`WHY.md`](WHY.md) | hřbitov — co se zkusilo, co padlo a proč |
| [`sakura/`](sakura/) | Sakura — tatáž deska jako čidlo ovlhčení listů |

## Licence

Hardware: CERN-OHL-S v2 (`../LICENSE-HW`) · Software: MIT (`../LICENSE`) — Copyright (c) 2026 NIC — Native Intellect Community

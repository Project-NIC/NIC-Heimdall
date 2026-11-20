★ N.I.C. ★

# Arke — větev ModBus v terénu

[English](README.md) · **Čeština** · [Русский](README.ru.md)

> **Koncept ve fázi návrhu — nic není postaveno.** Čísla se ověřují proti katalogovým listům a součástkám, než vznikne deska.

**Arke je deska Iris ve druhé roli: master vlastní větve ModBus tam, kam kabel nedosáhne**, na
vlastním 12V packu a panelu, hlásící se Iris nebo Huginnovi — malý Heimdall, který zná jen pomalé
věci. Na jeho větvi visí kupovaná čidla RS-485 ze seznamu Palatine a vlastní MODy — **Ceres** pro
půdu, **Sakura** pro list — napájené jen po dobu čtení. Levná GNSS drží jeho čas a dává jeho polohu.

**Co přidává k sadě Ratatoskra** (`../../README.md`): rychlost a směr větru ve 2 m, globální záření,
vlhkost a teplotu půdy, měřené ovlhčení listu — s nimi evapotranspiraci (FAO-56 Penman–Monteith) a
závlahu a vítr, který zastaví postřik.

| | |
|---|---|
| deska | **ta z Iris** — `../HARDWARE.md`; osazená GNSS |
| vstup | samčí M12: 12 V z jeho packu a pár k BMS a MPPT |
| výstup | samičí M12: spínaných 12 V a pár jeho větve |
| pack | 4S sodík-iont nabíjený jako LiFePO4, 12,4–14,4 V; kupované BMS a MPPT; panel 20–50 W |
| spojení | [`../../LINK.md`](../../LINK.md) — jeho bloky v podobě Palatine, nejvýš 51 B na paket |

```
  panel ── MPPT ── pack ── BMS          (the pair to the BMS and the MPPT)
                     │                         │
                     └── 12 V ── ARKE (male in) ┘ ── female out: switched 12 V + 485 ── wind · radiation ·
                                  GNSS · SX1262                                       Ceres · Sakura · …
```

| soubor | |
|---|---|
| [`HARDWARE.md`](HARDWARE.md) | čím se liší od Iris: pack, BMS a MPPT, GNSS, větev |
| [`FIRMWARE.md`](FIRMWARE.md) | čím se liší: probouzení, větev, hodiny z GNSS |
| [`WHY.md`](WHY.md) | vlastní záznamy Arke; záznamy desky jsou u Iris |

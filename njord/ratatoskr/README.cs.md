★ N.I.C. ★

# Ratatoskr — malá polní jednotka

[English](README.md) · **Čeština** · [Русский](README.ru.md)

> **Koncept ve fázi návrhu — nic není postaveno.** Čísla se ověřují proti katalogovým listům a součástkám, než vznikne deska.

**Ratatoskr měří to, čím se liší místa na poli nebo v sadu — teplotu a vlhkost vzduchu ve 2 m a v
5 cm, tlak, srážky tam, kde je Imber — na jednom článku a malém skleněném panelu, a posílá to Iris
nebo Huginnovi.** Za klidné noci se vzduch vrství: tráva vychladne k rosnému bodu, zatímco 2 m zůstávají
sušší, a o 150 m dál přes silnici, vysoké stromy proti nízkým, se to liší znovu — takže pár jednotek,
jedna uprostřed a po jedné v rozích, zachytí mrazové kotliny, rosu a hodiny ovlhčení listu, které
chtějí modely houbových chorob.

| | |
|---|---|
| kde | krabice v 1 m na stromě nebo na 3m tyči v díře od vrtáku; panel v ~3 m; anténa nahoře |
| čidla | `SHT45` ve 2 m v malém žaluziovém krytu a v 5 cm, holé, jako u Palatine; `BMP585` v krabici |
| napájení | jeden sodík-iontový článek, 2,5–3,6 V; nPM1300; malý skleněný panel |
| MCU | STM32U535, LQFP48 — jediné MCU mimo seznam stanice (`../../NAMING.md`) |
| spojení | [`../LINK.md`](../LINK.md) — 27 B každých deset minut, potvrzeno |
| jeho den | **~0,2–2,3 mAh** (`../calc/power.py`); větší odběr je samovybíjení článku |

```
           panel ~3 m ── M12 ──┐
   SHT45 2 m (shield) ── M8 ──┤
                               RATATOSKR box at 1 m (U535 · SX1262 · nPM1300 · cell · BMP585) ── N ── antenna
   SHT45 5 cm (bare) ─── M8 ──┤
   Imber (where fitted) ─ M8 ─┘
```

| soubor | |
|---|---|
| [`HARDWARE.md`](HARDWARE.md) | deska, napájecí řetězec, sondy, piny, krabice |
| [`FIRMWARE.md`](FIRMWARE.md) | probouzení, spánek, ohřev, uskladnění |
| [`WHY.md`](WHY.md) | co se zvažovalo a zůstalo stranou |

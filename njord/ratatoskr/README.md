★ N.I.C. ★

# Ratatoskr — the small field unit

**English** · [Čeština](README.cs.md) · [Русский](README.ru.md)

> **Design-stage concept — nothing built.** The figures are verified against the parts' sheets and
> the parts before a board is made.

**Ratatoskr measures what differs across a field or an orchard — the air's temperature and humidity
at 2 m and at 5 cm, the pressure, the rain where Imber is fitted — on one cell and a small glass panel,
and sends it to Iris or Huginn.** On a calm night the air layers: the grass cools to the dew point
while 2 m stays drier, and 150 m across a road, tall trees against low ones, differ again — so a few
units, one in the middle and one in each corner, catch the frost pockets, the dew and the leaf's wet
hours that the fungal models want.

| | |
|---|---|
| where | the box at 1 m on a tree or a 3 m pole in an auger's hole; the panel at ~3 m; the antenna at the top |
| sensors | `SHT45` at 2 m in a small louvered shield and at 5 cm, bare, as Palatine's; `BMP585` in the box |
| power | one sodium-ion cell, 2,5–3,6 V; the nPM1300; a small glass panel |
| MCU | STM32U535, LQFP48 — the one MCU outside the station's list (`../../NAMING.md`) |
| link | [`../LINK.md`](../LINK.md) — 27 B every ten minutes, acknowledged |
| its day | **~0,2–2,3 mAh** (`../calc/power.py`); the cell's own self-discharge the larger draw |

```
           panel ~3 m ── M12 ──┐
   SHT45 2 m (shield) ── M8 ──┤
                               RATATOSKR box at 1 m (U535 · SX1262 · nPM1300 · cell · BMP585) ── N ── antenna
   SHT45 5 cm (bare) ─── M8 ──┤
   Imber (where fitted) ─ M8 ─┘
```

| file | |
|---|---|
| [`HARDWARE.md`](HARDWARE.md) | the board, the power chain, the probes, the pins, the box |
| [`FIRMWARE.md`](FIRMWARE.md) | the wake, the sleep, the heater, the shelf |
| [`WHY.md`](WHY.md) | what was weighed and left |

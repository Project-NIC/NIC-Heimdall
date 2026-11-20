★ N.I.C. ★

# Njord — Heimdall's air part

**English** · [Čeština](README.cs.md) · [Русский](README.ru.md)

> **Design-stage concept — nothing built.** The figures are verified against the parts' sheets and
> the parts before a board is made.

**Njörðr is the part of Heimdall that reaches by radio.** Midgard — the station on its cables and its
glass — measures where a cable reaches; Njord measures beyond it: slow quantities, a sample every ten
minutes, from small units on their own panel and cell, joined to Heimdall or straight to the internet
over LoRa at 868 MHz. Nothing fast rides it: EU 868's air carries tens of bits a second on average,
where a seismometer needs kilobits.

| unit | what it is | where it stands | MCU |
|---|---|---|---|
| **[Iris](iris/)** | the receiver: a MOD on a Palatine arm, ModBus type 8, every field unit a multi-sensor behind its address | at the station, its antenna on the mast | STM32H523 (LQFP48) |
| **[Arke](iris/arke/)** | **Iris's board in its second role** — a ModBus arm of its own in the field: bought RS-485 sensors and the house MODs, its own 12 V pack and panel, a GNSS for its time | a mast in the field | STM32H523 (LQFP48) |
| **[Ratatoskr](ratatoskr/)** | the small unit: temperature and humidity at 2 m and 5 cm, pressure, the rain where Imber is fitted; one cell, a small glass panel | a tree or a 3 m pole, an orchard's corners and middle | STM32U535 (LQFP48) |
| **[Huginn](huginn/)** | the gateway to the internet with no Heimdall at all — LoRa in, Wi-Fi out, the time from NTP | a window, a balcony, on a USB supply | ESP32-S3 |

```
   MIDGARD (the station)                                   NJORD (the field)
   Palatine ─── ModBus arm ─── IRIS  ))) 868 MHz (((  ARKE ── its own arm ── bought sensors · Ceres · Sakura
                                     │                (12 V pack, BMS, MPPT, panel, GNSS)
                                     ))) 868 MHz (((  RATATOSKR × n  (one cell, a small panel)
   the internet ─── Wi-Fi ── HUGINN  ))) 868 MHz (((  the same units, with no station
   a LoRaWAN network ─ its gateways  ))) 868 MHz (((  the same units, hourly, with neither
```

**One link for all four** ([`LINK.md`](LINK.md)): a packet every ten minutes, acknowledged; the
values as the house map's registers, big-endian, so the receiver hands on the bytes that flew; an
8 B tag on every packet and acknowledgement; the time stamped where the packet arrives — Iris by
Palatine's poll, Huginn by NTP — the unit giving only how long before sending it measured. A setting
or two may be sent back in the acknowledgement; never the registers, never the firmware.
**Every unit runs on LoRaWAN as well**, public or a site's own — the same packet as the payload,
six samples hourly where a public network's fair use binds (`LINK.md` §7).

**What each unit measures follows what a field's decisions need.** Ratatoskr gives the frost, the dew
and the leaf's wet hours, the pressure and the rain — the frost warning and the fungal models. Arke
adds the wind at 2 m (spraying stops in wind), the global radiation, the soil (Ceres) and measured leaf
wetness (Sakura) — with them the evapotranspiration and the irrigation. UV, ozone and particulates are
the station's.

| file | what is in it |
|---|---|
| [`LINK.md`](LINK.md) | the radio link — bands, cadence, the packets byte by byte, the acknowledgement, the tag and the keys, the slots, LoRaWAN, the reach |
| [`iris/`](iris/) | Iris and its board — the hardware and the firmware of both roles |
| [`iris/arke/`](iris/arke/) | Arke — what differs from Iris |
| [`ratatoskr/`](ratatoskr/) | Ratatoskr — its board, its probes, its firmware |
| [`huginn/`](huginn/) | Huginn — the gateway |
| [`calc/`](calc/) | the power budgets (`power.py`) and the reach (`link.py`), run as they stand |

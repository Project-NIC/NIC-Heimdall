★ N.I.C. ★

# Arke — a ModBus arm in the field

**English** · [Čeština](README.cs.md) · [Русский](README.ru.md)

> **Design-stage concept — nothing built.** The figures are verified against the parts' sheets and
> the parts before a board is made.

**Arke is Iris's board in its second role: the master of a ModBus arm of its own, beyond a cable's
reach**, on its own 12 V pack and panel, reporting to Iris or Huginn — a small Heimdall that knows only
slow things. On its arm hang the bought RS-485 sensors of Palatine's list and the house MODs —
**Ceres** for the soil, **Sakura** for the leaf — fed only while they are read. A cheap GNSS keeps its
time and gives its place.

**What it adds to Ratatoskr's set** (`../../README.md`): the wind's speed and direction at 2 m, the
global radiation, the soil's moisture and temperature, the leaf's wetness measured — with them the
evapotranspiration (FAO-56 Penman–Monteith) and the irrigation, and the wind that stops a spraying.

| | |
|---|---|
| board | **Iris's** — `../HARDWARE.md`; the GNSS fitted |
| input | M12 male: its pack's 12 V and the pair to its BMS and MPPT |
| output | M12 female: the switched 12 V and its arm's pair |
| pack | 4S sodium-ion charged as LiFePO4, 12,4–14,4 V; a bought BMS and MPPT; a 20–50 W panel |
| link | [`../../LINK.md`](../../LINK.md) — its blocks in Palatine's shape, at most 51 B a packet |

```
  panel ── MPPT ── pack ── BMS          (the pair to the BMS and the MPPT)
                     │                         │
                     └── 12 V ── ARKE (male in) ┘ ── female out: switched 12 V + 485 ── wind · radiation ·
                                  GNSS · SX1262                                       Ceres · Sakura · …
```

| file | |
|---|---|
| [`HARDWARE.md`](HARDWARE.md) | what differs from Iris: the pack, the BMS and MPPT, the GNSS, the arm |
| [`FIRMWARE.md`](FIRMWARE.md) | what differs: the wake, the arm, the GNSS clock |
| [`WHY.md`](WHY.md) | Arke's own entries; the board's are Iris's |

★ N.I.C. ★

# Iris — Njord's receiver at the station

**English** · [Čeština](README.cs.md) · [Русский](README.ru.md)

> **Design-stage concept — nothing built.** The figures are verified against the parts' sheets and
> the parts before a board is made.

**Iris brings the field into the station: a MOD on a Palatine arm, ModBus type 8, the radio gateway.**
It receives every Njord unit within reach, acknowledges each, keeps the beat of their slots, and holds
each unit's latest record in a window of registers that Palatine reads like any sensor. **Each field
unit is a multi-sensor behind Iris's address** — as Palatine already reads a bought box of many values
in one block (Chinook's gases): the block's first register is the unit's number, the rest its values,
and the unit's profile — keyed by Iris's type and that number — says what each register is.

**One board, two roles.** The same board in the field is **[Arke](arke/)**; which it is, it finds at
its first start from the current its switched output draws, and keeps until a magnet resets it.

| | |
|---|---|
| class | **MOD** — ModBus type **8**, the radio gateway; one slave on a Palatine arm, up to four Irises a station |
| feed | the arm's isolated 12 V on its male M12 input, with the arm's pair |
| radio | Wio-SX1262 at 868 MHz, its antenna ~3 dBi on the mast's insulating bracket, ~8 m up |
| MCU | STM32H523, LQFP48 |
| link | [`../LINK.md`](../LINK.md) |

```
  Palatine arm ── M12 male: 12 V · 0 V · A · B ──▶ IRIS ── IPEX ── N ── coax ── antenna on the mast
                                                   (H523 · SX1262 · 485 · the beat's crystal)
                                                   M12 female: capped
```

| file | |
|---|---|
| [`HARDWARE.md`](HARDWARE.md) | the board of both roles — parts, pins, connectors, enclosure |
| [`FIRMWARE.md`](FIRMWARE.md) | the firmware of both roles — the role, the receiver, the windows and the register map |
| [`WHY.md`](WHY.md) | what was weighed and left |
| [`arke/`](arke/) | the board's second role |

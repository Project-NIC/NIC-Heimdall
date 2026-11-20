★ N.I.C. ★

# Ratatoskr — the firmware

> **Design-stage concept — nothing built.** The figures are verified against the parts' sheets and
> the parts before a board is made.

**A firmware of its own on the U535**, `core/proto`'s portable C beneath it: the packet, the tag, the
acknowledgement of [`../LINK.md`](../LINK.md).

## 1 The sleep

**Stop 3 with all 274 KB of SRAM kept, 2,2 µA** (ST DS14217 rev 5), the RTC waking it at the slot the
last acknowledgement gave. The RAM keeps the ring of unacknowledged packets — a day is 144 × 27 B — so no
flash is written for it. Asleep the unit draws ~5 µA in all: the U535, the nPM1300's 0,8 µA, the radio's
1,62 µA, the `BMP585`'s 0,55 µA; the probes' branches are off.

## 2 The wake, every ten minutes

1. The two load switches on; the `SHT45`s and the `BMP585` read — milliseconds.
2. The switches off.
3. The packet built and sent in the slot; the acknowledgement awaited; a setting applied if one came.
4. Back to Stop 3.

The tips of Imber, where fitted, wake it for microseconds and are counted.

## 3 The heater

**When the humidity has stood over ~90 % for hours** — the grass at night, fog — the `SHT45`'s polymer
takes up water and reads high, a reversible creep; Sensirion's single-shot scheme drives it out
(its creep application note): **0,1 s pulses of the "200 mW" command every 2 s for two minutes** — the
heater's 10 % duty at most (the datasheet, §4.9) — then two minutes to cool to the air. At 2,4 V the
"200 mW" command gives ~106 mW at ~44 mA, 73 mA at most, inside the load switch's 100 mA. **It runs right
after a regular read,** so the next read finds the sensor cooled; when the humidity says, by day or by
night, and once at dawn to dry the 5 cm probe of its dew. ~0,07 mAh an event.

## 4 The shelf

Built, charged, programmed: a command puts the nPM1300 into ship mode, 370 nA; the magnet on the
`SHPHLD` reed, or the panel plugged in, wakes it. Held over ten seconds the same reed power-cycles the
part — the hard reset. Stored part-charged.

## 5 The floor

At 2,5 V the cell is empty for this unit: the boot log written, the nPM1300 sent to ship mode, the
panel's `VBUS` the way back.

## 6 Its description

Its place — latitude, longitude, the ground's height, from the handset's GNSS at commissioning or
typed — and its sensors' heights, fixed by the construction, written once into the head's mirror beside
its profile.

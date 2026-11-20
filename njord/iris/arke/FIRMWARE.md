★ N.I.C. ★

# Arke — the firmware, what differs from Iris

> **Design-stage concept — nothing built.** The figures are verified against the parts' sheets and
> the parts before a board is made.

The firmware is Iris's (`../FIRMWARE.md`), the role Arke. What it does as Arke:

## 1 The wake, every ten minutes

1. The eFuse feeds the arm.
2. **A ModBus broadcast** — address 0, a write no slave answers, standard RTU — tells the house MODs
   to start their measurement; the bought sensors measure from power-up.
3. After the longest warm-up on the arm, from each sensor's sheet, Arke reads every slave as Palatine
   would, block by block.
4. The feed is cut — every sensor asleep without a second broadcast.
5. The BMS and the MPPT read on the input pair.
6. The blocks packed into packets of at most 51 B, sent in Arke's slot, acknowledged per part.

A pulse sensor — cups, Imber's tips — stays fed and is counted between the wakes.

## 2 The clock

**Heimdall's own discipline loop, `core/proto/nic_kronos.c`, on the GNSS's PPS** — a fix a day, the
32,768 kHz crystal between. Arke's records carry their own time, with Iris or without it; its position
is taken by itself at installation and written into its description.

## 3 The description

At commissioning Arke reports what hangs on its arm — each block's sensor and its profile — through
the tunnel into the head's mirror, beside its place and its sensors' heights.

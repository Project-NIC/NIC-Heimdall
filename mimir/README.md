★ N.I.C. ★

# Mimir (mini-Heimdall) — the head, the clock, two cards and Palatine on one board

> **Design-stage concept — nothing built.** The figures are verified against the parts' sheets
> and the parts before a board is made.

**Mimir is one PCB carrying six sections of the station — Mayak, Kronos, a Bifrost, an Argus,
Palatine, and Hermes — each laid out as its own document draws it, its pins unchanged
and its firmware unchanged.** What the board removes is what joined them in a station: the Bifrost's
tap on the time bus ribbon, the crossed in-box cables from the Mayak's trunk to the Bifrost and
from the Bifrost's ports to Palatine and the Argus, and four of the five pairs of 12 V terminals
with their fuses. Outward it has **two NodBus ports, four mini segments, four ModBus arms with
`PWR EXT`, one spare MasterNOD for a Bifrost of its own, Ethernet and USB**. It is the box of a
station that measures the weather, a few spurs and a few sondes, and grows by one more card.

Named for **Mímir**, whose well lies under the root of the tree and holds Heimdall's horn.

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

## Files

| file | contents |
|---|---|
| [`HARDWARE.md`](HARDWARE.md) | the six sections, what becomes a trace and what each trace preserves, what is fitted, Ethernet, the 12 V, placement, what it buys and costs |
| [`WHY.md`](WHY.md) | the graveyard |

## Licence

Hardware: CERN-OHL-S v2 (`../LICENSE-HW`) · Software: MIT (`../LICENSE`) — Copyright (c) 2026 NIC — Native Intellect Community

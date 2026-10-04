★ N.I.C. ★

# Proteus — the head, the clock, a Bifrost and Hermes on one small PCB

> **Design-stage concept — nothing built.** The figures are verified against the parts' sheets
> and the parts before a board is made.

**Proteus is the smallest PCB that is a whole station head: Mayak, Kronos, a Bifrost and Hermes on
one printed board, Polaris plugged into it, with one copper NodBus run on the board itself and
nothing else.** No data body, no spare port, no ribbon: the four data pairs of the one run land on
terminals, the run's 12 V feed is the one plug-in board — the isolated 12 V cell on its power body,
so a surge burns a module and not the board — and the board is fed through an isolated DC/DC brick
on the PCB from a vehicle's 12 V or a site's pack. It fits a 1-DIN radio slot or a small box.
Each section is laid out as its own document draws it, pins and firmware unchanged — the station's
classic structure on one PCB. **The modem is an LTE-M modem**, and with the Mayak and Hermes it is
what the backup cell carries: when the 12 V goes, the last report still leaves.

Named for **Charles Proteus Steinmetz**'s middle name.

```
   vehicle 12 V / pack ──▶ [isolated DC/DC brick] ──▶ the 12 V node ── INA238 (LP I²C)
                                                        │
      ┌─────────────────────────────────────────────────┴─────────────────────────────────────────────────────────────────────┐
      │  KRONOS ◀ TIME IN 1 ◀ POLARIS ── CLK · PPS_K · label as traces ──▶ BIFROST                                            │
      │                                                  ▲        port 1 ──▶ 485 island on the board ──▶ 4 pairs on terminals │
      │  MAYAK ── one MasterNOD as traces ───────────────┘        PWR OUT 1 ──▶ the plug-in 12 V cell ──▶ feed pair           │
      │    ├── LP I²C ──▶ HERMES ── 485 · CAN · UART · I²C out                                                                │
      │    ├── the modem, LTE-M ── on the backup cell's rail with the Mayak and Hermes                                        │
      │    └── SD ×2 · Wi-Fi · BLE · USB-C · Ethernet                                                                         │
      └───────────────────────────────────────────────────────────────────────────────────────────────────────────────────────┘
```

## Files

| file | contents |
|---|---|
| [`HARDWARE.md`](HARDWARE.md) | the sections, what becomes a trace, the run on the board, the feed and the brick, the two bucks, what is fitted, the form |
| [`WHY.md`](WHY.md) | the graveyard |

## Licence

Hardware: CERN-OHL-S v2 (`../LICENSE-HW`) · Software: MIT (`../LICENSE`) — Copyright (c) 2026 NIC — Native Intellect Community

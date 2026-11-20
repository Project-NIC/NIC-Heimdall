★ N.I.C. ★

# Arke — the hardware, what differs from Iris

> **Design-stage concept — nothing built.** The figures are verified against the parts' sheets and
> the parts before a board is made.

The board is Iris's (`../HARDWARE.md`), the GNSS fitted. What is Arke's own:

## 1 The pack

**4S sodium-ion charged as LiFePO4** — 3,1–3,6 V a cell, **12,4–14,4 V**, a LiFePO4 4S pack's own window,
so a bought LiFePO4 4S BMS and an MPPT's LiFePO4 profile take it unchanged, their limits (~3,65 V up,
~2,5 V down) inside sodium's 1,5–4,0 V. **It charges in frost without heating**, which is why it is
sodium. The floor may go as low as the highest of: the arm's sensors' minimum supply, the BMS's
under-voltage stop, the MPPT's low-voltage cut. **Not isolated** — no wire joins Arke to the station.

**What the cell, the BMS and the MPPT must meet** — the builder's pick at the build, the market moving
faster than the design:

| | |
|---|---|
| cell | charges from −20 °C under 3,6 V a cell; self-discharge ≤ 3 % a month; cycles stated to 80 % with their depth and temperature — from its maker's sheet |
| BMS | LiFePO4 4S limits; its own standby draw stated and small; its cold-charge limit a setting; RS-485 ModBus |
| MPPT | a LiFePO4 profile; **common negative**; its own draw stated and small; RS-485 ModBus |

Both on Arke's input pair as ModBus slaves, read at every wake as Hermes reads the station's.

## 2 Its day

`../../calc/power.py`, December near Prague taken as ~1 peak-sun hour on a steep panel:

| load | a day | panel |
|---|---|---|
| sensors switched, a quiet MPPT and BMS | ~4 Wh | 20 W |
| sensors switched, a smart BMS at 20 mA | ~10 Wh | 20 W |
| sensors always on | 17–35 Wh | 50 W |

**The node is 27 mWh of it, the radio 105 mWh — the MPPT's and the BMS's own draw size the panel and
the pack.** Seven dark days want 2–19 Ah at 13 V.

## 3 The arm

The bought RS-485 sensors of Palatine's list (`../../../palatine/SENSORS.md`) and the house MODs —
Ceres at a soil depth, Sakura in the canopy — on the switched 12 V; each sensor's supply range checked
against the pack's 14,4 V; their ModBus addresses unique across the station.

## 4 The GNSS

A multi-constellation module of the cheapest class (ZKW `ATGM336H`, Quectel `L76K`), its antenna on the
mast, on `USART3` with its PPS on `TIM2_CH1` (`../HARDWARE.md` §2): a fix once a day, ~30 s at tens of
mA; between fixes the 32,768 kHz crystal holds.

## 5 The mast

The antenna and the GNSS antenna at the top, the panel steep so the snow slides, the box under its
roof, one earthing point at the foot for the panel's frame and the barriers.

★ N.I.C. ★

# Ratatoskr — why

## The H523, turned off by the nPM1300's Hibernate timer

The nPM1300's Hibernate (500 nA, its own wake-up timer) would power the H523 off between wakes and keep
one MCU in the station. A cold start every ten minutes loses the RAM, and kept asleep instead the
H523's Stop draws 150–270 µA — 3,6–6,5 mAh a day, five to ten times the radio. The U535's Stop 3 keeps
all its RAM at 2,2 µA.

## LTO 2S and its chargers

Lithium titanate 2S, 3,6–5,6 V: the nPM1300 charges one cell; TI's `BQ25798` reaches it, but its
power-up default for two cells is Li-ion's 8,4 V, fatal to LTO; TI's `BQ25570` holds a set voltage by
resistors but takes 5,1 V of panel and 100 mA. No 2S LTO pack with a BMS was found. Left for one
sodium-ion cell on the nPM1300.

## A 1,8 V rail

The SX1262 loses 6 dB at 1,8 V, and the Wio-SX1262 has one `VCC` for its supply and its logic. Left
for both bucks at 2,4 V.

## An MPPT part for the panel

ADI's `LTC3130` (2,4–25 V, MPPC) was proposed; the house `LMR43610` with its `EN` on the panel does the
same at this scale.

## The probes on RS-485

Bought probes on one 485 pair, Maxim's `MAX3471` running from 2,5 V at 1,6 µA: the probes themselves
want 5–30 V, a boost from one cell. Left: Ratatoskr stays on I²C.

## A 485 or a differential I²C on the leads, and isolation

A metre of lead is I²C's own; a differential extender or an isolator would want parts and milliamps —
an isolator an isolated supply on the probe's side — and the ESD at the probe's end still lands on the
probe. The station's ESD rule at both ends.

## A probe's own rail from the bucks

The leads fed from `BUCK1` at 2,4 V: a struck lead would reach the MCU's rail, and its protection
could stand no more than 3,6 V. Left for `VSYS` on the leads and an LDO on each probe. The `TPS7A2024`
there wanted a 10 Ω behind the TVS, which the heater's current would drop 0,3–0,6 V across; TI's
`TLV704` is made at no 2,4 V and asks 2,5 V in; Microchip's `MCP1703A` asks 2,7 V. The `LP2985-N` kept.

## The barometer only at the station; a GNSS on every unit

One sensor set on every unit, each a check on the others; the position from the handset at
commissioning, not a module that runs five minutes in the unit's life.

## The antenna inside the box

Sealed and safe from branches, but a few dB below a whip outside and a second build for the far band.

## A service connector

A sealed USB-C costs ~$13–24 a piece; out of proportion to the unit, one more way in for water.

## Potting the unit

No air to breathe or to mist — but a potted unit does not open for its firmware nor show a cell that
swells. Left for the vent and the coating.

## Sakura and the soil on Ratatoskr

A connoisseur's own; on an Arke.

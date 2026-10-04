★ N.I.C. ★

# Argus — the graveyard

Rejected alternatives and superseded states, with the reason. Nothing returns from here and
nothing is deleted from it. The board's graveyard is `../WHY.md`.

## The freeze broadcast

Sixteen ModBus arms through a gate tree; a custom function code made a group latch at one instant
and answer on timers keyed to unit number: **ModBus has no clock, so simultaneity was
manufactured**. Mini units sample on the wire clock in their own slots, and the frame index names
the instant. The freeze, gate tree, four baud/parity domains and per-group `DE` went. Do not
re-propose it for a clocked segment.

## The aggregation formats that lost — one stream, pass-through, the tagged block

① **One-address summed stream** (four Gauss at 210 Hz = 158 frames/s against the 256/s index
ceiling; eight = 315) — died on that ceiling and RESEND ambiguity. ② **Untagged pass-through**, one
mini per 32 B slot — wasted three quarters of it. ③ **Tagged mini block**, 3 × (1 B address + 1 B
age + 8 B payload) + present-mask — table-free decode, but **died on arithmetic**: four 8 B frames
tile 32 B, a header makes it three, doubling Argus's slot claim at matched rates. Enrollment-fixed
tiling won: the table decodes, an absent sonde rides zeros, the assembly ring keeps each frame in
its period, so no age. A spare-byte identity check went too — identity is the table's job, never a
frame's.

## Sixteen minis behind one Argus — dropped

Four sondes × four segments in 4 upstream slots, Sputnik-style, breaks **the project's one ceiling,
eight units per card**, which the assembly ring and reply bookkeeping are sized for. Argus is at
most two units, eight minis.

## The per-module ceiling on a remote Argus — gone

~102 mA on 48 V, ~135 mA on 300 V, from the withdrawn 8,0 W figure (`../../galvani/WHY.md`). **No
derivation survives**; every optical part fitted reads 100 mA, and the sizing stands on that and
the 300 V table.

## The terminator plug, the down ports' switched buck, the duplex strap and the segment echo

- **A plug in the last board's free `OUT`** — gone with IN/OUT chaining; an 80,6 Ω jumper per pair
  at each end of a run.
- **The `LMR43620` with its `EN` on a GPIO, the down ports cold in sleep** — no rail has `EN` on a
  pin; sleeping ports go dark by `ENABLE`/`LINE_EN` and clock gate.
- **A duplex strap on a host pin** — every NodBus run is full duplex.
- **The segment echo check** — Argus is master there, and a master never echo-checks; `RXD_ECHO`
  is unconnected on a down port.
- **One I²C "for the expanders"** — there are none; three controllers carry five `INA238`s.

## A mini segment on the LPUART

Saving one USART was not worth verifying its sampling — kernel clock at three times the baud, no
oversample — against the wire rate. Every segment is a full USART.

## Argus and Babel — why they are not one board

Babel makes a bare sensor chip into ModBus slaves for Palatine; Argus carries mini-NODs. **A Babel
speaks ModBus, a segment mini**: two boards, at the station's two ends.

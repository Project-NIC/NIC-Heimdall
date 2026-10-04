★ N.I.C. ★

# Kronos — the graveyard

Rejected alternatives and superseded states, with the reason. Nothing returns from here and
nothing is deleted from it.

## Polaris as a top-level project — moved under Kronos

Polaris stood at the top of the tree as if it were a unit; it is a bought receiver and antenna with
no processor, bus or address, fitted only where there is no Sputnik, and **the only board it serves
is Kronos** — so it is `kronos/polaris/`.

## Advancing the PPS at a remoted receiver — dropped

Proposed: send the ranged route to the far end as `$PNIC,ROUTE` and let it place its PPS that much
early, so Kronos's `corr` would be `CAB` + `INT` alone. **It cannot be done at a receiver and buys
nothing where it can.** An early edge needs a local timebase, and a Sputnik has none — its
`OSC_IN` is the station's 2²², which Kronos steers from the very pulses the Sputnik relays. Pip could, but a mechanism that works on one socket of two is worse than
one that works on neither. A spur unit advances its grid because it is a sampler; a remoted
receiver is one edge a second, corrected once, so the advance buys symmetry only. The UM980's PPS
adjusts in width and polarity only. The correction stays on Kronos; `FIRMWARE.md` §7 reports
`route` read-only beside `corr`.

## Five time sockets and two LVC buffers — superseded by one M-LVDS time bus

Kronos had five 6-pin time sockets, one per card and one for the Mayak, driving `CLK` and `PPS_K`
as CMOS from two LVC buffers — a star of five cables. One 10-pin ribbon along the card row with an
IDC tap per card replaced it: **one cable instead of five matched ones**, a differential clock
that radiates a fraction as much, and no length matching — a card's place on the
ribbon is nanoseconds, under the 7,45 ns capture grid. The cost is two receivers per card and
~0,25 W across the station.

**The parts.** `SN65MLVD200A`: ~7 $ a piece and not stocked. `DS91C176` at every end left the taps
at 14 mA receiving and 6 mA disabled, against the pin-compatible `THVD1450`'s 0,70 mA and 0,1 µA;
the `THVD1450` takes every tap, `DS91C176` stays the driver. Plain LVDS: a one-volt common-mode
window and no multipoint standard.

## An NTC against the TCXO's can instead of the digital thermometer — not taken

The holdover table is written and read by one sensor, so a constant offset cancels; **only repeatability and coupling matter**, and on coupling a bead on the
can wins on paper. But coupling is a layout job: the digital part sits as close to the can as the
outlines allow, on the same pour, under the same cover (`HARDWARE.md`), and the can-to-crystal
gradient neither sensor sees — the loop learns a bin only while the temperature is quiet. The NTC
would have cost an ADC channel, a switched divider, a dependence on `VREF+`, a drifting bead and a
second thermometer class, for thousandths of a ppm at a residual slope of ~0,05 ppm/°C.

## The clock as a corner of the Mayak or of a Bifrost — rejected

A bridging firmware bug or a timer contention on a shared processor would reach the discipline
loop; **a board whose firmware is the loop, the capture and the label stays small enough to
verify**, and one part number across the station makes an H523 cheap. It is also the one place the
delay table is summed, so it is subtracted exactly once.

## A clean 3,3 V on a `TPS7A2033` off the Mayak's buck — superseded

A dependency the clock should not have — the head supplies neither the cards nor the clock — and
3,3 V into a 3,3 V LDO has no headroom. The board makes its 3,3 V from the 12 V wire on the house buck. **Supply jitter on a ±1 ppm TCXO is picoseconds against ±1 µs**; a ferrite and local
capacitors at the TCXO do the filtering.

## A ±0,5 ppm TCXO, a clipped-sine part, a plain crystal — not taken

The ±0,5 ppm grade halves a holdover already comfortable — 86 ms a day at ±1 ppm — and improves
nothing else. A clipped-sine output needs a shaper before `OSC_IN`; a CMOS part drives it directly
(`NT0503D` was the other such candidate). A plain crystal under the same discipline is cheaper and
has no holdover worth the name.

## Socket feeds not gated, and trip points by board class — superseded

Power bodies were read, never switched, `ENABLE` left to the far end, each `INA238` set to
the fitted board's class. A host's `LINE_EN` and `ENABLE` are one GPIO per port,
the port comes up off and is switched on once wired, and **the trip point is the far end's load
measured at commissioning, not a class**.

## A power body on each receiver socket — deleted

Each RX/TX + PPS socket carried a power body, an `INA238` on I2C2 and `TRIP`/`PWR` registers, for a
receiver at the end of a fed run. **None stands there**: Polaris is in the box on the data body's
3,3 V, and a remote Sputnik or Pip is fed by its own NodBus run, so the clock run is data only. The
two bodies, I2C2, the `ALERT` pins and two `ID` inputs went.

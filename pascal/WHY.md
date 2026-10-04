★ N.I.C. ★

# Pascal — the graveyard

Rejected alternatives and superseded states, with the reason. Nothing returns from here and
nothing is deleted from it.

## `LINE_EN` on PE3 — superseded

A GPIO pulled to run, kept for the bench. **At a unit end nothing switches the Galvani boards**:
`LINE_EN` is 10 kΩ to 3,3 V in the socket and PE3 is free.

## A magnetometer riding along — not fitted

Mechanically free — same board, same oil — but **at 200 m near an island it is not a tsunami
sensor**: a wave's magnetic signal scales with the whole water column and wants the slope toe,
kilometres down. The magnetometer is Gauss.

## The gauge in the oil, reading the water through the wall — superseded

The fill expands with temperature more than the wall: **a pressure change in the hours band the
gauge cannot tell from the water**. A threaded sensor body through the tube now puts the gauge in
the water; the fill only protects the electronics.

## One to eight gauges, a fan on the source sector — superseded

The count ran 1 (detection) · 2 · 3 (bearing) · 4–5 (the fan on the source-facing sector) · 8
(the ring, for two arcs). Now a ring of 4, 8 or 12 round the island, Gauss's rule too; **twelve is
the ceiling because every sonde is a laid cable**.

## `0x0013 MUX` — gone

Selected which supply byte streamed; it went with the streamed byte (`../core/WHY.md`), and
`0x0013` is free.

## The PLL reference — 16, 4 and 2 MHz refused; `HSITRIM` set once — superseded

Gauss's loop and history: the refused references with their pull ranges, and the trim that now
steps whenever `FRACN` nears its window's edge, are in `../gauss/WHY.md`.

## The pressure arithmetic on a crystal's lid — out

"~2 GPa at 30 bar" for a 0,1 mm lid over a 2,5 mm span **did not scale with the pressure**; the
rule stands without it — a crystal is a gas cavity and this body has no gas.

## A stainless threaded body with a stainless membrane — superseded

A stainless-membrane depth sensor in a stainless threaded body around an `MS5837-30BA`-class gauge,
"sourced by the builder". **No catalogue part is that**: the `MS5837` has a gel face, and the
stainless-membrane I²C transducers (Keller's LD series) are another chip and protocol at hundreds.
Now the `MS5837-30BA` in the Blue Robotics `Bar30` housing, gel face in the water: the measurement
is a minutes-band change the face does not touch; stainless was a question of life in the sea, and
the housing unscrews.

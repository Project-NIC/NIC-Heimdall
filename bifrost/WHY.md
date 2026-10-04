★ N.I.C. ★

# Bifrost — the graveyard

Rejected alternatives and superseded states, with the reason. Nothing returns from here and
nothing is deleted from it. Argus's own entries are `argus/WHY.md`.

## The eight-port card — dropped

Eight ports is eight data and eight power bodies with their Galvani boards: **the card stops being
a card**. Eight need a large package and a lower core supply; four fit the hundred-pin H523, with
double the memory per port and simpler bookkeeping, at a clock tree per card. Glass is
point-to-point: eight optical sondes are two Arguses.

## The clock in and the clock out — superseded twice

**A rung per port on four timers**, from fast-and-slow-segment cards: no mini segment wants over
2¹⁹, and a timer's channels share one period. **Then Kronos's 2²⁴, regenerated** — 2²² on a
Bifrost, 2¹⁹ on an Argus, `ATTN` naming ×8 or ×32; an in-box Argus on 2²⁴ failed: `ATTN` names two
numbers, not three. Now the ribbon carries 2²² through `MCO1` and a quad buffer, prescaler 1 or 8;
one PLL ×32 locks to 2²⁷.

## The port clock's gates on the ports' `EN_n` — superseded

Each gate's `OE` on `EN_n` with `ENABLE` and `LINE_EN`, "no new pin": **a clock could not stop
without its feed**, so `PORT_CLK`, clock-off probing and the clock-first off were unreachable.
Each `OE` has its own GPIO.

## The 2-input OR at `OSC_IN` — superseded by two 3-state buffers

A pulled-down OR, assuming the unused source sits low; **a 485 receiver fails high** and would
have stuck it on every Argus, whose tap is empty. Two buffers enabled oppositely from `ATTN`
select instead. An analogue switch passes the edge and adds ~17 pF; a buffer redrives.

## Ranging — what lost to the timed turnaround

- **Overclocking the core.** No kernel multiplexer on the timers, so every unit's timebase moves;
  32 launches at 7,45 ns already average ~1,3 ns.
- **Copying the pulse.** It copies the shortening; a pulse of its own, with the received width
  reported, separates the two directions.
- **A data packet instead of a pulse.** A UART edge is a sampling window within a 477 ns bit at
  2²¹; a capture is one tick.
- **A PTP-style exchange.** A round trip sums both directions; no timestamp splits the asymmetry.
- **Once at floor-up.** No 1×9 module specifies delay or drift over −40 to +85 °C: every
  `RANGE_INTERVAL`.

## A slot-advance register — not built

Advancing each unit's slot by its delay, to align every spur in one round, **would couple the
spurs' delay domains**. Each spur keeps its `BUSCFG`; the route is applied at the unit.

## The trunk on the LPUART — dropped

Verifying its sampling against 2 Mb/s on the Mayak's crystal was not worth one port. The trunk is
UART4.

## A card that relays — rejected

Four interleaved streams for the head, each its own timing domain and index counter. The card
re-slots and stamps, at the cost of knowing the absolute second.

## The RTC as the card's time — rejected

It **cannot be phase-slaved to the wire**. The card's time is a sub-second count on TIM2 and a
second word stepped on `PPS_K`.

## A ModBus master port on the card — refused

It would stop being a bridge and grow a profile per sensor. ModBus hangs off Palatine.

## The time bus tap on `DS91C176` — superseded

14 mA receiving, 6 mA disabled; the `THVD1450` fits the footprint at 0,70 mA, `RE#` low, its
failsafe-high never read.

## The up port's `LINE_EN` on a GPIO, `FAULT` on the bodies, the up port's power-body `ID` unread — superseded

No processor pin switches a Galvani board at a unit end: `LINE_EN` is held in the socket, freeing
PD0. No board drove `FAULT`. A power board is known by its resistor, `INA238` or not, so its `ID`
is read — ten channels.

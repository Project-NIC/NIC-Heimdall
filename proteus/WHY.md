★ N.I.C. ★

# Proteus — the graveyard

Rejected alternatives and superseded states, with the reason. Nothing returns from here and
nothing is deleted from it.

## One board with Mimir — rejected

Mimir (Palatine and an Argus) and Proteus (the head, the clock and a Bifrost) were put on one PCB
with sections unfitted. **Two populations differing by two processors are two boards**, and a board
half empty in one of its uses is drawn wrong for both.

## Pads for the multipoint pull-ups — not laid out

The station's Mayak keeps four unfitted pads for a second card on a trunk. Here the one trunk is a
trace to the Bifrost with nothing ever stacked on it: **a pad for an unusable resistor is not laid
out**.

## LoRa on the base — dropped

The board stands where a vehicle's LTE router or a site's Wi-Fi reaches; **the LoRa heartbeat adds
nothing**.

## Hermes dropped with it — reversed the same day

For an hour the board had no Hermes, since a vehicle has no BMS. **It is also the base of a solar
site with a pack**, so Hermes stays — a section on the PCB, not a card on a power body — idling at
milliwatts where nothing answers its faces.

## Two ports out, on data bodies — superseded by one run on the board

The board first carried the Bifrost's ports 1 and 2 as complete Galvani sockets. **It has one run,
and a run inside one box is a connector and four pairs** on terminals, so no data body is laid out.
The second port had nothing to carry.

## A buck per section — superseded by one buck

The separate boards keep a buck per section so a shorted port browns out only its branch. Here the
one port is an isolated island and the one plug-in carries its own `INA238`, so **the isolation
buys nothing**; one `LMR43620` carries the board.

## The isolated block as a bought box in the cabin — superseded by a brick on the PCB

A separate 20 W block in its own case, cabled to the board, was **one more box and one more cable
in a vehicle**. The same converter exists as a PCB-mount brick; a site's pack goes through it too
at a watt of loss, so there is one input path and nothing to bypass.

## Polaris as a section on the PCB — corrected

The NEO-M8N, its bias tee and its resistors were drawn onto the board. **Polaris is a bought module
on a carrier of ours**: it plugs into Kronos's `TIME IN 1` as in every station, and the PCB carries
the socket, not the receiver.

## One buck under the backup cell — corrected

The one `LMR43620` fed the whole board, and the Mayak's cell sat on that rail: on a lost 12 V it
would have tried to carry ~1,3 A against its eFuse's 1 A and dropped at once, the flush lost — and
the rail stood at 3,3 V, not the cell's 3,43 V float. **The Mayak keeps its own rail and its cell as
in every Heimdall**, the modem on it; the rest goes dark with the 12 V.

## The vehicle's LTE router as the uplink — superseded by a modem on the cell

A router on the Ethernet dies with the 12 V, so the report of the loss never left. **The modem is an
LTE-M modem in the Mayak's modem position**, fed from the rail the backup cell carries.

## The 20 W `TEN 20-2412WIR` — superseded by the 10 W `REC10K-2412SAW/H2`

Twenty watts were sized for a board heavier than this one. The sections, the modem and the run's
feed come to ~7 W running and ~11 W at the worst instant, which a 10 W brick's 150 % current limit
passes, at a quarter of the size and a lower cost; its `UVLO` holds down to ~7 V where the Traco
stopped at 9. The Traco stays the part for a run heavier than the cell's declared 5 W.

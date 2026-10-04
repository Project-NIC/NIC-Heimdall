★ N.I.C. ★

# Steinmetz — what differs from Tesla's build

> **Design-stage concept — nothing built.** The figures are verified against the parts' sheets
> and the parts before a board is made.

**The board is Tesla's and is not described here**: `../HARDWARE.md` is the board, to the last
part. This document is the one value that differs, the run, the feed, the vehicle's supply and
the antenna.

## The front, 20 dB down

**Tesla's attenuation terminals are populated: 221 Ω metal film, 1 %, across each `Rf2` leg —
pressed into the clamp at a static site, soldered into the clamp position on a vehicle.** `Rf2` 2 kΩ ∥ 221 Ω = 199 Ω, so S2's gain falls from ×2,02 to ×0,20 —
20 dB — and no filter corner moves; the board is Tesla's to the last part and the two resistors
are the field trim Tesla provides for. What it buys and costs, from `../HARDWARE.md` §0.4 with
the one factor:

| | Tesla | Steinmetz |
|---|---|---|
| clipping field | ≈ 690 nT | **≈ 6,9 µT** |
| floor, field-referred | 9,1 pT — 5,7 the chain, 7,1 the converter | **≈ 71 pT** — the converter's 7,1 pT ×10, the chain's 5,7 unchanged |
| a 400 kV flashover's restrike, 820 A, clips at | 225 m | **23 m** |
| a 110 kV restrike, 225 A, clips at | 62 m | **6,2 m** |
| a 30 kA return stroke clips at | 2,9–4,3 km | **290–430 m** |

A line's arcing — tens to a few hundred amperes re-striking under the vehicle at 20 m — now sits
inside the range instead of on the rail; a full flashover at 400 kV still clips at the roadside,
and its time survives the clip. The lightning horizon moves in with the floor: a 30 kA stroke is
still tens of dB above 71 pT at 1000 km, an ordinary 5–10 kA stroke reaches the floor at
350–700 km, which is the reach this unit needs and no more.

## The link to the roof

**Copper, two-pair 485 at both ends — the communication board in the unit's `NB IN`, the same
parts on the Proteus's own board — on one hybrid cable: four 100 Ω pairs and two 0,75 mm² cores in a PUR jacket,
shielded, drag-chain rated, −40…+80 °C, through one gland on each enclosure.** The pairs carry
data TX, the clock at 2²², ground and data RX, full duplex at the rung `BUSCFG` hands down — 2²⁰,
the one unit on its port — as on every NodBus run; the cores carry the 12 V. The shield lands on
the station enclosure's ground at the station end only; the roof end floats, as every unit end
does. In a vehicle the run is 3–5 m: 15–25 ns of cable against a 954 ns bit, on 100 Ω pairs the
boards terminate at ~100 Ω.

**At a static site the same two boards carry the run to 500 m**, on the ~100 Ω UTP Cat 6 data
cable the family names; past 50 m the feed is a source cell at the station and a unit power
board at the unit's `PWR IN`, by the station's own run rules (`../../galvani/README.md`).

**Glass never stands on Steinmetz.** The copper board is already isolated and its common-mode
window covers a vehicle; a fibre's weak points — the gland, the bend, the connector — are exactly
where a vehicle flexes.

## The feed to the roof

**The isolated 12 V board in the Proteus's one power socket**: the board's 12 V across a barrier,
0,4 A, ~5 W, its `INA238` reading the whole run. **The unit takes it on its own `12V`
terminals** and makes its 5,3 V, 1,8 V and 3,3 V from it as it does behind any feed; no power
board stands on the roof and no converter switches beside the rods. The load is the measuring
unit's ~3 W and the communication board's ~0,3 W, against the cell's 5 W.

## The vehicle supply

**The Proteus is fed from the vehicle through the isolated DC/DC brick on its own board** —
9–36 V in, 12 V out, 10 W, off below 7 V (`../../proteus/HARDWARE.md`) — and sees the brick's output
as the 12 V wire, the way a station sees its pack. The vehicle's body is the brick's input ground
and nothing of the station's. The brick rides through a cranking dip down to its ~7 V cut; below
it the Mayak's backup cell writes, reports and lets go, and the station boots when the engine runs.
A small LiFePO4 pack ahead of the brick is the build where the unit must not reboot at every start.

## The antenna

**Polaris sits on Kronos in the cabin; only the antenna is remote.** In a vehicle the bought
active antenna is glued to the windscreen inside the cabin, its coax to Polaris's bias tee; at a
site it stands on the mast on its insulating bracket, as `../../kronos/polaris/README.md` draws it.
Nothing of the GNSS is on the roof.

## The enclosure

**Tesla's** (`../CONSTRUCTION.md`): the frustum of three rods, the board behind its sockets, and
**one penetration — the hybrid cable on its gland**; no connector outside, no antenna.

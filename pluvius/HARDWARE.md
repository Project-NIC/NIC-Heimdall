★ N.I.C. ★

# Pluvius — hardware

> **Design-stage concept — nothing built.** The figures are verified against the parts' sheets
> and the parts before a board is made.

## What is on it

| part | job |
|---|---|
| **`STM32H523VE`**, LQFP100 | the ModBus slave; reads the bridge over SPI, answers in grams |
| **`ADS1235`** | the 24-bit weigh-scale converter — ratiometric reference, PGA 64, AC excitation sequenced by its own GPIO |
| **`TPS7A4701`** at 5,0 V behind an **`LMR43610`** at 5,5 V | the one LDO — the converter's `AVDD` and the bridge's excitation, one node |
| **`LMR43610`** at 3,3 V | the MOD's buck, 36 V in so it outlives the transil clamp: the MCU, the `THVD1450`, the converter's `DVDD` through the shielded 2,2 µH |
| **`THVD1450`**, 2× 10 Ω, `SM712`, 2× `5.0SMDJ18A` | the line front and the basic set of every MOD (`../core/blocks/modbus.md`) |
| **four MOSFETs — 2× `BSS84`, 2× `BSS138`** | the bridge that reverses the excitation between conversions — the converter's own four-wire circuit (*AC bridge excitation*) |
| `TMP117`/`STS35` on I2C1, an NTC on ADC1 | `TEMP1` beside the cell, `TEMP2` in the vessel — populations |
| 2× **`DGPS2.5R-5.0`**, four poles | the arm — `A` · `B` · `GND` · `12 V`, the board's only terminal |
| the load cell, soldered — `CELL` | the six-wire cell tail on the board's own pads |
| two 2,54 mm headers — `PUMP`, 5 pins, and the vessel's NTC, 2 | `3,3 V · GND · PULSE · RUN · DIR` to the head's tail board, whose three couplers sit at the head and not here (*The head's tail*), or the Hall switch on three of the five; the NTC's lead |

**The head is not on this board.** Its 24 V is the switched supply on Palatine's `PWR EXT` body; this board asks for the drain in a status bit Palatine
reads anyway (*The switch*). No Galvani body, no 12 V terminal, no pump silicon here.

## The load cell and the signal chain

| | |
|---|---|
| the cell | **30 kg class, single point, suspended** — the vessel hangs from it. **`C4` is the floor, `C5` the working choice, `C6` where it can be had.** The builder sources it to this: 30 kg, single point, six wires, 350 Ω, 2 mV/V, survival to −40 °C |
| the catch | **200 cm² ≈ ⌀160 mm**, so **1 mm of rain = 20 g** |
| span | ~900 mm of rain standing in an 18 L vessel — 21 kg with the tare, 70 % of the cell; the counting drain means it never gets near. **Both are a reference build — *Sizing*, below** |
| temperature | accuracy above freezing, survival to −40 °C — snow is the radar's job |
| the converter | **ADS1235**, 24-bit ΔΣ for weigh scales: ratiometric reference, PGA 64×, SPI |
| the thermometers | populations, `TEMP1`/`TEMP2`: a **`TMP117`/`STS35`** on the board beside the cell, on **I2C1** (PB8/PB9) — the span's temperature covariate; an **NTC** in the vessel on a lead, 10 kΩ against 10 kΩ switched from PE3, on **ADC1** (PC0), ratiometric — the water's. Neither corrects anything on the node: the zero is tared at every drain and the span drift is the archive's to see |

**The cell is worked at a few percent of its span, on purpose.** A drain hands the firmware a
fresh zero, so the temperature drift of zero — the one error specified against the cell's full
scale and independent of what is on it — never enters the reading. What remains — non-linearity
and hysteresis over the arc used, creep under the standing load, the span's temperature drift —
scales with the *load*, not the capacity, so a lightly loaded cell is the better one here. What
light loading costs is signal-to-noise, and 2 g on 30 kg is 1/15 000, nothing for an `ADS1235` at
PGA 64. Nothing here reads a displacement, which is why the vessel may hang. **The one place 70 %
appears is the ceiling**: the full vessel may reach 70 % of the cell, so an overflow does not run
into the mechanical stop.

### Sizing — the vessel, the cell and what the reserve costs

**The reserve is set by how long a dead pump waits for somebody**, and everything else follows
from that. `A` is the catch in cm², `H` the wettest rainfall total over the response window in
mm, `m_tare` the vessel, lid, suspension and hose stub in kg, and `n` the class number of a `Cn`
cell.

```
  m₁mm  [g]  = A × 0,1                         200 cm² → 20 g per millimetre
  V     [L]  = H × A / 10 000                  the water the buffer must hold
  E_max [kg] ≥ (V + m_tare) / 0,7              the full vessel at 70 % of the cell
  v     [g]  = E_max / n                       the verification interval — capacity in kg over the class number
  v     [mm] = v / m₁mm
```

**Substituting shows what actually drives the answer:**

```
  v [mm] ≈ H / (700·n)  +  m_tare / (0,07·n·A)
```

**The catch area cancels out of the first term.** A bigger funnel raises the mass per millimetre
and the mass of the reserve in the same proportion, so **it buys no resolution at all** — its one
effect is to dilute the tare, which is the second term. **So a bigger funnel is worth building
only where the vessel's own mass dominates, which is the short-reserve end.** At 200 cm² with a
3 kg tare the tare term is 0,04 mm at `C5`: negligible against a 900 mm reserve, and the whole
error budget against a 100 mm one.

| reserve `H` | vessel | + tare | cell (catalogue) | `v` at `C4` | `C5` | `C6` |
|---|---|---|---|---|---|---|
| **100 mm** | 2 L | ~3 kg | **5 kg** | 1,25 g · 0,06 mm | 1,0 g · 0,05 mm | 0,83 g · 0,04 mm |
| **250 mm** | 5 L | ~6,5 kg | **10 kg** | 2,5 g · 0,12 mm | 2,0 g · 0,10 mm | 1,67 g · 0,08 mm |
| **900 mm** | 18 L | ~21 kg | **30 kg** | 7,5 g · 0,37 mm | 6,0 g · 0,30 mm | 5,0 g · 0,25 mm |

**The 30 kg row is the reference build and it is the expensive one.** It is chosen so one
mechanical design covers a wet tropical month and a temperate year, not because it measures well.
A site with road access should be built on the 5 kg row and will resolve five times finer.

**`v` is a bound on accuracy, not the noise floor and not the reported step.** `WEIGHT` reports
1 g and the converter resolves far below that; what `v` bounds is how far the reading may be from
the truth. Because the zero is refreshed every drain, the practical error inside one fill is
better than `v` — **so the table is a ceiling to design against, never a figure to publish.**

**The rate limit is the head's, not the vessel's.** The drain runs while it rains, so 600 ml/min
over 200 cm² is **1800 mm/h**, the 900's 800 ml/min 2400 mm/h, and no rain reaches either; a monsoon
site is the same 18 L vessel draining more often. **What the reserve buys is the time a dead head
waits for somebody**, and a site that wants more of it takes a bigger vessel at the price the table
shows — a 100 L vessel is 5000 mm of reserve on a 150 kg cell whose divisions are ~1,25 mm.

## The pump

**The drain is a 24 V peristaltic head on the switched 24 V of Palatine's `PWR EXT` body, on a
2-core cable of its own.** A peristaltic head pays its occlusion friction before it delivers
anything — the knee is at 10–15 W — and the arm's 0,4 A cannot reach it (`WHY.md`, *The arm-fed
head*). **The vessel is a buffer, not a working volume**, so the drain has two duties: in service
it takes out about a litre — 50 mm of rain, 3 % of the cell's full scale — and after a head has
died, up to 18 L. The request, the barrier, the failure direction and the pickup are the same
whatever turns the rotor; what the head decides is the watts and therefore the cable that carries
them.

### The pickup — the counted drain, fitted or not

**A head with a pulse output counts on its own wire** — the `KPHM600`'s yellow `FG` — through a coupler on the tail board
(*The head's tail*). **A head without one counts with the house build: a thin neodymium disc on the
roller cross and a Hall switch outside the cover**, because
plastic is transparent to a magnetic field. **Littelfuse 55100-3H**: flange-mount,
25,5 × 11 × 3 mm, screw *or adhesive*, cable already on it, IP67, −40…+100 °C, 2,7–24 V, sinking
output into a capture pin, 1–2 mA, 20 billion operations.

**Take 3H — 55 G, the most sensitive grade**, so there is the most room to be wrong about the
magnet. **Unipolar, never a latch:** a latch needs alternating N and S and sits in one state
forever on a single magnet.

**Its published 19 mm is quoted with Littelfuse's own 21 × 7 × 4,7 mm magnet.** Field scales with
volume and falls as `1/z³`, so `z ≈ 19 mm × (V/V_ref)^⅓` — a **⌀5 × 1 mm disc lands near 6 mm**,
comfortable for a cover plus an air gap.

**Placement need not be neat:** a counter cares that there is exactly one pulse per revolution,
not where in the turn it falls. Direction is not sensed: a head with two wires runs forward only and is
self-locking. The clearance between cross and cover decides whether the sensor
sits axial or radial, and changes nothing else.

**What the pickup detects is that the motor turned — not that water moved**, and the second is the
fault worth catching. A slipped tube, a split tube, a blocked outlet and a sucked-in air lock all
leave the head turning happily and the count climbing. **The weight sees all of them and the
count sees none**, so the pickup is not the stall detector; *Detecting a dead drain*, below, is.

**The pickup's job is the volume coefficient, and the cell calibrates it.** Every drain divides
the weight that left by the revolutions that turned, so **ml per revolution is re-measured on
every cycle against the load cell** — the tube's set does not have to be predicted, only
tracked. A slow drift is the tube ageing; a step is a slipped tube; a collapse toward zero is air.
The coefficient is only taken from cycles that ran dry — with rain falling, one equation
carries two unknowns.

**The coefficient has three sources, and each is written to `CAL`.** The tube's own figure — the
maker's or a table's millilitres per revolution for the bore fitted — is the starting value. A
measured one replaces it at commissioning: a set number of revolutions, 500 say, run into a
measuring cylinder, or a known volume poured into the vessel from one, a litre or ten, and counted
out by the drain. From then on every dry cycle re-measures it against the cell (`FIRMWARE.md` §5),
and the bench measurement is repeated as the tube ages, whenever the running value has walked.

**One pulse a revolution is enough.** At 800 ml/min and 300 rpm the `KPHM900` displaces **2,67 ml a
revolution**, so one pulse a turn is 0,27 % of a one-litre drain — an order finer than the tube's
set, which is what limits the number (the `KPHM600`'s manual gives no rpm and so no ml per
revolution; the coefficient is measured anyway).

**One capture pin, two sources.** The `KPHM600`'s yellow wire is referenced to the head's black —
the isolated switched 24 V — and Pluvius stands on the isolated 12 V of the arm, two islands that
stay two, so the pulse comes onto this board **through an optocoupler** on the tail board at the
head (*The head's tail*); 5 Hz at 300 rpm. The Hall switch runs off Pluvius's own 3,3 V and sinks
into the same capture. Either way **the sensor is Pluvius's, not the pump supply's**, and fitting
it changes nothing about the barriers.

### Detecting a dead drain — three signals, read by two boards

**A drain is watched on three things at once, and the fault is their combination.** Each one
alone is blind to something:

| signal | who reads it | blind to |
|---|---|---|
| **the weight falling** — the vessel loses what the head takes, over a window of 5 s | this board, every second | nothing that matters: whether the water left is the question itself |
| **the revolutions** — the pulse output, where the head has one | this board, `PUMP_REV` | a split tube, a slipped tube, a blocked outlet, sucked-in air: the head turns, the count climbs, no water moves |
| **the current** — the source board's `INA238` on the switched 24 V | Palatine, on `ALERT` (`../palatine/FIRMWARE.md` §6) | the same: a head pumping air draws less than one pumping water, not nothing |

| what happened | the weight | the revolutions | the current |
|---|---|---|---|
| a jammed head | flat | zero | **over** the limit — Palatine drops the 24 V |
| a split tube, a slipped tube, air in the line | flat | turning, often faster | **under** the running minimum |
| a dry head, a cut cable | flat | zero, or none counted | under |
| the head running and the vessel emptying | falling | turning | inside the window |

**The weight is the one that says whether the drain is doing its job, so it ends the drain on
this board whatever the other two say**: with `HEAD` at 1 and `RUN` driven, the 3 s median must
fall by at least `DEAD_G` over every 5 s, or the drain is dead — `PUMP` cleared, `DRAIN_TIMEOUT`
set, `FAULT`. The other two name the cause: `PUMP_REV` zero against a weight that did not fall is
a head that did not turn, `PUMP_REV` climbing against it is a tube or an air lock, and Palatine's
`INA238` says the same from the other end and cuts the 24 V on its own thresholds. The head is
stopped by whichever sees it first.

**`DEAD_G` is set to the head and the cell, never written as one number.** The rule: the fall the
window must show stands clear above the noise of the cell and its converter — never a figure the
chain cannot tell from nothing. The `KPHM600` removes ~10 g/s and the `KPHM900` ~13 g/s, so a 5 s
window sees 50–65 g; the heaviest rain the catch can see, 100 mm/h, adds back 0,55 g/s, 3 g in
the window; the cell's noise is under a gram. **20 g is the default** — a fifth of what the 600
removes, twenty times the noise, and a slower head that manages 100–200 ml/min still clears it.
The register is `DEAD_G` in the calibration block (`MODBUS.md`).

**So the drain has one pump and three fits of the pickup, differing only in what they can
diagnose.** They are not builds — the head, the feed, the board and the register set are the same
in all three.

| fit | fitted | catches a dead drain | names the cause | measures ml/rev |
|---|---|---|---|---|
| **plain** | nothing beyond the pump | **yes** — the weight, 5 s | the current at the station only | no; the drain is weight-only and `DRAINED`/`PUMP_REV` read zero |
| **the head's own pulse output** — the `KPHM600` with the five-wire motor | the tail board's coupler, nothing glued | yes, the same | **yes** — turned or not | **yes**, re-calibrated against the cell every cycle |
| **the glued magnet** — `KPHM900-HB` | magnet + Hall switch | yes, the same | yes | yes, the same way |

**The register set carries all three fits without change** — `STATUS` bit 3 `DRAIN_TIMEOUT` is the
flag either way, and `THRESH`, `EMPTY`, `DEAD_G`, `RAW` and `DRAINED` are already there
([`MODBUS.md`](MODBUS.md)). **The firmware chooses; the board does not.**

**The drain runs between two levels, both settings, in runs of a minute.** `EMPTY` is the base
level a cycle runs down to — water left standing so the tubes stay submerged and the head never
sucks air; about 100 ml is the kind of figure, and it may evaporate for days before the next
cycle re-takes the zero below it. `THRESH` is the fill that starts a drain; the cell wants it
low, so the vessel is emptied often and the cell carries little — a litre above `EMPTY`, 50 mm of
rain, is the kind of figure. **A run is a minute at most**: after a minute the weight decides —
at or below `EMPTY`, the cycle ends and the zero is taken; still above it and fallen, another
minute; not fallen, the fault above. So a vessel fuller than one minute drains — after a dead
head, after a pre-drain written from the server — is emptied in runs of a minute, and the head is
never told a duration; the 10 minute ceiling on a cycle here and Palatine's 12 behind it catch a
stuck bit and nothing else (*The switch*). **A stone in the vessel is the case the two levels and
the fault exist for**: the weight stands above `THRESH` for good, the head pumps the water down
to the real level and then air, the weight stops falling and the fault ends the drain; `FAULT`
holds until `DRAIN` is rewritten, so the head is not run dry again and again, and the fault is
reported. Both levels are read against the tared zero, so the absolute term never enters the
reading.

### The head — 24 V; a size and a motor

**The head is a Kamoer `KPHM` peristaltic in its 24 V form and it hangs on the switched 24 V at the
station, not on this board.** Its two supply wires run from the board's terminal to the head at the gauge on
a 2-core cable of their own, metres long beside the arm's — a motor on an isolated 24 V, nothing on this board
touching it. 24 V for the cable: half the current of the 12 V variant, a quarter of the loss. **A
head is a size and a motor, chosen separately.**

**The size** sets the drain time and nothing else — the register set, the switch and the dead-drain
rule are the same on both:

| | flow | power | 18 L · a litre | tubing |
|---|---|---|---|---|
| **`KPHM600`** — **the build** | **≥ 600 ml/min** brushless, ≥ 540 brushed at 24 V (its manual, CPBZ-KPHM600-01) | **12 W, 0,4 A at 24 V** | **~30 min** · ~100 s | `B17`, BPT, ID 6,4 × OD 9,6 mm, 1000 h |
| **`KPHM900`** — the option | **700–900 ml/min** at 300 rpm, three rollers | **22 W** — 1,5 A on the 12 V line, **~0,9 A at 24 V by the watts**; the maker publishes no 24 V current line | **~23 min** · ~70 s | `B24`, BPT, ID 6,4 × OD 11,4 mm, 1000 h |

**The motor** decides whether the head brings its own pulse and how it ages. Every other section of this project distinguishes only that — **a head with a pulse output and control wires, or a head with two wires** — and the motor is how a given head comes to have them. **The 600 is sold with
either motor; the 900 only brushed** (or as a stepper, `-ST`, with a `KMD-542` driver
— not a fit: a driver board at the station for a head that runs minutes a year):

| | **brushed DC** | **brushless, five wires** |
|---|---|---|
| wires | two — the supply | 24 AWG, PTFE: red `Vcc` · black `GND` · **white `SP`, speed — 0–0,5 V still, 0,6–5 V the range, 5 V full; or PWM 10–30 kHz, 11–100 %; the manual runs full speed with the white on the red** · **green `F/R` — open is REVERSE, tied to the black is forward** · **yellow `FG`, open collector, one pulse a revolution, rpm = pulses/s × 60**; the white and the green are TTL inputs |
| the pulse | none from the head — the glued magnet and the `55100-3H` where a count is wanted (*The pickup*) | **its own**, on the yellow wire, through a coupler on the head's tail board (*The head's tail*). The output is an open collector that pulls to the black once a revolution; whether the revolution is the motor's or the roller's — the reduction is 1:8 — the coefficient absorbs |
| life | the makers' documents differ — from 700 h to several thousand on the brushed motor, 2000 h on the brushless; at minutes a year of running none of them binds (*The head's tail*) | the same |
| size | 72 × 62,7 mm face, 111,6 mm long | 72 × 62,7 mm face, **106,6 mm long**, Ø32 motor; both mount on 4× Ø4 on a 50 mm square through a Ø56 opening, or on the maker's L-plate |
| the choice | **the 900, the option** — two wires, no board; a count only from the glued magnet, and its hose left full | **the 600 with the five-wire motor — the build**: the count for the price of the head, and the head under Pluvius's own hand — run, stop, a few seconds of reverse run (*The head's tail*) |
| working temperature | **0–40 °C** by the maker — *The vessel, the shell and the tubes* | the same |
| mass | ~806 g on the 900, ~307 g on the 600 | **~242 g** |

### The head's tail — a small board in the head's box with three optocouplers, and a thin cable of logic to this one

**Where the five wires land is a small board in the plastic box that houses the head, and the
isolation is on it.** In the order the 24 V travels:

- **the 2-core from the station** on a two-pole `DGPS2.5R-5.0`;
- **two `5.0SMDJ28A` in parallel across it** — the cable's far end takes the same ~45 V clamp as
  its near end on the station's power board, and by the lightning rule of a unit end that is the
  whole protection: no tube, no choke;
- **100 nF film across red and black at the head's terminals** against the controller's hash; the
  head's red and black on a second `DGPS2.5R-5.0`, its three signal wires on a 2,54 mm header;
- **the head-side 5 V from a linear regulator whose input outlives the clamp — `TPS7A1650`**,
  3–60 V in, fixed 5,0 V, 100 mA, 5 µA quiescent, HVSSOP-8 `DGN` (SBVS171F), pin by pin: **`IN` (8)**
  on the 24 V behind the transils with **10 µF 100 V X7R, 1210** beside the pin — the sheet asks
  ≥ 0,1 µF for stability and recommends 10 µF, and 100 V stands well above the clamp; **`OUT` (1)**
  the 5 V node with **10 µF 50 V 1206** — ≥ 2,2 µF for stability, 10 µF recommended; **`EN` (5) tied
  to `IN`**, which keeps `V_EN` ≤ `V_IN` always; **`FB/DNC` (2) on no net at all**, not even ground,
  as the sheet requires of a fixed version; **`NC` (6), `PG` (3) and `DELAY` (7) open**; **`GND` (4)
  and the PowerPAD on the black's copper**. It feeds the two output couplers' `V_CC` and the
  pulse LED, ~10 mA, so at 24 V it dissipates ~0,2 W, ~13 °C over ambient at the package's
  66 °C/W;
- **three optocouplers with totem-pole outputs**, so every output line is driven both ways and
  carries no pull-up and no transistor: two `TLP2745` for `RUN` and `DIR`, one `TLP2361` for the
  pulse;
- **a 2,54 mm header for a thin five-conductor cable to this board: `3,3 V · GND · PULSE · RUN ·
  DIR`** — Pluvius's logic, and nothing of the head's, travels on it.

The 24 V never leaves the tail board. A head with two wires takes no tail board (below).

**What draws what, and when.** Between drains the switched 24 V is off and the tail board is
unpowered — nothing on it draws anything, which is what the switched 24 V was for. While it is
on, the two output couplers draw 3 mA each at most from the 5 V node, and the pulse LED ~3 mA from it once a revolution.
No PWM anywhere: the white is 0 or 5 V.

**The two parts, and why two.** **`TLP2745`** (Toshiba, Rev. 8.0): **buffer logic — LED on, output
high**, totem-pole, SO6L, `V_CC` 4,5–30 V, threshold input current 1,6 mA max, `I_F(ON)` 2–10 mA
recommended, `V_F` 1,35–1,65 V, `V_OH` ≥ `V_CC` − 0,2 V at 3,5 mA, `V_OL` ≤ 0,4 V at 6,5 mA, 5 kVrms — on the 5 V node that is ≥ 4,8 V and ≤ 0,4 V into the head's TTL inputs, against their 2,0 V and 0,8 V. **`TLP2361`** (Toshiba,
Rev. 5.0): **inverter logic — LED on, output low**, totem-pole, SO6, **`V_CC` 2,7–5,5 V**, threshold
1,6 mA max, `I_F(ON)` 2–6 mA recommended, `V_F` 1,35–1,65 V at 2 mA, 3,75 kVrms. **The buffer goes
where the failure direction matters**: a dark Pluvius, a cut cable or a reset leaves both LEDs
off, so the white sits at 0 and the head stands, and the green at 0, forward. **The pulse comes
back to Pluvius's 3,3 V**, below the `TLP2745`'s 4,5 V floor, so it takes the `TLP2361`, which
runs from 2,7 V. Both carry **100 nF ceramic between `V_CC` (6) and `GND` (4) within 1 cm**, as the
sheets require. Both parts run −40 °C and above; the board is unpowered whenever it is cold anyway.

| coupler | the LED | the output |
|---|---|---|
| **the pulse, in** — `TLP2361` | head side — anode (1) through **1,2 kΩ** to the 5 V node, cathode (3) on the yellow: the open collector sinks the LED's 2,8–3,0 mA once a revolution, inside the 2–6 mA the sheet recommends, and the LED holds the yellow up between, so the yellow takes no pull-up of its own | Pluvius side — `V_O` (5) on `PULSE`, `V_CC` (6) on the cable's 3,3 V, `GND` (4) on its ground. **Yellow low → LED on → `PULSE` low**: the open collector's inversion and the coupler's cancel, so `PULSE` is the yellow's own pulse, one falling edge a revolution into the capture. The 4,7 kΩ pull-up on `PULSE` on this board is the Hall switch's, on a two-wire head; the totem pole drives against it at 0,7 mA |
| **`RUN`, out** — `TLP2745` | Pluvius side — anode from `RUN` (PB3) through **510 Ω**, cathode to GND: 3,2–3,8 mA | head side — `V_O` (5) on the white, `V_CC` (6) on the 5 V node, `GND` (4) on the black. **`RUN` high → the white at 5 V, the head at full speed; low → the white at 0, the head still** |
| **`DIR`, out** — `TLP2745` | Pluvius side — anode from `DIR` (PB5) through **510 Ω**, cathode to GND | head side — `V_O` (5) on the green, `V_CC` (6) on the 5 V node, `GND` (4) on the black. **`DIR` low → the green at 0, forward; high → the green at 5 V, reverse** — the manual's open is reverse, and a high level reads as open |

**Two hands on the head, and both must say run.** Palatine's switch puts the 24 V on the tail
board — because a converter standing off between drains is what
the energy budget wants (*The switch*); and **the head turns only while Pluvius drives `RUN`**. A
dark Pluvius, a cut cable or a reset leaves the `RUN` LED off and the white at 0, whatever the 24 V
does — the failure direction of the whole chain is off, at both hands. **So Pluvius runs its own
drain**: it asks for the 24 V in the bit it always did, and with `HEAD` at 1 it starts and stops
the head and runs it in reverse itself (`FIRMWARE.md` §4).

**A few seconds of reverse run empty the tube.** The outlet is a short hose that ends in the air
above a wider open pipe, not a line to a vessel, so at the end of a cycle the head runs backward
for `FLUSH_REV` revolutions (`MODBUS.md`) — a few seconds — draws air in behind the water, and the
tube stands empty under the rollers until the next drain: less set, and nothing in it to freeze.
**The count is signed**: the reverse revolutions are subtracted from `DRAINED`, and the next
cycle's first revolutions refill the tube and add them back, so the total is not moved by it.

**Speed is not controlled.** The white is at the 5 V node or at 0, full speed or still — the
manual's own full-speed connection is the white on the red, so 5 V is inside what the pin takes;
what the count measures is millilitres a revolution, which the speed does not enter. The 5 V node
is the regulator's 5,0 V, stiff at the couplers' and the white's few milliamps.

**A head with two wires has no tail board.** The 900, or a 600 with the brushed motor, takes its
protection on its own terminals: **two leaded `5KP28A` in parallel and a 100 nF film capacitor,
soldered across the motor's lugs**, and the 2-core lands on a small two-pole terminal there, or is
soldered with them — a board for two transils and a capacitor is not built. Where a count is
wanted, the glued magnet and the `55100-3H` (*The pickup*) go to this board's
header on `3,3 V · PULSE · GND`. **With this electronics a two-wire head's hose stays full**; a builder
who wants it emptied adds the direction switching to the head's electronics.

**The head does not hang on the weighing side** — ~242 g on the 600, 806 g on the 900; it mounts
to the structure and only its hose enters the vessel (*The vessel, the shell and the tubes*).

**And the rated hours stop mattering.** 200 cm² of catch against 600–1000 mm of annual rainfall is
**12–20 L a year**, which at 600 ml/min is **20 to 35 minutes of running a year**, against motor
ratings of hundreds to thousands of hours. **What ages the head is standing, not running** —
grease migration, brush oxidation, and a tube that sits compressed under the rollers and takes a
set.

### The switch — driven by Palatine

**The 24 V is Palatine's to switch, whichever head is fitted — a head with two wires has no other switch.** A slave cannot speak first on ModBus, so the request
rides in the answer Palatine asks for anyway: this board sets **`STATUS` bit 1 `PUMP`** when the
weight passes `THRESH` or `DRAIN` is written, Palatine reads it in its poll and raises `ENABLE` on
the `PWR EXT` body, and the power board on it puts 24 V on the head. Palatine writes **`HEAD` (`0x0014`)** back as 1
so this board knows the 24 V is on and starts its dead-drain window from there; **with a head that has control wires
the 24 V alone turns nothing — this board drives `RUN` from `HEAD` = 1, releases it at `EMPTY`
and runs a few seconds in reverse** (*The head's tail*); then the board clears `PUMP`, Palatine drops
`ENABLE` and writes `HEAD` 0. One poll period of
latency at each edge — Palatine reads this unit every 10 s — which a vessel that is a buffer
never notices (`../palatine/FIRMWARE.md` §6, *The `PWR EXT` body*).

```
   this board ── STATUS bit 1 PUMP ──▶ Palatine's poll ──▶ EN_X ──▶ ENABLE ══ BARRIER ══▶ 24 V ─▶ 2-core ─▶ the head
        ◀── HEAD written 1 / 0 ──┘                                   the power board's ammeter: over = stalled, under = dry or cut
```

- **the failure direction:** `ENABLE` is pulled down on the power board and `EN_X` on Palatine, so a
  reset on either side, a cut ribbon and a dark port all leave the head off; this board's bit
  is a request and can only ask — and on a head with control wires the white falls to 0 with this
  board's `RUN`, the second hand
- **what a jammed head does:** the power board's current limit holds it and its ammeter's alert
  reports it to Palatine, which drops the body; **the weight not falling ends the drain on this
  board regardless** (*Detecting a dead drain*), so the head is stopped by whichever sees it first
- **a head running dry or a cut cable** is the ammeter's under-current threshold, Palatine's
  to act on; this board sees the same thing as weight that does not fall
- **the runs and the ceilings:** this board runs the head a minute at a time and reads the
  weight between runs (*Detecting a dead drain*); a cycle ends at 10 minutes on this board and at
  12 on Palatine — the head is never told a duration, the ceilings only catch a bit that stuck. A
  cycle a ceiling ends starts again if the weight is still above `THRESH`, so a full vessel is
  emptied in instalments
- **standing loss:** none — the power board stands off with `ENABLE` low between drains, minutes a year
  of running (*The head*)
- **the cable:** 2-core, 2× 1,5 mm² is plenty, on `DGPS2.5R-5.0` at both ends, in the arm's
  conduit; nothing of it touches this board. **It is metres long, not tens**: the gauge stands
  beside the enclosure (`README.md`, *Where it stands*). The thin five-conductor cable from this board to
  the head's tail is a local cable inside the gauge, logic only, not a field cable

## The vessel, the shell and the tubes

**The gauge is two cylinders, one inside the other.** The **outer shell** is the structure: it
carries the catch at its top, the base with the load cell at its bottom, and the wind — nothing the
wind does reaches the vessel, which is what keeps the weighed mass still and the cell alive; a
vessel that sways in gusts is noise on every reading and side-load on a cell that cannot tell it
from rain. The **vessel** stands inside it with an annular gap, **on the single-point cell, centred**
— that is the geometry a single-point cell is made for — or hangs from a hanger in the shell's
axis with the tubes beside it; the shell makes either possible, and nothing touches the vessel but
the cell. **How the shell is made is the builder's**: a rolled and welded sheet, a bought plastic or
steel pipe with a cut-out, a laser-cut base. The professional gauges are built this way — a housing
with the collector on top, the bucket inside on a load cell or on wires, and the housing taking the
wind — and are expensive for everything else they add. **The shell is the station's solar-reflective
white** (`../daedalus/CONSTRUCTION.md`, *Finish*), which also keeps the vessel cool against
evaporation. **It is not a catch wind shield**: the catch is unshielded, and the WMO-SPICE transfer
functions are applied downstream with the *unshielded* coefficients, from the station's own wind at
gauge height and its temperature (WMO-No. 8, Vol. I, chapter 6, §6.5.2.1); a single Alter shield
around the orifice is the site's option, and a site that fits one says so in its metadata.

**The catch, by WMO's requirements of a gauge** (chapter 6, §6.2.1): **200 cm², Ø 159,6 mm, the area
known to 0,5 %** — a machined or spun ring, never a cut pipe end; **the rim a sharp edge, falling
vertically inside and bevelled steeply outside**; the vertical wall deep and the funnel at **45° or
steeper**, so nothing splashes in or out; the orifice **level**, since a tilted orifice biases the
catch either way; the inner surface smooth, with the least area that drains, against wetting
loss. **The catch stands at 1 m** — the national height, and height buys nothing for the data — and a
deep-snow or mountain site raises the whole unit on a pillar above the maximum snow depth, still
rigid and centred. **A hose from a high funnel to a vessel below is never run**: droplets cling to
the wall, worse as a film builds, and the lag and the loss are the measurement. The funnel's own
down tube is wide, thin-walled stainless, straight into the vessel.

**The vessel is closed but for its tubes.** A lid covers it, the two tubes pass through it with
**~2 mm of clearance** and nothing else is open, so the water's only path to the air is a few square
centimetres of annulus into a still space under a lid: WMO quotes 0,1–0,8 mm a day of evaporation
from open collectors and nothing of the kind from a closed one, and what remains is visible in the
archive as `WEIGHT` sinking on dry days. **Oil is an option, not the build**: a layer of light
paraffin oil floats on the standing water and stops evaporation entirely — WMO's standard practice
for accumulating gauges — and it survives the drain because the drain never reaches it: `EMPTY`,
the base level the firmware pumps down to, is set above the oil's volume plus 10 mm of water over
the intake, so the intake stays under water and the oil stays on top (`FIRMWARE.md` §5). The oil
must float without mixing, let light precipitation through cold, evaporate slowly itself and attack
neither the vessel nor the tube — a paraffin or silicone oil rated for the site's cold. **No
antifreeze and no heating**: a self-mixing antifreeze charge means tens of litres added every autumn
and a mixture the drain cannot be allowed to pump; a heated orifice means a heater and its power.
The winter consequence is written below, not avoided.

**Both tubes reach to 5 mm above the bottom, as close to the centre as the weighing arm allows** —
off-axis is side-force, the one thing a cell cannot tell from load. The funnel's is wide
thin-walled stainless, blanked at the end with holes around the circumference, so a shower does
not arrive as an impulse; the other is the head's hose. **Buoyancy is one exact multiplier and the
span calibration already applies it.** The water fills the annulus between vessel and tubes, the
level cancels, and `R = m·A_v/(A_v − A_t)` — linear in mass, so `m = R·(1 − A_t/A_v)`. Both tubes
stand open to the vessel's water, so only their *wall* displaces: **~0,8 cm² against ~300 cm², a
multiplier near 0,997** where solid rods would have cost 0,98. The head mounts to the shell
and only its hose enters the vessel — on the vessel it would be 2,7 % of the cell's span carried
permanently.

**Drain often, and the reason is the cell, not overflow.** A strain gauge's parameters drift with
the load it has carried and for how long — the lighter it sits, the more slowly its zero and span
move. Creep is the short-term version of the same thing, and every cycle down to the base level
hands back a fresh zero that compensates thermal drift and creep together.

**The hose.** A head with control wires empties its tube after every cycle with `FLUSH_REV`
revolutions of reverse run, counted and subtracted, and the next cycle refills it and adds them
back (*The head's tail*). A two-wire head leaves it full: steady state, every millilitre in at the
bottom is one out at the top, and the head occludes when it stops, so nothing siphons.

**The tube is a thermoplastic elastomer and never silicone.** `PharMed BPT`, `Norprene` or
`Tygon LFL`: an order more flex life than silicone and far less **compression set** — and a set is
exactly what walks the volume per revolution. Both heads ship on `BPT`, `B17` on the 600 and `B24`
on the 900, rated 1000 h. **The tube is a consumable** — at minutes a year of running even a 200 h
tube outlives the station, so it is chosen on cold behaviour and set, never on rated hours.

**Freezing.** The head sits at the gauge, above ground, at most in a box with the electronics, and
at −40 °C that box freezes; nothing is built to stop it, because when the vessel is ice there is
nothing to pump. **Draining happens above freezing; below it the unit accumulates**, which the
18 L reserve also serves — 900 mm of rain. The thermometers say when it freezes and the
firmware starts no drain then, so the head never runs outside the maker's 0–40 °C; a head with
control wires empties its tube after every cycle, so nothing stands in it to freeze. **No heater,
no winter mode.** **The vessel may ice over and thaw as it likes — weight is weight.** What that
costs, in WMO's terms (chapter 6, §6.3.1): **below 0 °C the gauge reads the water equivalent of
what fell when it melts, not when it fell**, and **wet snow can cap the orifice** — a blocked catch
that the data alone cannot show. The firmware raises **`FROZEN`** in `STATUS` while the vessel's
thermometer is below 0 °C, so a reader knows the hourly values of that time are not precipitation
rates, and the snow radar on the mast is the winter instrument (`../palatine/SENSORS.md`). The
hazard is the funnel's stiff tube, fixed to the shell while the vessel is weighed, where ice
bridging the two couples the weighed mass to the structure; what protects it is the drain policy,
since its level is the vessel's level.

## The board

A plain ModBus slave: the MCU reads the bridge ratiometrically and answers in grams. A polled arm
carries no clock — a local 2²⁴ crystal, one RS-485 pair.

```
   four wires ── A · B · 12 V · GND        the arm's isolated 12 V
       │
       ├── A/B ──▶ 2× 10 Ω series + SM712 ──▶ THVD1450 ──▶ the MCU's USART   (any arm)
       │
       ├── 12 V ─▶ 2× 5.0SMDJ18A ─▶ LMR43610 ─▶ 3,3 V ──┬─▶ the MCU and the THVD1450 — they do not care
       │                                             └─▶ 2,2 µH + 10 µF + 100 nF ─▶ ADS1235 DVDD — a converter's digital supply, no LDO
       │
       └── 12 V ─▶ LMR43610 ─▶ 5,5 V ─▶ TPS7A4701 ─▶ 5,0 V ─┬─▶ ADS1235 AVDD
                                                        └─▶ the excitation bridge — EXC+ / EXC−

   the head ── nothing on this board: the switched 24 V on Palatine's EXT body, asked for in STATUS bit 1 (The switch, above)

            ┌──────────── STM32H523, LQFP100 ───────────────────────────────┐
            │ SPI ↔ ADS1235 (DIN/DOUT/SCLK/CS#) + DRDY + START/RESET#/PWDN# │
            │ USART ↔ the THVD1450 (Modbus slave on the arm's pair)         │
            │ timer capture ← the pulse, where the count is used            │
            │ HSE ↔ 2²⁴ crystal · SWD/BOOT → the SWD header                 │
            └───────────────────────────────────────────────────────────────┘

   The pair's barrier and ladder are the arm's communication board's, the 12 V's its power board's; the
   line front and the basic set — the THVD1450, the series pair, the SM712, the two transils on
   the 12 V — are on this board, like every MOD's.
```

**One LDO, and it is the analogue one.** The MCU takes the raw 3,3 V because it does not care;
the converter's `DVDD` is a digital supply and takes the same rail through the station's shielded 2,2 µH,
`SWPA252012S2R2MT`, into 10 µF + 100 nF — the position for a converter's digital supply
(`../core/POWER.md`, *The filter parts*). The one inductor part
on the board is the shielded 2,2 µH under the MCU's `VDDA`/`VREF+` (*Analogue and reference*). The 5,0 V sits on the 5,54 V — 0,5 V of headroom against a dropout under 0,1 V at this current.
**Both bucks are the `LMR43610`**, one order code, because the 12 V node is behind the transils
and they clamp at ~29 V: a 17 V part there dies on the first surge it was fitted to survive. The
5,5 V branch runs in PFM — its ~20 mA cannot fill a 2 MHz cycle with any inductor the part
allows — and the `TPS7A4701` behind it is the filter; the 3,3 V branch at ~150 mA runs CCM on 10 µH — the house cell at 2,08 MHz with the inductor of a rail that feeds a measurement (`../galvani/HARDWARE.md`, *The buck cell*).
Ranging does not apply to a MOD; the ModBus arm carries no clock.

### AC bridge excitation

**The bridge is excited AC by a bridge of four MOSFETs on the 5,0 V — the converter's own four-wire circuit (SBAS824, *AC-Bridge Excitation Mode*), unmodified.** Two halves, each a P-channel `BSS84` from the 5,0 V over an N-channel `BSS138` to ground, the two gates of a half tied together and driven straight by one of the ADS1235's two excitation outputs — `ACX1` on `AIN0`/`GPIO0` the one half, `ACX2` on `AIN1`/`GPIO1` the other, complementary — so one mid-point stands at 5,0 V while the other stands at ground, and both swap at every conversion. The outputs are referred to `AVDD`, so a 5 V gate turns the `BSS138` fully on and a 0 V gate the `BSS84`; **100 Ω in each gate lead**, and **1 µF + 100 nF on the 5,0 V at the bridge** for the instant both transistors of a half conduct while a gate passes its thresholds. The on-resistances — up to 10 Ω on the P side, a few ohms on the N side, 14 mA through them — drop ~0,2 V that the Kelvin-sensed reference measures and cancels, their drift with temperature with it; the two phases differ a little and the combination of the pair absorbs that too. The excitation polarity
reverses between conversions and the pair is combined, which cancels **thermocouple EMFs** across
the cell and its wiring (µV-class, a gram on a 30 kg span, weather-dependent), **the converter's
own offset and its 1/f drift** — exactly where a once-a-minute measurement lives — and **parasitic
DC** in the wiring — and no DC stands across the cell's joints, so moisture on a terminal neither corrodes nor polarises it. With the ratiometric reference beside it, the front's drift is chopped out
rather than specified out, which is why the electronics need not be a precision purchase; what
remains is the cell's ageing, and the drain policy handles that.

**The mode and the settle time, from the converter's sheet (SBAS824).** **Four-wire mode**,
`CHOP[1:0]` = 11: **`ACX1` and `ACX2` complementary on `GPIO0` and `GPIO1`**, and the reversal is
sequenced by the converter itself, once per conversion. **The start-conversion delay is `DELAY[3:0]` = 0110, 189 µs**: the sheet asks for at
least 15 time constants of the input and reference filters, and the slower of them is the signal
side — the bridge's 350 Ω plus the two 100 Ω legs into 10,5 nF, 5,8 µs, so 87 µs — which 189 µs
clears twice over; a once-a-minute measurement pays nothing for the margin. A reversal that has
not settled before the conversion would put the error back.

### ADS1235 front-end (6-wire load cell, `CELL`)

- **EXC+ / EXC−** — the two mid-points of the four-transistor bridge on the `TPS7A4701`'s 5,0 V (`AVDD`): one at 5,0 V and the other at ground, both swapped every conversion (*AC bridge excitation*).
- **SENSE+/−** → 100 Ω → **REFP0 / REFN0** (Kelvin ratiometric ref) — **10 nF C0G across the pair,
  1 nF C0G from each leg to ground**.
- **SIG+/−** → 100 Ω → **AIN5 / AIN4** (anti-alias) — the same: **10 nF across, 1 nF each leg to
  ground**, the differential cap ten times the common-mode ones so their mismatch does not convert
  common-mode noise into signal.
- **`CAPP`/`CAPN`**: 4,7 nF C0G across the pair, the PGA's filter. **`BYPASS`**: 1 µF to `DGND`, the sub-regulator's output. **`AVDD`** 5,0 V and **`DVDD`** 3,3 V: 1 µF + 100 nF at each pin. The sheet's values (SBAS824). **`CLKIN`** to `DGND`: the internal oscillator.
- PGA 64×. `AVDD` and the excitation share the one LDO, so the ratiometric cancellation holds;
  the LDO runs whenever the 12 V is there and the converter sleeps by `PWDN#`.

### Connectors

**Connectors: `MB IN` — the arm's four wires · `CELL` — the load cell · `PUMP` — the cable to the head's tail board, or the Hall switch · `SWD` — programming** (`../galvani/README.md`, *Connector names*).
**One terminal, and the rest is soldered or on 2,54 mm headers.**
- **the arm, 4 poles on two of the family's `DGPS2.5R-5.0`** — in the order `A` · `B` · `GND` · `12 V`, the family's for every MOD (`../babel/HARDWARE.md`), the four-wire cable
  as it comes, like every sensor on the arm. **This is the board's only terminal.** No Galvani
  body and no 12 V terminal: the pair comes to the terminal and the transceiver is this board's;
  the 12 V is the arm's. A MOD is an end station: one cable in, nothing carries on.
- **the load cell — soldered**, six conductors on the board's own pads: EXC+, SENSE+, SENSE−,
  SIG−, SIG+, EXC−. The cell arrives with its tail on it and is never unplugged in service, and a
  soldered tail is the joint that does not walk under the vessel's vibration.
- **the head's tail and the vessel's NTC — 2,54 mm headers**, 5 pins (`3,3 V · GND · PULSE · RUN ·
  DIR`) and 2. With a head that has control wires the five go on a thin cable to the tail board's couplers
  (*The head's tail*); with a two-wire head the glued magnet's Hall switch sits on `3,3 V · PULSE ·
  GND` of the same header. Both headers are inside the enclosure, both are fitted at build, and
  neither is a field cable.
- **the head: nothing** — its two supply wires run from the station to the head at the gauge
  and never touch this board.
- **`SWD` — programming, a 6-pin 2,54 mm header** — `3V3(VTref) · BOOT0(+10k↓) · SWCLK(PA14) · SWDIO(PA13) · NRST · GND` — `NRST` carries its 100 nF and no pull beyond the internal, as on every board.

### Series resistors

SPI and control lines to the converter **33 Ω** each, the house SPI value; the 485 pair **10 Ω** per line, the house
value on every data pair (`../core/blocks/modbus.md`, the basic set); the four load-cell lines **100 Ω**
into their RC; the bridge transistors' four gate leads **100 Ω**.

## The processor — pins, timers, clock tree

*The record to draw the H523 from and to set CubeMX by. Package LQFP100, `STM32H523VE`; pin
numbers and alternate functions from DS14540 Rev 3, Table 13. Every pin below is either
assigned or listed free — nothing is left to guess.*

### Peripherals

| peripheral | instance | pins | job | mode |
|---|---|---|---|---|
| the arm | **USART1** | PB6 `TXD` · PB7 `RXD` · PE2 `DE` | the `THVD1450` — a ModBus slave, answering in grams | RTU, 19 200 8N1; `RE#` tied low |
| the converter | **SPI1** + `CS` PA4 | PA5 `SCLK` · PA6 `DOUT` · PA7 `DIN` | the ADS1235 | mode 1, ≤ 8 MHz; `DRDY#` on PC4, `START` PC5, `RESET#` PB0, `PWDN#` PB1 |
| the drain count | **TIM3** CH1 | PB4 `PULSE` | the pulse — the head's feedback wire through the optocoupler, or the glued magnet's switch | capture |
| the head's run | GPIO | PB3 `RUN` | the `RUN` coupler's LED — high, the head turns | out |
| the head's direction | GPIO | PB5 `DIR` | the `DIR` coupler's LED — low forward, high reverse, the few seconds at the end of a cycle | out |
| `TEMP1` | **I2C1** | PB8 `SCL` · PB9 `SDA` | the `TMP117`/`STS35` beside the cell | 100 kHz, 4,7 kΩ pull-ups; 0x48 or 0x4A |
| `TEMP2` | **ADC1** | PC0 `NTC`, PE3 `NTC_EN` | the NTC in the vessel, 10 kΩ against 10 kΩ, the divider switched on for the conversion | ratiometric against `VREF+` |
| clock | **HSE crystal** | PH0 · PH1 | 2²⁴, the house crystal | |
| debug | **SWD** | PA13 · PA14 | | the `SWD` header |

The AC excitation's polarity switches are driven from the ADS1235's own GPIO pins, per its
sheet — not from the processor — so the reversal and the conversion are sequenced by the part that
converts.

### Pins

| pin | port | signal on the board | AF / function | dir | note |
|---|---|---|---|---|---|
| 1 | PE2 | `DE` | GPIO | out | the `THVD1450`'s `DE`; `RE#` tied low — the receiver is never off |
| 2 | PE3 | `NTC_EN` | GPIO | out | the NTC divider's top — high for the conversion only |
| 3 | PE4 | — | | | free |
| 4 | PE5 | — | | | free |
| 5 | PE6 | — | | | free |
| 6 | VBAT | — | | | tied to `VDD`; no RTC anywhere |
| 7 | PC13 | — | | | free (tamper/RTC pin, unused) |
| 8 | PC14 | — | | | free — no 32 kHz crystal |
| 9 | PC15 | — | | | free |
| 10 | VSS | | | | |
| 11 | VDD | 3,3 V | | | |
| 12 | PH0 | `OSC_IN` | HSE crystal | | 2²⁴ = 16,777216 MHz, the house crystal |
| 13 | PH1 | `OSC_OUT` | HSE crystal | |  |
| 14 | NRST | reset | |  | 100 nF, no pull beyond the internal |
| 15 | PC0 | `NTC` | `ADC1_INP10` | in | the vessel's NTC, ratiometric |
| 16 | PC1 | — | | | free (ADC) |
| 17 | PC2 | — | | | free (ADC) |
| 18 | PC3 | — | | | free (ADC) |
| 19 | VSSA | | | | |
| 20 | VREF− | | | | |
| 21 | VREF+ | 3,3 V through an inductor, `SWPA252012S2R2MT` — the ADC reference | | | |
| 22 | VDDA | 3,3 V through the same inductor | | | |
| 23 | PA0 | — | | | free (ADC) |
| 24 | PA1 | — | | | free (ADC) |
| 25 | PA2 | — | | | free (ADC) |
| 26 | PA3 | — | | | free (ADC) |
| 27 | VSS | | | | |
| 28 | VDD | 3,3 V | | | |
| 29 | PA4 | `CS_ADC` | GPIO | out | ADS1235 `CS#` — 33 Ω in series on every line to the converter |
| 30 | PA5 | `SCLK` | `SPI1_SCK` | out | ADS1235 `SCLK` |
| 31 | PA6 | `DOUT` | `SPI1_MISO` | in | ADS1235 `DOUT/DRDY#` |
| 32 | PA7 | `DIN` | `SPI1_MOSI` | out | ADS1235 `DIN` |
| 33 | PC4 | `DRDY` | GPIO, EXTI4 | in | ADS1235 `DRDY#` |
| 34 | PC5 | `START` | GPIO | out | ADS1235 `START` |
| 35 | PB0 | `RESET` | GPIO | out | ADS1235 `RESET#` |
| 36 | PB1 | `PWDN` | GPIO | out | ADS1235 `PWDN#` |
| 37 | PB2 | — | | | free |
| 38 | PE7 | — | | | free |
| 39 | PE8 | — | | | free |
| 40 | PE9 | — | | | free |
| 41 | PE10 | — | | | free |
| 42 | PE11 | — | | | free |
| 43 | PE12 | — | | | free |
| 44 | PE13 | — | | | free |
| 45 | PE14 | — | | | free |
| 46 | PE15 | — | | | free |
| 47 | PB10 | — | | | free |
| 48 | VCAP | 2× 1 µF 50 V 0805 + 2× 100 nF 50 V 0603 — 2,2 µF, the sheet's 2,2 µF ±20 % | | | |
| 49 | VSS | | | | |
| 50 | VDD | 3,3 V | | | |
| 51 | PB12 | — | | | free |
| 52 | PB13 | — | | | free |
| 53 | PB14 | — | | | free |
| 54 | PB15 | — | | | free |
| 55 | PD8 | — | | | free |
| 56 | PD9 | — | | | free |
| 57 | PD10 | — | | | free |
| 58 | PD11 | — | | | free |
| 59 | PD12 | — | | | free |
| 60 | PD13 | — | | | free |
| 61 | PD14 | `PGOOD` | GPIO | in | the buck's window |
| 62 | PD15 | — | | | free |
| 63 | PC6 | — | | | free |
| 64 | PC7 | — | | | free |
| 65 | PC8 | — | | | free |
| 66 | PC9 | — | | | free |
| 67 | PA8 | — | | | free |
| 68 | PA9 | — | | | free |
| 69 | PA10 | — | | | free |
| 70 | PA11 | — | | | free (USB, unused) |
| 71 | PA12 | — | | | free (USB, unused) |
| 72 | PA13 | `SWDIO` | SWD | i/o | |
| 73 | VDDUSB | 3,3 V | | | tied, USB unused |
| 74 | VSS | | | | |
| 75 | VDD | 3,3 V | | | |
| 76 | PA14 | `SWCLK` | SWD | in | |
| 77 | PA15 | — | | | free |
| 78 | PC10 | — | | | free |
| 79 | PC11 | — | | | free |
| 80 | PC12 | — | | | free |
| 81 | PD0 | — | | | free |
| 82 | PD1 | — | | | free |
| 83 | PD2 | — | | | free |
| 84 | PD3 | — | | | free |
| 85 | PD4 | `LED` | GPIO | out | the status LED, fitted — 1 kΩ from the 3,3 V, one 50 ms blink a minute while healthy, two on a fault (`../core/HARDWARE.md`) |
| 86 | PD5 | — | | | free |
| 87 | PD6 | — | | | free |
| 88 | PD7 | — | | | free |
| 89 | PB3 | `RUN` | GPIO | out | the `RUN` coupler's LED through 510 Ω — high runs the head; low, the reset state, leaves it still (*The head's tail*) |
| 90 | PB4 | `PULSE` | `TIM3_CH1` capture | in | the counted drain's pulse — 4,7 kΩ to 3,3 V |
| 91 | PB5 | `DIR` | GPIO | out | the `DIR` coupler's LED through 510 Ω — low is forward, the reset state; high runs the head in reverse, the few seconds at the end of a cycle (*The head's tail*) |
| 92 | PB6 | `TXD` | `USART1_TX` | out | the `THVD1450`'s `D` |
| 93 | PB7 | `RXD` | `USART1_RX` | in | the `THVD1450`'s `R` |
| 94 | BOOT0 | 10 kΩ to ground | |  | |
| 95 | PB8 | `SCL` | `I2C1_SCL` | o/d | the thermometer |
| 96 | PB9 | `SDA` | `I2C1_SDA` | o/d | the thermometer |
| 97 | PE0 | — | | | free |
| 98 | VCAP | 2× 1 µF 50 V 0805 + 2× 100 nF 50 V 0603 — 2,2 µF, the sheet's 2,2 µF ±20 % | | | |
| 99 | VSS | | | | |
| 100 | VDD | 3,3 V | | | |

**24 GPIO used, 56 free** (PA0–PA3 · PA8–PA12 · PA15 · PB2 · PB10 · PB12–PB15 · PC1–PC3 · PC6–PC15 · PD0–PD3 · PD5–PD13 · PD15 · PE0 · PE4–PE15). **The head's 24 V is switched by Palatine, never by a pin here** — the request is a bit in a register Palatine reads; on a head with control wires `RUN` and `DIR` drive it through the tail board's couplers (*The switch*, *The head's tail*).

### Timers

| timer | width | clock | channels used | role |
|---|---|---|---|---|
| **TIM3** | 16 | 2²⁷ ÷ prescaler | CH1 capture ← PB4 | the pulse — a pulse per revolution from the head's wire or the Hall switch, the counted drain |
| **TIM6** | 16 | 2²⁷ ÷ prescaler | none | the RTU frame gap; the drain's duration |
| all others | | | | free |

### Clock tree

| | |
|---|---|
| HSE | **the house crystal, 2²⁴ = 16,777216 MHz — `SWXBEABVF0-16.777216`** (Starwave XB, 5032 ceramic, ±20 ppm over −40…+85 °C, `C_L` 10 pF), **12 pF C0G** on each side — 2·(`C_L` − 4 pF). One part on every board that carries a crystal (`../core/blocks/clocks.md`). A polled ModBus arm carries no clock, and the host stamps the value when it polls |
| PLL1 | **M 2 · N 32 · P 2** → the PLL sees 2²³, VCO 2²⁸ = 268,435456 MHz, `P` 2²⁷. `M` is 2 because **`f_PLL_IN` is 2…16 MHz** (DS14540 Rev 3, Table 46) and the crystal's 16,777216 MHz is above it |
| SYSCLK, AHB, APB1/2, timer kernel | **2²⁷ = 134,217728 MHz** |

A power of two like every clock in the station: the board's own tick arithmetic stays binary,
and the baud tolerance is what the crystal's grade is bought for.

### Analogue and reference

**Every filter inductor on this board is a π filter: 10 µF 1206 + 100 nF 0603 before it on the rail and
the same pair at the pins behind it** — the house rule (`../core/POWER.md`, *The filter parts*).

`VDDA` and `VREF+` on the 3,3 V through the shielded 2,2 µH, `SWPA252012S2R2MT`, into 10 µF + 100 nF —
the house filter for an ADC reference on a buck's output (`../core/POWER.md`, *The filter parts*). The processor's ADC reads
one thing, the vessel's NTC — ratiometric against `VREF+`, so the reference's value cancels; the
measurement is the ADS1235's, ratiometric to the excitation it shares its reference with, and the
processor sees counts over SPI.

## Parts

| part | qty | where |
|---|---|---|
| `STM32H523VE`, LQFP100 | 1 | the processor |
| `SWXBEABVF0-16.777216` + 2× 12 pF C0G | 1 set | HSE |
| 2× 1 µF 50 V 0805 + 2× 100 nF 0603 | 2 sets | `VCAP` |
| 100 nF | one a supply pin | every supply pin of the processor and of every part |
| 100 nF · 10 kΩ | 1 each | `NRST` · `BOOT0` to ground |
| `ADS1235` | 1 | the converter, SPI1 |
| 4,7 nF C0G · 1 µF | 1 each | `CAPP`/`CAPN` · `BYPASS` |
| 1 µF + 100 nF | 2 sets | the converter's `AVDD` and `DVDD` |
| 100 Ω | 4 | the cell's `SENSE±` and `SIG±` into their RC |
| 10 nF C0G | 2 | across `REFP0`/`REFN0` and across `AIN5`/`AIN4` |
| 1 nF C0G | 4 | each cell leg to ground |
| `BSS84` · `BSS138` | 2 each | the excitation bridge |
| 100 Ω | 4 | the bridge's gate leads |
| 1 µF + 100 nF | 1 set | the 5,0 V at the bridge |
| 33 Ω | 8 | `SCLK` · `DIN` · `DOUT` · `CS_ADC` · `START` · `RESET` · `PWDN` · `DRDY` |
| `TPS7A4701` — `3P2V` and `0P4V` to ground for 5,0 V · 10 µF in · 10 µF + 100 nF out, at the pin · 10 nF `NR/SS` · 10 nF `FF` | 1 set | the 5,0 V |
| `LMR43610R3RPER` | 2 | the 3,3 V and the 5,5 V |
| `R_FBT` 28,0 k · 54,9 k, `R_FBB` 12,1 k, `C_FF` 22 pF C0G | 1 set each | the two dividers — 3,31 V and 5,54 V (`../galvani/HARDWARE.md`, *The buck cell*) |
| 10 µH shielded, `I_SAT` ≥ 2,1 A · 4,7 µH shielded, `I_SAT` ≥ 3,5 A | 1 each | the 3,3 V's inductor · the 5,5 V's |
| 3× 10 µF 50 V 1206 + 100 nF · 4,7 µF 50 V + 100 nF · 1 µF · 100 nF | 2 sets | each buck's `C_OUT` · `C_IN` · `VCC` · `BOOT` |
| 100 kΩ | 1 | the 3,3 V buck's `PG` up, to PD14 |
| `5.0SMDJ18A` | 2 | the 12 V input |
| `THVD1450` · 2× 10 Ω · `SM712` | 1 set | the line front |
| `TMP117` or `STS35` + 2× 4,7 kΩ | 1 | `TEMP1`, I2C1, by population |
| NTC 10 kΩ + 10 kΩ | 1 | `TEMP2`, the divider switched from PE3 |
| 2,2 µH `SWPA252012S2R2MT` | 2 | `VDDA`/`VREF+` · the converter's `DVDD` |
| 10 µF + 100 nF | 4 sets | before and behind each inductor, π |
| 4,7 kΩ | 1 | `PULSE` up |
| 510 Ω | 2 | the `RUN` and `DIR` coupler LEDs |
| 1 kΩ + LED | 1 | the status LED on PD4 |
| `DGPS2.5R-5.0`, two-pole | 2 | `MB IN` |
| 2,54 mm headers, 5 · 2 · 6 pins | 1 each | `PUMP` · the NTC · `SWD` |
| six pads | 1 | `CELL` |

**The head's tail board**: 2× `DGPS2.5R-5.0` · 2× `5.0SMDJ28A` · 100 nF film ·
`TPS7A1650` `DGN` with 10 µF 100 V 1210 and 10 µF 50 V 1206 · 2× `TLP2745` · `TLP2361` · 3× 100 nF ·
1,2 kΩ · 2,54 mm headers, 3 and 5 pins. **A head with two wires**: 2× `5KP28A` and a 100 nF film
capacitor on its lugs, and the glued magnet with the `55100-3H` where a count is wanted.

## Layout / EMC

- **Split analog / digital:** the ADS1235, the LDO and the bridge traces are a quiet island.
  **Star ground.** The two rail bucks, the transceiver and the basic set sit at the far edge from
  the bridge input, with the arm's terminal.
- **Ratiometric routing:** tight EXC/SENSE pairs, guard SIG±.
- **The TPS7A4701** is the bridge's reference — keep it out of dropout.
- **Clock pin** (HSE) short, away from switching nodes.
- **Conformal coat** for the humid box — class SR or AR to IPC-CC-830 / IEC 61086, neutral cure
  (nothing acid released against the copper and the cell's wiring), the arm's terminal and the
  headers masked.

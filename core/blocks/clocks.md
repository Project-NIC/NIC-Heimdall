★ N.I.C. ★

# NIC HW Block — the station's oscillators, tier by tier

> **Design-stage concept.** The figures are verified against the parts' sheets and the parts
> before a board is made.

**Exactly ONE deliberately precise oscillator per station; everything else is wire, commodity or
calibration.** What follows is by tier, because what a board owes its oscillator is decided by how
it gets its time, not by what it measures.

## The whole station on one page

**One TCXO, four crystals of one part, and everything else takes its frequency off a wire or measures it.**

| what | oscillator | how it reaches SYSCLK |
|---|---|---|
| **Kronos** | **TCXO** ±1,0 ppm, the one deliberately precise part | HSE bypass → PLL, GPSDO-disciplined |
| **the cards** — Bifrost, Argus | **none** | HSE bypass off Kronos's time bus or the up port, 2²² either way |
| **NodBus units** — Quake, Tesla/Pip, Marconi, the two scintillation `Quark` boards, Palatine, Sputnik | **none** | HSE bypass off the data body's `CLK`, 2²² |
| **NodBus mini** — **Gauss, Pascal, `Quark-Tubes`** | **none** | **HSI into the PLL, the rung steering `FRACN`** — one clock tree across the tier |
| **ModBus MODs** — Pluvius, Babel, Sakura, Ceres | **the house crystal**, 2²⁴ `SWXBEABVF0-16.777216` | HSE → PLL, M 2 · N 32; a polled arm carries no clock at all |
| **Hermes** | **none** | CSI or HSI ÷ 8, 4 or 8 MHz, no PLL |
| sensor clocks, detector heads, `RM3100` | **none** | derived on `MCO` or a timer; the `RM3100`'s timebase is a resistor |

**Every H523 board in the station but Hermes (4 or 8 MHz, no PLL) runs SYSCLK 2²⁷ = 134,217728 MHz
off VCO 2²⁸, and every H7A3 board 2²⁸ off VCO 2²⁹** — what differs is only where the PLL's input
comes from. **On the mini tier that binary output is made by the loop and not by an oscillator's
value**: the source is a 64 MHz RC whose nominal is not binary at all, and `FRACN` is what holds the
VCO on 2²⁸.

**`f_PLL_IN` is 2…16 MHz on both parts and it is a real ceiling** (H523: DS14540 Rev 3, Tables 46
and 47 — H7A3: DS13195, Tables 55 and 56; the 1…2 MHz band belongs to the medium VCO range on
either). **A 2²⁴ source is 16,777216 MHz and is above it**, so `M` is never 1 on a board fed 2²⁴ —
Kronos and every crystal board divide by 2 first and the PLL sees 2²³. A 2²² source is
inside the band as it stands, which is what every other board is fed — the cards included.

**Two ceilings above that, and the H7A3 tier sits close to both.** The H7A3's wide VCO range is
128…560 MHz — the H523's is 192…836 MHz (RM0481) — and **the H7A3 boards' 2²⁹ is
536,870912 MHz — 96 % of the top, with 23 MHz of headroom and no room to go higher.** Their `P` output of 2²⁸ = 268,435456 MHz then needs **VOS0,
whose ceiling is 280 MHz**; VOS1 stops at 225 and would not carry it, so the scale is a
requirement on those four boards and not a setting. **On the H523 tier 2²⁷ = 134,217728 MHz needs
VOS2 or better** — VOS3 stops at 100 MHz — and there is a third of the scale in hand.

## The one precise part — Kronos

| | |
|---|---|
| part | **±1,0 ppm TCXO**, Mercury `MQF574T33-16.777216-1.0/-40+85` — the 2²⁴ part (`../../kronos/HARDWARE.md`) |
| what it makes | PLL → **8,388608 MHz = 2²³, GPSDO-disciplined** — THE network timebase |
| class | 1 ppm holdover, about 86 ms a day with no GPS; ~0 ppm long-term |
| fan-out | the **M-LVDS time bus** — one `DS91C176` driver per pair on Kronos, a `THVD1450` receiver at every tap, the cards and the Mayak (the head receives the clock and its firmware uses `PPS_K` only). A 485 link exists only on a Galvani board — the `THVD1450` here only receives an M-LVDS pair that never leaves the box; in the box the clock is a driven pair on a ribbon |

**Price is not a question at this position and was never weighed.** There is **one** of these in a
station and it is the part every other clock in the building is derived from — a worse one is paid
for by every measurement, every day, forever. That is the opposite of the crystal tiers below,
where the part is bought by what is in stock. **Never derive the timebase from the ESP32's
crystal.**

## The cards and the NodBus units — no oscillator at all

| tier | part | why |
|---|---|---|
| Bifrost / Argus | **none** | Kronos's 2²² on `OSC_IN` off the time bus, or the up port's; the card multiplies it and keeps no oscillator of its own |
| NodBus units (Quake, Tesla/Pip, Marconi, the two scintillation `Quark` boards, Palatine, Sputnik) | **none** | HSE bypass off the data body's `CLK`; the internal RC covers boot and handshake, where the 38k4 base rate tolerates ±1 % with margin |
| sensor clocks — ADXL `EXT_CLK`, ICM `CLKIN`, the converters' `ENC` | derived on `MCO` or a timer | they inherit the wire; no local part anywhere |
| detector heads (Photon, Helion, Gadolin/Rhodion) | **none — the heads carry no MCU** | they hand `Quark-Tubes` pulses on its EXTI lines and keep no time; the counting and the binning are the board's, on the rung it captures |
| `RM3100` | **no oscillator — `REXT` is its timebase** | thin-film 0,1 %, 10–25 ppm: the sonde's precision "oscillator" is a resistor |

## NodBus mini units — no crystal, one clock tree

**Three boards — Gauss, Pascal, `Quark-Tubes` — and one tree.** The segment's rung lands on a
**timer's external clock input, not on HSE**, so the grid is the rung's and the core clocks only
the UART and the ranging turnaround. **The rung disciplines the HSI through the PLL on all three**:
`TIM5` gates the core against the rung, the error goes into `FRACN`. On the sondes the body forbids
a crystal (below); on `Quark-Tubes` nothing forbids one, and it is left off so the tier has one
clock tree — the board gains two pins and loses a part and its load pair. `Quark-Tubes` keeps no
holdover table: a land board has no reason to keep time without the rung, which is a lost link.

## The two sondes carry no crystal at all

**Gauss and Pascal, every form of both.** The reason is the body, and it does not depend on how
deep the body goes.

**A crystal is a sealed gas cavity by construction** — the blank has to vibrate in one — and
these are potted or oil-filled bodies that pass hydrostatic pressure straight through to the
parts; the tube wall protects nothing. It is the same rule that keeps a 1×9 optical module out of
a sonde. **And the numbers close it**: a lid's stress goes as `p·(span/thickness)²`, so a 0,1 mm
lid over a 2,5 mm span sees **~20 MPa at 5 bar, ~2 GPa at 30 bar and tens of GPa at 800 bar**
against a few hundred MPa of yield. **Depth is not a grade question and no vendor rates it** —
past the shallow envelope the part simply goes, and short of it it is still pulled off frequency
by a stress nobody specifies. **So the position is deleted in both builds, not graded per build**
— one board and one clock tree is worth more here than a crystal on the shallow one.

**The grid never came from the crystal** — the rung is already on a timer's external clock input
— so what the crystal has to be replaced for is the core and the UART, and there **the HSI's own
±2 % over temperature is what the crystal used to cover**. That is why a MOD, which has no rung,
carries one.

**What covers it is a loop and not a part: the rung disciplines the HSI through the PLL, the same
way Kronos disciplines its TCXO against GPS.** The reference is on the pin continuously instead of
once a second, the steered part is an RC instead of a TCXO, and the correction goes into the same
place — `FRACN` in PLL1, written with the PLL running, dithered between two codes when the
residual is finer than one step. **Each board carries the loop written out with its own timers and
its own consumers** (`../../gauss/HARDWARE.md`, `../../pascal/HARDWARE.md`, `../../quark/tubes/HARDWARE.md`); what belongs here is
only that the tier owes no oscillator and buys no part for one.

**There is no fallback part.** A clock multiplier off the rung was the one candidate and it is
dropped on power (`../WHY.md`); if the loop falls short, the answer is a better use of the rung.

**What sets a sonde's depth is the cavity inventory and the mechanics, never the clock.** The sea
build of both goes to **~250 m** (`../../gauss/CONSTRUCTION.md`), inside Pascal's own
`MS5837-30BA` at 30 bar; the deep build to 8 km, where at 800 bar no sealed cavity of any size
survives, is Atlantis's and shelved. The question to ask of every part is
the same one: *does it contain gas?* The crystal did and is gone; a 1×9 optical module does and is
already refused; MEMS packages do, which is why Gauss dropped the inclinometer; the MCU, the
transils, the thermometer and the `RM3100` do not. What is left after that is the tube and the
cable entry.

## ModBus MODs — the same crystal, for the baud alone

**Four boards: Pluvius, Babel, Sakura, Ceres.** A polled arm carries no clock and the host stamps
the value when it polls, so the part is there for 9 600 / 19 200 RTU, which wants about 2 % from
both ends together. **On Sakura and Ceres it also sets the excitation**: `TIM1` makes 2²⁰ / 2²² /
2²⁴ Hz from SYSCLK by whole divisors, so the value must stay binary — see below.

**There is no exception in these two tiers.** Every form of every one of those boards takes
its rung or its poll over the wire or the glass — a standalone Gauss on its stake and the sea sonde
included — so **no board in them carries a precision oscillator** (`../../gauss/WHY.md`).

## The house crystal — one part on the four MODs

**`SWXBEABVF0-16.777216`** (Starwave, XB series) — **2²⁴ = 16,777216 MHz**, 5032 seam-welded
ceramic, 5,0 × 3,2 mm; ±10 ppm at 25 °C, ±20 ppm over −40…+85 °C, ageing ±3 ppm/year, `C_L`
10 pF, ESR ≤ 60 Ω, drive 10 µW typical. **The load pair is 12 pF C0G** on each side,
2·(`C_L` − 4 pF), 4 pF being the pins and the traces. **PLL1 is M 2 · N 32 · P 2** on every board
that carries it: `f_PLL_IN` tops out at 16 MHz, so the PLL sees 2²³, VCO 2²⁸, SYSCLK 2²⁷.

**One part, not a choice among three.** It is the one binary crystal a distributor actually
stocks at this frequency. Should it go out of stock, a 2²³ or 2²² binary part is the substitute —
M 1 · N 32 or M 1 · N 64, two fields of one register — and the load pair follows its `C_L` by the
same rule; that is a substitution at the order, not a second house part.

**Package is free on all four.** Sakura and Ceres are potted, but the spacer ring is cut a few
tenths taller than whatever stands highest and the fill follows it — Ceres filled solid to its
glass disc — so a can would cost nothing there either, and nothing in this tier goes under water. **The one
place where the package would have been a selection rule is the mini tier, and it carries no
crystal at all** (above).

**What is not free is leaving the binary series.** An ordinary 16,000000 MHz part reaches 2²⁷ by no
integer factor, and then the timers' periods stop being whole — which is what Sakura's and Ceres's
excitation and every frame count on. **Binary is the requirement**, and the house part meets it.

**And the sourcing fact belongs with the part: the binary frequencies are badly stocked.**
16,777216 MHz is a catalogue corner where an ordinary 16 MHz is not, the small packages are
scarcer than the cans, and a tighter grade at this value is priced like a different class of
component. Whoever buys it plans for a substitution rather than assuming the row is fillable.

## What a station's precision costs, counted

**One TCXO, one GNSS module, one thin-film resistor.** Precision is distributed by architecture —
the wire on the trunk, edges and anchors on the leaves — and not by parts, which is why a station
of this cost can time like an observatory.

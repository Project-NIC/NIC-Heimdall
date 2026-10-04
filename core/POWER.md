★ N.I.C. ★

# Power and temperature — the source, the pack, the lockdown and the vault

> **Design-stage concept — nothing built.** The figures are verified against the parts' sheets and
> the parts before a board is made.

This document owns where the station's energy comes from, how it is stored, how the station goes
down when it runs out, and how the electronics survive −40 to +60 °C. What happens to the 12 V
after the pack — the wire, the feeds, the isolation and the surge ladder — is the Galvani family's
(`../galvani/README.md`). The house parts every board shares — the capacitor rating, the three
resistor values, the filter parts — close the document.

## 1. The load

**The draw is the station's population, summed from the boards' own figures.** A full station —
the head, Kronos, four cards, Palatine with its arms, Tesla, Marconi, Quake, the radiation units and
two remote Arguses on 300 V — draws **~27 W average**, and since the station does not save power in
normal running, the average is the steady state. **Count 40 W, 1 kWh a day**: the converters'
no-load draw, which the sum leans on hardest, is estimated. A bare station — the head, Kronos, one
card, Palatine — is **~13 W**. **Peak, everything at once, ~60 W** — the pump, a Wi-Fi dump, a
heater: 5 A off 12 V, which is what the BMS's current rating and the fuses see. **The sum of the
boards' ratings is ~340 W** and never happens: it is what the `INA238` thresholds and the fuses
stand if every port carried a far end at its cell's ceiling. Each build sums its own population.

**Nothing is underclocked to save power, and no build is advised to.** The processor rate is one
number per tier: every H523 but Hermes (4 or 8 MHz) at **2²⁷ = 134 217 728 Hz**, every H7A3 — Tesla,
Pip, Marconi and the two `Quark` scintillation boards — at **2²⁸ = 268 435 456 Hz**, the compute
that tier was bought for. A CMOS core scales its dynamic power with frequency, but its static draw
and every analogue front do not move, and a non-binary core rate puts a remainder into every divider
a card and a unit own and takes the ranging tick off 7,45 ns. **What is saved is saved at the
parts**: peripherals left unclocked, converters in their own standby. **A board is not parked**: one
with nothing to do is ended and switched off, and what "off" means depends on whether it has a feed
above it (`PROTOCOL.md` §7).

**The source follows the load.** Solar is sized by the worst season: a mid-latitude winter of
~2,5 peak-sun-hours a day needs **~570 W** of panel for 1 kWh a day (1 000 Wh ÷ 2,5 h ÷ ~0,7
system efficiency) before oversizing — every watt of load is ~14 W of panel. The lockdown does not
shrink this: the source is sized against the full load, and the lockdown is survival, not a budget
line.

## 2. The source — solar or grid, by site

- **Solar, off-grid.** A surface panel → an **MPPT** charge controller (better cold and cloudy
  harvest than PWM) → the pack. The panel is sized for the site's **worst-season insolation**, not
  the annual average, and **oversized**: overcast yield is ~10–20 % of rating, and the surplus of a
  clear day is what catches up. It stands on its own low frame **≥ 10 m from the air-temperature
  sensor**, whose plume it would otherwise warm; the string travels to the vault at string voltage,
  and **the 12 V never leaves the vault** (`../daedalus/CONSTRUCTION.md`, *The structures*).
- **Grid, where there is one.** A wide-input DC supply feeds the same charge path; the pack then
  only rides through outages and can be much smaller.
- Wind and other harvesters are a site's add-on, not the base.

**Low power is what makes the network affordable.** Solar and storage cost scales with the draw: a
kilowatt station needs kilowatt-class solar and storage and is still short in winter, a
tens-of-watts station runs for days on its pack alone. The pack is sized for **autonomy**, the
panel for the worst season.

## 3. The pack — eight 314 Ah cells, charged between 20 and 80 %

**Eight 3,2 V prismatic cells of 314 Ah, two in parallel and four in series — 2P4S, 12,8 V, 628 Ah,
8,0 kWh on the nameplate — and that is the ceiling.** A larger pack is a vehicle battery by weight
and by price; what it does not carry is what the lockdown is for.

**It is charged between 20 % and 80 %, for the cells' life.** The charge stops at ~80 %, ~3,40 V a
cell, ~13,6 V on the pack; the lockdown takes the station down at 20 %. **4,8 kWh is the working
energy:**

| | average draw | Wh/day | autonomy on 4,8 kWh |
|---|---|---|---|
| full station | ~27 W summed — **count 40 W** | 1 000 | **~4,8 days** at 40 W, ~7,4 at 27 |
| bare station (head · Kronos · one card · Palatine) | ~13 W summed | 310 | **~15 days** |

**The pack and panel are the last things chosen, and against a measurement.** The sum starts the
build; the finished station measures itself — the BMS and the MPPT report through Hermes from day
one, and that telemetry is the sizing instrument — then `autonomy = 4,8 kWh ÷ measured Wh/day`, and
the panel from the worst season, oversized. **Nothing upstream of the battery terminals is a part on
the bill of materials**: panel, charge controller, cells and BMS are chosen per build against the
site and against what the country of assembly can buy. The cells below are the reference a purchase
is compared against; any cell of the class that meets their rows is the pack.

### The chemistry — for the cold

| chemistry | in the cold | trade | fit |
|---|---|---|---|
| **LiFePO₄** | long life, safe, cheap — **a standard cell does not charge below 0 °C** | wants a place that never freezes | **the pack**, sited in the vault or frost-free |
| **LiFeYPO₄** (yttrium-doped LiFePO₄, Winston) | **−45 to +85 °C on the sheet for charge and discharge**, ~90 % of capacity at −25 °C, ~80 % at −45 °C at 0,5 C; the distributor's guidance is charge to about −25 °C at reduced current | ~3× the price per Ah of a standard cell; one maker | **the pack for a vault that may see frost**, the same 2P4S — the cold rating is margin over the vault, not a licence to stand the pack in frost |
| **LTO** (lithium titanate) | charges and discharges deep into frost, ≈ −30 to −40 °C; ~10 000 cycles | low energy density, higher €/Wh | a pack that must stand exposed in frost |
| **Na-ion** | good to ≈ −20 to −40 °C, cheap materials, safe | lower density, supply still maturing | a pack that must stand exposed in frost — **in the 20–80 % window it fits the station's 12 V input with nothing changed** (below) |

**LiFePO₄ is the pack, and its siting keeps it above 0 °C**; the LiFeYPO₄ cell covers a vault that
turns out colder than planned; LTO or Na-ion only where a pack must stand exposed in frost. A BMS
with a low-temperature charge rule is fitted in every case.

**A Na-ion pack in the 20–80 % window.** A Na-ion cell has no plateau — its voltage falls about
linearly with charge — so the window cuts it cleanly. The layered-oxide cell against hard carbon,
at rest unless marked:

| state of charge | a cell | 4S |
|---|---|---|
| 20 % | ~2,9–3,0 V | **~11,6–12,0 V** |
| 50 % | ~3,2–3,3 V | ~12,8–13,2 V |
| 80 % | ~3,5–3,6 V | **~14,0–14,4 V** |
| the charge stop at 80 %, with current | ~3,6 V | **~14,4 V** |
| 20 % under load, in the cold | ~2,85 V | ~11,4 V |

**14,4 V stays under the input transil**, `5.0SMDJ14A`, which conducts from 15,6 V, and **11,4 V
stays inside the pin's 10–20 V and above the source boards' 11 V floor**, so a 4S Na-ion pack whose
BMS holds 2,9–3,6 V a cell takes the station's inputs as they are. The polyanion cell (NFPP) is
flatter, ~3,0–3,2 V a cell, near 11 V on 4S under load in the cold; the cell's own table of voltage
against charge is what the BMS is set by.

**The BMS derates rather than refuses.** Lithium plating is a rate effect: a cell charged at
≤ 0,1 C tolerates a few degrees of frost that 0,5 C would plate. The BMS **limits charge current to
≤ 0,1 C below 0 °C and cuts it only below about −10 °C**, so a cold snap charges slowly instead of
not at all — on the LiFeYPO₄ cell. **A standard cell carries its sheet's charge map instead**: EVE's
`MB31` allows 0,05 P at 0 °C, 0,12 P at 5 °C, 0,3 P at 10 °C and the full 0,5 P from 15 °C, and
**nothing below 0 °C**. The vault keeps either from being needed; the BMS keeps it from being a
fault.

### The cells

**The reference is EVE `MB31`, 2P4S**: 628 Ah and 8,0 kWh, ~45 kg of cells in a steel frame, at
about a third of a LiFeYPO₄ cell's price. **Where the vault may see frost the pack is the same 2P4S
of Winston's `WB-LYP300AHA`** — 600 Ah, 7,7 kWh, ~78 kg — from its sheet (*Specification for rare
earth lithium yttrium power battery*):

| `WB-LYP300AHA` | |
|---|---|
| capacity · voltage | 300 Ah · 3,2 V nominal (Winston writes 3,3). **4,0 V charge and 2,8 V discharge are damage limits, not working points**; 2,5 V is a dead cell |
| internal resistance | ≤ 0,3 mΩ — on the bus the cells' voltage is the rail |
| currents | 0,5 C standard, 150 A; 3 C charge and continuous discharge, 10 C pulse. The station draws ~3 A, **0,01 C on one string** — no current figure binds |
| cycle life | **≥ 5000 at 80 % DoD, ≥ 7000 at 70 %**; the 0,5 C curve: ~90 % at 3000, ~80 % at 5000, ~70 % at 8000 |
| temperature | **−45 to +85 °C, one band for charge and discharge on the sheet**; the distributor (GWL) says charge to about −25 °C at reduced current, and the BMS rule above stands whatever the sheet allows |
| capacity in the cold | 0,5 C discharge: ~90 % at −25 °C, ~80 % at −45 °C, the plateau at 2,7–2,8 V |
| self-discharge | ≤ 3 % a month, ~4 Wh a day on a 3,84 kWh string; the storage curve reaches **~70 % after a year**, so a pack that waited in a store is charged before it goes into the vault |
| case | 362 × 55,5 × 306 mm, 9,7 kg, **rigid plastic, no compression frame**, two M8 posts; rated to 200 °C |
| charge window | **20–80 %, the charge stopping at ~3,40 V a cell, ~13,6 V on the pack**; the sheet's 4,0 V and the distributor's 3,65 V are ceilings, never the set point |
| source · price | GWL (Prague), EV-Power, EV Europe — stock in the EU; ~370–400 € a cell (2026) |

**The standard ESS cells, from their sheets.** Every other maker of a ~300 Ah prismatic cell sells
standard LiFePO₄, and the sheets agree: **3,65 V charge and 2,5 V discharge cut-off, a recommended
window of 10–90 %, no charging below 0 °C on any of them**, self-discharge ≤ 3 % a month, storage for
a year at 0–35 °C and 30–50 % with a cycle every six months, an aluminium can measured and cycled
**in a 300 kgf fixture**, and a third of a LiFeYPO₄ cell's price. The EVE sheets are the maker's own;
the CATL 280 Ah is a reseller's one-page sheet and CATL's 2020 BESS brochure:

| cell · sheet | Ah | `R_i` | currents | charge · discharge °C | in the cold | cycles, 25 °C | case at 300 kgf · kg |
|---|---|---|---|---|---|---|---|
| **EVE `LF280K`** · B | 280 | ≤ 0,25 mΩ | 0,5 C standard, 1 C continuous, 2 C for 30 s | 0…55 · −20…55 | ≥ 70 % at −20 °C, 0,5 C to 2,0 V | **≥ 6000 at 0,5 C/0,5 C to 80 %**; ≥ 2500 at 45 °C | 173,7 × 72,0 × 207,5 mm · 5,42 |
| **EVE `LF304`** · B | 304 | ≤ 0,5 mΩ | 0,5 C standard, 250 A continuous, 2 C for 30 s | 0…60 · −30…60 | ≥ 70 % at −20 °C, 1 C to 2,0 V | ≥ 3500 at 1 C/1 C to 80 %; ≥ 1800 at 45 °C | 173,5 × 72,0 × 208,8 mm · 5,49 |
| **EVE `MB31`** · A | 314 · 1004,8 Wh | 0,18 ± 0,05 mΩ | 0,5 P standard and maximum, both ways | 0…60 · −30…60; absolute −35…65; **cut-off 2,0 V at ≤ 0 °C** | ≥ 80 % of the energy at 5 °C | **8000 to 70 % SOH** at 0,5 P | 173,7 × 71,7 × 207,2 mm · 5,6; **swelling force ≤ 50 kN at 70 % SOH, ≤ 60 kN at 60 %** |
| EVE `LF230` · D | 230 | ≤ 0,30 mΩ | 0,5 C, 1 C, 2 C for 30 s | 0…60 · −30…60 | ≥ 70 % at −20 °C, 1 C to 2,0 V | ≥ 3500 at 1 C/1 C; ≥ 1800 at 45 °C | 173,9 × 53,9 × 207,3 mm · 4,11 |
| CATL 280 Ah · reseller sheet, BESS brochure | 280 | ≤ 0,18 mΩ | 1 C continuous, 3 C maximum | 0…65 · −35…65 | — | 8000 at 0,5 P (brochure) | 173,9 × 71,7 × 207,2 mm · 5,51 |
| CATL 228 Ah, long-life · 2022 brochure | 228 | — | — | −35…65 "operating", the pack's figure with its heating films | — | **15 000 at 100 % DoD** (the high-energy variant: 4000) | 53,7 × 173,9 × 204,6 mm · 4,2 |

Hithium, CALB and REPT sell the same class; no sheet of theirs was read. CATL also sells four in a
box — the 228 Ah `1P4S` low-voltage module, 12,88 V, 2,94 kWh, 10–14,6 V, 1 C, 267 × 178 × 237 mm,
UN38.3 and UL2580 — a vehicle platform part, whose availability to a single station is a purchasing
question.

**Low-temperature cylindrical cells are not a pack of this size**: JYH's and Wiltson's cells are
1,5–4 Ah, so a 300 Ah string is a hundred cells built to order; a cell of that class, ~3 Ah, is the Mayak's
backup cell and stays there.

### The pack, mechanically

- **The frame follows the case.** The LiFeYPO₄ cell's plastic case stands on its own, banded in a
  row. An aluminium-can ESS cell needs **a steel frame**, preloaded at the sheets' 300 kgf and holding
  up to **50 kN of swelling force at 70 % SOH** — five tonnes across the stack, which no strap or
  plastic end-plate holds. A bought pack is a genuinely decent one, never the foam-filled commodity
  kind; the mechanics matter as much as the cells.
- **The BMS trips at a low current**, so a short cuts out before it becomes a fire.
- **Behind the BMS the 12 V goes to the enclosure's fuse field, one fast-acting cartridge to a
  board**, sized per install on the branch's input power and its thinnest wire
  (`../daedalus/CONSTRUCTION.md`, `../galvani/README.md`, *The rails*).

### The pack talks to the master — the BMS and the MPPT through Hermes

**Every pack has a BMS** — cell balancing, over- and under-voltage, over-current, the
low-temperature charge rule. **The BMS and the MPPT report to the head through Hermes** on the
head's LP I²C, the LP core's own bus, so the survival loop reads it (`../hermes/README.md`). Hermes
is a small H523 card whose standard face is **485 Modbus RTU on `THVD1450`**, what smart BMS and
MPPT controllers speak, with a plain UART as the second face; that bus and its supply stay on
Hermes's far side. **It is BMS and MPPT only, and it never leaves the enclosure** — no field sensor
hangs there; an external Modbus device reaches the station through a Galvani board like everything
else.

**The station names no BMS and no MPPT.** They are bought, by the builder, in whichever of the
arrangements `../hermes/FIRMWARE.md` §5 describes — a separate MPPT and a separate smart BMS, a hybrid
charger relaying the BMS, or a pack with its charger inside — and Hermes runs the map the builder
fills for them. What a unit must bring to be bought: **a digital face Hermes carries** — 485 Modbus
RTU, a UART with a documented frame, CAN or I²C; a unit with Bluetooth or a cloud alone is not one —
**the pack's voltage and state of charge readable on it**, and a protocol document the map can be
written from. Any supply or load output a unit puts on its connector is left unconnected.

**What a substitute is checked against, two things:** **it switches itself back on at its own
recharge threshold**, with no command and no processor alive — on most smart BMS a protection parameter
of the variant ordered — and its current rating stands over the station's ~5 A peak at 12 V; 50 A is
the common stock part, and the smallest variant serves. **The self-recovery is a selection
criterion, not a nicety**: the BMS drop is the station's last recovery — a hard reset of every board
at once, after which everything comes up from the beginning (`BRINGUP.md`) — and a BMS that latches
off until somebody presses a button turns a cloudy fortnight into a site visit.

**What they report**, folded into the head's state of health and the uplink: pack voltage, current
and state of charge, cell temperature, cycles and health; the solar input and the charge state
(charging, float, fault); the BMS's faults (imbalance, over-temperature, protection trips).

**It closes two loops.** **The server sees a station running low before it dies** — the state of
charge trending down through a cloudy week — so a trip or a load cut is planned. **And the power
drives the lockdown**: the head sees the deficit coming over Hermes and takes the station down
itself, before the BMS acts, so nothing is cut mid-write. **The head computes the capacity itself**
rather than trusting the BMS, since cheap modules differ in whether they track it: it polls once a
minute and integrates, and the station's steady draw is what makes that good enough. **The Hermes
link is never gated**, and in the deepest deficit the loop runs on the head's LP core alone
(`../mayak/FIRMWARE.md` §11). A deficit is not an outage — under cloud the panel still gives a few
watts, which holds the head — and what the head sends meanwhile is a low-rate distress frame with a
**reason code** (low irradiance, pack depleted), so a silent station and a station saying why it
went quiet are different events at the server. **The BMS is a single point of failure and stays
one**: no second BMS, no second pack tap.

### The lockdown — what is still running when the pack is nearly gone

**In normal running the spine stays up.** While anything is being recorded, **Kronos**, **the GNSS
source** — Sputnik or Polaris — **the Mayak** and **Hermes** are not switched off and not slept;
everything else is ended branch by branch as it is finished with. **Ending Sputnik ends its
measuring side only**: the TEC observables stop by register and its time relay to Kronos keeps
running.

**In the lockdown nothing is being recorded, and the spine goes too.** At 20 % the head takes the
station down in this order:

1. **The writes end hard.** Everything in the buffers is committed, overwriting if it comes to that
   — a lost sector is cheaper than a lost hour, and a store cut mid-write is worse than both.
2. **Everything behind a port goes off** — `ENABLE` down, branch by branch, Sputnik and Polaris with
   the rest. There is nothing being timed.
3. **Everything in the enclosure goes into its deep sleep** — Kronos and every card, on the Mayak's
   command, since a board on the battery wire has no `ENABLE` above it; each wakes on the link it
   already has (`PROTOCOL.md` §7).
4. **What is left is the Mayak's LP core and Hermes.** Big cores down, the HP domain gated, the LP
   core reading Hermes's block about once a minute and waiting on the state-of-charge threshold
   (`../mayak/FIRMWARE.md` §11). A distress frame with its reason code is the last thing out.

**What the enclosure still draws is not the processors.** Stop takes an H523 to **0,15–0,27 mA
typical at 25 °C** — LDO, SRAMs retained, by the voltage scale (DS14540 Rev 3, Table 33; 0,10 mA
with SRAM2 alone, 0,43–0,81 mA the 25 °C maximum) — and keeps every pin where it was left. **That
figure is leakage and climbs with heat: 4,0–6,4 mA maximum at T_J 85 °C.** Beside it stand
**Kronos's TCXO, ~20 mA, 66 mW, the largest single item**, and each card's two `THVD1450`
time-bus receivers, 1,4 mA, 4,6 mW, their `RE#` tied low because switching them would save nothing
worth a pin. **Four cards and Kronos sit at ~0,09 W**, of which the five processors are ~3 mW cool
and up to ~0,1 W at an 85 °C junction. **The TCXO has no off** — disabled, `MQF574T` still draws
18 mA of its 21 with the oscillator running — so ~0,09 W is the floor. **Standby would be ~3–5 µA**
(Table 34) and is not used: it buys forty-fold on a part that is not the bill, and it costs the wake.

**The way back is the BMS's own hysteresis, with no processor in it.** The recharge threshold stands
well above the cut — a couple of volts of pack, a set fraction of charge — so a pack just emptied is
not allowed to restart the station on the next hour of sun and be emptied again. The cut and the
recovery are BMS settings and the deployment's; the 80 % charge stop and the 20 % lockdown are the
station's.

**The station stands on two boards.** Kronos dead is the end of the timebase; the Mayak dead is the
end of the station — it stops reporting and somebody drives out. Nothing is redundant and nothing
votes: a second head is a second thing to keep in step, against a failure a technician resolves in
an afternoon. **So the survival argument rests on the head's firmware** — the ordered shutdown, the
sleeps, the coulomb estimate, the distress frame — with no hardware fallback but the BMS's drop, and
**the firmware is debugged before a station is left alone**: no untested path, no unhandled state.

## 4. Bury the electronics, expose the sensors — the answer to −40 and +60

**Burial is regional, chosen where heat or cold forces it.** Over about a third of the world — the
temperate zones — the whole station stands **above ground under a roof, in the reflective white
finish, IP68, the pack with it**; much of that land never sees −40. Where the temperature is the
problem, **the electronics and the pack go underground and only the sensors stay above**
(`../daedalus/CONSTRUCTION.md` holds the vault, the roof and the finish).

**The ground is a thermal buffer both ways.** At ~1 m depth the temperature follows the annual mean
with the seasonal swing strongly damped; a few metres down it is nearly constant, in seasonally
frozen ground and permafrost alike. So the buried pack and electronics never see −40 and never see
+60 — one move solves the cold-charge problem of the pack and the sun-baked enclosure of the
electronics, and the ground takes the waste heat. **In an insulated vault the pack's own losses and
the electronics' waste heat lift the inside above the surrounding ground**, with no heater.

**It suits the architecture**: Quake wants to be in the ground anyway — coupling to bedrock is the
measurement — and only the environment-facing heads stay out: the weather sensors, the radiation
heads, the antennas. **Self-heating never reaches an ambient sensor**: warming a GNSS receiver or an
accelerometer by its own draw is fine, warming the air-temperature and humidity sensors is wrong —
they read true ambient in the ventilated shield.

## 5. What burial asks for

- **Water is the first risk.** Buried is groundwater and condensation: a sealed **IP68 enclosure**
  with condensation managed — a pressure-equalising membrane and desiccant, or potting for what is
  never serviced. Water kills a buried box faster than any temperature.
- **A vault, not a grave.** The pack is the one consumable and the electronics may need a hand, so
  the vault is **lidded and reachable at grade**.
- **Frost heave.** The vault stands **below the local frost line**, and every cable has a service
  loop and strain relief so ground movement does not tension it.
- **The cable's transition zone** crosses from mild-and-wet to cold-and-UV and wants a **UV-stable
  jacket** and sealed glands at both ends. The bus cable is **outdoor UTP Cat 6** in a direct-burial
  jacket for the buried leg. **The feed runs beside it in its own 2-core cable, 2× 1,5 or 2× 2,5
  mm², 300/500 V** — CYSY (H05VV-F) indoors, and **H07RN-F or a direct-burial equivalent** on an
  exposed or buried leg, CYSY being PVC and neither UV-stable nor a burial type; the conductor size
  is chosen by the reach tables (`../galvani/README.md`, *Reach*).
- **Every antenna feeds down a UV-resistant coax**; the GNSS one is active, so the gain sits in the
  antenna and the coax loss does not eat the signal.

## 6. Temperature targets — who survives what

| part | where | target |
|---|---|---|
| sensors and heads — weather, radiation, antennas | exposed | **−40 to +60 °C** operating — the widest-range part sets it |
| units that carry a processor outside — Tesla's H7A3 at the antenna, `Quark-Tubes`'s H523 on the tubes, the scintillation boards' H7A3 in its −40 to +85 °C grade | exposed | **−40 to +60 °C** |
| the head, Kronos, the cards and the pack | the vault, or above ground where the site allows | a mild, stable band near the ground's annual mean, lifted by self-heat |

**The exposed processors are fine**, for three reasons: the H523 and the H7A3 grade used are
−40 to +85 °C parts; these units are fed over their run with no pack of their own, so the
charge-below-0 °C question does not exist there; and what actually protects them is **the
enclosure** — IP68 with condensation managed, under the reflective finish — since the outdoor killer
is thermal-cycling condensation, not the temperature. **The passives are derated for the range**:
X7R or X8R ceramics, 105 °C bulk. The −40/+60 figure is a *sensor* specification; burial keeps the
electronics and the storage out of it.

## 7. From the pack to the boards — no station bus voltage

```
FROM THE SUN TO A BOARD — and there is no station rail anywhere in it


   PV panel            the only thing left on the surface
      │
      ▼
    MPPT              bought, read over Modbus, never built
      │
      ▼
  pack + BMS ─── the fuse field, one fuse to a board ───▶  THE BATTERY AS IT IS, 12–13,5 V
      │                                                            │
      │                                                            ├──▶  MAYAK ....... its own terminals, its own buck
      │                                                            ├──▶  KRONOS ...... its own terminals, its own buck
      │                                                            ├──▶  every CARD .. its own terminals, its own bucks
      │                                                            ├──▶  PALATINE, SPUTNIK, where they share the enclosure
      │                                                            │
      │                                                            └──▶  A SOURCE POWER BOARD on a host's port,
      │                                                                  on its own terminals
      │                                                                           │
      │                                                                    the feed cell
      │                                                                           │
      │                                                                           ═══▶ 48 V or 300 V, isolated,
      │                                                                                ONE per run, only ever on a run
      │
      └─── BMS + MPPT, on their own bus ──▶ Hermes ──▶ the head's LP I²C, never gated:
                                 V · I · SoC · cell temperature · charge state · faults.


   AT THE FAR END OF A RUN

     48 V or 300 V ──▶ the unit power board's island converter ──▶ 12 V ──▶ the unit, on its terminals;
                       the unit makes 3,3 V and, where its LDOs want it, 4,0 V on its own bucks,
                       side by side, never in cascade; a true-5 V board makes its 5 V from the 12 V too
```

**Inside the enclosure the 12 V is the battery, as it is.** No fixed rail and no regulation on the
wire: every board takes it on its own two terminals behind its own fuse and makes its own rails on a
wide-input buck, so nothing between the pack and the boards has to care what the pack is doing.
**There is no supply board in the box**, and no board carries 12 V across itself for another
(`../galvani/README.md`, *The rails*).

**A made voltage exists in one place: on a run that leaves the station** — 48 V, or 300 V where 48
does not carry the load that far, made by a Galvani source power board on the host's port and taken
off the cable by a unit power board at the far end, in a 2-core cable of its own. **48 V is the
number** — the telecom standard with the cheapest parts ecosystem, and a feed the station makes
itself, so its protection is dimensioned for 48 V and nothing else. **300 V is the other number**,
one nominal, picked by watts × distance. Which carries how far on which cross-section is
`../galvani/README.md`'s; so is every board.

**A regulated 12 V exists only as a unit power board's island output or as `G-12-S`'s isolated
output**, and it is what a bought Modbus sensor runs on: many will not run below 9 V and waste power
at 15, and the common windows on a bought part — 10–30 V, 12–24 V, 5–24 V — all hold 12. A part that
cannot run there is not the part. `G-12-S` is unregulated off the battery through its `SN6507`,
inside every one of those windows (`../galvani/README.md`). **No other voltage is handed to a
board**: Pluvius's head takes 24 V from `G-24-S`, a source power board on Palatine's `PWR EXT`,
switched by `ENABLE` and standing off between runs — a load, not a rail.

## 8. Grounding and lightning — everything that leaves the box is isolated

**The station is protected against the conducted and induced surge, the 8/20 µs class, not the
direct strike**, and it is sited out of the zone of direct strike; the rule, the common earthing
point, the mast's conduits and the three legs — insulation, isolation, entry arresters — are
`../daedalus/CONSTRUCTION.md`'s, the per-conductor ladder `../galvani/README.md`'s. What this
document takes from them:

- **Inside or outside, never distance.** Anything leaving the enclosure is treated as far, mast runs
  included: a strike raises the mast by hundreds of kilovolts in about a microsecond, and a cable
  routed through it couples `I = C·dV/dt` — ten picofarads against 10¹¹ V/s is an ampere — so three
  metres inside a mast is a worse position than five hundred buried. The mast's delivery is
  charge-limited, tens of µC, which is why entry treatment suffices there, while the tube ladder
  stays at the source end where surge energy can arrive galvanically.
- **A unit end floats.** Isolation is kept at the station-to-field boundary — the isolated
  transceivers and the unit power board's island — and the unit end carries the transil alone, no
  tube and no choke. A strike on the long cable has no galvanic path into the station.
- **A remote site is not earthed, on purpose.** A proper earth at every position is rods, strap,
  ground work and a check, and a bad one is worse than none: a metre of strap is ~1 µH, 1250 V at a
  lightning front. The run itself is the better limiter — a twisted pair's surge impedance of
  ~110 Ω drives ~90 A from a 10 kV front, and a kilometre of cable puts 600 µH and 24 Ω in the path.
- **One earth, at the station**: the common earthing point, where every SPD common and the internal
  system's bonding conductor land.
- **A unit's own sensor leads are the one exception** to "everything that leaves is isolated": the
  GNSS coax and a unit's sensor heads cross without a barrier and take entry treatment instead.

## Every low-rail capacitor in the station is a 50 V part

**One voltage rating on every low rail, and it is 50 V**; the cable-side bulk on a Galvani power
board is the exception, rated at its own clamp (`../galvani/HARDWARE.md`). No rail needs it — the
battery tops out near 15 V and the low rails are single digits — but **one rating is one stock line,
one footprint family and one part to substitute**, and the margin is free at these values. A builder
who wants 25 V parts on a 3,3 V rail may fit them; the documents specify 50.

**It is not free in capacitance.** A ceramic loses value to DC bias, and a 10 µF X7R in that class
can sit near half its marking at 12 V. **The nominal figures in the board documents are deliberately
generous**, and a part is chosen on its bias curve rather than its printed value.

## Three resistor values hold a line, and every board uses the same three

| value | where | why this value |
|---|---|---|
| **4,7 kΩ** | every I²C pull-up, one pair per bus at its controller and one pair on each side of an isolator; a 1-Wire pull-up | the largest standard value that meets the rise time at the station's rates: 100 kHz on a 30–40 cm ribbon with its taps (~150 pF, 0,6 µs of the 1 µs allowed) and 400 kHz on a board (~50 pF, 0,2 µs of 0,3 µs) |
| **10 kΩ** | the `ID` reference on every Galvani socket; `BOOT0` to ground on every STM32 | the `ID` reference is not a pull: it is the scale the whole `ID` table was computed against (`../galvani/README.md`), and changing it reissues every code. `BOOT0` is sampled at reset beside a switching node, and a false bootloader entry is a board that does not start |
| **100 kΩ** | every pin held in a state while the processor is in reset — an `EN`, an `OE`, an `ENABLE` | a hold, not a drive: the GPIO overrides it, and a driven-high pin burns 33 µA against it instead of 330 |

The one exception is functional, not a pull: the 330 Ω pads on the Mayak's trunk `RXD` lines, the
multipoint build's wired-OR, unfitted on every board (`../mayak/HARDWARE.md`).

## The filter parts — the shielded 2,2 µH, and a bead behind it where a converter's interface runs fast

| part | where | why |
|---|---|---|
| **inductor `SWPA252012S2R2MT`** — 2,2 µH, shielded, 2,5 × 2,0 × 1,2 mm, `DCR` 0,17 Ω, `SRF` 69 MHz, `I_SAT` 1,85 A; JLCPCB `C23894` | **every filtered supply position in the station**: a measuring board's per-consumer branch, the MCU's `VDDA`/`VREF+`, a converter's digital supply, a TCXO, a channel split behind an LDO, an optical module's `VccT` and `VccR`, the RM3100's `AVDD` and `DVDD` — **always a π filter: 10 µF 50 V X7R 1206 + 100 nF 50 V X7R 0603 on BOTH sides of the inductor — before it on the rail and behind it at the pins — on every position, nothing smaller and no position without**; its `DCR` damps the pair to a Q of ~3–4 and is the whole DC drop, 0,17 Ω | the noise on every rail in the station is a buck's own 1–2,7 MHz first, and there a ferrite bead is a few ohms and filters nothing; 2,2 µH into 10 µF is a ~34 kHz corner and ~70 dB at 2,1 MHz |
| **ferrite bead `GZ2012D301TF`** — Sunlord, 0805, 300 Ω ±25 % at 100 MHz, `DCR` 0,20 Ω max, 500 mA | **behind an LDO on a fast front's analogue supply — the `AD9251`'s and `AD9265`'s `AVDD`, the `THS4551`, `THS4541`, `LMH5401` and `LTC6268` supply pins on the scintillation boards and Marconi — LDO → its output capacitor + 100 nF → bead → 100 nF at the pin**, where the LDO rejects 45 dB at 1 MHz and less above; and **behind the inductor, never in its place, where a converter's interface runs at tens of MHz**: the `AD9251`'s `DRVDD` on `Quark-Photon` and `Quark-Neutron/Positron` (words at 88 MHz, so a data line's fundamental is at most 44 MHz; ≤ 11 mA), the `AD9265`'s `DRVDD` on Marconi (67 MHz words, ≤ 34 MHz a line; 14 mA in CMOS at 80 MSPS), the `ADS127L14`'s `IOVDD` and `AVDD2` on Tesla (33 MHz clock; ≤ 5 mA and ≤ 21 mA) — **2,2 µH → 10 µF + 100 nF → bead → 100 nF at the pin** | the fundamentals sit under the inductor's 69 MHz self-resonance, but the edges' harmonics — the third of 44 MHz is 132 — sit above it, where the inductor is a capacitor; the bead is a resistor there and burns them, and its flux stays inside its ferrite. **A bead has 100 nF on both sides**: what it turns back needs a low node on its near side, and at 100 MHz a 10 µF is its own ESL and the inductor past its self-resonance a capacitor that hands the harmonics on up the rail. At ≤ 21 mA against its 500 mA it drops ≤ 4,2 mV and keeps its impedance; 300 Ω is enough against 100 nF, which is ~0,016 Ω at 100 MHz |

**No RC anywhere.** A magnetometer takes no magnetic part beside its coils, so its inductors stand
at the buck, a board's or a tube's length away, and only ceramic capacitors sit beside the sensor. A
slow converter's interface — Pluvius's `ADS1235` — takes the inductor alone. No other filter part is
specified; a position that is not one of these is a position to delete.

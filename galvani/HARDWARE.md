★ N.I.C. ★

# Galvani — the parts

> **Design-stage concept — nothing built.** The figures are verified against the parts' sheets and
> the parts before a board is made.

The component layer of the family: which chip sits in which position, the windings, the
rectifiers, the protection ladder's parts and values, the worksheets, the optics, and each board's
parts. What the family is, which board goes where and what each may carry is `README.md`, which
wins on any conflict. A number here is read from a datasheet or computed.

## The converters — battery to position

| supply, from the battery, for | the chip | note |
|---|---|---|
| **every 1 A buck position** — a unit's own 3,3 V and, where its sensor LDOs want it, 4,0 V from the 12 V it is handed; a board's own logic; the head's ESP (Wi-Fi TX peaks 295 mA, the source sized for 0,6 A); the UM980; every MOD, behind transils clamping at ~29 V | **`LMR43610R3RPER`, 1 A — the house buck** | one converter, no rail, no LDO; one part covers 20 mA and 700 alike, and **one order code covers every frequency** because the `R_T` resistor sets it (200 kHz – 2,2 MHz). **`R`-trim, spread spectrum OFF from the factory, no `SYNC` pin, `PGOOD` present.** The full cell is *The buck cell*, worksheet below |
| **the 48 V onto the cable** | **LT3748 + the `ISC165N15NM6` + 750310988** (1:4,42, 14 µH) | the battery-side feed cell — worksheet below; **`G-48-S` carries it** |
| **the 300 V onto the cable** | **LT3748 + the `ISC165N15NM6` + 750310349** (1:10, 5 µH) | **`G-300-S` carries it — ~47 W, one number.** Its input is 12 V at a station and 12 V at a remote Argus, so nothing about the board changes between them; what a port hands out is a programmed `INA238` threshold |
| **the card's four down ports' 3,3 V** — 12 V → 3,3 V for the boards on the four down ports, its `EN` on no pin; the processor and the up port take a `TPS629206` each. **The down branch carries ~420 mA with four optical ends, 700 mA at their ceiling, ~300 mA with four copper** (*The fibre front*; *What a communication board costs the host's 3,3 V*) | **`LMR43620`, 2 A — the same cell one size up** | the same inductor value as the 1 A design (3,3–4,7 µH) with `I_SAT` ≥ 3,5 A, so light-load behaviour is the 1 A part's and the reserve is real (*The buck cell*) |
| **a rail up to 600 mA on a 12 V that stays under 17 V** — the card's processor and up port; a unit whose whole board fits it (Quake); **both rails of every H7A3 board**, the processor's 1,8 V and the analogue feed — never on a MOD, whose 12 V sits behind transils clamping at ~29 V | **`TPS629206`, 600 mA** | 3–17 V in, `V_OUT` 0,6–5,5 V by divider against 0,6 V, 2,5 or 1 MHz and forced PWM or auto by one resistor on `MODE/S-CONF`, 40 ns minimum on-time, 1,1–1,7 A high-side limit, `PG`, SOT-5X3 — pin-to-pin the `TPS629203` and `TPS629210` (*The buck cell*) |
| **the lasers** (`G-O-10-10` · `G-O-2-100`) | **the host's 3,3 V through the module's own π filter — no buck, no LDO** | the TX half is digitally keyed, so ripple does not reach anything that cares and an LDO there would only burn watts; the RX half is the one that has to *hear*, so it takes its 3,3 V through the module's own π filter on the host's rail. The seat carries local bulk. **The draw is ~105 mA at a sending end and ~55 mA at a receiving end on average, 175 and 125 mA at most** — every module this project fits reads `I_TX + I_RX` = 100 mA on its own sheet, the receiver ~25 mA and the laser ~75 of it, and the data laser is dark between slots (*The fibre front*) |
| **the 300 V off the cable** | **`LT8316` + `FCD260N65S3`** + `11328-T078` (8:1:1, 670 µH) or `11338-T195` (14:1:1,7, 1000 µH) → 12 V | **the `G-300-U` pair carries it — 40 W on `G-300-U-40`, 6 W on `G-300-U-6`.** `G-300-U` without a suffix means both: what the two boards share is everything but the winding, the shunt and the rectifier |
| **the 485 line side** (`G-I-N-025`, `G-I-M-005`) | **the host's 3,3 V over the connector** | the isolated line-side copy: **SN6505B (its own 420 kHz oscillator — no clock, no sync, nothing to set) + a WE-PP centre-tap transformer + Schottky rectifier + capacitors, NO LDO** — the raw rectified rail wanders a few hundred mV with load and diode drops, and the ISO145x VCC2 window (3–5,5 V) eats that whole; the regulating already happened at the buck-fed 3,3 V input. The winding is **Würth 750313734** — the sheet's own *3,3 V → 3,3 V, 100 mA, no LDO* row: 1:1,1 ±2 %, V×T **7 Vµs**, **5000 VRMS**, and at 420 kHz it uses 3,9 Vµs of that 7 — one winding on every island in the family |
| **the telemetry's measuring side** — every power board | **the host's 3,3 V over the power connector, through the same island** | `SN6505B` + `750313734` + 2× `PMEG10020ELR` make the isolated 3,3 V the `INA238` and the far side of the `ISO1642` run on — ~3 mA, always on (*The measuring-side island*) |

**The 150 V FET is the `ISC165N15NM6`** (Infineon OptiMOS 6, PG-TDSON-8) — one part on every
`LT3748` position. What carries it: **150 V**, 100 % avalanche tested (`E_AS` 123 mJ, `I_AS`
16 A) behind the snubber; **`R_DS(on)` 20,1 mΩ max at `V_GS` = 8 V** — the row that matters,
the LT3748 drives a ~7 V class gate — worth ~0,7 W at the worst cell (`G-300-S`,
`I_pk` 11,8 A), nothing against the board powers; and **`Q_G` 14,8 nC**, which is the
figure that was actually shopped for: at `G-48-U`'s 752 kHz the gate costs ~80 mW from the
LT3748's own supply, and the half-resistance parts carry more than double the charge at twice
the price — the resistance is cheap here and the charge is not. `I_D` 50 A / 200 A pulse,
`V_GS(th)` 3–4 V (normal level, fully driven).

## The battery — what the numbers are actually taken at

**The pack is LiFePO4 and its working range is 12–13,5 V.** Lithium-titanate lands in the same
place. **The 11–15 V written across this file is a design reserve, not an operating range**, and
the two must not be confused when a figure is read:

| | |
|---|---|
| **12 – 13,5 V** | **where the station actually sits.** Size for margin here |
| **11 – 15 V** | the reserve the design tolerates, and **that is the whole reserve.** A chemistry with a wide spread fits only where its BMS holds it inside — a 4S Na-ion pack in its 20–80 % window does (`../core/POWER.md`) — and widening the window to take one turns every converter in the family into a bench supply |

**Three things follow from stating it.**

- **`G-48-S` delivers ~26 W at the real low corner**, and ~25 W even at the 11 V reserve floor —
  `I_pk` 9,90 A at `R_SENSE` 9,1 mΩ, 28,4 W in, 2,4 W of loss across the shunt, both windings, the
  rectifier, the switch and the leakage. **A 3 W measuring unit therefore sits eight times
  under the cell**, and a far end at ~21–24 W just inside it.
- **`G-300-S`'s input window stops being special.** It was written as *12–13,5 V, narrower than the
  family's* — that is now **the family's own window**, and the catalog 40 µH sits inside its
  recommended band for the same reason every other position does.
- **A reserve corner is still a rating corner.** Nothing is re-specified to 12 V: the switches,
  the clamps and the windings still have to survive 11 V and 15 V. What changes is which number a
  reader treats as the design point.

## What each cell carries, and what stops it

**Every ceiling in this family is a switch, a programmed limit or a winding — never an output
voltage.** That is the row to read before sizing anything, because the mistake it prevents is
assuming that going to 300 V buys power: it does not. **Voltage buys reach; power costs a
different converter.**

| board | the cell | **what it delivers** | **what stops it** |
|---|---|---|---|
| **every buck** | `LMR43610` (1 A) · `LMR43620` (2 A, the card's port rail) · `TPS629206` (600 mA, the small rail and the H7A3 boards) | the part's rating, `V_OUT ≤ 0.95·V_IN` | **the part's own current.** One buck per rail; **40–50 % of the rating on average, 70 % at the ceiling** (*The low rails*) |
| **`G-48-S`** | **`LT3748` + the `ISC165N15NM6`** + **750310988**, 1:4,42, 14 µH, `R_SENSE` **9,1 mΩ** | **~26 W delivered** — `I_pk` **9,90 A** at the guaranteed 90 mV threshold, 41 kHz at full load, 393 kHz on a 3 W leaf | **the winding, and it is now the wall.** `I_SAT` **15 A min**: the 110 mV threshold corner lands at 12,1 A, **81 % of saturation**, and that is what fixes the shunt |
| **`G-48-U`** | **`LT3748` + the `ISC165N15NM6`** + **750311607**, 2,5:1, 14 µH, `R_SENSE` **15 mΩ** | **~25 W at 12 V out** — `I_pk` **3,00 A**, 455 kHz at the rated point; the shunt allows 6,0 A at 90 mV, ~50 W | **the source cell, not this board.** The shunt is fixed by the winding rule (*The one rule that screens a transformer*); its 110 mV corner is 7,33 A, **77 %** of the 9,5 A saturation. `V_IN(MAX)` is **50 V**, not 75 — the feed is a regulated 48 V. The winding does not set the output voltage, `R_FB`/`R_REF` do |
| **`G-300-S`** | **`LT3748` + the `ISC165N15NM6`** + **750310349**, 1:10, 5 µH, `R_SENSE` **5,5 mΩ** | 300 V out, **~47 W declared** — `I_pk` **11,8 A**, 145 kHz, 50,6 W in and 3,0 W of loss, held there by the `INA238` threshold; the shunt allows 16,4 A at the guaranteed 90 mV, ~66 W. **One number at both ends**: the input is 12 V at a station and 12 V at a remote Argus | **the winding rule** (*The one rule that screens a transformer*): 4,41 µH needed at 5,5 mΩ against 4,5 µH at the winding's low corner. The 110 mV corner is 20,0 A, **80 % of the 25 A typical saturation** |
| **`G-300-U-40`** | `LT8316` + `FCD260N65S3` + **11328-T078**, 8:1:1 (`N_TS` = 1), 670 µH, `R_SNS` **58 mΩ** | **40 W at 12 V declared**, 91 kHz — the 12 V bus of a remote Argus, or any load of that size (a heated unit, a camera — examples, not units of the station) | **the station, not this board.** `G-300-S` delivers ~47 W and this board's own loss is 3,7 W of it, so 43 W reaches the far end at 12 V and 41 W at the 11 V floor; 40 W is declared under both |
| **`G-300-U-6`** | `LT8316` + `FCD260N65S3` + **11338-T195**, 14:1:1,7, 1000 µH, `R_SNS` **130 mΩ** | **6 W at 12 V** — a pod, where a 40 mm pipe is the volume. **140 kHz**, discontinuous, at full load | **the enclosure.** Electrically `G-300-U-40` would do it; 8,6 mm of height against 23,5 is why this board exists |
| **`G-24-S`** | **`LT3748` + the `ISC165N15NM6`** + **750311592**, 1:1, 8 µH, `R_SENSE` **10 mΩ** | **~33 W delivered** at 24 V — `I_pk` **9,0 A** at 90 mV, 112 kHz at full load, ~166 kHz under a 22 W load; **switched by `ENABLE`**, off between runs | **the sampling rule, not the winding.** At 24,7 V reflected the 400 ns window wants `L ≥ 6.6 µH` at this shunt and the winding's −10 % corner is 7,2; a smaller shunt would want more. `I_SAT` 18 A: the 110 mV corner is 11,0 A, **61 %** |
| **`G-I-N-025`** 485 line side | `SN6505B` + 750313734, 1:1,1 | **~100 mA** of isolated 3,3 V, no LDO | the winding's **7 Vµs**, of which 420 kHz uses 3,9. Barrier **5000 V_RMS** |
| **`G-12-S`** isolated 12 V, source | `SN6507` + 750319691, 1,13:1 + `INA238` · `ISO1642` **on the output** | **0,4 A** at ~12 V — the pack through the winding, **unregulated**, so ~10,7 V at the 11 V reserve floor and ~16,2 V at the 15 V ceiling under light load | **`R_LIM` 34,8 kΩ → ~0,7 A** (*`G-12-S`*, below). **The `SN6507`'s OCP shortens the pulses, it does not switch off**, so a limit above the switches' rating is sustained until the die overheats. The rate has a floor too: `f_min ≥ V_IN(max)/(2·V-t)` = **1,0 MHz** at the part's slowest, so `R_CLK` = 7,87 kΩ → ~1,26 MHz typical, ≥ 1,07 MHz |

**Three consequences worth having in one place:**

- **300 V does not buy watts.** `G-48-S` and `G-300-S` are the same switch, so an installation that needs
  more power than the source cell makes needs **a different feed cell**, whatever voltage it
  runs at. What 300 V buys is `P_max = V₀²/4R` on the cable — reach, and load *at* reach.
- **The spur's 3 W is upstream of every unit-end number.** `G-48-U`'s island is bigger than the
  spur; sizing it up changes nothing until the feed changes.
- **`G-12-S` is the only unregulated output in the family**, and every rating behind it — the 18 V transil,
  the rectifier, the bought device's own window — is taken at the **light-load full-pack corner**
  and not at the nominal, because that is the corner a port sits at most of its life.

### The rectifiers — one rule

**A fast diode with the lowest forward drop the position allows, picked by reverse leakage and
deliberately over-rated on voltage** — Schottky wherever the voltage lets it be one, which is
every position but `G-300-S`'s 300 V. Forward drop is not what decides here: every rectifier in this family sits on an always-on
branch, so its reverse leakage is a *standing* loss — it climbs with temperature and with how
close the part runs to its rating, and on an island it is drawn from the same small budget as the
island's own load.
Buying more volts than the position needs buys the leakage down, and the extra drop is paid at
a current these positions never reach.

**Four order codes cover all six positions:**

| position | reverse stress | average current | the part |
|---|---|---|---|
| **the 485 crossing** | same relation at N = 1,1 and 3,3 V in → **11 V** | ~57 mA at a head port, ~5 mA at the far end → **~30 mA a diode** | **PMEG10020ELR** *(PMEG10030ELP · PMEG100T50ELP · PMEG100V060ELPD)* |
| **`G-48-U`** island output | `V_OUT + V_IN/N` = 12 + 50/2,5 = **32 V** | 24 W at 12 V → **2,0 A** average, the secondary peak `N · I_pk` = 2,5 × 3,00 = 7,5 A | **`V10P10-M3`** — 100 V / 10 A. **The over-rating is what kills the leakage**: a 100 V die at 32 V of reverse stress leaks a fraction of its datasheet figure, and that is the answer to a Schottky at 100 °C |
| **`G-300-U-40`** island output | 12 + 300/8 = **50 V** | 40 W at 12 V → **3,7 A** average | **`V10P10-M3`**, 100 V / 10 A — 37 % loaded. **It is the board's biggest single loss, 2,4 W of 3,7**, and the only way past it is a higher output voltage, which this position does not have |
| **`G-300-U-6`** island output | 12 + 300/14 = **33 V** | 6 W at 12 V → **0,5 A** average, 4,6 A secondary peak | **`V10P10-M3`**, 100 V / 10 A |
| **`G-48-S`** feed cell output | `PIV = V_OUT + N·V_IN` = 48 + 4,42 × 15 = **114 V** | 26 W at 48 V → **~0,54 A** | **`V5N22-M3`**, 220 V / 5 A |
| **`G-300-S`** feed cell output | `PIV` = **435–450 V at the 300 V feed** | 47 W at 300 V → **~0,16 A** | **`US3M`**, a 1000 V ultrafast part — the family's rectifier on 300 V, and its blocking diode on every feed |

| part | ratings | why it sits where it does |
|---|---|---|
| **PMEG10020ELR** — the recommended part | Nexperia, **100 V**, the *low-leakage* line | **one part for every low-voltage position.** 100 V against stresses of 11–51 V is the over-rating the rule asks for |
| *the interchangeable grades:* **PMEG10030ELP · PMEG100T50ELP · PMEG100V060ELPD** | same 100 V, same family, ascending ampere grade; the **V060** is the dual | **any of them drops into any of these positions.** The recommendation is the lowest grade that clears the fault metric, because a bigger die tends to leak more at the same voltage — a build that wants margin, or that already stocks one of the others, fits it and nothing else changes |
| **`V5N22-M3`** | Vishay TMBS eSMP, Schottky, **220 V, 5 A** | the feed cell's 114 V of reverse stress with room; the 100 V parts would not carry that margin, and five amperes cost nothing here |
| **`V10P10-M3`** | Vishay TMBS, Schottky, **100 V / 10 A** | **the one grade on every 12 V and 24 V output** — `G-48-U`, `G-300-U-6`, `G-300-U-40`, `G-24-S`. At the same current the larger die drops less than a 3 A part — 0,45 V at 5 A — so it lowers the loss where the current is — by ~0,4 W on `G-48-U` at 2 A, ~0,25 W on `G-24-S`, ~0,06 W on `G-300-U-6`. The voltage grade is bought for the leakage, not the stress: a Schottky's reverse current doubles every 10 °C and a die at a third of its rating leaks a fraction of the datasheet figure |
| **`US3M`** | Nexperia, **1000 V / 3 A ultrafast**, `t_rr` ≤ 75 ns, `I_FSM` 100 A, SMC | `G-300-S`'s 435 V worst case with room, at 0,16 A; and the blocking diode on every feed, where the surge rating is what it is for (*The protection ladder*) |

**Volts and amperes are bought by opposite rules here:** over-rate the *voltage*, because
leakage falls as a part runs further from its limit; buy the *current* grade to the load,
because a bigger die tends to leak more at the same voltage.

**What a short does — the ampere grade is sized to the fault, not to the load.** Nothing here
is fused on the secondary side; what bounds a shorted output is the converter above it, and
each one bounds it differently:

| position | what caps the fault | secondary sees |
|---|---|---|
| **the 485 crossing** | the driver's own 1 A class against an 11 V, 30 mA position | never near the part |
| **`G-48-U`** island | **`I_DIODE(MAX) = I_SW(MAX)·N_PS`, and `I_SW(MAX)` is the shunt** rather than a chip ceiling: 110 mV / 15 mΩ = **7,33 A** at the threshold's hot corner, times 2,5:1. `LT3748` has no output-current pin — the fault is bounded by the peak-current limit and the winding, which is why the declared rating is set at the shunt (*`G-48-U`*) | **18,3 A** of secondary peak, carried by `V10P10-M3`'s surge rating until the `INA238`'s `ALERT` trips the source; its 10 A average rating never sees it |
| **`G-48-S`** feed cell | the shunt: `I_pk` **9,90 A** at the guaranteed 90 mV, **12,1 A** at the 110 mV hot corner, then foldback | the 220 V Schottky, `V5N22-M3`, is picked against it (*Switch and rectifier stress*) |
| **`G-300-U-40`** island | **we set it, and the part regulates into a short rather than running away** — `LT8316` is constant-current as well as constant-voltage. `R_SNS` sets the *switch* peak; the **output**-current regulation point is `IREG/SS`, one resistor to ground, which is how one sheet's shunt and another's declared amperes stay separate things | **`R_SNS` = 58 mΩ, 8:1** gives `I_pk` 1,55 A at 90 mV and 1,90 A at the 110 mV corner, 63 % of the winding's 3,0 A; the output's own limit is `IREG/SS`: **76,8 kΩ → 4,24 A**, 127 % of the 3,33 A declared, from `R_IREG/SS = 2,5 MΩ · I_OUT · R_SNS / N_PS` (the `LT8316` sheet, *Output Current Regulation*), which asks 120–150 % of the maximum load so the current loop never meets the voltage loop |

**The margin against a real fault is not in the diode's ampere grade but in the converters' own
limits**, read above.

**The leakage figures are the selection criterion, and only the datasheets carry them across
temperature.** The families are the right ones (Nexperia's low-leakage PMEG line, Vishay's
TMBS), and each part's headline ratings clear its position; I_R at temperature is what the
bench sees anyway.

## The unit board's 12 V terminal — the island's raw output, ahead of everything

**Both unit boards hand their 12 V out on their own two-pole terminals, and on nothing else — the
connector carries no 12 V pin.** Two terminals for 2,5 mm², an input and a tap, the one block the
family uses on every terminal position — Degson `DGPS2.5R-5.0`, order code `10060008518`, a PCB
PUSH-SNAP spring block, two poles: 5 mm pitch, 0,75–2,5 mm² solid or flexible, 20 A, 400 V (III/2),
4 kV rated surge, strip 9 mm, −40…+105 °C (`README.md`, *The connectors*); a station board taps
the 12 V wire on the same block and puts a made voltage on its cable — the same block again, two
pairs, on every feed position of the family. **Its 400 V is a working classification, not a
breakdown**: the 300 V feed's ~565 V clamp is met by the 4 kV surge the part is rated for, and the
tube and the transil ahead of it take the surge. **A field conductor enters facing the gland** —
nothing in the family takes its field cable from above.

**The data pairs take the same block**, four of them on an eight-pole position: 0,75–2,5 mm² is
the listed range, and every conductor in the family goes in bare — no ferrule, no sibling, one
block on every position of the station.

**One pole to a net, and a run that carries on is twisted into the pole it shares** — the two
conductors stripped together and pushed in as one, which is how a field terminal is wired
everywhere else and is why nothing is doubled for chaining. The count is what the cable brings:
**8 on `G-I-N-025`** (the four pairs), **8 on `G-I-M-005`** (four positions, 4× `A`, 4× `B`),
**8 on `G-12-S`** (four positions, 4× +, 4× −), **4 on a MOD**
(`A` · `B` · 12 V · GND), **4 on every board that carries 12 V** and **4 on a feed position**.
**The four positions on an arm board are the star of four sensor cables**, because a bought
sensor comes with its own cable and cannot be given a second; past four they chain at each
other's terminals. **Board-to-board communication is not on a terminal at all** — it is on the
2,54 mm IDC bodies, which is why a card carries four poles and nothing else.

**One use, one terminal.**

| | |
|---|---|
| **a bought device** — a ModBus part on an arm at a fed position | takes the 12 V as it stands; the common supply window on a bought part is 10–30 V, 12–24 V or 5–24 V. Bounded by the island's programmed current limit and by the run's budget — the device, the unit board and every Galvani beside them are one load on one run |
| **the fault bound** | the island's current limit. A tapped load that peaks above it browns out the measurement it exists to serve, so **a load with an inrush needs it bounded** |

**Nothing switches the tap on the board.** Every branch in the family is switched at its supply —
the port's `ENABLE` on the source board — and a load that needs switching of its own has its own
supply board, `G-24-S` on the host's dedicated power socket.

## The protection ladder — the parts

**A blocking diode sits between the cell's bulk and the transil.** Winding → rectifier → bulk →
**blocking diode** → transil: the diode fixes the transil node's polarity (a unipolar part is
enough) and blocks the bulk and the converter from feeding a line fault. **It is `US3M` on every
feed — Nexperia, 1000 V / 3 A ultrafast, `t_rr` ≤ 75 ns, `I_FSM` 100 A, SMC.** In reverse it
stands the difference between what the line does and the bulk behind it: the tube's let-through,
525 V at `2036-07-SM` and 650 V at `2036-30-SM`, less the bulk's 48 or 300 V — at most ~480 V, half
its rating.

**What it carries is the bulk discharging into a fired tube.** When the tube strikes, the bulk
empties forward through the diode and the two 22 µH chokes into the arc:

| | the bulk | the peak, `V / √(L/C)` | lasting | energy |
|---|---|---|---|---|
| `G-300-S` | 4× 100 nF, ~0,4 µF at 300 V | **~29 A** | ~13 µs | ~18 mJ |
| `G-48-S` | ~7 µF effective at 48 V | **~19 A** | ~55 µs | ~8 mJ |

and on 300 V, where the tube does not quench, the converter's current-limited output follows —
~1 A peak — until `ALERT` and `ENABLE` take the feed away, milliseconds. **The peak is under a
third of the part's 100 A surge rating and the follow current a third of its 3 A**, which is why the
3 A part and not a 1 A one. Cost: one `V_F` ≈ 1–1,7 V, ~2 % at 48 V, 0,6 % at 300 V — affordable
only because the rail is high, which is why the low rails do not carry it.

**A run has at most one earth bond, and it is the three-electrode tube's centre.** The feed is
isolated end to end — winding, cable, far end. The three-electrode tube passes one gap per leg with
its common centre to the single-point earth; the gaps' leakages are the symmetriser, so the float
settles at half the rail per gap and a strike in one gap ionises the shared chamber and takes the
other, leaving no gap holding the full rail.

**"At most" is why a unit end carries no tube at all.** Its centre would float to mid-rail and a
differential surge would have to break both gaps in series, doubling the let-through for no
benefit — and a twisted pair leaves almost no loop, so what arrives there is common mode, which at
a floating end has no path. What is left is the differential residue, and that is the transil's.

**A 300 V feed carries the same chain one grade up, and the tube's ignition sets the grades.**
The tube reaches its impulse sparkover — hundreds of volts above the DC figure — before it fires,
and that number, not the 300 V rail, is what the parts behind it must stand: the
**blocking diode is the same `US3M`**, its 1000 V against the ~350 V the let-through stands over
the bulk, and the **chokes are the heavy element** — they drop the tube's let-through, so on a 300 V feed they are sized for the
650 V across them and the surge current through them (insulation and I_SAT, not just inductance). The tube is
the three-electrode form through the common earthing point; the float halves to ±100–150 V per gap. One
argument does not carry over: no tube's holdover sits above a 300 V rail, so **extinction
on a 300 V feed rests on the source converter's current limit**, not on the tube.

**The order is the whole thing: GDT → chokes → transil, reading from the cable.** The chokes
sit *between* the tube and the clamp and never beside them. A transil next to a GDT is a
transil that eats the tube's whole let-through; the chokes are what drop that let-through
across an impedance instead, so the transil only has to hold the remainder. Which is also why
the chokes are pointless on their own — without a crowbar outside them there is nothing to
decouple from.

**Two Bourns SMD families cover every position in the project, and the family is the electrode
count:** **`2036-xx-SM`** (Mini-TRIGARD) is the **three-electrode** part and goes **through the
common earthing point**, and it is the only gas tube in the station. What changes between positions
is the voltage code, and what sets it is that **each gap of the three-electrode tube sees half the
rail**. The whole ladder is SMD (*The surge path*).

**Two numbers qualify a tube for a DC feed, and they are not the same one** — the figures below are
`2036-07-SM`, which carries every 48 V and data position:

| | the 75 V codes | what it means here |
|---|---|---|
| minimum DC sparkover, through service life | **60 V** | above the 48 V feed, so it never fires in service |
| **DC holdover** (ITU-T K.12) | **52 V**, < 150 ms | **the one that actually binds a DC line** — once struck, the tube must extinguish, and it does only if the line sits below this. 48 V does |
| maximum impulse sparkover at 5 kV/µs | **500 V** | what the transil behind it has to hold |

That holdover figure is why the feed voltage and the tube are one choice and not two: a feed
above ~52 V would leave the arc lit on the station's own supply.

**The 300 V code, read from the sheet:** **`2036-30-SM`** through the common earthing point — 300 V ±20 %,
240 V minimum against 150 V a gap, **1,6×**; impulse sparkover **500 V at 100 V/µs and 650 V at
1000 V/µs**.

**One tube at each position, so there is nothing to coordinate.** The `US3M`'s 1000 V covers the
tube's 500 V and 650 V let-through at 100 V/µs and 1 kV/µs, less the bulk, with room. **The chokes are `22 µH`,
surge-rated `5 A`, ONE part on all
four source boards** — `G-48-S`, `G-300-S`, `G-12-S`, `G-24-S` — bought to the hardest position,
`G-300-S`'s 650 V across the pair, so the three lower boards take it with room; a unit end carries
none (*The ladder by board*).

The sheets stop at 1 kV/µs, where `2036-07-SM` gives 525 V and `2036-30-SM` 650 V; the ladder is designed at those figures.

The series legs' **body is already specified and does not change here**: non-inductive and
pulse-withstanding — anti-surge 2512/MELF where a board can be reached, large TH
composition/metal-oxide on a potted sonde.

## The surge path — the fabrication rules, because the layout IS the rating

**Everything from the cable entry through the tubes and out to the common earthing point is a kA conductor**, and
nothing about its rating comes from a part number. **Five rules, and each one is a number.**

### 1 · The track runs on both layers, mirrored, and the copper is what carries

**Adiabatic heating is the failure mode** — the pulse is over before anything conducts heat away, so
the temperature rise is set by cross-section alone:

```
   ΔT = ρ · J² · t / (ρ_m · c)      →      A = √( ρ · I² · t / (ρ_m·c·ΔT) )

   ρ = 1,68e-8 Ω·m ·  ρ_m·c = 3,45e6 J/(m³·K)
```

| impulse, 8/20 µs | copper needed at **ΔT ≤ 200 K** |
|---|---|
| 5 kA | **0,11 mm²** |
| **10 kA** — the grade this family's tubes are quoted at | **0,22 mm²** |
| 20 kA | **0,44 mm²** |

**Which turns straight into a track width, and this is why the track is doubled:**

| build | thickness | width for 10 kA |
|---|---|---|
| 1 oz, one side | 35 µm | **6,3 mm** |
| 1 oz, both sides mirrored | 70 µm | **3,2 mm** |
| **2 oz, both sides mirrored** | **140 µm** | **1,6 mm** |

**2 oz on both layers is the build**, and 1,6 mm is then an ordinary track rather than a bus bar. On
one layer of 1 oz the same path is 6,3 mm wide and does not fit beside a gland and a tube footprint.

**Skin effect is not a constraint here, and it is written down so nobody derates for it.** An 8 µs
front is an equivalent ~44 kHz, where copper's skin depth is **0,315 mm** — four times the whole foil
thickness. **The entire cross-section conducts**, so cross-section is the only number that matters.

### 2 · Vias stitch the two layers, and the count is set at the TRANSITIONS

**Along the run the two layers carry in parallel and the vias only equalise them. At the ends — the
tube's pad, the gland terminal, the earth lug — all the current crosses**, and that is where the count
is decided.

```
   a 0,8 mm plated hole at 25 µm barrel  =  π · 0,8 · 0,025  =  0,063 mm² of copper each

   10 kA needs 0,22 mm²   →   3,5 vias minimum   →   FIT EIGHT
```

**0,8 mm finished hole is the minimum and it is not a signal via** — a 0,2–0,3 mm via carries a sixth
of this. **Eight at every transition**, spread out rather than lined up in a row, so no single barrel
takes the front. **More than eight costs nothing but board area**, and board area is what a surge path
has plenty of.

**They go BESIDE the pad, never in it.** A 0,8 mm open via inside a solder pad wicks the joint dry;
plugging and capping it is a fab option that costs more than it buys here. **The vias sit in the track
at the pad's edge**, within a millimetre or two, which is close enough that the current has spread
before it crosses.

### 3 · Normal solder mask — and it is the better choice, not a concession

**Nothing special goes on the surge path.** No unmasked window, no tinning, no flood: **ordinary mask,
ordinary finish, and a conformal coat over the assembly like every other outdoor board.** Three
reasons, and only the first is the one that started this:

**Solder does not carry the surge.** It is ~8× copper's resistivity, so even HASL at the top of its
range buys **3 µm of copper equivalent — 2,1 % of a doubled 2 oz track**. Immersion tin buys 0,1 %.
**The cross-section comes from the copper alone**, which is the whole argument for rule 1 and is why
2 oz on both layers is a condition rather than a nicety.

**The mask halves the clearance requirement**, because IPC-2221 puts a coated conductor in a smaller
column than a bare one — see rule 5. Leaving the path open would have made the spacing *worse*, and
the boards are conformal-coated anyway.

**And the mask holds the copper down.** At a 200 K excursion the laminate bond is what stops a track
lifting, and mask over it is mechanical restraint for nothing.

### 4 · SMD tubes and SMD transils — and the reason is manufacturing, not inductance

**Lead inductance does not decide this, and the arithmetic is worth having so nobody argues it does.**
At 10 kA in 8 µs, `di/dt` is 1,25 × 10⁹ A/s:

| | inductance | volts at 8/20 µs |
|---|---|---|
| a pair of 5 mm wire leads | ~10 nH | **12,5 V** |
| the loop a through-hole body makes over the track | ~50 nH | **63 V** |
| **one metre of earth strap** | **1 µH** | **1250 V** |

**A component lead is nothing and a metre of strap is kilovolts.** The *1 µH is kilovolts* rule belongs
to the strap ([`../daedalus/CONSTRUCTION.md`](../daedalus/CONSTRUCTION.md)) and does not reach a package; even at a
front ten times faster a lead is 125 V.

**Two things decide it:**

- **Manufacturing.** A through-hole part is a separate process at a volume house — a hand or wave step
  beside the pick-and-place line — and that is cost and a second yield to manage.
- **Cross-section, and here SMD is genuinely ahead.** A 0,5 mm lead is **0,196 mm²**, *just under* the
  0,22 mm² that 10 kA wants — adequate and no more. **Eight 0,8 mm vias are 0,50 mm²**, 2,5× the
  requirement. **An SMD pad with proper stitching beats a wire lead on the number that matters.**

**Their pads ARE the surge path** and take rule 1's width and rule 2's vias. A footprint drawn to the
datasheet's recommended layout and no further is a footprint that fails at 10 kA.

**The transils are the `5.0SMDJ` family, TWO IN PARALLEL at every position that meets a cable.** It is a **5 kW SMD
body**, so going SMD does not drop the `5KP` grade and no expensive single part is needed.

| position | part | stand-off against the rail | `V_C` class |
|---|---|---|---|
| **48 V feed** | **2× `5.0SMDJ54A`** | 54 V is **1,12×** the rail | ~87 V |
| **300 V feed** | **2× `5.0SMDJ350A`** | 350 V is **1,17×** the rail | ~565 V |
| **`G-12-S`'s ~12 V** | **2× `5.0SMDJ18A`** | 18 V clears the **16,2 V** full-pack light-load corner | |
| **a MOD's 12 V input** | **2× `5.0SMDJ18A`** | the same part as `G-12-S`'s output — the basic set (`../core/blocks/modbus.md`) | |
| **`G-24-S`'s 24 V output** | **2× `5.0SMDJ28A`** | 28 V is **1,17×** the rail | ~45 V |
| **a unit board's 12 V output** — `G-48-U` · `G-300-U-6` · `G-300-U-40` | **one `5.0SMDJ12A`** — the one single position: the rail is behind the barrier and meets no cable, so it clamps a converter below its floor and never a surge | a regulated rail, 12 V ± 1 V: breakdown 13,3–14,7 V, so it holds the output under the 17 V of the smallest buck behind it when the load falls below the cell's floor | 19,9 V at its rated pulse |
| **every 12 V input in the enclosure** — the hosts' and the source boards' | **one `5.0SMDJ14A`**, behind the board's own fuse in the enclosure's fuse field | 14 V stands off the pack's charge stop; it conducts from 15,6–17,2 V, under the 18 V absolute maximum of the `TPS629206`. A reversed pack drives it forward and the fuse clears | ~23 V at its rated pulse |

**Six codes. Outside the Galvani boards the ladder has one position, the 12 V input: a transil and a fuse, no tube and no choke.**

**The stand-off sits one grade above the rail.** A part at exactly 1,00× is specified only not to
conduct *at* its stand-off, which leaves nothing for ripple, tolerance or the −40 °C corner where
`V_BR` drifts down ~6,5 %; one grade up costs nothing.

**350 V on the 300 V run, and where `G-300-U`'s FET is rated — at the steady rail.** Taken at the
clamp, *switch stress = clamp + reflected* would put the `FCD260N65S3` at 565 + 98 = **663 V against
its 650** on `G-300-U-40`. **That convention is wrong here, not the part: `V_C` is not an operating
point** — it is the voltage at the transil's own rated pulse current, `5000/565` = **8,8 A** for a
5 kW body, about 18 A for the pair. The node cannot climb there, because the **~2 µF bulk bank is
on it** and the transil only begins to conduct at its `V_BR`, ~390 V:

| | `G-300-U-40`, 8:1 | `G-300-U-6`, 14:1 |
|---|---|---|
| `V_refl` = `N_PS · (V_OUT + V_F)` | 98 V | 172 V |
| **at the steady rail, 300 V** | **402 V of 650** | **472 V of 650**; **550 V** with the leakage spike on its RCD clamp |
| at the transil's clamp, ~565 V | 663 V | 737 V |

```
   to reach the transil's V_BR, ~390 V    Q = 2 µF × 90 V = 0,18 mC      →   9 A into the node for 20 µs
   to reach its clamp, ~565 V              the pair's rated ~18 A through the transils, over the bank
```

**What arrives at a unit end is the differential residue, and the run limits it.** The surge on a
twisted pair is common-mode, a floating end gives it no path, and what is left is the imbalance
between the legs, arriving through the run's own ~110 Ω of surge impedance and ~600 µH a
kilometre. A **1 kV differential front** — far above what a twisted pair converts — is 9 A on
110 Ω, **180 µC in 20 µs, and 90 V on the bank**: the node reaches `V_BR` and no further. Past it
the transils take the current, and holding the node at their 565 V clamp takes their rated ~18 A —
a **2 kV** differential front, twice the reference. **So the FET's rating is taken at the steady rail — 402 V of 650 on `G-300-U-40`, a 38 %
margin; on `G-300-U-6` its clamp holds the drain at the input plus 250 V, 550 V at the steady rail
and 640 V with the node at `V_BR` — and the clamp figure belongs to the parts on the cable side of the bulk: the transil and
the bulk capacitor itself.** No tube and no choke stands in front of them at a unit end; the run is
the limiter.

**On 48 V the switch is rated at the clamp, which costs nothing there**: 119 V of the
`ISC165N15NM6`'s 150 on `G-48-U` at the ~87 V clamp, 82 V at the steady rail, and the rectifier's
32 V of its 100. **The 300 V boards are rated at the steady rail and must not be re-derived from
the 48 V convention.**

**The bank is at least ~2 µF — the surge residue's 180 µC over the 90 V between the rail and `V_BR`** —
and it is built two ways, because one of the two boards goes under water:

| | `G-300-U-6` — **pressure** | `G-300-U-40` — **land only, no pressure** |
|---|---|---|
| the bank | **5× 1 µF 500 V X7R 2220** (~0,45 µF each at 300 V, ~2 µF) | **4× 1 µF 630 V polypropylene film**, 15 mm pitch (4 µF) |
| at the node | — the bank is ceramic already | **100 nF 1 kV X7R 1812**, for the edge the film's inductance misses |
| the reference front puts the node at | `V_BR`, ~390 V | ~345 V; `V_BR` at a 2 kV front |
| rating against the 565 V clamp | 500 V, standing on the bank holding the node under `V_BR` | **630 V — above the clamp**, whatever the bank does |
| price, JLCPCB, 100 pcs | ~$2,45 | ~$0,59 |

**`G-300-U-40` is the land build.** Nobody sinks a remote Argus, so it carries the cheap and large film; a
pressure build of it takes `G-300-U-6`'s ceramic bank and the recalculation is the reader's.

**Film is the cheap part and the pressure-unfit one**: a wound film has voids between its layers and
nobody rates it for 350–500 bar, so the pod's board is ceramic and pays for it. A 630 V SMD film
exists only in PET and PEN — 0,22 µF in a 15 × 14 mm body — and polypropylene does not survive
reflow at all. **No electrolytic and no hybrid on a pressure board**; elsewhere
the bulk on a node of ~25 V or less is the family's **hybrid polymer, 47 µF 50 V, ZA class,
105 °C / 10 000 h**, and above that ceramic (*The power stage*).

**The capacitance is derived, not assumed.** The switching requirement is tens of nanofarads (at
`G-300-U-40`'s 1,55 A peak and 3,5 µs on-time — 670 µH from 300 V — the charge is 2,7 µC a cycle,
so 3 V of ripple wants 0,9 µF; the kilometre of cable is 528 Ω of reactance at 140 kHz, so the bank feeds every cycle
whole), and hold-up asks nothing — the `LT8316` regulates down to `V_IN(MIN)` 16 V. **What sizes
the bank is the surge residue above**: 180 µC into ~2 µF is ~90 V over the rail and the node stops at
`V_BR` — 640 V on `G-300-U-6`'s FET, behind its clamp; into `G-300-U-40`'s 4 µF it is 45 V and **443 V of 650**. The
steady-rail rating stands on it.

**Two in parallel is worth more than one and less than two, and the reason is `V_BR` tolerance.** The
`A` suffix is ±5 %, so two parts can differ by 10 %, and a TVS `V-I` curve is steep enough that the
lower one takes most of the current. **Fitting them from one reel and giving them symmetric,
equal-length traces is what buys the sharing** — a layout rule, not a purchasing one.

**And the transil's binding case is not the strike.** The `2036` sheet gives an impulse transverse
delay of **< 75 ns at 1000 V/µs**, so on a fast front the tube crowbars before the transil has done
much, and a 5 kW body quoted at 10/1000 µs handles an order more at a hundred nanoseconds. **What
actually sizes the transil is a sustained overvoltage that never reaches the sparkover** — a converter
fault rather than a strike — and there it is the current limit and `ENABLE` that save it,
not the joules in the diode.

**Scatter them.** Where a rail crosses onto a board, leaves a converter or reaches a connector, a
transil is a few cents and a footprint; what costs money on these boards is the 2 oz stack and the
tubes.

### 5 · Clearance follows the tube's impulse sparkover, never the rail

**A fired tube shorts the pair, and what stands across the board before it fires is the impulse
sparkover** — not the 48 V or 300 V of the feed. The worst figure is **`2036-30-SM`'s 650 V at 1000 V/µs** on the 300 V boards and **`2036-07-SM`'s
525 V** on the 48 V and data positions. **Against 650 V the impulse binds nowhere** — the
working voltage sets every column on the 300 V boards. **Two standards split the job, because IPC-2221 has no row for a
microsecond event**: the *working* voltage is spaced per **IPC-2221B Table 6-1** (B1 internal ·
B2 external bare, ≤3050 m · B4 external under mask), and the *impulse* is spaced per
**IEC 60664-1 Table F.2** (case A, inhomogeneous field — the next row above the figure).

| what stands there | internal B1 | bare copper B2 | under mask B4 / IEC |
|---|---|---|---|
| 300 V working (the rail) | 0,20 mm | **1,25 mm** | 0,40 mm |
| 650 V impulse (`2036-30-SM` let-through) | — | 0,10 mm | 0,10 mm (IEC, 0,8 kV row) |
| 54 V working (the 48 V feed, full pack) | 0,10 mm | 0,60 mm | 0,13 mm |
| 525 V impulse (`2036-07-SM` let-through) | — | 0,04 mm | 0,04 mm (IEC, 0,5 kV row) |

**The build rule that falls out: on the 300 V boards 0,40 mm under mask, 1,25 mm bare copper,
0,20 mm internal — the working voltage binds in every column, the impulse asking a quarter of it.
On 48 V everything is ordinary: 0,6 mm bare, 0,13 mm under mask.** **Above 3050 m the bare-copper column re-reads as B3** — a siting note, not a board
change, since the binding uncoated runs sit at glands and connectors that a high site's enclosure
still covers. The 0,40 mm under mask holds up to the IEC 1,2 kV row (0,25 mm).

### What this costs, so it is a decision and not a default

**The whole spec is now one line item: 2 oz on both layers.** Everything else — the mirrored track,
the eight vias, the mask, SMD parts — is drawing, not purchasing, and a volume house takes the order
without a note attached. **That is deliberate**: a fabrication rule that needs a hand operation is a
rule that does not get followed.

**The 2 oz stack is paid on the boards that meet a cable and on no others**: `G-48-S` · `G-48-U` ·
`G-300-S` · `G-300-U` · `G-I-N-025` · `G-I-M-005` · `G-12-S`. The optical boards
(`G-O-10-10`, `G-O-2-100`) carry no ladder and no surge path, so they stay an ordinary 1 oz stack — one
more reason glass is the cheaper medium wherever it reaches.

### The ladder on each board

One drawing per board, each complete on its own — read the board you are holding and stop
there. The earth symbol means a short connection to the common earthing point, a **2,5 mm² strap a few
centimetres long** off the board's **earth stud** — an M4 plated through-hole, a pad of at least 10 mm
on both layers, a ring of stitching vias around it, the tube's centre running onto it on the doubled
2 oz track of rule 1; stacked screw · plain washer · the board · plain washer · the strap's crimped
ring lug · plain washer · spring washer · nut, all brass. No spring block carries the tube's current:
at 10 kA the contact spot repels its own surfaces with a force of the spring's order and a bolt does
not. ([`../daedalus/CONSTRUCTION.md`](../daedalus/CONSTRUCTION.md) owns the strap and the point.) At
8/20 µs edges every metre of wire is ~1 µH and adds kilovolts of L·di/dt, which is why the board
mounts at the enclosure entry beside the common earthing point and never dumps down an interconnect cable.

**The in-box cable has no drawing here and no ladder at all** — no cable leaves the enclosure,
so there is nothing to clamp, nothing to crowbar and nothing to bond.

**A source board sources the cable, so its ladder reads left to right, out of the board.**

**`G-48-S`**
```
  the feed cell ─▶ blocking diode ─▶ 2× 5.0SMDJ54A ─▶ chokes ──────────────────▶ ═══ 48 V
                                    clamps               across the pair
                                    ~87 V                2036-07-SM ────────────▶ ⏚ earth point
                                                         through the common earthing point
```

**`G-300-S`**
```
  the feed cell ─▶ blocking diode ─▶ 2× 5.0SMDJ350A ─▶ the two chokes ─────▶ ═══ 300 V
                                        clamps ~565 V  across the pair
                                                       2036-30-SM ─────▶ ⏚ earth point
                                                       through the common earthing point
```
*At a remote Argus the same board is fitted identically and the earth strap is not made.*

**`G-12-S`** — the isolated 12 V a bought device plugs into
```
  SN6507 + 750319691 ─▶ 2× 5.0SMDJ18A ─▶ chokes ─────────────▶ ═══ ~12 V, unregulated
   push-pull, 1,13:1     clamps          across the pair
                                         2036-07-SM ───────────▶ ⏚ earth point
                                         through the common earthing point
```
*The output is **unregulated** — the pack through the winding, so **~10,7 V at the 11 V reserve floor
and ~16,2 V at the 15 V ceiling under light load**. **The 18 V transil is chosen against that 16,2 V
corner**, not against a nominal 12. The tube's 52 V holdover sits far above the rail, so a struck
tube quenches itself.*

**`G-24-S`** — the switched 24 V
```
  LT3748 + 750311592 ─▶ blocking diode ─▶ 2× 5.0SMDJ28A ─▶ chokes ──▶ ═══ 24 V, switched
   1:1, 8 µH                                 clamps        across the pair
   [ISC165N15NM6]                                          2036-07-SM ─────▶ ⏚ earth point
                                                           through the common earthing point
```
*`ENABLE` is the switch and **the board stands off between runs**, so this ladder is unpowered most
of the time. The `INA238` sits in the **isolated output** as the load's ammeter. The tube quenches
on 24 V for the same reason it does on 48.*

**`G-I-N-025` at a source end** *(`G-I-M-005` is the same ladder on its one pair)*
```
  ISO1452 ×2 + ISO1450 ▶ SM712 ──▶ 10 Ω ─────────────────────▶ ═══ TX / RX / clock
                                            across the pair
                                            2036-07-SM ────────────▶ ⏚ earth point
                                            through the common earthing point
```
*One board serves both ends: the footprint is on every board, and the tube and its strap are
populated at the source end and left off at a unit end.*

**`G-O-10-10` · `G-O-2-100` — glass, either end**
```
  1×9 module ×2 ────▶ fibre — nothing electrical to protect, which is
                      the whole reason these boards are shorter than G-I-N-025
```

**A unit board eats the cable, so its ladder reads the other way — and it carries no tube at all.**
No gas tube and no footprint for one — and **no chokes either**: without a crowbar outside them
there is nothing to decouple from, and the run's own ~600 µH a kilometre already dwarfs a 22 µH
part. What a unit end holds is the differential residue, on the transil.

**`G-48-U`**
```
  ═══ 48 V ─────────────────────────▶ 2× 5.0SMDJ54A ─▶ bulk ─▶ LT3748 island
               across the pair          + to −, against the unit's own floating ground
               NO gas tube, NO chokes
```

**`G-300-U`**
```
  ═══ 300 V ─────────────────────────▶ 2× 5.0SMDJ350A ─▶ bulk ─▶ LT8316
               across the pair          + to −, against the unit's own floating ground
               NO gas tube, NO chokes
```

**`G-I-N-025` — one board, both ends, and the tube is the source end's population**
```
  ═══ A / B ────────────────────────▶ 10 Ω ──▶ SM712 ──▶ 2× ISO1452 + ISO1450
                across the pair                              to the unit's own floating ground
                2036-07-SM ───────────▶ ⏚ earth point — fitted and strapped at the source end,
                                          left off at a unit end; the footprint is on every board
```

### The ladder by board

**Every board that meets a cable carries a transil. What differs is what stands in front of it.**
The series element is a **choke on a feed** and **10 Ω on a data pair**, and the **tube is the end
of the cable's**, not the board's.

| board | series | transil | tube, through the common earthing point |
|---|---|---|---|
| **station power** — `G-48-S` · `G-300-S` | **chokes**, 22 µH / 5 A | **yes** — `5.0SMDJ54A` · `5.0SMDJ350A` | **yes.** Here the **strap** is the population: bolted at a station, **fitted and unconnected at a remote Argus**, because the same sheet stands at both |
| **`G-12-S`** — the isolated 12 V a bought device plugs into | **chokes**, 22 µH / 5 A | **yes** — `5.0SMDJ18A` on the ~12 V output | **yes.** It is the one board that **never travels** — a station only — so its centre is always bolted |
| **`G-24-S`** — the switched 24 V | **chokes**, 22 µH / 5 A | **yes** — `5.0SMDJ28A` on the 24 V output | **yes**, on the same grounds |
| **unit power** — `G-48-U` · `G-300-U` | **none** | **yes** | **no — no part and no footprint.** A unit end floats by construction, so there is nothing for a three-electrode tube to reference and nothing to reserve space for |
| **communication, copper** — `G-I-N-025` · `G-I-M-005` | **2× 10 Ω** per pair | **yes** — `SM712` across the pair | **at the source end only** — one board serves both ends, so the **footprint** is on every board and the tube and its strap are populated where the source end is; a unit end carries 10 Ω and the `SM712` and nothing else |
| **optical** — `G-O-10-10` · `G-O-2-100` | **none** | **no** | **no** — nothing electrical arrives on a fibre |
| **a MOD on a ModBus arm** | **2× 10 Ω** | **yes** — `SM712` on the pair, **2× `5.0SMDJ18A`** on the 12 V | **no** |

**Why a unit end carries neither the tube nor the chokes.** A GDT's job is dumping common-mode bulk
into an earth, and a floating board has no earth to dump into — the three-electrode part degenerates
into two gaps in series the moment its centre floats. The chokes go with it: they sit **between** a
crowbar and a clamp, and with no crowbar outside them there is nothing to decouple from, while the
run's own **~600 µH a kilometre** already dwarfs a 22 µH part. What reaches a unit end is the
differential residue, and the transil holds it.

**"One board serves both ends" is true of the communication boards and false of the power boards.**
A communication board carries the tube's footprint at both ends and the tube at one; a unit power
board is its own design and does not carry even the footprint of the part it cannot use.

**Optical boards stay bare on purpose.** No tube, no transil, no chokes — that is not an omission
and it is not to be "finished" later.

**A remote Argus is not earthed and is not going to be.** Four outgoing `G-300-S` boards there mean
four earthing points with nothing to bolt to, and a tube centre with no reference is an unconnected
part — so at that position the strap is simply not made and the board is the sacrificial one. **The
accepted outcome of a direct strike there is the loss of the boards**, which shifts the goal from
protection to replacement, and the replaceable unit is a small board that someone is travelling to
the site to swap anyway. **Branch spacing is the only other mitigation and its job is stated
honestly: it does not stop a strike, it stops one strike taking all four branches**
([`../daedalus/CONSTRUCTION.md`](../daedalus/CONSTRUCTION.md)).

**The GDT grade follows the conductor, not the board** — a data pair idles at a couple of
volts and a feed pair at the feed:

| conductor | transil | series | **through the common earthing point** — `2036-xx-SM`, source end only |
|---|---|---|---|
| **48 V feed** | **2× `5.0SMDJ54A`** — clamps ~87 V, the number every downstream rating is built to | 22 µH chokes | **`2036-07-SM`** |
| **the data pair** | **SM712** | 10 Ω | **`2036-07-SM`** |
| **300 V feed** | **2× `5.0SMDJ350A`** (clamps ~565 V) | 22 µH chokes | **`2036-30-SM`** |
| **glass** | — | — | — nothing electrical arrives |

**Why those two codes, in the order the choice is actually made:**

| | what the part sees | minimum sparkover | margin |
|---|---|---|---|
| `2036-07-SM` — 75 V ±20 % | **24 V**, half the rail per gap | 60 V | **2,5×** |
| `2036-30-SM` — 300 V ±20 % | **150 V**, half the rail per gap | 240 V | **1,6×** |

**On the three-electrode tube, 300 V is chosen over 250 for two reasons and neither is the per-gap margin alone.**
`2036-25-SM` would give 1,33× and work; **`2036-30-SM` gives 1,6×, and its line-to-line breakdown —
"approximately 1,8 to 2 times the line-to-ground rating", the sheet's own note — is 540–600 V, so it
cannot fire across a floating 300 V pair** at a remote Argus where its centre is unconnected. That case
does not exist for the 250 V part with the same certainty.

**Holdover splits the two voltages, and it is the one place the 300 V run is genuinely worse.**
The tube extinguishes only if the line sits below its DC holdover:

| | holdover | the rail | |
|---|---|---|---|
| `2036-07-SM` | **52 V** | 48 V | **extinguishes on its own** |
| `2036-30-SM` | **135 V** | 150 V a gap | **does not.** The one tube at that position stays lit on the feed once struck; the converter's current limit holds the arc and **`ENABLE` is what clears it** — the `INA238` on the output raises `ALERT` and the host takes the feed away |

**So on a 300 V run a fired tube is a dead short across the feed until something else acts** — the
converter's current limit, which holds the arc but does not end it, and then **`ENABLE`**. Both are
already in the design and are now mandatory rather than belt-and-braces. **No fuse anywhere in the
family** (`WHY.md`): a polyfuse re-closes into the same arc and reports nothing, where the
`INA238`'s `ALERT` reports it and `ENABLE` stays off until somebody decides.
On 48 V the 52 V holdover clears the feed and the tube quenches itself.

**And which positions a board carries is not a grade question at all** — it is which board it
is (*The ladder by board*): source power boards and the communication boards carry both,
**a unit power board carries no tube at all**, and **`G-12-S` the three-electrode tube**, because it
never stands at a floating end.

**The part, physically — `2036-xx-SM`, Mini-TRIGARD:**

| | |
|---|---|
| electrodes | **3**, the centre through the common earthing point |
| body | 5 mm dia × 7,3 mm, pad ~8,2 × 5,6 mm |
| **8/20 µs, repeatable** | **10 kA, 10 operations — total, divided between the legs**, so 5 kA a leg |
| 8/20 µs, once | 20 kA |
| 10/350 µs | 2 kA, 1 operation |
| capacitance | < 2 pF |
| temperature | −55…+105 °C |
| tolerance | ±20 % |

**The 10 kA repeatable rating is the number the surge path is built to** — see *The surge path*. The
20 kA single-operation figure is deliberately **not** used: a tube that survives it once leaves a board
that is a write-off anyway, and sizing copper to a once-only rating buys nothing at a position whose
accepted outcome is replacement.

The data pair's 60 V class is not a loose number: it is coordinated with the SM712 and the
10 Ω legs — the array clamps near 24 V and the drop across the series R carries the rest, so
the GDT takes over at about 4 A; 5 Ω legs would put ~7 A through the array first.
**The same tube carries the feed pair**, where what matters is that it never fires in service:
its **60 V minimum DC sparkover** sits above the 48 V feed, and its 52 V *holdover* — a different
parameter — is what lets it extinguish afterwards.

The three-electrode part fires as a pair by construction — a strike in one gap ionises the shared
chamber. **With the centre bolted to the earthing point that is the common-mode path; with the centre floating it
degenerates into two gaps in series**, which is why a unit end carries none (*The ladder by board*).

**One order code per position:**

| position | codes |
|---|---|
| **48 V and data** — through the common earthing point | **`2036-07-SM-RP LF`** — Bourns, three-electrode, 75 V, 5 × 7,3 mm |
| **300 V** — through the common earthing point | **`2036-30-SM-RP LF`** — 300 V ±20 % |

*(`-RP` is the 24 mm reelpack and `LF` the RoHS suffix; the family orders as `2036-xx-SM` plus
those two options, and a reel is what a pick-and-place line wants.)*

> **What the chips actually see is the impulse sparkover, not the DC figure.** A GDT is slow
> and it lets through hundreds of volts before it strikes — under a fast edge a 60 V-class
> part strikes far above 60 V, and that let-through is what arrives at the barrier. So the DC
> sparkover only decides that the part never fires in service; **the transil behind it and the
> series legs between them are what the survivable number is built from.** The GDT eats the
> coulombs, the transil eats the let-through, and the resistance in between is what lets the
> transil win. **The two codes bound it, at the two conditions the sheets give:** `2036-07-SM`
> **250 / 525 V** and `2036-30-SM` **500 / 650 V**, at 100 V/µs and 1000 V/µs. Those are ceilings, not
> typical values — the actual strike depends on the edge and the temperature, which is why the
> ladder is designed to the ceiling and the peak is measured if a build ever needs it.

### The line side, read

**The line side, read (ISO1450 sheet):** VCC2 = 3–5,5 V, so the 3,3 V line rail is legal;
**VOD ≥ 1,5 V into the RS-485 load at 3,3 V (2,3 V typ)**; thresholds ±200 mV, 30 mV
hysteresis; PWD ≤ 6 ns on the 50 Mbps part — absorbed by the ÷2 one-edge clock reception
anyway; **propagation delay: the driver 19 ns typical, 41 maximum, the receiver 36 and 60**,
over the whole temperature and supply range — what the ranging halves, and where its
transceiver term comes from (`../core/blocks/gps-pps.md`). The draw, VCC2 at 3,3 V (static rows read, the 2²² point sits on the sheet's own
rate curve between the static and 50 Mbps rows):

| line-side draw | typ | max |
|---|---|---|
| clock TX, continuous 2²² (~8,4 Mbps signaling) | ~54 mA | ~65 mA |
| a channel receiving | 2,6 mA | 4,3 mA |
| a channel driving (data burst) | 48 mA | 58 mA |
| **head port steady — clock TX + data RX** | **~57 mA ≈ 190 mW** | ~69 mA |
| far-end steady — both channels RX | ~5 mA ≈ 17 mW | ~9 mA |

So the crossing is a **100 mA-class** part per port, and each port pays its own ~0,25–0,4 W from
the battery. Logic side: ICC1 ≈ 2,6/4,4 mA per chip, on the host's 3,3 V.

### What a communication board costs the host's 3,3 V

**The rows above are the ISOLATED side — the host's pin does not feed it.** The host's 3,3 V
feeds the `SN6505B`'s input, and the line side arrives through the transformer at the
efficiency the sheet plots for this pair — **`SN6505B` + 750313734 at `VCC` 3,3 V** (SLLSEP9I,
Figure 6-14): **~85 % from 40 mA up, ~75 % at 20 mA, ~50 % at 5 mA and ~40 % below that**, the
part's own 1,56 mA of supply current being what the light end pays. Beside it the host's pin carries the logic side directly, `ICC1`
per transceiver — **three on `G-I-N-025`**, one on `G-I-M-005`. This table is what a host adds up,
and it is the one to quote:

| board and position | line side | + island at the plotted η | + logic | **on the host's 3,3 V** |
|---|---|---|---|---|
| **`G-I-N-025`** at a **head port** — a card's down port: clock TX + data RX | 57 mA | 67 mA (85 %) | 3 × 2,6 | **~75 mA, 248 mW** |
| **`G-I-N-025`** at a **far end** — a unit, or a card's up port: both channels RX | 5 mA | 10 mA (50 %) | 3 × 2,6 | **~18 mA, 59 mW** |
| **`G-I-N-025`** at a head port, **during a data burst** — the third channel driving | ~105 mA | 122 mA (86 %) | 3 × 2,6 | **~130 mA** |
| **`G-I-M-005`** on a ModBus arm, listening | 2,6 mA | 6,5 mA (40 %) | 2,6 | **~9 mA** |
| **`G-I-M-005`** on a ModBus arm, asking | 48 mA | 57 mA (84 %) | 2,6 | **~60 mA** |
| **an optical board** at a master port / at a far end — no barrier, no island | — | — | — | **~105 mA / ~55 mA; 175 / 125 mA at most** (*The fibre front*) |

So a copper communication board is an **18 to 130 mA** part on the host's rail depending on which
end it is and whether it is mid-burst, and glass is ~55 mA at a far end and ~105 mA at a master port, 125 and 175 mA at most. **The 80 % chain figure does not apply to
this island** — the sheet's curve does, and it is kinder at a head port and harsher at a far
end.

## The islands — which chip on which board

**Two controllers, and the boundary is the input voltage.**

**`LT3748` carries the source sheet and the 48 V unit board** — `G-48-S` / `G-300-S`, `G-48-U`. It runs
5–100 V in on an external N-FET, samples the primary so it needs **no opto and no third winding**,
and its gate driver sources and sinks 1,9 A. **`LT8316` keeps the one board fed from a 300 V
cable** — `G-300-U` — where 600 V of input rating is the whole reason it is there.

### Specifying a winding, when the recommended part is gone

**A winding does not know which controller spins it.** A controller's datasheet names one part
because somebody built one board; what the position actually demands is a list of numbers, and a
catalogue part that meets them works whether it was drawn for this controller, another one, or none.
That is how the six order codes in this family were picked, and none of them is a recommended part.

| the number | where it comes from | what it is **not** |
|---|---|---|
| **turns ratio `N_PS`** | `V_OUT + V_F` and the reflected voltage the switch must stand | not the controller's; the output voltage is set by `R_FB`/`R_REF`, not by the winding |
| **primary inductance `L_PRI`** | with `R_SENSE` it must clear the controller's minimum, and it sets the **frequency** at a given load | **not the power.** `P_max = ½·I_pk/k` with `k = 1/V_IN + 1/V_refl` — `L` cancels out of it entirely |
| **`I_SAT`** | above `I_pk` at the sense threshold's **hot** corner, not its typical one | not a headline figure to match: this family runs at 61 % to 81 % of it and says which at every position |
| **the minimum `L_PRI` at a given `R_SENSE`** | the controller's sampling window and minimum on-time — *The one rule that screens a transformer*, below | the one entry in this table that **is** the controller's |
| **isolation, `V_RMS`** | what the barrier at that position has to hold | not a place for the cheapest grade |
| **leakage inductance** | the clamp's dissipation and the FET's `V_DS` spike | not negligible on a high-ratio part — it is why a 1:10 winding is a different animal from a 1:1 |
| **height and footprint** | the enclosure | **sometimes the whole decision**: `G-300-U-6` exists because a 40 mm pipe takes 8,6 mm of winding and not 23,5 |

**Four things belong to the controller and they are the only four.** The **sense threshold** picks
`R_SENSE` and never the winding. The **minimum on-time and the sampling window** set the
minimum inductance — the one case where a controller rejects a winding. **`V_IN(MIN)`** is the part
and not the magnetics: `LT8316`'s 16 V is why it cannot take a battery-side position however good
the winding is. And the **frequency clamp with the burst behaviour** sets the minimum load, which
is a rating question and not a selection one.

**A flyback winding is gapped and a winding for another topology is not — and the datasheet does
not always put it on the first page.** A flyback stores its energy in the gap and hands it over
between pulses; forward, push-pull and resonant parts pass it through within the pulse and their
cores are ungapped. **An ungapped part in a flyback saturates in the first cycles whatever its
ratio and inductance say**, so a candidate counts only where its own datasheet calls it a flyback
transformer or a coupled inductor. **The converse is equally true and this family uses both**: the
`SN6505B` and `SN6507` positions are push-pull, so they take the ungapped centre-tap part
(`750313734`, `750319691`), and a gapped flyback winding would be the wrong part there.

**So what a search is given is five lines** — ratio, inductance with its tolerance, `I_SAT`,
isolation and height — and every candidate that comes back is put through the screen below before
anything else about it is read.

### The one rule that screens a transformer

Everything the `LT3748` demands of a winding is a minimum primary inductance, and the sense
resistor is in it:

```
L_PRI  ≥  max( V_refl · R_SENSE · 400 ns / 15 mV ,  V_IN(MAX) · R_SENSE · 250 ns / 15 mV )
```

The first term is the **400 ns the sampling circuit needs to settle and read the output voltage off
the flyback pulse**, taken at the minimum current limit, `V_SENSE(MIN)` = 15 mV. The second is the
**250 ns minimum gate on-time**, taken at maximum input. `V_refl` is `(V_OUT + V_F) × N_PS`.
**The winding is screened at its low tolerance corner, −10 %**, and **the shunt is chosen as large
as the rule allows and no larger** — a larger shunt would lower the peak current further, and the
rule is what stops it. `V_IN(MAX)` is 20 V on the step-ups and 50 V on `G-48-U`.

| position | `V_refl` | `R_SENSE` | settling | on-time | **needs** | the winding, nominal · −10 % |
|---|---|---|---|---|---|---|
| `G-48-S`, 1:4,42 | 11,0 V | 9,1 mΩ | 2,7 µH | 3,0 µH | **3,0 µH** | 14 · 12,6 µH |
| `G-300-S`, 1:10 | 30,1 V | 5,5 mΩ | 4,4 µH | 1,8 µH | **4,4 µH** | 5 · 4,5 µH |
| `G-48-U`, 2,5:1, 12 V out | 31,3 V | 15 mΩ | 12,5 µH | 12,5 µH | **12,5 µH** | 14 · 12,6 µH |
| `G-24-S`, 1:1 | 24,7 V | 10 mΩ | 6,6 µH | 3,3 µH | **6,6 µH** | 8 · 7,2 µH |

**On `G-300-S`, `G-48-U` and `G-24-S` the rule fixes the shunt**, and the peak current each delivers
follows from it; on `G-48-S` saturation fixes it and the rule has four times the margin.

Two numbers a datasheet's first page always carries — inductance and saturation current — decide
whether a part can work at all, before anything else is read.

### No burst, and therefore a minimum load

**The `LT3748` has no burst mode.** Below 15 % of the current limit it drops the peak to its
minimum and inserts a delay, so it runs in discontinuous mode with the **frequency falling to a
floor near 42 kHz**. That floor and the energy of one minimum pulse are the whole answer:

> **`P_MIN` = `f_FLOOR` · ½·`L_PRI`·(0,15·`I_pk`)²**, which against the cell's own rating is
> **`P_MIN`/`P_MAX` = 0,0225 · `f_FLOOR`/`f_FULL`**, because in boundary mode
> `P` = `f`·½`L`·`I²` at every point and only `f` and `I` move.

**The floor is therefore a property of the CELL and not a percentage of the family.** `f_FLOOR`
is fixed near 42 kHz while these cells run from 41 kHz to 225 kHz at their shunt's limit, so the
ratio spans five to one. **The minimum pulse is 15 % of the SHUNT's peak, not of the declared
operating point** — where a shunt allows more than the board is declared at, the floor follows the
shunt. **The sheet's "about 2 % of maximum" is the case where a cell's
full-load frequency already sits at the floor — which here is `G-48-S` and nothing else.**

| board | controller | rating | `f` at full load | `I_pk` · `L_PRI` | **minimum load** | of rating |
|---|---|---|---|---|---|---|
| **`G-48-S`** | `LT3748` | ~26 W | **41 kHz** | 9,90 A · 14 µH | **0,65 W** | 2,5 % |
| **`G-300-S`** | `LT3748` | ~66 W at the shunt, ~47 W declared | 105 kHz at the shunt's limit | 16,4 A · 5 µH | **0,63 W** | 1,3 % of the declared |
| **`G-24-S`** | `LT3748` | ~33 W | 112 kHz | 9,0 A · 8 µH | **0,31 W** | 0,9 % |
| **`G-48-U`** | `LT3748` | ~50 W at the shunt, ~25 W declared | 225 kHz at the shunt's limit | 6,0 A · 14 µH | **0,24 W** | 1,0 % of the declared |
| **`G-300-U-40`** | `LT8316` | 40 W | — | — | **~0,40 W** | 1 % |
| **`G-300-U-6`** | `LT8316` | 6 W | — | — | **~0,06 W** | 1 % |
| **`G-12-S`** | `SN6507` | 0,4 A | — | — | **none** | — |

**The `LT8316` boards keep the part's own figure.** It bursts, so its floor is a burst packet and
not one pulse; the sheet states **~1 % of the rated power** flatly, and a one-pulse derivation
underestimates it by three to one and is not used. **`G-12-S` has no floor at all** — an
unregulated push-pull driver has no loop to lose.

**Below the floor the output rises, and on a spur nothing self-limits** — the load is downstream
converters, which are constant-power — **so the rail climbs until the transil conducts and sits
at its knee**, a surge part carrying a continuous load. The protection stays inside its ratings;
its margin is gone until the load returns.

**Which board binds, and when — and in service none of them does.** **Nothing in this station is
ever parked half-on**: a port is either running with the unit's full load on it, or dark with the
converter stopped, because a board with nothing to do is ended and switched off rather than left
drawing a trickle (`README.md`, *Three states*). `G-48-S`'s 0,65 W is 13,5 mA out of the 48 V rail
and any live unit is far past it; `G-48-U`'s 0,24 W is 20 mA out of its 12 V and a unit's own
processor covers it. **The one state that reaches no floor at all is an ENABLED PORT WITH
NOTHING ON IT**, and that is an operating rule, not a component: a run is wired before its port is
switched on.

### Sized for the largest far end — what a smaller build changes

**Every power board is sized for the most capable case its position must serve** — a far end at ~24 W
behind a port, ~47 W out of a 300 V cell — and for this use the `LT3748` and the `LT8316` with
their external FETs came out best of the parts weighed. A leaf at 0,5–1 W sits above
`G-48-U`'s 0,24 W floor, so a unit board never binds; what such a build can fall
below is the SOURCE cell's 0,65 W (*No burst, and therefore a minimum load*). A build that wants that
back changes the rating and keeps the controller: **the winding**, **`R_SENSE` / `R_SNS`** and
**the rectifier grade**. The controller and the FET stay — they set the frequency window the
cell was computed in — and a different controller is a recomputation of the whole cell, not a
swap.

### Switching the converter — `EN/UVLO`

**There is no input divider.** Cell chemistry varies, the BMS cuts, and the processor reads the
BMS over 485 — so undervoltage is decided upstream and the converter is told, not left to work it
out.

```
   processor GPIO ───────────┬──── EN/UVLO      threshold 1,223 V
                             │
                           100 k
                             │
                            GND
```

**1 = runs · 0 or Hi-Z = stops.** Plain logic, and **not a claim of fail-safe** — a cross-wired
pair of BMSs and redundant paths cost more than they buy. Pull the wire and it stops.

**That is the source end — `G-48-S`, `G-300-S`, and `G-12-S`'s `SN6507` the same way — where a host drives the pin.** On
`G-48-U` the same cell has no processor in front of it — the unit's processor is behind the
converter — so the 100 kΩ goes **to the input instead of to ground** and the converter runs from
the moment the feed arrives. **A unit power board is never switched off**; the kill is the
source end's. `G-300-U` is the same: `EN/UVLO` tied to run.

**The 100 kΩ is not arbitrary.** Below the threshold the pin sinks **2,4 µA** (up to 2,9) for
programmable hysteresis, and that current sits on the pull-down: 100 kΩ holds the pin at 0,24 V,
inside the **< 0,5 V** window where the part draws under 1 µA. **208 kΩ is the edge of true
shutdown and 410 kΩ the edge of shutting down at all** — a megohm would leave the converter
running with the processor dark.

**The resistor lives on the Galvani board, at the pin**, behind the connector — so an
unplugged connector reads as off, which is what the doctrine already says about `ENABLE`. **The
GPIO must be push-pull**; open-drain would never produce the high. The pin's absolute maximum is
100 V, so 3,3 V is nothing, and at reset the GPIO is Hi-Z and the pull-down holds it stopped.

## The islands — the no-load rating

**Rated at no load:** the far end then sees the source's FULL V₀, never the loaded voltage — on the
48 V feed the transil pair clamps at ~87 V and with the reflection on top the switch sees ~119 V, which is
what puts **the 150 V `ISC165N15NM6` and not a 100 V part** on that board (*Choosing the FET*). The 300 V
board sits behind its own ladder, one class up.

## The low rails — the host's own

**Every buck is sized at 40–50 % of its rating on average and at most 70 % at its ceiling.** The
average is the board's typical draw; the ceiling is every part at its maximum — the processor with
its peripherals at its real clock, the lines and dividers that run continuously, and the
communication board at its own end's ceiling (*What a communication board costs the host's 3,3 V*).
A short behind a port is the buck's own current limit and hiccup, never a fuse.

**An LDO stands only where a part has no regulator of its own and its rail feeds an analogue path**
— a converter's analogue supply, an ADC reference. A MEMS sensor carries its own internal
regulators, so it takes the buck's rail through an inductor and no LDO.

**A power board hands out 12 V and nothing lower; every host makes its own rails from it** — a unit that needs only 3,3 V carries one buck, a unit with sensor LDOs a second to 4,0 V, side by side on the 12 V and never in cascade; a 300 V unit board is the converter and nothing else.

**The 12 V is the island's own output** — regulated on a fed run, the battery itself inside the
enclosure (10–20 V, off the wire) — and a board that wants a true analogue 5 V makes it
on its own board from the 12 V: an `LMR43610` to 5,5 V, then its `TPS7A4701` (*The buck cell*).
Digital rides a buck and never an LDO on purpose: an LDO straight from 12 V would burn (12 −
3,3) × 0,3 A ≈ 2,6 W against a node total of ~1,1 W; the buck costs ~0,06 W.

**1,8 V on the boards that carry the lower rail is the optimum on consumption** — the
processor and the converter each draw about half of what they draw at 3,3 V, and the
converter's `IOVDD` takes no other level. **3,3 V stayed where it cannot be otherwise** — the
buses, 485, the optics, the Galvani body — so the boundary between them is a level translator
on a few slow lines and not on a whole board. Everything at 1,8 V is not on: the line, the
lasers and the Galvani body are held by 3,3 V parts for availability and support, so converting
the station would mean translators everywhere instead of on the boards with the lower rail.

**The H7A3 boards take two `TPS629206` straight off the 12 V, no cascade.** One makes the
processor's **1,8 V at 1 MHz in forced PWM** — 12 → 1,8 V is a 15 % duty, 150 ns of on-time,
122 ns at the 14,7 V the unit board's transil lets through, against the part's 40 ns minimum; at
2,5 MHz it would be 49 ns, which is why this rail runs at 1 MHz. The other makes the **analogue
feed at 2,5 MHz in forced PWM**: **5,3 V** on Tesla (its 5 V analogue LDOs, 0,3 V over their
output) and **4,0 V** on Marconi, `Quark-Photon` and `Quark-Neutron/Positron` (their clean 3,3 V
LDOs); Tesla's 3,3 V interface rail is a third. **The digital supplies of the converters — `IOVDD`, `DRVDD`, `DVDD` — take the
processor's rail through the shielded 2,2 µH into 10 µF + 100 nF, then the bead `GZ2012D301TF` into 100 nF at the pin, not a regulator of their own**, and the MCU's `VDDA`/`VREF+` take it through the shielded 2,2 µH into 10 µF + 100 nF, the buck-noise inductor (`../core/POWER.md`, *The filter parts*);
the analogue supplies keep their LDOs, and a converter's quiet analogue 1,8 V takes a
**`TPS7A2018`** of its own where it is the converter's analogue core (Marconi's `AVDD`, the `AD9251`'s on the scintillation boards); Tesla's `ADS127L14` takes its `AVDD2` off the processor's 1,8 V through the inductor and the bead, its sheet allowing 1,74–5,5 V. The board documents carry
the trees (`../tesla/HARDWARE.md` §6, `../marconi/HARDWARE.md`, `../quark/tubes/HARDWARE.md`,
`../pluvius/HARDWARE.md`).

**Placed, not parked.** No switching frequency this family can buy is outside every measuring
band, and the island flyback sweeps with its load anyway, so the rule for a converter that
shares a box with a measuring board is distance, a shielded inductor, a small hot loop, and
`MODE` set per board — forced PWM where a band listens, auto elsewhere — and **what leaves the
board on a cable is filtered**, so the converter's frequency stays on its board (`README.md`,
*Converters and measuring boards*). The converter's own spectrum — fixed, swept by its load or
spread by the part — is then not a criterion. Where a board parks its own buck on its own sampling grid,
that is the board's business (`../tesla/HARDWARE.md`). **The criterion is a bench measurement
rather than arithmetic:** the spur coupled into the receiver's chain input must stay under
**65 mV** for the folded product to sit under the noise floor of a 16 kHz analysis strip
(`../tesla/HARDWARE.md` §2, §5); a sane layout clears that by two to three orders.

## Feed telemetry — INA238, at both ends of a run that leaves the ground

The feed voltage is a property of the cable, and the cable lands on the block — dragging RAW
onto a host board just to measure it re-imports the surge path the block exists to keep out.

**Which boards carry it:**

| board | station | far end | why |
|---|---|---|---|
| **every power board** | **fitted** | **fitted** | the run is kilometres, or the far end is potted and unreachable; both ends measured is what makes the loop visible. **Mandatory on every power board**: its threshold is what holds a source cell at its declared rating — the shunts allow more — and on 300 V its `ALERT` is what takes the feed away from a struck tube |
| **the communication boards** | **not fitted** | **not fitted** | there is no feed on them to measure — that is the power board's job, and its `INA` is on the connector |

**Which side of the barrier the shunt sits on — one rule, no exceptions.** **The low-side shunt
goes on the side away from the host**: the feed on a source cell, the feed on a unit board, the
isolated arm on `G-12-S`. **So every power board crosses the barrier and every one carries the
`ISO1642`.** The reason is not symmetry — it is that the `INA238` reads bus
voltage as well as current, and on the far side both halves are the thing that fails. On the
host's side the voltage is the host's own rail and does not move.

**Monitor: INA238** (16-bit / 85 V) — bus voltage AND current through a low-side shunt.
It earns the position by being **dumb**: the host asks, the part measures, the part answers. No
sample stream to receive, no coefficients to apply, no processor time between questions — the poll
rate is whatever the host wants it to be, once a second or 128× a second.
Hardware averaging; µA shutdown. **The address is the SOCKET's, not the board's**: `A1` to ground on the board, `A0` from the isolator's channel A, so a socket strapped low is **0x40** and one strapped high **0x41** — two identical boards on one bus, and no more than two, which is the rule that caps a controller at two power sockets. **`APOL` = 1** (`DIAG_ALRT` bit 12) so the alarm is the high level and a dark board reads quiet; the part samples `A0`/`A1` on every transaction, so nothing latches at power-up.

**The shunt range is `ADCRANGE` = 1, ±40,96 mV, 1,25 µV an LSB**, and the shunt is chosen so the
converter's own current limit lands under 40 mV — one package, three values, **1210 metal-element
sense resistors, 1 %, ≤ 75 ppm/°C, 0,5 W, Kelvin-connected to `IN+`/`IN−`**:

| shunt | boards | rated · at the limit | drop at the limit | LSB | loss |
|---|---|---|---|---|---|
| **50 mΩ** | `G-48-S` · `G-48-U` · `G-12-S` | 0,55 · 0,7 A (`G-12-S` 0,45 · 0,7) | 35 mV | 25 µA | 25 mW |
| **200 mΩ** | `G-300-S` · `G-300-U-6` · `G-300-U-40` | 0,16 · 0,2 A | 40 mV | 6 µA | 8 mW |
| **20 mΩ** | `G-24-S` | 1,4 · 1,7 A | 34 mV | 63 µA | 58 mW |

**`VBUS` is a divider with a 100 kΩ bottom leg, or the pin straight, by the voltage** — the pin is
0–85 V at 3,125 mV an LSB, its own input impedance 0,8–1,2 MΩ, so the bottom leg in parallel with
it is 90,9 kΩ ±2 %, one scale calibration at commissioning and 1 % parts after that:

| feed | top leg | on the pin, rated · at the clamp | LSB on the feed | loss |
|---|---|---|---|---|
| **12 V · 24 V** | none — the pin straight; the clamps are 30 V and 45 V, under 85 | 12 · 24 V | 3 mV | — |
| **48 V** | **90,9 kΩ**, 1206 | 24 V · 43 V | 6 mV | 13 mW |
| **300 V** | **3× 422 kΩ in series**, 1206 — 100 V a part at the rail, 188 V at the clamp | 20 V · 38 V | 47 mV | 66 mW |

**10 nF 50 V X7R from `VBUS` to `GND` at every `INA238`**: a low-pass of 0,45 ms on 48 V and ~0,85 ms
on 300 V — the divider's Thévenin resistance into the 10 nF — so a strike's front — hundreds of nanoseconds at the tube's sparkover — arrives at the pin
as millivolts. Above 85 V the pin's own protection conducts, and the sheet's limit is then 5 mA into
any pin: behind 90,9 kΩ that is 540 V, behind 1,27 MΩ 6,4 kV, both above what the transil lets
stand — so an overvoltage stops the measurement for its duration and does not take the part.

**Conversion 4,12 ms, averaging 128 — 527 ms a sample, noise-free 16 bits on the 40 mV range** (the
sheet's Table 7-2); `ALERT` is evaluated after every conversion, so a threshold trips within 4 ms
between the host's minute reads. **The host reads every register twice and takes the value when
the two agree**, re-reading on a mismatch — the `INA238` has no CRC, and that is the whole defence
against a glitch across the isolator.

**What it sees, and what it does not:**

| | |
|---|---|
| measured | the feed as it lands on the block — bus voltage through the series divider, current through the low-side shunt, **ahead of the buck** |
| not measured | the rails the host makes behind the board |

**Reaching it:** `SDA · SCL · ALERT` on the power connector, pull-ups on the host, the address strapped in the socket on `A_SEL`. The board carries no processor; the host on the other end of the ribbon polls it.

The rest of the front:

- **Isolator: `ISO1642`** — the I²C both ways **plus one unidirectional channel each way, which
  is why it is this member of the family and not `ISO1640`/`1641`**: channel A carries `A_SEL`
  down to the `INA238`'s `A0` (`INA` pin 4 on the host side → `OUTA` pin 13 on the measuring
  side), channel B carries `ALERT` up (`INB` pin 12 → `OUTB` pin 5). Side 1 is 3,0–5,5 V —
  **it does not take 1,8 V**, so on an H7A3 board the I²C crosses a level shifter on the unit
  board, never here. Its host side runs on the host's 3,3 V — the card's over the port
  connector on a source board, the unit's rail on a unit board. **The channel defaults fall
  the right way**: an output goes low when its input is open or its input side is unpowered, so
  an unwired `A_SEL` lands the board on the base address by itself, and a dark board reads *no
  alarm* with `APOL` = 1 at the `INA238`. **`DW-16`, and it is the only package this member has** —
  reinforced: **5000 V<sub>RMS</sub> isolation, 1500 V<sub>RMS</sub> working, 10 kV surge**, which
  is what the 300 V boards want anyway. **One part carries the whole front's crossing**: the
  I²C, the address down and the alarm up, so no photorelay is fitted on a power board at all
  (`WHY.md`). **`ENABLE` drives the
  converter's `EN` directly** — same ground, pull-down at the receiving end, no opto. **At the unit end the same resistor goes to the
  input**: the board runs by itself and the unit's processor drives nothing — a population,
  like the earth strap.
- **Powering — the measuring-side island, on every power board.** The `INA238` and the far side
  of the `ISO1642` run on an isolated 3,3 V the board makes for itself from the host's 3,3 V on
  the power connector: **`SN6505B` + `750313734` + 2× `PMEG10020ELR`** — the same cell as a
  communication board's line-side supply, one winding for every island in the family, `750313734`
  being the 1:1,1 centre-tap part with **5 kV** isolation. **10 µF 50 V 1206 + 100 nF at the
  `SN6505B`'s `VCC`**; the two Schottkys on the centre-tapped secondary; **10 µF + 100 nF on the
  isolated 3,3 V**, and 100 nF at the `INA238`'s `VS` and the `ISO1642`'s `VCC2`. **`EN` tied to
  the 3,3 V — always on, never behind `ENABLE`**, so the board reads a dark port too: an output at
  0 V is a reading, not a silence. ~3 mA out, the `SN6505B`'s 1,5 mA on top; the raw rectified rail
  wanders a few hundred millivolts with load and needs no LDO — the `INA238` runs 2,7–5,5 V and the
  `ISO1642`'s side 2 3,0–5,5 V.
- **Duty:** the host reads `VBUS` and `CURRENT` once a minute into its `PORTS` frame and the
  Mayak keeps the baselines (`../core/PROTOCOL.md` §5); between reads the programmed thresholds
  raise `ALERT`. The front runs continuously, paid from the host's 3,3 V.
- **The shunt's input filter**, on every `INA238`: **2× 82 Ω in series and 100 nF across `IN+`/`IN−`**,
  the sheet's Figure 7-1 (≤ 100 Ω, 0,1–1 µF). A 10 µs corner, which also takes the flyback's
  40–500 kHz off the input. Against the part's 92 kΩ differential input it is a **0,18 % gain
  error, taken out by the commissioning calibration**. No clamp diodes: reaching the input's
  ±40 V would take 800 A through a 50 mΩ shunt that sits behind the transil and the bulk.

### Leak watch — by the voltage, and on 300 V it is mandatory

**300 V is a voltage that injures on contact, and a floating run hides its first fault**: an
insulation break to earth draws no current and trips nothing, and the second break — or a person
— closes the circuit. **So every 300 V run is watched for leakage, from its source end**, and the
48 V and lower runs are not:

| feed | the watch | why |
|---|---|---|
| **300 V** — `G-300-S` | **a second `INA238`** measuring the run against earth, below | the contact hazard |
| **48 V** — `G-48-S` / `G-48-U` | **the two ends' currents compared** — what leaves the station and does not arrive has leaked. No part added | SELV in the dry (≤ 60 V DC); only a large leak matters |
| **12 V, 24 V** — `G-12-S`, `G-24-S` | none | within 30 V DC, the limit SELV keeps even immersed |

**On `G-300-S`: one more `INA238`, two legs, nothing else.**

```
   + ──[R_A 8,84 MΩ]──┬──[R_B 7,84 MΩ]── VBUS ──(ZVBUS ~1 MΩ)── −   (the second INA238's ground)
                      │
                      the tube's centre ── the strap ── the common earthing point
```

- **The part:** a second `INA238`, **`A1` to `VS` and `A0` from `A_SEL`** — **0x44 / 0x45**, clear of
  the first one's 0x40 / 0x41 on the same crossing. **`VBUS` only**; its shunt inputs are tied.
  **Its `ALERT` is not wired** — `APOL` = 1 makes two open-drain alarms on one line mask each
  other, and a leak is slow: the host polls it.
- **The legs:** `R_A` **4× 2,21 MΩ in series**, `R_B` **4× 1,96 MΩ in series** — ordinary 1206
  thick film, 1 %, E96, 37 V a part at the rail and 163 V at a gap's 650 V sparkover, inside a
  1206's 200 V. **Both legs total 8,84 MΩ**, so the tube's centre sits at half the feed when the
  insulation is sound. **The legs sit across the tube's own gaps and never see more than a gap's
  sparkover** — `R_A` across the upper gap, `R_B` and the pin across the lower — and the tube is
  the spark gap: whatever the run does against earth, a leg sees one gap's sparkover at most.
  **10 nF from `VBUS` to `GND`**, τ ≈ 9 ms with the 7,84 MΩ ‖ 1 MΩ, so the sparkover's front
  never reaches the pin; the pin sits at 74 V at 650 V, under its 85 V, and above that its
  protection clamps at 83 µA against the sheet's 5 mA. The value is not halved for range: at
  4,42 MΩ the sparkover would land 147 V on the pin and want a clamp whose leakage is percent of
  the 34 µA measuring current; 28 mV an LSB at the centre against 5 V per 10 MΩ of leak is 180
  steps and enough.
- **What it reads, at 300 V:**

| | `VBUS` |
|---|---|
| sound insulation — the centre at 150 V | **17 V** |
| a 100 MΩ leak on + | +0,7 V |
| a 10 MΩ leak on + | 22 V |
| + shorted to earth | 34 V |
| a leak on − | the mirror, towards 0 |
| a gap striking at ~650 V | ~74 V — inside the pin's 85 V, no clamp |

  **The measuring current is 34 µA at most**, into a dead short.
- **Calibrated by a baseline, not by an accuracy**: `ZVBUS` is 0,8–1,2 MΩ, so the healthy ratio
  of the second part's `VBUS` to the first part's is written at commissioning and the alarm is a
  drift from it. **The leakage itself** is Kirchhoff at the centre — what arrives through `R_A`
  and does not leave through `R_B` flows to earth — and gives each pole's insulation resistance.
- **One reading covers the whole run**: the winding's secondary, the cable and the far end's
  input are one galvanically connected domain, so a break anywhere on it moves the centre.
- **At a remote Argus the strap is absent** and the centre has nothing to measure against; the watch
  is the source end's function. A remote Argus's 300 V runs fall back to the 48 V method.
- **On a sea return the run is not floating** — its return is the sea, on purpose — so the centre
  has no insulation to measure and the run falls back to the 48 V method too: a breach of the
  negative conductor is current into the sea, and it shows as the two ends' currents parting.
- **How the readings reach the head:** the host carrying the `G-300-S` — a card, a Palatine —
  sends the second `INA238`'s raw `VBUS` per 300 V port in its **`PORTS` frame, `kind` 7**, once a
  minute with its sockets' `VBUS` and `CURRENT`, and the Mayak holds the commissioning baseline
  and watches the drift; **the 48 V comparison is made at the Mayak** from the same `PORTS`
  frame (what left the station) and the unit's `REPORT` frame (what arrived), both raw `INA238`
  registers once a minute (`../core/PROTOCOL.md` §5). **12 V and 24 V are watched by current
  alone** — a low-side shunt on one potential sees a short and nothing subtler, and that is the
  accepted state: a port that drops is read with `GET PORTS`, and the socket at the current limit
  names the converter that shorted.

## Protecting the feed — what an eFuse would have done, and what does it instead

**An eFuse was worked through and is not fitted.** It is a series semiconductor in the supply
path: it adds its own failure rate, its own R_DS(on) and its own quiescent draw, and **it can
fail closed as easily as open** — so on a feed whose converter already holds a hard ceiling and
folds the output back (*What a short does*, above), it protects against a fault the topology
already bounds and introduces one the topology did not have. Detection, action and the coarse
stop all exist elsewhere, and each of them is a part that is fitted for another reason as well:

| the job | what does it | where |
|---|---|---|
| **bound the fault** | the flyback's own switch limit and foldback — the fault is not sustained and the arithmetic is worked in *What a short does* | every island and the feed cell |
| **detect it** | **INA238**, bus voltage and current at the board, with an **open-drain `ALERT`** on programmable thresholds — shunt over-voltage is the overcurrent trip, and bus under-voltage catches the collapse | both ends of every power board (*Feed telemetry*) |
| **report a converter's own trouble** | **`PGOOD`** on the buck — open-drain, and it is a **window on the output, not the converter's opinion of itself**: it trips **above 108 %** of the regulation target and **below 91 %** (min/max 104–111 and 89–94,2), with 2,4 % / 3,3 % of recovery hysteresis, so a broken track or a shorted load reads as a fault and not as a healthy regulator. It needs a pull-up, stays valid down to `V_IN` **1,5 V** with `EN` low, and takes a **6 ms** start-up delay before it first flags high. **The flybacks have no fault output at all** — neither `LT3748` nor `LT8316` gives one — which is why the INA ahead of them is what sees their trouble | every board that carries a buck |
| **act** | the block's **`EN`**, driven by the host: the port kill the rogue-unit ladder already pulls. **It sits outside the supply path**, which is the whole point | every port board |
| **protect the copper** | **the installation's fuses** — PV → MPPT, MPPT → pack, pack → BMS, and the enclosure's fuse field behind the BMS, one fast-acting cartridge to a board, rated to open only on a real fault. **No fuse on a board**: its supply's current limit is its fuse | the cabling (`../daedalus/CONSTRUCTION.md`) |

**The fuses are deliberately not tuned to the working point.** Two reasons and they pull
the same way: a fault down a long run is limited by its own impedance, so a fuse set close to the
load current may never clear at all and gives false confidence; and a fuse set that close
nuisance-trips on inrush. **The fuse protects the wire. The precise per-port limit lives in
firmware**, read from the INA and executed with `EN` — settable per port in the host's register
(`0xFF20 + port`, `../bifrost/FIRMWARE.md` §10).

**A build that substitutes parts must revisit the firmware limits** — the thresholds are written against these
converters and these shunts.

## The buck cell — `LMR43610R3RPER`, the worksheet

One cell for every buck position on the station, drawn once and reused.

| | |
|---|---|
| the part | **`LMR43610R3RPER`** — 1 A, VQFN-HR 9-pin. The order code reads letter by letter: `1` = 1 A · **`R` = RT trim**, frequency set by a resistor · **no `S` = spread spectrum OFF from the factory** · `3` = 3,3 V fixed/adjustable · `RPER` = tape and large reel. Pins: `RT` · `PGOOD` · `EN/UVLO` · `VIN` · `SW` · `BOOT` · `VCC` · `VOUT/FB` · `GND`. **`RT` and `MODE/SYNC` share pin 1 and are alternatives** — taking `RT` means there is no `SYNC` input on the board at all, which is what this project wants |
| the input | **3,6 V to start, 3,0 V to keep running**, continuous to **36 V**. Rising threshold 3,2 / 3,35 / 3,5 V, falling 2,45 / 2,7 / 3,0 V |
| the output | the divider on `VOUT/FB` against a **1,00 V ± 1 %** reference (`V_IN` 3,0–36 V, FPWM). Fixed-output variants are a factory option, so **every position keeps its divider** |
| frequency | **one resistor on `RT`, anywhere in 200 kHz – 2,2 MHz**: `RT [kΩ] = 18286 / f[kHz]^1,021` (its equation 1); `RT` to `GND` is 2,2 MHz, to `VCC` 1 MHz. **The house value is 7,50 kΩ → 2,08 MHz**, above Tesla's band. **The R variant is auto mode and nothing else** — pin 1 is `RT`, there is no `MODE`, and light load is PFM by the order code. **A rail that must stay in forced PWM is not this part**: PFM's burst comb lands in the audio band, so a listening board's rails are `TPS629206`s with forced PWM on their `MODE/S-CONF` resistor |
| power good | **`PGOOD`, open drain**, pulled up to a logic rail. Trips above **108 %** of the regulation target and below **91 %** (104–111 / 89–94,2 over the range), 2,4 % / 3,3 % recovery hysteresis, `R_ON` 100 Ω, valid down to `V_IN` 1,5 V with `EN` low, 6 ms of delay before it first flags high. **A window on the output, not the converter's opinion of itself**, so a broken track or a shorted load reads as a fault |
| quiescent | **250 nA** in shutdown (0,25 µA typ / 1 µA max at the `VIN` pin); **1,5 µA at `VIN` running, no load, auto mode** (13,5 V in, fixed 3,3 V out) — an adjustable output adds its divider's own current |
| `VCC` · `BOOT` | **1 µF** from `VCC` to `GND` · **100 nF, ≥ 10 V, from `BOOT` to `SW`** — its sheet, §9.2.2.6–7 |
| the inductor | `L = (V_IN − V_OUT)/(f·K·1 A) · V_OUT/V_IN` with `K` 0,3–0,4 and the DEVICE's 1 A where the load is smaller (its equation 7): **4,1 µH at 12 → 4,0 V and 2,08 MHz, so 4,7 µH standard**; the ceiling is the 10 % ripple rule, ~12 µH. **`I_SAT` ≥ 2,1 A at the least, the high-side peak limit's maximum** (1,4 / 1,8 / 2,1 A), and the house part carries ≥ 3,5 A so that the `LMR43620` takes the same line — so a 2,5 × 2,0 mm 2R2 at 1,85 A is a filter part, not this buck's |
| current limits | high-side peak 1,4–2,1 A; the auto-mode minimum peak 0,17–0,4 A and the zero-cross 30–135 mA are what set where PFM begins: a load whose valley falls under the zero-cross limit is in PFM |

**The same cell one size up — `LMR43620`, 2 A — is the card's down-port 3,3 V** (12 V → 3,3 V for the
boards on its four down ports, its `EN` on no pin; the processor and the up port sit on a `TPS629206` each; its draw is ~420 mA with four optical
ends, 700 mA at their ceiling and ~300 mA with four copper, and at the card's own idle it
runs in PFM like the 1 A part).
Same pinout class, same `RT`. **It takes the house 4,7 µH, `I_SAT` ≥ 3,5 A — the part every `R3` position takes — not the
2 A design's smaller one**: at 100–200 mA a 2,2 µH inductor's 0,6 A ripple would reverse the
inductor current every cycle for nothing. What changes is `I_SAT` — **≥ 3,5 A**, because the
part's current limit is ~3 A and an inductor below it saturates before the limit acts. At the
base draw the 2 A part costs 2–3 points of efficiency (~20 mW) for its bigger gates; at 1 A it
is the better part.

**The small cell — `TPS629206`, 600 mA — is the card's processor rail and its up-port rail, a
unit whose whole board fits it — Quake — and both rails of every H7A3 board**: on the card's 12 V, behind the input's `5.0SMDJ14A`, which conducts from 15,6–17,2 V — under its 18 V absolute maximum — where its 17 V ceiling
is inside the 16,2 V corner; at a unit, on the unit power board's regulated 12 V, whose one
`5.0SMDJ12A` breaks down at 13,3–14,7 V when the load falls below the cell's floor. **It stands on no MOD**: a MOD's 12 V is behind the basic set's
transils, which clamp at ~29 V, so every buck there is the `LMR43610` — Ceres, Sakura and Babel to
3,3 V, Pluvius twice, 3,3 V and the 5,5 V that feeds its `TPS7A4701`.
3–17 V in (the 10–15 V rail with margin), **`V_OUT` 0,6–5,5 V** by external divider against 0,6 V,
2,5 or 1 MHz and FPWM or auto by one resistor on `MODE/S-CONF`, **40 ns minimum on-time**,
**1,1–1,7 A high-side current limit**, 4 µA quiescent, `PG`, SOT-5X3 (SLVSGE2 and the `TPS629206`
sheet; the family is pin-to-pin, 300 mA / 600 mA / 1 A).

| position | value |
|---|---|
| `MODE/S-CONF`, 1 % to ground | **9,31 kΩ** — external feedback, forced PWM, 2,5 MHz · **22,1 kΩ** — external feedback, forced PWM, 1 MHz · **17,8 kΩ** — external feedback, auto, 1 MHz (the card, where nothing listens). No output discharge on any |
| the divider, `R2` 137 kΩ 1 % on every position | `R1` **274 k → 1,800 V** · **619 k → 3,311 V** · **787 k → 4,047 V** · **1,07 M → 5,286 V** |
| the inductor | **2,2 µH, shielded, `XGL3530-222`** — the sheet's value |
| `C_OUT` · `C_IN` · `VCC` · `BOOT` | 3× 10 µF 50 V 1206 + 100 nF · 4,7 µF 50 V 1206 + 100 nF · 1 µF · 100 nF |
| `PG` | to the processor through 100 kΩ, where the board reads it |

**The 1,8 V runs at 1 MHz, everything else at 2,5 MHz where it listens**: 12 → 1,8 V at 2,5 MHz is
49 ns of on-time at 14,7 V in, against 40 ns. The family's efficiency, read from the
`TPS629203` sheet, 12 V in, 5 V out:

| load | FPWM 2,5 MHz | auto 2,5 MHz | FPWM 1 MHz | auto 1 MHz |
|---|---|---|---|---|
| 10 mA | ~37 % | ~83 % | ~30 % | ~91 % |
| 50 mA | ~76 % | ~85 % | ~72 % | ~92 % |
| **100 mA** | **~85 %** | ~86 % | ~84 % | **~93 %** |
| 300 mA | ~92 % | ~90 % | ~93 % | ~93 % |

In FPWM at 12 V it measures ~2,7 MHz, not 2,5 (its figure 9-5). **The card runs it in auto** — nothing on the card listens. Why no buck does better at 10 mA: at light load the loss is gate charge,
`C_OSS` and the controller — fixed per cycle, independent of the load — and a bigger MOSFET makes
it worse, not better; the only levers are frequency and pulse skipping, which is what auto mode
is.

**All of it is ceramic — no electrolytic sits at any buck.** Stored energy scales 1/f, so a
megahertz-class rate turns the 47 µH-class inductor and its electrolytics into single-digit µH and
tens of µF of X7R, and the one aging part is gone from boards potted for years. (The far-end feed
bulk is the deliberate exception, its own section.)

**The passive set, from its sheet (SNVSBY5B, Tables 9-2 and 9-6, §9.2.2).**

| position | value | reason |
|---|---|---|
| **the feedback divider**, `R_FBT` / `R_FBB`, 1 % | **`R_FBB` 12,1 k on every position**; `R_FBT` **28,0 k → 3,31 V** · **36,5 k → 4,02 V** · **29,4 k → 3,43 V** (the Mayak, its backup cell's float) · **54,9 k → 5,54 V** (Pluvius) | `V_OUT` = 1,00 V × (1 + `R_FBT`/`R_FBB`), the pair's parallel value 8,5–9,9 kΩ, inside the sheet's 5–10 kΩ window (its equation 5; the sheet's own 6 V row uses 12,1 k). **One bottom resistor, and every rail below 5 V is 3,3 or 4,0 V** — 4,0 V sits 0,7 V over a `TPS7A2033` |
| `C_FF` | **22 pF C0G 0603** across `R_FBT` | the sheet's value at every rate and output — on every position, the MODs included |
| **the inductor — two parts** | **4,7 µH, shielded, `I_SAT` ≥ 3,5 A** on every `R3` position at 2,08 MHz, 3,3–5,5 V, and on the card's `LMR43620` · **10 µH, shielded, `I_SAT` ≥ 2,1 A** on a rail that feeds a measurement and must stay in CCM — Ceres, Sakura, Pluvius's 3,3 V | ripple `K` at the device's 1 A: 4,7 µH at 2,08 MHz is 0,25–0,30; 10 µH at 1 MHz is 0,24–0,30 — the sheet's 20–40 % — and at 2,08 MHz 0,12, the MODs' deliberate low ripple under the ~12 µH ceiling. The 3,5 A part on the 1 A positions buys one stock line |
| **`C_OUT`** | **3× 10 µF 50 V X5R 1206** (basic `C13585`) + **100 nF 50 V 0603** | ~21–25 µF effective at 3,3–5,5 V against the sheet's 2× 10 µF minimum; the house's one 10 µF part, 50 V by the low-rail rule |
| **`C_IN`** | **4,7 µF 50 V X7R 1206** (basic `C29823`) **+ 100 nF 50 V X7R 0603** (basic `C14663`) tight at `VIN` | the sheet's 4,7 µF minimum, rated at twice the input: a MOD's input stands behind a transil clamping at ~29 V, and one 50 V part serves every position |
| `VCC` · `BOOT` | **1 µF 50 V X7R 0805** (basic `C28323`) · **100 nF 50 V 0603** | the sheet's values, in the house's parts |
| **the minimum times** | `t_ON(MIN)` 75 ns and `t_OFF(MIN)` 85 ns, maximum | **at 2,08 MHz** 3,3 V from the 16,2 V corner is a 98 ns on-time, and 5,5 V from 10 V leaves a 202 ns off-time; **at 1 MHz** the margins double. Only a surge on a MOD's input — 3,3 V from 29 V wants 55 ns — pushes a cell under its on-time, and there the part folds its frequency back and keeps regulating (§8.4.3.4) |
| **the light-load boundary** | **`R3` is PFM below the zero-cross limit** | a MOD and every H523 board listen to nothing and keep PFM's efficiency; a rail on a listening board is not an `LMR43610` — it is a `TPS629206` in forced PWM |

## The battery-side feed cell — LT3748 + an external FET, the worksheet

**One cell, two source boards.** `G-48-S` runs `750310988` at 1:4,42; `G-300-S`
carries `750310349` at 1:10 for both of its loads — the same controller, FET and topology on
each, and only the winding, the shunt and the rectifier differ. Every number
below is read from a datasheet or computed, never remembered.

**The controller, read.** `LT3748` — **5–100 V in**, external N-FET, gate driver **1,9 A source and
sink**, boundary mode, **primary-side sensing** so no opto and no third winding, output set by two
resistors against the turns ratio, soft-start on a capacitor, `EN/UVLO` with programmable
hysteresis, MSOP-16 with four leads removed.

| what the sheet gives | value |
|---|---|
| `SENSE` threshold, full scale | **100 mV** (95/100/105 typ, 90/100/110 over temperature) |
| `SENSE` threshold, minimum | **15 mV** at `V_C` = 0 |
| max-to-min ratio | 5,2 / 6,6 / 8,2 |
| `SENSE` overcurrent | 115 / **130** / 145 mV |
| `EN/UVLO` threshold, rising | 1,19 / **1,223** / 1,25 V |
| `EN/UVLO` hysteresis current | 1,9 / **2,4** / 2,9 µA below the threshold |
| settle and sample time | **400 ns** |
| minimum gate on-time | **250 ns** |
| minimum load | the frequency floor is **≈ 42 kHz** and the sheet quotes **≈ 2 % of maximum**; that 2 % holds only where a cell's full-load frequency already sits at the floor, so **the family computes it per board** (*No burst, and therefore a minimum load*) |

**Its frequency ceiling is derived, not quoted.** The sheet names the mechanism — minimum gate
on-time 250 ns and minimum off-time 700 ns — and gives no maximum. `1/(250 + 700 ns)` is
**≈ 1,05 MHz**, and that is a clamp reached at light load, not an operating point.

**The turns ratio must be held to ±1 %.** The part infers the output voltage from the flyback pulse
on the primary, so a ±5 % ratio is more than ±5 % of output regulation. Every winding in this
family is specified at ±1 % or better, and the catalogue parts are.

**Why the family runs boundary mode and not continuous.** The secondary current returns to zero
every cycle, so resistive drops do not become load-regulation error — which is what lets the output
be sensed on the primary at all. It also lets the transformer be smaller and it cannot oscillate
subharmonically.

**The frequency falls as the load rises.** `f = 1/(L·I_pk·(1/V_IN + 1/V_refl))` and `I_pk` is
proportional to the delivered power, so **full load is the slowest the converter ever runs**. That
is the opposite of a fixed-frequency part and it is why the declared rating is chosen against the
frequency criterion rather than against the load.

**Switch and rectifier stress, per position:**

| | `V_DS` on the FET | rectifier reverse |
|---|---|---|
| `G-48-S`, 1:4,42 | **26 V** | 114 V → `V5N22-M3`, 220 V |
| `G-300-S`, 1:10 | **45 V** | **450 V** → `US3M`, 1000 V |
| `G-48-U`, 2,5:1, 12 V out | 82 V running, **119 V at the transil clamp** | 32 V → `V10P10-M3`, 100 V |

**Every `LT3748` position therefore takes one FET, the `ISC165N15NM6`: 150 V, `R_DS(on)` specified
at `V_GS` ≤ 7,5 V, `I_D` ≥ 10 A.** `R_DS(on)` is not a criterion below ~25 mΩ — at
`G-300-S`'s worst case the difference between a 7 mΩ part and a 20 mΩ one is **430 mW on ~47 W**,
against a transformer that takes half the loss budget on its own.

**Where the loss actually goes**, at the family's working points: transformer ≈ 51 %, MOSFET 17 %,
snubber 13 %, sense shunt 7 %, PCB 6 %, rectifier 5 %. **Buy the transformer, not the FET.**

### The leakage clamp — RCD, on all four cells

**Every `LT3748` cell carries an RCD clamp.** Unclamped, the winding's leakage rings into the
FET's ~330 pF of `C_oss` and the drain overshoots by `I_pk·√(L_LK/C_OSS)`: **~300 V on `G-48-S`,
~250 V on `G-300-S`, ~140 V on `G-48-U`, ~310 V on `G-24-S`** — each past the 150 V part. An RC snubber would burn
~1,7 W at light load, where the frequency is highest; an RCD's loss follows the load, because the
leakage energy is a fixed share of the input — `L_LK/L_PRI`, 1,4 % · 2 % · 3,6 % · 5 %.

**The circuit is the sheet's Figure 7**: `D_CL` from the drain to `C_CL`, `C_CL` ∥ `R_CL` back to
`V_IN` behind the bulk. `R_CL` sets the clamp voltage at the rated load:

```
    P_CL  =  ½ · L_LK · I_pk² · f  ·  V_CL / (V_CL − V_OR)          R_CL  =  V_CL² / P_CL
```

| | `G-48-S` | `G-300-S` | `G-48-U` | `G-24-S` |
|---|---|---|---|---|
| `V_OR`, the reflected voltage | 11 V | 30 V | 31 V | 24,7 V |
| `V_CL` rated · at the 110 mV corner | 50 · 58 V | 90 · 112 V | 60 · 83 V | 100 · 109 V |
| drain, worst | **78 V** at 20 V in | **132 V** at 20 V in | **133 V** at 50 V in, **151 V** with the surge residue on the bulk | **129 V** at 20 V in |
| `R_CL`, 2× 2512 in series, 2 W class | 2× 2,43 kΩ | 2× 2,67 kΩ | 2× 845 Ω | 2× 2,05 kΩ |
| loss in `R_CL`, rated · corner | 0,52 · 0,68 W | 1,5 · 2,35 W | 2,1 · 4,1 W | 2,4 · 2,9 W |
| taken from the output | 0,12 W | 0,5 W | 1,1 W | 0,6 W |
| **`C_CL`** | **2,2 µF 100 V X7R 1210** | **100 nF 1 kV X7R 1812** | **2,2 µF 100 V X7R 1210** | **100 nF 1 kV X7R 1812** |

- **`C_CL` is a part the board already carries** — the 48 V bulk ceramic on `G-48-S` and `G-48-U`,
  the 300 V one on `G-300-S` — so the clamp adds no part type. That is why `G-48-S` clamps at 50 V:
  it keeps the 100 V part at 1,75× and costs 0,06 W. `G-48-U` runs it at 1,2× at the corner.
  **`G-24-S` clamps at 100 V on `G-300-S`'s 1 kV part**: `750311592`'s leakage is 0,4 µH max, 5 % of
  its primary and the largest share in the family, and the leakage energy alone is 1,8 W at full
  load — at the 100 V ceramic's 57 V ceiling the clamp would burn 3,2 W; at 100 V it burns 2,4 W
  and takes 0,6 W from the output.
- **`D_CL` is `V5N22-M3`**, the 220 V / 5 A Schottky `G-48-S` already carries as its rectifier. The
  sheet asks for a Schottky: a slow PN diode lets the spike through before it clamps. It blocks at
  most 151 V and carries `I_pk` for tens of nanoseconds.
- **The corner is a fault, not a load.** The 110 mV column is the shunt's own limit, which the
  `INA238` threshold keeps the board from reaching in service; an overload that gets there trips
  `ALERT` and the source's `ENABLE` within milliseconds. `G-48-U`'s 4,1 W is past its two 2 W
  resistors for that time only, and its 151 V — an overload and a surge residue at once — is 1 V
  into the FET's avalanche, which the `ISC165N15NM6` is rated for, 100 % tested, `E_AS` 123 mJ.
- **The declared watts do not move.** The loss budgets already carry the leakage energy; the clamp
  adds only the magnetising energy it takes while the leakage current resets — the last row — and
  `G-48-S` stays ~26 W, `G-300-S` ~47 W, `G-48-U` ~25 W.
- **Regulation is not disturbed.** The reset takes 20–55 ns against the sheet's 200 ns, and at
  10 % load `V_CL` still sits at 21 · 43 · 36 · 42 V, above `V_OR`, so the clamp is off during the
  plateau the part samples.

### The power stage — gate, input filter, capacitors, compensation

**The capacitors are chosen for the fewest types, because every extended part is a setup fee on
every assembly order** (~$3 at JLCPCB); a basic-library part costs nothing to set up. A position
takes a multiple of one type:

| type | library | where | price, 100 pcs |
|---|---|---|---|
| **hybrid polymer 47 µF 50 V**, Panasonic ZA class (`EEHZA1H470P`), 8 × 10,2 mm SMD — 30 mΩ, 1,8 A ripple at 100 kHz, 105 °C / 10 000 h | extended | bulk on a land board's 12 V node | $0,35 |
| **2,2 µF 100 V X7R 1210** | extended | every 48 V node, and `C_CL` on the 48 V cells | $0,069 |
| **100 nF 1 kV X7R 1812** | extended | every 300 V node, and `C_CL` on `G-300-S` | $0,15 |
| **1 µF 500 V X7R 2220** | extended | `G-300-U-6`'s bank | $0,49 |
| **1 µF 630 V polypropylene film**, 15 mm pitch | extended | `G-300-U-40`'s bank | $0,11 |
| **10 µF 50 V X5R 1206** (`C13585`) | basic | HF and `V_IN` on 12 V; the 12 V bulk on a pressure board | $0,20 |
| **4,7 µF 50 V X7R 1206** (`C29823`) | basic | `INTVCC` | $0,13 |
| **1 µF 50 V X7R 0805** (`C28323`) | basic | the 3,3 V parts | $0,031 |
| **100 nF 50 V X7R 0603** (`C14663`) | basic | `C_SS` | $0,012 |
| **4,7 nF 50 V X7R** · **100 pF 50 V C0G**, 0603 | basic | the compensation | < $0,01 |

**A board carries one or two extended capacitor types and no more**: `G-48-S` the hybrid and the
100 V ceramic, `G-300-S` the hybrid and the 1 kV ceramic, `G-48-U` the 100 V ceramic alone,
`G-300-U-6` the 500 V ceramic alone, `G-300-U-40` the film and the 1 kV ceramic.

**The 100 V ceramic is the 48 V part although it keeps only ~55 % at 48 V**: ~1,2 µF of 2,2 is
$0,058 an effective microfarad in 8 mm², against $0,12 in 14 mm² for 1 µF 250 V 1812 and $0,35
for 200 V. The hybrid carries no DC-bias loss and brings the ESR that damps the input filter;
its SMD range stops at 80 V, so it serves 12 V only. **No hybrid on a pressure board** — it takes ceramic alone.

| | `G-48-S` | `G-300-S` | `G-48-U` |
|---|---|---|---|
| **`R_GATE`** | 10 Ω | 10 Ω | 10 Ω |
| **`L_IN`** | 1,5 µH shielded, `I_SAT` ≥ 8 A | the same | — |
| **12 V bulk** | input: 2× hybrid + 2× 10 µF 50 V | input: **3× hybrid** + 2× 10 µF 50 V | output, at the rectifier: **8× 10 µF 50 V** — a pressure board, no hybrid |
| **48 / 300 V bulk** | output: 6× 2,2 µF 100 V | output: 4× 100 nF 1 kV | input: 3× 2,2 µF 100 V |
| `V_IN` / `INTVCC` decoupling | 10 µF 50 V / 4,7 µF 50 V | the same | **2,2 µF 100 V** / 4,7 µF 50 V |
| **`R_C` + `C_C`** | 24,9 kΩ + 4,7 nF ∥ 100 pF | the same | the same |
| **`C_SS`** | 0,1 µF | the same | the same |

- **`R_GATE` 10 Ω.** Boundary mode turns the FET on at zero current, so only the turn-off costs:
  `Q_G` 14,8 nC gives an ~8 ns edge and 0,06 / 0,4 / 0,6 mW of switching loss. The slower edge is
  bought for EMI; the leakage clamp takes the same energy either way.
- **`L_IN` and the 12 V bulk are one filter.** The input current is a triangle to 12 A at
  41 kHz–1 MHz, inside Tesla's band, on a wire every board shares. With 1,5 µH and ~100 µF the
  corner is ~13 kHz — **~20 dB at 41 kHz** (`G-48-S` at full load), ~40 dB at 145 kHz. The
  converter's input is a negative resistance, −V²/P: **−4,3 Ω on `G-48-S`, −2,4 Ω on `G-300-S`**
  at 11 V. The hybrids' ESR damps the filter's peak to **~0,7–0,8 Ω**, three times under it.
- **Hybrid count is ripple current.** 3,1 A rms on `G-48-S`'s input and 3,9 A on `G-300-S`'s;
  the ceramics take under a tenth of it at these frequencies, so two hybrids (3,6 A) carry the
  first and three (5,4 A) the second. **`G-48-U` is a pressure board** and takes its 3,6 A rms on
  eight 10 µF 50 V X5R 1206 — the basic-library part — ~4 µF each at 12 V, ~32 µF, 0,45 A a part.
- **Ripple on the fed side:** `G-48-S` ~7 µF effective, **~4 %** at rated and 6 % at the 110 mV
  corner; `G-300-S` ~0,28 µF, **~1,5–2 %**; `G-48-U`'s 12 V ~2 %.
  The far end regulates again, so 1 % buys nothing.
- **`G-48-U`'s input needs no damping part.** The run is its own: `R_line` ~24 Ω/km against
  √(L/C) ~11 Ω, and stability asks `C > L/(R_line·|R_neg|)` = 0,27 µF a kilometre against the
  3,6 µF fitted, with `R_line` under the −92 Ω of the load to ~3 km, where the watts end anyway.
- **Compensation and soft start are the sheet's worked example**, within its rule of `C_C` ≥ 1 nF
  and `R_C` ≤ 50 kΩ, the 100 pF across for noise; the sheet's own last step — a load step at both
  ends of the input range — sets the final pair on the bench. 0,1 µF of `C_SS` ramps `V_C` at
  0,05 V/ms.
- **`G-48-U`'s gate drive comes off 48 V through the part's LDO**: 15,5 mA × 41 V ≈ **0,64 W** in
  the `LT3748` at the 1,05 MHz light-load clamp, 0,28 W at full load — ~+26 °C over the board in
  the MSOP-16E. The winding has no third winding to offload it, and the die stands it.

### The feedback network — `R_FB`, `R_REF` and `R_TC`, read from the sheet

**`LT3748` samples the flyback pulse on the primary, so the output voltage is set by a resistor
ratio and the part's own bandgap** — not by the winding, and not by an opto:

```
              R_REF · N_PS · [ (V_OUT + V_F) + V_TC ]
    R_FB  =  ————————————————————————————————————
                              V_BG
```

| | |
|---|---|
| **`R_REF`** | **6,04 kΩ** — **the part is trimmed and specified at this value**, so it is not a free choice; several percent of variation is accepted and the window is 5,76–6,34 kΩ |
| **`V_BG`** | **1,223 V**, the internal bandgap |
| **`V_TC`** | **0,55 V** |
| **`N_PS`** | the effective **primary-to-secondary** turns ratio |
| **`V_F`** | the output diode's forward drop — **the one input that comes from the rectifier's own sheet**, per board |
| **`R_TC`**, first order | **`R_FB` / `N_PS`** — the sheet's own approximation, which assumes the diode's and `V_TC`'s temperature coefficients are equal. The exact form is `R_TC = (R_FB/N_PS) · 1,85 mV/°C / (ΔV_OUT/ΔTEMP)` |

**One check falls out of the equation and confirms it:** the sheet wants the average current through
`R_FB` during the flyback to be **≈ 200 µA**, and that current is `(V_OUT + V_F)·N_PS / R_FB`. Every
value the equation gives lands there by construction.

**The sheet's procedure does not end at the equation.** It says to build the first iteration with
the final transformer, diode and MOSFET, **measure `V_OUT`, and re-evaluate**:
`R_FB(NEW) = V_OUT(desired)/V_OUT(measured) · R_FB(OLD)`. With `R_FB` and `R_TC` then fixed,
board-to-board regulation is **typically under ±5 %** at 1 % resistors and 1 % winding matching. **A
changed transformer, diode or MOSFET, or a dramatically altered layout, moves `V_OUT` again.**

`V_F` is each board's rectifier's, at its average current, from its sheet. The compensation and the soft start are set (*The power stage*, above).

## The fibre front — the actual parts

**The fibre front is an off-the-shelf 1×9 transceiver module — no discrete optics build.**
One module type covers every link in the network, in two uses:

- **one section of a module** — the one-way clock fibre: the transmit section powered at the
  sending end, the receive section at the receiving end, the other half dark (below).
- **1×9 run FULL duplex** — the data link. Both directions are live at once, so unlike the 485 pair
  it replaces there is **no direction to switch**: no DE, no turnaround, no carrier-detect trick,
  and **the link delay stops mattering** because nothing waits for the other end to finish. What
  glass drops is not the duplex — copper runs full duplex too — it is the **multidrop**: an
  optical spur is point-to-point, so the shared TX pair's DE gating has nothing to gate.

**So there are exactly two optical transfers, and they are not alike:** the **data** link is one
module run full duplex; the **clock** is one-way, continuous by definition, and therefore the one
link that can never be duty-cycled — its module draws its full current always. **Two modules at each
end.**

**The one-way clock is a population, not a property — which is what makes the channel
reversible.** Both sections exist in the part; the one-way build simply leaves the far one
unpowered. Power both and channel B is two-way; **the socket's strap** then points it inward to carry
**PPS** instead of outward to carry the clock — the GPS build, identical in behaviour to the
copper one (*The reversed channel*). Nothing above the module changes, and **no laser is ever
turned round**: a two-way channel is one with a transmitter at each end.

**An RX-only or TX-only part does not have to exist**, which is just as well because it largely does
not: these modules carry **separate `VccT` and `VccR` supplies with separate grounds**, so the
one-way link is built by leaving the other half unpowered — VccR only at the receiving end, VccT
only at the sending end. The datasheet quotes I_TX + I_RX as one figure; the rails are sized on a
~75/25 mA split between the laser and the receiver, and leaving a section dark is a whole section
saved, not a trim.

**Idle must map to laser-dark.** A DC-coupled TTL module puts the idle level straight on the laser
and a UART idles HIGH, so the wrong polarity leaves the laser burning continuously and bursting
saves nothing. One inversion decides half the far end's optical budget.

**The land module: Optcore `OPT10-3110xxxR`.** 1310 nm, 10 km, single-mode, TTL in and out,
`SD` low when there is no light. **Read from the sheet:**

| | |
|---|---|
| **rate** | **0 to 10 Mb/s — it starts at DC**, which is what the idle-maps-to-dark rule needs |
| supply | single **+3,3 V** (3,14–3,47) or +5 V |
| **current** | **`I_TX + I_RX` = 100 mA max, both sections together** — the sheet's figure, the receiver ~25 mA and the laser ~75 of it; what an end costs on the rail is below: **~105 mA at a sending end and ~55 mA at a receiving end on average, 175 and 125 mA at most** |
| transmit | `P_out` **−7 / −5 / −3 dBm** · centre 1260/1310/1360 nm · spectral width 3 nm |
| **extinction ratio** | **9 dB** — *a different parameter from the link budget, and they happen to be the same number here* |
| receive | sensitivity **−16 dBm** · **overload 0 dBm** · LOS de-assert −17, assert −30 dBm |
| **link budget** | `−7 − (−16)` = **9 dB**, of which a 2 km spur spends about 2 |
| safety | Class 1 per IEC 60825-1 |

**The order code decodes, so read it rather than remember it:** `OPT10-3110` **· voltage ·
connector · grade ·** `R`.

| field | |
|---|---|
| voltage | **`3`** = 3,3 V · `5` = 5 V |
| connector | `S` = duplex SC · `T` = duplex ST · `F` = duplex FC · **`P`** = FC pigtail |
| grade | `C` = 0…+70 commercial · `E` = −10…+85 extended · **`T`** = **−40…+85 industrial** |

**This project fits `OPT10-31103PTR`** (3,3 V, FC pigtail, industrial) **or `OPT10-31103STR`**
(the same in a duplex SC receptacle, which is the one a boxed unit with a bulkhead wants).
**The grade letter is read before the order goes out** — a `C` in that position is the commercial
part and is not what this project fits.

**`OPT10-3110xxxR` is DUPLEX and nothing else.** Its ordering table lists SC, ST, FC and
FC-pigtail, all duplex, in three temperature grades — **there is no BiDi row** — so a two-way
channel costs **two fibres** and a full link with both channels two-way costs **four**. At 2 Mb/s
BiDi is in the catalogue (below).

**The 2 Mb/s module: Optcore's long part, and it is the only 2 Mb/s part fitted.** Two sheets
read: `OPT2-xx40(60/A0)xxxR` (duplex) and `OTB2-xx40(60/A0)xxxR` (BiDi). Both are **1×9 SIP, TTL
in and out, `SD` low when there is no light, single +3,3 V or +5 V, `I_TX + I_RX` = 100 mA, and
0 to 2 Mb/s — starting at DC**, which is what the idle-maps-to-dark rule needs. **Duplex and BiDi
are the same part** — every number below is shared and only the wavelength plan differs:

| | **the long part — duplex or BiDi** |
|---|---|
| wavelength | **1550 duplex · 1310/1550 pair BiDi** |
| `P_out` min/typ/max | **−6/−3/0** |
| sensitivity | **−39** |
| overload | **0** |
| **link budget** (`P_out(min) − S`) | **33 dB** |
| **the receiver's window** | **−39 to 0 dBm — 39 dB**, so one part serves a metre of fibre and 100 km |
| spectral width | **1 nm DFB** |
| grades offered | C · **T** |

**One part covers every 2 Mb/s run.** Its window is wide enough that a shorter-reach part buys
nothing: the catalogue's 20, 40 and 60 km rows are not fitted. Extinction ratio is **9 dB**; LOS
de-asserts at −20 and asserts at −30.

**The code decodes:**

```
OPT2- ww A0 v c g R        duplex
OTB2- ww A0 v c g R        BiDi

ww   55 = 1550 nm  ·  35 = TX 1310 / RX 1550  ·  53 = TX 1550 / RX 1310
A0   100 km
v    3 = 3,3 V  ·  5 = 5 V
c    S = SC  ·  T = ST  ·  F = FC — no pigtail row
g    C = 0…+70  ·  T = −40…+85 industrial
```

**What this project fits:**

| | part |
|---|---|
| **`G-O-2-100`** | **`OPT2-55A03STR`** — 3,3 V, SC, industrial, 1550 nm, **33 dB budget** |
| the same on one strand per channel | **`OTB2-35A03STR` + `OTB2-53A03STR`** — a **matched pair**, the ends are not interchangeable |
| **the pressure build** | the same part, **obtained from the maker in a pressure-tolerant form** against this specification; the catalogue parts are land parts |

**The whole optical catalogue, in one table:**

| board | rate | reach | duplex | BiDi | **minimum length** |
|---|---|---|---|---|---|
| **`G-O-10-10`** | 10 Mb/s | 10 km | **`OPT10-31103STR`** | **not in the catalogue — ask the maker** | **none**, 3 dB spare |
| **`G-O-2-100`** | 2 Mb/s | 1 m to 100 km | **`OPT2-55A03STR`** | **`OTB2-35A03STR` + `OTB2-53A03STR`** | **none, but zero margin** |

### Choosing the module — by the run, never by which one is strongest

**A long-reach module is not a better module. It is a different trade** — a hotter transmitter
and a more sensitive receiver — and **on a short link the transmitter is the liability.**
Overloading is as much a failure as not reaching, and it is the nastier of the two to diagnose,
because every fibre is plugged in and every light is on. At 2 Mb/s there is no choice to make:
one part, and an attenuator where the run is short.

| the run | the module |
|---|---|
| **shorter** | **the weaker part is the better part** |
| **longer** | the stronger one |
| **a module on a run shorter than its rating** | **a fixed attenuator**, a 1–2 $ insert in the connector — not a shorter cable and not a different board |

**Two instincts to resist.** *Fit the 100 km part everywhere and never think about reach again*
— it has **0 dBm out against a 0 dBm overload** and is the first thing that fails on a short
run. And *drop to multimode below 2 km to save money* — `G-O-10-10` already works from zero
metres with 3 dB to spare, so multimode buys nothing and puts a second fibre type in the trench
and in the store.

**AC coupling is the other thing not to reach for.** Manchester or any line code that keeps the
laser modulated makes the transmitter run at a **50 % duty for ever**, where the DC-coupled
idle-maps-to-dark rule leaves the data laser **dark ~95 % of the time**. The optical budget it
buys is not the constraint here; the standing current is. **That is why this project is
DC-coupled from zero, and it is what the 0–2 Mb/s and 0–10 Mb/s parts were chosen for.**

### The minimum length — a long-reach module on a short fibre overloads

**A receiver has a floor and a ceiling, and the ceiling is the one nobody checks.** `P_out(max)`
arrives at the far end minus the loss; if that is above **`Receiver Overload`** the link does not
work, and it fails on a *short* fibre rather than a long one. **No part fitted here has a floor**
— every one sits exactly on its ceiling and none stands above it:

| module | `P_out` max | overload | loss it needs | of which fibre | **≈ minimum** |
|---|---|---|---|---|---|
| `OPT10` 10 km | −3 dBm | **0 dBm** | −3 dB — **none needed** | — | **none** |
| **the long part**, `OPT2` duplex or `OTB2` BiDi | 0 dBm | **0 dBm** | **0 dB — exactly at the limit** | — | **none, and no margin** |

*(0,35 dB/km at 1310 nm, 0,22 at 1550; two connectors at 0,5 dB each.)*

**Three readings:**

- **`G-O-10-10` is safe at any length, 500 m included.** Worst case it receives −3 dBm against a
  0 dBm ceiling — **3 dB of margin with no fibre at all**, so a short optical hop is a legal
  build and needs nothing added.
- **The 2 Mb/s part has no minimum length.** It sits exactly on its overload ceiling, so it
  works at any length, duplex and BiDi alike. What it does not have is margin: a connector cleaned to better than spec is the whole of it. **On a run much shorter
  than the part's rating an attenuator is cheap insurance.**

**The attenuator, by run — an example.** The rule: the strongest signal the part can send arrives
**about 10 dB under the overload ceiling** on the 2 Mb/s part, so the weakest arrives some 20 dB
above sensitivity and the difference is the margin for ageing, splices and repairs. Fibre
0,35 dB/km at 1310 nm and 0,22 at 1550, two connectors 1 dB; the pad is the nearest stock value
(1 · 3 · 5 · 7 · 10 · 15 dB):

| module | run | loss | received, strongest · weakest, no pad | **pad** | margin over sensitivity | **after 3 dB of ageing** |
|---|---|---|---|---|---|---|
| `OPT10`, 10 Mb/s | 2 km | 1,7 dB | −4,7 · −8,7 dBm | **none** | 7,3 dB | 4,3 dB |
| | 5 km | 2,8 dB | −5,8 · −9,8 dBm | **none** | 6,2 dB | 3,2 dB |
| | 10 km | 4,5 dB | −7,5 · −11,5 dBm | **none** | 4,5 dB | 1,5 dB |
| the long part, 2 Mb/s | 10 km | 3,2 dB | −3,2 · −9,2 dBm | **7 dB** | 22,8 dB | 19,8 dB |
| | 20 km | 5,4 dB | −5,4 · −11,4 dBm | **5 dB** | 22,6 dB | 19,6 dB |
| | 50 km | 12 dB | −12 · −18 dBm | **none** | 21 dB | 18 dB |
| | 100 km | 23 dB | −23 · −29 dBm | **none** | 10 dB | 7 dB |

**The 10 Mb/s part never takes a pad** — its ceiling is 3 dB above its own strongest output — and
its whole rated 10 km is usable. **The 2 Mb/s part takes one below ~40 km**, and on a patch of a
metre 10 dB. The table is an example; the pad fitted is chosen from the power measured on the
run.

**Ageing is taken as 3 dB over the link's life**: the laser 1–2 dB, as its threshold rises and the
power control runs out of current; connectors, splices and repairs 0,5–1 dB; on a submarine fibre a
few tenths from hydrogen. The photodiode does not age in any way that matters at these levels — its
responsivity holds and only its dark current grows. Every run in the table keeps a margin after
it; the 10 Mb/s part at its full 10 km is the thinnest, 1,5 dB.

**At commissioning the received power is measured at both ends before the link is trusted.**

**At 2 Mb/s duplex and BiDi are both catalogue.**
At 10 Mb/s only duplex is — the
`OPT10` ordering table has SC, ST, FC and FC-pigtail in three grades and **no BiDi row**, so one
strand per channel there is an **enquiry to the maker against the same spec**, with a lead time,
and **nothing is drawn assuming it can be bought.**

**A BiDi pair is bought as a matched A/B pair** — one end TX 1310 / RX 1550, the other the
reverse — which is a stocking note, not a board difference.

**10 Mb/s is the floor of the class, the clock sets it, and the data sit far underneath.**
The cable clock is 2²² Hz — a square wave, so an **8,39 Mb/s equivalent** — and it runs
continuously however short the link. **The data channel never asks for more:** its top rung is
2²¹, 2,097 Mbaud, a quarter of the clock's figure (`../core/blocks/nodbus.md`). The clock is
therefore the load that sizes the part and **one ordinary 10 Mb/s part carries every rate this
network has** — the data never call for a faster class. Going up is cheap (**the step to a 52 Mb/s part
is about a dollar**, less than a metre or two of cable) and it is bought for **reach**, which is
the only thing it buys. It is the floor that does not move.

**Reach, within the land spur, is the module's own — and 10 km is already more spur than a site
wants.** The 10 km part covers everything the 48 V feed reaches, which is where a spur ends
anyway, so **the land class is this one part and there is no second grade to choose between.**
A build that wants further swaps the module in the same 1×9 seat and keeps what the block is
built on: **DC coupling with idle mapped to laser-dark, the clock rate, TTL levels.** Past
about 40 km at this clock the blocker is **DC coupling, not distance** — a part that is both
long-reach and DC-coupled could not be found, and the idle-maps-to-dark rule needs DC. **That
step stops there**, and what crosses it is a lower clock rung, not a longer spur
(`../core/blocks/nodbus.md`). A crossing measured in tens of
kilometres is not a longer spur — it is the sea link, with its own feed voltage, its own converters
at both ends and its own optics; it lives in [`../atlantis/`](../atlantis/README.md)
and shares nothing with this page but the socket.

**Past the catalogue the route is CUSTOM MANUFACTURE, and that is a normal thing to buy.** These
makers re-spec stock modules for a living — a different laser grade, a different sensitivity, a
DC-coupled front where the catalogue part is AC — so a long-reach DC part is an order with a
drawing and a minimum quantity, not a dead end and not a research project. What it is not is a
line in this table: it has no price, no lead time and no second source until someone asks for
it, so the design never depends on one.

**Receiver overload is per module, and it bites on short links.** These parts clamp at **0 dBm**
in, and a long-reach module put on a short run arrives near that ceiling — its transmitter is sized
for a distance it never travels. Check the fitted module's own TX maximum against its own overload
figure for the actual run; it is a property of the part, not of the design.

**The strand count follows the population, and on this family it is the only lever.** The fitted
part is **duplex — two strands per two-way channel** — so a unit takes **three fibres** with the
one-way clock and **four** with channel B populated both ways. There is no BiDi row in the
`OPT10` ordering table, so two strands for a whole link is not on the menu unless a part is
ordered against the spec (*The land module*). Since strand count barely moves the cable price,
it is not a decision worth agonising over.

**It deletes the direction switching, and the line coder was already gone.** Two fibres are two
independent one-talker streams, so the whole shared-segment apparatus — DE, turnaround, the delay
budget that goes with it — simply does not apply on glass. **On copper it survives the flip to
full duplex**, because the units' TX pair is still shared by up to eight drivers; what glass
removes is the sharing, not the duplex. **One end sends, the other receives some
time later, and the delay is nobody's business.** That is worth more than it first looks, because the
NodBus spur is already point-to-point: the unit behind it shares the glass with nobody, so it owes
no inter-frame gap and needs no further synchronisation — it sends when it has something and batches
on its own schedule, which is also what drops the spur's rate. More expensive in parts, simpler in
everything else. And the module is DC-coupled, so a plain
async UART rides it at any rate down to DC: no framer, no line coding, no run-length constraint on
the protocol, TTL straight onto the UART pin.

**Cable, on land: two ordinary outdoor cables — the fibre and the copper, run together.** Both are
commodity, neither is specified here, and one trench takes both. A hybrid is an option rather than
a requirement; where one is used, **4 fibres and 1,5 mm² are enough**. The **submarine run is the
exception that wants the hybrid** — there the armour and the laying are the cost, which is what
makes the strand count nearly free ([`../daedalus/CONSTRUCTION.md`](../daedalus/CONSTRUCTION.md)).

## Choosing the FET — two parts, and the boundary is the input voltage

**The family runs two external switches and nothing else, and the split follows the controller.**
Every `LT3748` position takes a **150 V** part; the two boards fed from a 300 V cable take the
650 V one.

| | **`ISC165N15NM6`**, 150 V | **`FCD260N65S3`** |
|---|---|---|
| maker, family | Infineon OptiMOS 6 | onsemi `SUPERFET III Easy Drive` |
| `V_DS` | **150 V** | **650 V** (700 V at `T_J` 150 °C) · `V_GS` ±30 V |
| `R_DS(on)` | specified **at `V_GS` ≤ 7,5 V**, which is what `INTVCC` delivers | 222 mΩ typ / **260 max at `V_GS` = 10 V** |
| `I_D` | **≥ 10 A** | 12 A (7,6 at 100 °C) · `D-PAK` |
| where | **`G-48-S` · `G-48-U` · `G-300-S` · `G-24-S`** — every `LT3748` position | **`G-300-U`** — the board fed from a 300 V cable |

**Why 150 V on every position.** The flyback plateau is 26–45 V on the step-ups and 82 V on
`G-48-U`, but the leakage clamp holds the drain at **78–133 V** at the worst corner (*The leakage
clamp*), 151 V on `G-48-U` only with a surge residue on top of an overload, and `G-48-U` sees 119 V at the transil clamp; a 100 V part fits none of them.

**`R_DS(on)` stops being a criterion below ~25 mΩ.** At `G-300-S`'s worst case — `I_rms` 5,76 A —
the difference between a 7 mΩ part and a 20 mΩ one is **232 mW against 664**, which is 0,9 % of
the ~47 W it delivers, against a transformer that takes **half the loss budget on its own**. Spend the money on the
winding.

### The rule: a FET changes on a rating, never on a loss

**Which loss dominates is a property of the position, and the crossover is far from every operating
point in this family.**

| | conduction `I_rms²·R_DS` | gate `Q_g·V_g·f` | the crossover | where we sit |
|---|---|---|---|---|
| **`G-300-S`**, ~47 W off a 12 V pack | `I_rms` 5,76 A → **664 mW** at 20 mΩ | ~15 nC × 7,5 V × 145 kHz → **16 mW** | far above the band | 145 kHz, against a derived ceiling of 1,05 MHz |
| **`G-300-U-40`**, 40 W off 300 V | `I_rms` ~0,36 A → **~34 mW** | 24 nC × 10 V × 91 kHz → **22 mW** | conduction passes switching here | 40 W at 91 kHz, well under `LT8316`'s 140 kHz clamp |

**On the battery side the crossover lies above the controller's frequency ceiling, so gate charge
can never dominate — buy the lowest `R_DS(on)`. On the cable side the crossover lies above the
board's rated power, so conduction can never dominate — buy the lowest `Q_g` and `C_oss`.** A
smaller die is the better part on one side and the worse part on the other.

**And in neither case is it worth paying for.** A part twice as good buys **under one per
cent** of throughput on either board, against a chain that throws away **54 % by construction**
(buck 90 % × island 80 % × cable 75 % × cell 85 %). **So the criterion is: a FET is replaced when it
fails a *parameter* — voltage, current, thermal, gate charge against the driver — and never because
it would improve a loss.** Both parts pass all four with wide margins, and the cheapest part that
passes is the right one.

**Where the losses actually are, so nobody optimises the wrong term:**

- on **`G-48-S`** the **sense shunt is 258 mW** against the FET's own 92, and it is
  `I_pk·D·V_TH/3` — set by the peak current and the controller's threshold, **not by the
  resistor.** It cannot be bought away; it goes down only with `I_pk`.
- on **`G-300-U`** the biggest term is **`C_oss` at ~70 mW**, and turn-off is nearly free because
  at 0,45 A the drain needs ~220 ns to traverse 402 V through 248 pF while the channel closes in
  12 ns. **The same capacitance that costs the switching loss removes the turn-off loss.**

## The boards, drawn — the parts on each

One drawing per board, each complete on its own, and the part table that goes with it. The
doctrine — what a board IS and what its straps say — is [`README.md`](README.md). **Eleven
boards**; the two source cells, the two 300 V unit boards and the two optical fronts share a
drawing each in the tables below, because the cell is the same and only the population column
differs.

**No power board carries a buck at all.** The host side of the telemetry front (the
`ISO1642`) runs on the host's 3,3 V over the connector — the card's on a
source board, the unit's on a unit board; the measuring side runs on the board's own island off the same 3,3 V (*Feed telemetry*). `ENABLE` drives the converter's own `EN`
pin. A station feed cell taps the 12 V wire on its own terminals (2,6 A on `G-48-S`, 4,6 A on
`G-300-S` at a remote Argus — not ribbon currents) and takes its 3,3 V off the port; a unit power board hands 12 V out on its two terminals, never on a ribbon. **No communication board carries a buck or a terminal**: it takes
3,3 V from the host over the connector.

### The `ID` resistors — every value on both scales

**One resistor per body, to ground, read by the host against its own 10 kΩ (1 %) to 3,3 V** — or
to 1,8 V on an H7A3 host; the reading is a ratio and does not care. **ONE scale serves both
bodies**: the two sockets are two ADC pins and could not confuse a host, but a number that means
two things confuses a person, so no value is ever issued twice. **The windows are ±0,025 of full
scale.** The E96 grid lands every ratio within 0,006 of its nominal and 1 % parts on both sides
move it by at most ±0,005, so the margin is four to one; a 12-bit ADC resolves 0,00024 and is
not in the argument.

| body | board or host | resistor, E96 | reads |
|---|---|---|---|
| data, on `ID_RET` | **the Mayak** | **1,10 kΩ** | 0,10 |
| data, on `ID_RET` | **a card** — Bifrost or Argus, one code for both | **1,78 kΩ** | 0,15 |
| data, on `ID_RET` | **Palatine** | **2,49 kΩ** | 0,20 |
| data, on `ID_RET` | **Sputnik** | **3,32 kΩ** | 0,25 |
| data, **on `ID` AND on `ID_RET`** | **`GNSS`** — every port carrying the NMEA time stream, at both ends: Kronos's two RX/TX + PPS sockets, and Polaris, a Sputnik's clock run or a Pip's `TIME OUT` facing them | **5,36 kΩ** | 0,35 |
| data | `G-I-N-025`, two pairs | **6,65 kΩ** | 0,40 |
| data | **`G-I-M-005`** | **8,25 kΩ** | 0,45 |
| data | `G-O-10-10` | **10,0 kΩ** | 0,50 |
| data | `G-O-2-100` | **12,1 kΩ** | 0,55 |
| power | `G-48-S` | **15,0 kΩ** | 0,60 |
| power | `G-300-S` | **18,7 kΩ** | 0,65 |
| power | `G-48-U` | **23,2 kΩ** | 0,70 |
| power | `G-300-U-6` | **30,1 kΩ** | 0,75 |
| power | `G-300-U-40` | **40,2 kΩ** | 0,80 |
| power | **`G-12-S`** | **56,2 kΩ** | 0,85 |
| power connector | **Hermes** — the one board on that body that is not a power board | **90,9 kΩ** | 0,90 |
| power | **`G-24-S`** | **4,32 kΩ** | 0,30 |
| — | **0,05 (523 Ω) and 0,95 (191 kΩ) are held as margin at the ends of the scale, not issued** | | |
| either | nothing plugged in | **open** | 1,00 |
| either | a short | **0 Ω** | 0,00 — **a fault; no board takes this code** |

**`G-300-U-6` and `G-300-U-40` are two boards and their `ID` resistors differ** — the one
difference outside the power train, and a necessary one: a host that could not tell them apart
would ask a 6 W pod for 40 W. **`G-I-M-005` and a strapped `G-I-N-025` read alike on purpose**: one
transceiver on one pair is one transceiver on one pair, and what runs over it is the host's.

**0,00 is left empty on both connectors** — no board and no host takes it, the Mayak included. The
pin's remaining failure is a short to ground, and leaving 0 unassigned is what turns that from a
board into a reported fault. **The resistor answers before the `INA238` is programmed, and when it
does not answer**: the source end needs to know which cell it is driving before it can programme
any threshold at all, and a board whose isolator or island has failed still reads as itself.

### `G-48-S` · `G-300-S` — power, source · two boards, one cell

**The source cell makes the feed for one run.** One `LT3748`, one `ISC165N15NM6`; the winding,
the shunt, the rectifier and the protection codes are what differ, and they are two drawings —
merging them into one board with two populations saved nothing (`WHY.md`).

```
  12 V, 10–20 V, tapped off the wire on the board's own terminals
     │
     ├── L_IN 1,5 µH ▶ LT3748 ──▶ [ISC165N15NM6] ──▶ the winding ──▶ the rectifier ─┐
     │                 no-opto, no third winding,   750310988 1:4,42 · V5N22-M3        │
     │                 boundary mode, no burst      750310349 1:10   · US3M     THE BARRIER
     │                 R_SENSE 9,1 mΩ · 5,5 mΩ                                       │
     │                                                                              ▼
     │                                     2× transil ─▶ the two chokes ─────────────────▶ ═══ the feed
     │                                     5.0SMDJ54A · 5.0SMDJ350A      │
     │                                                                   the three-electrode tube ──▶ ⏚ earth point
     │                                                                     2036-07-SM · 2036-30-SM
     │
     └── INA238 + ISO1642 on the output; the host side on the port's 3,3 V

  to the host:  the POWER body — 3,3 V in, ENABLE in, SDA · SCL · ALERT out,
                ID out: 15,0 kΩ on G-48-S · 18,7 kΩ on G-300-S
                the 12 V comes in on the two terminals, not on a connector
  telemetry:    INA238 fitted, both ends
```

| | `G-48-S` | `G-300-S` |
|---|---|---|
| **the winding** | **`750310988`** — 1:4,42, `L_PRI` 14 µH ±10 %, `I_SAT` 15 A min, leakage 200 nH (1,4 %), `R_DC` 20 / 520 mΩ, **1500 V AC**, SMD 32,31 × 27,03 × 13,69 mm. Its own target application is 12 V → 48 V / 0,85 A at 200 kHz — used in the direction it was specified for | **`750310349`** — 1:10, `L_PRI` 5 µH ±10 %, `I_SAT` 25 A typ (Δ`L` < 20 %), `I_R` 18 A thermal (ΔT 25 K), leakage 100 nH (2 %), `R_DC` 10 / 500 mΩ, EE35/18/10 with wire leads, 29,1 mm pitch; target application 12–24 V → 300 V at 100–300 kHz. **Its insulation is 1000 V AC, below the family's 1,5 kV floor — accepted for the concept**; a production run orders a custom-insulated winding (*The frequency window*, below) |
| **`R_SENSE`** | **9,1 mΩ** → `I_pk` **9,90 A** at the guaranteed 90 mV threshold, **~26 W delivered**. **The shunt is chosen against saturation, not against the rating**: the 110 mV corner is 12,1 A, 81 % of the winding's 15 A, and the part's own 130 mV overcurrent trip lands at 14,3 A, still inside the core. The minimum-inductance rule wants only 3,0 µH against 12,6 at the winding's low corner, so it never binds. Peak power in the shunt is **0,9 W** — a 1 W part | **5,5 mΩ, two 11 mΩ in parallel** → `I_pk` **16,4 A** at 90 mV, ~66 W. **The shunt is fixed by the winding rule**: 30,1 V reflected wants `L ≥ 4.41 µH` at 5,5 mΩ against 4,5 at the winding's low corner, so it cannot be larger (*The one rule that screens a transformer*). The 110 mV corner is 20,0 A, **80 % of the 25 A typical saturation**, and the 130 mV overcurrent trip 23,6 A, still inside it. **~47 W declared** — the `INA238` threshold holds `I_pk` at 11,8 A, where the shunt dissipates **0,77 W** peak; 1,1 W in each part at the corner |
| **the rectifier** | **`V5N22-M3`** — 114 V reverse (`PIV = V_OUT + N·V_IN` = 48 + 4,42 × 15), a **220 V / 5 A** Schottky at 0,54 A. The grade is bought for the voltage margin, not the current | **`US3M`** — 435–450 V reverse at the 300 V feed, a 1000 V / 3 A ultrafast |
| **stress** | the FET sees 26 V on the plateau, **78 V** at the leakage clamp's worst corner, of its 150 | `V_in + V_refl` = 45 V on the plateau, **132 V** at the clamp's worst corner, of its 150 |
| **frequency** | boundary mode, so it falls as the load rises: **508 kHz at 2 W · 393 at 3 W · 236 at 5 W · 41 kHz at the full 26 W.** The station enclosure has nothing that listens in that band | **145 kHz** at the full 47 W, rising as the load falls: 199 kHz at 37 W, 589 kHz discontinuous at 8,6 W. The station enclosure has nothing that listens there either |
| **minimum load** | **0,65 W** — `f_FLOOR` 42 kHz against 41 kHz at full load, so 2,5 % and the one cell in the family the sheet's generic 2 % fits (*No burst, and therefore a minimum load*). Below it the rail climbs to the transil, because the load downstream is constant-power and does not self-limit | **0,63 W** — the same rule at the shunt's 16,4 A, 1,3 % of the declared ~47 W |
| **the port budget** | **the far end's measured load with margin, up to the cell's ~26 W** — a programmed `INA238` threshold with `ENABLE` behind it. What a port hands out is a number, not a converter property (`README.md`, *The two choices, and they are independent*) | the same, up to ~47 W (`README.md`, *The two choices, and they are independent*) |
| **the ladder** | `2036-07-SM`, 22 µH chokes, `5.0SMDJ54A` | `2036-30-SM`, 22 µH chokes, `5.0SMDJ350A`; **holdover 135 V is under the rail**, so a fired tube is a short until the current limit holds it and `ENABLE` clears it |
| **the earth stud and its strap** | a population: the M4 brass stack and the 2,5 mm² strap bolted at a station; the hole empty at a remote Argus | the same |

**What the voltage buys is reach, and load at reach** — `P_max = V₀²/4R` on a constant-power
load; on a 300 V run the cable stops being the constraint: 105 W at 7 km on 2× 1,5 mm² against
this cell's ~47 W (`README.md`, *Reach*).

#### What sizes `G-300-S`, and why the station and a remote Argus are the same board at the same number

**A station and a remote Argus are not two loads on this board.** Both feed it **12 V** — the pack
at one, the remote Argus's own bus at the other — and both take 300 V off it, so there is nothing for two
figures to come from. The board delivers **~47 W** and what a port hands out is a programmed
`INA238` threshold, the way `G-48-S`'s port is programmed.

The relation that sizes it is `P = ½·I_pk·(V_in·V_refl)/(V_in+V_refl)` = **4,286 · `I_pk`** at
1:10 from a 12 V pack — **in boundary mode the inductance cancels**, `f` falling exactly as
`½·L·I²` rises, so only `I_pk` and the two voltages move the answer. At `R_SENSE` **5,5 mΩ** the
shunt allows 16,4 A guaranteed, ~70 W in and ~66 W out; **the board is declared at ~47 W** — 11,8 A,
**50,6 W in**, 3,0 W of loss at 145 kHz — and the `INA238` threshold is what holds it there.

**What a remote Argus actually draws — four ports, and never eight.** Each port is a mini
segment whose far end is its own site: glass carries no power, so the remote Argus's own budget is its five
near ends (four segments sending the clock, the upstream receiving it) plus the tree its 12 V bus
feeds.

| module | one port, from the 12 V | 12 V bus | `G-300-U` | **`G-300-S`** | from the battery | `I_pk` |
|---|---|---|---|---|---|---|
| **100 mA** — every part this project fits | 0,64 W | 3,2 W | 3,8 W | **5,1 W** | 6,0 W | 1,19 A |
| 150 mA | 0,96 W | 4,8 W | 5,6 W | 7,5 W | 8,8 W | 1,74 A |
| 200 mA | 1,28 W | 6,3 W | 7,4 W | 9,8 W | 11,6 W | 2,30 A |
| **300 mA** — the 52/84 Mb/s class, **not fitted anywhere** | 1,92 W | 9,3 W | 10,9 W | **14,6 W** | 17,2 W | 3,40 A |

*(`../bifrost/argus/HARDWARE.md`: a segment end 1,75 modules, the upstream 1,25, the card 61 mA —
100 mA modules: 4 × 578 + 413 + 203 mW = 2,92 W on 3,3 V ÷ 0,9 buck = 3,2 W on the 12 V bus; then
`G-300-U` 85 %, cable at the 75 % criterion, cell 85 % — the chain is **48,8 %**, so a watt on
3,3 V at a far end is 2,05 W from the battery.)*

**The whole range fits one cell.** The worst case is **3,40 A**, against the winding's 25 A of
saturation and the shunt's **20,0 A** at the 110 mV corner. Every optical part this project fits
is **100 mA**, so a four-port all-optical remote Argus draws **~3,2 W** at 12 V, card and all —
against the cell's **~47 W**, fourteen times over. **The
optics stopped being what forces a remote Argus onto its own site**; what decides it now is the
cable, at watts × distance, and the module count only moves the load along that table.

### `G-48-U` — power, unit, 48 V

```
  ═══ 48 V, in through the gland onto the feed terminals
     │
     ├── 2× 5.0SMDJ54A ─▶ bulk ─▶ LT3748 ─▶ 750311607, 2,5:1 ─▶ V10P10 ────────────────┐
     │   NO gas tube, NO chokes    [ISC165N15NM6]   L_OUT 100 µH                                  │
     │   the far end of a km-class line is a soft source,                                  THE BARRIER
     │   and the bulk is what rides the dips                                                     │
     │                                                                                                  ▼
     │                                                                     12 V ─┬─▶ the two terminals — the unit makes its own rails
     │                                                                           └─▶ the 12 V terminal — a bought device, ≤ 0,5 A
     │                                                                     NO buck on this board
     │
     └── INA238 + ISO1642 on the input; the host side on the unit's 3,3 V over the connector

  The feed in and out on two terminal pairs, straight through, so units chain on one segment.

  to the unit:  the POWER body — ID out: 23,2 kΩ · SDA · SCL · ALERT out · ENABLE strapped · 3,3 V in for the
                12 V out on the two terminals, not on a connector; and for the
                isolator's host side (README.md, The connectors)
  telemetry:    INA238 fitted — this end is often the unreachable one
```

| | |
|---|---|
| **rated output** | **~25 W at 12 V** — `I_pk` **3,00 A**, 28,7 W in and 2,6 W of loss. `R_SENSE` **15 mΩ** allows 6,0 A at the guaranteed 90 mV threshold, ~50 W; the corner is 7,33 A, the overcurrent trip 8,67 A. **The declared run is 24 W** and the source cell is what sets it, not this board (`README.md`, *The two choices, and they are independent*) |
| the converter | **`LT3748` + the `ISC165N15NM6`.** No opto and no third winding — it samples the primary, so the winding is an ordinary two-winding part |
| transformer | **`750311607`** — 2,5:1, `L_PRI` **14 µH**, `I_SAT` **9,5 A**, leakage 500 nH, `R_DC` 45 / 10 mΩ, **1500 V AC**, 29,08 × 23,11 × 11,43 mm. Table 1 of the `LT3748` datasheet, target application **12 V / 2,5 A from 20–75 V** — this position, in the direction it was specified for. At `R_SENSE` 15 mΩ the 110 mV corner is 7,33 A, **77 % of saturation**. The output voltage is set by `R_FB`/`R_REF`, not by the winding |
| **`V_IN(MAX)` is 50 V, not 75** | the feed is a **regulated** 48 V, and the winding's own window is 20–75 V, so neither end binds. **The shunt is set by the inductance rule**: at 50 V in and 31,3 V reflected both terms want `L ≥ 12.5 µH` at 15 mΩ, against 12,6 at the winding's low corner, so it cannot be larger (*The one rule that screens a transformer*) |
| frequency | boundary mode: **455 kHz at the full 24 W**, rising as the load falls until the part's own 250 ns on-time and 700 ns off-time clamp it at **1,05 MHz**, which happens below about 10 W out — a lightly loaded board pulse-skips there, which is how the part regulates. `t_on` 0,88 µs and `t_off` 1,32 µs at full load, both clear. In a Tesla box the converter is placed, not parked: distance, a shielded inductor and the bench criterion (`README.md`, *Converters and measuring boards*) |
| minimum load | **0,24 W at 12 V out** — 42 kHz against 225 kHz at the shunt's 6,0 A, so **1,0 %** of the declared rating and not the sheet's generic 2 %; a unit's own processor covers it (*No burst, and therefore a minimum load*) |
| rectifier | `PIV = V_OUT + V_IN(MAX)/N` = 12 + 20 = **32 V**; **`V10P10-M3`**, 100 V / 10 A — 2,0 A average at 24 W, the secondary peak `N · I_pk` = 2,5 × 3,00 = 7,5 A, and ~1,5 W in the package. **The 100 V grade is bought for the leakage, not the voltage:** a Schottky's reverse current roughly doubles every 10 °C and a die run at a third of its rating leaks a fraction of the datasheet figure, which is what keeps a hot enclosure out of thermal runaway |
| **no low rails** | the 12 V is the only output; the unit makes 3,3 V and 4,0 V on its own board (*The low rails*) |
| **isolation — scoped, not open** | the winding's one insulation figure is **1500 V (AC)** N1 to N2 and no DC withstand; the grade follows **exposure**, not position: short benign → 1,5 kV class, which this part is; long, buried or near-HV → 3 kV, which needs a custom winding |
| above the declared rating | **not this board — the 300 V pair.** What a 48 V *run* carries is the reach table in `README.md` |
| **under pressure** | **passes, part by part**: every capacitor on the board is X5R/X7R ceramic — the 12 V bulk eight 10 µF 50 V 1206, the 48 V bulk 2,2 µF 100 V 1210, the decoupling and compensation 0603/1206 — the semiconductors plastic-moulded, the transils and the terminals solid, no tube and no choke at a unit end, and `750311607` a winding the vacuum fill soaks. Nothing on it is film, electrolytic, hybrid or a cavity (`README.md`, the pressure banner) |

### `G-300-U-6` · `G-300-U-40` — power, unit, 300 V

**`LT8316` + `FCD260N65S3` + a 12 V winding, on two boards.** **What splits them is the
enclosure, not the watts**: `G-300-U-6` goes where a 40 mm pipe is the whole of the available volume,
`G-300-U-40` where a remote Argus's 12 V bus, or any load of that size, is the load — a heated unit or a camera are examples, not units of the station. **The controller, the
FET, the ladder, the telemetry and the connector are identical**; the winding, `R_SNS`, the
rectifier and the `ID` resistor are not — **30,1 kΩ on `G-300-U-6`, 40,2 kΩ on `G-300-U-40`**, because a host
that could not tell them apart would ask a 6 W pod for 40 W.

```
  ═══ 300 V, in through the gland — the far end sees 227–300 V loaded to no-load
     │
     ├──────────────────────────────────────────────────────────▶ 2× 5.0SMDJ350A ─▶ the bank: U-6 ceramic, U-40 film ──▶ LT8316
     │   no gas tube and no chokes — a unit board                                                      [FCD260N65S3] │
     │                                                                                                                 │
     │                                                                                                  11328-T078, 8:1:1, 670 µH
     │                                                                                                  THE BARRIER    │
     │                                                                                     R_SNS 58 mΩ                 ▼
     │                                                                                                  100 V Schottky ─▶ 12 V ─┬─▶ the two terminals ─▶ a unit
     │                                                                                                                          └─▶ the 12 V terminal — a device, or the remote Argus's bus, 3,33 A
     │
     └── INA238 + ISO1642 on the input

  NO GDT and NO earth strap — a pod cannot be earthed, and a remote box floats by construction.
  to the unit:  the POWER body and the two terminals, as on G-48-U; at a remote Argus the Argus takes the 12 V on its own input
```

| | |
|---|---|
| the converter | **`LT8316`** — a controller with an external MOSFET: input class 16–560 V, gate driver bias 12 V at start and 10 V in operation, up to 100 W, quasi-resonant boundary mode. **Its output power is the FET, `R_SNS` and the winding — not the chip** |
| the FET | **`FCD260N65S3`** — 650 V against `V_IN + N_PS·(V_OUT + V_F)`; `I_rms` is small at this input voltage, so **`Q_g` and `C_oss` are what this position buys, not `R_DS(on)`** (*Choosing the FET*). Rated at the steady rail, 402 V of 650 (*The protection ladder — the parts*) |
| **the winding — `G-300-U-40`** | **`11328-T078`** — PQ2620, 8:1:1, `N_TS` = 1, 670 µH, **rate current 3,0 A**, leakage 8,0 µH max / 4,0 typ, **reinforced 3 kV**, 31 × 28,5 × **23,5** mm, PIN |
| **the winding — `G-300-U-6`** | **`11338-T195`** — CEEH178, 14:1:1,7, `N_TS` = 1,7, 1000 µH ±10 %, **rate current 0,9 A** on the primary, leakage 34 µH max / 17 typ, `R_DC` 3,0 Ω / 60 mΩ / 420 mΩ max, **basic, 3000 V AC** Np,Naux–Ns, 19 × 17,4 × **8,6** mm, **SMD**. Same Sumida datasheet as `G-300-U-40`'s, so it is one order and not two |
| **`R_SNS`** | **58 mΩ** on `G-300-U-40` · **130 mΩ** on `G-300-U-6` — the largest the sampling floor allows on `11338-T195`: `t_OFF(MIN)` 800 ns × 172 V over 20 mV / 130 mΩ asks 896 µH, against 900 at the winding's low corner |
| **rated output — `G-300-U-40`** | **40 W at 12 V declared.** `R_SNS` stays at 58 mΩ because **the shunt protects the winding and the threshold declares the watts**: `I_pk` 1,55 A at 90 mV, the 110 mV corner 1,90 A, **63 % of the winding's 3,0 A**. The board itself would carry ~54 W; **what caps it is the station.** `G-300-S` delivers ~47 W, this board's own loss is **3,7 W** of it, and the far end therefore sees **43 W at a 12 V pack and 41 W at the 11 V floor**. 40 W is declared under both, as a programmed `INA238` threshold, so the same board delivers more the day a stronger source cell exists |
| **where the 3,7 W goes, at 47 W in** | rectifier `V10P10-M3` at 3,71 A average **2,41 W** · winding leakage, 8 µH at 91 kHz, **0,56 W** · secondary winding, 9 mΩ at 4,95 A rms, **0,22 W** · core ~**0,40 W** · primary, FET, shunt and gate together **0,11 W**. **Two thirds of it is the rectifier**, and the only way past that is a higher output voltage, which a 12 V island does not have. A higher-current grade of the same Vishay family would buy about 0,4 W — 1 % of the throughput, for a stock number — and is not taken |
| **rated output — `G-300-U-6`** | **6 W at 12 V, ~7,5 W in, 81 %.** The boundary point would be ~870 kHz, so the part runs **discontinuous at its 140 kHz clamp** at every load — legal, and the sheet's front page regulates that way. `I_pk` **0,33 A** at full load; `t_ON` 1,1 µs at 300 V, `t_OFF` 1,9 µs, both clear of the part's 300 ns and 800 ns. **The shunt's cycle limit is 0,77 A at 100 mV and 0,85 A at the 110 mV corner, 94 % of the winding's 0,9 A** — under the sheet's 30 % saturation margin, and taken: no sustained state reaches it, because full load is 0,33 A and a short is held by `IREG/SS` at 0,65 A out, ~0,1 A on the primary; only a transient touches the limit. Loss: the clamp 0,81 W, the rectifier ~0,3 W, the windings ~0,1 W, the core, FET and bias ~0,2 W |
| **nothing stands still** | `LT8316` draws **12 µA** on `V_IN` shut down and **75 µA** on `BIAS` in burst — **milliwatts at 300 V** — because `N_TS` takes the bias over once the converter runs and the high-voltage start path closes. It bursts to a **3,5 kHz** floor, **220 Hz** with `SMODE`, which leaves a **minimum load of ~1 % of the rating** — 0,4 W on `G-300-U-40`, 60 mW on `G-300-U-6` — so a 3 W node on either board is in the converter's normal range |
| **and a small load does not pay for a big board** | in boundary mode `B` follows `I_pk` and `f` follows `1/I_pk`, so with ferrite's usual exponents the core loss goes as `I_pk^1.1` — **it tracks the load rather than standing on it.** What the big board does pay at a small load is the frequency clamp: below about 10 W it can no longer speed up, so it carries a **larger** peak than `G-300-U-6` does at the same watts. That, and not the size, is the argument for the small board |
| **the snubber, `G-300-U-40`** | **RC, not fitted unless the drain ring asks for it.** No clamp diode is owed: the drain sits at **402 V of 650**, so the leakage spike has 250 V to land in; it dissipates **0,70 W** from the winding's own leakage at the operating peak. Where a drain ring asks for an RCD clamp instead, its diode is **`US3M`** |
| **the clamp, `G-300-U-6`** | **an RCD, fitted**: 14:1 reflects 172 V and leaves 178 V to the FET's 650, and 34 µH of leakage at 0,33 A rings past it. **`D_CL` `US3M` · `C_CL` 100 nF 1 kV X7R 1812 · `R_CL` 2× 38,3 kΩ 2512 in series** → `V_CL` 250 V above the input: `P_CL` = ½·`L_LK`·`I_pk²`·`f`·`V_CL`/(`V_CL` − `V_OR`) = **0,81 W**, 0,56 W of it taken from the output. 550 V on the drain at the steady rail, 640 V with the node held at the transil's `V_BR`. All ceramic and solid — a pressure part |
| **why 12 V** | `BIAS` needs `N_TS` 0,83–2,5 at 12 V instead of ≥ 1,6 at 6 V, which opens the catalogue; and 40 W at 12 V is 3,33 A across the output where 6 V would be 6,7 A |
| the BIAS coupling | `BIAS` rides the output through the tertiary: **12,3 V** on `G-300-U-40` (`N_TS` 1) and **20,9 V** on `G-300-U-6` (`N_TS` 1,7), inside the sheet's 10–30 V |
| **the feedback — `G-300-U-40`** | the sheet's `V_OUT = (1 + R_FB2/R_FB1) · 1,22 V / N_TS − V_F`, `R_FB1` 1–10 kΩ for the divider's speed: **`R_FB1` 10,0 kΩ · `R_FB2` 90,9 kΩ** at `N_TS` 1 and the diode's 0,3 V near zero current — the sheet's own 12 V example. **`R_TC` 249 kΩ** from `R_TC = −R_FB2 · 4,1 mV/°C / (TCF · N_TS)` at the sheet's nominal −1,5 mV/°C for the diode, TC pin to FB |
| **the feedback — `G-300-U-6`** | the same equations at `N_TS` 1,7: **`R_FB1` 10,0 kΩ · `R_FB2` 162 kΩ** → 12,04 V · **`R_TC` 261 kΩ** |
| **the current limit** | `R_SNS` sets the *switch* peak; the **output**-current regulation point is `IREG/SS`, one resistor to ground: `R_IREG/SS = 2,5 MΩ · I_OUT · R_SNS / N_PS`, set at 120–150 % of the maximum load. **`G-300-U-40`: 76,8 kΩ → 4,24 A**, 127 % of 3,33 A. **`G-300-U-6`: 15,0 kΩ → 0,65 A** at 130 mΩ and 14:1, 129 % of 0,5 A. The part is constant-current as well as constant-voltage: a short is regulated, not survived. The sheet puts the loop within 5 %, the winding's capacitance being the error |
| the rectifier | **`G-300-U-40`**: `PIV` = 12 + 300/8 = **50 V** at **3,7 A** average → **`V10P10-M3`**, 100 V / 10 A, 37 % loaded. **`G-300-U-6`**: `PIV` = 12 + 300/14 = **33 V** at 0,5 A, 4,6 A peak → **`V10P10-M3`**, 100 V / 10 A |
| **no low rails** | the 12 V is the only output — a unit on the connector, the remote Argus's bus on the terminal |
| the 12 V terminals | a device, or **the remote Argus's 12 V wire — the card and its source boards tap it, terminal to terminal, and their sum stays within the board's 3,33 A**: the card's own draw plus the four ports' `INA238` thresholds, each source board carrying only its own far ends and running far below its ceiling. The wire's input window (the pack's 12–13,5 V) has a regulated 12 V in the middle of it |
| the ladder | **2× `5.0SMDJ350A` across the pair, and nothing else** — no gas tube and no chokes. A floating end has nothing for a three-electrode part to reference, and the run's own ~600 µH a kilometre is the series element |
| **under pressure** | **`G-300-U-6` passes, part by part**: the bank five 1 µF 500 V X7R 2220, the clamp's 100 nF 1 kV X7R 1812 and its 2512 resistors, the 12 V output eight 10 µF 50 V 1206, `BIAS` and `INTVCC` on 4,7 µF 50 V 1206, the semiconductors plastic-moulded, `11338-T195` a winding the vacuum fill soaks — no film, no electrolytic, no hybrid, no cavity. **`G-300-U-40` is land only**: its bank is polypropylene film and its 12 V output carries two hybrid polymers; a pressure build of it takes the `U-6` parts and the recalculation is the reader's |

### `G-I-N-025` — communication, 485 on copper, NodBus

```
  the host ── data connector ──▶ 3,3 V from the host (the card's LMR43620, or the unit's rail)
     │                        │
     │  LINE_EN ──▶ SN6505B's EN   └── SN6505B ──▶ 750313734 ──▶ PMEG10020ELR ──▶ the isolated
     │                                 420 kHz fixed   1:1,1                       line-side 3,3 V, NO LDO
     │                                                 THE BARRIER
     │
     └── on the line side:
             ISO1452 ── data, both directions kept — full duplex
             ISO1452 ── THE ECHO-CHECK RECEIVER, paralleled onto the units' TX pair.
                        Its DRIVER IS HARD DISABLED — tied off, not host-gated, so it
                        can never babble — and its UART's own TX stays unconnected
             ISO1450 ── the clock channel, or PPS inward on the reversed build.
                        Its direction is B_DIR's, set in the socket — no enable pair on the cable.
                        D and R both on the CLK/PPS pin; DE and RE# on B_DIR, tied in the socket —
                        R is at high impedance while the driver is on, so nothing else is needed
                                    │
                                    ▼
                        SM712 ─▶ 10 Ω ─────────────────▶ ═══ TX / RX / clock
                                          across the pair
                                          2036-07-SM ───────────▶ ⏚ common earthing point
                                                                   strapped at the source end only

  four two-pole DGPS2.5R-5.0, eight poles, one pole to a net, a chained run twisted into the pole it shares.
  Termination: 80,6 Ω across each pair behind a JUMPER on a 2-pin 2,54 mm header — three, one per
  pair — fitted on the first and the last board of a segment and on no board between.
  The tube is the SOURCE END's population: one board serves both ends, so the footprint is on
  every board, and at a unit end the tube and its strap are left off — a floating end gives a
  three-electrode part nothing to discharge into, and 10 Ω and the SM712 hold the residue.

  to the host:  the DATA body — 3,3 V, the logic, LINE_EN, ID
```

**The same board serves both ends.** What it is, is the socket's business: `B_DIR` from the
socket points the clock transceiver, the `ID` resistor tells the host what the board is, and the
GDT and the earth strap are populated at the source end and left off at the unit end.

**No buck, and no 12 V terminal — the one block on the board is the line's.** The 3,3 V arrives from the host over the connector; `LINE_EN`
drives the `SN6505B`'s `EN`, and everything behind the barrier goes dark with it — the port kill
is one wire. **`EN` carries 100 kΩ to ground on the board**, so an unplugged or reset host leaves
the line side dark. At a source end the host drives it from the port's GPIO; at the unit end the
host's socket holds it up with 10 kΩ to 3,3 V and no processor pin — the unit never switches its
own link off. The isolated line-side 3,3 V is ~100 mA, no LDO (*The islands*).

**Three identical isolated transceivers, and their function comes from a strap.** Mixing part
types to save a channel's quiescent draw was weighed and dropped: the saving is about half a
milliamp on a bus and the cost is a second order code and a board that is no longer the same at
both ends. **The outputs are laid out crossed** so a reversed build does not swap the UARTs.

**Under pressure, at a unit end, it passes part by part**: every capacitor is X5R/X7R ceramic — the
10 µF 1206 and 100 nF 0603 either side of the barrier and at each isolator — the `SN6505B`, the
three isolators and the rectifier plastic-moulded, `750313734` a winding the vacuum fill soaks, the
transils, the resistors, the jumper headers and the terminals solid; the tube is not fitted at a
unit end. Nothing on it is film, electrolytic, hybrid or a cavity.

### `G-I-M-005` — communication, 485 on copper, ModBus

**`G-I-N-025` stripped to a ModBus arm: one transceiver, one pair, half duplex.** The same
barrier, the same isolated supply, the same ladder on the one pair.

```
  the host ── data connector ──▶ 3,3 V from the host ──▶ SN6505B ──▶ 750313734 ──▶ PMEG10020ELR ──▶ isolated 3,3 V
     │ LINE_EN ──▶ SN6505B's EN                          420 kHz      1:1,1          THE BARRIER           │
     │                                                                                                    ▼
     │                                                                            ISO1450 ── A/B, DE from the host
     │                                                                                     │
     │                                                                SM712 ─▶ 10 Ω ──────────────▶ ═══ the pair
     │                                                                                  2036-07-SM ──▶ ⏚ earth point
     │
     └── the ID resistor: 8,25 kΩ, the one-pair value;
         B_DIR lands on nothing, there is no channel B

  four two-pole DGPS2.5R-5.0, four positions of the pair; the 80,6 Ω behind a jumper on a 2-pin
  header, fitted on this board — the source end; the far end of an arm is the sensor's own.
```

| | |
|---|---|
| the transceiver | **`ISO1450`** — the shared-pair part; `DE` from the host gates the driver around the poll |
| what it drops against `G-I-N-025` | two `ISO1452`, two pairs of ladder, two termination resistors; ~$7 of parts and half the board |

### `G-O-10-10` · `G-O-2-100` — communication, glass · two boards, one front

```
  the host ── data connector ──▶ 3,3 V from the host ─────────────────────────────────────┬─▶ π filter ─▶ 1×9 seat — channel A, data
     │                                                                                └─▶ π filter ─▶ 1×9 seat — channel B, the clock
     │                                                     the π filter is the module sheet's own:
     │                                                     100 nF, the shielded 2,2 µH, 10 µF + 100 nF per Vcc,
     │                                                     local bulk at the seat for the keyed laser's step
     │
     └── the ID resistor

  2× 1×9 SEATS, soldered, not socketed ══▶ channel A: data, full duplex
                                            channel B: the clock — VccT at the sending end, VccR at the
                                            receiving end; both sections for the reversed build.
                                            74AUP1G126 from the CLK/PPS pin to TD, OE active high;
                                            74AUP1G125 from RD to the pin, OE active low;
                                            TPS22917 on VccT, TPS22917L on VccR — all four inputs
                                            on B_DIR, tied in the socket: high feeds and connects the
                                            transmitter, low the receiver; 100 kΩ to ground on TD
                                            and on the 125's input

  NO barrier and NO isolated line-side supply — the fibre isolates by construction.
  NO buck and NO LDO — the module's own π filter, and the receiver's own limiting amplifier.
  The fibre needs no protection: nothing electrical arrives on it.

  to the host:  the DATA body — 3,3 V, the logic, B_DIR, SD, ID
```

| board | the module | reach | fibres | the rung |
|---|---|---|---|---|
| **`G-O-10-10`** | **`OPT10-31103PTR`** (FC pigtail) or **`OPT10-31103STR`** (duplex SC), 3,3 V industrial, 0–10 Mb/s | 10 km, 1310 nm | **duplex only — no BiDi row**: two per two-way channel, four per link | 2²² — a 4,19 MHz square wave is an 8,39 Mb/s NRZ equivalent; **ask a maker in hertz** |
| **`G-O-2-100`** | **`OPT2-55A03STR`**, 1550 nm, 33 dB budget; BiDi `OTB2-35A03STR` + `OTB2-53A03STR` | 1 m to 100 km | two per two-way channel, or one with BiDi | **2¹⁹** — 2 Mb/s does not carry the 40 B bus's clock; the far end takes the sync on a timer capture, as a mini-NOD does |

| | |
|---|---|
| **every module this project fits reads 100 mA**, `I_TX + I_RX`, from its own sheet | **The clock TX is the one that burns** — lit continuously — where the data TX is idle-dark between slots, ~95 % of the time. **The rails are sized on each end's own ceiling — both sections of the data module and the one section of the clock module — never on two whole modules**: 175 mA at a master port, 125 mA at a far end; the card's four down ports 700 mA of the `LMR43620`'s 2 A, its up port 125 mA of the `TPS629206`'s 600 mA |
| **so a board is two modules, and the split matters once** | the data channel runs both sections; the clock channel runs `VccT` only at the sending end and `VccR` only at the receiving end — a `TPS22917` on `VccT` and a `TPS22917L` on `VccR`, both `ON` from `B_DIR`, so the strap that points the channel also feeds its one section. At ~75/25 mA — the laser and the receiver — that is **~105 mA at a master port and ~55 mA at a far end on average, 175 and 125 mA at most**. The split is an estimate — the sheets quote `I_TX + I_RX` as one figure |
| **the rail is sized for the module, and the burst comes out of a capacitor** | a keyed laser's step must not be asked of a ribbon or a cable: local bulk at the seat, the host's rail carries the average |
| **DC-coupled, idle mapped to laser-dark** | the whole family, 0–2 and 0–10 Mb/s, starts at DC; **`DE` has no laser to gate** (*The fibre front*) |
| soldered, not socketed | the field-replaceable unit is the board — someone is travelling to the site either way, and swapping a small board beats extracting a module from a seat in salt air |
| **under pressure** | **the board passes and the module does not**: every other part is ceramic, a plastic-moulded logic gate or load switch, a shielded inductor the fill soaks — but every 1×9 module is a lidded metal body with an air cavity around its laser and its receiver. `G-O-2-100` goes in a pod only with the module obtained from the maker in a pressure-tolerant form (*The fibre front*); `G-O-10-10` never does — its rung is not the pod's |

### `G-12-S` — isolated 12 V, source

```
  12 V, 10–20 V, tapped off the wire on the board's own terminals
     │
     ├── 2,2 µH ────▶ SN6507 push-pull ──▶ 750319691 ──▶ 2× PMEG10020ELR ─┐
     │                 1,26 MHz by R_CLK      THE BARRIER                  │
     │                 R_LIM 34,8 kΩ → ~0,7 A                              ▼
     │                 ENABLE ──▶ its EN                 2× 5.0SMDJ18A ─▶ the two chokes ─▶ 2036-07-SM ──▶ ⏚ common earthing point
     │                                                                                   ══▶ + / − ~12 V, unregulated, follows the pack
     │
     └── no buck: the SN6507 runs from the 12 V and nothing on the host side needs a rail

  NO unit-end partner.  What stands at the far end is the bought device itself, and
  its own supply window is what this board is sized against.

  to the host:  the POWER body — 3,3 V in, ENABLE in, ID out: 56,2 kΩ, SDA · SCL · ALERT out;
                the 12 V in on the two terminals
  telemetry:    INA238 + ISO1642 ON THE OUTPUT, the same front as every other
                power board.  The arm is on the far side of the transformer, so the shunt is
                too, and the I2C and ALERT cross the barrier; the host side runs on the
                port's 3,3 V.  It is what sees an overload: the SN6507 answers one by
                shortening its pulses, so an overloaded arm does not die, it DROOPS, and its
                sensors go wrong in a way the host's polls cannot tell from a broken sensor.
                A primary-side shunt would miss it -- the host's 12 V does not move.
```

| | |
|---|---|
| what it is for | **a bought ModBus device a few metres out.** A feed cell and a unit board would be two boards doing one board's work at that distance |
| the part | **`SN6507`** — 3–36 V in, **0,5 A**, UVLO/OVLO/OCP, soft start |
| transformer | **Würth `750319691`** — the sheet's own 12 V → 12 V row (Table 9-3): **N = 1,13**, V-t **7,5 Vµs**, isolation **2,5 kVRMS**, 8,5 × 12,87 × 5,16 mm |
| the input | **`SWPA252012S2R2MT`**, the house 2,2 µH, into **2× 10 µF 50 V 1206 + 100 nF** at `VCC` — the station's filter part for a 1 MHz node (`../core/POWER.md`); 0,5 A average against its 1,15 A |
| **`R_LIM` = 34,8 kΩ → ~0,7 A** | **the sheet's 35 kΩ row, 700 mA typical; its characterised rows spread ~±30 %, so ~0,5–0,9 A.** It is set above the switches' 0,5 A on purpose: 0,4 A out is ~0,45 A through the switches at N = 1,13, and a 50 kΩ limit (0,5 A typical, ~0,35 A at the low end) would droop the rated load on a low part. **The overload is not left to the OCP** — which shortens pulses rather than switching off, and would hold a sustained overload until the die reached thermal shutdown: the `INA238` on the output sees the arm past its 0,4 A, `ALERT` goes to the host and `ENABLE` takes the board off. The OCP covers the start into a sensor's input capacitors and a short, inside the 1,6 A absolute peak |
| **rated output — 0,4 A** | ~0,45 A through the switches under a ~0,7 A limit, which leaves the start into a bought sensor's input capacitors somewhere to go. At the pack's working corner that is **~12,7 V × 0,4 A ≈ 5,1 W** out and **~6,0 W in at 85 %**, so **0,5 A off the host's port** — the figure Palatine's ribbon is counted against. At the 11 V reserve floor it is ~11,5 V and 4,6 W |
| the rate — **not a free choice** | the winding's V-t sets a floor: `f_min ≥ V_IN(max) / (2·V-t)` = 15 V / (2 × 7,5 Vµs) = **1,0 MHz**. **The floor applies to the LOWEST rate, and the sheet puts that 15 % under the typical one** for a resistor-set clock (and 780 kHz for the default, CLK to ground). So the typical rate must be ≥ 1,18 MHz: **`R_CLK` = 7,87 kΩ (E96) → ~1,26 MHz typical, ≥ 1,07 MHz**, from the sheet's table (9,6 kΩ → 1,07 MHz, 4,1 kΩ → 2,13 MHz) and its Figure 8-6 — **7,0 Vµs used of 7,5** at 15 V, 6,3 at the pack's 13,5 V |
| why 1,13 and not 1:1 | the ratio makes up the two rectifier drops, so the port lands near the pack rather than a diode pair below it |
| rectifier | **`PMEG10020ELR`** — `V_R > 1.5 · 2 · N · V_IN(max)` = 1,5 × 2 × 1,13 × 15 V = **51 V**; ≤ 0,4 A shared by two diodes on alternate half-cycles → ~0,2 A each |
| **why the transil is an 18 V part** | the secondary is **unregulated** — the pack through a 1,13 winding, less two rectifier drops — so it rises with the battery and is highest exactly when the drops are smallest: **16,2 V at a full pack under light load**, which is the corner that actually happens because a port sits idle most of the time |
| what it is not | **an isolation barrier, not a supply.** Nothing regulates; the ~12 V label is the battery's own voltage. **The bought population runs to 24–30 V, so the port's swing is never the constraint** |
| **the telemetry front** | **`INA238` + `ISO1642` — one part for the whole crossing: the I²C both ways, `A_SEL` down onto `A0` and `ALERT` up, high on alarm — the same front the whole family carries, with the low-side shunt in the ISOLATED OUTPUT.** The family's rule is that the shunt sits on the side away from the host, and the side away from the host here is the arm. **What it must see is the droop**: an overloaded `SN6507` shortens its pulses, so the output voltage falls while the current sits on the limit — the `INA238` reads bus voltage as well as current and that pair is the whole diagnostic. **On the primary neither half survives**: the current reads the arm plus the converter's own quiescent, magnetizing and loss current in a proportion that moves with load, and the voltage is the host's 12 V, which does not move at all. The host side of the isolator runs on the port's 3,3 V |
| the measuring-side island | the same `SN6505B` + `750313734` island as every power board, off the host's 3,3 V on the power connector (*Feed telemetry*) |
| reach | **isolation only.** A leaf that also needs reach takes the 48 V pair — reach is the feed cell's job, and this board has none |

### `G-24-S` — power, source, 24 V, switched

**The feed cell with a 1:1 winding and `ENABLE` as the switch.** The same `LT3748`, the same
`ISC165N15NM6`, the same RCD clamp, the same telemetry front and island; the winding, `R_SENSE`, the
rectifier and the transil code are its own, and the tube is fitted like every source board's.

```
  12 V, 10–20 V, tapped off the wire on the board's own terminals
     │
     ├── L_IN 1,5 µH ▶ LT3748 ──▶ [ISC165N15NM6] ──▶ 750311592, 1:1, 8 µH ──▶ V10P10-M3 ─┐
     │                 EN/UVLO from ENABLE — the switch  R_SENSE 10 mΩ                  │ THE BARRIER
     │                                                                                  ▼
     │                                     2× 5.0SMDJ28A ─▶ the two chokes ─────────────────▶ ═══ 24 V, one two-pole DGPS2.5R-5.0
     │                                                      2036-07-SM ─────────────────────▶ ⏚ earth point
     │
     └── INA238 + ISO1642 on the output, on the board's own island; the host side on the port's 3,3 V

  to the host:  the POWER connector — 3,3 V in, ENABLE in, SDA · SCL · ALERT out, ID out: 4,32 kΩ
```

| | |
|---|---|
| **the winding** | **`750311592`** — 1:1:0,44, `L_PRI` 8 µH ±10 %, `I_SAT` 18 A typ, leakage 0,4 µH max, `R_DC` 15 / 20 mΩ, 1500 V AC, 32,3 × 27,0 × 13,7 mm SMD; the third winding is left open — the `LT3748` samples the primary |
| **`R_SENSE`** | **10 mΩ** → `I_pk` **9,0 A** at the guaranteed 90 mV threshold, **~33 W delivered** at 24 V, 36 W in at the working corner, **112 kHz** at full load and ~166 kHz under a 22 W load |
| **the rectifier** | **`V10P10-M3`**, 100 V / 10 A — `PIV = V_OUT + N·V_IN` = 24 + 15 = 39 V at ~1,4 A average |
| **the switch** | `ENABLE` on the `LT3748`'s `EN/UVLO` through the 100 kΩ pull-down (*Switching the converter*); off between runs, the board draws nothing but the island's 1,5 mA |
| **the clamp** | the RCD of *The leakage clamp*: `V_CL` 100 V, **`R_CL` 2× 2,05 kΩ** 2512 in series, 2,4 W rated and 2,9 W at the corner; `D_CL` `V5N22-M3`, **`C_CL` 100 nF 1 kV X7R 1812** — the winding's 0,4 µH of leakage is 5 % of its primary |
| **minimum load** | **0,31 W** — 42 kHz against 112 kHz, 0,9 % of the rating; in service the port is on with its load or off |
| **the ladder** | `2036-07-SM`, 22 µH chokes, **2× `5.0SMDJ28A`** — 28 V is 1,17× the rail, clamp ~45 V. The tube quenches on 24 V as on 48 |
| **the telemetry** | the shunt on the isolated output as the load's ammeter; **over** the programmed limit is a stalled load, **under** the minimum a load running dry or a cut cable — two thresholds the host programs from what `ID` said was seated |
| **the output** | one two-pole `DGPS2.5R-5.0` for the load's 2-core cable; no unit-end partner, nothing regulates after the barrier |

## `LT8316`, read — and the sheet settles four things the family had open

**Every figure below is from the datasheet.** Boundary mode has no nominal frequency at all; the 140 kHz clamp is the only switching-frequency figure the part has.

| | | what it decides here |
|---|---|---|
| **`V_IN(MIN)`** | **16 V** for startup, `BIAS` floating | **the part cannot run on a 12–13,5 V pack.** `LT3748` starts at 5 V and keeps every battery-side position |
| input range | **16–560 V**, 600 V transient absolute maximum | 48 V and 300 V are both comfortably inside; the ~87 V clamp on a 48 V feed is nothing against 600 |
| **`F_SW(MAX)`** | **138 / 140 / 142 kHz**, a hard clamp | above it the part reduces frequency and runs discontinuous. **This is also a floor on inductance** — see below |
| `F_SW(MIN)` | **3,5 kHz** in Burst Mode · **220 Hz** with `SMODE` tied to `INTVCC` | and it **enforces a minimum load of ~1 % of the rated power**, because the output is sampled once per flyback pulse |
| `INTVCC` | 12 V at startup, **10 V in operation** | the gate drive, and it is the voltage `FCD260N65S3`'s `R_DS(on)` and `Q_g` are specified at |
| **`BIAS`** | **≥ 9,5 V** after startup, clamps 34–39 V | `BIAS = N_TS · (V_OUT + V_F)`, and the sheet holds it at **10–30 V**, so **`N_TS` ≥ 1,6 on a 6 V rail and 0,83–2,5 on a 12 V one.** Most of the catalogue is 8:1:1 or 4:1:0,5 and fails the 6 V case outright |
| **`t_OFF(MIN)`** | **500–850 ns** across temperature | the sampling window: the flyback pulse must outlive it |
| `V_SENSE` | **20 mV minimum**, 100 mV maximum | `I_pk(max)` = 100 mV/`R_SNS` and `I_pk(min)` = 20 mV/`R_SNS`, and the second one is what sizes the winding |
| **`IREG/SS`** | 10 µA out, one resistor to ground | **the output-current regulation point is this pin, not `R_SNS`** — two different jobs, and conflating them under-rates the board |

### Two floors on the primary inductance, and both are floors

```
   sampling      L_pri  ≥  t_OFF(MIN) · N_PS · (V_OUT + V_F) / I_pk(min)      I_pk(min) = 20 mV / R_SNS
   clamp         L_pri  ≥  1 / ( 140 kHz · I_pk(max) · (1/V_IN + 1/V_REFL) )

   Neither is a ceiling, so no window closes — take the larger of the two.
```

**The clamp floor is the one that is easy to miss.** In boundary mode the frequency rises as the
load falls, so full load is the *low*-frequency corner; if `L` is too small the part is already
discontinuous at its rated power, which is legal but puts `I_pk` above what the boundary arithmetic
predicted. **A board whose rated power is small enough cannot reach boundary mode at all and is not
faulty for it** — `G-300-U` at 2 W would need 8,3 mH, and the datasheet's own front-page
application regulates from 30 mA to 4 A.

### What that gives on each board

| | ratio | `L_PRI` | `R_SENSE` / `R_SNS` | the part |
|---|---|---|---|---|
| **`G-48-S`** 12 → 48 V | 1:4,42 | 14 µH | **9,1 mΩ** | **`750310988`** |
| **`G-48-U`** 48 → 12 V | 2,5:1 | 14 µH | **15 mΩ** | **`750311607`** |
| **`G-300-S`** 12 → 300 V | 1:10 | 5 µH | **5,5 mΩ** | **`750310349`** |
| **`G-300-U-40`** 300 → 12 V | 8:1:1, `N_TS` = 1 | 670 µH | **58 mΩ** | **`11328-T078`** |
| **`G-300-U-6`** 300 → 12 V | 14:1:1,7 | 1000 µH | **130 mΩ** | **`11338-T195`** |
| **`G-24-S`** 12 → 24 V, switched | 1:1 | 8 µH | **10 mΩ** | **`750311592`** |

**Six positions, six order codes, and no winding is wound to order.** The `LT3748` positions
need no tertiary at all — it samples the primary — so what those boards want is an ordinary
two-winding part, and the catalogue has them. Only `G-300-U` carries a `N_T`, because `LT8316`
samples a third winding. `750310349` carries both loads of `G-300-S` with the same shunt.

## The frequency window — what one winding can cover, and why it stopped mattering

**In boundary mode a single winding covers only a bounded range of power, and the bound is the
switching frequency rather than the core:**

```
   P = ½ · I_pk · (V_in · V_refl)/(V_in + V_refl)        L cancels — power is I_pk and the voltages
   f = 1 / (L · I_pk · (1/V_in + 1/V_refl))              L sets where the frequency sits

   P is LINEAR in I_pk and f goes as 1/I_pk
   →  the power range a winding covers IS the frequency range it is allowed
```

| | the `LT3748` positions | the `LT8316` board |
|---|---|---|
| floor | none as a rule — a converter that shares a box with a measuring board is placed, not tuned (`README.md`, *Converters and measuring boards*); the load-dependent sweep is what makes a frequency plan impossible | the same |
| ceiling | **≈ 1,05 MHz**, derived from the 250 ns on-time and 700 ns off-time | **140 kHz**, the part's clamp |
| **window** | the part's own — the 42 kHz discontinuous floor to ≈ 1,05 MHz, **25 : 1** | **2 : 1** — 70 kHz to the 140 kHz clamp before burst |

**What the window still decides is the inductance floor**, and on the cable side it is tight. A
2 : 1 window means `L_pri` has to be picked so that **full load sits at or below 140 kHz** — below
that inductance the part is already discontinuous at its rated power, which is legal but puts `I_pk`
above what the boundary arithmetic predicted. Both floors are in *`LT8316`, read*.

**And a board whose rated power is small enough cannot reach boundary mode at all.** `G-300-U` at
2 W would want 8,3 mH; it runs DCM and bursts, which is what the part is built to do. **DCM at rated
power is a fault on a 20 W board and normal on a 2 W one** — the distinction is the rating, not the
topology.

**Every position is filled by a catalogue part — this is what is fitted and what it has to clear.**

| | `G-48-S` | `G-300-S` | `G-48-U` | `G-300-U-40` · `G-300-U-6` | `G-24-S` |
|---|---|---|---|---|---|
| **fitted** | `750310988` | `750310349` | `750311607` | `11328-T078` · `11338-T195` | `750311592` |
| ratio | 1:4,42 | 1:10 | 2,5:1 | 8:1:1, `N_TS` = 1 · 14:1:1,7 | 1:1 |
| `L_pri` | 14 µH | **5 µH** | 14 µH | 670 µH · 1000 µH | 8 µH |
| `I_SAT` | 15 A min | 25 A typ | 9,5 A | rated 3,0 A · 0,9 A | 18 A |
| **what actually binds it** | the shunt: `I_pk` **9,90 A** at 9,1 mΩ, against 15 A of saturation | the **sampling rule**: 30,1 V reflected at 1:10 wants `L ≥ 4.41 µH` at 5,5 mΩ, against 4,5 at the winding's low corner — the shunt cannot go larger; the 110 mV corner is 20,0 A, 80 % of 25 A | the **sampling and on-time rules**: both want `L ≥ 12.5 µH` at 15 mΩ, against 12,6 at the winding's low corner — the shunt cannot go larger; the 110 mV corner is 7,33 A, **77 %** of 9,5 A | the shunts: 58 mΩ, `I_pk` **1,55 A**, 63 % of the winding's rated current at the 110 mV corner · 130 mΩ, the largest the sampling floor allows, 0,33 A at full load | the **sampling rule**: 24,7 V reflected at 1:1 wants `L ≥ 6.6 µH` at 10 mΩ, against 7,2 at the winding's low corner — the shunt cannot go larger; `I_pk` 9,0 A is 61 % of 18 A at the 110 mV corner |
| tertiary | **none** — `LT3748` samples the primary | none | none | **yes** | none |
| isolation | 1500 V AC | **1000 V AC — under the family's 1,5 kV floor**, accepted for the concept; a production run orders a custom-insulated winding | 1500 V AC | reinforced 3 kV · basic 3 kV | 1500 V AC |

**The rule on insulation, stated once:** above 3 kV would be the ideal on every winding, and the
catalogue parts that fit the cell were taken with the figure they have — 1,5 kV on the 48 V
boards, 1 kV on `G-300-S`, 3 kV on `G-300-U-40` and `G-300-U-6` — knowingly, for the concept. A production run
that wants the ideal orders custom-insulated windings on the same cells.

## The cells — drawn once, instantiated by every board

Cells are drawn once and instantiated by every board
that needs them; a board's own drawing then carries only what is its own:

| cell | who instantiates it |
|---|---|
| **the buck cell** — `LMR43610`, the worksheet above; `LMR43620` the same cell one size up; `TPS629206` the small one | every host, on its own board: a unit's buck · the card (`LMR43620` and two `TPS629206`) · Quake (one `TPS629206`) · the MODs (`LMR43610` — the basic set; Pluvius twice, its 5,5 V branch too) · the H7A3 boards (two `TPS629206`, Tesla three) · Mayak, Kronos. **No Galvani sheet instantiates it** |
| **the feed cell** — `LT3748` + the `ISC165N15NM6` + `R_SENSE` + its catalogue winding, off the 12 V tapped on the board's terminals | `G-48-S` · `G-300-S` · `G-24-S`, where `ENABLE` is the switch and the cell stands off between runs |
| **the cable-side island cell** — `R_SNS` + the DCM network + the FB/TC divider, **the controller follows the input voltage**: `LT3748` + the `ISC165N15NM6` on **`G-48-U`** off 48 V; `LT8316` + `FCD260N65S3` on **`G-300-U`** off 300 V (*Choosing the FET*) | `G-48-U` · `G-300-U-6` · `G-300-U-40` |
| **the 12 V output** — the island's output onto the two-pole terminal, the only place 12 V leaves the board; no buck, nothing to switch | `G-48-U` · `G-300-U-6` · `G-300-U-40` |
| **the measuring-side island** — `SN6505B` + `750313734` + 2× `PMEG10020ELR`, `EN` tied on | every power board |
| **the telemetry front** — `INA238` + the 1210 shunt + the `VBUS` divider and its 10 nF + `ISO1642` (the I²C both ways, `A_SEL` down on channel A to the `INA238`'s `A0`, `ALERT` up on channel B, high = alarm with `APOL` = 1; `ENABLE` needs no channel, it never crosses the barrier); the host side on the host's 3,3 V; the measuring side on the board's island; on `G-12-S` the VBUS series resistor is not needed | every power board |
| **the isolated line-side supply** — `SN6505B` + `750313734` + `PMEG10020ELR`, `LINE_EN` on its `EN` with 100 kΩ to ground | `G-I-N-025` · `G-I-M-005` |
| **the protection ladder** | every board that meets a cable |
| **the connectors** — the data connector and the power connector, and what each board takes and gives on them | every board |

**No board carries a ranging loop.** The return for the line-delay measurement is the unit's own
timer — capture on `RXD`, a timed pulse out of `TXD` (`../bifrost/HARDWARE.md`, *Ranging*) — so
there is no gate, no `OE`, and a communication board is transceivers, an isolator and a supply
and nothing else.

**The pin assignment is settled once for the family** — the data connector's twelve and the power connector's eight (`README.md`, *The connectors*). Every board, host and cable uses it.

## The cables — the pair map and the feed cable's reach

**The data cable is UTP Cat 6, system impedance 100 Ω.** House colour reference: the
white-striped core of a pair is A, the solid core is B. On a copper spur the four pairs map the
NodBus exactly: **orange = data TX · blue = clock · green = signal ground (both cores) · brown =
data RX**. **The feed cable is 2× 1,5 mm² or 2× 2,5 mm², 300/500 V class**: solid = +, striped =
return. Gel- or grease-filled cable is not used — stiff, multi-jacketed, dear to buy and to
pull.

**Termination:** 80,6 Ω across the pair behind a jumper on a 2-pin header, on the first and the last board of a segment, the two 10 Ω legs on the board completing the ~100 Ω — 80 + 10 and not 90 + 5, because the larger series leg roughly halves the `SM712`'s current before the tube fires, ~4 A against ~7 A, for a dB of signal that Cat 6 does not miss; not
120 Ω, which belongs to dedicated 485 cable this project does not use. **The 10 Ω series
resistors stay on every board, terminated or not** — they sit in front of the clamp array and
halve the `SM712` current before the GDT fires; driving into ~100 Ω, 20 Ω of series costs about a
quarter of the amplitude against an RS-485 margin of several times the receiver threshold, and
the receive input is ≥ 12 kΩ.

**The feed cable's reach is a constant-power problem.** The far end is a converter, so a falling
line makes the current rise and the run ends at a cliff, `R_loop = V²/4P` — **288 Ω at 48 V and
2 W**. The working point is three quarters of the cliff (`V_end = 75 % V₀`), which leaves room
for the copper warming up (+0,39 %/°C). Copper loop resistance is **34,4 / A[mm²] Ω/km**, both
conductors, there and back:

| copper, each of 2 | loop Ω/km | cliff at 48 V, 2 W | **working reach** |
|---|---|---|---|
| 0,5 mm² | 68,8 | 4,2 km | 3,1 km |
| 0,75 mm² | 45,9 | 6,3 km | 4,7 km |
| 1,0 mm² | 34,4 | 8,4 km | 6,3 km |
| **1,5 mm²** | 22,9 | 12,6 km | **9,4 km** |
| **2,5 mm²** | 13,8 | 20,9 km | **15,7 km** |
| 4,0 mm² | 8,6 | 33,5 km | 25,1 km |

At 500 m on 1,5 mm² the 3 W spur drops 0,73 V and lands 47,3 V. On glass the feed is the same
2-core cable or the metallic cores of a hybrid opto-power cable, which carry 2× 1,5 and 2× 2,5
mm² as the workhorses.

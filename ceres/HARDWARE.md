★ N.I.C. ★

# Ceres — hardware

> **Design-stage concept — nothing built.** The figures are verified against the parts' sheets
> and the parts before a board is made.

**One board for Ceres and Sakura, and one enclosure.** The electronics is identical and so is the
build — a round board in a borosilicate Petri dish, the comb on its underside reading through the
glass (*The bowl*, below); what differs is the height of the spacer ring, so the thickness of the
epoxy behind the board, what closes the back on Sakura, where the dish hangs, and the calibration
curve in its cells. This document is the one record; `sakura/HARDWARE.md` carries only what
differs.

## What it measures, and how

The comb under the glass against the medium is a capacitor with a loss — an admittance
`Y = G + j·2πf·C`. **`C` is the water** (permittivity ~80 against ~4 for dry soil or a dry leaf)
and **`G` is the ions** in it. A read that sees only the magnitude of the two mixes them, and a
sensor that mixes them reads salinity as moisture. The board therefore reads **both parts of the
admittance, at three frequencies**: `C` barely moves with frequency where `G` and the electrode's
interface do, so 2²⁰, 2²² and 2²⁴ Hz — 1,05, 4,19 and 16,8 MHz — separate the water from the ions and from the coating's own
polarisation.

**The read is a synchronous detector.** A square wave of known phase drives the electrode through
a series resistor `R`; the voltage on the electrode node is the divider `R` against `Z_x`. A
follower buffers that node and two switches multiply it by a reference square wave at 0° and at
90° — a CMOS SPDT switch with the follower's `+` output on one input and its `−` output on the
other, clocked by the reference, is a multiplier by ±1. The RC after each switch keeps the mean:
**`I`, in phase with the drive, and `Q`, a quarter period behind it.** With the reference
capacitor read the same way in the same cycle,

```
V_x = I + jQ                        (after the C_ref read fixes the chain's gain and phase)
Z_x = V_x · R / (V_drive − V_x)
Y_x = 1 / Z_x  = G + j·2πf·C
```

The square wave's harmonics are detected by the switch too (a square reference has them at 1/n);
the series `R` into the electrode capacitance already rolls them off, and `C_ref` is read through
the same chain, so what remains is the same on both reads and cancels. **The read is a ratio to
`C_ref`** — the driver's amplitude, `R`'s tolerance, the switches' on-resistance and the chain's
phase delay drop out of it.

## The chain

```
 TIM1 @ 2²⁷ Hz ──CH1 0°──▶ 74LVC1G17 ──R──┬── ELECTRODE
                                           │
   CH2 0°  ──────────────────────┐    SEL──┤── 74LVC1G3157 ──R── C_ref
   CH3 90° ───────────────┐      │         │
                          │      │      THS4541 (follower, VOCM 1,65 V)
                          │      │        + │ │ −
                          │      └──▶ 74LVC1G3157 ──RC──▶ ADC1 diff ── I
                          └─────────▶ 74LVC1G3157 ──RC──▶ ADC2 diff ── Q
```

| stage | part | notes |
|---|---|---|
| **timebase** | `TIM1` on the H523, clocked at **2²⁷ Hz = 134,217728 MHz** from the PLL off the house 2²⁴ crystal — a power of two like every clock in the station | three outputs of one timer: `CH1` the drive, `CH2` the I reference, `CH3` the Q reference. Same `ARR`, and 90° is a `CCR` offset of a quarter of it |
| **frequencies** | **2²⁰ / 2²² / 2²⁴ Hz — 1,05 / 4,19 / 16,8 MHz** | `ARR` 127 · 31 · 7; the quarter period 32 · 8 · 2 ticks. One frequency at a time — the firmware reprograms `ARR`/`CCR` between reads |
| **driver** | `74LVC1G17` | a Schmitt buffer on the 3,3 V; 24 mA class, enough to swing 100 pF through `R` at 16,8 MHz — the edge slopes and the detector reads the fundamental anyway |
| **series R** | **220 Ω** | picked so the divider sits mid-range at 4,19 MHz for the dry electrode; a 16× frequency spread moves the divider from ~0,06 to ~0,94 of the drive and the ratio read tolerates it. One `R` for all three |
| **follower** | `THS4541` | 850 MHz GBW fully differential amplifier, **gain 0,8**, single-ended in from the electrode node, differential out; `VOCM` on a 1,65 V divider, 2× 10 kΩ + 100 nF. **Its input is the `RG`/`RF` network, not a high-impedance pin**: with `RG` **2 kΩ** and `RF` **1,6 kΩ** (not the sheet's 402 Ω) it presents ~2,6 kΩ to the node — a constant of the board, the same on the electrode and the `C_ref` read, de-embedded by the firmware from the `C_ref` and open-pad reads. Its input capacitance (~1 pF) sits on the node the same way. `PD` is active-low, pulled down: the follower is off until the firmware raises `AMP_EN` |
| **detectors** | `74LVC1G3157` ×2 | SPDT; on at `VCC` 3,3 V, the sheet's 3 V row: **7 Ω typ / 9 Ω max at 0 V in, 9 / 20 Ω at 3 V in, 25 Ω max over the signal range** in SOT-23 (`DBV`), flatness 9 Ω — the same in both reads, so it is in the ratio; `t_pd` 0,8 ns, `t_en`/`t_dis` 2,5/1,5 ns typ (7,6/6 max); the follower's `+` on `B1`, `−` on `B2`, the reference on `S`, `A` is the product. **The switch's 17,3 pF `C_on` (5,2 pF off) sits on the follower's output**, which is a low impedance, so it costs nothing on the measurement |
| **selector** | `74LVC1G3157` ×1 | `A` to the follower's input, `B1` the electrode node, `B2` the `C_ref` node, `S` a GPIO. Its capacitance sits on both nodes alike and is in the ratio |
| **C_ref** | **100 pF** NP0, 1 % | driven through its own `R` of the same value; read every cycle |
| **RC** | 1 kΩ · 100 nF | 1,6 kHz corner — the detector output is DC plus 2f ripple, and the ADC's oversampling averages what the RC leaves |
| **ADC** | `ADC1` (I) and `ADC2` (Q), **differential mode**, `IN+` the RC node, `IN−` `VOCM` | both sampled at once on one trigger; **hardware oversampling 256×** to a 16-bit-class result. The differential read is why zero phase reads zero and the sign survives |
| **MCU** | `STM32H523`, LQFP100 | the timer, the two ADCs, the arithmetic, Modbus; a 2²⁴ crystal, 16,777216 MHz, the house part, and the PLL makes 2²⁷ from it — the leaf bus carries no clock. **The crystal stays for the baud, not the read**: 19 200 out of 2²⁷ is a divisor of 6990,5 with the USART's fractional baud register, **0,007 % of error**, 9 600 the same — no other crystal does better, and the internal RC at ±2 % over temperature would sit at the edge of RTU's tolerance |
| **line front, the basic set** | `THVD1450` + 2× 10 Ω + `SM712` on the pair; **2× `5.0SMDJ18A` on the 12 V input** | the arm's transceiver and every MOD's protection, on this board (`../core/blocks/modbus.md`) |
| **supply** | **one switching node, no LDO** | **`LMR43610`** at **3,3 V** off the arm's isolated 12 V on four wires — the MOD's buck, **36 V in, so it outlives the transils' ~29 V clamp** — the one rail, an inductor per branch — *The rails*, below |
| **thermometer** | `TMP117` or `STS35` on I²C — two makers, one fitted — **on the board's top face at the comb region's centroid**, under the lid's glass, 4,7 kΩ pull-ups, 100 kHz; read between cycles, the front end off | the compensation, and the plate's temperature reported on `0x0001` — the soil temperature at the depth (Ceres), the temperature at the plate (Sakura) (`README.md`) |

**A read cycle.** Select `C_ref`, run the three frequencies, then select the electrode and run
them again — six `(I, Q)` pairs, each a burst of 2048 oversampled conversions, 7,8 ms; ~53 ms
in all, once per configured interval, the driver and the follower off between cycles. The
firmware computes `Y_x` per frequency and reports the finished value and the raw counts
(`README.md`, the registers).

## The bowl — the enclosure is the electrode's cover

**No connector on the unit.** The four-wire cable is soldered to the board and leaves through the
bowl's channel; its far end is stripped into the arm board's terminals at Palatine, like a bought
sensor's. **One build for both units: a borosilicate bowl as the measuring glass, one round board
potted into it comb-down, a spacer ring on the board, and the fill poured solid up to the back that
closes it.** No air anywhere, in either unit. What differs is the height of the ring and what
closes the back, and where the bowl hangs; Sakura's are `sakura/HARDWARE.md`.

| | |
|---|---|
| **the bowl** | a borosilicate 3.3 bowl of Ø ~90 mm — the **lid** of a Simax Petri dish, or any dish of the size bought alone — inner Ø ~92, wall ~8 mm, bottom 0,8–1,2 mm as delivered: the measuring glass. Borosilicate: zero water uptake, UV-stable, inert; a clean surface wets like a leaf (contact angle 20–30°). Not soda-lime, which leaches and grows a conductive gel layer in wet soil |
| **the board** | **Ø 84, 1,6 mm FR4, two layers**, smaller than the bowl so the spacer ring stands inside it. Bottom copper: the **comb**, two interleaved sets, pitch **3 mm** to the 1 mm glass, over three quarters of the disc. Top: every part in the remaining quarter, and the `TMP117`/`STS35` at the centroid of the comb region, ~15–20 mm in from the rim, away from the channel. **1,6 mm and not thinner, because the comb's field goes both ways**: a coplanar comb splits its field between cover and substrate by permittivity and penetrates ~pitch/π ≈ 1 mm each way; in 1,6 mm of FR4 (0,1 % water uptake) the substrate half is a constant, where over a thin board it would sit in the fill and drift as that hydrates |
| **the field** | leaves through the bowl's glass into the medium. The cover stack is 50 µm of adhesive and 1 mm of glass, ~40 pF/cm² against a few pF/cm² of medium — a series capacitance that hardly costs sensitivity; its loss (tan δ ~0,004) is under a percent of a wet medium's `G` |
| **the fill** | **a potting compound that is inert and takes up little water** — epoxy of a low-uptake grade is the reference, and any compound that meets the two conditions serves; *epoxy* below stands for it. In the channel it is the only material that meets the outside. Not hot melt: polyamide grades take 1–3 % water, polyolefin grades do not bond to glass (`WHY.md`) |
| **the spacer ring** | a slice of PP or PVC tube, wall 2 mm, standing on the board; the fill is poured in excess, the back pressed onto the ring, the excess leaves by the channel, so no air is left under it. **No single height — the ring follows the tallest part**: the buck's 10 µH, **4,0 mm high at most**, stands tallest, the two `5.0SMDJ18A` at ~2,4 mm, the LQFP 1,6, the 1206 capacitors and the filter inductors 1,2 — nothing on the board is taller, which is why its 12 V bulk is ceramic (*Decoupling*). **Ceres's ring runs to the bowl's rim less the disc's thickness**, so the whole bowl is one block |
| **the back, on Ceres** | **a borosilicate disc Ø ~88 × 1–2 mm, flush with the rim** — glass on both faces and a solid block between them, so **the sand's load goes through the block and not through an unsupported plate**: 1 mm of glass over 86 mm with air behind it would see ~18 MPa under 50 cm of sand, past annealed borosilicate's long-term ~7 MPa. Mass is nothing in the ground |
| **the channel** | **~1 mm wide, ~8 mm long**, up between the outer wall and what sits inside it, filled with the compound — the one seal — and the flat cable leaves through it |
| **the cable** | **flat, four conductors, ≤ 1 mm thick, a telephone-class flat lead with a UV-stable jacket** — FEP or PUR — so it passes the channel and leaves the smallest gap for the fill. Not the IDC ribbon: its insulation is not for the sun, and a sleeve would not pass the channel |
| **assembly** | a few millilitres of the compound in the bowl; the board pressed in at an angle so no air is trapped under the comb; the whole degassed under vacuum, which seals the laminate's edges too; the ring set on the board and the compound poured past its top; the back pressed onto the ring, the excess out by the channel; the cable set in the channel before the compound gels. **Every part is one JLCPCB places**, from LCSC stock |

**Ceres lies horizontal in the patrona's packed sand, glass down**, its cable up the tube, so its
own weight keeps the glass on the sand as the column settles. The sensed layer is the few
millimetres under the glass, acceptable **only in the packed matrix**, which is homogeneous and
was tamped against the glass on the bench. **The thermometer measures the plate**, and on Ceres
the plate is the soil at the depth.

**The unit sleeps between reads for the thermometer's sake, not the battery's.** Awake, the
processor at 2²⁷ Hz and the front end would put ~100 mW into a sealed bowl of 63 cm² and lift the
plate 1–2 K above the medium it reports — on Ceres a false soil temperature, on Sakura a plate
that forms no dew. So between cycles the H523 is in Stop (~30 µA), the `THS4541` in power-down,
the thermometer in shutdown, the buck at ~1,5 µA and its divider at ~80 µA, and awake stays only
the `THVD1450`'s receiver, ~1,5 mA: **~5 mW at 3,3 V, ~7 mW off the 12 V, ~0,04 K over the bowl**;
a read cycle is ~53 ms a minute (`FIRMWARE.md` §3). The crystal stays — its driver is ~0,5 mA
while running and off in Stop, where an internal RC at ±2 % over temperature would put 1–2 % into
`C` (computed from ω) and sit at the edge of RTU's baud tolerance. The `THS4541` is switched at
its `PD` pin, never at its rail: a powered-down part on a live rail takes tens of µA, an unpowered
part with the selector and the references still driving it takes the drive through its input
protection.

## The rails

**One switching node on the board and one rail, 3,3 V, split into four branches by inductors.**

```
 12 V from the arm ─▶ 2× 5.0SMDJ18A ─▶ LMR43610 ─▶ 3,3 V ─┬─ L1 ─▶ STM32H523 · THVD1450 · the thermometer
                                                           ├─ L2 ─▶ VDDA / VREF+
                                                           ├─ L3 ─▶ THS4541 · the VOCM divider
                                                           └─ L4 ─▶ 3× 74LVC1G3157 · 74LVC1G17
```

| branch | what sits on it | draw |
|---|---|---|
| **L1** | `STM32H523` · `THVD1450` · the thermometer | ~110 mA, of which the 485 driver is the step |
| **L2** | `VDDA` / `VREF+` | a few mA |
| **L3** | `THS4541` · the `VOCM` divider | ~10 mA |
| **L4** | the three `74LVC1G3157` · the drive gate `74LVC1G17` | ~10 mA in edges during a read |

**Everything is on one level.** The drive gate, the selector, the detectors, the follower and the
processor all stand on the 3,3 V, so every signal between them stays inside every part's own
supply: the electrode node reaches 0,94 of the drive, 3,10 V, and the selector and the follower's
input take it inside their rails.

**The `THS4541` at 3,3 V**: 2,7–5,4 V single supply, 9,7 mA, PSRR 85 dB minimum. Its input
common-mode range is `VS−` − 0,1 to `VS+` − 1,3 = 2,0 V; with `RG` 2 kΩ, `RF` 1,6 kΩ and a
single-ended source the input pins sit at `VOCM`·0,56 + `V_node`/2·0,44 = 1,61 V at the top of the
node's swing. Its outputs run 0,25–3,05 V; `VOCM` is 1,65 V off a 2× 10 kΩ divider with 100 nF on L3's branch.

**The gain is 0,8 so the outputs stay inside that range.** Each output moves from `VOCM` by half
the node voltage times the gain. At the node's 3,10 V and gain 1 the outputs would stand at 3,20 and
0,10 V, past both limits; at 0,8 they stand at 2,89 and 0,41 V. The read is a ratio to `C_ref`
through the same chain, so the gain drops out of the result.

**`VDDA` and `VREF+` are on L2's branch.** The read is a ratio to `C_ref` through the same chain,
so neither the reference's absolute value nor its drift reaches the result.

**The buck's inductor is 10 µH, shielded, `I_SAT` ≥ 2,1 A** (the `LMR43610`'s peak limit): a
ripple of 0,12 A at 3,3 V out, so a read's ~145 mA keeps the valley above the zero-cross limit, in
CCM at 2,08 MHz rather than at the edge of PFM.

**L1–L4 are `SWPA252012S2R2MT`** (Sunlord, LCSC/JLCPCB stock) — 2,2 µH, shielded wire-wound,
2,5 × 2,0 × 1,2 mm, `DCR` 0,166 typ / 0,199 max Ω, `SRF` 69 MHz min, `I_SAT` 1,85 A, `I_RMS`
1,15 A. Into 4,7–10 µF it is a 20–50 kHz corner, ~50–60 dB at the 1,05 MHz read and ~70 dB at the
buck's 2,08 MHz, its self-resonance four times the 16,8 MHz read; the `DCR` is the damping — Q 2,6
into 10 µF, ~4 into 4,7 µF, a peak of ~+12 dB at ~50 kHz where nothing on the board runs. **No
ferrite bead on this board**: every filter stands against the buck's 2,08 MHz, where a bead is a
few ohms (`../core/POWER.md`, *The filter parts*).

**The drive gate shares L4 with the switches.** Its `C·V·f` = 100 pF × 3,3 V × 16,8 MHz = 5,5 mA
comes in edges synchronous with the detection, the same on the electrode read and the `C_ref`
read, so it cancels in the ratio; each part's own 100 nF within 2 mm of its `VCC` carries the edge.

| inductor | consumer | at the pins |
|---|---|---|
| **L1** | `STM32H523` · `THVD1450` · the thermometer | 100 nF at every `VDD`, 10 µF bulk; 100 nF + 10 µF at the transceiver; 4,7 µF + 100 nF at the thermometer's `V+` |
| **L2** | `VDDA` / `VREF+` | 10 µF + 100 nF at the pins |
| **L3** | `THS4541` and the `VOCM` divider | 10 µF + 100 nF at `VS+`, the divider's 100 nF |
| **L4** | the three `74LVC1G3157` and the drive gate | 10 µF + 100 nF, and 100 nF within 2 mm of each `VCC` |

### Decoupling

**Three rules and the values follow them.** ① **The 12 V input side is 50 V, and its bulk is
ceramic** — the two transils clamp at ~29 V, and the family's 47 µF hybrid polymer is an
8 × 10,2 mm can that does not fit the ~4 mm the bowl leaves above the board, so the reservoir is
the house 10 µF 50 V 1206, four of them, and the 4,7 µF 50 V at the buck; no 100 V ceramic is
kept for this position alone. ② **Below the buck every capacitor is a 50 V X7R** — the project's
rule, `../core/POWER.md`. ③ **Every timing and feedback position is NP0/C0G** — the crystal's
load pair, `C_ref`, the detectors' RC — because an X7R's value walks with voltage and
temperature and those positions are in the arithmetic. **The values are deliberately generous**:
where a part wants 1 µF it gets 10, so a bias curve worse than the sheet's is absorbed by count.

Read in the order the current travels:

| position | |
|---|---|
| **the unit's 12 V input**, behind the two transils | **4× 10 µF 50 V 1206 + 100 nF 50 V** — 50 V against the transils' ~29 V clamp — the reservoir the whole board's steps come out of |
| `LMR43610` **`VIN`** | **4,7 µF 50 V + 100 nF 50 V**, at the pin — the house cell |
| `LMR43610` **`VCC`** · **`BOOT`** | **1 µF** · **100 nF** ≥ 10 V, `BOOT` to `SW` — its sheet |
| `LMR43610` **divider** · **`CFF`** | **28,0 k / 12,1 k** → 3,31 V and **22 pF C0G** — the house cell |
| `LMR43610` **out**, the 3,3 V node | **3× 10 µF 50 V 1206 + 100 nF** — the charge every branch pulls its steps from, so it is the one not to shrink |
| `STM32H523` | **100 nF at every `VDD`** + **10 µF** bulk; **`VCAP` 2× 1 µF 50 V 0805 + 2× 100 nF 50 V 0603 on each of the two pins** (48, 98) — 2,2 µF, the sheet's 2,2 µF ±20 %, ESR < 100 mΩ |
| `THVD1450` | **100 nF + 10 µF** — its transmit step is the largest on the board |
| `THS4541` | behind **L3**: **100 nF + 10 µF** at the supply pin, close; the `VOCM` divider and its 100 nF off this node |
| the three `74LVC1G3157` and the drive gate | behind **L4**: 10 µF + 100 nF, and **100 nF at each `VCC`, within 2 mm** — the switches' `C·V·f` is 50 pC an edge and the gate's 5,5 mA comes in edges; that capacitor is what supplies them |
| `VDDA` / `VREF+` | behind **L2**: **10 µF + 100 nF** at the pins |
| the thermometer | behind **L1**: **4,7 µF + 100 nF** at `V+` |

**DC bias is counted, not avoided.** A 50 V X7R 1206 can sit near half its marking at 12 V, so
four of them leave ~16–20 µF — against a board that draws ~0,5 mA from the 12 V at rest and
~50 mA through a read, with the arm's power board behind the cable. The hybrid polymer that does
not derate is the house answer where a board has the height for it (`../galvani/HARDWARE.md`);
the bowl has not, and a ceramic chosen for its bias curve beats a bigger nominal value.

## The processor — pins, timers, clock tree

*The record to draw the H523 from and to set CubeMX by. Package LQFP100, `STM32H523VE`; pin
numbers and alternate functions from DS14540 Rev 3, Table 13. Every pin below is either
assigned or listed free — nothing is left to guess.*

### Peripherals

| peripheral | instance | pins | job | mode |
|---|---|---|---|---|
| the arm | **USART1** | PB6 `TXD` · PB7 `RXD` · PE2 `DE` | the `THVD1450` — a ModBus slave | RTU, 19 200 8N1; `RE#` tied low |
| the timebase | **TIM1** | PE9 `DRIVE` · PE11 `REF_I` · PE13 `REF_Q` | one timer, three outputs: the drive and the two references; `TRGO` triggers both ADCs | PWM, `ARR` 127 · 31 · 7 for 2²⁰ · 2²² · 2²⁴ Hz; CH3's `CCR` a quarter of `ARR` |
| the detectors | **ADC1** + **ADC2**, dual simultaneous | PC0/PC1 `I` · PC2/PC3 `Q` | differential, `IN−` on `VOCM`; hardware oversampling 256× | triggered by TIM1 `TRGO`, both at once |
| the selector · the follower | GPIO | PE5 `SEL` · PE6 `AMP_EN` | the `74LVC1G3157` selector; the `THS4541`'s `PD`, active-low, pulled down | high = the follower on |
| thermometer | **I2C1** | PB8 `SCL` · PB9 `SDA` | the `TMP117`/`STS35` on the board, at the comb region's centroid | 100 kHz, 4,7 kΩ pull-ups; address 0x48 or 0x4A, one driver tells them apart |
| clock | **HSE crystal** | PH0 · PH1 | 2²⁴, the house crystal | |
| debug | **SWD** | PA13 · PA14 | | no JTAG, no trace |

The two differential pairs are `INP10`/`INN10` and `INP12`/`INN12` — the H523 pairs an `INPx`
with the `INNx` on the next pin, which is why `I` is PC0/PC1 and `Q` is PC2/PC3 and not any four
ADC pins.

### Pins

| pin | port | signal on the board | AF / function | dir | note |
|---|---|---|---|---|---|
| 1 | PE2 | `DE` | GPIO | out | the `THVD1450`'s `DE`; `RE#` tied low — the receiver is never off |
| 2 | PE3 | — | | | free |
| 3 | PE4 | — | | | free |
| 4 | PE5 | `SEL` | GPIO | out | the selector's `S` — the electrode or `C_ref` |
| 5 | PE6 | `AMP_EN` | GPIO | out | the `THS4541`'s `PD`, active-low with a 100 kΩ pull-down — low (and reset) is off, high is on |
| 6 | VBAT | — | | | tied to `VDD`; no RTC anywhere |
| 7 | PC13 | — | | | free (tamper/RTC pin, unused) |
| 8 | PC14 | — | | | free — no 32 kHz crystal |
| 9 | PC15 | — | | | free |
| 10 | VSS | | | | |
| 11 | VDD | 3,3 V | | | |
| 12 | PH0 | `OSC_IN` | HSE crystal | | 2²⁴ = 16,777216 MHz, the house crystal |
| 13 | PH1 | `OSC_OUT` | HSE crystal | |  |
| 14 | NRST | reset | |  | 100 nF, no pull beyond the internal |
| 15 | PC0 | `I+` | `ADC12_INP10` | in | the I detector's RC node — ADC1, differential |
| 16 | PC1 | `I−` | `ADC12_INN10` | in | `VOCM`, 1,65 V |
| 17 | PC2 | `Q+` | `ADC12_INP12` | in | the Q detector's RC node — ADC2, differential |
| 18 | PC3 | `Q−` | `ADC12_INN12` | in | `VOCM` |
| 19 | VSSA | | | | |
| 20 | VREF− | | | | |
| 21 | VREF+ | the 3,3 V through L2, 2,2 µH — the ADC reference | | | |
| 22 | VDDA | the 3,3 V through L2 | | | |
| 23 | PA0 | — | | | free (ADC) |
| 24 | PA1 | — | | | free (ADC) |
| 25 | PA2 | — | | | free (ADC) |
| 26 | PA3 | — | | | free (ADC) |
| 27 | VSS | | | | |
| 28 | VDD | 3,3 V | | | |
| 29 | PA4 | — | | | free (ADC) |
| 30 | PA5 | — | | | free (ADC) |
| 31 | PA6 | — | | | free (ADC) |
| 32 | PA7 | — | | | free (ADC) |
| 33 | PC4 | — | | | free (ADC) |
| 34 | PC5 | — | | | free (ADC) |
| 35 | PB0 | — | | | free (ADC) |
| 36 | PB1 | — | | | free (ADC) |
| 37 | PB2 | — | | | free |
| 38 | PE7 | — | | | free |
| 39 | PE8 | — | | | free |
| 40 | PE9 | `DRIVE` | `TIM1_CH1` | out | the `74LVC1G17` — the electrode drive, 0° |
| 41 | PE10 | — | | | free |
| 42 | PE11 | `REF_I` | `TIM1_CH2` | out | the I detector's `S`, 0° |
| 43 | PE12 | — | | | free |
| 44 | PE13 | `REF_Q` | `TIM1_CH3` | out | the Q detector's `S`, 90° — a `CCR` offset of a quarter of `ARR` |
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
| 89 | PB3 | — | | | free |
| 90 | PB4 | — | | | free |
| 91 | PB5 | — | | | free |
| 92 | PB6 | `TXD` | `USART1_TX` | out | the `THVD1450`'s `D` |
| 93 | PB7 | `RXD` | `USART1_RX` | in | the `THVD1450`'s `R` |
| 94 | BOOT0 | 10 kΩ to ground | |  | |
| 95 | PB8 | `SCL` | `I2C1_SCL` | o/d | the thermometer |
| 96 | PB9 | `SDA` | `I2C1_SDA` | o/d | the thermometer |
| 97 | PE0 | — | | | free |
| 98 | VCAP | 2× 1 µF 50 V 0805 + 2× 100 nF 50 V 0603 — 2,2 µF, the sheet's 2,2 µF ±20 % | | | |
| 99 | VSS | | | | |
| 100 | VDD | 3,3 V | | | |

**20 GPIO used, 60 free** (PA0–PA12 · PA15 · PB0–PB5 · PB10 · PB12–PB15 · PC4–PC15 · PD0–PD3 · PD5–PD13 · PD15 · PE0 · PE3 · PE4 · PE7 · PE8 · PE10 · PE12 · PE14 · PE15).

### Timers

| timer | width | clock | channels used | role |
|---|---|---|---|---|
| **TIM1** | 16 | 2²⁷ | CH1 → PE9, CH2 → PE11, CH3 → PE13; `TRGO` → ADC1 + ADC2 | the read: `ARR` 127 / 31 / 7 makes 1,05 / 4,19 / 16,8 MHz, CH1 and CH2 at `CCR` 0, CH3 at a quarter of `ARR` — 32 / 8 / 2 ticks — for the 90° reference; the ADC trigger is the same timer's update through its repetition counter, `RCR` 3 / 15 / 63 — **262 144 Hz** at every frequency, so the oversampled burst is phase-locked to the drive and inside the converter's rate |
| **TIM6** | 16 | 2²⁷ ÷ prescaler | none | the RTU frame gap |
| **LPTIM1** | 16 | LSI | none | the wake from Stop: the thermometer every 10 s, the cycle every interval (`FIRMWARE.md` §3) |
| all others | | | | free |

One frequency at a time: the firmware rewrites `ARR` and CH3's `CCR` between reads, with the
outputs idle low, so the gate never sees a glitch.

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

`VDDA` and `VREF+` on the buck's 3,3 V through L2, the house 2,2 µH, 10 µF + 100 nF at the pins;
`VREF−` and `VSSA` to the ground plane at the island. **No precision reference**: the read is a
ratio to `C_ref` through the same chain, so the reference's absolute value and its drift drop out.
`VOCM`'s 1,65 V divider is on L3's branch, and both ADCs' `IN−` sit on it, so a zero-phase
read is a zero code.

## Bench criteria

- **The selector and the follower cancel:** a read of `C_ref` against a second `C_ref` on the
  electrode pads reads unity within 0,5 % in magnitude and 1° in phase at 16,8 MHz. If it does
  not, the two nodes are not laid out alike.
- **The 90° reference is a timer constant**, so the phase error is the driver's and the
  follower's delay, and the `C_ref` read removes it. Verify: an open electrode pad reads `G` ≈ 0
  at all three frequencies.
- **Parasitics:** the electrode node's stray to ground reads as a constant `C` offset and is
  subtracted by the open-pad read; keep it under a third of the dry electrode's own `C`.
- **The follower's input impedance is de-embedded, not ignored:** ~2,6 kΩ from the `RG`/`RF`
  network, in parallel with the node on both reads. The `C_ref` read and the open-pad read are
  two known loads through the same chain, which fixes the chain's gain, its phase and that
  impedance together; a third check, `C_ref` against a second `C_ref` on the pads, reads unity.
- **The buck is not in the band, and it is the only switching node:** the `LMR43610` switches at
  2,08 MHz, 7,50 kΩ on `RT` (`../galvani/HARDWARE.md`), and a read's ~145 mA holds it in CCM with
  the 10 µH — auto mode by its order code, PFM only in sleep — so the read at 1,05 MHz sits 1,0 MHz
  below it; the detector's RC and the
  oversampling reject the difference frequency. If a bench read at 1,05 MHz drifts with the buck's load, move that read to 2²⁷/112 = 1,20 MHz
  (`ARR` 111, quarter 28).

## Parts

| part | qty | where |
|---|---|---|
| `STM32H523VE`, LQFP100 | 1 | the processor |
| `SWXBEABVF0-16.777216` + 2× 12 pF C0G | 1 set | HSE |
| 2× 1 µF 50 V 0805 + 2× 100 nF 0603 | 2 sets | `VCAP` |
| 100 nF 50 V | one a supply pin | every `VDD` of the processor and every part's `VCC`, within 2 mm |
| 100 nF · 10 kΩ | 1 each | `NRST` · `BOOT0` to ground |
| `74LVC1G17` | 1 | the drive gate |
| 220 Ω | 2 | the series `R` into the electrode and into `C_ref` |
| 100 pF NP0 1 % | 1 | `C_ref` |
| `74LVC1G3157` | 3 | the I and Q detectors, the selector |
| `THS4541` | 1 | the follower |
| 2 kΩ · 1,6 kΩ | 2 each | `RG` · `RF` |
| 2× 10 kΩ + 100 nF | 1 set | the `VOCM` divider |
| 100 kΩ | 1 | `AMP_EN`'s pull-down on `PD` |
| 1 kΩ + 100 nF C0G | 2 sets | the detectors' RC |
| `LMR43610R3RPER` | 1 | the 3,3 V |
| 10 µH shielded, `I_SAT` ≥ 2,1 A, ≤ 4,0 mm high | 1 | the buck's inductor |
| 28,0 k · 12,1 k · 22 pF C0G · 7,50 kΩ | 1 set | the divider, `C_FF`, `RT` for 2,08 MHz |
| 4,7 µF 50 V + 100 nF · 1 µF · 100 nF | 1 set | `VIN` · `VCC` · `BOOT` |
| 3× 10 µF 50 V 1206 + 100 nF | 1 set | the 3,3 V node |
| 100 kΩ | 1 | `PG` up, to PD14 |
| 4× 10 µF 50 V 1206 + 100 nF | 1 set | the 12 V input |
| `5.0SMDJ18A` | 2 | the 12 V input |
| `THVD1450` · 2× 10 Ω · `SM712` | 1 set | the line front |
| 100 nF + 10 µF | 1 set | the transceiver's supply |
| 2,2 µH `SWPA252012S2R2MT` | 4 | L1 · L2 · L3 · L4 |
| 10 µF + 100 nF | 8 sets | before and behind each of L1–L4, π |
| `TMP117` or `STS35` + 2× 4,7 kΩ + 4,7 µF + 100 nF | 1 | the thermometer, I2C1, by population |
| 1 kΩ + LED | 1 | the status LED on PD4 |
| flat four-conductor cable, ≤ 1 mm, FEP or PUR | 1 | the arm, soldered to the board |

**The enclosure**: the borosilicate 3.3 bowl Ø ~90 (a Simax Petri lid, or a dish of the size) ·
the board Ø 84 · the spacer ring, PP or PVC tube, wall 2 mm · the borosilicate disc Ø ~88 × 1–2 mm
(Ceres's back) · the potting compound. Sakura's back and mounting: `sakura/HARDWARE.md`.

## Layout

A round board, Ø 84, 1,6 mm, potted comb-down into the bowl (*The bowl*). **The comb takes three quarters
of the bottom copper; the top face over it carries only the thermometer and its two traces**, so
the comb sees a constant stray and nothing switching. The parts sit in the remaining quarter: the
comb's feed, the `C_ref` node, the selector and the follower's input one small island at the
comb's edge; the buck and the transceiver at the rim, where the flat cable lands. The two detector RCs and
the two ADC inputs routed as pairs with `VOCM`. A ring of ground on the bottom copper around the
comb is the comb's return and the guard against the rim.

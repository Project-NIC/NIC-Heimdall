<p align="center">
  <img src="NIC-QuarkTubes.svg" width="200"/>
</p>

★ N.I.C. ★

# Quark-Tubes — the tube unit

> **Design-stage concept — nothing built.** The figures are verified against the parts' sheets and
> the parts before a board is made.

> **mini-NOD, NodBus mini type 2 `Quark-Tubes`** — the high-voltage build of Quark, the radiation part:
> every tube of the assembly counted on one H523, four channels in the 8 B mini payload behind an
> Argus. The radiation part is [`../README.md`](../README.md); the low-voltage build is
> [`../scintillation/`](../scintillation/).

**One board and the heads on it.** `Quark-Tubes` counts Photon's three GM tubes (K1 bare, K2
β-stopped, K3 behind Pb), the neutron detector on K4 — Helion's He³/BF₃ tube, or the
Gadolin/Rhodion ring's thirteen tubes merged — and feeds every GM tube from its one ~400 V module.
The heads carry no MCU ([`HEADS.md`](HEADS.md), [`gadolin/`](gadolin/)).

## What it detects

Two physically different jobs share one moderator / reflector / lead assembly (`../NEUTRONS.md`):

- **Gamma, in coarse energy bands.** GM tubes behind **graded Pb / PE absorbers** —
  differential absorption turns a set of plain tubes into a crude spectrometer
  (bare, β-stopped, behind Pb → **K1 / K2 / K3**, soft-to-hard — `HEADS.md`).
- **Neutrons.** A neutron is uncharged — it must first be converted. Two routes, both
  on cheap **stainless-wall SI-22G** GM tubes (family **(b)** and **(c)** in
  `../NEUTRONS.md`):
  prompt **gamma from ¹⁵⁷Gd capture** (~1 % efficiency, but allows a sub-ms
  coincidence window) or **delayed hard beta from an activation foil** (~30× more
  efficient per capture, but delayed by a half-life). Efficiency is honestly **~1–2 %**;
  the network, not the single node, is the instrument.

He³ proportional tubes (~70–96 %) are the high-end alternative but cost and need a
charge pre-amp; the tubes by fill and where to buy them are `../NEUTRONS.md`; the head is [`helion/`](helion/).

## Moderator / reflector

Fast neutrons are slowed in the moderator; the **graphite reflector** (closed, incl. ends) scatters
them back so more pass through the tube → **higher capture efficiency**.
At capture the reaction splits (He³ → p + t; Gd(n,γ) → prompt γ) → **one pulse; one neutron = one count**.
Frame count **saturates at 65535** (`uint16`, [`BUS.md`](BUS.md)); keep the reflector thin enough that neutron
**die-away ≪ 7,8 ms** (counts must not smear across frames).

## NIC integration

**Quark-Tubes is the tube unit: one board, a mini-NOD — NodBus mini type 2, `0x21` for unit 1.**
*(This section is Quark-Tubes'; the scintillation NODs' contract is in [`../scintillation/photon/BUS.md`](../scintillation/photon/BUS.md).)* It counts every tube itself, bins on the network clock, and a
bin rides its own frame — no polls, no markers, the slot is the time. The channel layout and the
whole bus contract are [`BUS.md`](BUS.md).

**It is a mini-NOD, and the clock is why.** The TGF work needs the counts and the strike on one
timebase (`../../tesla/DETECTION.md`), and ModBus gives no time. **NodBus mini does** — same framing,
same TDMA, same clock, 8 B of payload — so Quark-Tubes sits on an Argus segment and takes `CLK` onto a
**timer's external clock input**, where the bin boundary falls out as a whole-power-of-two
division of the segment's rung (`÷4096` of 2¹⁹ is the 128 Hz frame). **Its bins are the station's
frames, not its own oscillator's**, so there is no drift to correct and no PTP to run: the board
carries no crystal — the rung disciplines the HSI through the PLL, as on Gauss and Pascal — and the core
clocks the UART, and the only timing term left is the sync edge itself — ns-class,
fixed, ranged out at the card (`../../core/blocks/nodbus.md`).

**Mini type 2** (`../../core/PROTOCOL.md` §2).

- **Where the counting happens — all of it on Quark-Tubes.**
  - **Photon's tube build** (γ/X-ray, GM behind graded Pb) and **Helion's tube build** (He³
    proportional tube) stay **MCU-less pulse heads**; their shaped pulses go **to Quark-Tubes'
    H523**, on the same assembly, and land **straight on hardware timer inputs**. Counting them
    costs no CPU at all.
  - **Gadolin** (Gd converter) and **Rhodion** (its rhodium/Rh variant) keep the **13 SI-22G
    tubes (1 central + 12 in a ring) on EXTI**, merged in software so tubes lit by one particle
    count once. That merge is the **neutron channel** — Helion's tube fills the same channel on a
    build that carries it instead (`BUS.md` owns the layout).
  - **So the two jobs are split by hardware, not by board:** the 13-tube coincidence takes the
    software interrupts because it has to compare arrival times; everything else takes a timer
    input and is counted without the core noticing.
- **MCU:** the **H523** at 2²⁷ like every H523 in the station. The lightning front is Tesla's own
  H7A3 board ([`../../tesla/README.md`](../../tesla/README.md)), not here.
- **Transceiver:** on the communication board like every node; a CRC drops a frame a transient
  hit, and the next frame's accumulators carry its counts.
- **HV:** the one potted ~400 V module on the board, every GM tube on its bus
  ([`HARDWARE.md`](HARDWARE.md), *The one 400 V source*); the He³ tube on Helion's kV source ([`helion/`](helion/)).

### Remote HV — the source travels, the head stays (deployment topology)

The HV converter is the noisy part, and it does **not** have to sit at the sensor. The general
form on any detector: **generate HV at the head end and run it up the cable; at the remote site
sits only the detector — plus a read-amp where the signal needs one.** What is allowed to travel
is set by *signal integrity*, not convenience:

- **A robust volt-level pulse (GM tubes — Photon, Gadolin) goes straight down the cable.** No
  charge front to protect, so the whole detector head stays put and **only the HV source peels
  off** ([`gadolin/HARDWARE.md`](gadolin/HARDWARE.md) §2).
- **A charge front (the He³ tube) must be converted at the tube.** The transimpedance stage sits
  millimetres from the cathode; the cable carries the kV up, the 3,3 V up and the comparators'
  pulse down ([`helion/HARDWARE.md`](helion/HARDWARE.md) §1). The kV source stays on its own board.

The switcher's ringing stays off the sensitive front either way.

### Data model

Counts are **fast, co-timestamped** data — the whole point is to stamp a
radiation burst against a lightning strike on the one network clock. Each channel is
**one `uint16`, an accumulator reset on the second** (a GM tube maxes ~1000 CPS — the width matches the
scintillation units' format, not any tube); downstream sums to CPS/CPM, and a TGF burst stands
out as a co-timestamped spike. The counts ride the **8 B mini payload** — **beta · soft γ ·
hard γ · neutron derived on the unit, or the four detectors raw, K1..K4** — and [`BUS.md`](BUS.md) owns
the layout. The bins sit on the network clock and the slot is the time, so there are no offsets,
no markers and no age fields anywhere.

Three GM tubes of one type at one height over the plastic deposition plate — bare, β-stopped, behind Pb
— give the beta and the two gamma channels by subtraction; K4, the neutron channel, is
Gadolin/Rhodion or Helion's tube, whichever is fitted.

**The Pb thickness is climate-driven, not a fixed spec** — an **equivalent ideal-Pb thickness**,
the builder's choice (a no-hail site can run lighter); build the filter in Pb or another material,
but a different material means **converting to its Pb-equivalent and re-calibrating the tube**.
`HEADS.md` owns it.

## The tree

```
tubes/        Quark-Tubes — the board and the unit  mini 2
├── HEADS.md  Photon — the GM tubes K1–K3
├── helion/   Helion — the He³ / BF₃ tube on K4, and the kV source
└── gadolin/  Gadolin / Rhodion — the thirteen-tube ring on K4
```

## Files

| File | Contents |
|---|---|
| [`HARDWARE.md`](HARDWARE.md) | the board — the 400 V module and its bus, the five NTCs, pins, timers, the rung-disciplined clock |
| [`FIRMWARE.md`](FIRMWARE.md) | the firmware, described — the counters, the ring merge, the bin, the registers |
| [`BUS.md`](BUS.md) | the mini frame — four channels, raw or derived |
| [`HEADS.md`](HEADS.md) | the GM head (K1–K3, filters, the shaper board, the background at commissioning) |
| [`helion/`](helion/) | Helion — the He³/BF₃ head on K4 and the kV source |
| [`gadolin/`](gadolin/) | the Gadolin/Rhodion ring — its board and its construction |

## Licence

Hardware: CERN-OHL-S v2 (`../../LICENSE-HW`) · Software: MIT (`../../LICENSE`) — Copyright (c) 2026 NIC — Native Intellect Community

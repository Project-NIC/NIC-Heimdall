<p align="center">
  <img src="NIC-Gadolin.svg" width="200"/>
</p>

★ N.I.C. ★

# Gadolin — the Gd neutron head

**English** · [Čeština](README.cs.md) · [Русский](README.ru.md)

> **Design-stage concept — nothing built.** Shared neutron physics → [`../../NEUTRONS.md`](../../NEUTRONS.md); detector build →
> [`CONSTRUCTION.md`](CONSTRUCTION.md); readout electronics → [`HARDWARE.md`](HARDWARE.md); the
> graveyard → [`WHY.md`](WHY.md).

**Gadolin** is the **cheap** way to catch **neutrons**. It is named after **Johan Gadolin**, the Finnish
chemist behind gadolinium — the element it leans on. It's the budget counterpart to **Helion** (the
premium He³ tube): both are neutron heads on **Quark-Tubes** ([`../`](../)) and neither carries an
MCU — the tubes' pulses are counted on Quark-Tubes' H523. Both count the same particle, just with
different tricks.

## How it detects

Gadolinium is a **neutron magnet** — `¹⁵⁷Gd(n,γ)` has one of the largest capture appetites known
(~254 000 barns). A neutron ionises nothing by itself, so Gadolin lets the Gd swallow it; the capture
then spits out a **burst of gamma (~7,9 MeV)**, and *that* is what the **Geiger–Müller tubes** see
(family **(b)**, a capture gamma — `../../NEUTRONS.md`).

**Construction — a layered moderator + reflector, not a flat "Gd/plastic core".** The full detector
build (why the neutron's neutrality forces this, the tube choice, converter placement, the star core,
and the Rh variant) is in **[`CONSTRUCTION.md`](CONSTRUCTION.md)**. In short, outside → in:

```
graphite reflector           (encapsulated — conductive dust vs. the 400 V)
 → thin outer moderator       (poly, no Gd) — thermalise + feed neutrons inward
 → 13× SI-22G tubes           (1 central + 12 in a ring)
 → central moderator core     (~1 % Gd₂O₃ dispersed, multi-point star)
```

Moderation is a **forward-drifted** random walk (n–p scattering keeps neutrons heading inward as they
slow), so the **central 13th tube** catches what drifts to the middle; the star core spreads the
neutron-**energy** response and the Gd↔tube coupling. The **same skeleton** also runs the **rhodium
variant — "Rhodion"** (`¹⁰³Rh(n,γ)¹⁰⁴Rh` → hard 2,44 MeV beta) — see CONSTRUCTION.md §6: its 42 s activation delay is
a **non-issue** because the strike time comes from Tesla, so you separate **time** (Tesla) from
**count** (Rh) and integrate from T0.

- **Efficiency ~1 %** (the gamma is only weakly caught) — but the parts are **cheap, tough and
  gamma-tolerant**: no He³, no toxic BF₃, no photomultiplier. As always, the **network** is the
  instrument, not the single tube.
- **Variant:** an **activation foil** (`¹⁰³Rh(n,γ)¹⁰⁴Rh` → delayed hard beta; family **(c)**) trades
  speed for ~30× more captures per event. The same readout.
- **It avoids scarce He³.**

## In the network

- The count rides **Quark-Tubes' neutron channel K4** — the same channel Helion's tube fills on a
  build that carries it instead (`../BUS.md` owns the layout) — co-stamped with **Photon** (γ) and
  **Tesla** (lightning) for the TGF correlation.
- **The merge runs on `Quark-Tubes`, not here.** This board carries no MCU: the thirteen cathode
  lines cross the assembly to the H523's EXTI inputs, and the merge and the count are that
  board's (`../FIRMWARE.md` §A5).
- **Board:** the per-tube front end and the thirteen lines out, one board with no source on it —
  the ~400 V arrives on a bus from `Quark-Tubes`' module, the one source for every GM tube of the
  assembly ([`HARDWARE.md`](HARDWARE.md)).

## Readout — 13 tubes, no MCU, 13 wires out

**Thirteen `SI-22G` tubes, read at the cathode, each on its own wire to `Quark-Tubes`' EXTI.**
The board is not a node and carries no MCU; `Quark-Tubes` merges the thirteen in software by
comparing arrival times — every edge within a **16 µs window** of the first is the same particle,
so one event that lit several tubes counts once and two separate events count twice — and counts
the result into **K4** (`../FIRMWARE.md` §A5, the window a register).

For **Rh**, the readout is the same; the activation tail is read downstream, where Tesla's strike
times are (`CONSTRUCTION.md` §6).

**Front end, per tube:** the anode string from the 400 V bus, the cathode resistor the pulse
develops across, and a series resistor into the pin, whose own clamp clips the overshoot; no
capacitor on the pulse line ([`HARDWARE.md`](HARDWARE.md) §3).

## Files

| File | Contents |
|---|---|
| [`HARDWARE.md`](HARDWARE.md) | the readout electronics — the bus RC, the per-tube front end, the thirteen lines |
| [`CONSTRUCTION.md`](CONSTRUCTION.md) | detector construction — moderator, reflector, converter |
| [`WHY.md`](WHY.md) | the graveyard |

## Licence

Hardware: CERN-OHL-S v2 (`../../../LICENSE-HW`) · Software: MIT (`../../../LICENSE`) — Copyright (c) 2026 NIC — Native Intellect Community

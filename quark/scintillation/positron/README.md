★ N.I.C. ★

# Positron — the beta unit

> **Design-stage concept — nothing built.** The physics, the entrance window, the tile
> and the whole front end live in [`../photon/SCINTILLATION.md`](../photon/SCINTILLATION.md); the
> record in [`../photon/BUS.md`](../photon/BUS.md). This file is the unit's identity.

**Positron** detects **beta radiation, both signs** — β⁻ and β⁺ carry the same mass and the same
magnitude of charge, ionise alike, and one head reads both. The name is the β⁺ particle, and it
is not only a name: the TGF photonuclear chain (`¹⁴N(γ,n)¹³N`, β⁺) makes real positrons over
thunderstorms, on the same clock this station stamps.

**NOD, NodBus type 11 `Positron` — one build.** A **25 × 25 × 25 mm plastic scintillator block**
(`EJ-240`, the slow plastic — `HARDWARE.md`), PTFE-wrapped, with the SiPM straight on one face — no fibre — on the shared
**`Quark-Neutron/Positron`** board; one box carries it with the **`Neutron`** channel's screens and answers
as **ONE** NOD, this one. **It has no high-voltage part**: on a station that runs tubes the beta is
Quark-Tubes' channel, the bare K1 tube against the β-stopped K2, both 0,5–1 m over the plate — or the raw
counts (`../../tubes/HARDWARE.md`, `../../tubes/BUS.md`).

**Every frame it publishes the station's one scintillation record**, all of it accumulated since
the second began so that frame 127 carries the second and the difference of two frames carries one
frame: **count `uint16` · the sum of the events' energies `uint32` in keV · the largest single
event `uint16` · `Neutron`'s two counts, thermal and epithermal, `uint16` each · ten energy bands**; the flags are the frame
header's `status` byte and not payload. The mean is the sum over the count and the median is read out of the
bands, both downstream and exactly; the dose is that same sum × a constant. A register switches
the record to the list of particle energies, 16 a frame, for calibration (`../photon/BUS.md`).

**The block measures energy.** 25 mm of plastic is 2,56 g/cm², and the
hardest beta this unit is built for — **Y-90 at 2,28 MeV** — ranges 1,1 g/cm², so it stops inside
and deposits all of it; containment holds to about 5 MeV.

**There is no handover on this board and nothing to switch.** It carries one SiPM channel, the
photomultiplier's beside it, and no PIN at all. A fully contained Y-90 beta sits at **~2 % cell
occupancy** on `EJ-240` against the ~30 % where the response begins to bend, and **the muon lands at
~5,1 MeV** — a factor of two clear of the beta band, which makes it a ruler and a veto rather than a
contaminant.

**The neutron rides here because it has nothing else to send.** Two screens behind one
photomultiplier, a threshold and two height windows give two counts and no energy, so four bytes carry the whole
channel; a NUMBER of its own would have bought an address to hold 28 empty bytes. **NodBus type 10 is reserved for `Neutron`**, should a build want it on a NUMBER of its own ([`../neutron/`](../neutron/)).

**Why the unit exists at all: it is the one eye on the pure beta emitters.** `Sr-90`/`Y-90`
decays with **no gamma line whatsoever** — a gamma spectrometer of any quality sees nothing while
it is present — and `Kr-85` is the same case in a noble gas. Thirty-year half-life, a calcium
analogue that goes to bone. No other channel in the station can see this class of event.

The head looks down at the **1 m × 1 m plastic deposition plate**, window down, 0,5–1 m above it —
the plate is in every build, tube or scintillation — and the deployment section of
[`../photon/SCINTILLATION.md`](../photon/SCINTILLATION.md) owns the geometry, the window budget
(≤ 30 mg/cm²) and the isotope reach tables.

## Files

| File | Contents |
|---|---|
| [`HARDWARE.md`](HARDWARE.md) | the block and the SiPM, the entrance window, and the board they land on |
| [`../neutron/`](../neutron/) | `Neutron`, the photomultiplier channel on the same board |
| [`WHY.md`](WHY.md) | the graveyard |

## Licence

Hardware: CERN-OHL-S v2 (`../../../LICENSE-HW`) · Software: MIT (`../../../LICENSE`) — Copyright (c) 2026 NIC — Native Intellect Community

<p align="center">
  <img src="NIC-Photon.svg" width="200"/>
</p>

★ N.I.C. ★

# Photon — the gamma unit

> **Design-stage concept — nothing built.** The figures are verified against the parts' sheets and
> the parts before a board is made.

> The board and the H7A3 layer both scintillation boards share: [`HARDWARE.md`](HARDWARE.md); the rails' parts are the family's (`../../../galvani/HARDWARE.md`). **The GM-tube build of Photon is the HV variant**, counted on `Quark-Tubes`: [`../../tubes/HEADS.md`](../../tubes/HEADS.md).

**Photon** watches the **gamma rays and X-rays** — the energetic light thrown off by radioactive decay,
by cosmic rays, and by the gamma flashes that ride on big lightning storms (TGFs). Gamma and X-rays are
**photons** — one quantity, separated by origin and not by energy — and Photon is the unit that
measures it, **in two builds**:

- **Scintillation — NOD, NodBus type 9 `Photon`, one NUMBER**: CsI(Tl) cube + SiPM + PIN on the
  shared H7A3/`AD9251` board, counts *and* energy per event. The design is
  [`SCINTILLATION.md`](SCINTILLATION.md); the bus contract is
  [`BUS.md`](BUS.md).

  **Every frame it publishes one 32 B record**, all of it accumulated since the second began so
  that frame 127 carries the second and the difference of two frames carries one frame: **count
  `uint16` · the sum of the events' energies `uint32` in keV · the largest single event `uint16` ·
  twelve energy bands**. The mean is that sum over that count, the median is interpolated in the
  band crossing 50 %, and the dose is the same sum × a constant — all downstream and exact.
  Nothing that cannot be added leaves the unit, and the flags are the frame header's `status`
  byte, not payload.

  **The SiPM and the PIN are ONE measurement, switched per event by its energy** — the SiPM's
  below 500 keV, the PIN's above 700 keV, a linear blend between. Two instruments with overlapping
  windows give one reading and not two series: under its ceiling the SiPM is the better instrument
  and over it distorts, and the PIN is the reverse. What watches the pair is the **ratio** of the
  two areas on every event that crosses both thresholds, against a persisted constant — over 10 %
  out is `HEALTH` DEGRADED, so a channel going into saturation reports itself without costing an
  address.

- **Tubes — the HV variant**: GM tubes behind graded Pb, K1 · K2 · K3, counting on `Quark-Tubes`
  and riding its mini frame ([`../../tubes/HEADS.md`](../../tubes/HEADS.md)). Beside the scintillation
  build stand the neutron side (**`Neutron`**, a channel in Positron's record, type 10 reserved for it)
  and the beta unit (**Positron**).

## The converter, and the rail

`AD9251-80` — dual 14-bit, 80 Msps, `DRVDD` 1,8–3,3 V, SPI configuration, an internal clock divider
1–8 and a `SYNC` pin, so **it takes the encode clock the bus hands it** rather than making its own
([`HARDWARE.md`](HARDWARE.md) owns the part; Marconi's `AD9265-80` is its one-channel 16-bit
cousin). Two channels is what **one head** wants: the gamma crystal is read by a SiPM and a PIN
together and their ratio only means anything on the same event, so that pair owns a converter. **One
`AD9251-80`, two channels, per board** ([`HARDWARE.md`](HARDWARE.md)). The part is clocked at the
word rate, 88,08 MHz, and divides by two inside, because its interleaved output is double data rate
and the PSSI reads one edge; the same 88,08 MHz is the PSSI's read clock. *(The footprint and the
register map are the family's — `AD9648`, `AD9258`, `AD9268`, `AD9650` drop in; none is asked for.)*

**Everything rides 1,8 V, exactly as Marconi does it.** `DRVDD` will do 3,3 V, and there is no
reason to. Going to 3,3 V buys nothing and costs a second rail scheme. The station interface stays
3,3 V through the level shifters.

**The rate is `21 × 2²¹` = 44,040192 MSPS a channel**, 88,080384 MHz on the pins. Interleaving costs
half the encode rate, so the binary rung would be 2²⁵; this is the highest rate that keeps an
integer sample count **per second and per frame**, is made by an integer PLL, and leaves the pins
under the PSSI's 100 MHz.

**What it delivers, per channel:**

| channel | electrical pulse width | samples |
|---|---|---|
| CsI(Tl) gamma | 1 µs | **44,0** |
| plastic beta — `EJ-240`'s 285 ns decay on Positron's 603 ns charge stage | ~600 ns | **~26** |
| a single microcell's rise | 54 ns | **2,38** |

The first two are what the energy is read from. **The third is not measured**: the calibration
ruler, the one-cell step, is a cumulant statistic of the dark stream between events
(`SCINTILLATION.md`, *The calibration chain*) and asks nothing of the sample count; the next rung
up would leave less than 8 % of margin on the PSSI pin and nothing wants it.

**No high voltage is tracked on the scintillation side.** Nothing on the two boards exceeds ~30 V; the
one kilovolt on this side is the photomultiplier's, on Helion's kV source.

## Files

| File | Contents |
|---|---|
| [`HARDWARE.md`](HARDWARE.md) | the `Quark-Photon` board — and the H7A3 layer both scintillation boards share |
| [`FIRMWARE.md`](FIRMWARE.md) | the firmware, described — one image on this board and Positron's |
| [`SCINTILLATION.md`](SCINTILLATION.md) | the gamma head's physics and front end, the calibration chain, the reference tables, the bill of materials |
| [`BUS.md`](BUS.md) | the 32 B record, Photon's and Positron's |

## Licence

Hardware: CERN-OHL-S v2 (`../../../LICENSE-HW`) · Software: MIT (`../../../LICENSE`) — Copyright (c) 2026 NIC — Native Intellect Community

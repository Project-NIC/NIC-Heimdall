★ N.I.C. ★

# Quark-Tubes — the bus contract

> **Design-stage concept — nothing built.** The figures are verified against the parts' sheets and
> the parts before a board is made.

> What Quark-Tubes is and how it counts: [`../README.md`](../README.md). The bus itself — framing, the arm,
> addressing — is [`../../core/blocks/nodbus.md`](../../core/blocks/nodbus.md), which wins on any bus
> question.

**Address:** the computed one, `TYPE«4 | NUMBER` — **NodBus mini type 2**, so **0x21** for unit 1
(`../../core/PROTOCOL.md` §2, which owns the three type spaces and their packings).

**There are no Modbus registers here.** Quark-Tubes is a mini-NOD: what it publishes are **channels**,
read straight out of its records, and what it accepts are **commands** on the CONTROL frame. The
numbers below are channel numbers and nothing else.

**Quark-Tubes is a mini-NOD on Argus, not a ModBus module.** It needs the network clock, and a house
unit that needs the clock does not go on ModBus — it goes on **NodBus mini**: 8 B of payload on
the same framing, the same TDMA and the same clock as any node (`../../core/PROTOCOL.md` §2). **ModBus carries no clock anywhere** (`../WHY.md`).

## The bin — and why nothing has to mark it

Quark-Tubes counts on the **network clock** and closes a bin every frame — **1/128 s**, the same quantum
the payload uses. A bin is the four channels below and it rides its own frame.

**The slot is the time, so there is no marker and no age field.** A bin rides the frame it belongs
to. That is the whole reason this unit is on mini rather than on a polled bus: a polled reply has to
carry a marker because the poll instant says nothing. Here it arrives when it is due.

## The payload — four channels, two bytes each, and nothing else

**Quark-Tubes is a mini-NOD, so its payload IS its data.** No address, no channel, no age: the frame
header carries the address and the slot carries the time (`../../core/PROTOCOL.md` §2).

```
  raw:       0-1  K1       2-3  K2       4-5  K3       6-7  K4        — each tube's pulses
  derived:   0-1  beta     2-3  soft γ   4-5  hard γ   6-7  neutron   — worked on the unit
```

**All four are ACCUMULATORS reset on the second boundary**, the same rule the scintillation record
follows: frame 127 carries the whole second, the difference of two neighbouring frames carries one
frame, and a lost frame costs nothing because the next one still holds the running total. **No
extra frame is sent to close a second** — the second is already there. A GM tube's dead time caps
it near 1 000 CPS, so `uint16` never fills.

**Four detectors, four channels, and two ways to fill them — both kept in the concept.** The
assembly carries three GM tubes of one type at one height, 0,5–1 m over the plastic deposition
plate — **K1 bare, K2 behind a beta-stop, K3 behind Pb** — and **K4, the neutron detector**:
Gadolin/Rhodion's thirteen merged on EXTI, or a He³/BF₃ proportional tube. **Raw** sends each
detector's count per second and leaves the arithmetic to whoever reads it. **Derived** does it on
the unit: **beta = K1 − K2, soft γ = K2 − K3, hard γ = K3, neutron = K4**, the filtered tube
subtracted from the less-filtered one. Same frame, same four `uint16`; **which one a unit sends is the `MODE` register** — the default per
build, read with `GET` after the `DISCOVER` reply, written with `SET`, and a change is a config in the
archive from that frame, like any runtime setting (`../../core/PROTOCOL.md` §5). **The tube type is a register too**, so the
reader knows what a count is worth. **Counts only, no energy: a tube delivers none.** X-ray is not a separate channel — it is the soft end of the
photon spectrum and rides the soft γ band.

**Two bytes per channel keeps the format uniform with the scintillation units, and the ceiling
is not a limit anyone will meet** — a GM tube's dead time caps it near 1 000 CPS whatever the
field does. An unfitted tube sends zero. The supply and the board's temperature ride the `REPORT`
frame once a minute like on every unit, and the header's SUPPLY flag says when the supply leaves
its window (`../../core/PROTOCOL.md` §5).

**Quark-Tubes carries five thermometers — the board's and one on each head — and never sends them with the counts.** The temperature correction is applied **here**,
on the counts, before they are published — which makes it a **tier C value: used on the node,
never transmitted** (`../../core/PROTOCOL.md` §5). Sending it would only let the master redo work the
unit already did, against a reading taken at a different instant.

**And there is no status byte.** A fault shows as counts stopping, which the master sees without
being told; health worth reporting is the header's `FAULT` flag and `HEALTH` on `GET`, like every other node's.

The He³ head's second count — its LLD, gamma and neutron together — is a register and never a
channel: the tube's health, read with `GET` (`FIRMWARE.md` §A7).

**Commands** ride the CONTROL frame: the clock anchor at start-up, HV on/off, and the registers
behind `GET`/`SET` — the payload mode (raw or derived) and the fitted tube type.

## The firmware

The counters, the ring merge, the bin and the registers: [`FIRMWARE.md`](FIRMWARE.md).

★ N.I.C. ★

# Gadolin — the graveyard

Rejected alternatives and superseded states, with the reason. Nothing returns from here and
nothing is deleted from it.

## The local H523 and its one-wire output — superseded

An H523 here merged the 13 tubes' pulses into one train on one GPIO (3,3 V, push-pull or
open-drain, small series R) for the parent's neutron channel: a de-collision queue, not a counter.
This stands: **an OR gate merges simultaneous pulses and loses a count**; only a queue delays the
second — hence 13 lines.

It went because Quark-Tubes carries the assembly's MCU, so the lines reach it directly; **a second
H523 centimetres away bought a part, a crystal, a rail and a boot pin**. Every detector board is
MCU-less for this (`../../../NAMING.md`). The V1.0 sheet went with the pre-Galvani schematics.

## The original per-channel build — a source on every board, a Schmitt at every tube

Four PCBs, K1–K4, each with a ~400 V flyback (`LT8303` + `750311681` + `UF4007`, 100 nF / 500 V
film, feedback pair, snubber) and a `TPS7A2033` 3,3 V rail for a `74AHC1G14` Schmitt behind a
5,6 MΩ quench and a 47 kΩ / 100 kΩ divider, 33 Ω and 470 Ω out; thirteen tubes OR-ed through seven
`BAV99` pairs; the counting H523 on an HSE crystal, reading PA0–PA12.

**One 400 V module on `Quark-Tubes` feeds every GM tube** (`../HARDWARE.md`, *The one 400 V
source*); the cathode pulse enters a GPIO directly, so no rail or Schmitt; thirteen wires retired
the diodes; the counting board, crystal-less, reads PD0–PD12 with PC4 and PC9 (`../HARDWARE.md`).

## Glass GM tubes for the ring — refused

Lost on cost, ruggedness and outdoor mounting; **a glass wall cannot take plated rhodium**. The
thin stainless `SI-22G` wall passes hard beta as well: areal density decides (`CONSTRUCTION.md` §3).

## Silver as the activation target — the budget rival, not the default

Hundreds of times cheaper than rhodium, the classic activation material, but **three half-life
windows**: Ag-110, 24 s (hard β, 2,89 MeV, the useful part) · Ag-108, 2,4 min · Ag-110m, ~250 days.
Three exponentials resist a matched filter against Tesla's T0; the isomer builds a permanent foil
background. Rhodium is one window (42 s; Rh-104m at 4,4 min minor). Silver is the budget option
where that is acceptable.

## The concentric-ring core — the study's first geometry

Gd core, a ring of 10–12 `SI-22G`, Gd-free polyethylene, closed graphite reflector, ≥ 5 mm lead:
Ø ~150 × 290 mm with 10 tubes at ~0,8–1 %, Ø ~205 × 290 mm with 12 at ~1,5–2 %. Monte Carlo for
6 / 8 / 10 / 12 tubes: 46 / 61 / 77 / 92 % coverage, ~1,9 / 2,5 / 3,1 / 3,8 % pulse per capture, so
ε ≈ 0,45 × 0,036 ≈ 1,6 % at 12; three rings, 2–3× more. Background ~13,9 cps at 12, ~0,05 counts
in a 3,7 ms window, so 2–3 coincidences stand out; TGF neutrons want a ~50–100 ms window.
Superseded by the 13-tube star-moderator build (`CONSTRUCTION.md`).

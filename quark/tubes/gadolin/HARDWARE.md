★ N.I.C. ★

# Gadolin — the readout electronics

> **Design-stage concept — nothing built.** The figures are verified against the parts' sheets and
> the parts before a board is made.

The **Gadolin readout board**, block by block with its values: the ~400 V GM supply, the
**13× per-tube front-end**, and the **13 tube lines out to Quark-Tubes' EXTI** — no MCU on this board
(`README.md`). The **detector stack** itself (moderator / reflector / Gd-or-Rh
converter geometry) is in [`CONSTRUCTION.md`](CONSTRUCTION.md); the shared neutron physics is in
[`../../NEUTRONS.md`](../../NEUTRONS.md).

## 1. Board block diagram

```
                       GADOLIN READOUT (13 GM tubes → 13 wires out)
 ┌────────────────────────────────────────────────────────────────────────────────────────┐
 │  13× SI-22G tube ─┬─[Ra from HV]──● anode                                              │
 │  (1 central +     │               ● cathode ─[Rk to GND]─┬─[Rs]─► PDx (EXTI)           │
 │   12 ring)        │                                      └ (pin's own clamp)           │
 │                                                                                        │
 │  HV: the ~400 V bus from Quark-Tubes' module ──► 13× [R_iso 1 MΩ + 100 nF] ──► 13× Ra  │
 │      (no source on this board — one module feeds every GM tube of the assembly)        │
 │                                                                                        │
 │  NO MCU, NO RAIL, NO SOURCE: the 13 cathode lines leave the board on 13 wires and      │
 │  land on Quark-Tubes' EXTI pins, where the merge and the count run. Nothing on this    │
 │  board takes 12 V.                                                                     │
 └────────────────────────────────────────────────────────────────────────────────────────┘
   The tubes + Gd/Rh converter + moderator are the CONSTRUCTION.md assembly, wired to the board.
```

---

## 2. High voltage — the bus from `Quark-Tubes`, no source here

SI-22G runs on its GM plateau at **~400 V** — the same tube and the same plateau as Photon's three,
so **the ring takes the one 400 V module on `Quark-Tubes`** over a bus, and this board carries no
converter, no rectifier, no filter bank and no feedback divider (`../HARDWARE.md`, *The one
400 V source*). The load is nothing — thirteen tubes at their 1 000 CPS ceiling are 0,65 µA — and
the sag is the module's ~20 µF reservoir's: the ring's one-particle salvo of thirteen pulses,
0,65 nC, moves the bus under 0,1 mV.

| Part | Sets | Value | Note |
|---|---|---|---|
| **`R_iso`**, ×13 | one tube's discharge kept off the others | **1 MΩ** — 3 × 332 kΩ 0805 in series | on the bus side of each tube's string |
| **`C_iso`**, ×13 | the tube's own reservoir | **100 nF, 630 V polypropylene film** | to ground, between `R_iso` and `Ra` — never between `Ra` and the anode; a film part, because an HV ceramic loses its value under bias |
| **`Ra`**, ×13 | current limit + self-quench | tube-specified (SI-22G ≈ 10 MΩ) | at the anode, §3 |

**The plateau voltage is the module's setting, not this board's**: a different tube wants a
different plateau and a different `Ra`, and both are set where the module and the tube are
(`../HARDWARE.md`, `../HEADS.md`).

The GM pulse is a volt-level edge with no charge front to keep at the tube, so the detector, this
board and `Quark-Tubes` are one assembly: the bus arrives on two terminals, the thirteen pulse
wires leave, and nothing else crosses.

---

## 3. Per-tube front-end — ×13, no caps on the pulse pin

Cathode readout straight into a GPIO — the pin's own protection diode clips the overshoot:

```
   HV ──[ Ra ]── anode ●═════ SI-22G ═════● cathode ──[ Rk ]── GND
                                                  │
                                                  └──[ Rs ]──► PDx / PC4 / PC9 (EXTI)   (pin clamp clips)
```

| Part | Sets | Start value | Note |
|---|---|---|---|
| **Ra** (anode) | current limit + self-quench | **the tube's sheet** — `SI-22G` 10 MΩ, as **3 × 3,3 MΩ 0805 in series** | stands at the 400 V: one 0805 is rated 150 V, three in series self-balance to 450 V |
| **Rk** (cathode) | the pulse develops across it | **100 kΩ** | the discharge current is bounded by `Ra` — 40 µA at 400 V / 10 MΩ — so the edge is ~4 V, clipped by the pin's clamp to the rail; 47 kΩ would give ~2 V, under the H523's `V_IH` of 0,7 × 3,3 V |
| **Rs** (series to the pin) | the clamp's current | **100 kΩ** | a few microamps in service; a full 400 V arc onto the line is 4 mA, inside the H523's 5 mA of injection |
| (the pin's diodes) | the overshoot clip | `Quark-Tubes`' H523, at the receiving end | — |

**`Ra`, `Rk` and the plateau are the tube's.** Every GM tube has its own anode resistor and its own
plateau; the values here are `SI-22G`'s, and a build on another tube sets the three from that
tube's sheet — the plateau at the module, `Ra` and `Rk` here. **Every resistor that stands at
the 400 V is three 0805 parts in series** — `R_iso` as 3 × 332 kΩ, `Ra` as above — because one
0805 is rated 150 V working.

**No capacitor on the pulse line** — a capacitor with `Rs` rounds the edge and swallows counts.

---

## 4. MCU — none, and no low rail with it

**This board carries no MCU.** The 13 cathode lines leave the board on 13 wires and land on
**Quark-Tubes' EXTI**, where the merge and the count run (`README.md`). The board's
whole electrical content is 13× `R_iso`/`C_iso`/`Ra`/`Rk`/`Rs` and the wiring; the clamp that
catches the overshoot is the receiving pin's, at Quark-Tubes. **Nothing here wants 3,3 V**, so the
board carries no LDO and no logic rail.

## 5. Output — 13 wires, and the length is the constraint

The 13 lines run **across the assembly** to Quark-Tubes, not down a cable to a node: Quark-Tubes and this
board are one build (`../README.md`). Keep them short and away from the HV bus; `Rs` at this end is what limits the clamp current at the far one (§3).

---

## 6. Power tree

```
  the ~400 V bus from Quark-Tubes ──► 13× R_iso + C_iso ──► 13× Ra ──► the anodes
```

**No 12 V and no rail on this board** — the source is `Quark-Tubes`' module (§2). Keep the HV bus
and its return away from the 13 pulse traces.

---

## 7. No thermometer

**None — by the station-wide temperature policy** (`../../../NAMING.md`, *Temperature-sensor policy*): Geiger-plateau counting is
temperature-stable and the threshold-vs-background is drift-tolerant; classic dosimeters omit it,
correctly. The ring's channel on `Quark-Tubes` reads that board's own NTC — the block's ambient
(`../HARDWARE.md`, *The thermometers sit on the head boards*).

---

## 8. Bench criterion

The edge on the fitted tube clears the H523's `V_IH`, and the clamp current stays microamps in
service — on a scope, per tube type.

## 9. Bill of materials

| Ref | Part | Note |
|---|---|---|
| R_iso/C_iso | per-tube bus RC ×13 | `R_iso` 3 × 332 kΩ 0805, `C_iso` 100 nF 630 V film |
| Ra/Rk/Rs | per-tube front end ×13 | `Ra` 3 × 3,3 MΩ 0805, `Rk` 100 kΩ, `Rs` 100 kΩ |
| J* | 13× tube terminals, the 13 lines to Quark-Tubes, the 400 V bus in | — |

Off-board: the **detector assembly** (13× SI-22G + Gd/Rh converter + moderator + graphite —
CONSTRUCTION.md).

★ N.I.C. ★

# Neutron — the photomultiplier neutron channel

> **Design-stage concept — nothing built.** The head, the screens and the readout →
> [`HARDWARE.md`](HARDWARE.md); the board it lands on → [`../positron/HARDWARE.md`](../positron/HARDWARE.md); the
> graveyard → [`WHY.md`](WHY.md).

**`Neutron` counts neutrons on the low-voltage variant**: a stack of two `⁶LiF/ZnS(Ag)` screens with
a PMMA moderator between them, read by a photomultiplier of the 3 to 5 inch class, its anode on **channel 1 of
`Quark-Neutron/Positron`**. The height of each pulse says which screen fired, so it gives **two
counts — thermal and epithermal — and no energy**.

**It rides Positron's record**: the two counts are bytes 8–11 of Positron's 32 B record, two `uint16`
ahead of its ten bands (`../photon/BUS.md`), under Positron's NUMBER, because four bytes are the whole
channel. **NodBus type 10 is reserved for it**: a build that wants `Neutron` on a NUMBER of its own
takes type 10 and nothing else on the board changes.

**Its high voltage is Helion's kV source** (`../../tubes/helion/`), built for this head — three
ladder stages, tapped ~1200 V, clocked on `SYNC` from the board.

```
   screen · PMMA · screen ──▶ photomultiplier (kV source, ~1200 V) ──▶ THS4551 ──▶ Quark-Neutron/Positron, channel 1
                                                                                  ──▶ thermal · epithermal, in Positron's record
```

## Files

| File | Contents |
|---|---|
| [`HARDWARE.md`](HARDWARE.md) | the stack, the photomultiplier and its base, the shield, the enclosure, the screen's physics |
| [`WHY.md`](WHY.md) | the graveyard — the routes to the neutron's energy, the SiPM reading, the one-screen build |

## Licence

Hardware: CERN-OHL-S v2 (`../../../LICENSE-HW`) · Software: MIT (`../../../LICENSE`) — Copyright (c) 2026 NIC — Native Intellect Community

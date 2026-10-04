<p align="center">
  <img src="NIC-Tesla.svg" width="200"/>
</p>

★ N.I.C. ★

# Tesla — the lightning and sferic receiver

> **Design-stage concept — nothing built.** The figures are verified against the parts' sheets
> and the parts before a board is made.

**Tesla is the station's lightning receiver: NodBus type 7, a NOD.** Three ferrite rods, a
differential front end, a four-channel ΔΣ converter at 2²⁰ SPS and the STM32H7A3 that runs the
detection share one board in one sealed plastic box, the rods inside it. Named for Nikola Tesla.

**What it measures** — every impulse in 5 kHz – 512 kHz, and for each one:

- **the time**, by constant-fraction discrimination on the rising edge, to ±0,48 µs on the
  network's clock — what lets the network locate a stroke by time of arrival;
- **the signed amplitude**, logarithmic, in a field of 128 dB at 0,016 dB a step — room for the
  receiver's 98 dB, from a ~9 pT floor to a ~690 nT clip;
- **the class** — lightning, power-line arcing, or unknown — decided on the node, because only the
  node has the raw stream and the mains phase under it;
- where the supplement NOD is armed, **four more bytes**: a raw azimuth, the rise time and two
  between-band delays that range the source.

A persistent source — an arcing insulator, a tracking line — ships as a state, four records a
second, and the running noise floor ships once a second. The node thinks where only it can and
stays dumb where the network is better: polarity, stroke grouping and position are the server's.

**What one node sees.** It detects; it does not locate — position is TOA across stations. A stroke
inside ~3–4 km clips the rods: that blind circle is given away by design, because the network sees
what the nearest station cannot. An arc has no blind circle at any distance a station may stand,
and its reach is set by the site's QRN.

## How it hangs on the station

**Tesla is alone on its own Bifrost spur — point-to-point, full duplex, the unit end of the run.**
It carries no line part: a power board for its feed and a communication board for its medium plug
into its sockets, and the run — copper or glass, 48 V or 300 V — changes nothing on its own board.
The clock is Kronos's, on the spur's clock pair; the board locks its core, the converter's clock
and every sample to it. It takes **one NOD, two or three** — events, source states, supplement —
on the same spur.

**The board carries a second unit.** Flashed with its other image it is **Pip**, NodBus type 12 —
the longwave carriers and the station's longwave time, served on the board's second data body,
`TIME OUT`, which Tesla's image leaves idle (`pip/`). **And a third**: under its line-fault image
it is **Steinmetz**, NodBus type 13 — the mains-locked sources of a power line, from a vehicle or
a site, the front 20 dB down on the attenuation terminals (`steinmetz/`).

```
   3 ferrite rods ──▶ 3× S1/S2 THS4551 ──▶ ADS127L14, 2²⁰ SPS ──SAI──▶ ┌─────────────────────────┐ ── NodBus spur, type 7 ──▶ BIFROST
   REF6041 4,096 V ────────────────────────▶ REFP                      │ TESLA — STM32H7A3       │    (Pip's image: type 12,
   4 NTCs on the H7A3's ADC                                            │ filterbank · detector · │     and TIME OUT to Kronos)
   one IP68 plastic box; the Galvani sockets and the 12 V at its edge  │ CFD · classifier        │
                                                                       └─────────────────────────┘
```

## The tree

```
tesla/          Tesla — lightning and sferics; one board, three images            NodBus 7
├── design/     the rod worksheet, rod_calc.py; rod_field.py, its solve; chain.py, the clip and the floor
├── pip/        Pip — the longwave carriers: the SID channel, and time            NodBus 12
└── steinmetz/  Steinmetz — line faults on power lines, from a vehicle or a site  NodBus 13
```

## Files

| file | contents |
|---|---|
| [`HARDWARE.md`](HARDWARE.md) | the board — the chain with its values, the converter, the clock, the rails, the thermometers, the processor's pins, the sockets, the parts |
| [`DETECTION.md`](DETECTION.md) | timing by CFD, the filterbank, the strips, the three classes and the mains lock, a grid-only build, the TGF correlation |
| [`BUS.md`](BUS.md) | the backward offset, the 4 B event record, the supplement NOD, the rule for what goes on the wire, one to three NODs |
| [`FIRMWARE.md`](FIRMWARE.md) | the firmware, described — boot, states, the bus, time, the always-on path, emission, registers, faults |
| [`CONSTRUCTION.md`](CONSTRUCTION.md) | the frustum and the one plastic box, the rods and how to choose them, the winding, the rod thermometers |
| [`WHY.md`](WHY.md) | the graveyard — rejected alternatives and superseded states |
| [`pip/`](pip/) | Pip — the same board under its other image |
| [`steinmetz/`](steinmetz/) | Steinmetz — the same board under its line-fault image |
| [`design/`](design/) | the rod worksheet — a calculator for a rod you can buy: permeability, inductance, sensitivity and noise floor |

## Licence

Hardware: CERN-OHL-S v2 (`../LICENSE-HW`) · Software: MIT (`../LICENSE`) — Copyright (c) 2026 NIC — Native Intellect Community

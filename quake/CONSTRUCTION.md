★ N.I.C. ★

# Quake — construction

> **Design-stage concept.** The figures are verified against the parts' sheets and the parts
> before a node is made.

## The seat — a hole, coupled to the ground

**A Quake is seated in a hole, coupled to solid ground, never bolted to the mast.** A borehole is
the better seat and not a workaround: quieter, thermally stable a few metres down, and better
coupled — what professional networks do with posthole instruments.

- **Depth: ~20 m drilled, or a posthole of a few metres for most of the benefit.** At 20 m the
  groundwater pressure is ~2 bar, which a sealed tube takes with orders of margin; no pressure
  vessel.
- **Rock first; soil where the geology offers none.** Soft ground amplifies and carries surface
  noise, so the same instrument reads a higher floor on sediment. It is a preference and not a
  gate — a properly driven tube in soil is a working station.
- **Coupling is the pass/fail**: a tube that moves against the ground makes the record worthless.
  Hard rock is **drilled and the liner grouted in**; soft coral or sediment takes the liner
  **driven into an undersized hole** — the best coupling, and it wants tough HDPE, because
  fibreglass cracks under driving.
- **The liner is non-magnetic** — fibreglass, laminate, HDPE or PVC, never steel: steel distorts
  the magnetometer's DC field, shields it with eddies, and corrodes.
- **The node is a cartridge in the liner**, clamped or wedged, as professional borehole
  seismometers are: the liner stays coupled for good, and a fault is served by pulling the
  cartridge.
- **Seated within ~10° of vertical.** The levelling matrix takes the rest (`FIRMWARE.md` §7), and
  inside 10° the inclinometer runs its fine range. Vertical comes from gravity, azimuth from the
  magnetometer where it is fitted, or is left unknown where detection is the job.

## The body — the Barrel family

**A length of ~40 mm plastic pipe** (DN40, ID ~34 mm is the worked example; the exact pipe is the
builder's), **two boards inside, wired together**: the power and communication boards at the
cable end, and the sensor board — the H523, the MEMS, the thermometers — at the far end. The
cable enters through a gland; there is no connector on the node.

```
[gland] → [power and communication boards] → [H523 and its buck] → [ADXL355 · ICM · SCL3300 · RM3100 — the quiet end]
 the noisy end, ~0,2 W of heat                                        rigidly mounted, far from the switcher
```

**The magnetometer picks the length.** Near field falls as 1/r³, and length is the only lever on
the sources a measurement window cannot average away: the node's own 128 Hz current draw and its
transmit bursts pass such a window untouched, where a 140 kHz switcher is already ~53 dB down.

**One profile in steps of 25 cm, and the name is the multiple**: Barrel 25 cm · DoubleBarrel
50 cm · TripleBarrel 75 cm · QuadroBarrel 100 cm.

| body | length | sensor to switcher | fitted |
|---|---|---|---|
| DoubleBarrel | 50 cm | ~40 cm | Quake without the magnetometer |
| QuadroBarrel | 100 cm | ~90 cm | Quake with the RM3100 |

Even the short body clears the magnetometer's single-digit nT target, so **the length is picked
by the hole and the handling**. What length does not fix is **the feed run along the tube, the
nearest radiator the sensor has: it is twisted**, which drops its residual ~200× under the board's
own loop.

- **The noise gradient is the geometry.** Every source of heat and interference sits at the cable
  end; past the sensors flows only µA–mA of clean rail current, and the sensor half burns ~15 mW —
  no self-heating.
- **The tube does not absorb ground motion — it moves with the ground as one piece.** A seismic
  wave at 20 Hz is over 10 m long even in soft soil and hundreds of metres in rock; a metre of
  tube is a point on it. The plastic's own damping only calms the tube's resonances.
- **What length does to the tube is its bending modes, and the ground removes them.** DN40 PVC,
  40 × 3 mm, E ≈ 3 GPa, filled with gel and carrying its boards, ~1,6 kg/m, first mode:

  | length | free at both ends | held at one end, hanging | in contact along its length |
  |---|---|---|---|
  | 25 cm | 605 Hz | 95 Hz | ≥ ~400 Hz |
  | 50 cm | 151 Hz | 24 Hz | ≥ ~400 Hz |
  | 75 cm | 67 Hz | 10,5 Hz | ≥ ~400 Hz |
  | 100 cm | 38 Hz | 6 Hz | ≥ ~400 Hz |

  In contact, the tube is a beam on an elastic foundation and the ground's stiffness sets the
  mode, not the length: ~10 MPa of loose sand puts it at ~400 Hz, ~100 MPa of grout or compacted
  backfill at ~1,3 kHz. **So every body is in contact with the ground along its whole length** —
  grouted, driven, or in compacted backfill — **and only a Barrel may hang free**, its 95 Hz above
  the 64 Hz of the band carried; from 50 cm up a hanging tube rings inside the seismic band.
- **The seismic sensors sit at the deep end**, where the coupling is best and the cable with the
  surface's noise is furthest away.
- **The seismic sensors are hard-mounted to the tube**, screwed to an insert or bulkhead. A sensor
  floating in fill is a spring-mass system, and fill creep is slow tilt — on the part that is the
  drift reference.
- **The sensor half is a non-ferrous zone**: brass or nylon hardware only.
- **The fill is a meltable gel** — paraffin or gel-wax class, melting at ~55–70 °C — so a pot of
  hot water re-opens the node: the station's gel, not potting, rule (`../daedalus/CONSTRUCTION.md`).
  It fills around the sensors against moisture and voids; it holds nothing.
- **The axis mapping follows the orientation** — a vertical bore and a horizontal trench map the
  ADXL355 and SCL3300 axes differently, and the mapping is set at installation.

## Depth — air is free, water costs bars

**The MEMS parts are cavity packages** — the ADXL355's hermetic LCC, the SCL3300, the ICM, each
~1 atm inside. The fill passes hydrostatic pressure straight to them; the pipe wall protects
nothing, and a MEMS output shifts with package stress long before anything breaks.

- **In air** — a dry bore, above the water table: no depth limit from the node; the run's reach
  is the limit.
- **Submerged: ~30–50 m of water column (3–5 bar)**, the offsets re-zeroed after deployment.
- **Deeper is the oil-filled, pressure-balanced build, and it is not Quake's**: its MEMS are
  cavities. A pressure seismic front is a coil geophone or a hydrophone, or the MEMS kept in a
  1 atm housing.

**The wall thermometer is fitted by depth.** In a shallow burial of 1–2 m it is left off — soil
temperature there is Palatine's. In a borehole 50–100 m down it is fitted: the mK record of the
ground at depth is a channel no other unit has there.

## The cable

**UTP Cat 6, outdoor, UV-stable jacket, and the feed's own two-core cable, each on its own
gland** — or the hybrid of both in one jacket. Both land on the power and communication boards
inside the tube, and the node's board sees logic only. The spur is point-to-point; the 80,6 Ω
termination is a jumper on the communication board at each end, set at commissioning. There is no
inline connector outside the enclosure: outdoors, a hanging junction is the part that fails.

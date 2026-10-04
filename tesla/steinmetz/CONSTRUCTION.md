★ N.I.C. ★

# Steinmetz — the vehicle mount

> **Design-stage concept — nothing built.** The figures are verified against the parts' sheets
> and the parts before a board is made.

**At a static site Steinmetz is built as Tesla is** (`../CONSTRUCTION.md`): the frustum of three
rods in its one plastic box at ground level, the hybrid cable its only penetration. This document
is the vehicle.

## The mount

- **Tesla's box, unchanged, on the roof bars.** The frustum's base plate bolts to two cross bars
  with four M6 and a locking washer each; the pitched shape sheds rain and takes the wind at
  100 km/h as a few tens of newtons. The box is IP68 and plastic, as Tesla demands — a metal roof
  box would shield the band.
- **As high above the roof as the bars allow, 10–20 cm.** The steel roof is a conducting plane
  under the rods: at VLF its skin depth is a fraction of a millimetre, so it reflects the tangential
  field and cancels the normal one, and the three slanted rods see a field that is not the free
  one. The distortion is a fixed linear mixing like the rods' own coupling, and **the 3×3 crosstalk
  matrix and the azimuth table are measured with the unit mounted on the vehicle it will run on**;
  a remount or another vehicle is a new commissioning.
- **The cable** leaves the box through its gland and reaches the cabin through a roof
  pass-through where the vehicle has one, otherwise in a flat loop through the tailgate seal; it is
  tied to a bar every 30 cm so nothing flaps. Inside, the Proteus — the DC/DC brick is on its board — and the
  LTE router sit where the vehicle's fit-out and construction allow, a 1-DIN slot included.
- **A display head in the driver's reach, the size of a mobile radio**, on Proteus's Ethernet: a
  client of the live board showing what the unit sees — the sources in view with their class and
  their distance along the line, the floor, the GNSS fix and the link. The handset in a holder
  shows the same over BLE where no head is fitted.
- **The GNSS antenna** is glued to the windscreen inside the cabin, its coax to Polaris on the
  Proteus (`HARDWARE.md`).

## What a vehicle does to the measurement

- **Vibration.** The rods are clamped and glued as on Tesla and the board sits on its standoffs;
  **the two attenuation resistors are soldered into the clamp positions, not pressed** — a
  pressed resistor does not ride a vehicle.
- **The vehicle's own noise.** Alternator ripple, the DC/DC converters, LED drivers and motors are
  steady or slow and sit in the detector's rolling background; they raise the floor and are
  reported in it. A petrol engine's ignition is impulsive at a rate the engine sets — 100 Hz at
  3 000 rpm on four cylinders — and may lock for a moment; what rejects it is geometry, not
  spectrum: it sits at one azimuth with zero strip deltas however the engine runs, while every
  source on a line walks past, and the image drops such a slot after 10 s of motion
  (`FIRMWARE.md`, *The classes*).
- **Speed.** At 28 m/s a tower passes every ~10 s against the tracker's 0,3 s lock and 2 s drop;
  the position is the head's, once a second (`FIRMWARE.md`).

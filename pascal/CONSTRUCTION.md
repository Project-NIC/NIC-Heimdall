★ N.I.C. ★

# Pascal — construction

> **Design-stage concept — nothing built.** The figures are verified against the parts' sheets
> and the parts before a pod is made.

**The body is Gauss's sea build, whole** — the PVDF tube, the fused caps, the glued gland at the
bottom, the water-blocked cable, the oil or petroleum jelly filled under vacuum with no gas
anywhere and no bladder, the Galvani boards at the cable end (`../gauss/CONSTRUCTION.md`, *The sea
build*). This document carries what Pascal adds to it.

## The hole and the sensor body

**One threaded hole through the tube wall, and the gauge's housing screwed into it.** The gauge is
a TE Connectivity `MS5837-30BA` in a Blue Robotics `Bar30` housing — an M10 threaded penetrator,
sealed into the wall with its thread and O-ring; **the gauge's gel face is the face in the water**
and takes the pressure directly, so the fill never enters the measurement. Its four leads run
inside the tube to the board. The hole is drilled before the caps are fused and the housing fitted
before the fill; the fill then protects the electronics and nothing more. The gel face is a
service part in the sea — it is what ages, and the housing unscrews for it.

## The footing

**The pod is anchored, not laid.** A pod that rises 10 cm reports 10 mbar that never happened, and
the wave it waits for is about 20 mbar. A plain unreinforced concrete footing is enough: submerged
weight is ~57 % of dry, so a 50 kg block holds ~29 kg down, and the drag on it at 0,5 m/s is about
1,3 kg-force — it does not slide. **No reinforcement in the concrete where a Gauss stands beside
it**: rebar ends a magnetometer.

## The cable

**Clamped to the footing and slack beyond it.** A cable tensioned between shore and pod is a pump:
it lifts and drops the pod in the water column, in the same band as the wave.

## Where

**150–200 m of water on the island's flank, in a ring of 4, 8 or 12 with Gauss**
(`README.md`, `../gauss/ARRAY.md`). The pods on one segment share one feed, chained pod to pod,
and the run's voltage is chosen for their sum at the last one.

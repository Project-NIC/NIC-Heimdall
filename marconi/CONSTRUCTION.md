★ N.I.C. ★

# Marconi — construction

> **Design-stage concept.** The figures are verified against the parts' sheets and the parts
> before a unit is made.

## One loop, standing, and its two nulls are a siting condition

**One converter channel, so one antenna input, and the antenna is one vertical loop** — a figure of
eight standing, its two azimuth nulls broadside to the ring.

**The nulls are occupied once, when the mast goes up.** Every target is a fixed transmitter at a
published position and the station knows its own, so the bearing to each is a great-circle
calculation from two coordinate pairs — no azimuth is measured and none is discovered on site.

**The orientation is computed before the mast goes up, and it is one line of arithmetic.** The
loop's response is `|cos(azimuth − plane)|`: maximum for a wave arriving **in** the plane of the
ring, zero perpendicular to it. The bearings to all monitored transmitters are computed and the
plane turned to the angle that maximises the **worst** of them; a figure of eight is symmetric, so
only the bearings modulo 180° matter and the search is over half a turn. The number is computed by
`models/pointing.py` from the station's coordinates and the transmitters' published positions, and
the ring is bolted at that angle; nothing rotates in service.

**The residual is the price of one loop.** Worked for a site at 50,08° N 14,43° E, the best
orientation is **plane 3°/183°, nulls at 93°/273°**:

| | bearing | off the plane | response |
|---|---|---|---|
| WWVH Kauai | 354,3° | 8,5° | −0,1 dB |
| Buzzer 4625 | 35,2° | 32,4° | −1,5 dB |
| WWV Fort Collins | 316,9° | 45,9° | −3,2 dB |
| RWM Taldom | 55,3° | 52,5° | −4,3 dB |
| CHU Ottawa | 303,1° | 59,7° | −6,0 dB |
| **BPM Pucheng** | 62,6° | 59,8° | **−6,0 dB** |

Six bearings over 120° do not fit one figure of eight, so **the worst target keeps −6 dB and no
rotation improves it** — and it is 6 dB of signal-to-noise, because atmospheric noise arrives from
every azimuth and a figure of eight integrates to the same total whichever way it points. A site
whose transmitters cluster does better.

**No electric antenna, at any height or shape.** A short vertical is blind at the zenith, and a
transmitter near the station arrives there — the best measurement of all, because a near-vertical
path reads `foF2` directly instead of through the secant law. The station does not know where it
will stand, so it cannot carry an antenna that assumes the transmitter is far. The vertical loop is
flat in elevation from the horizon to the zenith.

## The loop — the requirement is fixed, the route is not

| | |
|---|---|
| a closed ring, **≈ 1 m across** | the diameter sets the area and therefore the sensitivity |
| conductor **≈ 10 mm** outside diameter | it enters `L` only logarithmically |
| if a tube, **wall ≥ 140 µm** | three skin depths at 2 MHz; every real tube clears it |
| **both ends at the feed point**, nothing else connected | the loop floats at the amplifier's common mode |
| **the plane's bearing fixed at build** | computed from the coordinates (above) |
| if corrugated, **annular, not helical** | a helical corrugation makes the surface current follow a helix and adds a solenoidal component |

**The material is free.** At 5 MHz, against the loop's 92,5 Ω of reactance:

| | `R` | share of \|Z\| |
|---|---|---|
| **thin-wall copper** | 58 mΩ | 0,06 % |
| stainless, smooth | 377 mΩ | 0,41 % |
| stainless, corrugated | 566 mΩ | 0,61 % |

The spread between the best and the worst is **0,0002 dB**, and the noise each adds (0,03–0,12
nV/√Hz) is invisible against the amplifier's. Wall thickness does not enter: at 2–12 MHz the
current lives in the top 19–47 µm. Taking the short-circuit current is what makes the metal
irrelevant.

## The enclosed ring

**A thick-walled PPR pipe bent into the ring, the conductor inside it.** The pipe is the structure,
the weatherproofing and the appearance at once:

- **it holds itself** — PPR 32 × 5,4 as a 1 m ring is 1,75 kg with the conductor, `EI` 35,3 N·m²,
  and deflects of order 10 mm at 20 °C. **It is supported at two or three points**, which also
  removes creep;
- **the feed exits through a welded tee** — a standard fitting, fusion-welded, so there is no
  outdoor joint; an outdoor joint going high-resistance over years is what kills a field antenna;
- **it looks like a piece of plastic** — nothing worth stealing and nothing that advertises.

**Three conductors, electrically identical — pick on buildability:**

| | |
|---|---|
| **thin-wall copper tube** | pushed in and bent with the pipe; the least metal, and the tube can carry the feed wires inside it |
| **stainless annular corrugated hose** | pulled through a ring bent first; the most robust and the easiest to thread |
| **copper braid or stranded** | the cheapest and easiest to pull; higher loss than solid and still far below stainless |

- **UV-stabilised black PPR, or overpainted in the station's white.** Plain white or green PPR
  chalks and embrittles outdoors in a few years; it is the cheapest line in the build and it
  decides ten years against three. The coat holds on PPR only over a polyolefin adhesion promoter
  (`../daedalus/CONSTRUCTION.md`, *Finish*), and a white ring runs cooler in the sun than a black
  one, which is creep not suffered.
- **Creep costs almost nothing electrically** — polypropylene at 60 °C in the sun loses half its
  modulus, but 10 % ovalisation is −0,14 dB and 20 % is −0,60, and a slow drift cancels in the
  day/night ratio because both frequencies of a pair pass the same antenna at the same instant.

**The plastic is not lightning protection** — a leader at megavolts a metre goes through PPR. What
it buys is no sharp metal point, no corrosion and no galvanic contact with the mount.

## The enclosure, the island and the strike

**The electronics sit in a plastic box at the ring's feed**, the amplifier centimetres from the
loop; the spur's cables enter it on glands and land on the Galvani boards.

**The island floats, and the mount must let it.** The surge a stroke induces is common mode on the
whole island, which floats and so is not stressed (`HARDWARE.md`, *Surge*). **Bond the enclosure to
a mast that is bonded to anything else and the common mode becomes a difference**: the ring and the
box are fixed to the mast through insulating clips and nothing metal of the unit touches it.

**Marconi is this station's sacrificial branch.** Tesla is one sealed box with its rods inside;
Marconi is a metre-wide ring on a roof or a mast, so of the units on a station it is the one most
exposed to a direct strike and the most likely to be lost. The station accepts that shape
(`../daedalus/CONSTRUCTION.md`): a remote branch's boards are the sacrificial part, and branch
spacing does not stop a strike, it stops one strike taking all four branches.

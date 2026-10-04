★ N.I.C. ★

# Sputnik — where the computation lives

> **Design-stage concept.** The figures are verified against the parts' sheets and the parts
> before a board is made.

The station ships one GNSS stream (`BUS.md`); the science — slant TEC, vertical TEC, the pierce
points, precipitable water vapour, the tomography — is computed from it somewhere. **Where is a
deployment's choice, and every choice ends at the same observable**, so stations that choose
differently still merge into one product.

## What is measured

**Total electron content along every satellite-to-receiver path.** The ionosphere delays a signal
in proportion to 1/f², so the same satellite on two frequencies gives the TEC of that path with no
reference station. With every constellation tracked — ~35 satellites typically visible, 40–60 at
the most, up to ~64 — one station produces a dense set of ionospheric pierce points overhead, and a
grid of stations a live regional TEC map. It serves space weather, HF propagation and the detection
of the transients that shake the upper atmosphere: a large earthquake's acoustic-gravity waves reach
the ionosphere 8–12 minutes after the ground motion, so Quake and Sputnik on one site see both ends
of that chain.

**Precipitable water vapour, from the same stream.** The troposphere delays every band equally, so
it is not pulled out by differencing frequencies; it comes from the total delay, the geometry and
the surface met Palatine measures on the same site. Its wet part is proportional to the water
vapour along the path: ZTD with surface met → the hydrostatic delay (Saastamoinen) → the wet delay →
**PWV** (Bevis). It is established GNSS meteorology — ground-GNSS PWV is assimilated into
operational forecasts (E-GVAP) — and many satellites at many angles through the same air are the
input of water-vapour tomography: moisture advection, fronts, pre-convective build-up, as relative
changes co-timed with the rest of the grid. Palatine's met does double duty: it corrects the
troposphere for the TEC and anchors the wet-delay retrieval.

## The invariant — the card always holds raw

**Every station archives Tier B raw to its card**, whatever else it does, because raw is the one
irreplaceable measurement and can be reprocessed later with better models. Any window of it can be
pulled on demand. What the choice below moves is only where the science is computed and what leaves
over the uplink.

## Two axes of accuracy, and neither is the station's processor

- **Spatial resolution** — the 3-D field over an area — comes from **more stations**, more rays
  crossing at more angles. It is a network product.
- **Absolute accuracy** — sub-TECU TEC, millimetre PWV — comes from **reprocessing the raw** with
  precise products centrally.

**Distributing the work loses nothing, because the corrections are additive.** The geometry-free
combination cancels everything common to both frequencies — range, satellite and receiver clocks,
orbit, troposphere:

```
P1 − P2 = (ionosphere ∝ TEC) + DCB + noise          TEC_absolute = TEC_geometry-free − DCB
```

So precise orbits and clocks are not needed for TEC at all; **the one central product that makes it
absolute is the network's differential code bias**, one per satellite and one per receiver, and it
is subtracted afterwards wherever the geometry-free value was formed. **The condition is a lossless
record** — the value to ~0,01 TECU with its PRN, its geometry and its exact time; the station keeps
all three. A real-time reduction on the station is marginally rougher than a post-process over the
whole satellite arc, where levelling and slip detection see the complete pass — and because the card
holds the raw, anyone can close that gap later.

## The spectrum — four points, one contract

| | where the science is computed | uplink | server | best for |
|---|---|---|---|---|
| **A — central** | all on the server | raw — the widest | the largest | the science archive; the best absolute accuracy |
| **B — edge-assisted** | a district box near a backhaul reduces several stations' raw to observables; the server inverts | medium | small | clusters sharing one uplink |
| **C — on the station** | **the Mayak** computes slant TEC per ray and PWV from its own GNSS and met; the server inverts | observables and met — thin | laptop-scale | **the default**: autonomous, low bandwidth, local detection in real time |
| **D — dedicated** | the station ships only the minimal reduced observables | the thinnest | the smallest | the most bandwidth-starved deployments |

**Every point ends at the same observable**: a **slant TEC per ray** — PRN, pierce-point geometry,
value — and a **PWV / ZTD** per station, each with the shared time and the station's surveyed
position. So one country reducing on the station and another shipping raw feed one tomographic
inversion, regionally inverted results stitch with their boundary rays, and a station's PWV drops
into a national water-vapour field beside any other. **The observable is the contract, not the
method.**

## Two rules that follow

1. **The combined product lives on the Mayak.** Slant TEC needs only GNSS, but PWV needs GNSS and
   surface met together, and only the Mayak holds both — Sputnik's stream and Palatine's met on one
   clock. Method C and D run there.
2. **Met travels with the observable on the uplink**, where the observable is shipped separately
   (A, B, or a C/D station that ships raw for reprocessing): ~once a minute —

   | field | bytes | |
   |---|--:|---|
   | surface pressure | 2 | 0,1 hPa / LSB — drives the hydrostatic delay |
   | surface temperature | 2 | 0,01 K / LSB — drives the mean temperature |
   | relative humidity | 1 | 0,5 % / LSB — quality control; PWV comes from ZTD |

   The station's latitude and height are static, sent once at enrolment; ~5 B a minute beside the
   GNSS stream. On the station nothing extra rides the NodBus — the Mayak already has the met.

## Stage 1 — the light reduction (the station or a district box)

| product | method | reference |
|---|---|---|
| slant TEC | geometry-free code and phase, carrier-to-code levelling; the widest band pair, the third band for DCB and slip checks | Ciraolo et al. 2007 |
| VTEC and the pierce point | single-layer mapping, the thin shell | Klobuchar 1987 |
| PWV | ZTD + surface met → ZHD → ZWD → PWV; ZTD taken as an input — a by-product of the navigation solution | Saastamoinen 1972; Bevis 1992/1994 |

**No internet and no external products**: a few pages of arithmetic on libm, which runs on the
Mayak, a small ARM box, a phone or a browser. It stops at the DCB-biased observable.

## Stage 2 — the absolute products (the server)

Established software and fetched products, never rewritten and never on a field node:

- **precise orbits and clocks** — IGS `SP3` and `CLK`, or decoded from the augmentation streams; for
  position and ZTD, not TEC;
- **the code bias** — CODE/AIUB and IGS DCB/OSB in bias-SINEX, or co-estimated in the inversion;
- **the augmentation pages Tier C captures and the node does not decode** — Galileo E6-HAS (the
  Galileo HAS SIS ICD) and BeiDou PPP-B2b (the BDS SIS ICD); QZSS L6 MADOCA (IS-QZSS-MDC, the open
  MADOCALIB) and SBAS (RTCA DO-229, RTKLIB) are fetched from their services where wanted, since
  this receiver logs neither (`BUS.md`, *Tier C*);
- **the workhorses** — RTKLIB for RINEX and PPP, gLAB for the algorithms step by step, georinex for
  I/O;
- **maps and tomography** — IONEX for VTEC grids; 3-D electron density from crossing rays with
  research codes of the MIDAS class — the product one station cannot make;
- **mapping functions** — VMF3/GMF and the GPT3 empirical P/T where surface met is missing.

**The formats that make the pieces interoperate**: RINEX (Tier B converts to it), IONEX, SP3/CLK,
bias-SINEX, ANTEX.

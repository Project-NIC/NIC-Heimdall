★ N.I.C. ★

# INTEROP — feeding the world's existing systems

> **Design-stage concept — nothing built.** The figures are verified against the parts' sheets and
> the parts before a board is made.

**The claim, up front: NIC does not compete with the global scientific networks — it feeds
them.** Most of the world's geophysical infrastructure is not a closed club; it is a set of
**aggregators that live on incoming data** (SuperMAG, Madrigal, MADIS, the IOC sea-level
facility, OpenAQ…), plus a few strict observatory clubs (INTERMAGNET, IGS core) whose job —
absolute reference — we deliberately leave to them. The path is proven: **Raspberry Shake**
got an FDSN network code (`AM`), its ~2k community stations flow into EarthScope, and the
ISC cites their picks. NIC walks the same path with ten quantities instead of one.

**Why it is cheap for us:** the architecture already assumes it. The HMC archive is raw and
self-describing (derive-from-raw, `archive/HMC.md`), so **an export format is just another reader** — no
change to the station, the bus, or the protocol. How each one is written is
[`archive/EXPORTERS.md`](archive/EXPORTERS.md). And the trade goes both ways: we give the
aggregators the density they don't have (see `../gaia/` — India, Turkey, Indonesia…), and
they give us **credibility and calibration for free** (the variometer-vs-observatory
regression in `../gauss/CONSTRUCTION.md` is exactly this, and FDSN locations validate our picks).

## The map — system by system

| domain | system | takes us? | format / channel | from the station |
|---|---|---|---|---|
| **seismology** | FDSN / EarthScope, SeisComP, ObsPy | **yes** — Raspberry Shake precedent | **miniSEED** + StationXML | the miniSEED exporter on the HMC reader (Steim-2 + StationXML); the network code is a registration (the gate below) |
| | ISC (bulletin) | yes — takes picks/data from registered networks | via FDSN | paperwork only |
| **geomagnetics** | INTERMAGNET | **no** — needs an *absolute observatory*: weekly DI-flux measurements by a human on a fixed pier (see below) | — | out of scope by design |
| | **SuperMAG** | **yes** — aggregates ~600 *variometers*, strips baselines itself | **IAGA-2002**, `Data Type: variation` | the IAGA-2002 exporter on the HMC reader (calibrated nT out of the archive) |
| **GNSS: TEC** | Madrigal (MIT Haystack) | **yes** — takes any dual-frequency RINEX | **RINEX** | the RINEX exporter on the HMC reader, from Sputnik's Tier B |
| **GNSS: geodesy** | Nevada Geodetic Lab, EarthScope | **yes** — NGL processes any submitted RINEX (20k+ stations) → PPP positions, crustal motion | RINEX upload | the same RINEX export — pairs beautifully with Quake |
| | CORS / IGS densification | **yes** — the UM980 is survey/geodetic class | RINEX + site info | needs a decent antenna |
| | IGS core | per-station — needs a **calibrated antenna (ANTEX) + monument + site log**; the receiver is NOT the blocker | RINEX | achievable where a station earns it |
| **tsunami / sea level** | IOC Sea Level Station Monitoring Facility (VLIZ) | **yes** — aggregates ~1000 radar gauges worldwide | station registration + HTTP push | the IOC exporter on the server; Pascal is a bottom-pressure gauge, a class the facility also takes |
| | magnetometer tsunami array | research-grade → science collaborations, not operations | IAGA-2002 / raw | doctrine in `../gauss/ARRAY.md` |
| **weather** | CWOP → NOAA MADIS | **yes** — citizen stations feed forecast assimilation | CWOP protocol (trivial) | the CWOP exporter on the server |
| | WIS2 / GBON — a national service's surface network | per national service | **BUFR** 307096 hourly, 307092 ten-minute | the server encodes a mode-B Palatine's `HOUR` and `TEN` rows (`../palatine/WMO.md`) |
| **air quality** | sensor.community, OpenAQ | yes, takes anything | HTTP push | the air-quality exporter on the server, from Chinook's units |
| **lightning** | Blitzortung | it IS a community network | TOA receiver protocol | Tesla could join as a receiver, by agreement on their protocol |
| **cosmic ray** | NMDB | case-by-case | — | science collaboration |

## Where calibration lives — the one rule to keep straight

Two exporters, two deliberate conventions, one raw archive:

- **miniSEED carries raw counts**; calibration belongs in **StationXML** metadata (that is
  seismology's own split — data vs. response). Both are written by the miniSEED exporter.
- **IAGA-2002 carries physical nT**; the exporter applies the scale of the field's layout in the HMC
  header (`physical = (raw + add) · mantissa · 10^exp10`, mantissa `0 ≡ 1`) on the way out.

The archive itself never stores corrected values (derive-from-raw, `archive/HMC.md`); an export
is a *view*, re-derivable forever with a better model.

### No field calibration — precision by selection, zero by reference

**The station stores no calibration curves and no instrument-response tables; the one
reference it ever establishes is zero.** The sensors are chosen so a field or laboratory
calibration buys nothing:

- **A factory-calibrated digital sensor is its own reference.** Calibrating it in the field needs
  a reference better than the sensor, which a station does not carry. Where a known local offset
  appears and the unit exposes a writable offset register, the offset is written over the bus in
  place; where the element itself has degraded, the unit is replaced.
- **The rain gauge is precise by construction, not by a fitted curve**: a ratiometric 24-bit
  converter and a cell worked at a few percent of its span. What it measures is the change from a
  zero re-established at every drain; acceptance is a go/no-go check with known weights. A
  damaged cell is replaced — its error is path-dependent, and no static table corrects it.
- **The seismic MEMS are flat across the band**: one factory sensitivity, no poles or zeros to fit.

**So the StationXML response is flat — `InstrumentSensitivity` only** — which is correct for this
sensor set and is what the miniSEED exporter emits. A sensor that one day needs a response is
given a write-once calibration block per unit, carried to the head as one out-of-band packet
and into the HMC header's `RESPONSE` section, from which StationXML takes poles and zeros.

## Why INTERMAGNET is a "no" — and why that costs nothing

An INTERMAGNET observatory guarantees **absolute** field values for decades: a
**non-magnetic pier** in its own hut (arcsecond-stable orientation, an azimuth reference
mark), a **DI-flux** (fluxgate on a non-magnetic theodolite) that a **trained human** works
weekly to measure true declination/inclination, a proton/Overhauser magnetometer for
absolute intensity, and the resulting **baseline** that anchors the continuous record to
single-nT/year stability. That is a building, an instrument worth thousands, and a decade
of weekly ritual — the exact opposite of an unattended potted sonde.

And it does not matter: everything the network hunts — storms, pulsations, SSC, tsunami,
dB/dt, GIC — is a **variation**, and the absolute baseline cancels in every variation
product. The world has run on *a handful of absolute observatories + a sea of variometers*
for 150 years. **We are the sea; SuperMAG is where the sea reports.** (The variometer
doctrine ①–④: `../gauss/CONSTRUCTION.md`, *Calibration without rotating the site*.)

## Registrations — and the gate on them

The registrations and channels (FDSN network code · SuperMAG station registration · IOC SLSMF
registration · CWOP ID + push · NGL / Madrigal upload jobs) are paperwork + the server's exporters, no
station change — they live on the server, never on the node.

**The gate: no registrations until the hardware is validated.** A network is registered when it
produces real data. The exporters exist so the door is ready; it opens the day a real station has
run and its output has been checked. **The station never changes for interop** — the archive is
raw, and the exporters are readers.

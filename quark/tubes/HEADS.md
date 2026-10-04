★ N.I.C. ★

# The heads — what stands on `Quark-Tubes`

> **Design-stage concept — nothing built.** The figures are verified against the parts' sheets and
> the parts before a head is made. The board is [`HARDWARE.md`](HARDWARE.md); the Gadolin/Rhodion ring and Helion's He³ / BF₃ tube,
> which have builds of their own, are [`gadolin/`](gadolin/) and [`helion/`](helion/). **A head carries no MCU**: tube, HV where it is the head's,
> pulse shaping, its thermometer, and a wire to the board.

## Photon's GM head — three tubes, K1 · K2 · K3

### What it detects

Several **Geiger–Müller tubes, each hidden behind a different thickness of lead and plastic**. Since
each thickness stops a different slice of the spectrum, a handful of plain tubes together act as a
crude **energy sorter** — hard gamma + cosmic, mid gamma, soft X-ray. Those three bands are derived from the tubes
**K1 / K2 / K3** (K1 bare, K2 β-stopped, K3 behind Pb — below). It is a **citizen-science relative monitor**, not a calibrated dosimeter: the real
instrument is the **dense, clock-synced grid**, which lines a radiation burst up with the lightning
strike that caused it, right across the network.

### The spectrum — what lightning makes, what actually arrives, and the tubes

**Lightning's photon spectrum is hard, not soft.** The **stepped leader** emits **X-rays in µs bursts**
at **~30–250 keV** (relativistic runaway electrons → bremsstrahlung); a **TGF** goes far higher, **gamma
to the MeV range**. There is **nothing usefully soft** (< ~30 keV) to catch: even if the leader made it,
**air is the soft filter** — soft X-rays are absorbed within cm–m of atmosphere, so from a channel
hundreds of metres away **only the > ~30–50 keV component survives the air path**. The atmosphere pre-filters
the soft end for you.

**So steel-wall tubes are exactly right.** `SI-22G` / `SBM-20` / `SBM-19` all have a **steel wall** — ~40 mg/cm² on
the SBM-20, ~200 mg/cm² on the SI-22G (`gadolin/CONSTRUCTION.md` §3) —
that blocks soft beta (< ~0,3 MeV on the thin wall, < ~0,6 MeV on the SI-22G) and soft X-ray (< ~30–40 keV) — which is fine, because that soft stuff
isn't in the arriving lightning spectrum anyway. The "K2 / RTG" channel means **hard X-ray ~30–100 keV
(lightning leaders)**, not < 10 keV soft-X. The **bare K1 tube** catches the ground's **β + γ + X-ray total**
(deposition / "how hot is it here") as well: at 0,5–1 m over the plastic deposition plate the beta still
reaches it through the thin steel wall, and the beta channel is K1 minus the β-stopped K2
(`BUS.md`).

Three tubes of **one type per build** — any of the three, never mixed — so a band is a difference
between two filters and nothing else; the calibration is per type, and the type is a register on
`Quark-Tubes` (`BUS.md`). `K4` is the
neutron channel and never a GM tube (`BUS.md`).

**Shielding is climate-driven — and Pb is only the reference unit (concept).** These thicknesses are a
**recommendation for a worst-case hail site**, not a fixed spec: a desert with no hail can run nearly bare,
hail country wants the full cover — the builder picks for the climate and the worst case that can happen
there. The recommended **~2–3 mm GRP shell does double duty** — hail armour **and** filter: its low-Z mass
**stops the beta and passes the X-ray**, so with the laminate **K2 needs no lead** (Pb is K3's, where it sets the
hard gamma band). Each shield is quoted as an **equivalent ideal-Pb
thickness**; build it in Pb or anything else, but a different material means **converting to its
Pb-equivalent and re-calibrating the tube** (the shield sets what reaches the gas — the paradox being that
the laminate you need for hail is itself a fine radiation filter). Thicknesses are **starting points a
technician adapts to the site**.

**Extreme weather is the builder's own hardening** — a tornado or hurricane site re-designs the head
for its own conditions; this is the temperate, hail baseline.

**Mechanical — laminate armour outside, lead inside (the Pb is a spectral element, not armour).** A bare
GM tube on a mast dies in hail and thin Pb protects nothing, so the build is **concentric**: a hard
**pressed-fibreglass (GRP) tube, ~2–3 mm**, is the hail-and-wind shell → the **Pb filter sits inside it,
protected** → the GM tube is innermost. GRP is **low-Z → gamma-transparent**, so the shell is purely
mechanical and never touches the spectrum. An **optional graphite-cloth core** in the laminate adds
stiffness and a **Faraday / RF shield** (worth it — these sit in storms; a near-strike EMP couples
capacitively into the HV / signal wiring). If used it must be **grounded, insulated from the tube's 400 V,
and encapsulated** (no loose conductive dust vs. the HV — the same rule as Gadolin's graphite reflector),
and kept off bare metal fittings (carbon is galvanically aggressive). Note the **grounded Pb layer is itself
a Faraday shield**, so the graphite mainly earns its place on the **open channels** (K1 and K2 — no lead to ground)
— fit it per-channel, not everywhere. Two hygiene items: a **breathing
membrane / desiccant** (sealed GRP → condensation on the HV) and optional **Pb end-caps** if axial
(tube-end) leakage matters to the banding. The **Pb channel, K3,** stays lead-first — there the lead is
**both** armour and filter.

### The high-voltage part — three tubes, counting on `Quark-Tubes`

**Tube, HV and pulse shaping, and nothing else.** The shaped pulses go to `Quark-Tubes`'s H523 and
land straight on hardware timer inputs, so counting them costs no CPU at all.

**The pulse is read at the cathode, and the HV never lands on the shaper board.** The anode is
wired from the ~400 V module through the anode resistor; the cathode goes to ground through
`Rk` on the shaper board, and the pulse across it, a volt or so, is what the shaper sees
(`gadolin/HARDWARE.md` §3). **The anode string is the family's**: on the bus side an `R_iso` of ~1 MΩ and 100 nF to
ground, then **`Ra` per the tube's sheet at the anode itself**, on two pads at the tube — everything
between `Ra` and the anode is capacitance that dumps into the discharge, ~50 pF a metre of wire,
which is charge of the pulse's own order, and the RC before it is what keeps this tube's discharge
off the other tubes on the one 400 V bus (`HARDWARE.md`, *The one 400 V source*). **The shaper
board carries the channel's thermometer**: an `NCP18XH103F03RB` NTC, 0603, reflow-soldered, two
wires on the head cable to the divider on `Quark-Tubes` (`HARDWARE.md`, *The thermometers
sit on the head boards*). The tube, the board and the block are one thermal mass; nothing is
glued to a tube. Everything on the board is SMD and goes on in one reflow; the tube's leads and
the cable are the only hand joints.

| band | filter | tube | why |
|---|---|---|---|
| **K1** β + X + γ, everything | **none, bare** — the steel wall alone | **one of `SI-22G` · `SBM-20` · `SBM-19`** | the beta reaches it through the thin stainless wall at 0,5–1 m over the plate |
| **K2** γ + hard X-ray | **none** — the GRP shell is the beta-stop | the same | GRP passes the X-ray that Pb would kill |
| **K3** hard γ + cosmic | **Pb** — stops the beta and the soft γ; thickness climate-driven (*Shielding*, above) | the same | catch the little that gets through |

**One tube type on every position of a build** — `SI-22G`, `SBM-20` or `SBM-19`, the three sensitive stainless-wall tubes, never mixed within a build. One plateau, one anode string, one shaper board; a difference between two tubes is then a difference between two filters and nothing else, and **the calibration is per type** — the fitted type is a register on `Quark-Tubes` (`BUS.md`).

**Three tubes and no more, all at the same height — 0,5 m to at most 1 m over the 1 m × 1 m plastic deposition plate**, which every build carries. `Quark-Tubes` publishes the three tubes and the neutron detector **either raw — each tube's pulses per second, K1 · K2 · K3 · K4 — or derived on the unit: beta = K1 − K2, soft γ = K2 − K3, hard γ = K3, neutron = K4** (`BUS.md`, both modes). **`K4` is the neutron channel** — Gadolin/Rhodion's thirteen or a He³/BF₃ tube — never a GM band.

### Commissioning — the background, measured

**The head's own background is measured before anything is read against it**, and differential
absorption turns plain tubes into a crude spectrometer. Three states, each long, one straight after
another because radon moves by tens of per cent through a day:

```
1. thin plastic   → everything (the cover already stops the soft beta)
2. ~1 mm lead     → the soft X-rays stopped
3. a few mm lead  → hard gamma and cosmics left

soft X-ray ≈ (1) − (2)     middle band ≈ (2) − (3)     hard + cosmic ≈ (3)
```

~1 cps a tube wants ~10⁴ counts for 1 %, so the states run for hours, and a difference counts only
where it is well above √(sum). A thin copper layer goes under the lead, whose own fluorescence at
~75–88 keV otherwise lands in the band. The intervals between pulses are logged too: clean
background is Poisson, exponential spacing, and a burst stands out against it.

**The alarm is relative, never a dose**: a threshold as a multiple of the running background, in
levels (5×, 20×, 100×), confirmed by persistence over seconds to tens of seconds against cosmic
showers and interference. No absolute calibration is needed for it; it runs downstream, not on the
board.

### In the network

- **The tube build is not a node.** Its counts ride **Quark-Tubes' derived channels** (soft γ ·
  hard γ after normalisation and subtraction) — Quark-Tubes is a mini-NOD behind Argus and
  `BUS.md` owns the layout. In this build Photon is the detector head; Quark-Tubes is the
  unit on the bus. The scintillation build is its own NOD, Photon, type 9 (`../scintillation/photon/`).
- **The counts are fast and co-stamped** — two bytes per channel, an accumulator reset on the second, four derived
  channels in the 8 B mini payload (`BUS.md`). A TGF burst then shows up as a
  spike stamped to the same instant as a **Tesla** strike and the **Quark-Tubes** neutron count.
  - **Caveat — GM dead time during the burst.** A GM tube of this class has a dead time in the ~200 µs class, and a TGF
    delivers its photons in a **sub-millisecond** burst, so the tube **paralyses after the first few
    counts** — a K-channel registers a TGF as *"a few counts in one frame,"* i.e. a **detection**
    (co-timestamped, which is all this design claims), **not** a flux measurement. The one channel that
    survives a TGF intact is **He³** (µs recovery, gamma-blind) — see Helion. So read the K-channel spike
    as "something happened, now," not as "this many gammas."
- **Siting and leads.** The head lives on its **own ~1 m non-conductive post beside the
  station, never the mast** (`../../daedalus/CONSTRUCTION.md` — the siting roster and the flash-over
  arithmetic); the pulse and supply leads run **buried** to the enclosure. The direct wires are the design, not
  a shortcut: the K counts must co-stamp at 1/128 s, which polling cannot give.
- **Board:** the **shaper board, one per tube** — the anode string's `R_iso`, `C_iso` and `Ra`,
  the cathode's `Rk` and `Rs`, the shaper and the NTC; **no source on it**, the 400 V being the
  one module on `Quark-Tubes` (`HARDWARE.md`, *The one 400 V source*). This document is the
  record to draw from.

★ N.I.C. ★

# Neutron — the graveyard

Rejected alternatives and superseded states, with the reason. Nothing returns from here and
nothing is deleted from it. The board's graveyard is `../positron/WHY.md`.

## Routes to the neutron's ENERGY — described, and none taken

The build is a `⁶LiF/ZnS(Ag)` screen behind a moderator: counts, no energy, deliberately. The
routes were weighed when `Quark-Neutron/Positron` had two SiPM channels and no PIN; none touches
`Quark-Photon`, whose SiPM + PIN pair answers a TGF.

**Why energy is hard.** `⁶Li`, `¹⁰B` and He³ capture thermal neutrons, eight orders below a fission
neutron, so behind a moderator every neutron reads 0,025 eV. The moderator also smears the time by
tens to hundreds of µs of wandering, the die-away the frame rate constrains
(`../../tubes/README.md`) — worse, for a neutron pinned to a stroke on ±1 µs, than the lost energy.

| | in front of the two sensors | gives | costs |
|---|---|---|---|
| **A — the build** | block + screen/moderator | β · thermal n, counts | no n energy, no sharp time |
| **B** | bare block + metal-covered block | β by subtraction · fast n, energy and arrival time | all thermal n; a ~1 MeV floor |
| **C** | B with `EJ-276` class plastic | B, cleaner, by pulse shape too | the bench question below |
| **D** | B plus a third moderated head | everything | a third channel, a redrawn board |
| **E** | A or B, no hardware | energy by time of flight | located strokes only |

**B.** ~1,5 mm steel or 4 mm aluminium stops every beta in the band (Y-90's 2,28 MeV ranges
1,1 g/cm²) and nearly passes fast neutrons: bare minus covered is beta, covered is fast neutrons
with their recoil spectrum, the tube head's `K1 − K2` trick (`../photon/HARDWARE.md`). The cover
also takes ~8 % of 662 keV gammas and far more below 100 keV, so the subtraction needs a calibrated
correction, and gamma noise in both blocks does not subtract.

**C.** A recoil proton excites a stronger slow component than an electron, read as late-window over
total charge. It fails below ~1 MeV, where quenched proton light is ~0,1 MeV electron-equivalent;
`EJ-276` gives ~14 % less light than `EJ-200`. The bench question: the `AD9251` samples every 30 ns,
the slow component lasts 100–300 ns — comfortable on a liquid, marginal on a plastic, beyond
calculation. Flammability does not bar liquid: `EJ-301`/`NE-213` flashes near 26 °C but `EJ-309`
near 144 °C with nearly as good PSD, in sealed cells with the sensor potted in.

**D.** Loses nothing; needs a third SiPM channel on a two-channel board — the whole objection.

**E.** Over a kilometre a photon takes 3,3 µs, a 1 MeV neutron 72 µs, a 10 MeV one 23 µs: the band
spreads over ~50 µs against a ≤ 1,5 µs pulse pair, and the spread is the spectrum, started at
Tesla's stroke time over the stroke's distance. Located strokes only, never the cosmic background;
the distance error enters the energy twice; `¹⁴N(γ,n)` along the gamma path smears the source. A
spectrum over many events, not one particle's energy.

**Why A stayed.** Each route gains one direction and loses another, and **none can be chosen
without bench data that does not exist**. A is complete, costed and buildable, and the board was
the same for A, B and C, so A closed none of them; only D, until the board is redrawn for a third
channel.

**Unexplored**: an avalanche photodiode for the gamma head's PIN. Gain near 100 would drop that
channel's ~165 keV threshold, set by the PIN's ~500 e⁻ noise floor (`../photon/SCINTILLATION.md`),
to single keV, with continuous gain and no cell exhaustion. Its gain moves ~3 % per °C and wants
the stabilised bias the station already gives its SiPMs. A stone left unturned, not a proposal.

## The acrylic body between the screen and a SiPM — withdrawn with the SiPM

A SiPM on the screen needed a light-collecting body: `ZnS(Ag)` is opaque to its own light, so the
screen is thin and wide and the sensor small against it. Total internal reflection traps the light
in bonded PMMA (critical angle 42,1°, 48,4 % guided), and after a few reflections the body is an
integrating box:

```
η  =  A_window / ( A_window + A_wall·(1−R) + A_screen·α )
```

Position does not appear, so sensors sum as one; `A_screen·α` dominates, so window area buys light
almost linearly and a lens nothing (bound `A_window·n²/A_screen`). Screen bonded, reflector dry —
a bonded reflector kills the internal reflection.

**Why it went.** The budget stood on `α`, the screen's per-pass transmission of its own light,
which nobody could derive; **from 10 % to 30 % it moves the collected fraction two orders**. A
photocathode the size of the screen needs no body (`HARDWARE.md`).

## One screen, the fast branch closed — superseded by the two-screen stack

A single `⁶LiF/ZnS(Ag)` screen on the window: a second behind a moderator adds only the epithermal
decade, which the lightning population does not carry. Replaced by the stack — thermal screen, PMMA
moderator, thin gadolinium-free `⁶Li` screen at the window — for capture above ~15 % and two counts
naming the neutron's population (`HARDWARE.md`).

## The shape match on the anode — superseded by the area windows

A transimpedance readout with a ~200 ns pole keeping the photoelectron train, and a discriminator
correlating the pulse in ten bins against two bench templates, `Q_tail/Q_total` as cross-check.
With two screens ~10× apart in light and a gamma an order under either, **the area alone separates
the three**.

## A transparent second screen — sought, and there is none to buy

Wanted: thin, flexible, `⁶Li`-loaded without gadolinium, clear at the outer screen's 450 nm.
Transparent `⁶Li` plastics (lithium methacrylate and salicylate loadings, the LLNL films) are
laboratory materials at 0,4–3 % `⁶Li` by weight — a 0,5 mm film captures ~2 % thermal, nothing
epithermal. `EJ-270` is 0,5 % `⁶Li` in rigid PVT cylinders and cubes. The flexible siloxane `⁶LiF`
composites (Padova) are `ZnS` again, not for sale. Clear and lithium-rich is glass — `GS20`,
`Eu:LiCAF` — and rigid. The second screen is a thin `⁶LiF/ZnS(Ag)` on clear polyester, the glass
disc the flat-window alternative (`HARDWARE.md`).

## The charge stage's τ and the area sort — superseded twice

**τ 2–3 µs**, longer than the `ZnS(Ag)` tail so a capture was one step, its height the charge. The
area is `Q · Rf` at any τ; a long τ only kept the stage off baseline and raised pile-up at a near
stroke. Then **τ 0,5–1 µs with a ~3 µs integration window**, sorting by area into gamma, epithermal
and thermal, pile-up divided by its rises — but at a near stroke a capture every few µs lands in the
previous window and one area becomes several. Now τ ≈ 100 ns, each rise's peak above the prior
level (`HARDWARE.md`, *The readout*).

## The 76 mm class as the build — superseded

The 3″ class was written as the build, the 5″ dropped for mass. The class is 3 to 5 inches; past
5″ the faceplate's curve rules a tube out.

## A bought ¹⁰B/ZnS(Ag) counter in place of the head — weighed, not taken

Bridgeport's `PMT-N2000` family — `ZnS(Ag)` + ¹⁰B screens on PMMA guides, photomultiplier, supply,
LED gain stabilisation, HDPE moderator, a TTL pulse per neutron, 3 µs dead time; `PMT-N2K-9x2`
5 400 € ex works, 3 850–4 200 € at a hundred. The head is ~1 000–1 500 € in parts on existing
boards, its design time spent. **The bought part costs four times that and measures less** — one
count, not thermal and epithermal; moderated, not bare; about half ⁶Li's capture per area — and
saves no design. The boron screen stays a count on `Quark-Tubes`, where a TTL output belongs.

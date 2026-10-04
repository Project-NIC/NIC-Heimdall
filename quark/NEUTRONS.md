★ N.I.C. ★

# Neutrons — the physics every neutron head stands on

> **Design-stage concept — nothing built.** The figures are verified against the parts' sheets and
> the parts before a head is built. The heads are Helion (`tubes/helion/`), Gadolin/Rhodion
> (`tubes/gadolin/`) and `Neutron` (`scintillation/neutron/`); this file is what they share.

**The neutron heads are a relative monitor, not dosimetry.** A tube head's detection efficiency is
of the order of 1 % on the Gd route, which is the edge of detectability for a weak event; the value
is the dense, cheap, clock-synced grid, whatever a single TGF yields.

## Detection — a neutron must be converted first

A neutron carries no charge and crosses a gas without ionising it. A nuclear reaction on a target
turns it into a charged particle or a photon first, and **what that reaction yields sorts every
method into three families**:

| family | what the capture gives | examples | what reads it |
|---|---|---|---|
| **(a) a prompt charged product** | a charged particle that ionises at once | ³He(n,p)³H — 0,76 MeV · ¹⁰B(n,α)⁷Li · ⁶Li(n,α)t | ~100 % of captures, **if the converter is the active volume** — the gas, the screen. Helion, `Neutron` |
| **(b) a capture gamma** | a gamma cascade, to be converted again | ¹⁵⁷Gd(n,γ), ~7,9 MeV | a GM tube at ~0,5–1 % per gamma, a scintillator tens of %. Gadolin |
| **(c) activation** | a radioactive nucleus that decays later | ¹⁰³Rh(n,γ)¹⁰⁴Rh — β 2,4 MeV, 42 s | the delayed hard beta. Rhodion |

(b) and (c) run on cheap stainless-wall GM tubes; (a) wants scarce He³, toxic BF₃, or a screen and a
photomultiplier.

## Capture fraction is not detection efficiency

The Gd chain:

```
fast neutron → moderation → thermal → capture on Gd → 7,9 MeV gamma → GM → pulse
                ~40–50 %               ~98 % on Gd     ~0,5–1 %
```

**Capture fraction** (~40–50 %) is how many neutrons are captured on the Gd, bounded by the
moderation. **Detection efficiency** (~1 %) is how many are counted, bounded by the GM tube's ~1 %
on gamma. Geometry lifts the second to ~1–2 % and no further; family (a) goes round the limit — a
He³ tube reaches ~70–96 %.

## The short-range rule — where the converter sits

Charged particles stop in millimetres or less; photons do not.

- **A penetrating gamma** (Gd, 7,9 MeV — ~38 cm in polyethylene): the converter may sit centrally in
  the moderator, and the gamma reaches the tubes around it.
- **A charged product** (beta, conversion electrons, alpha, triton, proton): the converter sits **on
  the tube wall**, or the moderator swallows the product.

Ranges in polyethylene: a 100 keV conversion electron ~0,15 mm · a 1 MeV beta ~4 mm · a 2 MeV beta
~10 mm · alpha and protons µm. **A stainless tube wall passes only a hard beta above ~0,7 MeV**, set
by its areal density — so the Gd route stands on the gamma alone, and no converter for a charged
product can be laid on the outside of a tube, metal or glass. Rhodium plated on the conductive
stainless wall works because its product is exactly that hard beta.

## Two timings, and nothing between them

The capture's energy leaves as a prompt gamma or as the beta of a daughter's decay, and **a beta
decay always has a half-life** — there is no prompt hard beta.

- **Gd, the prompt gamma**: read at ~1 %, but coincident within a sub-millisecond window.
- **Rh, the delayed hard beta**: read ~30× more efficiently per capture, but spread over its
  half-life, with no prompt coincidence.

**The delay does not cost the lightning correlation**: the time `T0` comes from Tesla, the count from
the foil. The activity is ∝ e^(−t/τ) from `T0`, so a decay-weighted integration from `T0` — a matched
filter — collects the signal where it is dense and ignores the late tail. The price is a spread into
minutes of background, the √ penalty, largely paid back by the ~100 % per-capture beta and by the
epithermal response (Rh's 1,26 eV resonance). **Gd is the best signal-to-noise per event; Rh the
integrated flux and the broader spectrum.**

```
production   R = N_foil · σ · φ_th                 saturation   A_sat = R
build-up     A(t)  = A_sat · (1 − e^(−λt))         decay        A(t') = A_end · e^(−λt'),  λ = ln2 / T½
counts       ∫ A(t') · ε_β dt',   ε_β ~0,3–0,5 on a GM tube
```

A 25 µm rhodium foil is ~1,8 × 10²⁰ cm⁻² and captures ~2,6 % of thermal neutrons per pass — more over
the many passes the cavity gives; the exact figure is a transport calculation.

## The materials, by role

| role | what | why |
|---|---|---|
| **moderator** | hydrogen only — polyethylene, water, wax | the targets capture thermal neutrons |
| **reflector** | graphite | scatters and barely absorbs; slows nothing much. Beryllium reflects slightly better and is never machined here |
| **absorber** | boron, cadmium | belongs in a shield, never inside |
| **gamma shield** | lead | stops gamma, passes neutrons |

## Cross-sections, thermal

| nuclide / reaction | σ | product | note |
|---|---|---|---|
| Gd (natural) | ~49 000 b | gamma + electron | the product is the gamma |
| ³He(n,p) | 5 330 b | proton | direct, gamma-blind |
| ¹⁰B(n,α) | 3 840 b | alpha | BF₃ or a B-10 layer |
| ⁶Li(n,α) | 940 b | alpha + triton | screens |
| ¹⁰³Rh(n,γ) | ~145 b | beta, 42 s | activation |
| ²³⁸U(n,γ) | ~2,7 b | beta | own activity, regulated — not used |
| ¹⁴N(n,p) | 1,8 b | proton | range below the wall |
| H(n,γ) | 0,33 b | — | the competing capture in the moderator |

**Polyethylene with 1 % Gd₂O₃**: Σ_Gd ≈ 1,5 cm⁻¹ against Σ_H ≈ 0,027 cm⁻¹, so ~98 % of thermal
captures land on the Gd, with a mean path to capture of ~6 mm. The same loading as a stearate in wax
wants ~3–4 % stearate by mass, the stearate carrying less Gd per gram than the oxide.

## Activation targets

| target → product | β⁻ max | T½ | σ | note |
|---|---|---|---|---|
| ¹⁰⁹Ag → ¹¹⁰Ag | ~2,9 MeV | 24 s | tens of b | the cheapest |
| **¹⁰³Rh → ¹⁰⁴Rh** | ~2,4 MeV | 42 s | ~145 b | plated from a jeweller's bath — **the default** |
| ⁵¹V → ⁵²V | ~2,5 MeV | 3,7 min | ~5 b | too slow for one strike |
| ⁵⁵Mn → ⁵⁶Mn | ~2,85 MeV | 2,6 h | ~13 b | too slow for one strike |

Gd is never in an activation head — it would take the neutrons before the foil.

## Tubes by fill

**The prefix does not say the fill** — the SI and SNM series each carry several kinds; the stated
fill, efficiency and size do. **The voltage says the mode, not the fill**: ~400–500 V is a GM tube
(gamma/beta, not neutron); ~1000–1800 V proportional, He³ and boron alike; ~1750–2400 V corona.

| tubes | converter | mode | efficiency | note |
|---|---|---|---|---|
| SNM-9…14, SNM-42 | B-10 cathode coating, inert gas | corona | low, single % | cheap, robust, gamma-tolerant |
| SNM-3, 5, 8, 20, SNMO-5 | BF₃ gas | proportional | medium | toxic gas |
| SI-19N, SI-14N, SNM-16/18/56, SNK-18 | He³ gas | corona or proportional | high, ~70 % | gamma-blind, dearer |

A coated tube converts only in its thin layer, hence the low efficiency; in a He³ tube the gas is
the converter throughout.

**He³ tubes** — efficiency rises with pressure and diameter, the count with length:

| tube | He³ | Ø × length | efficiency | mode |
|---|---|---|---|---|
| Reuter-Stokes `RS-P4-0810-227` | 10 bar | 50 × 250 mm | ~96 % | proportional, ~1100–1600 V — to order |
| SNM-16 | 9 bar | 18 × 135 mm | high, small volume | proportional ~1600 V or corona |
| SNK-18/130-5,0 | 5 bar | 18 × 130 mm | high | proportional |
| SI-19N | high | 32 × 218 mm | ~70 % | corona, ~2400 V |
| **SNM-18** | ~4 bar | 32 × 305 mm | high, large volume | **proportional, ~1600 V — the one to buy** |

For a weak, diffuse flux the absolute capture and simple electronics decide, so a large volume in
proportional mode wins; even the best single tube has little area against a diffuse field.

**Supply.** He³ comes from tritium decay (12,3 years) and demand exceeds it: a bare surplus tube
~$200–400, a complete set $1200–2000 and up, a new Western tube to order with its sheet. A surplus
tube is checked for its real fill (He³, boron, or gamma only), its plateau, the marking on the body,
and returnability; new old stock beats a tube pulled from service, which may have leaked or burnt.
Makers with sheets: LND (He³, BF₃, boron, GM) · Mirion/Canberra (He³) · Centronic/Exosens (He³,
B-10) · VacuTec (He³, GM) · Reuter-Stokes/Baker Hughes (He³, RS-P4) · Global Nucleonics (He³, BF₃)
· Proportional Technologies (boron straws, the He³ substitute).

**He³ is gamma-blind**, which makes it the neutron channel and useless for the gamma background;
every route needs a moderator for fast neutrons.

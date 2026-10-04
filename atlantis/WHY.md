★ N.I.C. ★

# Atlantis — the graveyard

Rejected alternatives and superseded states, with the reason. Nothing returns from here and
nothing is deleted from it.

## The electrical worksheets of `HARDWARE.md` — superseded by the Galvani 300 V pair

Tier 2's electrical half, worked before the 300 V boards joined the family:

- **100 V, then 200 V, then 300 V.** A laser-only estimate gave 0,5 W and 100 V; the modules'
  datasheet current is 2,6× that. 200 V on 1,5 mm² held a collapse margin of 3,0× at the
  typical 1,32 W, 2,0× at the maximum 1,98 W. 300 V is the one nominal; the cross-section absorbs
  the rest. All are ES3 under IEC 62368-1 (ES1 ≤ 60 V, ES2 ≤ 120 V): the class never differed.
- **The head on `LT8304-1` + `750315839`** — the −1's 950 ns blanking against a 1:10 secondary's
  ringing, a 26,7 µH floor, a comb table against the withdrawn 70 kHz criterion
  (`../galvani/WHY.md`), Sumida's three-secondary `0399-T208` for headroom. Now `G-300-S`;
  `750315839` fails the sampling rule at 1:10 (`../galvani/HARDWARE.md`).
- **Würth `749197221` at k=1** — one of six windings as primary, five in series, 1:5 into 200 V,
  246 kHz at 2,5 W, 500 V AC basic insulation; a wound-to-order Sumida EE1618 as the reinforced
  alternative.
- **The far end on `LT8316` + Sumida `15364-T008`** (1500 µH, 20:1:2,4), `11338-T195` in reserve.
  Now `G-300-U-6`, 12 V out.
- **The pod bulk table at 40 B frames** on 2¹⁸, 2²⁰, 2²¹. The pod is a mini-NOD on 16 B frames;
  its rule — send often and small, size for the average — stands in the README.
- **`UCC28911` and `UCC28881`** as cost-class far-end challengers, a shelf note.

## "One new board and two re-populations", the "O series" — superseded

The README read "one new board — the island converter, which needs a three-winding part — plus two
re-populations", both optics open, and named the optical boards an "O series". **The
300 V pair is catalogue** and the shore optics is `OPT2-55A03STR`; only the pod's
pressure-tolerant module is not bought.

## Dead ends of the long link

- **Reversing a step-down transformer.** A flyback is a gapped inductor, L ∝ N²: reversing a 20:1
  part divides `L_PRI` by 400, putting every high-voltage-input catalogue winding at 0,7–2 MHz from
  12 V, several times the controller's ceiling.
- **N transformers, primaries in parallel, secondaries in series.** Buys volts; watts were short.
- **Borrowing the project's step-up designs.** Gadolin and Photon make 400 V and Helion 2000 V on
  catalogue magnetics, but for Geiger tubes drawing microamps: voltage-stressed, not
  current-stressed — `750311681` is rated 0,8 W.
- **Thicker copper instead of voltage.** Over 100 km, 2,5 → 6 mm² is 6,3 t of copper against
  perhaps 50 $ of higher-rated parts.

## A ~50 km reach default and a ~3 600 km network — superseded

The cable and depth budget was read at a ~50 km reach default, with the arithmetic worked at 100 km
for margin: 1–2 km of water, 100–200 bar, ≈ 25–50 % of the transport signal, 41 shelf sites dead at
any sane cable, and ~3 600 km of cable for the whole build, a figure the per-site bathymetry never
supported. **The 1550 nm module reaches 100 km**, so the reach is the module's: the median site sits
at ~2,9 km, ≈ 80 % of the signal, and 13 sites stay on the shelf. Tier 2 is a selection, so the
cable is budgeted per site: at most 500 km, 800 km at a two-arc ring.

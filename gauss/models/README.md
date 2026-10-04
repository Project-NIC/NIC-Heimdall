★ N.I.C. ★

# gauss/models — the shallow-water siting model

**English** · [Čeština](README.cs.md) · [Русский](README.ru.md)

The computation behind the **depth siting rule** in [`../ARRAY.md`](../ARRAY.md)
(*Where along the slope*): a linear 2-D shallow-water sweep past a
circular island (shelf → slope → deep plain, plane tsunami incoming), tracking
**max |H·u|** — transport = velocity × depth, the motional-induction (magnetometer) proxy.

- [`slope_toe_sweep.py`](slope_toe_sweep.py) — the model (pure numpy + matplotlib;
  staggered C-grid, forward–backward, sponge edges). Run it: `python3 slope_toe_sweep.py`.
- [`slope_toe_sweep.png`](slope_toe_sweep.png) — the committed output. Left: the 2-D peak
  transport field (note the flank refraction brightening and the dead lee). Right: the
  radial curve — signal ~0,30× on the shelf, climbing the slope, **~1,00× at the toe,
  flat plateau beyond** (extra cable toward the trench buys ~0 %).

**Honest scope:** linear, frictionless, schematic bathymetry — it fixes the *shape*
(signal tracks depth; the toe is the optimum; the plateau is flat) and the toe optimum,
not the second decimal. Nonlinear run-up and bottom friction are deliberately out.

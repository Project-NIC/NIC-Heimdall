★ N.I.C. ★

# Schematics — waiting for the first builder

**This folder is empty on purpose.** NIC-Heimdall is a concept, and a concept this size is designed
in its documents: every board's `HARDWARE.md` carries the parts, the values, the pins and the
arithmetic behind them. That is a schematic in words, and it is complete enough to draw from.

The drawn schematic belongs to the next step — from concept to build — and that step is open. If
you draw a board, it goes here:

- **one folder per board**, named exactly as the board is named in its project's `HARDWARE.md`
  (`bifrost/`, `G-I-N-025/`, `Quark-Photon/` …);
- the EDA sources, and a PDF anyone can open without the tool;
- a short `README.md` saying **which state of the description you drew from** (the commit), what
  you changed and why — a change to a value belongs back in the `HARDWARE.md`, so the description
  and the drawing never part ways.

Found a mistake while drawing? That is the most useful thing a first drawing can do: fix the
description too.

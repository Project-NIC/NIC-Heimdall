★ N.I.C. ★

# Helion — the graveyard

Rejected alternatives and superseded states, with the reason. Nothing returns from here and
nothing is deleted from it.

## The V1.0 sheets — retired

Two EasyEDA sheets, a detector board and a kV source, retired with the other pre-Galvani
schematics. What went: a 5 V rail on two `TPS7A4701` carried up the cable — the head runs on the
cable's 3,3 V; a `TLV3512` comparator, `74LVC1G17` stretchers, `BAS416` clamps and `SMF3.3A`
transils on two counting outputs — the discriminator is on `Quark-Tubes`; `Rf` 10 MΩ with a 0,5 pF
trim as the whole `Cf` — `Rf` follows the tube's leakage (below); a 13× 47 MΩ divider — the string
is 625 MΩ / 500 kΩ.

## `Rf` at 4,7–10 MΩ with `C47` 1 nF — superseded by 22–47 MΩ and 4,7 nF

The window stood on 100 nA of tube leakage, setting the 10 MΩ ceiling, and a collection of
"microseconds", setting the 4,7 MΩ floor. **The sheets give 4–8 µs** (electrons from along the
proton–triton track, the ions inducing the rest), so τ 4,7–10 µs would have peaked at 55–75 % of
`Q/Cf`; and **a proportional tube's insulation is above 10¹² Ω at bias — a nanoampere, not a
hundred**. `Rf` moved to 22–47 MΩ (τ 22–47 µs, peak 84–96 %, ceiling 23–50 nA of leakage), `C47`
to 4,7 nF so the high-pass loses under a tenth of the longer tail. The same figures closed the
`TLV9061` slew check — 0,13–0,26 V/µs asked of 6,5.

## The kV source as a fixed ×4 ladder — superseded by the 400 V stage populated to the ceiling

A ×4 Cockcroft-Walton on a 325–400 V base, 1300–1600 V for a standard He³ fill, the winding
re-tapped to 2 × 250 V for 2000 V; the `FBX` string at 1250 MΩ / 1 MΩ; a gas discharge tube across
the output as an option; the HV source on its own board as a recommendation. **Re-tapping put the
extra voltage on the winding, the one part not to be stressed.** With the stage at 400 V, the stage
count (2, 3 or 4 for 1200, 1600 or 2000 V) sets the ceiling and the `FBX` ratio the voltage under
it: one board and one BOM cover every fill and the photomultiplier, the winding at 400 V whatever
the output. The 1250 MΩ string doubled the `FBX` offset and bleed time to save 1,6 µA. The options
became decisions: the clamp fitted, the source its own board (`HARDWARE.md` §2).

★ N.I.C. ★

# Sputnik — the graveyard

Rejected alternatives and superseded states, with the reason. Nothing returns from here and
nothing is deleted from it.

## Tier A — the receiver's ionospheric delay on the wire — removed

6 B per satellite from `SATELLITESTATUSB` on COM1, a `TIERS` bit. **A broadcast Klobuchar or
NeQuick model at ~50–70 % RMS error, not a measurement**; the server computes the real delay from
Tier B, and nothing needs it live. Azimuth now rides a round-robin record, `PRN · 0xFE · azimuth`,
one satellite an epoch. The register became `TIER_C`, one bit, the augmentation pages.

## The data model as it stood in `DATA.md` and `THEORY.md` — superseded

- **One constellation per slot, by node address** — the five slots are one pool; the PRN names it.
- **Navigation Unix second plus a PPS subsecond in frame 0** — now the last `RMC`'s Unix second
  and `pps_tick`, the PPS edge on the node's grid.
- **37 B a satellite, unpooled** — an upper bound; 31 B pooled, 55 % of five slots at worst sky.
- **"All three tiers in real time and archived"** — Tier B and C are the archive.
- **"Five nodes on one board"** — one unit, five NUMBERs, its own H523 with the UM980.

## The node as a copy of the NIC core with its own 485 — superseded

No 485 part: plain logic on the Galvani data body, 3,3 V from its own `LMR43610`, not "a 1 A
class" of a port board.

## `LINE_EN` on two GPIOs, and the sockets' `B_DIR` written backwards — superseded

Both `LINE_EN` were GPIOs set high at boot; **a unit end holds its Galvani boards on with
resistors**, and Kronos's heartbeat on `DE_T` gates the time stream. Channel B was written
backwards: the NodBus socket hears the card's clock (`B_DIR` to ground), the time port drives PPS
to Kronos (`B_DIR` to 3,3 V).

## The UM980 on a small carrier — superseded by the module soldered to the board

A 54-pin LGA, 22 × 17 × 2,6 mm, its 48 inner pads on a large ground for heat; a carrier adds
connectors for three UARTs, PPS and RF: **the board is the carrier**. The "half-amp peak" was
145 mA typical, 180 mA max; `SBAS` left the constellation list, unnamed by the sheet.

## The receiver's message set as first written — superseded by the Unicore names

`RANGECMPB`, `IONUTCB`, `UNLOGALL`, `E6HASB`, `B2BPPPB`, `L6EMADOCAB`, an SBAS grid log — another
maker's names, **none in the Unicore manual** (N4 products, R1.15); `FIRMWARE.md` §6 has the
Unicore sequence. Four changes:

- **QZSS L6E MADOCA is not logged.** `CONFIG SIGNALGROUP` is a fixed menu: the L6E groups (3, 10)
  drop GPS L1C, GLONASS G3, BeiDou B1C and NavIC, the Tier B backbone and archive; group 2 keeps
  it with E6 and B2b.
- **The SBAS grid is not a Tier C source.** SBAS L1C/A serves the fix, its messages not output;
  `CONFIG SBAS` only steers the fix. The `REGION` register went with it.
- **`GSV` on COM3 gave way to `SATSINFO` on COM1** — same azimuth and elevation per PRN, binary,
  on the parsed port; COM3 is configuration only.
- **The PPS width is 10 ms, not 100** — the receiver takes 50–20 000 µs.

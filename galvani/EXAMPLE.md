★ N.I.C. ★

# An example build — one card in a box

> **This is an EXAMPLE, not doctrine.** Nothing on this page is a requirement — sizes,
> boxes and fasteners are illustrative, the builder decides at the bench. The binding
> contract (signals, supplies, connectors) is [`README.md`](README.md).

A plain plastic junction box, around 20×20 cm. The card and the Galvani boards each sit on a
pair of standoffs — a Galvani board is small, roughly 10×5 cm with two mounting holes, held by
plastic screws. If the protection parts want room they can stand vertical; the box still
swallows a complete card with all four ports fitted — two of them, in fact.

**A port is two boards and two ribbons; the 12 V reaches the power board on the wire.** For a fed copper spur that is `G-48-S`, the source
power board, and `G-I-N-025`, the 485 communication board.

Wiring is snap-and-done, and **nothing is screwed to a Galvani board**:

- **the ribbons** — a 12-pin data cable and an 8-pin power cable per port: the card's sockets, the
  communication board, the power board. No splice. It carries the data, `CLK`/`PPS`, `LINE_EN`
  and `ID` on the data connector, and `ENABLE`, `ID` and the I²C telemetry on the power connector, plus the
  3,3 V each plugged board runs on.
- **the 12 V to the power board** — the 12 V wire, 2,5 mm², a star from the fuse field: one fuse
  a board, every board taking it on its own terminals — the card, the power boards, the Mayak.
  No board carries 12 V across itself for another.
- **the line out** — the four pairs on the data cable through one gland, and **the 48 V feed in a
  2-core cable of its own** through a second. They never share a cable and the feed never rides
  a connector, so a cross-plug damages nothing.
- a glass port instead takes `G-O-10-10` and the pigtail through the gland; nothing else about
  the build changes.

No motherboard, no backplane, no board that everything must fit onto, **and no supply board**: add a
spur, screw two boards next to the others, plug two ribbons, run the power board its 12 V from a
fuse of its own, land the line. A unit sharing this box needs no Galvani board at all — it takes a
**crossed cable** on the same 12-pin data connector, `TXD`↔`RXD`, `ID`↔`ID_RET`; the 12 V off the
wire on the unit's own terminals.

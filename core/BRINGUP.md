★ N.I.C. ★

# NIC — bring-up (from JLC box to green data)

> **Design-stage concept.** The figures are verified against the parts' sheets and the parts
> before a board is made.

**The order, and the gates.** Every line is a gate — do not skip forward past a red one. What a
board must show on the bench is its own `HARDWARE.md`'s, under *Bench criteria*; this page is the
order the station comes together in and the two gates that belong to no one board.

## 0. Common: before any board

1. Bench PSU with current limit (50 mA first power-up), scope, USB-RS485 dongle.
   **At a live station you power from the PACK and from nothing else.** The battery is the 12 V
   node and every board hangs on it — solar-charged or mains-charged, it is the same one node —
   **so the ground is common by construction** and there is no second source to bring. On the
   bench the board is bonded to no earthing point and the question does not arise. **A second
   earth appears only where somebody powers a board at a live station from a source of its own**,
   and then the path between the two anchors is through the board.
2. Every board passes its own `HARDWARE.md` bench criteria before it meets another board.
3. Nothing to set on a fresh unit: it carries its TYPE and answers on the default number until the sweep gives it one (`PROTOCOL.md` §2).

## 1. The boards, in the build order

The feed cell first, then Kronos, the card, the Mayak, then the units — each on its own bench
criteria, the card and the Mayak against Kronos's time bus, each unit against a card's port. A
board is never judged on noise inside its first 30 minutes: the settle window is flagged in the
data, not waited out by eye.

## 2. Station (integration)

0. **Ports come up OFF and are enabled only after the run is wired and the far end established.**
   Never enable a port to find out whether something is on it: the family's converters are no-opto
   flybacks with no loop to hold an unloaded output down, so an empty enabled port climbs to the
   transil and a device plugged in afterwards meets the risen rail
   (`../galvani/README.md`, *Three states*).
1. The ladder by the end of the cable: the tube at a source end, the transils everywhere (`../galvani/README.md`, *The protection ladder*).
2. The delay survey (`COMMISSIONING.md`, Phase 3) — the antenna coax, the one delay entered; the spurs
   range themselves.
3. 48 h soak: SOH clean, no CRC storm, no unexpected boot-counter increments — then bury it.

## 3. The cold return — and it is a gate, not a footnote

**A station that has been off comes back by itself or it does not come back.** The path has no
operator in it and it is the same path whether the pack went flat, the BMS tripped, or somebody
pulled the main fuse.

1. **The pack returns.** The BMS switches itself back on at its own recharge threshold
   (`POWER.md`) — nothing commands it and nothing needs to be alive for it to happen.
2. **Only the main enclosure comes up.** The battery lands on the Mayak and on the cards, so
   those are what have voltage: the head, the clock, the cards and whatever sits on their
   in-box cables. **Every fed unit outside stays dark**, because its feed is made by a station
   power board whose `ENABLE` is a GPIO that has just booted low.
3. **The Mayak works out what happened before it turns anything on.** Boot counter, the BMS's
   own log read through Hermes, the last state in its store — a return from a flat pack
   and a return from a power cut are different events and reach the server as different ones.
4. **Then it brings the station up in the ordinary order**: Kronos first, because nothing is
   timed until the clock is locked, then port by port — `ENABLE` up, the sweep, enrollment
   (`PROTOCOL.md` §2). A unit's configuration is in its own store, so this is a restart and not
   a reconfiguration.

**Verify it on the bench by pulling the pack, not by pressing reset.** A reset is not this test:
what is being tested is that nothing in the chain waits for a human, and a reset button proves
the opposite of that.

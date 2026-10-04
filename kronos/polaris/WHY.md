★ N.I.C. ★

# Polaris — the graveyard

Rejected alternatives and superseded states, with the reason. Nothing returns from here and
nothing is deleted from it.

| rejected | reason |
|---|---|
| **A card of our own — two footprints, the u-blox pair on one and the `UM960` on its own, a bias tee, an SMA socket, the module's decoupling and backup pin on our board** | A maker's GNSS board with SMA, bias tee and a pin row **costs less than our RF layout would**, and the layout is a fortnight of design for a part that does one thing. Polaris is the bought board on a carrier with one connector, one row and two resistors; the choke, the DC block, `V_BCKP` and the decoupling are the bought board's, checked against its listing, not designed |
| **The receiver at the mast top, in the antenna, on an isolated 485 run with its own 12 V** | It would have removed the coax — the station's one unisolated conductor — and its two arresters, **at the price of a second cable up the mast, a doctrine change** and either an RS-422 timing receiver or a carrier with a buck at the top (the shelved standalone carrier). The module in the box costs no design and keeps the doctrine; the coax keeps its two DC-pass arresters and the insulating bracket |
| **A code of its own on `ID`** | The code names the interface: `GNSS` at 0,35 sits on both ends of every port carrying the NMEA time stream; the receiver is typed in `0x5C SRC_TYPE` |
| **Four types on the carrier — `M7N` · `M8N` · `M9N` · `UM960`** | **Four types meant four boot dialogues in Kronos** — u-blox 7 and M8 on the legacy `CFG-*` messages, M9 on `CFG-VALSET`, the UM960 on Unicore's commands — for one job all do alike: a PPS inside 60 ns and `RMC`/`GGA`. `M7N`, weighed for price and draw, lost to the M8N's concurrent GPS + GLONASS at ~30 mA; `M9N` and `UM960` bought nothing for time. One type, `M8N`; a build fitting another receiver types it and brings its own dialogue |

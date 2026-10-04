★ N.I.C. ★

# Pip — the graveyard

Rejected alternatives and superseded states, with the reason. Nothing returns from here and
nothing is deleted from it. The board's graveyard is `../WHY.md`.

## Shelved, and held back by coverage

Longwave time services reach Europe, Japan, the USA, China and Korea — **where NIC puts the fewest
stations**. Without a time-code transmitter in range it recovers a rate, not a date, so it cannot
restart a dead station; a severe solar event takes GNSS and longwave together. It came off the
shelf as the SID channel, useful wherever a carrier is heard, and because eLoran is being built
out (China; a European revival discussed).

## A board of its own, and an H523 for it — superseded

A dedicated board with a 34–120 kHz front end died when Tesla's band widened: **every transmitter
Pip hears is inside Tesla's band**, and keeping sferics off the converter is done by its dynamic
range and the DSP gating. An H523 for the carrier trackers would make Pip a board again for
nothing. Pip is Tesla's board under its own image.

## Band edges

- **An eLoran-only narrow variant (80–130 kHz)** — never needed: DCF77 is only loud where eLoran
  does not exist.
- **The military VLF transmitters (17–25 kHz)** — communication, not time: MSK flips the carrier
  phase, they leave the air unannounced, and never tell the date.
- **HBG 75 kHz** — off the air since 2011.

## Off the bus — superseded

Pip was first a task in Tesla's image behind a configuration bit, then a unit on Tesla's board
speaking only to Kronos, with no NodBus body and no type. **It had no way up**: a station wanting
SID too ran the same carriers through two trackers, its state reached the head only through Kronos,
and it had no address. Now a NOD, type 12,
`NB IN` to the bus and `TIME OUT` to Kronos (Sputnik's model), with the SID channel moved here.

## The in-box build — superseded

Crossed cables to a card and to Kronos, `ID_RET` carrying the `GNSS` code, `LINE_EN` following
`DE_PIP` — which held the line side dark while serving waited on a heartbeat that needed it up.
**A measuring unit is never in the box and a ferrite antenna cannot be**: `TIME OUT` always
carries a Galvani board, `ID_RET` unconnected, `B_DIR` and `LINE_EN` held in the socket, and takes
all three communication boards, channel B either way by `B_DIR`. The 2 Mb/s glass board was refused for a while because its channel
B ran one way, the wrong way.

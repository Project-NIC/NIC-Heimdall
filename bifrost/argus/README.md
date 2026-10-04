<p align="center">
  <img src="NIC-Argus.svg" width="200"/>
</p>

★ N.I.C. ★

# Argus — the card carrying four NodBus mini segments

> **Design-stage concept — nothing built.** The figures are verified against the parts' sheets and
> the parts before a board is made.

**Argus is the Bifrost card with nothing on its `ATTN` pin** — the same board, the same firmware,
the low level at boot making it an Argus (`../`). It has no sensor of its own: it is a
**NodBus unit on a Bifrost port whose whole job is to carry small clocked units**, the mini-NODs —
Gauss, Quark-Tubes, Pascal, types 1–3 of the mini space. Four of its six USARTs each drive one **NodBus
mini** segment, the fifth is its link up to the Bifrost, the sixth that link's echo receiver.

**NodBus mini is NodBus with an 8 B payload** — the same framing, the same TDMA, the same wire
clock, the same `TYPE«4 | NUMBER` addressing — at the Argus's own rung, **2¹⁹**, on all four
segments. It is synchronous, so the slot is the time: a record assembles because it arrived when
it was due, with no marker and no age field. **A segment costs a whole UART**, because TDMA needs a
continuous ear — that is the price of the timing, and it is why ModBus, which shares one UART
among many, stays in the station for what needs no clock.

**Eight sondes a card, the same eight as a Bifrost.** Glass is point-to-point and copper may chain,
so eight optical sondes are two Arguses. **Upstream an Argus tiles four 8 B mini payloads into one
32 B payload** at positions fixed at enrollment and takes **⌈sondes/4⌉ NUMBERs** of its own type,
3: four full-rate Gauss are one slot, eight are two. The segment table the card reports decodes the
positions; no byte is added.

**It stands in the enclosure on a crossed in-box cable to a Bifrost port, or at a distance behind
Galvani boards.** Every segment leaves on Galvani boards, because a mini-NOD stands outside.

```
   BIFROST port ══ the up port, 2²² ══▶ ┌─────────────────────────┐ ══ segment 1, 2¹⁹ ══▶ mini-NODs
   no time bus: ATTN low                │ ARGUS — the card, H523  │ ══ segment 2 ══▶      Gauss · Quark-Tubes · Pascal
                                        │ 1 up + its echo,        │ ══ segment 3 ══▶      each segment a whole USART,
                                        │ 4 segments              │ ══ segment 4 ══▶      8 B TDMA
                                        └─────────────────────────┘
        four 8 B payloads tiled into one 32 B payload · ⌈sondes/4⌉ NUMBERs up the port
```

## Files

| file | contents |
|---|---|
| [`HARDWARE.md`](HARDWARE.md) | what differs from a Bifrost, and Argus at a distance — the site's optics, its feed and its 12 V wire |
| [`../HARDWARE.md`](../HARDWARE.md) | the card itself — its pins, sockets, supply and parts, which are Argus's |
| [`../FIRMWARE.md`](../FIRMWARE.md) | the firmware — one image for both roles; the Argus layer in §8, the card as a unit in §9 |
| [`WHY.md`](WHY.md) | the graveyard: the freeze broadcast, the aggregation formats that lost, sixteen minis, and the rest |

## Licence

Hardware: CERN-OHL-S v2 (`../../LICENSE-HW`) · Software: MIT (`../../LICENSE`) — Copyright (c) 2026 NIC — Native Intellect Community

★ N.I.C. ★

# Steinmetz — line faults on power lines, from a vehicle or a site

> **Design-stage concept — nothing built.** The figures are verified against the parts' sheets
> and the parts before a board is made.

**Steinmetz is Tesla's board under its own image: NodBus type 13, a NOD.** The image does one
thing: it finds and classifies the mains-locked impulsive sources of a power line — arcing,
corona, partial discharge — with the whole H7A3 given to that task and nothing else. The unit is
the same on a vehicle roof at 80–100 km/h and on a mast at a static site; nothing is fitted or
removed between the two.

- **The board is Tesla's, identical to the last part** — the three rods, the chain, the
  converter, the rails, both data bodies. The three bands stay; **the front is 20 dB down** on Tesla's own attenuation terminals, so
  the line overhead sits inside the range and the lightning horizon falls to 500–1000 km
  (`HARDWARE.md`); the image is sized for the line, not for the storm beyond the horizon
  (`FIRMWARE.md`).
- **It hangs on a station like any unit**, the unit end of a run: a communication board for its
  medium and the 12 V on its terminals. In a vehicle the station is a Proteus in the cabin
  (`../../proteus/`), the run **one hybrid cable to the roof** through one gland; the GNSS antenna
  sits on the windscreen, the receiver on Kronos as in every station.
- **The record is Tesla's 4 B event**, offset and amplitude unchanged, the two type bits reading
  **0 arc · 1 corona · 2 partial discharge · 3 UFO**; the floor and the source states ride in the
  second NOD as on Tesla. **What the server makes of it**: TOA across units along a line whose
  position is known is one unknown coordinate; the leading edge timed, ~1 000 firings an episode
  give 30–35 dB, and a pair of units straddling the fault resolves ~150 m — a short list of
  towers, and the alarm a source changing class from UFO to arc over days.

Named for **Charles Proteus Steinmetz**, who wrote the arithmetic of transients on transmission
lines and built lightning in a laboratory to test them.

## How it hangs on the station

```
  roof / mast                                       cabin / enclosure
  ┌─────────────────────────────┐   hybrid cable    ┌──────────────────────────────────────────────────────┐
  │ Tesla's board, Steinmetz's  │ 4 pairs + 2 cores │ PROTEUS — Mayak · Kronos · Bifrost                   │
  │ image, type 13              │◀═════════════════▶│ port 1 — the 485 island, 4 pairs on terminals        │
  │   NB IN  ◀─ communication   │  one gland each   │ PWR OUT 1 — the 12 V across a barrier                │
  │           board             │                   │ Polaris on Kronos — coax — antenna on the windscreen │
  │   12V    ◀─ terminals       │                   │ Wi-Fi · modem · Ethernet · USB                       │
  │   3 rods                    │                   └──────────────────────────────────────────────────────┘
  └─────────────────────────────┘
```

## Files

| file | contents |
|---|---|
| [`HARDWARE.md`](HARDWARE.md) | what differs from Tesla's build: the link to the roof, the feed, the vehicle supply, the antenna, the enclosure. The board is `../HARDWARE.md`, whole |
| [`CONSTRUCTION.md`](CONSTRUCTION.md) | the vehicle mount — the box on the roof bars, the roof under the rods, the cable, the vehicle's own noise; a static site builds as Tesla |
| [`FIRMWARE.md`](FIRMWARE.md) | the image — the record's four classes, the registers it fixes, what the head does for a moving unit, what the image leaves idle |
| [`WHY.md`](WHY.md) | Steinmetz's own graveyard; the board's is `../WHY.md` |

## Licence

Hardware: CERN-OHL-S v2 (`../../LICENSE-HW`) · Software: MIT (`../../LICENSE`) — Copyright (c) 2026 NIC — Native Intellect Community

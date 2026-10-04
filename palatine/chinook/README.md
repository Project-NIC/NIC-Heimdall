<p align="center">
  <img src="NIC-Chinook.svg" width="200"/>
</p>

★ N.I.C. ★

# Chinook — what the air is made of

**English** · [Čeština](README.cs.md) · [Русский](README.ru.md)

> **Design-stage concept — nothing built.** The figures are verified against the parts' sheets and
> the parts before a board is made.

**Chinook is Palatine's air-composition set, not a board and not a unit.** Every air sensor worth
fitting is a bought RS-485 ModBus unit on a Palatine arm, a slave at the address written into it
like every bought sensor's (`../HARDWARE.md`, *The Modbus address map*), read by Palatine like its
other sensors (`../`, `../../core/blocks/modbus.md`). Nothing is manufactured, nothing is enrolled
on the NodBus, and nothing is added to the protocol.

**Palatine measures the weather, Chinook what the air is made of.** Temperature, humidity,
pressure, wind, radiation, UV, precipitation, snow, soil and leaf wetness are Palatine's. What is
left is **air chemistry and particulates** — the quantities a site fits for its own reason, in its
own setting, on its own budget.

## The base fits nothing

**A station measures no air chemistry unless a site asks for it, and the reason is money.** Every
air-chemistry unit is a consumable or a service item:

| class | what wears | life |
|---|---|---|
| electrochemical cell — CO, NO₂, O₃, SO₂, H₂S, NH₃ | the cell, which consumes its target gas | ½–2 years |
| MOX — a VOC or NOx index | the heated element drifts; the index re-baselines, the absolute never holds | 1–2 years of usable index |
| laser particle counter — PM | a fan, and an open optical chamber that fouls with the dust it counts, condenses in fog and takes insects | a season between cleanings outdoors |
| NDIR — CO₂, CH₄ | nothing: a sealed optical path, no moving part | fitted and left |

One unit is about a hundred dollars and a couple of years; five on a station are a few hundred
dollars a year and a visit; a hundred stations are a standing budget and a maintenance round — for
quantities that in open country mostly measure the country. **So the base carries none of it.** A
site that wants air chemistry buys the units, budgets their replacement and cleaning, and hangs
them on an arm; `SENSORS.md` says what to buy by setting and what each must meet.

## What the station gives an air unit

- **A Palatine arm**: the arm's 12 V (a bought part takes 10–30 V), one rate per arm, 9 600 or
  19 200, and a roster entry with the unit's register run; a bought unit is verified by answering.
- **The station clock on every reading**, so a CO rise sits on the same second as the strike that
  lit it where Tesla is fitted.
- **Gating per unit** at the arm's power board between reads, where the part tolerates it — a heated
  unit or a fan-and-optics unit may not.
- **Nothing else**: no correction on the node, no air board, no heated chamber in the base.

## The block

```
   PALATINE ──a ModBus arm──▶ bought RS-485 ModBus units, each at its written address:
                               particulates · CO · CH₄ · NO₂ · O₃ · SO₂ · H₂S · NH₃ · CO₂ · VOC — per site, none in the base
```

## Files

| file | contents |
|---|---|
| [`SENSORS.md`](SENSORS.md) | what to buy by setting, and what an air unit must meet — the particle counter, the chemistry classes, temperature, humidity |
| [`CONSTRUCTION.md`](CONSTRUCTION.md) | the housing — the chimney, the mesh and the baffle, the counter's inlet, where it stands, the cold-climate build |
| [`WHY.md`](WHY.md) | the graveyard |

## Licence

Hardware: CERN-OHL-S v2 (`../../LICENSE-HW`) · Software: MIT (`../../LICENSE`) — Copyright (c) 2026 NIC — Native Intellect Community

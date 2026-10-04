★ N.I.C. ★

# Contributing to NIC-Heimdall

**NIC-Heimdall is a concept, and its documents are the design.** Nothing has been built yet; what
is here is the description a builder works from — the parts, the values, the pins and the
arithmetic behind them, and the reasons for each choice. Helping means making that description
truer, or taking it one step towards hardware.

## What helps most

| you can | how |
|---|---|
| **find a mistake** | the most useful thing anyone can do. Open an issue naming the file and the section, what is wrong, and the source that shows it — a datasheet page, a measurement, a calculation |
| **draw a board** | from its `HARDWARE.md`, into [`schematics/`](schematics/README.md), one folder per board, saying which state of the description you drew from |
| **build and measure** | a board's `HARDWARE.md` says what it must meet, many in a bench-criteria section; a measured number against one of them settles more than any page of reasoning |
| **write firmware** | against the unit's `FIRMWARE.md` and [`core/PROTOCOL.md`](core/PROTOCOL.md), once its board exists |
| **write a reader of the archive** | an exporter as [`core/archive/EXPORTERS.md`](core/archive/EXPORTERS.md) enters it, on the HMC reference |
| **site a station** | where one would fill a gap — [`gaia/`](gaia/README.md) |

## The rules of the record

- **One document owns each fact.** A change goes into the document that owns it; another
  document may repeat it, never contradict it. When two disagree, the owner is right until it is
  corrected.
- **Nothing is deleted from a `WHY.md`.** Every project keeps one: what was tried, what was
  rejected or superseded, and why. A change that replaces a decision adds an entry there — the
  reason is the part worth keeping.
- **A figure comes from a source.** A part's sheet, a standard, a measurement, a calculation shown
  in the text. "About" is allowed; unsourced is not.
- **Our own parts are reference points, not a mandate.** A part number fixes the topology and the
  target values; an equivalent part that meets them is as good.
- **Bought sensors are specified by what they must meet**, never by type: a named part is the one
  a builder in another country cannot get.
- **English is the master text.** Czech and Russian follow once the design settles.

## Issues and pull requests

- **An issue for a question or a finding, a pull request for a change** — one topic each, small.
- Say **which document owns** the change, and add the `WHY.md` entry when a decision moves.
- Keep relative links and `#anchors` working; a moved section takes its links with it.

## Safety

Some units work with lethal voltages — the radiation tubes run at 400–2400 V — and with toxic
materials, never machined beryllium and BF₃ among them. Each project's README says what it asks
for. Nothing here replaces the judgement of the person at the bench.

## Licence

By contributing you agree that your contribution is licensed as the repository is: **hardware
under CERN-OHL-S v2** ([`LICENSE-HW`](LICENSE-HW)), **software under MIT** ([`LICENSE`](LICENSE)).

★ Viva La Resistánce ★

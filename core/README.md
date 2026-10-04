★ N.I.C. ★

# NIC Core — the universal platform

**English** · [Čeština](README.cs.md) · [Русский](README.ru.md)

> **Design-stage concept — nothing built.** The figures are verified against the parts' sheets and
> the parts before a board is made.

This is the **front-agnostic** half of NIC: the libraries and hardware blocks that
every product front (Quake, Palatine, Sputnik, …) reuses *unchanged*. Core knows nothing
about seismometers, rain gauges or GNSS receivers — it moves bytes, distributes a
clock, keeps time, schedules slots and talks to the master uplink. All product meaning
lives in a front's glue, never in here.

> **Guardrail.** If wiring up a front tempts you to reach into a core library and add
> product-specific meaning, stop: that meaning belongs in the front's glue. The core
> stays generic so a new front costs zero core changes (config is generic — see
> `PROTOCOL.md` §5).

---

## The load-bearing principle — a node depends only on the NodBus

**A Node is hardware-independent. It depends on nothing but the NodBus protocol — so
it runs anywhere, with anything, connected to anything.**

That is the whole reason the platform is universal, and it is stated here **once**:

- A node is not "a seismograph" or "a weather station". It is *a thing that speaks
  NodBus*. What it senses is a detail of its front glue.
- It makes no assumptions about who its neighbours are, what the master is, what the
  other nodes measure, or what cabling it hangs on. It boots, announces its **type** (what
  kind), takes the **number** the sweep gives it, takes a TDMA slot, and streams frames.
- Therefore any node runs on any NIC bus, mixed freely with any other node types, behind
  any master — because the only contract between them is the NodBus protocol (the
  `nic-link` frames, the control opcodes, the shared clock and the self-running slots).

Everything else in `core/` exists to make that contract small, robust and portable.

### NodBus, in one paragraph

**NodBus** is the data + clock backbone the nodes hang on, on copper 485 or glass behind a Galvani board. The UART data rides
self-timed TDMA slots — **the duplex follows the feed, and every class now takes its power in a
cable of its own, so every class is full duplex** (`G-I-N-025` on copper, `G-O-10-10` / `G-O-2-100` on glass); there is no half-duplex NodBus
build. Another pair carries the network clock
(born on Kronos, the single source being `blocks/nodbus.md`, *The network clock*),
regenerated at each transceiver so it survives long cable. The NodBus proper starts
behind the station's Bifrost bridges — the head speaks to it over point-to-point
MasterNOD links. Bought Modbus sensors and the slow house modules sit on Palatine's ModBus arms —
same physics, a polled bus with no clock. See `blocks/nodbus.md` (NodBus) and `blocks/modbus.md`
(ModBus).

## The second load-bearing principle — a unit survives interference without knowing what it is

**No unit in the station ever identifies the interferer, and none needs to.** A stroke, an
arcing line, a switching converter, a radar, a welder in the next field, an EMP, something
nobody has characterised — naming the source is a downstream question and no input stage can
answer it. **What is required of every unit is the same four things, and they are a condition
of the design, not a per-board choice:**

1. **It detects that its input is no longer the signal** — against its own running background,
   never against a compiled-in threshold. Every unit already computes that background for its
   own measurement, so this costs nothing new.
2. **It degrades predictably.** A saturated stage recovers in a bounded time, a loop coasts
   across the hole, a filter is not left ringing into the next frame. Nothing latches, nothing
   runs away, and **no unit needs a power cycle to come back.**
3. **It flags the frame** — `status` 6 CLIPPED where a value hit its range, `status` 7 DISTURBED
   where it did not clip but the input was not the signal, and the affected fraction of the
   window where the unit measures one (`PROTOCOL.md` §1).
4. **It never publishes a disturbed number as if it were clean.** An archive that cannot tell a
   disturbed reading from a good one is worse than a gap: the gap is honest.

**A defence that has to recognise the source first fails on the first source nobody
characterised** — which is the class this station exists to find (Tesla's UFO type,
`../tesla/DETECTION.md`). So the measures are source-agnostic by construction: blanking on the
envelope, recovery on a bounded time constant, a flag on the frame. **The recovery time is a bench
criterion of every unit that saturates**, written in its own `HARDWARE.md`: it is what turns a
blanked fraction into a level error.

---

## Hardware blocks (`blocks/`)

Reusable hardware blocks shared by every node and master — copy a block, don't redraw it:

| Block | What |
|---|---|
| [`nodbus.md`](blocks/nodbus.md) | The NodBus: the data pairs + the network clock pair, copper or glass (**its first section is THE clock definition**), termination |
| [`clocks.md`](blocks/clocks.md) | **Which board carries which oscillator, tier by tier** — the one precise part at Kronos, nothing on the cards and the NodBus units, the HSI disciplined by the rung on the mini units, a commodity crystal on the ModBus MODs, and the binary rule that lets it be bought by stock |
| [`modbus.md`](blocks/modbus.md) | The ModBus leaf — Palatine's arms, the bought sensors and the house MODs on them, and the house register map |
| [`gps-pps.md`](blocks/gps-pps.md) | GPS PPS / timing doctrine + the in-box time distribution (Kronos) |

---

## The block

```
   KRONOS ──the M-LVDS time bus, 2²² + PPS_K──▶ the CARDS ──spurs, 2²², 40 B TDMA──▶ NODs: Quake · Palatine · Sputnik · Tesla · Marconi · Photon · Positron
      │                                          │
   PPS-K + label ──▶ MAYAK ◀──trunks, 48 B────────┘        ARGUS ──segments, 2¹⁹, 16 B─▶ mini-NODs: Gauss · Quark-Tubes · Pascal
                       │                                   PALATINE ──ModBus RTU arms──▶ MODs and bought sensors
                    the uplink
   the core: framing · addressing · TDMA · the clock · the node contract        a front: one sensor and its glue, nothing of the core changed
```

## The tree

```
core/                    the node core — the bus, the frame, the clock, the uplink
├── archive/             HMC, the container · HCC, the codec · the exporters · ref/, the reference library
└── blocks/              the hardware blocks, one file each
```

## Files

| File | Contents |
|---|---|
| [`PROTOCOL.md`](PROTOCOL.md) | frames, addressing, TDMA, the time model, the data tiers, the uplink, the node contract |
| [`COMMS.md`](COMMS.md) | **the communication map** — every link in the station in one table, each row naming the owning document |
| [`HARDWARE.md`](HARDWARE.md) | how the blocks assemble into a station — the universal node and the head |
| [`POWER.md`](POWER.md) | source, storage, distribution, the thermal case and the BMS backstop |
| [`DESIGN.md`](DESIGN.md) | the decision index — D-numbers, each row naming where the content lives |
| [`BRINGUP.md`](BRINGUP.md) | bring-up in order, from the box to green data, and the cold-return gate |
| [`COMMISSIONING.md`](COMMISSIONING.md) | commissioning — bench, then field, then bring-up |
| [`UPLINK_TRANSPORT.md`](UPLINK_TRANSPORT.md) | how a station reaches the server |
| [`INTEROP.md`](INTEROP.md) | who takes our data, and in what format |
| [`WHY.md`](WHY.md) | the graveyard |
| [`blocks/`](blocks/) | the hardware blocks, one file each — the table above |
| [`archive/HMC.md`](archive/HMC.md) | the archive: the container — a series of files per unit, the recording and send rules, data from the front and the description from the back |
| [`archive/HCC.md`](archive/HCC.md) | the archive: the codec — one address's kept records, column by column, lossless; no table, a rule |
| [`archive/EXPORTERS.md`](archive/EXPORTERS.md) | the archive's readers — miniSEED, IAGA-2002, RINEX, BUFR, CWOP, IOC, air quality, Blitzortung, CSV, SQL, the viewer; one template each |
| [`archive/ref/`](archive/ref/README.md) | the reference code, Python — HCC's coder and decoder, Steim-2, the measurement |

---

## Licence

Hardware: CERN-OHL-S v2 (`../LICENSE-HW`) · Software: MIT (`../LICENSE`) — Copyright (c) 2026 NIC — Native Intellect Community

★ N.I.C. ★

# Iris and Arke — the firmware

> **Design-stage concept — nothing built.** The figures are verified against the parts' sheets and
> the parts before a board is made.

**One firmware, two roles**, on the MODs' engine (`nic-mod`) with the radio added and `core/proto`'s
portable C beneath — the tag as `nic_cmd` computes it, the packet and the acknowledgement of
[`../LINK.md`](../LINK.md). Arke's own part is in [`arke/FIRMWARE.md`](arke/FIRMWARE.md).

## 1 The role

At the first start the board switches its output on, waits for the current to settle and reads the
eFuse's `ILM`: **current out is Arke, none is Iris.** The role is kept, with the address and the
settings, until a magnet held on the reset reed clears it; the start after finds it again. It is
written into the boot log with how it was found. An Arke whose arm has died stays Arke and reports
that its arm draws nothing.

## 2 Iris — the receiver

- **Listening:** the radio receives, retuned in each unit's slot to that unit's band and spreading
  factor; between slots the H523 sleeps in Stop, woken by the radio's `DIO1` or a ModBus frame.
- **A packet:** the tag checked under the unit's key, the counter against the last (a replay refused,
  a duplicate dropped), the record copied into the unit's window, the acknowledgement sent with the
  milliseconds to the unit's next wake and any setting waiting in the window.
- **Palatine's read:** a window's age register is computed at the read — the time since arrival plus
  the unit's own age — so the record's time is Palatine's stamp less it.

## 3 The register map

The house map and the house block (`../../core/blocks/modbus.md`), and Iris's own:

| register | name | |
|---|---|---|
| `0x0000` | `VALUE` ro | units heard in the last ten minutes |
| `0x0002` | `STATUS` ro | the house bits |
| `0x0004`+ | Iris's own ro | packets received, refused by the tag, met in the air |
| `0x0080` | the key register, wo | a unit's number and its 16 B key, written by the head after unwrapping it; never read back |
| `0x0100 + 16·n` | **unit n's window**, n 1–255 | below |
| `0x1100 + 16·a` | **an Arke block's window**, a its sensor's station-unique address | below |
| `0xFF00`+ | the house block | |

**A unit's window:**

| reg | high byte | low byte |
|---|---|---|
| 0 | the unit's number | kind and flags |
| 1 | counter | |
| 2 | age at the read, 0,1 s | |
| 3–7 | the values as they flew — Ratatoskr: T 2 m · RH 2 m · T 5 cm · RH 5 cm · pressure | |
| 8 | the cell, mV | |
| 9 | the cell's temperature | 0 |
| 10 | RSSI, dBm | SNR |
| 11–12 | a setting waiting: id · value (rw) | |
| 13 | the unit's link: band · SF (rw) | |
| 14 | Imber's tip count, where fitted | |
| 15 | spare | |

**An Arke block's window:** 0 Arke's number and flags · 1 the counter · 2 the age · 3–14 the block's
registers, at most twelve; a longer run of a sensor is read in two.

**Palatine reads a window as one run**, at most fifteen registers, a block of 30 B in its payload; which
unit it is comes from register 0, since every window has the same length.
**Each window is one entry of Palatine's roster** — address `0x20`, the window's start, fifteen
registers, every 600 s, on an ungated arm, the radio listening between polls — and the roster holds 64 entries for all four arms (`../../palatine/FIRMWARE.md`
§6): **on a full weather base an Iris carries some fifty units**, not the 255 its numbers allow; more
is a second Palatine. A window read again before its unit's next packet gives the same counter, and
**the head keeps a field record once per counter**.

## 4 Keys and settings

A unit's key comes from the handset at the station wrapped under the station's K0; the head unwraps
it and writes it through the tunnel into Iris's key register — never in clear on any air. A setting
written by the head into a window's registers through the tunnel rides the next acknowledgement; the unit's next
packet says it applied it, and Iris clears the registers. A link setting is on trial (`LINK.md` §6).

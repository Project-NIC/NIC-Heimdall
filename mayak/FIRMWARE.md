★ N.I.C. ★

# Mayak — the firmware, described

> **This document is the deliverable, not a description of code.** It says everything the
> head's firmware does, in the order it does it, with the numbers it uses — enough to write the
> build from, and enough to test the build against. What the head *is*: [`README.md`](README.md);
> the board and the pins: [`HARDWARE.md`](HARDWARE.md); the frames, the opcodes, the sweep,
> the data model, storage and uplink, the card's control surface:
> [`../core/PROTOCOL.md`](../core/PROTOCOL.md); the card's side of the trunk:
> [`../bifrost/FIRMWARE.md`](../bifrost/FIRMWARE.md); the clock board's side of the time bus:
> [`../kronos/FIRMWARE.md`](../kronos/FIRMWARE.md); the power doctrine:
> [`../core/POWER.md`](../core/POWER.md); the phone: [`handset/README.md`](handset/README.md).
> Where this document and one of those differ, that one wins and this one is corrected.

## 1. What the firmware is

**A listener with four ears, one archive and two radios.** The head brings four Bifrost floors
up with one verb each, then receives what the cards hand it — finished, stamped 48 B records —
checks their CRCs, closes each second at T+1 s, keeps of each unit what its recording rule
selects and writes it into that unit's own files on two microSD cards. It composes the station tree from the tables the cards report, holds the
configuration mirror, and is the one door between the station and the world: the modem's values
frame out, the Wi-Fi session to the server, the phone over BLE at the open box. It reads the
pack through Hermes once a minute and is the one thing in the station that puts units down in
order before a deficit takes them. **It interprets no front's payload**: a Quake sample and a
Sputnik chunk are bytes under a TYPE, kept or dropped by a rule that compares bytes, and the
server reads them.

**Three processors, three jobs.** Core A captures and stores; core B talks — Wi-Fi, the modem, BLE,
Hermes, the mirror; the LP core alone survives a deficit. Nothing time-critical crosses a core
boundary: the only precise instant the head handles is the PPS-K edge, and that is a hardware
capture. **The split is by stall**: the trunks land by DMA in rings that absorb a microSD card's
wear-levelling pause of tens of milliseconds, so a slow write never costs a frame, and the Wi-Fi
and BLE stacks — the worst stallers on the chip, pinned to their own core by ESP-IDF anyway — sit
wholly off the capture path. The head is a listener: the cards run their floors and their clocks,
and nothing on the head hands out a token per sample.

| the firmware does | on | how often |
|---|---|---|
| receives trunk frames | UART0–3, DMA into four rings | up to 4 × 8 units × 128 Hz = 4096 frames/s |
| closes a second, applies the rules; writes a segment per address | core A, the SD host | once a second · a segment at 16 kB or 8 s |
| captures PPS-K, reads the label | MCPWM capture on `GPIO47`, HP I²C 0 | every second · every minute |
| brings floors up, runs the control plane | UART0–3, CONTROL frames in the gaps | at boot and on command |
| reads Hermes and the `SHT45` | LP I²C | once a minute |
| sends the modem's values frame | the modem position | once an hour as the heartbeat, and on a marker — which restarts the hour |
| runs the Wi-Fi session | the radio | in windows the site's power allows |
| serves the phone | BLE GATT | while the button's window is open |

## 2. Boot

| # | step | what happens | if it fails |
|---|---|---|---|
| 1 | reset | the four trunk UARTs idle at 38 400, the modem held in reset, the LED off, the BLE radio off; `VBUS` (`GPIO49`) read — **USB power with no 12 V is service mode** (§3) | — |
| 2 | flash | the configuration read from the module's flash: the mirror, the climatology tables, K0 and K1, the uplink settings, the corrections table; each block CRC-checked | a block that fails is replaced by its default and `HEALTH`, `FAULT` up says so; a station with no mirror is fresh |
| 3 | the cards | the SD host brought up on both slots at 1-bit; each unit's last file on each card found, its description binary-searched for the last good entry (`../core/archive/HMC.md`); the archive's last timestamp read — the floor for a seed | a card that fails to mount is reported and the other carries on; both failing is `NO_STORE` — the head still runs and uplinks, the PSRAM buffers the only store |
| 4 | IDs | the four port `ID`s and Hermes's read on ADC1 once, 16× oversampled: 0,15 is a card, 1,00 an empty port, 0,90 Hermes; a port at 0,00 or off-scale is not spoken to | — |
| 5 | Kronos | the time bus's HP I²C 0 up as a multi-master; `ATTN` (`GPIO5`) read once and mirrored; the MCPWM capture armed on `GPIO47`; Kronos's `STATE` and `QUALITY` read (`../kronos/FIRMWARE.md` §8); the corrections table offered — Kronos's copy read, the head's copy written where they differ, the head's being the authority | no Kronos on the bus: `NO_CLOCK`, reported; the head runs, archives nothing stamped, and uplinks the fault |
| 6 | Hermes | the LP I²C up; Hermes's block read once; the thresholds of §11 written; the `SHT45` read | no Hermes: `NO_BMS`, reported; the survival loop is disabled and the head says so in every SOH |
| 7 | the trunks | on each port whose `ID` names a card: `DISCOVER` at 38 400; the card's tag taken; `ASSIGN_ADDR` by tag with **NUMBER = the port number** — port 2 is Bifrost 2 by construction; `BUSCFG` (count 8, rate code 7, rung 2²¹); the trunk UART to 2²¹ | a port whose round closes on no reply — 700 ms of silence, `DISCOVER` once more, 700 ms again — is polled again once a minute and reported `PORT_EMPTY` |
| 8 | the floors | `FLOOR` to each card, in port order, 200 ms apart; the segment tables come up as `kind` 4 within seconds and the station tree is composed (§7) | — |
| 9 | the mirror | the tree compared with the mirror: units that match are confirmed, collisions and strangers renumbered through the card (§7), the roster written on the card's confirmation | — |
| 10 | run | the capture path live; the modem out of reset and a boot marker sent; the Wi-Fi session attempted per its schedule | — |

**No `HEALTH` is pulled before the floors are up.** After a floor's sweep the head sends
`GET HEALTH` to every unit it found; the GO / NO-GO is composed from the answers and the phone
shows it (§10).

## 3. The states

```
   RESET ──▶ BOOT ──▶ RUNNING ◀──▶ SERVICE (the button's window: BLE and the LED on)
                        │
                        ├──▶ SHEDDING (the ordered shutdown, §11) ──▶ SURVIVAL (the LP core alone) ──▶ BOOT
                        │
   USB power, no 12 V ─▶ USB SERVICE (console only, radios off, Hermes and the SHT45 alive)
```

`RUNNING` — everything of §1. `SERVICE` — `RUNNING` plus the BLE radio and the LED, for the
window (§10). `SHEDDING` — the head is putting the units down, floor by floor (§11); capture
continues until the last floor is silent. `SURVIVAL` — the big cores are off, every rail on the
board that a GPIO can idle is idle, and the LP core polls Hermes once a minute and waits for the
pack; the return is a boot. `USB SERVICE` — the board is powered from a phone or a laptop with
the station dead: the console on USB Serial/JTAG, no radio, no trunk traffic; the mirror, the
cards' last state and the archive's tail readable over the console.

## 4. The trunks — the head's side

**Four point-to-point links, one talker per direction.** After boot each trunk runs at 2²¹
8N1; the card sends its records and its own frames up, the head sends CONTROL frames down in
the gaps. Every frame is 48 B (a record — `unix.3 · unix.2 · unix.1` ahead of the spur's 40 B,
five reserve bytes zeroed, one CRC) or 12 B (CONTROL); length tells them apart and nothing else
does (`../core/PROTOCOL.md` §1).

**Reception.** Each UART's DMA fills a 4 kB ring; the idle-line interrupt on core A walks the
ring for whole frames, checks the CRC-16, and hands each good frame to the reassembly of §6
with the port number it arrived on. A frame with a bad CRC is dropped and the head asks the card
`RESEND frame · unix.0` under the unit's address in the next gap; the card answers from its
ring or with a filler. **The head never asks twice for one frame**: a second miss is a hole,
logged as one, and the archive carries the filler.

**The gaps.** A CONTROL frame from the head goes out between the card's frames — the card's
round has a gap after its last slot every period, 7,8 ms of which the head uses a fraction. One
command is open per port at a time; a reply (`ACK`, a `GET` answer, a `TUNNEL_REPLY`, `ERROR`)
closes it; **200 ms** with no reply is a timeout, the command is retried once, and a second
timeout is a line in the head's event log and the roster row's health.

**What the head sends down**, and to whom:

| to | op | when |
|---|---|---|
| a card (TYPE 2) | `FLOOR` · `PORT_PWR` · `PORT_CLK` · `GET HEALTH` · `END` | boot; on the operator's or the survival loop's word; every minute (`HEALTH`) |
| a unit, through the card | `GET` · `SET` · `END` · `HARD_RESET` · `RESEND` · a `kind` 1 `TUNNEL` package to Palatine; **a sonde behind an Argus is addressed with the Argus NUMBER in `TYPE\|NUM` and the sonde in `SUB`**, both read off the station tree | on the mirror's change; on a miss; on the operator's word |
| every unit | `END` as a broadcast, then `PORT_CLK` 0 and `PORT_PWR` 0 per card | the ordered shutdown, §11 |

**The head does not `DISCOVER` a unit, `ASSIGN_ADDR` a unit or `SYNC` anything.** Those are
the card's, on its floor; the head only did them once on the trunk, for the card. A unit's
NUMBER is the card's sweep's, and the head's renumbering goes through the card as an
`ASSIGN_ADDR` by tag the card relays (§7).

## 5. Time on the head

**The PPS-K edge is the head's whole time input, and it is captured in hardware.** The MCPWM
capture on `GPIO47` latches the 80 MHz timer at every edge; nothing in software is in the path.
From the capture the head keeps its own second: the label from Kronos (§8 of its document)
loads it, the edge increments it, and the head's own crystal carries the sub-second between
edges — microseconds a second of drift, which its stamps do not notice: **the head stamps
nothing that reaches the archive**. The 4 B second on every record is the card's, and the head
only checks it.

**The label** — the general-call write on the time bus, once a minute and on a change of
quality — is read by the HP I²C 0 as a slave to address 0x00: eight bytes, CRC-8 checked. Equal
to the head's count: nothing. Off by one: corrected, counted. Off by more, or the quality byte
changed: taken, and the change is a line in the event log and a change in the SOH byte. **No
label for 62 s**: the head's second is `STALE` and every SOH says so. The head also reads
Kronos's `QUALITY`, `PHASE` and `SRC_INFO` registers once a minute for the SOH detail.

**The clock SOH byte** is the head's, derived from Kronos's quality byte: [1:0]
`UNSYNCED` 0 · `HOLDOVER` 1 · `LOCKED` 2 · `SEEDED` 3 · [3:2] leap pending · [7:4] satellite
count. It is a field of the SOH, refreshed once a minute; a change is a line in the event log,
never a record of its own (`../core/PROTOCOL.md` §4).

**Seeding.** If Kronos reads `UNSYNCED` for **10 minutes** after boot, the head writes `SEED` with
the best second it has, in this order: NTP over Wi-Fi or an LTE-M modem's session if one is up, or
the network time an LTE-M modem reports; the archive's last timestamp plus one — a floor, never a
guess from a counter. It writes `POSITION` from its configuration where the station has no receiver.
A later `LOCKED` from Kronos is a change of quality and the head marks the boundary in the archive's
event log.

**The leap offset rides into the archive.** With the label once a minute the head reads Kronos's
`0x65 LEAP` — the GPS − UTC offset, a pending leap and its boundary (`../kronos/FIRMWARE.md`) —
and when any of it changes writes a `TIME` config into every live file from that second
(`../core/archive/HMC.md`), so an exporter turns the archive's leap-blind seconds into UTC from the
file alone.

**A record's second is checked, not trusted.** A record whose 4 B second is more than
**2 s** from the head's own second is archived under the card's second — the card stamped it
and the card is the authority — but the event is counted per card and reported in `HEALTH`; a
card whose seconds run away from the head's by more than 2 s for a minute is `FLOOR`ed again.

## 6. Ingest, the second, and the archive

**The reassembly horizon is T+1 s.** Every good frame lands in the open second's bucket by its
4 B second — a bucket per second, two buckets open at any moment, 4 × 8 × 128 = 4096 slots of
40 B each per bucket, 160 kB a bucket in PSRAM. A second is **closed one second after its
end**: late frames and the answers to `RESEND` fold in until then; after it a repair is a
record with an older second in the unit's next segment, never the live packet
(`../core/PROTOCOL.md` §5). A slot no frame
arrived for by the close is a hole, and the archive carries the card's filler for it, or a
head-made filler with reason *no data arrived* where the card sent nothing at all.

**The selection.** At the close the bucket is walked by address, and each address's frames go
through its **recording rule** — `ALL`, `CHANGE` (on Palatine per ModBus block), `DECIMATE n`,
`INTERVAL s`, `NONZERO` or `NONE`, `CHANGE` and `NONZERO` with an optional *at least every s* —
by comparing bytes, never values. Whatever the rule, a frame whose `kind` or `status` differs from
the last one kept is kept, and so is every report frame. The defaults are per TYPE — `CHANGE` at
least hourly on Palatine, `NONZERO` at least every minute on Tesla, `ALL` on the rest — and the
mirror holds the rule per address, set from the handset or by the server; a change is written into
the unit's file as a config from that second (`../core/archive/HMC.md`). The report frames are the
one kind the head reads, for the leak watch (§8).

**The spool.** **A buffer per address in PSRAM; a buffer becomes one segment when it holds 16 kB or
8 s after its first record**, coded by the shortest codec — HCC (`../core/archive/HCC.md`) or raw —
and written into the unit's live file — the data at the front, its 16 B description entry at the
back, the data first. Files are allocated whole at **1 MB × 2^size** — 64 MB for an address under
`ALL`, 1 MB for every other — and the next opens when the two meet, under the unit's address and the
day: a Quake's file lasts hours, a Palatine's months; the FAT is touched only when a file opens.
Both cards get every segment; a write that fails on one card is reported and that card is marked
until it is swapped; the primary for reads is slot 0 unless slot 0 is the failed one. The buffers
cover a card's wear-levelling stall; a stall longer than 8 s costs the oldest records and counts.
The writer is the HMC/HCC reference library in C, the same code the readers use.

**The rules are the only processing on the way to the card.** What the head derives beside the
archive is only what its own thin frame needs: the alive mask, the event count, the last peak,
and the SOH.

**The SOH is a state, never a stream.** Once a minute the head refreshes it — the clock byte
(§5), the `HEALTH` counters of the unit whose turn it is (§8), Hermes's block and the `SHT45`
(§11), the `NO_BMS` / `NO_CLOCK` / `NO_STORE` flags — and holds it in RAM for the phone's status
document, the console and the heartbeat's flags byte. It is not written to the archive and it
does not go over the modem; what is merely OK is shown, not sent.

**The event log is what changes, and it is a record.** A line is written when something
happens — a change of the clock's quality, a `FAULT` edge with the `HEALTH` the head pulled on it, a SUPPLY
edge with its value, a change of Hermes's fault bits, a card reboot, a MAC failure, a shed, a
wake — and never on a schedule. Each line goes into the archive as one record of the series of
address 0: **the head's second · the source address (0 for the head, the card's or the unit's
otherwise, Hermes as its `ID`) · a code · a detail · a 32-bit value**, 12 B, so it travels to the
server with the archive and is searched like any record. The last 4096 lines are kept in
the module's flash as well (§14), the tail that survives a reboot and what the console shows.
A line whose code is in the **marker set** — a strong ALARM, a `FAULT` whose `HEALTH` is in the set, a SUPPLY
edge, `NO_CLOCK`, `NO_STORE`, `NO_BMS`, a shed, a wake — also goes out on the modem at once (§9).

**Loss of the 12 V.** The backup cell takes the rail, the rail staying above 3,1 V, and `PG` on `GPIO52` raises
the highest interrupt on core A: the Wi-Fi off, the charge path (`GPIO51`) off, every buffer
written to both cards. **Only then is the modem keyed** — the cards' writes and the modem's
bursts never draw on the cell together, which keeps it under the eFuse's breaker
(`HARDWARE.md`, *Backup cell*), and a head that browns out at a burst in the cold has lost the
report, never the data. Core B reads Hermes for the cause, the head sends it on the modem with the charge the cell has given so far — `ILM` on `GPIO53`, 0,604 V/A,
sampled from the interrupt on and summed — and releases the hold on `GPIO50` — the board goes dark and boots again when
the 12 V returns (§11). If the 12 V comes back before the release, `PG` falls and the head
carries on. A short on the rail while on the cell trips the eFuse, which stays off until the
hold is released; the cut in its `EN` divider turns it off at a rail of 2,82–2,92 V if the head
never releases it. The brownout detector on the 3,3 V stays as the last guard.

**On the cell, the silence of everything else is expected.** The cell carries the head, Hermes and
the modem and nothing more, so Kronos, the cards and every unit go dark with the 12 V. **From
`PG` on, nothing they fail to do is a fault**: no `NO_CLOCK`, no `FAULT`, no lost-unit line, no
retry, no recovery ladder and no restart; the clock interrupts and the trunks' receivers are
masked. The head writes **one** line — the 12 V lost, the cause Hermes gives — flushes, sends, and
lets go. The same holds wherever a Mayak runs: in a full Heimdall, on Mimir and on Proteus.

## 7. The station tree, the roster and the numbers

**The tree is composed from what the cards report.** After each `FLOOR` the card sends its
segment table as a `kind` 4 payload — 8 × (unit address · port · rung · status) — and every
Argus in the tree sends its own for its segments; the head composes them: port → card → slot →
unit → (a carrier's) table → position → sonde. The tree is what the head decodes by, and the
roster and the mirror are written from it, never typed (`../core/PROTOCOL.md` §10).

**The head owns the numbers.** A card's sweep sends up each unit's TYPE|NUM as the unit said it
and its tag. The head, which alone sees the whole station:

| what it finds | what it does |
|---|---|
| a tag it knows, on the number the mirror holds | confirmed, nothing sent |
| a tag it knows, on another number — a unit moved between ports | `ASSIGN_ADDR` by tag through the card, back to the mirror's number; the roster row's port updated on the card's confirmation |
| two units of one TYPE on one NUMBER — a collision, one brought from another station | the newer tag renumbered to the lowest free NUMBER of its type; the event log *renumbered* |
| a tag it does not know, on a default NUMBER — a fresh unit | given the lowest free NUMBER of its type; a new roster row; the phone shows it as new |
| a tag it does not know, on a non-default NUMBER | a unit from another station: renumbered as a collision would be, and the phone is told |
| a roster row whose tag did not answer any sweep | *missing*: the row kept, the phone asked *replaced?*; a fresh unit of the same TYPE answering `yes` takes the old number and the series continues |

**The mirror** is the station's configuration in one few-kB table. The roster — who is enrolled,
TYPE + NUMBER → port and slot — is its spine, and per-unit configuration hangs off it. A change
travels as a command to the unit and is **written to the mirror only when the unit confirms it**;
the phone reads the mirror and never crawls the station.

**The layout is fixed, per unit, identical for every unit.** Each unit's slot holds its NUMBER,
port and slot, the persisted registers the head has written to it, and its delay and
synchronisation block, laid out the same way for all — so an address and a field's place never
move across a reset, a re-enrol or a fresh boot; only the values do. It is the node's fixed flash
cells (`../core/PROTOCOL.md` §7) one level up, at station scope. **The measured tables ride the
same slot as read-only rows**: the segment tables the cards report after `FLOOR`, the ranged
routes, the rungs, the units' `DELAY`s, the Argus positions, the Palatine roster and arm table. So
the phone shows in one place what a unit is set to and what the sweep measured about it — nothing
crawled, nothing typed, and the firmware hard-codes only the dictionary.

**A value has three states, and the phone shows all three**: *edited* locally and not yet sent ·
*pending* — sent to the unit, awaiting its confirmation · *confirmed* — the unit's `ACK` through the
card came back and the mirror committed. A value never reads done the moment it is saved. A direct
write to a unit would tunnel, wait and confirm exactly the same, so the mirror costs nothing and
adds the audit point.

**The mirror is also the firewall.** The phone, the server and the uplink talk to the head and stop
there; nobody outside the station addresses a unit directly.

**The climatology tables sit beside the mirror, not in it.** The country's normality boundaries
(`../palatine/CLIMATE.md`) are the largest thing the head stores and the only one that describes
where the station stands rather than what it is made of: no unit confirms them, nothing travels as
a command, and they are replaced wholesale when a country or a normal period changes.

## 8. The control plane — what the head asks, and what it does with the answers

| the head sends | to | with | on |
|---|---|---|---|
| `GET HEALTH` | every card, every unit in turn | — | once a minute, one unit per gap, round-robin; the counters land in the SOH |
| `GET` a register | a unit | the register the phone or the server asked for | on request |
| `SET` a register | a unit | the mirror's edited value | on a mirror change; `ACK` confirms it |
| `TUNNEL` | Palatine | a whole RTU package — Pluvius's `DRAIN`, a sensor's offsets, a provisioning write | on the phone's or the server's word; one open package per Palatine; the reply validated by its RTU CRC here, retried once |
| `END` | a unit, or every unit | — | the operator; the ordered shutdown |
| `HARD_RESET` | a unit | — | the operator, through the phone, after the ladder's step 5 |
| `PORT_PWR` · `PORT_CLK` | a card | port, on/off | the operator; the survival loop (`PORT_PWR 0 off` on every card is the shedding's third step) |
| `FLOOR` | a card | — | boot; a card that reset (its table arriving after silence); a card whose seconds ran away |
| `RANGE` on the corrections page | Kronos's register | — | commissioning; the phone's *range now* |
| `GET 0xFF50 + slot` | a card | the slot whose route-change count moved in the card's `HEALTH` | on that count, and once a day; the route is shown on the phone as diagnostics — the head applies nothing, the unit already placed its grid on it |
| `SET 0xFF70 + port` · `0xFF80 + port` | a card | `RANGE_INTERVAL` · `RANGE_WIDTH` | the operator, from the mirror; the defaults need no write |
| `SET 0x0019 UNIX` | a mode-B Palatine | the UTC second of a named frame — the station's second less the GPS − UTC offset of Kronos's `LEAP` | after the Palatine's `SYNC`, once a day and after a leap second (`../palatine/WMO.md`) |

**What comes up unasked, and where it goes:** a `FAULT` edge on a card's or a unit's frames → one
`GET HEALTH`, the answer to the event log, the SOH, the roster row's health and the phone's status
document, a marker on the modem's frame if the entry is in the marker set, and *route changed* also
triggers the route read above; **the ALARM flag on a
unit's frame → the event board per the policy** (`EVENT` where the policy says so, the marker
pipeline, one window looked at — below) — the head watches flags, never values; **the SUPPLY
flag → one `GET REPORT` and a line in the event log with the value, once on the edge in and once
on the edge out** — the supply's value is the report frames' and never a stream in the
SOH, the unit's `0xFF0A VIN_WINDOW` is where the thresholds live and the mirror edits them; **a
`kind` 6 `REPORT` or 7 `PORTS` frame → the report tables**: written to the archive as the slow
channel under the sender's TYPE and NUMBER, an Argus's `REPORT` payload split by the segment
table's positions, a host's `PORTS` read in the host's map, and **the 48 V leak watch computed
here** — a port's `CURRENT` from the host's `PORTS` against the unit's from its `REPORT`, both
raw and converted by the boards' `ID`s, the difference against the run's threshold into the
event log; a 300 V port's leak word against the baseline written at commissioning, a drift past
the threshold into the event log and to the phone (`../galvani/HARDWARE.md`, *Leak watch*); **a
`kind` 8 `HEALTH` frame** comes only as the answer to a `GET HEALTH` — the minute's round, a `FAULT`
edge, the phone's — and goes where that asking says, never to the archive as a frame of its own; **Kronos's faults arrive over the
time bus's I²C**, not as a frame, and go to the SOH; a filler → the archive and the alive mask; a `kind` 4
table → the tree. **`ACK` closes the open command and moves the mirror's value to confirmed.**

**The command channel from outside** — the phone's writes and the server's group commands and
window requests — is authenticated: an HMAC over the command with K0‖K1, the nonce the 3-byte
station identity and the station's **32-bit command counter** — monotonic, persisted in the
configuration cells (§14), reported in `HELLO` and as a register over BLE; a command whose
counter is not above the stored one is dropped before the MAC is checked, a failed MAC is logged
and dropped (`../core/PROTOCOL.md` §6). **`SET K1` alone is MAC'd with K0** — K0 roots the
re-key; K0 itself was made by the TRNG at the first boot, lives in the encrypted flash behind
the Key Manager, left the board once over the service USB into the operator's key store, and
is never sent anywhere.

## 9. The uplink

**The modem — the thin backbone, outbound.** The module in the modem position sends the 16 B values
frame: station id, the second, pack voltage, board temperature, the event count since the last
frame, the alive mask, the flags — the clock SOH and the shed/survival reason among them — the last
peak, CRC-16. One frame serves both uses, and it is not made shorter: 16 B is one LoRa packet whose
preamble and header outweigh it, and a second format would buy nothing. **Once an hour as the
heartbeat**, and **immediately on a marker** — a line of the event log whose code is in the marker
set (§6): a `strong` event from a unit's ALARM flag with the peak of §12, a card's `FAULT` of the
set, a SUPPLY edge, `NO_CLOCK`, `NO_STORE`, `NO_BMS`, a shed or a wake — rate-limited to one marker
a minute so aftershocks do not flood. **Every marker restarts the hour**: it carries the same
fields, so the next heartbeat is due an hour after the last frame of either kind. Nothing that is
merely OK is sent; the heartbeat exists because silence cannot be told from death, and the pack and
the alive mask in it are the two numbers that predict a fault before it is one. Weak events sum into
the heartbeat's count. **What comes back depends on the modem.** On LoRa and a satellite module the
one downlink is a resend request for a frame the server missed, answered from the last 16 frames
kept, and nothing else is accepted on that radio. **An LTE-M / NB-IoT modem runs the session of
Wi-Fi below** — `HELLO`, the `CURSOR`, the command channel under its MAC, the live board — within
its tariff's volume. Duty cycle is a configured cap per hour and the summary cadence stretches to
meet it.

**Wi-Fi — the rich channel, when the site has it.** In a configured window the head joins the site's
network, takes NTP once, and runs the session of `../core/PROTOCOL.md` §6 — `HELLO` MAC'd with K0‖K1
carrying the command counter, every archive packet with its 8 B truncated HMAC chained to the
session, the live packet and the values frames bare: `HELLO` with the station identity, the server's
`CURSOR` per address — the file and the last description entry it holds — and the head sends
**archive packets** forward from each, cumulative `ACK`s trimming what it may forget — it forgets
nothing; the cards are the store. **Bounded windows on demand**: the server asks `(ts, ±window)` and
the head cuts the segments that touch it out of the files.

**The live packet** goes on every link given a cadence — Wi-Fi, cellular, a satellite module on a
freed trunk UART: what each address's **send rule** for that link selects from what was recorded
since the last live packet, coded by HCC, one segment per address (`../core/archive/HMC.md`, *The
uplink*). It is the server's live view and never its archive. A station with no Wi-Fi sends one live
packet an hour over the modem — a mode-B Palatine's `HOUR` row, and its `DAY` row once a day
(`../palatine/WMO.md`), 32 B each, every other address `NONE` — and keeps the rest on its cards
until they are read. **On the modem the live packet is a frame of its own beside the 16 B values
frame**, sent after the hour's heartbeat and counted in the same duty-cycle cap. **The live board
comes back on the same session**: a head on a link that carries it subscribes at HELLO and receives
every other head's live packets and markers as the server fans them out; it writes them to the
archive as a received stream and to the handset's live view, and **acts on nothing in them** — a
command arrives on the authenticated channel and nowhere else. **Nothing is encrypted.** OTA is
head-only and lands in the second app partition, verified by the secure-boot signature before the
switch, the old image kept for a rollback on a failed boot. The session ends at the window's end or
when the pack says so (§11); the archive is complete either way.

**Interop is the server's** — the exporters of `../core/archive/EXPORTERS.md` are readers of the
archive (`../core/INTEROP.md`); the head emits HMC and nothing else.

## 10. The service window — the phone, the button, the LED, the console

**The phone is the screen, over BLE.** BLE and not classic Bluetooth: it works on Android and
iOS alike, it is low-power, and the S31 carries BLE 5.4 and no classic radio. There is no IP stack
on this link, no access point and no captive portal — pair, read, write; the app owns the whole
UI and the head stays a thin serializer. Wi-Fi keeps the data-home job, the archive, NTP and OTA,
which BLE cannot carry, and the two coexist. **Where there is no phone, or no firmware, the USB
console is the way in** (below).

**The button (`GPIO0`) opens the window: BLE advertising and the LED for 5 minutes**, extended
while a phone is connected, closed on the button again or the timeout; in normal running BLE
is off and the LED gives the station-wide blink. The GATT service is `handset/README.md`'s: one *status* characteristic, notify, a
newline-delimited JSON document reassembled across notifications; one *command* characteristic,
write, a newline-terminated JSON command. The status document is the head's HEALTH/SOH aggregate
— `go`, the nodes with TYPE, NUMBER, `bus` and each sensor's health — plus the corrections table
and the mirror's pending values; it is pushed on connect, on every change, and on `refresh`.
The commands: `refresh` · `recommission` (every floor run again, the sweep results put to the
human as *new* / *missing* / *replaced?*) · `provision node` (a Palatine learning session, §6 of
its document, driven by the phone one sensor at a time) · `corr set` (a typed term written to
the corrections table, Kronos's copy updated; a ranged term refused) · a mirror edit. **Every
write is MAC'd as in §8**; the phone holds K1 and never K0 — it receives a station's K1 from the
operator's server, for the stations the technician is assigned and for a bounded time, and loads
it before leaving where it will work offline; it reads the station's command counter over BLE
before its first write.

**GO / NO-GO** is computed here: GO when every roster row is present and every fitted sensor
reads OK, and the corrections table has no missing `route` on a remoted receiver
(`handset/README.md`); anything else is NO-GO with the offending names.

**USB Serial/JTAG** carries a console in every state — a line-oriented command set that reads
the mirror, the tree, the event log, the archive's tail, and Hermes's block, and in `USB
SERVICE` is the only thing alive. `EFUSE_DIS_USB_JTAG` is never burnt.

## 11. Power — Hermes, the coulomb count, the ordered shutdown, survival

**Once a minute** core B reads Hermes's block over the LP I²C — pack voltage and current, SoC
as the BMS reports it, the cell temperatures, the BMS status and faults, the PV input and the
charge state (`../hermes/FIRMWARE.md` §6) — and the `SHT45`. The reading goes into the SOH, the
event log on a change of fault bits, and the modem's summary. **The capacity is the head's, not
the BMS's**: the head integrates the pack current over the minute readings into its own
coulomb estimate, re-anchored at full charge (the MPPT's float state), and that estimate is the
SoC the decisions below use; the BMS's own SoC rides beside it for the record.

**The thresholds are the head's configuration**, written into Hermes at boot so `ALERT` fires
between polls (§6 of its document): pack voltage low and high, cell temperature low and high,
PV lost, SoC low. `ALERT` on `GPIO1` wakes core B to read the block at once.

**The ordered shutdown.** When the head's SoC estimate falls under the **shed threshold**
(configured; 20 % the default, the station's lockdown — `../core/POWER.md` §3) or the pack voltage under its low threshold:

1. a marker goes out on the modem with the reason — *pack depleted* or *low irradiance* — before
   anything is switched;
2. `END` to every unit, floor by floor, the highest-drawing floors first as the mirror's
   measured loads rank them; the head waits **10 s** per floor for the last frames and the
   `ACK`s;
3. `PORT_CLK 0 drop` then `PORT_PWR 0 off` on each card — the clock first, the feed second
   (`../core/PROTOCOL.md` §7);
4. every buffer written to both cards, the session closed if one is up, the Wi-Fi off;
5. `END` to each card, which flushes its ring up the trunk and goes into its **deep sleep** —
   a card sits on the enclosure's battery wire and has no `ENABLE` above it, so software is the
   only off it has, and it wakes on the next start bit on the trunk (`../core/PROTOCOL.md` §7);
6. `GATE 0` to Kronos, then Kronos down the same way — **nothing is being timed, so nothing
   needs the timebase**; it wakes on an address match on the time bus I²C;
7. the head enters `SURVIVAL`. **What is left running is this core and Hermes, and nothing
   else** (`../core/POWER.md`, *The lockdown*).

**Survival runs on the LP core alone.** The big cores are powered down; the LP core reads
Hermes's block over the LP I²C once a minute, keeps a coarse SoC by the same integration in
LP SRAM, sends nothing, and waits for the **wake threshold** (configured; 50 % the default, and
the pack voltage above its recovery level for 5 minutes). Then it resets the head and the boot
of §2 brings the station back: `GATE 1` to Kronos first, the cards fed and floored in order.
**The BMS's own drop is the unordered case below all of this**: it is a hard reset of every
board, and every unit's configuration survives it in its own store. The head alone rides it on
its backup cell long enough to write every buffer and send the cause on the modem (§6), then switches
itself off, and boots with the rest when the 12 V returns.

**The backup cell's charge** follows the `SHT45`: `GPIO51` is off below −20 °C and on again above
−17 °C — the low-temperature LiFePO4's charge limit, or the chosen cell's own (`HARDWARE.md`).

**Pluvius's pump and every other declared load** sit behind a host's port and go with its floor;
nothing on the head switches a load.

## 12. Event assessment

**On an alarm or an EVENT marker the head looks at one window, never the stream.** The marker
names the second; the head pulls **±30 s** around it from the archive into PSRAM, runs one
bounded pass — a peak and a duration on a seismic channel, a count on a sferic channel — and
packs the verdict into the values frame's `peak` and `flags` for the marker that goes out. The
server stays the detector; the head's number is a headline. The pass is bounded at 200 ms of
core B and is dropped, with the marker still sent, if the window is not in the archive yet.

## 13. Faults

| trigger | action | reported |
|---|---|---|
| a trunk silent for 2 s while its card was up | `GET HEALTH` to the card; no reply in 200 ms: the port is re-floored — `DISCOVER` at 38 400, and the whole of §2 step 7–8 for that port | *trunk n down* in the event log, a marker |
| a card's table arriving after its silence — it rebooted | `FLOOR` on that port; the mirror's rows for that floor go *pending* until the tables come back | event log |
| a card's table change, or its `FAULT` | the roster row's health from the slot's status and the `HEALTH` pulled; the ladder is the card's, the head only records; after step 5 the phone shows *dead* | event log, SOH |
| a record with a CRC miss | `RESEND` once; a second miss is a hole | `HEALTH` per port |
| the head's second and a card's disagree by > 2 s for a minute | the port re-floored | event log |
| no label for 62 s | `STALE` in the SOH; Kronos's `STATE` read for the reason | SOH |
| Kronos silent on the time bus for 5 min | `NO_CLOCK`; the archive continues under the cards' seconds, the SOH says the head cannot check them | a marker |
| a card write fails | the card marked, the other carries on; both failed: `NO_STORE`, the PSRAM buffers the only store, the uplink told | a marker |
| the buffers overflow (a card stalled past 8 s) | the oldest records dropped, counted | `HEALTH` |
| a MAC fails on a command | dropped | event log, counted; ten in a minute is a marker |
| Hermes silent for 5 min | `NO_BMS`; the shutdown falls back to the pack-voltage threshold read nowhere — so it is disabled, and the SOH says the station will go down unordered | a marker |
| brownout | the flush of §6 | — |
| the task watchdog on either core | a reset; the boot of §2 | `HEALTH` boots |

## 14. Persistence

In the module's flash, each block CRC-framed, written in two copies alternately so a torn write
leaves the previous one:

| block | content | written |
|---|---|---|
| the mirror | the roster and per-unit slots, fixed layout | on a confirmation |
| the corrections table | the typed terms; the ranged ones as last reported | on a `corr set`; on a report |
| the climatology tables | the country's boundaries | wholesale, from the phone or the server |
| the keys | K0 (made by the TRNG at the first boot, read out once at provisioning, never again), K1 | K1 on a re-key |
| the command counter | 32 bits, monotonic | on every accepted command |
| the uplink | the Wi-Fi credentials and window, the modem's parameters and duty cap, the server's identity | from the phone |
| the power | the shed and wake thresholds, Hermes's alert thresholds | from the phone or the server |
| the event log's tail | the last 4096 lines, a ring — the reboot-surviving copy of what the archive holds in full (§6) | as they happen |
| the identity | the 3-byte station identity, the archive header's `STATION` section | at manufacture |

**No time is stored.** The archive's last timestamp is the seed's floor and nothing more. The
archive itself is the two cards.

## 15. The processor's budget

| resource | used | of |
|---|---|---|
| UARTs | 4 — the trunks | 4 HP + 1 LP |
| I²C | 2 — HP I²C 0 the time bus, LP I²C Hermes and the `SHT45` | 2 HP + 1 LP |
| SPI | 1 — SPI2 the modem position | 2 |
| SD host | 2 slots, 1-bit | 1 host, 2 slots |
| GDMA | 8 — the four trunks' RX and TX; the SD's own; the crypto engines on the spare pair | 5 + 5 AHB, 3 + 3 AXI |
| core A | the four idle-line handlers, the reassembly, the transcode, the spool — ~15 % at full traffic | one RISC-V core |
| core B | the Wi-Fi and BLE stacks, the modem, Hermes, the mirror, the control plane, the assessment | one RISC-V core |
| the LP core | the survival loop, 40 MHz, under 100 µs a minute | |
| PSRAM | two buckets 2 × 160 kB · the ring 8 s of traffic, 1,6 MB · the assessment window 1 MB · the Wi-Fi stack | 16 MB |
| flash | the two app partitions · the blocks of §14 | 16 MB |

**No interrupt sits in a timing path.** The PPS-K edge is a hardware capture; a record's
time is the card's stamp; a late handler moves nothing but the moment a block is written.

## 16. What is tested, and where

**On a PC, with the pins replaced by callbacks:** the trunk parser against recorded traffic —
a 48 B record cut by a stall, a 12 B `ERROR` between two records, a CRC miss and its
`RESEND`; the reassembly with frames arriving 0,9 s and 1,1 s late; every recording rule against
recorded traffic, its edges kept; the spool's segments, entries and file change against the HMC
reader, and every HCC stream round-tripped; the boot's binary search on a file cut mid-segment; the tree composed from three cards' tables and an
Argus's; the renumbering cases of §7 one by one; the mirror's three states; the MAC with a
replayed nonce; the values frame's fields and the rate limit; the session's resume from a
`CURSOR` in the middle of a file; the coulomb estimate against a recorded week of Hermes
blocks; the ordered shutdown's sequence and the survival loop's wake; the GO / NO-GO from a
recorded set of `HEALTH` frames; the console's command set.

**On the bench, against `HARDWARE.md`:** four trunks at 2²¹ carrying 32 units at 128 Hz for
24 h with zero holes; the PPS-K capture within ±1 µs of Kronos's edge on a counter; both
cards written for 24 h at full traffic and the archive read back whole; a card pulled
mid-write and the other carrying on; the BLE window opening on the button and closing at
5 min; a phone commissioning a fresh station without a laptop; the shutdown sequence on a bench
pack run down, and the station back on its own when charged; the head running from a phone's
OTG with the 12 V absent.

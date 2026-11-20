★ N.I.C. ★

# Njord — the link

> **Design-stage concept — nothing built.** The figures are verified against the parts' sheets and
> the parts before a board is made.

**Every Njord unit speaks one link: LoRa on an SX1262 at 868 MHz, our own packet on raw LoRa.** The
receiver is Iris at a station or Huginn at a window; every unit runs on a LoRaWAN network as well,
public or a site's own, its packet the payload (§7).

## 1 The air

| | near | far |
|---|---|---|
| band (ERC 70-03) | **h1.8**, 869,7–870 MHz | **h1.7**, 869,40–869,65 MHz |
| power | 5 mW e.r.p. | 500 mW e.r.p.; the module gives 22 dBm into its antenna, Ratatoskr's 2,4 V rail ~19 dBm |
| time on air | no limit | 10 % |
| bandwidth, coding | 125 kHz, CR 4/5, preamble 8, CRC on | the same |

**The band, the spreading factor and the power are a unit's settings**, not its board: one module and
one antenna serve both. The receiver listens on one channel and one spreading factor at a time and
tunes to each unit's own in that unit's slot (§5).

**The reach** (`calc/link.py`: the SX1262's sensitivity, −124 dBm at SF7 and −137 at SF12 in its
sheet's table 3-8; the two-ray ground model a unit a metre over the grass sees; 10 dB of margin; the
trees bracketed), the receiver's antenna at 8 m:

| | open ground | 15 dB of trees | 30 dB |
|---|---|---|---|
| h1.8, 5 mW, SF7 | 3,4 km | 1,5 km | 0,6 km |
| h1.8, 5 mW, SF10 | 5,3 km | 2,3 km | 1,0 km |
| h1.7, 22 dBm, SF7 | 8,1 km | 3,4 km | 1,4 km |
| h1.7, 22 dBm, SF10 | 12,5 km | 5,3 km | 2,2 km |

The receiver's antenna height is worth most: at 3 m the reach is two thirds of these.

## 2 The cadence

**A unit measures and sends every ten minutes; each packet is acknowledged.** No acknowledgement:
one retry after a random few seconds, then the packet is kept in a ring and sent again after the next
one. A ten-minute value is a sample, not a mean; a pulse sensor — cups, Imber's tips — is counted
between the wakes.

## 3 The packets

**Every value is a house-map register** (`../core/blocks/modbus.md`): sixteen bits at the type's
scale — 0,1 °C, 0,1 %, 0,1 hPa — `0x8000` for absent, **big-endian**, so the receiver serves on
ModBus the very bytes that flew. Raw LoRa has no address: **the first byte is the unit's number**,
1–255 under its receiver.

**Ratatoskr → receiver, 27 B**

| byte | field | size | |
|---|---|---|---|
| 0 | the unit's number | 1 | 1–255 |
| 1 | kind and flags | 1 | data or a resend from the ring; a setting applied; a weak cell |
| 2–3 | counter | 2 | the low half of the unit's 32-bit sequence: a replay refused, a resend and a duplicate known |
| 4–5 | age | 2 | how long before this packet the unit measured, in 10 ms |
| 6–7 | T at 2 m | 2 | 0,1 °C |
| 8–9 | RH at 2 m | 2 | 0,1 % |
| 10–11 | T at 5 cm | 2 | 0,1 °C |
| 12–13 | RH at 5 cm | 2 | 0,1 % |
| 14–15 | pressure | 2 | 0,1 hPa |
| 16–17 | the cell | 2 | mV |
| 18 | the cell's temperature | 1 | °C, signed |
| 19–26 | tag | 8 | §4 |

Imber, where fitted, adds its tip count as one more register, 29 B.

**Arke → receiver, at most 51 B a packet** — EU 868's ceiling at SF10–12, kept at every SF so one
table serves; a wake with more sends several:

| byte | field | size | |
|---|---|---|---|
| 0 | the unit's number | 1 | |
| 1 | kind and flags | 1 | |
| 2–3 | counter | 2 | one for all the wake's packets |
| 4–5 | age | 2 | 10 ms |
| 6 | part | 1 | high nibble this packet's index, low nibble how many |
| 7 … | blocks | ≤ 36 | Palatine's shape (`../core/PROTOCOL.md` §5): address · length · registers, ≤ 12 registers a block |
| last 8 | tag | 8 | |

Arke's sensors keep their own ModBus addresses, unique across the station; its BMS and MPPT ride as
their blocks.

**The acknowledgement, receiver → unit, 14 B, 17 B with a setting**

| byte | field | size | |
|---|---|---|---|
| 0 | the unit's number | 1 | |
| 1–2 | counter | 2 | the packet acknowledged |
| 3 | flags and parts | 1 | bit 7: a setting follows; Arke: a bit a part received |
| 4–5 | the next wake | 2 | ms to the unit's next measurement, on the receiver's crystal (§5) |
| (6) | the setting's id | 1 | when bit 7 |
| (7–8) | its value | 2 | when bit 7 |
| 6–13 / 9–16 | tag | 8 | |

## 4 The tag and the keys

**The tag is the station's own:** HMAC-SHA-256 under the unit's key, truncated to its first 8 B,
closing every packet and every acknowledgement over every byte before it (`../core/PROTOCOL.md` §6)
**and over the counter's high half**, which does not fly: both ends keep the 32-bit count, the
receiver taking the high half from the last packet it accepted, so the 16 bits on the air never wrap
into a replay (LoRaWAN's frame counter is kept so). A field unit's packet is written into the
station's archive, and the archive admits no stranger's writing. Nothing is encrypted. **The key**,
16 B as the station's own pair, is written into the unit at the bench, and into its receiver from
the handset at the station, wrapped under the station's K0 as a new K1 is (`../core/PROTOCOL.md` §6)
— Iris — or on its own set-up page — Huginn; never in clear on any air.

## 5 The slots and the time

**The receiver keeps the beat.** Each acknowledgement gives the milliseconds to the unit's next wake,
counted on the receiver's 32,768 kHz crystal, so every unit measures at the same instant — an
orchard's temperatures taken together — and sends in a slot of its own by its number, 600 s divided
among the units, so packets do not meet. Thirty-two units take ~0,3 % of the air at SF7 each way.

**The time of a value is stamped where it arrives.** Iris has no clock of the station — a ModBus arm
carries none — so its window gives the record's age when Palatine reads it, and Palatine's frame
gives the second. Huginn stamps the arrival from NTP. Either way the time measured is the stamp less
the ages: tens of milliseconds, enough for slow quantities. Arke carries a GNSS and stamps its own.

## 6 Back to the unit

**A value or two may be set from afar**, in the acknowledgement, signed: the head's command through
the tunnel it already has (`../core/proto/nic_tunnel.c`) into the unit's window at Iris, sent with the
next acknowledgement, the unit's next packet saying it applied it. A value that touches the link —
the band, the power, the cadence — is tried and falls back if the acknowledgements stop for a few
cadences, as `APPLY`'s trial boot does. **Never the register set, never the firmware** over a public
band. A unit's boot log is asked for the same way.

## 7 On LoRaWAN

**Every unit also runs on a LoRaWAN network** — a public one (The Things Network and its like) or a
site's own gateway and network server — its link chosen at the bench: Njord's own to Iris or Huginn,
or LoRaWAN. **The packet is the same one**, the payload on port 1, decoded on the server by the
unit's profile; the station's tag stays inside it, so a record that reaches the archive this way is
signed by the unit as one through Iris is. **The stack** is Semtech's LoRaMac-node (Revised BSD) on
the same SX1262: EU868, class A, joined over the air (OTAA) with its DevEUI, JoinEUI and AppKey
written at the bench beside the unit's key; the network's own encryption and MIC under its session
keys come on top.

**The air is the network's**: its three default channels, 868,1–868,5 MHz in ERC 70-03's h1.4, at
25 mW e.r.p. and 1 % time on air. **A public network's fair use** allows some 30 s of uplink a day
and some ten downlinks, so there a unit **measures every ten minutes and sends hourly, unacknowledged**:

| byte | field | size |
|---|---|---|
| 0–5 | the unit's number · kind and flags · counter · the age of the first sample | 6 |
| 6–65 | six sets of the five values, ten minutes apart | 60 |
| 66–68 | the cell, mV · its temperature, °C | 3 |
| 69–76 | tag | 8 |

**77 B, with LoRaWAN's 13 B 90 B on the air: 159 ms at SF7, 3,8 s a day; 513 ms at SF9, 12,3 s a
day** (`calc/power.py`). Above SF9 the network's ceiling is 51 B — two packets of three samples, 47 B
each, half-hourly — and SF10 already spends 33 s a day: **a unit that needs SF10 or slower belongs
on Iris, on Huginn or on a site's own network**, where only the band's 1 % binds. With Imber fitted
each set is six values, 89 B. **An Arke** sends hourly, each sensor's latest reading in its blocks,
51 B a packet, 118 ms at SF7 — some ten packets an hour within fair use; more sensors than that is
Iris's or Huginn's work.

**The gaps:** a downlink every few hours carries the last counter the server holds and a bitmap of
the hours missing; the unit resends them from its ring. **The time:** the unit keeps its ten-minute
marks on its own crystal, set once a day by LoRaWAN's `DeviceTimeReq` (1.0.3) — the network's GPS
time, to within a tenth of a second — and the server stamps nothing but the arrival; the age gives the time
measured, as at Iris. **Settings from afar** ride a downlink the same way, signed under the unit's
key as an acknowledgement's setting is, one at a time and a link setting on trial (§6); the
firmware and the register set never.

## 8 Capacity

The acknowledgements are the receiver's air: 14 B is 46 ms at SF7, so h1.7's 10 % holds ~7 800 an
hour — ~1 300 units at six an hour, ~7 800 at one; ~360 and ~2 200 at SF9. **Carrying data onward
over LoRa is not done**: the radio is the last hop, the way on is the internet or the station.

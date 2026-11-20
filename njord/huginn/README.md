★ N.I.C. ★

# Huginn — Njord's gateway to the internet

**English** · [Čeština](README.cs.md) · [Русский](README.ru.md)

> **Design-stage concept — nothing built.** The figures are verified against the parts' sheets and
> the parts before a board is made.

**Huginn receives Njord's units where no station stands and sends their data on over the internet** —
a small unit with LoRa and Wi-Fi at a window or on a balcony, on a USB supply. To the units it is a
receiver like Iris: it acknowledges, keeps the beat of their slots, carries a setting back; to the
internet it is a client that posts each unit's record in the field unit's format to the server it is
set to. A LoRaWAN gateway does the same with a network server behind it; Huginn needs none.

| | |
|---|---|
| board | an ESP32-S3 with a Wio-SX1262 (Seeed's XIAO ESP32-S3 & Wio-SX1262 kit, bought) |
| radio | one channel and one spreading factor at a time, retuned in each unit's slot — Njord's own packet needs no eight-channel LoRaWAN concentrator |
| uplink | Wi-Fi; the record posted to the server set on its page |
| time | NTP over its Wi-Fi — milliseconds to tens — stamping each packet's arrival; the unit's age gives the time measured |
| set up | its own web page on its own WPA2 access point, open only for setting up: the network, the server, the units' keys and their slots |
| feed | 5 V USB |
| link | [`../LINK.md`](../LINK.md) |

**One Huginn holds many units:** the acknowledgements are its air, ~7 800 an hour at SF7 within
h1.7's 10 %. Reach sets the count, not the air: a unit a metre over the grass reaches a Huginn at a
window some kilometres in the open, less through trees and walls (`LINK.md` §1).

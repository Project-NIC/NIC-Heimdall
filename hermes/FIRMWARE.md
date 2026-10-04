★ N.I.C. ★

# Hermes — the firmware, described

> **Design-stage concept.** The figures are verified against the parts' sheets and the parts
> before the build is written. What the card is: [`README.md`](README.md);
> the board, the pins and the faces: [`HARDWARE.md`](HARDWARE.md); the head's side of the link
> and the survival loop: [`../mayak/FIRMWARE.md`](../mayak/FIRMWARE.md) §11; the power
> doctrine: [`../core/POWER.md`](../core/POWER.md). Where this document and one of those differ,
> that one wins.

## 1. What the firmware is

**A poller with a block.** The firmware finds the BMS and the MPPT on whichever of its four
faces they answer, polls each on the map the builder filled for it, at its own cadence, folds the answers into
one 32 B register block, and holds it for the head to read over the LP I²C. It raises `ALERT`
when a value crosses a threshold the head wrote. It measures nothing, decides nothing, and is
never asleep — the head in survival reads this block and nothing else, so the block is always
current.

**One profile of the `nic-mod` engine with two faces**: a Modbus master toward the power boards,
an I²C slave toward the head. The house block a MOD carries on ModBus is here on the I²C, the same
registers in its one-byte map (§6).

| the firmware does | on | how often |
|---|---|---|
| polls the MPPT | USART1 (485) | every **10 s** |
| polls the BMS | USART1 (485) · USART2 (TTL) · FDCAN1 · I2C3 — whichever answered | every **10 s** |
| serves the block | I2C1, slave at **0x40** | on the head's read, ≤ once a minute in running, once a minute in survival |
| raises `ALERT` | PC8, push-pull, high = alarm | when a threshold is crossed, within one poll |

## 2. Boot

| # | step | what happens | if it fails |
|---|---|---|---|
| 1 | reset | every face asleep: `RE_485` (PE3) high and `DE_485` (PE2) low, `CAN_STB` (PE5) high, USART2 and I2C3 unclocked; `ALERT` (PC8) driven low; the IWDG at **2 s** | — |
| 2 | RC | the CSI at 4 MHz, or the HSI ÷ 8 where the head runs 400 kHz (a persisted flag); the flash cells read (§8): the maps, the faces found last time, the thresholds | a set that fails its check is no set: the card probes every face |
| 3 | the head | I2C1 as a slave at 0x40, the block zeroed, `STATUS` reading *probing* | — |
| 4 | probe | each face woken in turn and asked once with every map stored for it (§5) — 485 at the map's rate, the TTL UART at its, CAN at its, the I²C out for a part on the list; a face that answers is kept, the rest put back to sleep; no map stored for a face is a face never tried | nothing answers anywhere: `STATUS` *no unit*, the probe once a minute, the block zero but for `AGE` at 255 |
| 5 | run | the poll of §5 starts; the first block is complete within 20 s of power | — |

## 3. The states

```
   RESET ──▶ PROBING ──▶ POLLING (one or two units found) ──▶ (a unit silent 60 s) ──▶ PROBING
```

`PROBING` — the faces tried in the order of §2 step 4, once a minute until something answers.
`POLLING` — the found units polled at their cadence; a unit that fails three polls in a row is
dropped to `PROBING` on its face alone, the other unit polled on. There is no ended state and
no command that stops the poll; **`END` has no meaning here — Hermes is the way back** and the
link is never gated, in the lockdown least of all (`README.md`, `../core/POWER.md`).

## 4. The link to the head

**I2C1, slave, 7-bit address 0x40, 100 or 400 kHz as the head clocks it.** A register address
byte then data; the head's normal read is **one 32 B block from 0x00**, which the FIFO serves
with no interrupt in the path once the block pointer is set. Every write is a threshold or a
selection and lands in a cell. The block is double-buffered: a poll writes the back copy and
swaps it between two head transactions, so a read never sees half a poll.

**`ALERT`** goes high when any threshold of §7 is crossed on a fresh poll, and stays high until
the head reads `0x1E ALERT_CAUSE`; the head's LP GPIO wakes on it in survival. A threshold
crossed back is not an alert — the head reads the block and sees it.

## 5. The faces, the arrangements and the map

**What hangs on the faces is the builder's, and it changes faster than a document can follow.**
The station designs what it builds; the BMS and the MPPT are bought, every maker has a dozen
units and a register map of its own, and a unit recommended this year is withdrawn or reworked
the next. So **this document carries no vendor's protocol**. It carries three things: the
arrangements a 12 V LiFePO₄ pack is run in, the buses those units speak, and **the map — a
profile the builder fills once for the units bought** and the card runs. Which arrangement and
which units a station takes is its builder's; the card is the same for all of them.

**The arrangements:**

| arrangement | what talks | what Hermes polls |
|---|---|---|
| **a separate MPPT charge controller and a separate smart BMS** — the common build | two units, on two faces or sharing the 485 at two addresses | both: the MPPT for the sun side — PV voltage and current, charge current, the charge stage, its faults — and the BMS for the pack — voltage, current, state of charge, the cell extremes, the temperatures, the MOS states, its faults |
| **a hybrid charger** — an inverter-charger or an all-in-one that itself talks to the BMS | one unit on one face, relaying the pack's figures | one; the pack fields as the charger relays them, or the BMS polled directly on a second face where it has one |
| **a pack with its own charger** — an MPPT with a built-in BMS, or a "smart" battery with the charger inside | one unit | one, carrying both sides of the block |

**The buses, and which face takes them:**

| bus | the face | what rides on it |
|---|---|---|
| **485 Modbus RTU** — the MPPT world's standard, and many BMS | USART1, the `THVD1450` | holding and input registers (functions 03 and 04), 9 600–115 200 8N1, a slave address per unit; two units may share the pair |
| **a TTL UART with the vendor's frame** — the BMS world's standard | USART2, 3,3 V | a sync byte, an address, a command, a payload, a checksum; 9 600 the common rate |
| **CAN**, 250 or 500 kb/s | FDCAN1, the `TCAN334` | the battery-to-inverter protocols, vendor identifiers carrying the pack's figures without a request |
| **I²C** | I2C3 through the `PCA9306` | a part that offers one |

A unit with no bus but Bluetooth or a cloud is not a unit this card can read, and is not bought.

**The map.** For every field of the block (§6) the map says **where it comes from**: the face · the
unit's address · the request — a register address and count on Modbus, a command byte on a UART
frame, an identifier on CAN · the byte offset and width in the reply · the scale and the offset
that turn the unit's figure into the block's unit · the sign. A unit's poll is the set of distinct
requests its fields name, one exchange each; a field the bought unit does not give is marked
absent and reads `0xFFFF` (or `0x7F` in a byte) in the block. **The map lives in the cells and the
head writes it** over the I²C at commissioning, one field per register write at `0x40`–`0x7F`,
from the profile the builder wrote out of the unit's protocol document; the head keeps the profile
with the station's configuration, so a replaced card is written from it. **Two fields are not
optional**: `BAT_V` and `BAT_SOC` — the survival loop stands on them (`../mayak/FIRMWARE.md` §11) —
and a unit that gives no state of charge has it derived by the head from the voltage, never by
this card. The fault words are packed by the map into the block's own bit order (§6), so the head
reads one meaning whatever the vendor's.

**A map entry is one register of twelve bytes, and the register number says which field it
feeds.** `0x40`–`0x5F` are the block's value fields in the order of §6 — `BAT_V` at `0x40`, `BAT_I`
at `0x41` and so on, a two-part field (`BAT_T_MIN` · `BAT_T_MAX`, `CELL_MIN` · `CELL_MAX`) two
registers; `0x60`–`0x6F` the sixteen bits of `BMS_FAULT`, `0x70`–`0x7F` the sixteen of
`MPPT_FAULT`, one entry per block bit. Up to seven maps per unit are stored, selected at `0x30`.

| byte | field | meaning |
|---|---|---|
| 0 | face | 0 absent — the block field reads `0xFFFF` or `0x7F` · 1 485 · 2 TTL UART · 3 CAN · 4 I²C |
| 1 | address | the Modbus slave, the UART frame's address, the I²C address; 0 on CAN |
| 2–5 | request | 485: function (03 or 04), register address (uint16), count · UART: the command byte, the frame type · CAN: the 29-bit identifier · I²C: the register |
| 6 | offset | the byte offset of the value in the reply |
| 7 | format | bits 1:0 width 1 · 2 · 4 bytes · bit 2 signed · bit 3 big-endian · bits 7:4 the source bit, for a fault-bit entry |
| 8–9 | scale | int16 in 2⁻⁸: the reply's figure × scale → the block's unit |
| 10–11 | offset | int16 in the block's unit, added after the scale |

Entries that share a face, an address and a request are one exchange; the card sorts them at
the write. A value entry's result is clamped to the block field's width; a fault-bit entry takes
the source bit named in byte 7 of the value at the offset and sets the block bit the register
number names.

**The poll.** Every 10 s the card wakes the unit's face — 485: `RE_485` low, and `DE_485` high for
the request only; CAN: `CAN_STB` low for the exchange — sends the request, waits **200 ms** for
the reply on the face's USART or the FDCAN FIFO, checks the CRC or the checksum, folds the
fields into the back copy of the block, and puts the face back to sleep. A failed reply is
retried once, then counted; the block's age byte for that unit counts the seconds since the
last good poll, and at **255** it stops and stays.

## 6. The block

**32 B, little-endian, read from 0x00 in one transaction.** What the head's survival loop
needs is in the first sixteen; the rest is detail.

| offset | field | unit |
|---|---|---|
| 0x00 | `BAT_V` | mV, uint16 — the BMS's pack voltage |
| 0x02 | `BAT_I` | mA, int16, positive charging |
| 0x04 | `BAT_SOC` | 0,1 %, uint16, as the BMS reports it |
| 0x06 | `BAT_T_MIN` · `BAT_T_MAX` | °C, int8 each |
| 0x08 | `BMS_FAULT` | uint16 — the vendor's fault bits packed by the map: over-voltage, under-voltage, over-current, over-temperature, under-temperature (the charge cut-off), imbalance, MOS fault |
| 0x0A | `BMS_STATE` | bit 0 charge MOS · 1 discharge MOS · 2 charger present · 3 load present · 4 balancing |
| 0x0B | `CHG_STATE` | 0 not charging · 1 float · 2 boost · 3 equalise · 4 fault |
| 0x0C | `PV_V` | mV, uint16 |
| 0x0E | `PV_I` | mA, uint16 |
| 0x10 | `CHG_I` | mA, int16 |
| 0x12 | `MPPT_FAULT` | uint16 — the charger's fault bits packed by the map: PV over-voltage, battery over-voltage, over-temperature, charge fault, the rest vendor's |
| 0x14 | `MPPT_T` | °C, int8 |
| 0x15 | `MPPT_SOC` | %, uint8 |
| 0x16 | `CELL_MIN` · `CELL_MAX` | mV, uint16 each |
| 0x1A | `CYCLES` | uint16 |
| 0x1C | `AGE_BMS` · `AGE_MPPT` | seconds since the last good poll, uint8 each; 255 = never or lost |
| 0x1E | `ALERT_CAUSE` | bit per threshold of §7; reading it drops `ALERT` low |
| 0x1F | `FACES` | bit 0 485 · 1 TTL · 2 CAN · 3 I²C — which face each unit answered on: bits 0–3 the BMS, 4–7 the MPPT |

**Beyond the block:**

| register | r/w | content |
|---|---|---|
| `0x20`–`0x2F` | r/w | the thresholds, §7 |
| `0x30` | r/w | the map selection per unit: 0 probe every stored map · 1–7 the map index to run |
| `0x31` | r/w | the clock flag: 0 CSI 4 MHz · 1 HSI ÷ 8 |
| `0x40`–`0x7F` | r/w | **the map**, one field per register: face · address · request · offset · width · scale · sign, as §5 — written by the head at commissioning, persisted |
| `0xF0 BOOT_COUNT` | r | |
| `0xF1 IDENT` | r | the house code |
| `0xF2 TAG` | r | CRC-16 of the UID |
| `0xF4 HEALTH` | r | polls, misses, retries per unit; probes run; I²C recoveries; IWDG resets |
| `0xF6 STATUS` | r | probing · polling · no unit |
| `0xF8 VERSION` | r | |

## 7. The thresholds and `ALERT`

Written by the head at every boot of the head and kept in the cells; each is one register and
a crossing on a fresh poll sets its bit in `ALERT_CAUSE` and drives `ALERT` high:

| register | threshold | default |
|---|---|---|
| `0x20 V_LOW` | `BAT_V` below, mV | 12 000 |
| `0x22 V_HIGH` | `BAT_V` above, mV | 15 000 |
| `0x24 SOC_LOW` | `BAT_SOC` below, 0,1 % | 200 — the station's 20 % lockdown |
| `0x26 T_LOW` | `BAT_T_MIN` below, °C | 0 |
| `0x27 T_HIGH` | `BAT_T_MAX` above, °C | 50 |
| `0x28 PV_LOST` | `PV_V` below, mV, for longer than `0x2A` minutes | 5 000 · 60 |
| `0x2C FAULT_MASK` | any bit of `BMS_FAULT` or `MPPT_FAULT` under this mask set | all |

A threshold at 0 is off. The head decides what a crossing means (`../mayak/FIRMWARE.md` §11);
the card only says that it happened.

## 8. Persistence

The H523's flash, in the node contract's append-only cells (`../core/PROTOCOL.md` §7): a cell
is id, value, check; the last good cell per id wins.

| cell | content | written |
|---|---|---|
| the maps | every field's source, as written at `0x40`–`0x7F` | on a head write |
| the faces | which face each unit last answered on, and the map selection | after a successful probe |
| the thresholds | `0x20`–`0x2C` | on a head write |
| the clock flag | `0x31` | on a head write; takes effect at the next boot |

No value and no time is stored.

## 9. Faults

| trigger | action | reported |
|---|---|---|
| a unit silent for three polls | that unit's face back to `PROBING`; the other polled on | `AGE` counts, `STATUS` |
| a reply with a bad CRC or checksum | retried once; counted | `HEALTH` |
| I2C1 stuck low 25 ms | the peripheral reset; the block untouched | `HEALTH` |
| the IWDG expires | reset; the faces re-found from the cells without a probe | `HEALTH` |

## 10. The processor's budget

| resource | used | of |
|---|---|---|
| USARTs | 2 — USART1 the 485, USART2 the TTL | 7 |
| I²C | 2 — I2C1 the head, I2C3 the I²C out | 3 |
| FDCAN | 1 | 2 |
| timers | TIM6 — the RTU gap and the 10 s cadence | |
| interrupts | 0 I2C1 (the head) · 1 the USARTs' idle lines · 2 the FDCAN FIFO · 3 TIM6 | |
| SRAM | two block copies · a 64 B buffer per face · ~1 kB of state | 272 KB |
| CPU | under 1 % at 4 MHz; `WFI` between polls | 4 or 8 MHz |
| draw | **≤ 100 mA** on the 3,3 V pin, every face fitted and polling (`HARDWARE.md`, *Bench criteria*) | |

## 11. What is tested, and where

**On a PC, with the pins replaced by callbacks:** the map engine against recorded exchanges of
the units the first build bought — a Modbus register read, a UART frame and a CAN identifier
through the same field table, a truncated reply, a bad checksum, a field marked absent; the block's fields and the double buffer under a read that lands mid-poll; every
threshold crossing, `ALERT` high, and its drop on the `ALERT_CAUSE` read; the probe order with nothing, one and two
units answering; the cells.

**On the bench, against `HARDWARE.md`:** the build's MPPT and BMS polled for 24 h with zero missed
polls; the head's 32 B read at 100 kHz under 10 ms; `ALERT` within one poll of a written
threshold; the draw **≤ 100 mA** on the 3,3 V pin; the `ID` at 0,90 ± 0,025.

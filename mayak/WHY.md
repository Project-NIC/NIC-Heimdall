★ N.I.C. ★

# Mayak — the graveyard

Rejected alternatives and superseded states, with the reason. Nothing returns from here and
nothing is deleted from it. Hermes's own superseded states are in `../hermes/WHY.md`.

## The ESP32-S3 as the drawn fallback — superseded

Drawn on the `ESP32-S3-WROOM-1U-N8R8` while the S31 was pre-release. **Four MasterNOD trunks,
Hermes and the GNSS receiver want native serial ports**; the S3 has three UARTs, the S31 four HP
UARTs and an LP UART. The board is drawn on the `ESP32-S31-WROOM-3` only.

## Hermes on the LP UART — superseded

Hermes sat on the LP UART at 9600, its ~115 kbaud ceiling fit for nothing else. **Hermes is one
register block, which is what an I²C slave is**, and the LP I²C masters from the LP domain, so the
survival loop keeps its ear.

## PSRAM — the 8 MB class asked, 16 MB written — closed

The MLA ring buffer and Wi-Fi want the 8 MB class; **the external-antenna `-3U` comes only as
`N16R16V`** (module datasheet v0.7).

## The plug-in display dongle — dropped

A UART or Modbus dongle, or an SPI display, as the no-phone fallback. **It runs on the head's own
firmware, so it fails in the one case a fallback is for**; a dead head has the ROM's USB Serial/JTAG,
a live one the phone over BLE.

## The four 330 Ω pull-ups fitted on every head — superseded by unfitted pads

Fitted on every Mayak for two cards on one trunk port. **No build stacks cards** — the base firmware
does not, and one card a port bounds the failure domain — so they loaded a push-pull line for
nothing. A stacking build solders four 0603s on the pads.

## Port 1, port 4, port 2's `RXD` and the LoRa clock on the RMII pins — moved

Ports 1, 2 and 4 and the LoRa's `SCK`/`MISO` sat on `GPIO8`–`19`, **the S31's IOMUX-fixed RMII
pins**. Through the matrix they moved to the free JTAG pads, `TX0`/`RX0` and `GPIO36`; `MDC`/`MDIO`
took the boot straps. One pin table serves the Mayak, Proteus and Mimir.

## The multipoint pull-up on the head's `TXD` — wrong side, corrected

The 330 Ω pull-ups first sat on the head's `TXD`, with the open drain on the head. **The shared node
is the head's `RXD`**, where every card's `TXD` lands, so the resistors moved there and the open
drain to the cards; the head's `TXD` has one talker and stays push-pull. Push-pull on a card's
`TXD` saves nothing while the pull-up stands — low draws the same 10 mA; only a released open drain
idles high at no current.

## 220 Ω on the multipoint pull-ups — dropped

330 Ω against ~40 pF is 13,2 ns of a 238 ns bit at the 2²² rung, 5,5 %, enough. **220 Ω puts
15 mA on the line against the H523's `V_OL` guaranteed at 8 mA**, spending low-level margin on an
edge already short.

## The supercap bank and the brownout detector on the 3,3 V — superseded

A supercap bank or small LiFePO4 for one SD flush and clock hold, triggered by the 3,3 V brownout
detector. **It fires when the buck has already lost its input and the capacitor is spent**, and the
head has no RTC to hold. After a 30 mF bank triggered from the 12 V wire, a LiFePO4 18650 took the
job: the flush, and the head and Hermes alive to report the cause on LoRa, for two load switches
and an ideal diode. Two LTO cells for the cold were dropped: the cell sits frost-free beside the
pack, and one LiFePO4 cell needs no charger IC.

## The takeover on the 12 V wire, and the standard 18650 — superseded

The backup `LM66100` took `CE` from the 12 V wire through 47 kΩ, a 4,7 V zener and 100 kΩ. **A zener
specified at 5 mA got ~66 µA**, so the path could stay on with the 12 V healthy; `CE` went to
`VOUT`, following the rail's sag. The standard LiFePO4 18650 stops at −20 °C discharge and 0 °C
charge (the −35 °C was a prismatic cell's), so it became a low-temperature 18650: discharge to
−40 °C, charge to −20 °C.

## The `LM66100` pair on the backup cell and on `VBUS` — superseded

A `TPS22917` hold and an `LM66100` takeover, with `VBUS` into the buck input through a second
`LM66100`. **That one, rated ±6 V, faced the 12 V wire's up to 20 V**, and neither part limits
current, so a rail short took the cell's whole short-circuit current. Now a `PMEG10020ELR` on `VBUS`
and one `TPS259474LRPW` eFuse on the cell — hold, takeover, breaker and undervoltage cut; `PG` for
`ST`, `ILM` the cell's current. It blocks only 22–36 mV above the cell, where the `LM66100` turned
on 80–250 mV under it, so the layout keeps the cell out of the rail's load steps.

## The README's station profiles and flow — superseded

A `-DNIC_STATION_SEISMO` flag compiling an STA/LTA trigger (`nq-detect`) to fill the LoRa frame's
`events` and `peak`; cards gating their spur clocks, the supply voltage in every DATA frame, a
`GET(class)` block for CPU temperature. **The server is the detector**: on an ALARM or EVENT marker
the head packs a headline from one archive window (`FIRMWARE.md` §12), and the supply rides the
SUPPLY flag and the report frames.

## Clocking the head from the network — parked, not adopted

The S31's 40 MHz crystal carries the radio's calibration and is no integer multiple of 2²³, so an
external clock would need fractional synthesis. **The head needs no coherence**: it captures PPS-K,
the cards stamp, the trunk is asynchronous, and an injected clock would tie the head's life to
Kronos's where it now survives a dead clock and reports it. Loopholes noted: a 2²⁵ crystal input if
the radio allows, and trunk UARTs clocked from the time bus on `GPIO38` — a build's choice.

## The GPS pigtail on the head's board — superseded

A five-wire `VCC · GND · TX · RX · PPS` pigtail as the one connector for any receiver. The receiver
lands on Kronos's `TIME IN 1`; the head has no receiver connector (`../kronos/polaris/`,
`../kronos/HARDWARE.md`).

## The radio position — renamed the modem position, and bound to the cell

It was named for LoRa and the satellite modules, and nothing said what fed it. **The modem position
is whatever sends the last report** — LoRa, a satellite module or an LTE-M modem — and it is fed
from the rail the backup cell carries. **On the cell the silence of every other unit is expected**:
one line, the flush, the report, and the head lets go (`FIRMWARE.md` §6).

## The 1,5 Ah LiFePO4 18650 as the backup cell — superseded by ~3 Ah of any wide-temperature cell

One reference cell, one size. With a modem on the cell the event still takes a few percent of it,
but the cold and the sodium-ion option ask for room: **~3 Ah, low-temperature LiFePO4 or
sodium-ion, an 18650, a 21700 or a thin prismatic cell**, held to one condition — above 3,0 V at
−40 °C and 0,6 A.

## A wide-temperature Li-ion NCM cell as the backup — not taken

UltraXel's `HL18650V` — 2900 mAh, −40…+85 °C, charge from −20 °C — was weighed. **It is NCM, not
LiFePO4**: full at 4,2 V it is over the module's 3,6 V maximum, and floated at the rail's 3,43 V it
holds a few percent of its charge. It would need its own charger and a regulator to the rail; the
backup cell is on the rail through the eFuse alone, so only a chemistry whose voltage sits in the
module's window is taken — LiFePO4 or sodium-ion.

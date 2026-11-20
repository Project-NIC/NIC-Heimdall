★ N.I.C. ★

# Ratatoskr — the hardware

> **Design-stage concept — nothing built.** The figures are verified against the parts' sheets and
> the parts before a board is made.

## 1 The power chain

```
 panel ─ M12 ─ LMR43610 (5,0 V fixed; EN on a divider from the panel) ─▶ VBUS ┐
                                                                              nPM1300 ── cell (one sodium-ion)
   VSYS ─┬─ BUCK1 2,4 V ─▶ U535 · BMP585 · the I²C pull-ups
         ├─ BUCK2 2,4 V ─▶ the radio
         ├─ load switch 1 ─▶ the probe down (5 cm)   │ VSYS on the leads,
         └─ load switch 2 ─▶ the probe up (2 m)      │ each probe its own LDO
```

**The cell — one sodium-ion cell, about twice an 18650's capacity**, run as LiFePO4: **3,6 V full — the
nPM1300's termination at its power-up default** — down to **2,5 V**, where the MCU puts the nPM1300 into
ship mode. Some 55–75 % of the cell (`../../core/POWER.md` §3's sodium table). It charges in frost, its
limit written into the nPM1300's `NTCCOLD`. The cell is the builder's pick under these lines: charge
from −20 °C under 3,6 V, self-discharge ≤ 3 % a month, cycles stated to 80 % with their conditions.

**The nPM1300** (Nordic, PS v1.1): termination 3,50–3,65 V and 4,00–4,45 V, 3,60 V the default and
charging off after a reset; `VBUS` 4,0–5,5 V, 22 V absolute; no under-voltage cut in the part, so the
MCU's at 2,5 V; ship mode 370 nA, woken by `SHPHLD` held low — the reed under the lid — or by `VBUS`;
battery to `VSYS` 160 mΩ, load switches 200 mΩ. **Nothing hangs on `VSYS` but the bucks and the switches:**
the part does not regulate it, and with the panel's stage up it stands near 5 V.

**Both bucks at 2,4 V**, below the cell's whole window, so they never drop out and the rail stands;
the MCU's LDO and the radio's PA regulator draw about the same current at any supply, so 2,4 V takes
~73 % of a 3,3 V rail's power. The radio tops out near +19 dBm there (`TXDRP` −3 dB at 2,4 V); the
near band's 5 mW is unchanged. At its 120 mA bursts `BUCK2` holds 2,4 V to a cell of ~2,46–2,49 V.

**The panel's stage — the house `LMR43610`**, its fixed 5,0 V into `VBUS`; the nPM1300's `IBUSLIM`,
100 mA, the current limit; **its `EN` on a divider from the panel** — a precise rising threshold of
1,16–1,23–1,30 V and 0,30–0,40 V of hysteresis — so it starts only when the panel holds its working
voltage under load and stops at ~0,72 of it; set near that voltage it holds the panel there, a
constant-voltage tracker. 1,6 µA not switching, 0,25 µA shut down, all of it from the panel; dark,
the nPM1300 isolates `VBUS` below 3,6 V. **The panel** a small glass one, 6 V or 12 V class, 0,5–1 W:
the unit's day is a few mAh, and the panel only has to beat the cell's self-discharge.

## 2 The probes

**A small board of our own at each probe's end**, assembled at JLCPCB:

| part | why |
|---|---|
| `SHT45-AD1F` | the station's sensor, its PTFE membrane fitted by Sensirion; ±0,1 °C, ±1,0 %RH |
| `LP2985-N`, 2,4 V | the house family; 2,5–16 V in, so the TVS's clamp sits inside its rating; 150 mA for the heater |
| `SMF6.0A` on the feed | 6,0 V standing off, 6,67–7,37 V breakdown, 10,3 V at 19,4 A — one at each end of the feed |
| `TPD4E05U06` on SDA and SCL, 33 Ω after it | the station's rule for a bus on a connector (`../../core/HARDWARE.md`) — at each end |

**The `SHT45` on a tongue at the board's tip, a slot cut round it**, behind the shield's opening; the
LDO and the protection at the cable's end. The LDO dissipates (`VSYS` − 2,4 V) × 0,5 mA for a read's
tens of milliseconds — microwatts on average. **The board potted but the membrane**, on a bought M8
four-pole cordset. **The leads carry `VSYS`** through the nPM1300's load switches: a struck or shorted
lead takes its own switch and its own LDO, never the MCU's or the radio's rail. Between the probe's
2,4 V and the main board's pull-ups the I²C is open drain; ~10 mV is lost from the cell to the probe.

**The heights** are the construction's: the shielded probe at 2 m, the bare one at 5 cm, the box at 1 m.

## 3 The pins — STM32U535, LQFP48 without SMPS

From ST's DS14217 rev 5, Table 26 and its alternate-function tables. **Its pinout is the H523's but for pin
46** (the H523's second `VCAP`, the U535's `PB9`):

| function | peripheral | pins |
|---|---|---|
| the nPM1300 | `I2C1` | `PB8` SCL · `PB9` SDA; its interrupt on `PA3` (`WKUP5`) |
| the barometer | `I2C4` | `PB6` SCL · `PB7` SDA |
| the probe down | `I2C2` | `PB13` SCL · `PB14` SDA |
| the probe up | `I2C3` | `PA7` SCL · `PB4` SDA |
| the radio | `SPI1` | `PA5` SCK · `PA6` MISO · `PB5` MOSI · `PA4` NSS; `PB0` BUSY · `PA0` DIO1 (`WKUP1`) · `PB1` NRESET · `PB2` RF_SW |
| the "send now" reed | wake | `PC13` (`WKUP2`) |
| Imber's tips | wake | `PA2` (`WKUP4`) |
| USB, pads inside | | `PA11` · `PA12` |
| the crystal · SWD | | `PC14` · `PC15` · `PA13` · `PA14` |

**Four buses**, so a probe's fault never holds the nPM1300 — the charge and the ship mode — nor the
barometer; the two `SHT45`s keep one address, 0x44, on two buses. 130 pF on a metre of lead; 2,2 kΩ
pull-ups give a 242 ns rise against standard mode's 1 000 ns (`../calc/link.py`).

## 4 The connectors and the box

| | connector |
|---|---|
| the probes | M8, four poles, A-coded (IEC 61076-2-104), female: 1 the feed · 3 0 V · 2 SDA · 4 SCL |
| Imber | M8, three poles, female: the reed across two |
| the panel | M12, four poles, male, two used |
| the antenna | a short flexible whip on an N bulkhead |

The M8s cannot meet a 12 V M12; a 12 V plug in the panel's input harms nothing — the `LMR43610` takes
36 V. **The box:** an ePTFE pressure-equalising vent, the board coated and upright, little free air, a
white roof over it; **no service connector** — the settings and the boot log through the receiver, the
firmware with the lid off on SWD or the USB pads.

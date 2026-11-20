★ N.I.C. ★

# Iris and Arke — the board

> **Design-stage concept — nothing built.** The figures are verified against the parts' sheets and
> the parts before a board is made.

**One board for Iris and Arke**: the MOD's front twice, the radio, a switched 12 V out, in a sealed
box with two round connectors. What only Arke fits — its GNSS — is on this board, unfitted on Iris.

## 1 The board

| block | parts | why |
|---|---|---|
| the 12 V in | 2× `5.0SMDJ18A` across the input, the MOD's basic set | Arke's pack charges to ~14,4 V and the arm's 12 V reaches 16,2 V; the 18 V transils stand both |
| 3,3 V | `LMR43610R3RPER` | the house buck, 36 V in |
| the input's 485 | `THVD1450`, 2× 10 Ω in series, `SM712` across the pair | Iris: a slave on Palatine's arm; Arke: the master of its BMS and MPPT |
| the output's 485 | `THVD1450`, 2× 10 Ω, `SM712` | Arke: the master of its arm; Iris: off |
| the switched 12 V out | `TPS259474LRPW` eFuse, its `ILM` read | Arke's sensors' feed, on only for a read; its current tells the role |
| the radio | Seeed Wio-SX1262, 11,6 × 11 mm, its IPEX to an N bulkhead | 22 dBm at 125 mA, 1,62 µA asleep, −40 to +85 °C (its sheet V1.0) |
| the board's own sensors | `SHT45` (on a tongue, slotted off the board) and `BMP585` on `I2C1` | every Njord unit carries them: the box's own climate, the pressure |
| the beat | 32,768 kHz crystal on `LSE` | Iris's slots; Arke's clock between its GNSS fixes |
| Arke's GNSS | a multi-constellation module of the cheapest class (ZKW `ATGM336H`, Quectel `L76K`), its PPS on a timer capture | unfitted on Iris |
| the reeds | two reed switches under the lid | the role's reset; "send now" |

**Both `THVD1450`s have `RE#` on a pin**, so a line sleeps with what is on it; a MOD's ties it low.

## 2 The pins — STM32H523, LQFP48

From ST's DS14540 rev 3, Table 13:

| function | peripheral | pins |
|---|---|---|
| the input's 485 | `USART2` | `PA2` TX · `PA3` RX · `PA1` DE · `PA8` RE# |
| the output's 485 | `USART1` | `PA9` TX · `PA10` RX · `PA12` DE · `PB15` RE# |
| the radio | `SPI1` | `PA5` SCK · `PA6` MISO · `PA7` MOSI · `PA4` NSS; `PB2` BUSY · `PB12` DIO1 · `PB13` NRESET · `PB14` RF_SW |
| `SHT45`, `BMP585` | `I2C1` | `PB6` SCL · `PB7` SDA |
| the eFuse | GPIO · ADC | `PA11` EN · `PB8` PG · `PB0` `ILM` (`ADC12_INP9`) |
| Arke's GNSS | `USART3` · `TIM2` | `PB10` TX · `PB1` RX · `PA15` PPS (`TIM2_CH1`) · `PB4` reset |
| the reeds | wake pins | `PA0` (`WKUP1`) · `PC13` (`WKUP4`) |
| the crystal | `LSE` | `PC14` · `PC15` |
| SWD | | `PA13` · `PA14` |

## 3 The connectors

**M12, A-coded, four poles (IEC 61076-2-101), the gender keying it** — the side that feeds is female:

| | gender | 1 brown | 2 white | 3 blue | 4 black |
|---|---|---|---|---|---|
| **in** | male | 12 V | A | 0 V | B |
| **out** | female | switched 12 V | A | 0 V | B |

Iris's output wears a cap. **The antenna on an N bulkhead**, a gasket that outlives ten winters,
outdoor joints under self-amalgamating tape.

## 4 The enclosure

**Sealed, opened only for service, made for ten years outside:** an **ePTFE pressure-equalising vent**,
so the lid and the gaskets do not flex with each day's heat and the barometer reads the air; the
board **conformal-coated** against the dew a cold night brings, upright, never under the coldest wall;
as little free air inside as the parts allow. The box under a white roof against the sun by day and
the clear sky by night. No connector for service: Iris and Arke are set, read and updated over their
input's 485 pair with a USB adapter.

## 5 Power

| | |
|---|---|
| Iris | the receiver's 4,6 mA and the H523 — **0,6–1,3 Wh a day** off the arm's 12 V (`../calc/power.py`) |
| Arke | its own pack — `arke/HARDWARE.md` |

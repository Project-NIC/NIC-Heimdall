<p align="center">
  <img src="NIC-Babel.svg" width="200"/>
</p>

★ N.I.C. ★

# Babel — the protocol converter

> **Design-stage concept — nothing built.** The figures are verified against the parts' sheets
> and the parts before a board is made.

**Babel puts a sensor that is not sold as a Modbus unit on a ModBus arm.** The sensor's own
interface — I²C, SPI, UART or 1-Wire — goes in on one side; Modbus RTU comes out on the other,
and the host reads a finished number in the house register shape. The board is the same for
every sensor; what changes is the firmware profile compiled for the position.

**No foreign bus rides the station.** A sensor that does not speak Modbus is converted at the
sensor, on this board, never carried across the station and unpicked at the far end.

**The board is not universal; the processor is.** A board that carries a driver for every sensor
is never finished, so this one carries none: it is the `STM32H523` with its peripherals brought
out — SPI to 2²⁶ bit/s, a USART to 2²³ baud, I²C, a clock output, 59 free pins — on the same
crystal and the same buck cell as every H523 board in the station, and the builder configures it
for one sensor. The base firmware runs with nothing attached: the board answers on the bus with
its identity, its house block and zeros, and a profile per fitted sensor is added to that.

| | |
|---|---|
| class | house MOD — a Modbus RTU slave on a **Palatine** arm; there is no NodBus build |
| type code | **none.** Each fitted position answers as its own slave on the type of **its quantity**, `TYPE«2 \| NUMBER`; `0xFF01 IDENT` says the slaves are one board. **A board with nothing fitted answers once on ModBus type 4, bare**, with its identity and zeros (`MODBUS.md`) |
| positions | up to **four** sensors, one address each |
| sensor interfaces | I²C · SPI · UART · 1-Wire · 2× GPIO · a clock output, all 3,3 V |
| MCU | `STM32H523VE`, LQFP100 |
| feed | the arm's isolated 12 V on the four-wire cable, like a bought sensor; the board makes its 3,3 V on an `LMR43610` |
| line front | `THVD1450`, the basic set on board (`HARDWARE.md`) |
| firmware | the house MOD engine and one profile per fitted sensor (`FIRMWARE.md`) |

## The block

```
   the ModBus arm — A · B · 12 V · GND
        │
   ┌────┴────────────────────────────────────────┐
   │ BABEL — H523 · LMR43610 → 3,3 V             │──▶ I²C · SPI · UART · 1-Wire · 2× GPIO · a clock ──▶ up to four sensors
   │ THVD1450 · 2× 10 Ω + SM712 · 2× 5.0SMDJ18A  │    one ModBus address per fitted position,
   └─────────────────────────────────────────────┘    0xFF01 IDENT says they are one board
```

## Files

| file | contents |
|---|---|
| [`HARDWARE.md`](HARDWARE.md) | the board — supply, line front, terminal, the sensor header and its protection, every pin, the parts |
| [`MODBUS.md`](MODBUS.md) | identity, addressing, the registers the host reads |
| [`FIRMWARE.md`](FIRMWARE.md) | the engine and the profiles, described |
| [`WHY.md`](WHY.md) | the graveyard |

## Licence

Hardware: CERN-OHL-S v2 (`../LICENSE-HW`) · Software: MIT (`../LICENSE`) — Copyright (c) 2026 NIC — Native Intellect Community

★ N.I.C. ★

# Babel — the firmware, described

> **Design-stage concept.** The figures are verified against the parts' sheets and the parts
> before the build is written.

The board: `HARDWARE.md`; the identity and the registers: `MODBUS.md`; the leaf bus:
`../core/blocks/modbus.md`; the host's sweep and poll: `../palatine/FIRMWARE.md` §6; the node
contract: `../core/PROTOCOL.md` §2, §7. Where this document and one of those differ, that one
wins.

## 1. What the firmware is

**One engine, one profile per sensor, one slave per fitted position.** The house MOD engine
— the RTU slave, the house block, the cells — with up to four sensor profiles compiled in, one per
position the builder fitted. At boot each profile probes its position; every position that
answers is brought up as its own slave at `TYPE«2 | NUMBER` on the type of its quantity.

**A profile** is the driver — the bytes written and read on the header's interface, the
conversion to the finished value, the cadence — and the naming: the TYPE, the register layout,
the `STATUS` bits. The engine knows nothing about sensors; a profile knows nothing about Modbus.

| the firmware does | on | how often |
|---|---|---|
| answers the host | USART1, 19 200 8N1 (9 600 on `RATE`) | when polled |
| reads each fitted sensor | I2C3 · SPI1 · USART2 · 1-Wire on PD3 · PE5/PE6 · MCO1 | the profile's cadence, 1 s to 1 h |
| converts to the finished value | — | per read |

## 2. Boot

| # | step | what happens | if it fails |
|---|---|---|---|
| 1 | reset | `DE` (PE2) low; MCO1 off; the IWDG at **2 s** | — |
| 2 | clock | the 2²⁴ crystal validated against the HSI (±1 %); PLL1 M 2 · N 32 · P 2, SYSCLK 2²⁷ | no crystal: the HSI, `HEALTH` counts it, the board serves |
| 3 | cells | read (§8): per position its NUMBER and its profile's settings, the rate; the tag — CRC-16-CCITT over the 96-bit UID | a set that fails its check is no set: every position at NUMBER 3, the rate 19 200 |
| 4 | the probe | each compiled profile probes its position once — an I²C address acknowledged and an identity register matching, a SPI identity, a UART banner, a 1-Wire ROM — with the profile's own timeout, 100 ms at most | a position that does not answer is **not fitted**: no slave, no address |
| 5 | the slaves | one slave instance per position that answered, at `TYPE«2 \| NUMBER`, the house block filled, `IDENT` the same on all | no position answered: **one slave on type 4, bare** — the house block, `SENSORS` 0, `VALUE` 0 with VALID clear |
| 6 | the arm | USART1 at the persisted rate, DMA RX into a 64 B buffer, the RTU gap on TIM6; the slaves listen | — |
| 7 | the reads | each profile's first read; the values valid from then on | a failed read: SENSOR_FAULT on that slave, the value held |

## 3. The states

```
   RESET ──▶ PROBING ──▶ SERVING (n slaves, n ≥ 1 — the fitted positions on their types)
                │
                └──▶ BARE (no position answered): one slave on type 4 — identity, house block, zeros
```

**A bare board is on the bus.** The base firmware with no profile compiled, or a board whose
positions all failed the probe, answers as one slave at `4«2 | NUMBER`: `IDENT`, `TAG`,
`VERSION`, `HEALTH`, `BOOT_COUNT`, `RATE` as any house MOD, `SENSORS` 0, `VALUE` 0 with VALID
clear. The sweep finds it at NUMBER 3 and numbers it like any other; the operator sees a Babel
with nothing on it and not a dead cable.

A position that dies stays a slave with SENSOR_FAULT and its last value; a slave never
disappears at run time, because a population change is a boot's business (`../core/PROTOCOL.md`
§7). There is no sleep state: a MOD idles between polls at its parts' own currents, and a gated
arm is cut at its power board and comes back as a boot.

## 4. The bus — the slave's side

**Plain RTU, every slave on the one USART.** A frame ends when the line has been idle for 3,5
characters — **1,82 ms at 19 200, 3,65 ms at 9 600**, on TIM6; the CRC-16 is checked; a frame
addressed to one of this board's slaves is answered, any other is ignored; a broadcast (address
0) write is executed by every slave and not answered. Functions served: **03** read holding,
**04** read input (the same registers), **06** write one, **16** write many. A read across a
block returns the contiguous run as it stands; a write to a read-only register, or to an address
the block does not hold, returns exception **02**. `DE` high for the reply only, by the USART's
`DEAT`/`DEDT`; the reply starts after the gap and never before.

**The sweep.** A slave at NUMBER **3** is what the host looks for (`../palatine/FIRMWARE.md` §6):
a read of `0xFF01 IDENT` at `TYPE«2 | 3` answers, the host writes `0xFF05 NUMBER` with FC06,
the slave persists it and answers at the new address from the next frame. Each position is swept
and numbered on its own type. Two boards with the same quantity at the default collide and
neither answers cleanly; the operator plugs them in one at a time, as with any bought sensor.

**The rate.** 19 200 from the bench. Where the arm runs at 9 600, the learning session writes
`0xFF08 RATE`; the board persists it and boots at that rate ever after
(`../core/blocks/modbus.md`, *Two rates, one protocol*).

## 5. The reads

Each profile runs its own read on TIM6's schedule — a conversion started, its wait, the result
read and converted, the block updated under one interrupt-free copy so a poll never sees half a
value. A profile that needs the sensor clocked runs MCO1 at the power of two it asks for, 2²⁰…2²⁴,
while the position is fitted. **Cadences are the profile's constants, 1 s to 1 h**; a value polled
faster than it is read repeats.

## 6. The registers — each slave, the house shape

| register | contents |
|---|---|
| `0x0000` `VALUE` · `0x0001` `VALUE2` · `0x0002` `STATUS` · `0x0003` `RAW` | the reading, its pair, VALID/SENSOR_FAULT, the raw — `MODBUS.md` |
| `0x0004`… | the part's further values |
| `0x0010`… | the part's settings, persisted on write |
| `0xFF00 VERSION` · `0xFF01 IDENT` · `0xFF02 TAG` | the engine's; `IDENT` the same on every slave of one board |
| `0xFF04 HEALTH` | CRC misses, exceptions returned, sensor read failures, boots |
| `0xFF05 NUMBER` rw | this slave's NUMBER, written by the sweep |
| `0xFF06 BOOT_COUNT` · `0xFF07 STATUS` | the engine's |
| `0xFF08 RATE` rw | the arm's rate code, 9 600 or 19 200, persisted |
| `0xFF09 SENSORS` | the board's present-mask, one bit per fitted position — the same on every slave |

## 7. Faults

| trigger | action | reported |
|---|---|---|
| a sensor read fails | retried once; the value held; SENSOR_FAULT after three in a row, VALID cleared | `STATUS`, `HEALTH` |
| a sensor silent for ten reads | the slave keeps answering with the fault set | the host's `HEALTH`, `FAULT` up |
| a frame with a bad CRC | ignored | `HEALTH` |
| `PGOOD` low | noted | `HEALTH` |
| the IWDG expires | reset; the probe runs again — a sensor that died between boots is not a slave after the reboot, and the host's verify sees the population change; a board left with no position comes up bare on type 4 | `BOOT_COUNT` |

## 8. Persistence

The H523's cells (`../core/PROTOCOL.md` §7): per position its NUMBER, on the sweep's write ·
per profile its settings, on write · `RATE`, on write. `HARD_RESET`, the host's `SET` through the
tunnel: the NUMBERs to 3, the settings to their defaults, the rate to 19 200.

## 9. Budget and tests

USART1; the header's I2C3, SPI1, USART2, PD3, PE5/PE6, MCO1; TIM6 — under 1 % of the core at 2²⁷;
the engine ~8 kB of flash, a profile a few kB.

**On a PC, the engine with the pins as callbacks:** four slaves on one line; a read across a
block; a write to a read-only register; a broadcast write; the sweep's renumbering; the rate
switch; a truncated frame; a profile's driver against recorded sensor exchanges.

**On the bench, against `HARDWARE.md`:** a bare board answering on type 4 with zeros; a four-position board coming up as four slaves with one
`IDENT`; a position left empty coming up as three; the sweep numbering each on its own type from
a Palatine; 24 h of polling at 19 200 with zero CRC misses on a bench arm.

★ N.I.C. ★

# Quake — the graveyard

Rejected alternatives and superseded states, with the reason. Nothing returns from here and
nothing is deleted from it.

## 125 SPS on an 8,192 MHz crystal — superseded by 128 Hz off the 2²³ timebase

Slot, round and timeout arithmetic stays exact only at **a 2ⁿ rate**, with no `/125` remainder.
Multiplying 8,192 MHz (2¹⁶ · 125) fails: the 5³ cannot be divided out of a 2²⁰ target, and
÷125 / ÷3125 after a PLL brings the arithmetic back. The sensors took ×128/125: 41,94304 kHz sits
mid the ICM-42688-P's 31–50 kHz `CLKIN`; the ADXL355's `EXT_CLK` at 1,048576 MHz is 2,4 % high,
**inside its oscillator's guaranteed ±2,6 %**. One timer cannot make both: one period, clocks
25× apart.

## 500 Hz with decimation on the node — rejected

No shared ODR below 500 Hz; decimating costs node arithmetic and **four times the bus traffic**.
Clocked onto 128 Hz, each sensor sends one raw sample a frame.

## Free running with no `SYNC`, the ICM–ADXL offset characterised once — superseded

`EXT_CLK` with no per-sample pulse, each sensor latching its phase at start. Now a 128 Hz `SYNC`
train to the ADXL355, the ICM enabled at a chosen instant, both group delays removed.

## The SCL3300's mode as fixed configuration, with an auto-select at boot — superseded

Polled at ~10 Hz and averaged. Now chosen at calibration from the gravity vector, read every 16th
period.

## Or-equivalents weighed against the ICM and the SCL3300

- **LSM6DSV32X for the ICM-42688-P** — `INT2` sync, same gyro noise, worse accelerometer; the LSM
  disciplines toward the clock, the ICM's `CLKIN` **derives the ODR from it by construction**.
- **IIS2ICLX and IIS3DHHC for the SCL3300** — quieter or cheaper; the SCL3300 keeps its certified
  life-long drift (< 0,05°) and angle output.
- **IIS3DHHC as a cheaper seismic front** — no external clock input, so resampling.

## Two bucks and ten LDOs, an RC on the RM3100, one branch per sensor — superseded

Two `LMR43610`s (3,3 V, 4,0 V), a `TPS7A2033` per sensor rail, an RC (22 Ω, then 33 Ω, into 10 µF)
on the RM3100. The MEMS regulate themselves; **an inductor into 10 µF removes buck ripple better
than an LDO at that frequency**. Now one `TPS629206` in forced PWM, 2,2 µH branches to `L_ANA`
and `L_IO`, the RM3100 as on Gauss. The `MCP1700` cannot be sourced reliably.

## A 100 V input bulk — superseded

The board gets a regulated 12 V; every low-rail part is rated 50 V.

## The in-box Quake, the segment and the 3 W budget — superseded

An in-box Quake; up to eight nodes on one ≤ 3 W converter, a plug in the last free `OUT`. **A
measuring unit is never in the box**: one node per run, an 80,6 Ω jumper, the `INA238` trip at
that node's commissioned load.

## The half/full-duplex strap, the USART's hardware `DE`, `LINE_EN` on PE3 — superseded

Every NodBus run is full duplex; `DE` is a GPIO, the echo on `RXD_ECHO`. Nothing switches a
Galvani board at a unit end: `LINE_EN` is held in the socket.

## The supply and the temperature in the payload — superseded

Supply in the last payload byte, alarm in byte 30, die temperatures multiplexed, then queried.
Now SUPPLY and ALARM flags, `REPORT` for supply and board temperature, `HEALTH` for the dies.

## Low-power mode between samples, and a LoRa module on the node — deleted

Never parked; the node waits in `WFI`. **Every alert is the head's**, so the node has nothing to
say off its link.

## A rejoin phased off a neighbour's frame — superseded

A run carries one node, so the phase comes from the card's `TICK`.

## Hard potting in polyurethane — superseded

Now a meltable gel: hot water re-opens it.

## DoubleBarrel 60 cm and QuatroBarrel 120 cm — superseded

30 / 60 / 120 cm, the long one hanging. Now 25 cm steps, Barrel to QuadroBarrel, each lying full
length: **gel-filled, a hanging 50 cm tube rings at 24 Hz**.

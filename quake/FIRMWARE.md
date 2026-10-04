★ N.I.C. ★

# Quake — the firmware, described

> **Design-stage concept.** The figures are verified against the parts' sheets and the parts
> before the build is written. The frames, the opcodes and the node contract are
> `../core/PROTOCOL.md`'s, the clock and the bus start `../core/blocks/nodbus.md`'s; where this
> document and one of those differ, that one wins.

## 1. What the firmware is

**One build for one board, whatever is populated on it.** The image probes its three SPI buses
and the I²C at boot, keeps what answers, and runs the same acquisition, the same packing and the
same bus contract with one sensor or five. No configuration switch, no recompilation, no jumper.

**The node is dumb on purpose.** It owns no wall clock, no storage, no policy. What it does is
bounded and small: it locks to the wire clock, samples five sensors on that clock, levels the
seismic axes through one matrix, packs 32 B, and fires one frame per period in its slot. The one
piece of arithmetic on the node is the 3×3 matrix multiply per sample; everything else — the
tilt angle, the trigger's assessment, temperature correction, decimation, compression — is the
head's or the server's.

| | |
|---|---|
| bus | NodBus, type **5**, one slot; 40 B DATA at 128 frames/s, 12 B CONTROL in the gaps, `SUB` 0 |
| clock | the spur's 2²² on `OSC_IN`, PLL to 2²⁷; every sensor clock a division of it |
| sensors | ADXL355 (SPI1) · ICM-42688-P (SPI2) · SCL3300 (SPI3) · RM3100 (SPI3, option) · TMP117/STS35 (I3C1 in I²C legacy, option) · an NTC between the coils (ADC, option) |
| payload | 15 axes × 2 B + 2 reserve = 32 B, fixed, zeros where a sensor is absent; the trigger and the supply are the header's flags |
| report frame | `kind` 6, once a `REPORT_INTERVAL` (60 s), in the fourth frame-time of its own block behind the data: `VBUS` · `CURRENT` raw · the TMP117 · the coil NTC, int16 in 0,01 °C, positions 1–3 zeros; a reply owed goes first. `GET HEALTH` answers a `kind` 8 frame — the four die temperatures, then the error counters (`../core/PROTOCOL.md` §5) |
| rates | ADXL355 and ICM at 128 Hz, phase-locked; the RM3100 polled at 32 Hz and interpolated onto the grid as Gauss's is; the SCL3300 slow, latest value carried; the supply and the board's temperatures in the `REPORT` frame once a minute, the dies' on `GET HEALTH` |
| load | under 3 % of the M33 at 2²⁷: five SPI reads, one matrix per axis set, one CRC, one DMA transmit per period |

## 2. Boot

| # | step | what happens | if it fails |
|---|---|---|---|
| 1 | reset | every `CS` high, `DE` low, `LINE_EN` is tied and not a pin; IWDG started at **1 s** and fed from the main loop only | — |
| 2 | RC | HSI; the flash cells read (§11): the persisted set (NUMBER, slot, `BUSCFG`), the levelling matrix, the ranges, the SCL3300 mode, the present-mask; the tag computed — CRC-16-CCITT over the whole 96-bit UID | a set that fails its check is no set: the node boots as fresh |
| 3 | probe | SPI1: ADXL355 `DEVID_AD` = 0xAD, `PARTID` = 0xED · SPI2: ICM `WHO_AM_I` = 0x47 · SPI3, `CS_SCL`: SCL3300 `WHOAMI` = 0xC1 · SPI3, `CS_RM`: RM3100 `REVID` = 0x22 · I3C1 in I²C legacy: TMP117 at 0x48, `DEVICE_ID` = 0x117, or the STS35 at 0x4A · the coil NTC: a divider reading off the rails. What answers is present; what does not is absent and skipped for the whole run | **a present-mask that differs from the persisted one forces a fresh enrolment** — population is identity; the node stays on 38 400 and waits for a sweep |
| 4 | seismic source | ADXL355 present → it is the seismic and the levelling source; absent → the ICM's accelerometer is both | neither present: the node enrols and streams the slow sensors alone, `HEALTH`, `FAULT` up says why |
| 5 | configure | every sensor written from scratch — no sensor keeps NV state (§6): ADXL355 range, `DRDY`→`INT1`, external clock and sync; ICM `RTC_MODE`, pin 9 as `CLKIN`, accel and gyro ranges, 100 Hz ODR field, `INT1` data-ready; SCL3300 its persisted mode; RM3100 its cycle counts and continuous-measurement off; TMP117 continuous, 1 s | a sensor that does not take its configuration (read-back mismatch) is soft-reset once and then marked `NO_RESPONSE` |
| 6 | IDs | ADC1 reads `ID_D` and `ID_P` once, ratiometric, 16× oversampled — the communication board and the power board on the node's sockets, into `HEALTH`, `FAULT` up | a ratio in no window is *unknown board*; the node runs anyway — the boards are the port's business |
| 7 | the clock | the data body's `CLK` on `OSC_IN` validated — N clean periods at 2²² against the HSI — but **not taken**: a fresh node stays on RC at 38 400 until `BUSCFG`; a node with a persisted set locks HSE and listens on its persisted rung (§4) | no clock: RC, silent, waiting |
| 8 | the sensor clocks | TIM1 (`EXT_CLK` 2²⁰), TIM15 (`CLKIN` 41,94304 kHz) and TIM3 (`SYNC` 128 Hz) are started **only once HSE is locked** — a sensor clocked off the HSI would latch a phase that is ±1 % off the grid and keep it | — |

The 30-minute thermal settling is not the node's: it streams from `SYNC` and the head flags the
settling (`../core/PROTOCOL.md` §4).

## 3. The states

```
   RESET ──▶ RC (38 400, silent) ──▶ ENROLLED ──▶ LOCKED (HSE on the wire) ──▶ RUNNING
                 ▲    │                 ▲                 │                       │
                 │    └─ DISCOVER heard ┘                 └─ SYNC ────────────────┘
                 │                                                   │
                 │◀── clock lost (CSS) ─────────────────────────────┤
                 │◀── IWDG reset ───────────────────────────────────┤
                 └── ENDED (END: the parts stop, the buffer is flushed and answered; then the clock and the feed go)
```

`RC` — the HSI, the USART at 38 400, transmitting nothing; a DATA frame does not exist off the
clock (the mute rule). `ENROLLED` — the node has answered `DISCOVER` and taken `ASSIGN_ADDR`; it
holds a NUMBER and a slot and is still at 38 400. `LOCKED` — `BUSCFG` received, HSE switched onto
the wire, the USART on the rung, the sensor clocks started; no frame has left. `RUNNING` — `SYNC`
loaded the index, the sensors are phase-locked, the node fires every period. `ENDED` — on `END`:
the sensors go to standby, the ring is flushed and answered, the clock stays and the node listens
for as long as it lasts. **Clock lost** is a fault, not a state: the CSS raises an NMI, the node drops to the HSI,
mutes, stops the sensor clocks, and comes back through the rejoin (§4).

## 4. The bus — the unit's side

**The enrolment.** A `DISCOVER` broadcast at 38 400 is answered once by a node not in a running
round: the CONTROL frame `tag.0 · tag.1 · TYPE|NUM · 0 · DISCOVER · 0 · 0 · 0 · 0 · 0 · CRC`, after **tag × 10 µs**,
having listened first on `RXD_ECHO` — an edge seen while waiting restarts the wait; a bad CRC on
its own echo is a collision and the node waits a new delay, CRC-16 over the UID and the attempt
number. `ASSIGN_ADDR` with the node's tag in the time bytes gives it its NUMBER (`arg0`) and slot
(`arg1`); the node writes both to the cells, answers `ACK` and keeps them. `GET slots` is answered
**1** — a Quake takes one slot. A `DISCOVER` heard at 38 400 wins over the clock: a node with a
persisted set answers with the name it holds and lets the sweep validate it.

**`BUSCFG`** arrives as three broadcasts — `NODE_COUNT`, `FRAME_RATE` (the code; 7 is 128 Hz),
`RUNG` (2²⁰ alone, 2²¹ chained). On the third the node validates the clock it has been hearing
(N clean periods at 2²²), switches HSE onto it, locks PLL1 to 2²⁷, moves the USART to the rung,
starts TIM1, TIM15 and TIM3, and computes its deadline: **round start + (slot − 1) × width**, the
width `4 × 40 B × 10 bits / rung` — 763 µs at 2²¹, 1,53 ms at 2²⁰. The set is written to the
cells under one check.

**`SYNC`**, the last frame at 38 400, fired by the card on a second's boundary: its start-bit
edge is captured on TIM2 CH2 (PB3) — no CPU in the path — and the node loads `unix.0` from its
header and `frame` = 0 at that capture **less `ROUTE`** — the origin is placed the written
route early, in ticks of 2²⁷, so the grid is the card's and not the delayed one
(`../core/PROTOCOL.md` §7). From that origin TIM2 counts the period, 2¹⁶ ticks of 2²³
= 2²⁰ ticks of 2²⁷ = 7,8125 ms, and `frame` increments on every period, `unix.0` on every 128th.
**The ADXL355's `SYNC` pulse train (TIM3) is started from the same origin, 0,54 frame late**
(§5), so that what each sample describes falls exactly on a frame boundary.

**The slot.** A TIM2 compare fires the transmit: the frame is already assembled in the DMA
buffer, `DE` (PE2, a GPIO) goes up, the USART starts at the compare, `DE` drops on the USART's
transmission-complete after the last stop bit — so the driver's current into the termination
flows only while the node talks. The spur is point-to-point and full duplex: the
node is the one talker on its pair and waits on nobody. **The echo:** the node's own frame arrives on `RXD_ECHO`
(USART3, DMA); its CRC is checked when it ends; a miss counts `NIC_ERRC_BUS` in `HEALTH` and
holds the frame in the circular buffer for the `RESEND` the card will send.

**The gaps.** CONTROL frames addressed to the node arrive in its own block's third frame-time
and are answered in the fourth — `GET` under `GET`, `SET` with `ACK`, `RESEND frame · unix.0`
with the buffered frame from the circular buffer of `DELAY` frames (a `RESEND` reply displaces that period's
DATA sample; the buffer holds the displaced one too). A broadcast (0xFF) is taken by every node.
`TICK` is ignored by a running node.

**Ranging.** `SET` of the ranging register arms the turnaround for the next DATA frame's block:
after its DATA frame the node switches PB6/PB7 from USART1 to TIM4, raises `DE`, and TIM4 runs
one-pulse mode — CH2's capture of the incoming edge starts the counter, CH1 raises the return at
`CCR` = **256 ticks** (1,91 µs, past the incoming pulse's tail and the driver's turnaround) and drops it at `ARR` = `CCR` + the width the card asked for; CH2 in PWM-input mode
captures both edges of the incoming pulse and the width goes up in the next frame's `status`
register on `GET`. Then the pins go back to the USART. No interrupt is in the path; the constant
is 256 ticks by construction.

**The rejoin.** After any reset the node reads three things: is the clock there, does it hold a
set, has it heard the phase. Clock and set → HSE locked, the USART on the persisted rung,
listen; any frame with a good CRC proves the rung, a second of nothing tries the other one. The
phase comes from the card's `TICK`, valid at its start-bit edge, which TIM2 CH2 captured. The
node fires in the next round with `status` 1 REJOINED on its first frame. **The sensors are
restarted** at that point so their phase re-locks (the ADXL355 by its `SYNC` train, the ICM by
a standby–run cycle). No clock → RC, silent, wait for a sweep. Clock and no set → 38 400, silent,
wait for a sweep.

**`END`.** The sensors into standby (ADXL355 `POWER_CTL` standby, ICM `PWR_MGMT0`
off, SCL3300 power-down command, RM3100 idle), the ring flushed and the answer sent, the sensor
clocks kept running, the USART kept listening; the next CONTROL frame addressed to the node wakes it, the sensors are reconfigured
from scratch and re-phased on the next `SYNC`-equivalent — the node waits for a `TICK` and
restarts its sensor clocks at that edge. `END` takes no argument: what follows it is the card's
`PORT_CLK` 0 and then `PORT_PWR` 0, and the returning feed is a boot (`../core/PROTOCOL.md` §7).
No rail is switched (`../galvani/README.md`, *`ENABLE`, `LINE_EN` and the three states*).

## 5. Time on the node

**One 32-bit timer, never reset.** TIM2 counts 2²⁷ from PLL1 and is read as differences.
Three things hang on it: CH2's capture of every start-bit edge on `RXD` (the phase source), a
compare that fires the transmit at the slot deadline, and a compare every 2²⁰ ticks that
advances `frame`. The sub-second position of a sample is `frame` — 7 bits — and nothing finer
rides the wire: the sample instants are on the grid by construction, because the sensors are
clocked off the same PLL.

**128 Hz, sampled directly.** Six times the ~20 Hz seismic band, a quarter of the traffic of
500 Hz, and 2⁷, so the sub-second index is a clean 7-bit field. The two clocked sensors' own ODR
grids share no value below 500 Hz at their nominal clocks, so the clocks are tuned instead — each
lands on 128 Hz by its own divider — and the node sends raw, one sample to one frame, with no
decimation on it.

**Three sensor clocks, three timers, all integer divisions of 2²⁷:**

| | timer | division | value | into |
|---|---|---|---|---|
| ADXL355 `EXT_CLK` | TIM1 CH1, PWM 50 % | ÷128 | **2²⁰ = 1,048576 MHz** | pin 13 `INT2` |
| ADXL355 `SYNC` | TIM3 CH1, prescaler 16, `ARR` 65535, `CCR` 33; the counter preloaded so the first pulse lands **35 389 prescaled ticks = 4,219 ms** after the origin | ÷2²⁰ | **128 Hz**, a 3,9 µs pulse, **0,54 frame after each frame boundary** | pin 14 `DRDY` |
| ICM-42688-P `CLKIN` | TIM15 CH1, PWM 50 %, `ARR` 3199 | ÷3200 | **41,94304 kHz** | pin 9 |

The ADXL355 runs in full external-clock mode (`EXT_SYNC` 01, `EXT_CLK` 1): its ODR tap ÷8192
lands on 2²⁰ / 8192 = **128 Hz exactly**, and the `SYNC` pulse pins each sample's phase to the
frame boundary; the 2,4 % above the part's nominal 1,024 MHz is inside its working tolerance
— the part's own internal clock is guaranteed across ~±2,6 %, and its whole grid scales with
`EXT_CLK`. The ICM's 100 Hz ODR field scales with `CLKIN`: 100 × 41 943,04 / 32 768 =
**128 Hz exactly**, in the middle of the part's 31–50 kHz window. **Each sensor's digital filter delays what its sample describes — `LAT`, per sensor, from
its datasheet** (`../core/PROTOCOL.md` §7). **ADXL355: 1,54 ODR cycles** — Table 10 of its
datasheet (Rev. D), the 125 Hz row, in the external-sync mode without interpolation the data
"represents a sample point group delay earlier in time"; at our 128 Hz that is **12,03 ms**.
The delay is in cycles, so it does not move with the clock. **The `SYNC` train is placed 0,54
frame after the frame boundary** — TIM3 preloaded so its first pulse lands 35 389 prescaled
ticks (4,219 ms) after the origin — and what the sample taken at that pulse describes is
`boundary + 0,54 − 1,54` frames = **the start of the previous frame, exactly on the grid**; the
0,005-cycle rounding of the table is 40 µs. The ADXL355's sample that describes the start of
frame `N` arrives during frame `N + 1` and ships in frame **`N + 2`**: a fixed lag of two frames,
a constant of the type the archive subtracts, as Gauss's four. **ICM-42688-P: 5,1 ms at the
nominal 100 Hz ODR** — its datasheet (DS-000347 v1.6, *Group Delay @DC*, 2nd-order UI filter, the
default order, at `UI_FILT_BW` 1 = max(400 Hz, ODR)/4) — plus the 2nd-order anti-alias filter at
its reset bandwidths, computed at DC as √2 / (2π · BW): 0,19 ms on the accelerometer (1163 Hz)
and 0,38 ms on the gyro (~585 Hz). Every stage runs off `CLKIN`, so at our 128 Hz the whole
scales by 100/128: **accelerometer 4,13 ms = 0,53 frame, gyro 4,28 ms = 0,55 frame**. The ICM
has no sync pin; its ODR's phase is set by the instant it is enabled and locked to `CLKIN` from
then on. So it is enabled at a TIM2 instant chosen to put `DRDY` **0,53 frame after the frame
boundary**; the first `DRDY`'s timestamp (the `INT_ICM` handler reads TIM2) checks the phase,
and one re-enable corrects it if it missed by more than 0,02 frame. From then on every ICM
sample describes the frame boundary — the gyro 0,15 ms after it, 1° at 20 Hz, accepted — and
ships with the ADXL355's at the same two-frame lag. Both sensors ride the one payload at the one
lag; the ring holds the frames between.

**The SCL3300 has no clock input** and free-runs on its own oscillator; it is read at the
housekeeping cadence (§6) and carried at its latest value — its time is the frame's, exact
enough for a tilt moving over minutes.

**The RM3100 is Gauss's channel on this board and is read Gauss's way** (`../gauss/FIRMWARE.md`
§4): `POLL` on a TIM2 compare every 4th frame, **32 Hz**; `DRDY` on PD10 (EXTI10) timestamps
TIM2 in its handler — the handler's microseconds against a 3,5 ms conversion; the sample's
instant is `DRDY` less 1,76 ms — the middle axis's middle — the three axes as one instant; and the frame at grid
instant `g` carries the field at **`g − 4 frames`**, interpolated linearly between the two
samples that bracket it — the same fixed lag as Gauss, so the archive subtracts one constant
for both.

## 6. Acquisition — one period

Every period is the same sequence, driven by two interrupts and one timer compare, with the
processor asleep in `WFI` between them.

```
   t = 0        TIM3 SYNC edge ─▶ the ADXL355 samples; the ICM samples on its own CLKIN phase
   t ≈ 0,1 ms   INT_ADXL (PB0, EXTI0) ─▶ SPI1 DMA: 9 B, X/Y/Z 20-bit
   t ≈ 0,1 ms   INT_ICM  (PB2, EXTI2) ─▶ SPI2 DMA: 12 B, accel X/Y/Z + gyro X/Y/Z 16-bit
   after both   the levelling: three axis sets × R_level (FMAC, 3×3) ─▶ pack ─▶ the DMA buffer
   every 16th   SPI3: the SCL3300 (read-latest, 4 × 32-bit pipelined frames, CRC-8 checked)
   every 4th    SPI3: the RM3100 — POLL written on the compare, DRDY (PD10) timestamped, 9 B read; interpolated to g − 4 frames (§5)
   every 128th  I2C1: the INA238 voltage and current ─▶ VIN, the SUPPLY flag outside the window; I3C1: the TMP117 (2 B) ─▶ the REPORT frame
   every period nq-detect on the seismic source ─▶ the ALARM flag; the QC verdict ─▶ status 2 + FAULT, the detail in HEALTH
   at the slot  TIM2 compare ─▶ DE up ─▶ USART1 DMA, 40 B ─▶ DE down ─▶ the echo checked
```

**The sensors' configuration**, written at boot and after every wake, never trusted to survive:

| sensor | setting | value | why |
|---|---|---|---|
| ADXL355 | range | **±2 g** (`CFG` 0: ±2 / ±4 / ±8) | 3,81 µg per step under 25 µg/√Hz — noise-limited, not step-limited (`HARDWARE.md`) |
| | ODR / filter | ÷8192 tap, LPF ~32 Hz, no HPF | 128 Hz at 2²⁰ |
| | clock | `EXT_SYNC` 01, `EXT_CLK` 1; `DRDY` on `INT1`, `INT2` = clock in, `DRDY` pin = sync in | phase-locked to the grid (§5) |
| | mode | measurement; standby for a range write and on `END` | a range write needs standby — one or two samples perturbed, `status` 5 CHANGED |
| ICM-42688-P | accel range | **±8 g** (`CFG` 1: ±2 / ±8 / ±16) | twice the strongest recorded ground acceleration; the ADXL355 owns everything under 2 g |
| | gyro range | **±15,625 dps** (`CFG` 2) | the most sensitive — seismic rotation is tiny |
| | ODR | the 100 Hz field, in `RTC_MODE` with pin 9 = `CLKIN` | 128 Hz at 41,94304 kHz |
| | filters | UI filter 2nd order at `UI_FILT_BW` 1 — the defaults, 128 Hz bandwidth at our `CLKIN`; the anti-alias filters at their reset bandwidths; no on-chip decimation | raw 1:1, one sample one frame; the group delays are §5's `LAT` |
| | interrupts | `INT1` data-ready, push-pull, pulse | |
| SCL3300 | mode | **Mode 1** (±90°) or **Mode 4** (±10°), `CFG` 3, chosen once at calibration (§7) | range against resolution; a near-level install takes Mode 4's 10 Hz LPF and lowest noise. The mode commands, CRC included: Mode 1 `0xB400001F` · Mode 2 `0xB4000102` · Mode 3 `0xB4000225` · Mode 4 `0xB4000338` |
| | read | `ACC_X/Y/Z` and `TEMP`, 32-bit pipelined frames, CRC-8 poly 0x1D checked, `RS` bits checked | |
| RM3100 | cycle counts | **100 per axis**, as Gauss — 1,18 ms an axis, 3,5 ms the three, from the manual's 850 Hz single-axis maximum | the LR timebase is `REXT`; the count sets resolution against time |
| | mode | single measurement on `POLL`, `DRDY` on PD10 | no continuous mode — the chip idles between polls |
| TMP117 | mode | continuous, 1 s, 8× averaging | the board temperature — the report frame's third word |
| the coil NTC | the divider | switched on for the conversion only, once a minute | the coils' temperature — the report frame's fourth word |

**The levelling.** Every seismic axis set — the ADXL355's, the ICM accelerometer's, the ICM
gyro's — is multiplied by the one stored matrix `R_level` (§7) before packing; the SCL3300 and
RM3100 vectors are multiplied by the same matrix (the board's orientation is one thing). The
ADXL355's 20-bit values go through the multiply at full width; the FMAC does three 3×3 products
per period in a few hundred cycles.

**The packing** (`BUS.md`): each axis to **int16** — the ADXL355 by dividing off its low 4 bits
toward zero (`drop_bits` 4, the surviving noise ~3,3 LSB — the dither that keeps averaging
honest), the others as they are — then **saturated** to ±32767, so a railed axis clips and
never wraps. Field order: ADXL X/Y/Z · ICM accel X/Y/Z · ICM gyro X/Y/Z · SCL3300 X/Y/Z · RM3100
X/Y/Z · bytes 30–31 reserve, 0. An absent sensor's six bytes are zeros. Little-endian
throughout.

**The supply** is the `INA238`'s voltage and current on the power body, read once a second over
I2C1 into `VIN`; outside `VIN_WINDOW` the node sets the **SUPPLY flag** in its own header and
holds it while it lasts — no byte is streamed (`../core/PROTOCOL.md` §5).

**The ALARM flag** in the header is **`nq-detect`**: a short-term / long-term average ratio on
the seismic source's vertical axis after levelling — STA **0,5 s**, LTA **30 s**, trigger at
**ratio 4**, release at **1,5** — set while the condition lasts; the node holds no window and
sends no event, the head decides what an alarm is worth (`../core/PROTOCOL.md` §9, `EVENT`).
The LTA is not valid for its first 30 s after `SYNC` and the flag stays 0 until it is. The QC
verdict is `status` 2 SENSOR, the `FAULT` flag and `HEALTH` (§9), as on every unit; nothing of it rides the payload.

## 7. Calibration — the levelling matrix

On `SET CALIBRATE` from the head (a register write; the head sends it in a quiet window after
the 30-minute settle):

1. the node averages the seismic source's raw vector over **2¹³ samples — 64 s** at 128 Hz;
2. if the variance of the window exceeds **(10 mg)²** the average is rejected, `ERROR` code
   *calibration noisy*, and the head retries later;
3. the mean is gravity: normalised it is the new Z (toward the core); the old X crossed with it,
   normalised, is the new Y; Y crossed with Z is the new X — three vectors, one 3×3 matrix, no
   trigonometry;
4. the tilt from level is read off the same vector as `(x² + y²) / (x² + y² + z²)` compared with
   `sin² 10°`: within 10° the SCL3300 goes to **Mode 4**, otherwise **Mode 1**; the mode is written
   to the sensor and to `CFG` 3;
5. the matrix and the mode are written to the flash cells, `ACK` goes up in the node's slot, and
   the first frame levelled with the new matrix carries `status` 5 CHANGED.

**Vibration does not disturb it.** Gravity is DC and ground motion is zero-mean about its
equilibrium, so a long average leaves gravity; the quiet window and the variance bound keep an
event out of it. **One exact path**: the full rotation matrix, never a small-angle shortcut, and it
rotates the gyro's vector like the rest — at rest the gyro has no gravity of its own and inherits
the board's orientation. The SCL3300's mode is a hardware range chosen here once, not a second
formula.

The matrix persists across resets and is applied from boot; `HARD_RESET` puts it back to the
identity. Recalibration is needed only if the node has moved. The reported tilt angle is not
computed here — the head takes the arctangent of the SCL3300 vector.

## 8. The control plane

| op | what the node does |
|---|---|
| `DISCOVER` | answers with its tag, §4 — only when not in a running round |
| `ASSIGN_ADDR` by tag | takes NUMBER and slot, writes the cells, `ACK` |
| `BUSCFG` | takes the item; on the third, locks and computes the deadline (§4) |
| `SYNC` | loads the index at the captured edge, starts the sensor phase, starts streaming |
| `TICK` | ignored while running; the phase source on a rejoin |
| `END` | §4 |
| `HARD_RESET` | the NUMBER to 15, the matrix to the identity, the ranges and the mode to the defaults, the present-mask cleared; then a reset — the node comes up fresh at 38 400 |
| `RESEND frame · unix.0` | the frame from the circular buffer, in the node's own slot |
| `GET reg` | up to 16 bits under `GET`; a wider register as a DATA frame, `kind` 5, the register number first |
| `SET reg · value` | up to 16 bits in `arg1 · arg2`; a wider register — `R_level` — arrives as a `kind` 5 frame, the register number first; written, applied, `ACK`; a setting that reaches the sensor marks the first affected frame `status` 5 CHANGED |

**The registers** — the house block and the house positions are the house map's
(`../core/blocks/modbus.md`, *The house map*):

| register | r/w | what |
|---|---|---|
| `0x0002 STATUS` | r | the node state (§3), the clock state, the present-mask, the ranging width last measured |
| `0x0003 RAW` | r | one unlevelled sample of every present sensor — 30 B, `kind` 5 |
| `0x0010 CFG 0` | r/w | ADXL355 range: 0 ±2 g · 1 ±4 g · 2 ±8 g |
| `0x0011 CFG 1` | r/w | ICM accel range: 0 ±2 g · 1 ±8 g · 2 ±16 g |
| `0x0012 CFG 2` | r/w | ICM gyro range: 0 ±15,625 dps up to 7 ±2000 dps |
| `0x0013 CFG 3` | r/w | SCL3300 mode 1..4 |
| `0x0020 CALIBRATE` | w | 1 starts §7 |
| `0x0021 RANGE` | w | arms the ranging turnaround for the next block, `arg1` the width in ticks |
| `0x0023 SELFTEST` | w | 1 runs the self-test in the next quiet window (§9) |
| `0x0038 REPORT` | r | answers with the `REPORT` frame itself, `kind` 6 |
| `0x003A HEALTH` | r | answers with the `HEALTH` frame, `kind` 8: the ADXL355 · ICM · SCL3300 · MCU die temperatures, int16 0,01 °C, then the error counters |
| `0xFF00 VERSION` | r | firmware version |
| `0xFF01 IDENT` | r | the house code |
| `0xFF02 TAG` | r | CRC-16 of the UID |
| `0xFF03 slots` | r | 1 |
| `0xFF04 HEALTH` | r | echo/CRC misses, resends served, clock losses, IWDG resets, I²C recoveries |
| `0xFF09 SENSORS` | r | the present-mask: bit 0 ADXL355 · 1 ICM · 2 SCL3300 · 3 RM3100 · 4 TMP117 · 5 the coil NTC |
| `0xFF0A VIN_WINDOW` | r/w | the supply window — minimum and maximum voltage in 0,1 V, maximum current in 10 mA; the SUPPLY flag outside it. The house register every unit with a power body carries; defaults 10,0 · 20,0 V and the run's measured load with margin |
| `0xFF0B REPORT_INTERVAL` | r/w | seconds between `REPORT` frames, default 60; 0 disables. The house register every unit carries |
| `0xFF10 ID` | r | the two `ID` codes read at boot |
| `0xFF13 DELAY` | r/w | the frames the unit holds before it sends — written by its parent at floor-up, 32 on a Bifrost port and 16 behind an Argus, and persisted; never below 8, the image's floor, because `RESEND` is answered from this buffer (`../core/PROTOCOL.md` §7) |

**Up the link unasked:** `ACK` after a critical command and the `REPORT` frame once a minute — nothing else. `ERROR` only answers a command that failed; a fault is the header's `FAULT` flag, and its detail waits in `HEALTH` for the head's `GET HEALTH` (`../core/PROTOCOL.md` §1, §5, §7).

## 9. Health, QC and self-test

**`HEALTH` per sensor**, one vocabulary: OK · SELFTEST_FAIL · NO_RESPONSE · DEGRADED, and the
present-mask — read on `GET HEALTH`; every entry other than OK raises the header's `FAULT` flag
until it clears. Held for every position from boot — *not fitted* is `NO_RESPONSE` with the
present-mask bit clear, never indistinguishable from a fitted sensor reading zero.

**Self-test, in quiet windows.** When the head asks (`SET SELFTEST`) and `nq-detect` is quiet:
the ADXL355's and the ICM's built-in self-test — a known internal force, the datasheet-bounded
delta on each axis — and the SCL3300's `STO` self-test output; a delta outside the bound is
`SELFTEST_FAIL`. The stream carries the disturbed samples with `status` 4 WARMUP for the test's
duration and the head drops them.

**Value QC, at the write cadence (once a second), never per sample:** a railed axis (±32767 on
two consecutive reads) → bit 1 *saturated*; a seismic axis without its ±LSB noise floor for a
second (a stuck sensor reads a constant) → bit 2 *dead*; a cross-sensor disagreement — the ICM
accelerometer's levelled vertical against the ADXL355's, more than 5 % of g apart for a second —
→ bit 3 *inconsistent*. A sensor flagged dead is soft-reset once, reconfigured, and its next
frames carry `status` 4 WARMUP; a second failure within a minute is `DEGRADED` in `HEALTH`, `FAULT` up.
**Flag, never fake**: the raw value always goes up.

**Watchdogs.** The IWDG at 1 s, fed from the main loop after each period's packing; a hang is a
reset and a rejoin (§4), counted in `HEALTH`. The CSS on HSE loss: NMI, HSI, mute, sensor clocks
stopped, rejoin on the returning clock.

**I²C recovery.** A slave holding `SDA` low (the TMP117 or the `INA238` after a glitch) is
released by nine `SCL` pulses and a STOP, then the bus is reinitialised; counted in `HEALTH`.
No rail is cycled for it.

## 10. Faults

| fault | what the node does |
|---|---|
| a sensor absent at boot | skipped; its six bytes zero; `HEALTH` NO_RESPONSE; the present-mask records it |
| a sensor stops answering in operation (`WHO_AM_I` read every minute fails, or no `DRDY` for 8 periods) | soft reset by register, reconfigure, `WARMUP`; on a second failure `DEGRADED`, zeros in its fields, `HEALTH`, `FAULT` up |
| the ADXL355 lost | the ICM's accelerometer becomes the seismic and levelling source until the next boot; `HEALTH`, `FAULT` up |
| the clock lost | CSS → mute → rejoin; no frame leaves off the clock |
| a frame's echo fails CRC | `HEALTH` counts; the frame waits in the circular buffer for `RESEND` |
| the supply outside `VIN_WINDOW`, or the `INA238` `ALERT` high (PC8) | the SUPPLY flag in the header until it clears; the node changes nothing — the source end decides |
| brownout | the BOR at 2,7 V resets the node; a reset is a rejoin |
| a `SET` to a register the node does not have | `ERROR` *no such register* |
| population changed since the persisted set | full enrolment forced — 38 400, silent, waiting for a sweep |

**Nothing on the node is switched to clear a fault**: a part with no reset pin is cleared by the
source end's `ENABLE`, which is a cold restart of the whole node (`../galvani/README.md`,
*`ENABLE`, `LINE_EN` and the three states*).

## 11. Persistence

The H523's own flash, in the fixed append-only cells of the node contract
(`../core/PROTOCOL.md` §7): a cell is id, value, check; writes append; the reader keeps the last
good cell per id; a full 8 KB sector erases and rewrites the live set; the live set is rewritten
once a year on its own.

| cell | content | written |
|---|---|---|
| the set | NUMBER, slot, `BUSCFG` (count, rate code, rung), one check over all | at enrolment |
| the matrix | `R_level`, nine floats | at calibration |
| the ranges | `CFG` 0..3 | on `SET` |
| the mask | the present-mask | at every boot where it changed |
| the mount | `M_mount`, nine floats, the identity by default — for a board laid out differently | never in the base build |

No sensor keeps NV state; the node writes every sensor from these cells at every boot. A set
that fails its check is no set.

## 12. The processor's budget

| resource | used | of |
|---|---|---|
| USARTs | 2 — USART1 the link, USART3 the echo | 7 |
| SPI | 3 — one per seismic sensor, the RM3100 sharing SPI3 by `CS` | 4 |
| I²C | 1 — I2C1: the `INA238` | 3 |
| I3C | 1 — I3C1 in I²C legacy: the TMP117 | 2 |
| DMA channels | 9 — SPI1/2/3 RX+TX, USART1 TX, USART3 RX, ADC1 | 16 |
| timers | TIM2 timebase · TIM4 ranging · TIM1 `EXT_CLK` · TIM3 `SYNC` · TIM15 `CLKIN` · TIM6 housekeeping | |
| interrupts, by priority | 0 the RXD capture (TIM2 CH2) · 1 the slot compare · 2 `INT_ADXL`, `INT_ICM` · 3 the SPI DMA completions · 4 the USART idle line · 5 I2C1, I3C1 · 6 TIM6 | |
| SRAM | circular buffer 32 × 40 B · DMA buffers · STA/LTA state · ~2 kB | 272 KB |
| flash | the image · the cells 8 KB | 512 KB |
| CPU | < 3 % at 128 Hz; `WFI` between interrupts | 2²⁷ |

**No interrupt sits in a timing path.** The frame's transmit starts on a timer compare, the
`SYNC` edge is a timer capture, the sensor clocks are timer outputs, the ranging turnaround is a
timer alone.

## 13. What is tested, and where

**On a PC, with the pins replaced by callbacks:** the enrolment state machine against a recorded
sweep; the deadline arithmetic for every rung and slot; the packing at every range with railed
and negative inputs; the levelling matrix against a synthetic gravity at 20 orientations; the
STA/LTA on a recorded event and on quiet noise; the QC rules on a stuck axis, a railed axis and a
diverging pair; the cells with a torn write and a full sector; the rejoin from a `TICK`.

**On the bench, against `HARDWARE.md`:** `EXT_CLK` at 2²⁰ ± 0 and `CLKIN` at 41,94304 kHz on a
counter; the ADXL355's `DRDY` within one `EXT_CLK` cycle of the `SYNC` edge; ICM and ADXL355
sample instants a constant apart over an hour; the frame's start bit within ±1 tick of the slot
deadline; the ranging turnaround at 256 ticks ± 1 over a hundred launches; a power-cycle rejoin
in under 4 s; 24 h at 128 Hz with zero fillers on a bench spur; the IWDG reset and rejoin after a
forced hang.

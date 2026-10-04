★ N.I.C. ★

# Tesla — the firmware, described

> **This document is the deliverable, not a description of code.** It says everything the node's
> firmware does, in the order it does it, with the numbers it uses — enough to write the build
> from, and enough to test it against. The record and the wire rule: [`BUS.md`](BUS.md); the
> timing, the strips and the classes: [`DETECTION.md`](DETECTION.md); the chain, the converter, the clock, the pins:
> [`HARDWARE.md`](HARDWARE.md); Pip, the other image on this board: [`pip/FIRMWARE.md`](pip/FIRMWARE.md);
> the frames, the opcodes and the node's contract: [`../core/PROTOCOL.md`](../core/PROTOCOL.md);
> the clock and the bus start: [`../core/blocks/nodbus.md`](../core/blocks/nodbus.md). Where this
> document and one of those differ, that one wins and this one is corrected.

## 1. What the firmware is

**One image, one job.** Three rods sampled at 2²⁰ SPS, a filterbank, a detector, a
constant-fraction timer and a classifier, producing 4 B event records eight to a frame. **The
SID channel is not in this image**: the same board under Pip's image tracks the carriers
(`pip/FIRMWARE.md`), and the one type 2 record Tesla emits is the floor. **`TIME OUT`, the second
data body, is populated on every board and this image leaves it idle** — `DE_PIP` low, USART2
closed; what a board is, is the image in it, as with Bifrost and Argus.

**The node is dumb where it counts and a DSP where it must be.** It ships candidates, not
answers: a signed log amplitude, a backward offset in ticks and a 2-bit type per event, and —
where the supplement NOD is armed — four bytes of **raw** geometry beside it (`BUS.md`, *The
supplement record*). **Polarity, flash grouping, true SNR and the location are the server's**, and
so is every correction to that geometry: the node measures a bearing in its own mechanical frame
and calibrates nothing. What the node decides it
decides because only it can: whether an impulse is locally generated, from the raw stream and
the mains phase underneath it.

| | |
|---|---|
| bus | NodBus, type **7**; **one slot, two, or three** — NOD 1 the event records, **NOD 2** the persistent-source states and the floor record, **NOD 8** the per-event supplement (`BUS.md`, *The supplement record*). **A supplement NOD is its base's NUMBER + 7**; `GET slots` answers 1, 2 or 3 |
| clock | the spur's 2²² on `OSC_IN`, PLL1 ×64 to 2²⁸; the converter's `CLKIN` 2²⁵ off MCO1; every sample on the network grid |
| the converter | ADS127L14, three of four channels, sinc4, OSR 16, `CLK_DIV` 1 → **2²⁰ SPS**, 24-bit, three SAI lanes into DMA |
| the record | 4 B, eight per 32 B payload, 128 frames/s — **1024 events/s, the ceiling** |
| thermometers | four NTCs on the internal ADC, for the µ(T) amplitude correction; the rods' never leave the node, the board's rides the `REPORT` frame |
| PSRAM | the classifier's background model and training records — a population; the fixed detector runs without it |
| load | ~14 % of the M7 at 2²⁸ for the always-on path, **~18,5 % at the 1024 events/s ceiling** (§12) |

## 2. Boot

| # | step | what happens | if it fails |
|---|---|---|---|
| 1 | reset | `CS_C` high, `START` low, **`RESET` held low** — the converter leaves reset 104 `CLKIN` cycles after the rise and `CLKIN` is not running yet, so the pin stays low until `LOCKED` — `DE` low, `DE_PIP` low — `TIME OUT` idle for the whole run; the IWDG at **1 s**; the MPU set: the capture ring in AXI SRAM non-cacheable, DTCM for the detector's state | — |
| 2 | RC | HSI; the flash cells read (§11): the persisted set, the strip placement, the notch list, the crosstalk matrix, the µ(T) curves, the thresholds, the task bits; the tag computed — CRC-16-CCITT over the 96-bit UID | a set that fails its check is no set: the node boots as fresh |
| 3 | the rails | `PGOOD` (PD14) read; the 1,8 V is the processor's own, so a boot is proof of it; the 5,3 V and the 5 V are read as the converter's `ALV_FLAG` in step 5 | — |
| 4 | the converter, found — **at `LOCKED`, once `MCO1` drives `CLKIN`**: `RESET` released, 104 cycles waited | SPI1, mode 1: `DEV_ID` (00h) read — **04h** is the ADS127L14, 06h the L18 (same package, same pins, the fourth channel pair); anything else and the node enrols with `HEALTH` NO_RESPONSE for the front | the L18 is accepted and run as an L14 |
| 5 | the converter, configured — **at `LOCKED`, before `START`** | `STATUS` (02h) **cleared first** — `ALV_FLAG` and `POR_FLAG` reset to 1, and `SPI_ERR` blocks every write but `STATUS` while set; then, over SPI1: sinc4, OSR 16, `CLK_DIV` 1, the status header by `DP_STAT_EN`, `DP_TDM` 11b (four lanes), the three input precharge buffers and the `REFP` buffer on, `REF_RNG` the high range for the external 4,096 V, `REG_CRC_EN`, `SPI_CRC_EN`, `CLK_CNT_EN`; every register read back; `STATUS` cleared once more and PD2's EXTI armed, so `ERROR` is high from here on | a read-back mismatch: `RESET` pulsed, the sequence repeated once, then `HEALTH` DEGRADED and the node runs the thermometers and the bus alone |
| 6 | the clock | the data body's `CLK` on `OSC_IN` validated — N clean periods at 2²² against the HSI — and **not taken** until `BUSCFG`; the converter's `CLKIN` is not started off the HSI: a ΔΣ clocked off a ±1 % source would run a phase the grid cannot place | no clock: RC, silent, waiting |
| 7 | IDs and the power body | ADC1 reads `ID_D` and `ID_P` once, ratiometric — the boards on `NB IN`, the data body, and `PWR IN`, the power body; into `HEALTH`. `ID_PIP` is read into `HEALTH` too and nothing is done with it — `TIME OUT` is Pip's. Then I2C1: the `INA238` on `PWR IN`'s board at 0x40 configured — `APOL` = 1, `ALERT` high on alarm and latched; PC8 on EXTI, rising edge | a ratio in no window is *unknown board*, `FAULT` up; the node runs. No answer where `ID_P` names a power board: `HEALTH` *no telemetry*, `FAULT` up |
| 8 | the thermometers | the four NTC dividers read once, 256× oversampled over 100 ms; the rod temperatures seed the µ(T) correction | an open or shorted divider (full scale or zero) is `HEALTH` DEGRADED for that rod; the correction for it holds its last value |
| 9 | enrol | §4 | — |

## 3. The states

```
   RESET ──▶ RC (38 400, silent) ──▶ ENROLLED ──▶ LOCKED (HSE, CLKIN running, SAI armed) ──▶ RUNNING
                 ▲                                          │                                    │
                 │◀── clock lost (CSS) ─────────────────────┴────────────────────────────────────┤
                 │◀── IWDG reset ────────────────────────────────────────────────────────────────┤
                 └── ENDED (END: the parts stop, the buffer is flushed and answered; then the clock and the feed go)
```

`RC` — the HSI, transmitting nothing. `ENROLLED` — NUMBER and slot(s) held, still at 38 400.
`LOCKED` — `BUSCFG` taken, PLL1 at 2²⁸, MCO1 driving `CLKIN` at 2²⁵, the converter out of reset,
found and configured (steps 4–5 of §2, every register written before `START`, because a write to
08h–50h restarts the channels), then started (`START` high) and streaming into the ring, the detector running on it, no frame sent yet.
`RUNNING` — `SYNC` loaded the index; events go up. `ENDED` — on `END`: the converter in standby
(`START` low), the SAI stopped, the ring flushed and answered, the rung and `CLKIN` kept, the
USART listening for as long as the clock lasts. **Clock lost** is a fault: the CSS raises an NMI, the node
drops to the HSI, mutes, stops `CLKIN` (`START` low first, so the converter is not clocked by a
dying PLL), and comes back through the rejoin (§4).

## 4. The bus — the unit's side

**The enrolment** is the node contract, as on every unit: `DISCOVER` at 38 400 answered once with
the tag in the time bytes after `tag × 10 µs`, listening first on `RXD_ECHO`, backing off on a
bad echo; `ASSIGN_ADDR` by tag gives NUMBER and slot, `ACK`; **`GET slots` answers 1, 2 or 3** —
2 when the second-NOD bit is set, 3 when the supplement bit is set too, and the card then gives
the further NUMBERs and slots to the same tag. **The supplement's NUMBER is not negotiated: it is
the base's + 7** (`BUS.md`, *The supplement record*), so the card allocates the slot and the
NUMBER follows from the arithmetic. `BUSCFG` (count, rate code 7, rung) locks HSE onto the wire, PLL1 to 2²⁸, the
USART to the rung, MCO1 on, the converter started; the deadline is `round start + (slot − 1) ×
width` for each slot the node holds. `SYNC`, on a second's boundary, is captured on TIM2 CH2
(PB3) and loads `unix.0 · frame` at that edge **less `ROUTE`** — 2 × the written route in ticks
of 2²⁸ before it, so the grid is the card's and not the delayed one (`../core/PROTOCOL.md` §7) —
and **the converter's sample counter is zeroed there**: from that edge every sample has an index `n` on the node's grid, 8192 samples to the
frame, 2²⁰ to the second.

**The slot.** A TIM2 compare starts USART1's DMA transmit of the 40 B frame the emission stage
assembled (§7); `DE` up for the frame, down after the last stop bit; the echo on USART3 checked
when it ends; a miss counted in `HEALTH` and the frame held in the circular buffer of `DELAY` frames it is sent from. **Each
NOD is one compare a round, one buffer, one frame** — three NODs are three of each, on the one
USART.

**The gaps:** `GET`, `SET`, `RESEND` and `HARD_RESET` in the node's block, as §8. `TICK` ignored
while running; the phase source on a rejoin. **Ranging** on `SET RANGE`: after the next DATA
frame PB6/PB7 switch to TIM4, `DE` up, one-pulse mode — CH2's capture starts the counter, CH1
raises the return at `CCR` = **512 ticks** of 2²⁸ (1,91 µs) and drops it at the width the card
asked for; CH2 in PWM-input mode captures the incoming pulse's width for the `STATUS` register;
the pins go back. No interrupt in the path.

**The rejoin** after any reset: clock and a persisted set → HSE locked, the USART on the rung,
listen; a good CRC proves the rung, a second of nothing tries the other; the phase from a
neighbour's frame or the card's `TICK`, captured on TIM2 CH2; the node fires in the next round
with `status` 1 REJOINED. **The converter is restarted at the phase edge**, `START` low then
high, so its sample counter and the frame counter share one origin again. No clock → RC, silent.
No set → 38 400, silent, a sweep.

**`END`**: `START` low, the SAI stopped, the detector idle, what is in the ring flushed and the
answer sent. The USART keeps listening until the clock goes, so a CONTROL frame addressed to the node
brings it back if the shutdown is called off; otherwise the node stops on its watchdog when
the clock drops and **the returning feed is a boot**. No rail on the board is switched
(`../galvani/README.md`, *Three states*). If the node is brought back instead, the converter restarts on the
next `TICK`.

## 5. Time on the node

**One counter and one index.** TIM2 at 2²⁸ (3,7 ns a tick) is never reset; CH2 captures every
start-bit edge on `RXD`, compares fire the slots and advance `frame`. The converter's samples
carry no time of their own — their time is their **index `n` since `SYNC`**, because `CLKIN` is
2²⁸ ÷ 8 off the same PLL and `f_DATA` = `CLKIN` / (2 · 16) = 2²⁰ exactly; sample `n` was taken
at `n · 2⁻²⁰ s` after the `SYNC` edge, and the frame it belongs to is `n >> 13`.

**An event's time is a backward offset**, not a stamp: the CFD gives the event's position as a
sample index plus a sub-sample fraction (§6); the emission stage converts it to **ticks of 2⁻²⁰ s
backward from the start of the frame that carries the record**, and the **16-bit field spans
8 frames**, which Tesla's own buffer of 32 frames, `0xFF13 DELAY`, holds with room.
**A project tick is one sample here, by construction** — the chain runs at 2²⁰ SPS — so the field
counts whole samples.

**The arithmetic is done in quarter-ticks and rounded once, at the end.** `LAT` is 3,34 µs and
the CFD's fraction is known to about 4 ns; both are finer than a tick, and rounding either one
first would throw away a correction that is free. So the emission stage works in **2⁻²² s** —
`fine = ((frame_start_n − n_event) << 2) − fraction + LAT`, the sample difference shifted by two,
the CFD's fraction's top two bits taken — and the field is `offset = (fine + 2) >> 2`, one
round-to-nearest at the very end. That single rounding is the ±0,48 µs the budget carries
(`../core/blocks/gps-pps.md`, *The precision contract*); nothing upstream of it rounds at all.

**`LAT` — the chain's own delay, a constant of the image.** Sample `n` describes the rods as
they were `LAT` earlier: the ADS127L14's delay at sinc4, OSR 16 — **the filter's group delay,
half its impulse response, `2 · OSR − 2` = 30 modulator cycles = 60 clocks of 2²⁵ = 1,79 µs, plus
the datapath's pipeline, 39 clocks = 1,16 µs, which is what the datasheet's latency-time table
carries over the impulse response at every OSR: 2,95 µs, 12,4 ticks of 2⁻²²** (the table's
4,9 µs at 32,768 MHz is sync to the first settled conversion — the whole response plus the
pipeline — and is not the delay of an edge) — plus the analogue front end's group delay in the
band the CFD times the edge in, taken from the computed chain in `HARDWARE.md` (§2 — under
0,5 µs above 262 kHz, where a sferic's onset has its energy: **≈ 2 ticks**). **14 ticks by
default, from the datasheet and the worksheet, no bench**; `0x0019 LAT` is writable if a bench
ever refines it — a step on the input and a count of conversions to settle is the check, and the
pipeline's 5 ticks are the term it would move.
What `LAT` cannot remove is the curve's slope across the band — an edge whose energy sits lower
is timed later by up to the curve's spread, ±0,25 µs, and that is the term in the budget
(`../core/blocks/gps-pps.md`, *The precision contract*). A
record older than 8 frames when its frame comes up is dropped and counted.

**Two clocks the node checks.** `CLK_CNT` (03h) advances 2²⁰ counts a second; read one second
apart on the PPS-aligned frame boundary it returns the identical byte, and a difference is
`HEALTH` *converter clock*, `FAULT` up with the two values. The PLL's lock is the CSS's business.

## 6. The always-on path — from the SAI to the event

```
   three SAI DMA channels ──▶ THE RING — AXI SRAM, 3 rods × 10 ms × 4 B per sample, non-cacheable,
                               half-transfer interrupt every 5 ms (5120 samples a rod)
        │
        ▼  per half, per rod:
   [8 b status][24 b data], the data taken by one SBFX; the status byte's CH_ID checked against the lane,
   RPT_DATA clear, MOD_FLAG read as the sample's clip
        │
        ▼  the crosstalk matrix — 3×3, measured at commissioning, inverted once; identity by default
        │
        ├──▶ THE LADDER — halfband decimators, one octave a stage: 2²⁰ · 2¹⁹ · 2¹⁸ · 2¹⁷ · 2¹⁶ · 2¹⁵ · 2¹⁴
        │        └─ the 2¹⁷ stage (0–65 kHz) feeds the detector, the amplitude and the background
        │
        ├──▶ THE STRIPS — three 16 kHz cuts from stage 0 by mixing and decimation: 8–24, 248–264, 496–512 kHz
        │       by default, each anywhere in 5–512 kHz by SET; the notches — up to 8 biquads per rod on
        │       the site's notch list — sit ahead of the strips
        │
        ├──▶ THE DETECTOR — an envelope on the 5–50 kHz product of the 2¹⁷ stage against a rolling background (a 30 s median of
        │       the envelope, updated every second): a sample above background × 4 arms an event;
        │       the event closes when the envelope falls under × 1,5 or after 2 ms
        │
        ├──▶ THE CFD — on stage 0 around the arming sample: a 16-tap windowed-sinc interpolator that
        │       also carries the inverse-sinc droop correction reconstructs the edge; the time is where
        │       the reconstructed rising edge crosses 0,5 × the local peak, less LAT (§5); the same per strip, so
        │       three edge times per event and their differences
        │
        ├──▶ THE AMPLITUDE — the peak of the 10–50 kHz product, signed, corrected by the rod's µ(T) curve
        │       and the chain's gain, then to 13-bit log magnitude in 2⁻⁶ dB with the sign
        │
        └──▶ THE CLASSIFIER — per event, ~50 µs: the features below → type 0 lightning · 1 interference ·
                3 UFO; then the source tracker (§7)
```

**The features the classifier reads**, and nothing else: the delay between the three strips'
edges (dispersion — a far sferic arrives with its bands tens of µs apart, a local source with
none); the rise time from the CFD; the decay envelope over the next 200 µs; the spectral slope
from the ladder; and the event's phase within the mains cycle, from the envelope-lock test (§7).
The first cut is a rule set on those features; a learned classifier drops in behind the same 4 B
contract and needs the PSRAM for its model. **Nothing steady reaches the classifier**: mains
harmonics, switchers and carriers sit in the background estimate and never arm the detector.

**Levels.** The detector's multiple, the release ratio and the event's maximum length are
registers (§8); the site's noise floor sets the rest through the background. The floor itself
goes up once a second as a carrier record at frequency 0 (§7).

**The thermometers.** Every 10 s the four NTC dividers are read, 256× oversampled across 100 ms,
whole periods of 50 and 60 Hz, and the three rod temperatures update the µ(T) amplitude correction from
the per-unit curve in the cells; the fourth watches the electronics. No temperature rides the
data: the board's rides the `REPORT` frame, the rods' the `HEALTH` frame.

**The converter's status**, on the falling edge of `ERROR` (PD2, EXTI) and never polled: `STATUS`
read over SPI1 and cleared; the flag marks every record of the ring half the DMA is writing — `ALV_FLAG` and `ADC_ERR` set `status` 2 SENSOR on the frame,
`REG_ERR` triggers a full reconfiguration (step 5) — which restarts the channels, so `START` goes low
for it and high again at the next phase edge — and `HEALTH` *converter registers*, `FAULT` up.

## 7. Emission — what goes on the wire, and in which slot

**The rule** (`BUS.md`, *What goes on the wire*): independent events ship one by one; a
persistent source ships as a state, one record every 32 frames for as long as it lasts.

**The source tracker.** Impulses that pass the classifier as *interference* or *UFO* are tested
for lock: intervals collected over **0,3 s**; a train whose envelope repeats at a multiple of a
fundamental the node has **learned from its own background** — 50 or 60 Hz and their harmonics,
or a DC-traction ripple at 300 / 600 Hz — and whose **envelope rate is at least 100 Hz**, and
stands **× 8 above the background** (a second, higher
multiple than the detector's × 4, which is what keeps wet corona out) earns a slot in the source
table: **eight slots**, keyed by fundamental, phase within the cycle and amplitude; a ninth
source is counted in `HEALTH` and not slotted. A slotted source emits one record every 32nd
frame, the offset+amplitude layout of its latest pulse, its type; it is dropped after **2 s**
without a pulse. A one-off UFO keeps its own record. Lightning is never locked and never slotted.

**The 100 Hz floor on the envelope rate.** A source that locks **below** 100 Hz is not slotted
and ships as **UFO**, records and all — it is not discarded, only not tracked. What that drops is
AC traction (16,7 and 25 Hz, envelopes at 33 and 50 Hz) and nothing else; DC traction's ripple is
at 300 or 600 Hz and stays. **It is also what makes the 0,3 s window sound**: 0,3 s is 30
intervals at 100 Hz and was only 10 at 33 Hz, which is thin for a circular-variance lock. The
window is unchanged — the floor is what makes it enough. The reasoning is in `DETECTION.md`
(*What the classifier sees*).

**One NOD, two, or three.** With *one NOD* everything shares the eight slots under the priority
**lightning · arcing · UFO**, the floor behind them, and **there is no supplement**. With *two*,
NOD 1's payload carries lightning records only and NOD 2's the source states and the floor
record. With *three*, **NOD 8 carries the supplement** — its payload is position-parallel to
NOD 1's, slot for slot, in the same frame, and a slot whose NOD 1 counterpart is empty is
emitted as zeros. NOD 8 is never populated with one NOD set.

**Filling the frame**, once per period, before the slot compare: records are taken from the
event queue in time order, converted to backward offsets against the frame about to be sent,
packed as two little-endian 16-bit words — **word A the whole 16-bit offset**, word B `sign << 15 | magnitude << 2 |
type` — eight to the payload, zeros in empty slots. Nothing straddles the word boundary. A record
whose offset would exceed 2¹⁶ − 1 ticks is dropped and
counted. **No count, no overflow flag, no `Vin` byte**: an empty slot is zero, eight of eight is
the overflow flag, and the supply rides the `REPORT` frame.

**Filling the supplement**, in the same pass and from the same queue entry, so the two payloads
cannot drift: per event, four stores — the azimuth from the three rods' µ(T)-corrected peaks
through a Clarke transform and one `atan2` (the only new arithmetic in the image, ~55 cycles);
the rise time from the CFD, logged; and the two strip deltas, `t(mid) − t(lo)` and
`t(hi) − t(lo)`, **saturated to int8** at ±127 ticks rather than wrapped. A row carries no type
tag — `NUMBER > 7` is what says it is a supplement, and the slot index is what says whose
(`BUS.md`, *The supplement record*). **Nothing about NOD 1's payload changes**, and a
supplement is never emitted for a slot NOD 1 left empty.

**The floor record.** Once a second, a carrier record at frequency 0, type 2: word B the
detector's rolling background as 14-bit log in 2⁻⁶ dB. It is the one type 2 record this image
emits; the carriers themselves are Pip's (`pip/FIRMWARE.md` §6), in the same record shape.

**The report frame.** Once a `REPORT_INTERVAL`, 60 s, NOD 1's block carries the `REPORT` frame
behind its data: `VBUS` · `CURRENT` raw from the `INA238` · `NTC_PCB`, int16 in 0,01 °C · 0
(`../core/PROTOCOL.md` §5).

## 8. The control plane

| op | what the node does |
|---|---|
| `DISCOVER` · `ASSIGN_ADDR` · `BUSCFG` · `SYNC` · `TICK` | §4 |
| `END` | §4 |
| `HARD_RESET` | NUMBER to 15, the strips to their defaults, the notch list and the source table cleared, the crosstalk matrix to the identity, the task bits to *emission on, one NOD, no supplement*; the µ(T) curves stay — they are characterisation; then a reset |
| `RESEND frame · unix.0` | from the circular buffer, in the node's slot |
| `GET reg` | a byte under `GET`; wider as `kind` 5 |
| `SET reg · value` | written, `ACK`; a change that moves a strip or a threshold marks the first affected frame `status` 5 CHANGED, and the head writes the new settings into the unit's archive from that frame |

**The registers** — the house block and the house positions are the house map's
(`../core/blocks/modbus.md`, *The house map*):

| register | r/w | what |
|---|---|---|
| `0x0002 STATUS` | r | the node state, the clock state, `CLK_CNT`'s last two reads, the ranging width |
| `0x0010 STRIP_LO` · `0x0011 STRIP_MID` · `0x0012 STRIP_HI` | r/w | strip centres in kHz, anywhere in 13–504 (the strip stays inside 5–512) |
| `0x0013 DET_MULT` | r/w | the arming multiple over the background, default 4 |
| `0x0014 DET_REL` | r/w | the release ratio × 10, default 15 |
| `0x0015 SRC_MULT` | r/w | the source-slot multiple, default 8 |
| `0x0016 TASKS` | r/w | bit 0 lightning emission · bit 1 the second NOD (states + floor) · **bit 2 the supplement NOD** — bit 2 without bit 1 is refused, since a supplement has nothing to hang on |
| `0x0017 XTALK` | r/w | the 3×3 crosstalk matrix, 9 × int16 in 2⁻¹⁴, `kind` 5 |
| `0x0018 NOTCHES` | r/w | the notch list, up to 8 × 16-bit frequency, `kind` 5 |
| `0x0019 LAT` | r/w | the chain's latency in **quarter-ticks (2⁻²² s)** — a finer unit than the record's field on purpose, because the constant is known to better than a tick and is applied before the one rounding (§5); default 14 from the datasheet and the worksheet; persisted |
| `0x001A SRC_FMIN` | r/w | **the envelope-rate floor in Hz, default 100** — a source locking below it is not slotted and ships as UFO (§7). A railway build works it down to 33 and gets its own archive epoch for it |
| `0x0020 CALIBRATE` | w | 1 runs the crosstalk calibration: the node injects nothing — it records 10 s of the three rods' mutual coherence on the site's strongest carrier and reports the matrix as `kind` 5 for the head to confirm and write back |
| `0x0021 RANGE` | w | arms the ranging turnaround, `arg1` the width |
| `0x0023 SELFTEST` | w | 1 runs the self-test (§9) |
| `0x0038 REPORT` | r | answers with the `REPORT` frame itself, `kind` 6: `VBUS` · `CURRENT` raw · `NTC_PCB`, the board's NTC, int16 in 0,01 °C · 0 |
| `0x0039 FLOOR` | r | the background, 14-bit log |
| `0x003A HEALTH` | r | answers with the `HEALTH` frame, `kind` 8: the MCU die temperature, the three rods' NTCs, the converter's `STATUS`, the error counters |
| `0xFF00 VERSION` · `0xFF01 IDENT` · `0xFF02 TAG` · `0xFF03 slots` · `0xFF04 HEALTH` · `0xFF09 SENSORS` · `0xFF10 ID` | r | as every unit: version, house code, tag, **1, 2 or 3**, the counters (echo misses, resends, clock losses, IWDG resets, converter faults by flag, dropped records, unslotted sources), the present-mask (bit 0 the converter, 1..3 rods, 4..7 NTCs, 8 PSRAM), the three `ID` codes |
| `0xFF0A VIN_WINDOW` | r/w | the supply window, the SUPPLY flag outside it — the house register (`../quake/FIRMWARE.md`) |
| `0xFF0B REPORT_INTERVAL` | r/w | seconds between `REPORT` frames, default 60; 0 disables |
| `0xFF13 DELAY` | r/w | the frames the unit holds before it sends — written by its parent at floor-up, 32 on a Bifrost port and 16 behind an Argus, and persisted; never below 8, the image's floor, because `RESEND` is answered from this buffer (`../core/PROTOCOL.md` §7) |

**Up the link unasked:** `ACK` after a critical command and the `REPORT` frame once a minute — nothing else. `ERROR` only answers a command that failed; a fault is the header's `FAULT` flag, and its detail waits in `HEALTH` for the head's `GET HEALTH` (`../core/PROTOCOL.md` §1, §5, §7). `ERROR` *calibration noisy* is such an answer; a converter clock or register fault is `FAULT`.

## 9. Health and self-test

**`HEALTH`**, on `GET HEALTH`, the `FAULT` flag up while any entry is not OK: OK · SELFTEST_FAIL ·
NO_RESPONSE · DEGRADED, for the converter (its `DEV_ID`, its
`STATUS` flags, its `CLK_CNT`), each rod's thermometer, the PSRAM (a read-back at boot).

**Self-test on `SET SELFTEST`**: the converter's register CRC re-verified, the ring's DMA
alignment checked (`CH_ID` against the lane and `RPT_DATA` clear on every lane), each rod's noise floor against
the characterised value ±6 dB — a rod 6 dB quieter than characterised is an open winding, 6 dB
louder a local fault; either is `DEGRADED` for that rod and the classifier weights it out.

**QC at the write cadence:** a rod railed for a frame (`MOD_FLAG` set in a sample's header, or the log
amplitude at its ceiling on the raw stream) marks `status` 6 CLIPPED on that frame — the blind circle is intentional and the
record still goes up; a rod without its noise floor for a second is *dead* and `HEALTH`, `FAULT` up.

**Watchdogs.** The IWDG at 1 s fed from the ring's half-transfer handler's return — a stalled
DMA is a hang; the CSS on HSE loss.

## 10. Faults

| fault | what the node does |
|---|---|
| the converter absent or wrong | `HEALTH` NO_RESPONSE; the node enrols and answers `GET REPORT` and `GET HEALTH` for the thermometers; no records |
| `REG_ERR` | full reconfiguration; `FAULT` up; the block's records dropped |
| `ALV_FLAG` | the block's frames `status` 2; the supply behind the 5 V is the station's to look at, in the `REPORT` frame |
| `POR_FLAG` in operation | the converter reset itself: reconfigure, restart at the next `TICK` edge, `FAULT` up |
| `CLK_CNT` disagrees | `HEALTH` *converter clock*, `FAULT` up; the node keeps running, the head decides |
| a rod's NTC open or shorted | its µ(T) correction frozen at the last value; `HEALTH`, `FAULT` up |
| the ring overruns (a half not processed in 5 ms) | counted; the half is dropped whole, so no record carries a torn edge |
| the clock lost | CSS → `START` low → mute → rejoin |
| a record older than 8 frames | dropped, counted |
| PSRAM absent | the learned classifier is not loaded; the rule set runs; `HEALTH`, `FAULT` up |
| `ALERT` on PC8 (EXTI) | the `INA238`'s reading latched into `HEALTH`, `FAULT` up; nothing to switch on this board — the source end acts on its own `ALERT` |

Nothing on the node is switched to clear a fault; the source end's `ENABLE` is the cold restart.

## 11. Persistence

The H7A3's flash, in the node contract's append-only cells (`../core/PROTOCOL.md` §7): a cell
is id, value, check; the last good cell per id wins; a full sector is rewritten; the live set
rewritten once a year.

| cell | content | written |
|---|---|---|
| the set | NUMBER(s), slot(s), `BUSCFG`, one check | at enrolment |
| the strips | three centres | on `SET` |
| the thresholds | `DET_MULT`, `DET_REL`, `SRC_MULT`, `SRC_FMIN` | on `SET` |
| the tasks | the three bits | on `SET` |
| the crosstalk matrix | 9 × int16 | on `SET` after the head confirms it |
| the notch list | up to 8 frequencies | on `SET` |
| the µ(T) curves | four × (a, b, c) of a second-order fit in 2⁻¹⁶ | at characterisation, over `SET`; `HARD_RESET` keeps them |

The PSRAM holds nothing persistent — the classifier's model and training records are loaded
from the head at boot over the tunnel-free path of `SET` blocks, or the rule set runs.

## 12. The processor's budget

| resource | used | of |
|---|---|---|
| USARTs | 2 — USART1 the link, USART3 the echo; USART2 on `TIME OUT` is not opened. **Three NODs are three TIM2 compares a round, three circular buffers, three frames — one USART** | 10 |
| SAI | 3 blocks — SAI2 A, SAI1 A, SAI1 B (synchronous to A) | 4 |
| SPI | 1 — SPI1 the converter's configuration port | 6 |
| OCTOSPI | 1 — port 2, the PSRAM | 2 |
| DMA | 6 — three SAI RX, USART1 TX, USART3 RX, ADC1 | 16 DMA1/DMA2 streams |
| MDMA | 1 — the PSRAM's model load | 16 |
| timers | TIM2 timebase — CH2 the capture, **CH3 the slot compare, one channel re-armed per slot whatever the NOD count**, CH1 Pip's only, CH4 free · TIM4 `NB IN` ranging · TIM6 the housekeeping second | |
| interrupts, by priority | 0 the RXD capture · 1 the slot compares · 2 the SAI half/complete (the ring) · 3 the USART idle lines · 4 `ERROR` (EXTI2) and SPI1 · 5 `ALERT` · 6 ADC1 · 7 TIM6 | |
| SRAM | the ring 3 × 2 × 5120 × 4 B = **120 kiB** in AXI SRAM · the ladder's state in DTCM · the strips' state · the event queue 256 × 16 B · the circular buffers, one per NOD, up to 3 × 32 × 40 B | 1,4 MB |
| flash | the image · the cells 16 KB | 2 MB |
| CPU | the ladder ~9 % integer, the strips ~3 %, the detector ~1,5 %, the CFD and classifier ~0,5 % at 100 events/s — **~18,5 % at the 1024 events/s ceiling**, of which the supplement's azimuth is 0,02 % | 2²⁸ |

**No interrupt sits in a timing path.** An event's time is a sample index; a late handler
cannot move it, it can only run out of ring, and the ring is 10 ms against a handler in the tens
of microseconds.

## 13. What is tested, and where

**On a PC, with the pins replaced by callbacks:** the ladder and the strips against synthetic
tones (flatness, the notches' depth); the detector and the CFD against synthetic damped-sine
sferics in recorded noise — the edge time within 0,3 µs at 20 dB SNR, no time-walk over 60 dB of
amplitude; the backward-offset conversion at frame edges including the borrow into the second;
the classifier's rule set on a labelled set of recorded events; the source tracker on a synthetic
100 Hz train (slotted), on a 300 Hz train (slotted) and on a **33 Hz train, which must NOT be
slotted and must come out as UFO**; the emission rules — priority under 1024 events/s, the
quarter-second states, the drop of an over-old record; the cells.

**On the bench, against `HARDWARE.md`:** `CLKIN` at 2²⁵ ± 0 on a counter and `CLK_CNT` identical
a second apart; three lanes aligned by the status byte; 65 mV at the chain input with the
converters running; a strike simulator's edge timed to ±1 tick against a reference capture; the
crosstalk calibration reproducing a known matrix; 24 h at 128 Hz with zero fillers.

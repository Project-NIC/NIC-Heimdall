★ N.I.C. ★

# Photon and Positron — the firmware, described

> **This document is the deliverable, not a description of code.** It says everything the
> firmware of the two scintillation boards does, in the order it does it, with the numbers it
> uses — enough to write the build from, and enough to test it against. What the units *are*:
> [`README.md`](README.md), [`../positron/README.md`](../positron/README.md); the record on the
> wire: [`BUS.md`](BUS.md); the boards, their pins and timers: [`HARDWARE.md`](HARDWARE.md),
> [`../positron/HARDWARE.md`](../positron/HARDWARE.md); the physics, the handover, the calibration
> chain and the dose duty: [`SCINTILLATION.md`](SCINTILLATION.md); the frames, the opcodes and the
> node contract: [`../../../core/PROTOCOL.md`](../../../core/PROTOCOL.md). Where this document and one of
> those differ, that one wins and this one is corrected. `Quark-Tubes`' firmware is
> [`../../tubes/FIRMWARE.md`](../../tubes/FIRMWARE.md).

**One image, two boards.** `Quark-Photon` and `Quark-Neutron/Positron` run one H7A3 image: the
converter streams both channels into a ring, every pulse is integrated digitally, the energy scale
is a ratio against the single-photoelectron ladder, and the board answers as one NOD on either
board — Photon, or Positron with the neutron count in its record.

---

## B1. What the firmware is

**One image, two boards, three quantities.** The converter streams both channels interleaved —
88 080 384 words a second on the pins, **44 040 192 samples a channel**, which word is which read once at
boot from a per-channel test pattern (§B2 step 8) — into a ring the DMA fills without end; the firmware finds every pulse on a decimated view of the ring,
integrates it at full rate, converts its area to keV — the screen channel sorts its peak instead — against the single-photoelectron ladder it
maintains from the SiPM's own dark counts, and ships one 32 B record a frame, every field an
accumulator reset on the second — or, in list mode, every energy. It holds each SiPM's bias by a servo on
the 1 p.e. peak, corrects the crystal's light yield by temperature, watches the SiPM/PIN ratio
on the gamma board, and in a burst reads the PIN as an ion chamber. It computes no dose rate in
sieverts and no spectrum leaves unasked.

| | `Quark-Photon` | `Quark-Neutron/Positron` |
|---|---|---|
| NODs | one — **Photon**, type 9 | one — **Positron**, type 11: channel 1 the two neutron counts, channel 2 the beta energy, one record; type 10 reserved for `Neutron` |
| channel A | the CsI(Tl) crystal's SiPM | the photomultiplier's anode — the two-screen stack, thermal and epithermal (`../neutron/HARDWARE.md`) |
| channel B | the crystal's PIN, through the `LTC6268` | the 25 mm plastic block's SiPM, on the face |
| the energy | the SiPM below ~500 keV, the PIN above ~700 keV, blended between; one channel leaves | each channel its own quantity, its own ladder |
| the ratio check | SiPM/PIN = the persisted constant, ~330 000 | none |
| the dose duty | the PIN in current mode during a burst | none — a burst is counted as it comes |
| bias servos | one | one — the block's SiPM; the photomultiplier's HV is Helion's kV source, built for `Neutron` |

## B2. Boot

| # | step | what happens | if it fails |
|---|---|---|---|
| 1 | reset | `CSB` high, `PDWN` (PG2) high, `SYNC` (PG3) low, the amplifiers' `PD` (PG7 · PG8) high, `BIAS_EN` (PE5, and PE6) low, the DACs at 0, `DE` low; the IWDG at **1 s**; the MPU: the ring in AXI SRAM non-cacheable, the pulse state in DTCM | — |
| 2 | RC | HSI; the flash cells read (§B11): the set, the mode, the constants `k` per channel, the ratio constant, the bias windows, the crystal tempco, the ladder targets; the tag | a set that fails its check is no set |
| 3 | the rails | `PGOOD` (PH2) read | low: `HEALTH` *rail*, `FAULT` up |
| 4 | IDs | ADC1 reads `ID_D` (PC0), `ID_P` (PC1) once, ratiometric against `VREF+` | a ratio in no window is *unknown board*; the node runs |
| 5 | the converter | `CHIP_ID` read over SPI3; `PDWN` low; 1,8 V parallel CMOS with the output mux on (`0x14`), twos complement, **clock divider ÷2 (`0x0B`)**, test pattern off, internal reference, `0x100` = 0x07; read back; **one `SYNC` pulse** — the ÷2 divider reset to a defined state; then the test patterns for step 8: channel A (`0x05` = 01) `0x0D` = positive full scale, channel B (`0x05` = 10) negative full scale | a mismatch: `PDWN` pulsed and repeated once; then `HEALTH` DEGRADED and the node runs the bus alone |
| 6 | the clock | the data body's `CLK` on `OSC_IN` validated at 2²² and not taken until `BUSCFG`; `ENC` never off the HSI | no clock: RC, silent |
| 7 | the amplifiers, the bias | `PD` low; `BIAS_EN` high with the DAC at the persisted setpoint; `IBIAS_MON` read every 100 ms until it is inside the persisted window, **2 s** | out of window: `BIAS_EN` low, `HEALTH` NO_RESPONSE *SiPM n*; the other channel runs |
| 8 | the ring | the PSSI in circular DMA into the 1 MB ring — 524 288 half-words, **5,95 ms** — with a half-transfer interrupt every 2,98 ms; **the A/B parity read from the first words** — positive full scale is channel A, negative is B, and the DMA's start is asynchronous to the converter, so this and not the `SYNC` is what fixes it; the test patterns cleared (`0x0D` = 0 on both); the two spare lines checked to read zero | the pattern not seen, or a spare line at one: `HEALTH` DEGRADED *bus* |
| 9 | the PSRAM | a pattern written and read back through OCTOSPI1 at 2²⁶ — the part answers or it does not | absent or wrong: `HEALTH` DEGRADED *PSRAM*; the two-record fallback runs |
| 10 | the thermometers | ADC1: `NTC_EN` high, the NTC dividers read once against `VREF+`, low again; the crystal's reading through the B-curve. I2C1: the `INA238` through the `PCA9306` configured; `ALERT` (PC8) an EXTI on the rising edge — high is the alarm, the 100 kΩ holds it low | a divider at either rail — open or shorted: `HEALTH` NO_RESPONSE *thermometer*; the tempco correction held at 1 |
| 11 | the ladder | 1 s of the dark stream read (§B6) before the first energy is believed; until then events ship with `status` 4 WARMUP | no ladder in 10 s (a third cumulant at zero: no dark counts, a dead SiPM or no bias): `HEALTH` DEGRADED *ladder* for that channel |
| 12 | enrol | §B4 | — |

## B3. The states

The NOD state machine of every H7A3 unit (`../../../marconi/FIRMWARE.md` §3): `RC` → `ENROLLED` →
`LOCKED` (`ENC` running, the ring filling, the ladder building, nothing sent) → `RUNNING`
(`SYNC` loaded the index; records go up) · `ENDED` on `END` (`PDWN` high, `PD` high,
`BIAS_EN` low, the ring flushed and answered, the USART listening) · the CSS NMI on a lost clock (`PDWN`
high first, mute, the rejoin). **`Quark-Neutron/Positron` is one NOD** — one board,
one clock, one converter, one NUMBER, one slot and one ring of records; the neutron channel is four
bytes of that record (`BUS.md`).

## B4. The bus — the unit's side

The node contract as on every H7A3 unit (`../../../marconi/FIRMWARE.md` §4), with **`GET slots`
answering 1 on either board** — one NUMBER, one TIM2 compare a round, one circular buffer, one frame
(`BUS.md`). `SYNC` on TIM2 CH2 (PB3) loads `unix.0 ·
frame` at that edge less `ROUTE` (2 × the written route in ticks of 2²⁸, `../../../core/PROTOCOL.md`
§7) — a tick count becomes a sample index exactly, **× 21 ÷ 128**, because 2²⁸ / fs = 128/21 — and **the ring's write pointer at that origin is sample index 0**: from there every sample
has an index `n` on the node's grid, **344 064 samples to the frame and 44 040 192 to the second**, and a pulse's
time is its index less **`LAT`** — the AD9251's pipeline, **9 encode clocks** from its datasheet
(204 ns at 21 × 2²¹; its 1,0 ns aperture delay is below a tick and not counted),
plus the front end's delay to the half-peak crossing, **one constant per channel in samples,
measured on the bench (§B13) and persisted** (`../../../core/PROTOCOL.md` §7). Ranging: TIM4, `CCR` = **512 ticks** of 2²⁸. The rejoin re-zeroes the index at
the phase edge.

## B5. Time on the node

**A pulse's time is a sample index and nothing else.** The ring fills at 88 080 384 words a second — 44 040 192 samples a channel — in
lockstep with the bus clock, the index is zeroed at `SYNC`, and the time of an event is the
index of the sample where its reconstructed leading edge crosses half its peak — **±1 sample =
±23 ns**, far inside the µs the station delivers, which is what puts a TGF against Tesla's
sferic on one clock (`SCINTILLATION.md`, *What a TGF is*). No interrupt is in that path; a
late handler runs out of ring, and the ring is 5,95 ms against handlers in the tens of
microseconds.

## B6. Acquisition — from the ring to the energy

```
   the ring ── every 2,98 ms half ──▶ THE SCAN: every 16th word, both channels, against the threshold
                                            │  (a pulse's 1,5 µs tail is ~66 samples; every 16th still sees it 4 times)
                                            ▼  on a crossing:
                                     THE PULSE: the full-rate samples from 32 before to 128 after the crossing;
                                       baseline = the mean of the 32 before; area = Σ(sample − baseline)
                                       over the tail; peak; the half-peak crossing interpolated to the sample;
                                       width at half peak — the pile-up cut
                                            │
                                            ├──▶ THE LADDER (SiPM channels): the scan's every-16th samples, in 256 µs blocks
                                            │      with no crossing in them, summed over the second as Σd, Σd², Σd³, Σd⁴;
                                            │      N = (4/3) × κ₄/κ₃ — the one-cell step in LSB, no cell resolved
                                            │
                                            ├──▶ THE ENERGY: E = (area / N) × k, keV — k per channel from the bench (the cells);
                                            │      PIN: E = area × k_PIN; Photon: blended by the handover band
                                            │
                                            ├──▶ THE CRYSTAL TEMPCO: E × (1 + β × (T − 20 °C)), β = +0,3 %/°C for CsI(Tl)
                                            │      (the light yield falls, the correction rises); 0 on the screens and the block
                                            │
                                            └──▶ THE RECORD: (index, E) into the NOD's event queue; the second's accumulators
                                                   updated — the count, the energy sum, the largest event, its band
```

**The threshold** is per channel, in photoelectrons, converted to area by the ladder: **20 p.e.**
default on a SiPM channel (a 200 keV deposit in the crystal is ~300), so dark singles never
trigger the event path; the PIN's is 3 × its noise, measured at boot from 1 ms of quiet
baseline. A pulse whose width at half peak exceeds the channel's clean width by 50 % is
**piled up**: counted in `HEALTH`, its energy recorded with `status` 6 CLIPPED on the frame
that carries it, not dropped — a burst is exactly the case where pile-up is the data.

**The ladder** is the energy scale and the servo's sensor (`SCINTILLATION.md`, *The calibration
chain*). It is a statistic of the dark stream between events and resolves nothing: the scan
already reads every 16th sample, and every **256 µs block** of them with **no threshold crossing**
goes into four accumulators — the sample less the block's mean, to the first four powers. Once a
second the cumulants are formed, `κ₃ = m₃` and `κ₄ = m₄ − 3m₂²`, and **`N = (4/3)·κ₄/κ₃`**, the
one-cell step in LSB; the converter's and the amplifier's noise are Gaussian and enter neither, and
no offset and no noise floor is subtracted. `N` is a **16-second running mean**. At 10³ events/s
three blocks in four are free of an event and a burst leaves none, so in a burst `N` is held. No
known line, no reference. `N` is reported in `LADDER` and a channel whose `N` moves more than 5 %
in a minute is `HEALTH` DEGRADED.

**The bias servo** holds `N` — the one-cell step in raw LSB — at the persisted
target: once a minute, the DAC steps by one code toward it; the loop's gain is one code a
minute, so it follows temperature and never hunts. `IBIAS_MON` is telemetry: read every second,
its rolling mean reported, and a rise by ×4 over the mean in one second is the *enormous
event* flag in the summary (`SCINTILLATION.md`, *the SiPM's own bias current*). The dark current
at the fixed `N`, corrected for temperature by `2^((T − 20)/10)`, is the ageing figure in
`DARK` — a diagnostic, never a correction.

**The handover, on `Quark-Photon`.** Every pulse above the PIN's threshold is integrated on both
channels; the energy is the SiPM's below **500 keV**, the PIN's above **700 keV**, and between
them a linear blend by energy. The **ratio** of the two areas on every such event is averaged
over a minute and compared with the persisted constant: a departure by more than **10 %** is
`HEALTH` DEGRADED *handover* — something moved and no reference was needed to see it. The PIN's
own ruler is the cosmic muon at ~14 MeV: the daily histogram's peak above 10 MeV is compared
with the bench value and reported in `MUON`, never applied.

**The dose duty, on `Quark-Photon`.** The PIN channel's baseline — the mean of the every-16th
samples per microsecond, four of them — is a current-mode reading at 1 µs resolution and runs
always; a running 100 ms mean of it is the leakage baseline and is subtracted. When the SiPM
channel reports **more than 50 events in a frame, or any pile-up in three frames running**, the
node is *in a burst*: the current-mode waveform from **10 ms before to 50 ms after** the
onset — 60 000 × int16 at 1 µs, stamped by the onset's index — is the **burst record**. It is
assembled in a **120 kB staging block in SRAM1/2/4** (the 10 ms before it from a 20 kB
pre-trigger ring of the baseline stream, the 50 ms after as it comes) while the front measures,
and **copied to the PSRAM by MDMA only after the burst closes** — under a millisecond on the
octal bus, so the OCTOSPI carries no edges during a measurement. The PSRAM holds the records as a
ring of **256** (30 MB) for `GET`; two bursts under 60 ms apart are one record, by the same rule
that opened it. **Without the PSRAM fitted the staging block is the record and one more beside
it — two records, the newest kept** (`HARDWARE.md`, *The PSRAM*). The frames meanwhile carry
the counts as they come. Nothing on the wire changes shape.

**The capture window, on `Quark-Photon`.** The burst that starts the dose duty also opens a
**photonuclear window**: the event stream from **10 ms to 60 ms** after the onset is histogrammed
on its own, 256 bins over the channel's range, and kept beside the burst record for `GET`; a second
histogram accumulates **60 s** from the onset for the 511 keV line, whose source — `¹³N`'s β⁺ — has
a 9,97 min half-life and is a slow excess rather than a pulse. Nothing is decided on the node: both
histograms go out as they are and the lines are found downstream (`SCINTILLATION.md`, *The
photonuclear reaction has four signatures*). **The channel must be counting again by 10 ms after
the onset** — the recovery is the requirement the whole window rests on.

**The block**, on `Quark-Neutron/Positron`: the same pipeline as a SiPM channel, with its own
ladder and its own `k`.

**The screens**, channel 1 of the same board: **the photomultiplier's anode through a charge stage
with τ ≈ 100 ns**, so each capture's ~200 ns prompt is a rise of its own on the output. **The scan
runs every 4th word on this channel** — a rise is ~200 ns, nine samples, and every 4th still sees it
twice. **Every rise is an event**: its peak is taken above the level the stage stood at in the
samples just before the rise — the baseline when the channel was quiet, the previous capture's tail
when it was not — and that peak, in ladder units, is the event's one number. The ladder is the
tube's **single-photoelectron peak** — a `9390B` quotes a peak-to-valley of 2, so it resolves — and it
tracks temperature, ageing and the HV's own drift through the `FBX` divider's tempco, as the SiPM's
ladder does. **The peak sorts the event**: under the **threshold**, in photoelectrons, it is noise or
a gamma — a gamma in a thin screen, the window glass or the PMMA is a few photoelectrons — counted
in `HEALTH` and not in the record; between the threshold and the edge, and above the edge, are the
two screens — the **epithermal** and the **thermal** count (`../neutron/HARDWARE.md`, *The
readout*). The threshold and the edge are bench values in ladder units, persisted, and the only
tunables in the path.

**Pile-up on the screen channel is counted, not flagged away.** A rise on a falling tail is an event
of its own and is measured from that tail. Two captures closer than ~300 ns are one rise, ~3 % of
events at 103 kcps; a peak between 1,6 and 2,6 times the screen's mean is counted twice, in the window
its half lands in. What no rule resolves — a peak above any single capture's range — is counted in
`HEALTH` as *over range*, emitted with `status` 6 CLIPPED, never dropped: a near stroke is exactly the
case where pile-up is the data. The photomultiplier's gain sags with rate on the rule that one percent
of divider current costs one percent of gain — at ~28 pC a capture on the 100 µA divider, 0,1 % at
3 600 events/s, 1 % at 36 000/s (`../neutron/HARDWARE.md`, *The base*) — so nothing
is corrected below the persisted rate, and above it the window edges, being ladder ratios, move with
the gain and the sort survives. **In a TGF's gamma flood the tube is in deliberate gain collapse
behind its lead (`../neutron/HARDWARE.md`, *The lead shield*) and this channel is not
measuring at all**; what it owes there is to say so, and to be counting again within milliseconds.

## B7. Emission — what goes on the wire

**One 32 B record a frame, one NUMBER a unit** (`BUS.md`).
Little-endian; **every field is an accumulator reset on the second boundary**, so frame 127 carries
the second and the difference of two frames gives one frame.

| bytes | content | Photon | Positron |
|---|---|---|---|
| 0–1 | the count since the second began, `uint16` | ✓ | ✓ |
| 2–5 | the sum of those events' energies in keV, `uint32` — mean, median and dose all follow from it downstream | ✓ | ✓ |
| 6–7 | the largest single event of the second, keV, `uint16` | ✓ | ✓ |
| 8–9 | the thermal neutron count, `uint16` — the `⁶LiF/ZnS(Ag)` screen | — | ✓ |
| 10–11 | the epithermal neutron count, `uint16` — the second screen | — | ✓ |
| the rest | the energy bands, counts, `uint16` each; Σ must equal the count | **12** | **10** |

**On Photon the energy of each event is the handover's** — the SiPM's below 500 keV, the PIN's
above 700 keV, a linear blend between (*The handover*, §B6) — so the two channels leave as one
number. Positron has no PIN and no handover.

**No flags in the payload.** The frame header's `status` byte carries the unit's state at the
instant of the frame, and `6 CLIPPED` — *a value hit its range* — is what pile-up makes of a count;
the codes are `../../../core/PROTOCOL.md` §1's.

**List** — calibration and commissioning only:

| bytes | content |
|---|---|
| 0–31 | up to **16 energies** in keV, `uint16`, in order of arrival; zeros behind the last; a 17th and later in a frame are dropped and counted — a ceiling of 2048 particles/s |

**No mean, no median, no minimum, no separate dose accumulator and no histogram leave the unit.**
Each is either not addable across frames or already implied by the sum, the count and the bands;
the reasons are in `../../WHY.md`.

**Optional, not the base build:** a second NUMBER carrying 16 further bands, for 28 on Photon and
26 on Positron (`BUS.md`).

## B8. The control plane

The house table of every unit (`../../../marconi/FIRMWARE.md` §8), with these registers:

| register | r/w | what |
|---|---|---|
| `0x0002 STATUS` | r | the node state, the clock and converter state, per channel: bias in window · ladder valid · in a burst; the ranging width |
| `0x0010 MODE` | r/w | **0 the record** · 1 list |
| `0x0011 THRESH` | r/w | per channel, the threshold: in p.e. on a SiPM channel, in multiples of the noise on the PIN |
| `0x0012 K` | r/w | per channel, `k` in 2⁻⁴ keV per photoelectron (the PIN's in 2⁻¹⁰ keV per area count), `kind` 5 — the bench writes it |
| `0x0013 RATIO` | r/w | `Quark-Photon`: the SiPM/PIN area ratio constant, `uint32`, the bench writes it |
| `0x0014 BIAS` | r/w | per SiPM: the servo target for `N`, the DAC's setpoint as last settled, the `IBIAS_MON` window, `kind` 5 |
| `0x0015 TEMPCO` | r/w | β in 2⁻¹⁶/K per channel |
| `0x0016 WINDOWS` | r/w | `Quark-Neutron/Positron`: the screen channel's threshold and height-window edge in ladder units, the bench writes them |
| `0x0017 HANDOVER` | r/w | the two energies of the blend band, keV, default 500 · 700 |
| `0x0018 LADDER` | r | per channel: `N`, the second's third and fourth cumulants, the blocks that went in |
| `0x0019 DARK` | r | per SiPM: `IBIAS_MON`'s mean, the temperature-corrected dark current |
| `0x001A MUON` | r | the PIN's daily muon peak in area counts, and the bench value |
| `0x001B SPECTRUM` | r | per channel, the second's 256-bin histogram, `kind` 5, paged |
| `0x001C BURST` | r | the burst ring's headers (onset index, length, peak current) — up to 256 with the PSRAM, two without — and a waveform by index, `kind` 5, paged |
| `0x0021 RANGE` | w | the ranging turnaround, `arg1` the width in ticks |
| `0x0023 SELFTEST` | w | 1 runs the self-test (§B9) |
| `0x0038 REPORT` | r | answers with the `REPORT` frame itself, `kind` 6: `VBUS` · `CURRENT` raw · `NTC_X`, the crystal's NTC — on `Quark-Neutron/Positron` the block's — int16 in 0,01 °C · 0 |
| `0x003A HEALTH` | r | answers with the `HEALTH` frame, `kind` 8: the MCU die temperature, the SiPM's and the PIN's NTCs, the bias monitors, the error counters |
| `0xFF00`–`0xFF10` | r | `VERSION` · `IDENT` · `TAG` · `slots` 1 · `HEALTH` (echo/CRC misses, resends, clock losses, IWDG resets; events, pile-ups, list drops, bursts, ladder resets, bias excursions) · `SENSORS` (bit 0 the converter · 1 channel A's amplifier · 2 channel B's · 3 the thermometer · 4 SiPM A biased · 5 SiPM B biased · 6 the PSRAM) · `ID` |
| `0xFF0A VIN_WINDOW` | r/w | the supply window, the SUPPLY flag outside it — the house register (`../../../quake/FIRMWARE.md`) |
| `0xFF0B REPORT_INTERVAL` | r/w | seconds between `REPORT` frames, default 60; 0 disables |
| `0xFF13 DELAY` | r/w | the frames the unit holds before it sends — written by its parent at floor-up, 32 on a Bifrost port and 16 behind an Argus, and persisted; never below 8, the image's floor, because `RESEND` is answered from this buffer (`../../../core/PROTOCOL.md` §7) |

**Up the link unasked:** `ACK` after a critical command and the `REPORT` frame once a minute — nothing else. `ERROR` only answers a command that failed; a fault is the header's `FAULT` flag, and its detail waits in `HEALTH` for the head's `GET HEALTH` (`../../../core/PROTOCOL.md` §1, §5, §7).

## B9. Health and self-test

`HEALTH` per position, the house vocabulary, on `GET HEALTH`; the `FAULT` flag up while any entry
is not OK. **Self-test on `SET SELFTEST`**: the converter's
checkerboard pattern through every data line — `AD9251` register `0x0D` mode `0100`, the same
register that also carries the PN sequences
(`HARDWARE.md`, *The converter's port*); each amplifier's `PD` toggled and the channel's
noise floor seen to move by 10 dB; each bias switched off and the singles rate seen to fall to
zero and return. Every hour on its own: the converter's registers read back, the ladder's
validity, the handover ratio, the bias windows.

## B10. Faults

| trigger | action | reported |
|---|---|---|
| `IBIAS_MON` out of window for 1 s | `BIAS_EN` low for that SiPM, 5 s, back on; three in an hour and it stays off until `SET BIAS` | `HEALTH` NO_RESPONSE *SiPM n*, `FAULT` up |
| the ladder lost — a third cumulant at zero for 10 s | the last `N` held, energies flagged `status` 4 WARMUP | `HEALTH` DEGRADED *ladder* |
| the handover ratio off by > 10 % for a minute | flagged; energies still ship | `HEALTH` DEGRADED *handover* |
| a list frame overflows | the 17th and later dropped | `HEALTH` |
| the converter's read-back mismatches | reconfigured; a second mismatch is `DEGRADED` | `HEALTH` *converter registers*, `FAULT` up |
| the CSS fires | NMI: `PDWN` high, HSI, mute; the rejoin | `status` 1 REJOINED |
| `ALERT` high on PC8 | the reading latched into `HEALTH` | `FAULT` up |
| the IWDG expires | reset | `HEALTH` |

## B11. Persistence

The H7A3's flash, in the node contract's cells: the set (one NUMBER, its slot, `BUSCFG`,
one check) at enrolment · `MODE`, `THRESH`, `HANDOVER` on `SET` · **the bench set — `K`,
`RATIO`, `TEMPCO`, the bias targets and windows, the muon bench value, the delay constant
per channel, **the screen channel's threshold and height-window edge, in ladder units** — on `SET` at
characterisation, kept by `HARD_RESET`** · the last settled DAC setpoint per SiPM, once a day.
The PSRAM holds the burst records and nothing persistent.

## B12. The processor's budget

| resource | used | of |
|---|---|---|
| USARTs | 2 — USART1 the link, USART3 the echo | 10 |
| PSSI · SPI3 · I2C1 · ADC1 · DAC1 | the converter's data and registers; the `INA238` on I2C1 through the `PCA9306`; the NTC dividers, the IDs and `IBIAS_MON`; the bias setpoints | |
| MDMA | 2 — the ring, circular; the burst copy to the PSRAM, memory to memory, after a burst | 16 |
| OCTOSPI | 1 — port 2, the PSRAM, 2²⁶ octal DDR, idle while the front measures | 2 |
| DMA | 3 — USART1 TX, USART3 RX, ADC1 | 16 |
| timers | TIM2 timebase (CH2 the capture; one slot compare) · TIM4 ranging · TIM6 the housekeeping second | |
| interrupts, by priority | 0 the RXD capture · 1 the slot compares · 2 the ring's half-transfer · 3 the USART idle lines · 4 SPI3 · 5 ADC1 · 6 TIM6 | |
| SRAM | the ring 1 MB in AXI SRAM · **the burst staging block 120 kB and the 20 kB pre-trigger ring in SRAM1/2/4** · the pulse state and the ladders in DTCM · the event queues 2 × 256 × 8 B · the histograms 2 × 256 × 4 B · the circular buffer 32 × 40 B | 1,4 MB |
| PSRAM | the burst ring, 256 × 120 kB, written only after a burst | 32 MB |
| flash | the image · the cells 16 KB | 2 MB |
| CPU | the scan ~5 % (2,75 M compares a second per channel); the pulses ~2 % at 10³ events/s (a 160-sample integration each); the screen channel's every-4th scan, ~4 %, and its peak sort, a compare an event; the ladder ~1 %; the servo, the ratio and the summary under 1 %; the burst copy in a burst only | 2²⁸ |

**No interrupt sits in a timing path.** An event's time is a sample index; the ring holds
5,95 ms; the scan runs a half behind the MDMA and never catches it.

## B13. What is tested, and where

**On a PC, with the pins replaced by callbacks:** the scan and the integration against a
synthetic ring — a 1 µs pulse at 300 p.e., a 54 ns single cell, two pulses 400 ns apart
(piled up), a burst of 200 pulses in a frame; **the peak sort on the screen channel — a gamma under the threshold, an epithermal capture, a
thermal capture, a capture on the previous one's tail measured from that tail, two 200 ns apart
counted twice from the peak, each landing in its window or over range**; the ladder from a synthetic dark stream at 10⁵ and at 10⁷ cells a second, with crosstalk, an offset and the converter's noise, `N` within 2 % of the step put in; the servo's one-code step; the handover blend and the ratio alarm; the dose
baseline and the burst copy; the record's accumulators and Σ(bands) equal to the count; the
list packing and the frame-0 layout; the NOD's queue and slot; the register map; the cells.

**On the bench, against `HARDWARE.md` and `SCINTILLATION.md`:** the checkerboard through every
line; `N` read on each SiPM at the bench bias and stable to 1 % over an hour at constant
temperature, and equal within 2 % to the step seen directly in a histogram of single cells at
−20 °C in the chamber, where cells are seen alone; `k` measured against a known source per built head and
written; the SiPM/PIN ratio measured and written; the muon peak seen on the PIN in a day; a
burst of pulses from a pulsed LED into the crystal recorded as a current-mode waveform at 1 µs;
the delay constant per channel from the LED pulse's index against its trigger's, written;
each NOD in its slot at 128 Hz for 24 h with zero fillers on a bench spur.

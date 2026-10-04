★ N.I.C. ★

# Ceres — the firmware, described

> **Design-stage concept.** The figures are verified against the parts' sheets and the parts
> before the build is written. What the unit is: `README.md`; the measurement, the chain, the pins
> and the bench criteria: `HARDWARE.md`; the registers: `MODBUS.md`; the slave's side of the arm —
> the RTU frames, the sweep, the rate, the house block — is Babel's and the same here
> (`../babel/FIRMWARE.md` §4, §6). Where this document and one of those differ, that one wins.

## 1. What the firmware is

**A synchronous detector read at three frequencies, and one number out.** Once per interval
the firmware drives the electrode and the reference capacitor in turn at 2²⁰, 2²² and 2²⁴ Hz,
reads `I` and `Q` for each on two ADCs triggered by the same timer, computes the admittance's
two parts, separates the water's capacitance from the ions' conductance and the coating's
polarisation, compensates for temperature, and writes the volumetric water content into one
register and the soil temperature at the depth into the next. The driver and the follower are off between reads. It is a Modbus slave of type
**6**, one probe at one depth (`README.md`).

| the firmware does | on | how often |
|---|---|---|
| blinks the LED | PD4, 1 kΩ | one 50 ms blink a minute while healthy, two on a fault — the house code (`../core/HARDWARE.md`) |
| a read cycle — six `(I, Q)` pairs | TIM1, ADC1 + ADC2, `SEL` and `AMP_EN` | every **60 s** by default |
| reads the thermometer | the `TMP117`/`STS35` on I2C1, on the board under the glass, between cycles | every 10 s |
| answers the host | USART1, `THVD1450`, 19 200 8N1 — woken from Stop by the start bit | when polled |

## 2. Boot

| # | step | what happens | if it fails |
|---|---|---|---|
| 1 | reset | `DE` low; `AMP_EN` (PE6) low — the follower off (its `PD` is active-low, and the pull-down held it off through reset); TIM1's outputs idle low; `SEL` (PE5) to `C_ref`; the IWDG at **2 s** | — |
| 2 | RC | the 2²⁴ crystal validated against the HSI (± 1 %), **PLL1 at M 2 · N 32** — `f_PLL_IN` tops out at 16 MHz — and SYSCLK 2²⁷; the cells read (§5): the NUMBER, the interval, the calibration curve, the temperature coefficient, the open-pad offsets, the series `R` value; the tag | a set that fails its check is no set: NUMBER 3, the default curve, VALID clear |
| 3 | the ADCs | ADC1 and ADC2 in dual simultaneous mode, differential on `INP10/INN10` and `INP12/INN12`, hardware oversampling **256×**, triggered by TIM1 `TRGO`; calibrated once | — |
| 4 | the thermometer | the part found at 0x48 or 0x4A, one-shot mode, 8 averages; the first reading | silent: `TEMP` reads `0x8000`, the compensation runs on 20 °C and `COMP_STALE` is set |
| 5 | one cycle | a full read (§4); the `C_ref` read checked to lie within 20 % of nominal in magnitude | out: `STATUS` SENSOR_FAULT — the chain or `C_ref` is wrong |
| 6 | the slave | USART1 at 19 200, the RTU gap on TIM6; the slave at `6«2 \| NUMBER` — 0x18 for NUMBER 0 — listening | — |

## 3. The states

`IDLE` — **Stop mode, so the board does not heat the plate it measures** (`HARDWARE.md`, *The bowl*): the front end off, the `THS4541` in power-down, the thermometer in
shutdown, HSE off; **LPTIM on LSI** wakes the core for the thermometer every 10 s and for the
cycle every interval, and **the USART wakes it on a start bit** (kernel clock HSI16 kept for it),
so a poll is answered inside the RTU turnaround from Stop; `READING` — one cycle, ~53 ms,
HSE and PLL back on for it; back to `IDLE`. The `THVD1450`'s receiver
is the one part that never sleeps. A cut arm is a boot.

## 4. The read cycle

For the reference, then for the electrode — `SEL` switched between, 1 ms of settling after
each switch — and for each of the three frequencies:

1. `AMP_EN` high; TIM1 stopped with its outputs low, `ARR`, CH3's `CCR` and the repetition
   counter rewritten — **127 / 32 / 3 · 31 / 8 / 15 · 7 / 2 / 63** for 2²⁰ · 2²² · 2²⁴ Hz — and
   restarted (`HARDWARE.md`, *Timers*).
2. **1 ms** of settling: the drive's RC and the detectors' 1,6 kHz RCs.
3. The ADC pair triggered by TIM1's update through the repetition counter — every 4th, 16th or
   64th period, **262 144 Hz at every frequency**, phase-locked to the drive and far inside the
   converter's rate — for **2048 conversions, 7,8 ms**, hardware-oversampled 256× into eight
   results: one `(I, Q)` pair as their mean, 16-bit-class. The detectors' outputs are DC behind
   their 1,6 kHz RCs, so nothing asks for a conversion every period of the drive.
   Six pairs at 1 + 7,8 ms are the cycle's **~53 ms**.
4. Off: TIM1's outputs low.

`AMP_EN` low at the end. Six pairs; then the arithmetic:

```
for each f:   V_x  = I_x + jQ_x                 corrected by the C_ref pair at the same f —
                                                 the chain's gain and phase drop out, and the follower's
                                                 ~2,6 kΩ input load with them — the open-pad read is the second known load
              Z_x  = V_x · R / (V_drive − V_x)   R the series resistor, V_drive the C_ref read's implied drive
              Y_x  = 1 / Z_x = G_x + j·2πf·C_x
```

**The separation.** `C` barely moves across the three frequencies where `G` and the coating's
polarisation do (`HARDWARE.md`, *What it measures*): the firmware takes **`C` from the 4,19 MHz
read**, the slope of `C` against `log f` across the three as the polarisation term and
subtracts its extrapolation, and `G` at 1,05 MHz as the conductivity for `STATUS` and for the
salinity correction of the curve. The open-pad offset — the electrode node's stray, measured at
the bench with the pad open and stored — is subtracted from `C`.

**The finished value.** `VWC = curve(C_corrected)`, the calibration curve a table of 16 points
(`C` in fF against VWC in 0,1 %) in the cells, linearly interpolated, written at the bench
against gravimetric samples of the soil class the probe is for; then the temperature
compensation `VWC × (1 + κ × (T − 20 °C))`, κ per curve in the cells. Clipped to 0–1000.

**The registers are written under one copy**: `VWC`, `TEMP`, `RAW` (the corrected `C` in
fF) and `STATUS` change together at the cycle's end.

## 5. The registers

`MODBUS.md` is the contract:

| register | the firmware |
|---|---|
| `0x0000 VWC` | uint16, 0,1 %, the finished value |
| `0x0001 TEMP` | 0,1 °C, the soil temperature at the depth — the second value; `0x8000` absent |
| `0x0002 STATUS` | bit 0 VALID (calibrated, the last cycle sane) · bit 1 SENSOR_FAULT · bit 2 COMP_STALE (the thermometer's last reading older than a minute) |
| `0x0003 RAW` | uint16, `C` corrected, fF |
| `0x0010 INTERVAL` | r/w, seconds between cycles, 10–3600, default 60; persisted |
| `0x0011 CURVE` | r/w, 16 × (uint16, uint16), the calibration table; persisted; the bench writes it |
| `0x0012 KAPPA` | r/w, int16 in 2⁻¹⁶/K |
| `0x0013 OFFSET` | r/w, the open-pad `C` in fF, and `R` in Ω; the bench writes them |
| `0x0014 YQ` | r, the six `(G, C)` pairs of the last cycle — bench and diagnostics |
| `0xFF00+` | the house block (`../core/blocks/modbus.md`, *The house map*), type **6** |

## 6. Faults

| trigger | action | reported |
|---|---|---|
| the `C_ref` read out of 20 % of nominal | the cycle discarded, the values held | SENSOR_FAULT after three cycles; `HEALTH` |
| `C` outside the curve's range | clipped | VALID clear for that cycle |
| the thermometer silent | the last temperature used; `TEMP` reads `0x8000` after a minute | COMP_STALE |
| the IWDG expires | reset | `BOOT_COUNT` |

## 7. Budget and tests

USART1; I2C1; TIM1 and the two ADCs; TIM6 the gap; LPTIM1 the interval and the thermometer's 10 s; five GPIO —
a cycle of ~53 ms every minute, Stop otherwise; the front end off 99,9 % of the time and the board at ~5 mW between cycles (`HARDWARE.md`, *The bowl*). **On a
PC, with the pins as callbacks:** the admittance arithmetic against synthetic `(I, Q)` pairs
for known `G` and `C`, the polarisation slope removed, the curve interpolated at its ends, the
compensation, the registers. **On the bench, against `HARDWARE.md`:** `C_ref` against a second
`C_ref` on the pads reading unity within 0,5 % and 1°; an open pad reading `G` ≈ 0 at all three
frequencies; the stray under a third of the dry electrode's `C`; a read at 1,05 MHz not moving
with the buck's load; the probe in water, dry sand and a saline solution giving the curve's
end points and the salinity read in `G`.

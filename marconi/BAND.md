★ N.I.C. ★

# Marconi — the band and the targets

> **Design-stage concept.** The figures are verified against the parts' sheets and the parts
> before a board is made.

What Marconi listens to, why the band runs 0,5–16 MHz, and how a carrier 80 dB under the loudest
thing in the band is still read. The firmware that does it is `FIRMWARE.md`; the chain is
`HARDWARE.md`.

## The band, and why it reaches 16 MHz

**`foF2` lives between 3 and 13 MHz** — 3–6 at night, 8–13 by day with the solar cycle. A monitor
that stopped lower would see the bottom of the ionogram and watch the critical frequency leave the
top of its range for most of the day.

**The top is 16 MHz, and the reason is WWV, not `foF2`.** 12 MHz already catches `foF2` nearly
always. What 16 adds is two more permanent carriers and a third rung on the ladder:

| above 12 MHz | |
|---|---|
| **CHU** | 14 670 kHz |
| **WWV / WWVH** | **15 000 kHz** |

WWV then reads as a **triplet 5000 / 10000 / 15000**. Two frequencies give one absorption point;
three give a **slope**, which is a different measurement and a better one.

**The corner is at 16 MHz, not 15.** A 15,0 MHz band top would put WWV 15000 on the filter skirt,
where the response is steep and drifts with temperature. The same rule set the bottom of the band
against RWM 4996. What 16 MHz costs: S2's skirt is 1,68 octaves to the first fold; the 19 m
(15,10–15,80 MHz) and 22 m (13,57–13,87 MHz) broadcast bands are in band, which is more pressure
on `Rf1`; and the oversampling gain relative to the band is 3,4 dB.

**The low end is 0,5 MHz, because the band closes by day under D-region absorption and opens at
night, and that daily cycle is itself the measurement.**

## The targets — transmitters that never change

Time stations — WWV/WWVH/BPM 2500 · 5000 · 10000, RWM 4996 · 9996, CHU 7850, YVTO 5000; the
Russian channel markers — the Buzzer 4625 continuous, the Pip 5448 day / 3756 night, the Squeaky
Wheel 5367 / 3363,5; maritime NAVTEX 4209,5 and 518/490; NDB and DGPS beacons below the band.
**Broadcast AM is not a target**: its power, pattern and schedule move for reasons that are not
ionospheric.

**The markers' day/night pairs are one transmitter on two frequencies.** Their ratio cancels the
transmitter's power and the antenna, leaving absorption against frequency on one path. That turns
a level log into a measurement, and it is why one loop is enough: the azimuth factor is identical
for both frequencies of a pair and divides out.

**No transmitter database is shipped.** A slow background sweep promotes any tone that persists —
still there an hour later — to a monitored slot, tracked by its frequency.

**A revisit every 10–30 s.** The fastest thing worth catching is an SID onset, seconds;
acoustic-gravity waves after a large quake run 2–10 min, TIDs 10–60 min. Scintillation, 0,1–10 Hz,
does not fit and is not a target.

## One channel carries the whole band — the processing gain carries the range

A megawatt broadcaster and a distant time station sit 60–80 dB apart on one converter, and it does
not matter, because a carrier's level is read from **one 128 Hz bin**, not from the whole band:

```
524 288 samples at 2^26 SPS = 7,81 ms -> 128 Hz bin
processing gain = 10·log(33,554 MHz / 128 Hz) = 54,2 dB
```

| | per-bin range below full scale |
|---|---|
| the converter alone — its 79,0 dBFS SNR plus the 54,2 dB | **133,2 dB** |
| with this front end (86 nV/√Hz against the converter's 13,7) | **117,1 dB** |

So a carrier 80 dB below the loudest sits **37 dB above the per-bin floor**.

**What the spread still costs is clipping and intermodulation**, and no filter after the gain fixes
either:

- **clipping** — the sum of everything in band must fit; the lever is `Rf1`, set once for every
  site: 6 dB of headroom for 1,00 dB of SNR, 10 dB for 2,52 dB (`HARDWARE.md`, *Intermodulation
  in S1*);
- **intermodulation** — a full-scale carrier puts products at −90 dBFS, the converter's SFDR, and
  the front end's own are worse, so a weak carrier 80 dB down is close to them. **This is the one
  real limit, and it is answered in firmware.**

**A distortion product looks like a transmitter.** Marconi reads a carrier's frequency and level
and demodulates nothing, so distortion does not degrade the measurement — except that the
background sweep, which promotes any tone still present an hour later, would promote a permanent
broadcaster's second harmonic just as readily. **The fix is a frequency test and a level test,
not linearity**: a candidate on `2f`, `3f`, `2f₁ − f₂` or `2f₂ − f₁` of anything on the strong list
is a suspect — those frequencies are computed, not guessed — and a suspect is rejected when its
level follows its parents', because a product moves two or three decibels for each of theirs and
a transmitter fades on its own path (`FIRMWARE.md` §6). **`5000 → 10000 → 15000` is the case the
level test exists for**: WWV's triplet sits exactly on the second and third harmonic of its own
5000, and only its independent fading tells it from distortion.

**This is what professional direct-sampling HF receivers do**: one wide anti-alias low-pass, a
switchable attenuator and the converter's dynamic range. Their switched preselectors — Perseus
ships ten filters, Elad eight — keep a local blowtorch from intermodulating and are used one band
at a time; a passive ionogram needs the whole band at once, so that part does not transfer.

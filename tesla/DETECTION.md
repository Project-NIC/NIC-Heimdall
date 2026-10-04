★ N.I.C. ★

# Tesla — detection

> **Design-stage concept.** The figures are verified against the parts' sheets and the parts
> before a board is made.

What the node does with the 2²⁰ SPS stream: how it times an edge, how it cuts the band, what
separates a stroke from an arc, and where the node's decisions stop and the server's begin. The
firmware that runs it, register by register, is `FIRMWARE.md`; what goes on the wire is `BUS.md`.

## Timing — constant-fraction, on the rising edge

**A strike is timed by constant-fraction discrimination**, at a fixed fraction of the local peak on
the rising edge:

- **amplitude-independent** — a fixed threshold fires later on a weak stroke, which is pure
  time-walk;
- **early on the edge** — less dispersion than the peak, which arrives late and smeared.

**The rise is under one sample period** at the 512 kHz band top (~0,7 µs against 0,954 µs), so the
edge lands between samples more often than on one. Nothing is lost — the band is fully determined
at 2²⁰ SPS — but recovering it takes a **band-limited reconstruction, a 16-tap windowed-sinc
interpolator**, not a two-point fit, and **the same convolution carries the inverse-sinc droop
correction**. It is paid per event, not per sample: a few hundred thousand operations a second at
the 1024 events/s ceiling.

**The fine value is the node's; the record ships whole ticks.** The CFD reaches ~0,3 µs at 20 dB
SNR; the record rounds once, at the end, to ±0,48 µs (`BUS.md`).

**Why µs and not ns.** 1 µs is 300 m of light travel, and at the scale the network locates
strikes 300 m is below the ferrite and front-end edge distortion and the propagation smearing of a
distant sferic; the band's rise is the real floor. Timing precision is rise / SNR, so the floor is
what one impulse buys; a source that repeats buys √N by coherent averaging, which is why a
mains-locked arc can be timed better than any single stroke.

## Processing — one ladder, and it follows the silicon

The converter streams into a DMA ping-pong ring (`HARDWARE.md` §8).

| stage | when | cost (H7A3 at 2²⁸) |
|---|---|---|
| **dyadic filterbank** — halfband decimators, one octave a stage | always on | **~9 % integer**, 19 % float |
| **detector** — envelope against a rolling background, on a decimated stage | every ring half | ~1,5 % |
| **classifier** — features, FFT if wanted | on a trigger | 50 µs an event — 0,5 % at 100 events/s |

**A ladder of halfband decimators is a filterbank**, and each stage halves the rate and gives an
octave:

| stage | rate | band | what reads it |
|---|---|---|---|
| 0 | 2²⁰ | 0–2¹⁹ | the edge, the CFD |
| ÷2 | 2¹⁹ | 0–262 kHz | |
| ÷4 | 2¹⁸ | 0–131 kHz | Pip's image: the carriers |
| ÷8 | 2¹⁷ | 0–65 kHz | the detector (5–50 kHz) and the amplitude (10–50 kHz) |
| ÷16 | 2¹⁶ | 0–33 kHz | |
| ÷32 | 2¹⁵ | 0–16 kHz | |
| ÷64 | 2¹⁴ | 0–8 kHz | |

**The difference between two adjacent stages is a band-pass**, so every octave costs one
subtraction, and the whole ladder sums to twice the first stage — 50 MMAC/s across three rods at
2²⁰ — because each rung runs at half the rate of the one above.

**Not a continuous FFT.** It costs 15 % and gives less: it returns the spectrum of a block, not
when inside the block the event happened, so it cannot replace the time-domain path; and `Δt · Δf`
is bounded — a ~100 µs sferic in a 1024-point window of 1 ms smears and dilutes, while a shorter
window makes the bins coarse. The filterbank spends that bound the other way: short windows high
up where the transients are, long ones at the bottom where the carriers are.

## The strips — three narrow cuts, and the strongest feature is time

**What separates a sferic, an insulator discharge and an arc is not the level in one band:**

| feature | what it separates | cost |
|---|---|---|
| **delay between bands** | **a distant sferic from a local source** — the waveguide is dispersive near cutoff, so a far sferic arrives with its bands tens of µs apart (the *tweek*); a local source arrives with none | three CFDs instead of one |
| **mains synchronism** | insulator, arc, inverter — all phase-locked to the mains | timestamp modulo 20 ms |
| rise time | a slow arc from a sharp discharge | the CFD has it |
| decay envelope | a single discharge from a ringing one | a few samples |
| spectral slope | — | free from the ladder |

**The first row is why three strips beat one FFT**: an FFT gives the slope and destroys the
between-band timing, the strongest feature on the list.

**Three strips of 16 kHz, low, middle and high, inside 5–512 kHz**, cut from stage 0 by their own
mixers. A narrow strip sees only its own 16 kHz of noise; what lies between the strips never
enters the analysis.

| | width | default | edges |
|---|---|---|---|
| **low** | 16 kHz | centre 16 kHz | **8 – 24 kHz** |
| **middle** | 16 kHz | centre 256 kHz | **248 – 264 kHz** |
| **high** | 16 kHz | centre 504 kHz | **496 – 512 kHz** |

**The site places them, not the design.** PV inverters switch across 16–100 kHz, MW carriers fold
in from above, and what is clean in Bohemia is occupied in Texas, on Kamchatka or in Australia. A
station is commissioned with a spectrum in hand and each strip goes anywhere inside 5–512 kHz; at
binary centres the mixers cost nothing (±1, ±j) and the CORDIC makes an off-grid centre nearly
free. What a strip's height costs in alias protection is the table in `HARDWARE.md` §2 — the
high strip at its default is the exposed one, and moving it down buys protection back fast.

**Moving a strip is a `SET`**, one register a strip (`FIRMWARE.md` §8), and it is a config in the archive:
every feature is computed in the strips, so a record means something only against the placement
that produced it, and the head writes the new placement into the unit's file from the frame it
took effect (`../core/PROTOCOL.md` §5). The low end does not move: 5 kHz is where sferic energy peaks
and the long paths live.

## What the classifier sees — three classes of impulse

A sferic is **impulsive and broadband**; steady noise — mains and harmonics, switchers, carriers —
is narrowband or periodic and sits in the detector's rolling background, so it never reaches the
classifier. What does reach it is everything impulsive, and it splits three ways.

**Lightning.** Sharp onset, characteristic decay, and **random arrival** — nothing in the sky is
synchronised to anything.

**Arcing on a power line.** Also impulsive and broadband, so the pulse shape does not separate it;
**the envelope does**. A gap fires near a voltage peak, so the train is amplitude-modulated by the
mains and repeats at a harmonic of it:

| gap | fires per cycle | envelope on 50 / 60 Hz |
|---|---|---|
| phase to earth | 2 | 100 / 120 Hz |
| one phase pair | 2 | 100 / 120 Hz |
| all three pairs | 6 | **300 / 360 Hz** |

The test is **"is the envelope periodic and locked to something the site runs on"**, and the
mains is only one candidate:

| source | fundamental | envelope near | tracked |
|---|---|---|---|
| mains | 50 · 60 Hz | 100 / 200 / 300 · 120 / 240 / 360 Hz | **yes** |
| **AC traction, 25 kV 50/60 Hz** — CZ (south/east), FR, UK, CN, IN, TR, RU | **50 · 60 Hz** | **100 · 120 Hz** | **yes — identical to mains** |
| DC traction, 3 kV / 1,5 kV — CZ (north/west), NL, IT, PL, ES | rectifier ripple | 300 Hz (6-pulse) · 600 Hz (12-pulse) | **yes** |
| AC traction, 15 kV 16,7 Hz — DE, AT, CH, SE, NO | 16,7 Hz | 33 Hz | **no — UFO** |
| AC traction, 11 kV 25 Hz — legacy US | 25 Hz | 50 Hz | **no — UFO** |

**The fundamental is learned from the background, not compiled in** — the learning tells a 50 Hz
grid from a 60 Hz one and six-pulse ripple from twelve. **Envelope rates below 100 Hz are not
tracked**, and a source locking there ships as UFO: that drops two legacy railway systems and
nothing else. The world's standard AC catenary is 25 kV at the grid's own frequency, envelope
100 Hz, so a failing catenary insulator is located by the same machinery; DC traction stays at
300 or 600 Hz. A fault on the two dropped systems is mechanical and cleared by protection in a few
cycles — nothing to trend — and **0,3 s of intervals is 30 firings at 100 Hz** against 10 at
33 Hz, too thin for a circular-variance lock. A build that wants 16,7 Hz writes `SRC_FMIN` down to
33 and takes its own archive epoch (`FIRMWARE.md` §8).

**Arcing is kept, not discarded**: a rising rate on a line is a developing fault — a tracking
insulator, a cracked bushing, a loose clamp — and the station already computes everything the
count needs.

**Wet-weather corona is kept out by threshold.** It is mains-locked like an arc, so the envelope
test does not reject it, and a station with a dozen lines in view would fill its source table
every time it rains with an answer — *it is wet* — that the hygrometer already gives. **A source
earns a tracked slot only on a second, higher multiple of the running floor than the one that arms
the detector** (`SRC_MULT` 8 against `DET_MULT` 4). Corona sits close to the floor and mostly
raises it; an arc does not. A weak, distant arc falls through the same gap, and at that strength it
was never a usable product.

**UFO — Unknown Frequency Overload.** Everything impulsive that is neither of the above. An
explicit *don't know* is the honest output for what has not been characterised — an EMP signature,
an ionospheric disturbance and a town's HF rubbish look alike until a station has seen many — and
it is the only way the training set for the other two gets collected.

**The record is the contract; the classifier is implementation.** What leaves the node is an
offset, a signed amplitude and a 2-bit type. A hand-built rule set and a trained network produce
the same fields and nothing downstream can tell which ran, so the choice is firmware, sized to the
part under it: rules are what a first build is written against with no recorded waveform; a
learned classifier drops in behind the same record when the PSRAM carries its model; its ceiling is
the station's own recordings.

**What the node decides and what it does not.** Only the node can tell that an impulse is locally
generated — it has the raw stream, the mains phase under it and the returning signature, none of
which survives into 4 B. Only the server can tell where a strike was and whether it was real,
because that needs several stations. What the node discards nobody re-examines, and the UFO records
are what keeps that from being irreversible.

## A grid-only build — the same board, the same record, other registers

**The unit is sized for what is most common**: the 100 Hz floor, three strips, eight source slots,
each the commonest case taken as the default. **Nothing in the hardware changes for a line
monitor** — the analogue path is a 5–512 kHz magnetic receiver either way — and the record does
not change either, so a grid-only archive reads with the same decoder.

| | the universal image | a grid-only setting |
|---|---|---|
| `SRC_FMIN` | 100 Hz | **33 Hz**, the sensible bottom — the lock window is a fixed 0,3 s, 10 intervals at 33 Hz |
| the strips | 8–24 · 248–264 · 496–512 kHz, sited on a clean spectrum | on the line — one below the PLC band, one inside it, one past the traps |
| `DET_MULT` · `SRC_MULT` | 4 · 8, sized to keep wet corona out | lowered — corona is the product when an insulator is trended |
| the source table | 8 slots, keyed by fundamental and phase | the same 8, every slot a line |
| lightning | the primary product | still emitted — the same detector |
| the priority | lightning · arcing · UFO | unchanged |

**Every row is a register or a task bit** (`FIRMWARE.md` §8) — a `SET` and an archive epoch. A
grid-only station does not stop seeing lightning; it stops putting it first.

## TGF correlation — the coupling to the radiation units

**Every node shares the one network clock**, so a strike and the gamma and neutron counts of the
`Quark` units carry the same time. **The counts are binned to the frame**: T0 is Tesla's, and the
radiation channel only says which bin its counts fell in. The network then TOA-locates each strike
across stations and correlates by amplitude — a weak strike makes no hard gamma, a strong one does
— so a TGF in a frame with several strikes is attributed to the strong one. That spatial and
temporal correlation of TGF and neutron production with specific strikes is the scientific product
of a dense grid.

## The SID channel is Pip's

The same band hears the VLF and LF transmitters — DHO38 23,4 kHz, GQD 19,6 kHz, NAA 24 kHz, DCF77
77,5 kHz, MSF and WWVB 60, RBU 66,66, BPC 68,5, JJY 40 — as steady carriers off the D-region, whose
level jumps or drops in a solar flare. **Tracking them is Pip's image on this board**, NodBus type
12 (`pip/README.md`). Tesla's image tracks no carriers: during the storms and arcing it exists for
they are the quietest thing in the band. Its one `type = 2` record is the floor at frequency 0.

**The clip blinds both images alike, and they answer it differently.** Tesla gives the blind circle
away — the network sees what the nearest station cannot. A carrier level has no network behind it,
so Pip gates instead and reports the fraction it gated (`pip/CARRIERS.md`).

★ N.I.C. ★

# Time — the GNSS anchor, Kronos, and the precision contract

> **Design-stage concept — nothing built.** The figures are verified against the parts' sheets and
> the parts before a board is made.

This document owns the station's absolute time: the precision contract and its budget, the GNSS
receiver and where it stands, how the second travels from the receiver through Kronos to the cards
and the head, what happens without a receiver, and why there is no RTC. The clock rates are
`nodbus.md`'s (*The network clock*); the Kronos board is `../../kronos/`.

## The precision contract — ±1 µs, delivered

**The whole station runs at ±1 µs delivered, symmetric.** A computed correction can overshoot as
easily as fall short, so the figure is a symmetric worst case and never a one-sided budget. The
119 ns tick is internal plumbing, never a product figure and never a target. **No sub-µs engineering
anywhere** — no matched-length traces, no temperature compensation of parts, no live path metrology
beyond the one the cards already run: **re-ranging, periodic and automatic, every `RANGE_INTERVAL`**
(`../../bifrost/FIRMWARE.md` §4). The one design rule: **lay the signal path out so the errors do
not stack** — short, direct, one capture per hop, worst-case sums under the microsecond at −40 and
+80 °C.

**In the base station only Tesla consumes it.** Sferic time of arrival is the one measurement whose
own resolution is µs-class — 1 µs ≈ 300 m of path. Every other channel is **frame-synchronous**: its
sample instant is the frame index, and where a sensor converts on its own schedule the unit stamps
that instant and interpolates onto the grid (Gauss: 4 ms of placement error is 7° of phase at 5 Hz,
0,18° interpolated). No unit but Tesla needs sub-frame engineering, and none is bought for one.

**The budget — every term, worst case, summed**, never root-summed:

| term | where it comes from | ± |
|---|---|---|
| the receiver's PPS against UTC | a timing-grade module, sawtooth-corrected — its datasheet | 30–50 ns |
| `CAB` + `INT` — the typed numbers' error | 1 m of cable is 5 ns; `INT` from the datasheet ±20 ns | 30 ns |
| Kronos's loop — the residual phase of PPS-K against the corrected edge | one 7,45 ns tick plus the servo's residual, estimated (`../../kronos/FIRMWARE.md` §13) | 50 ns |
| the time bus, Kronos → card | M-LVDS onto a 30–40 cm ribbon: the `DS91C176`'s 3,4 ns and the `THVD1450`'s 25 ns (typical, datasheets) are the four ticks the card subtracts; what remains is the receiver's spread to its 40 ns maximum (15 ns), the driver's part-to-part spread (1,3 ns), the pulse skew (0,4 ns driver, 3,5 ns receiver), the tap's place on the ribbon (≤ 2 ns) and a capture of ±1 tick | 30 ns |
| ranging — half the round trip's error | ±1 tick from the median of three launches (`../../bifrost/FIRMWARE.md` §4); the two pairs of one cable within ~1 % (25 ns at 500 m, 13 halved); the `ISO145x` driver 19 ns typical and 41 maximum, the receiver 36 and 60, over temperature and supply — each leg carries one of each, so the typicals cancel in the halving and what survives is the two ends' difference, 23 ns halved with one end at the corner; **on glass the 1×9 modules' spread, which their sheets do not give, estimated** | 45 ns copper · 60 ns glass |
| the unit — `SYNC` captured, the PLL off the wire, `ROUTE` and `SKEW` applied in whole ticks | one tick; the synchroniser and the board's channel difference are in `SKEW` | 10 ns |
| the record's quantisation — the project tick 2⁻²⁰ s | half a tick | **477 ns** |
| Tesla's CFD at 20 dB SNR — the measurement's own resolution, Tesla only | `../../tesla/FIRMWARE.md` §13 | 300 ns |
| Tesla's chain latency `LAT` — what remains is the group-delay curve's slope across the band the edge sits in | `../../tesla/FIRMWARE.md` §5 | 250 ns |
| **sum** | | **~1,26 µs on glass with Tesla · ~0,71 µs without** |

- **The station's own part — Kronos to the unit's `SYNC` — is ~150 ns even on glass**; what the
  contract spends is the record and, on Tesla, the physics of the edge and its own chain.
- **The record is the largest single term, and on Tesla the worst-case sum is past ±1 µs.** That is
  the consequence of a two-byte offset field reaching eight frames (`nodbus.md`, *Time in the
  frame*), and it is accepted: root-summed the same rows give 0,63 µs, and a return stroke's onset
  reaches two stations smeared by about a microsecond whatever the field does, so the last two rows
  and the record are not the independent terms this convention treats them as. **Every unit but
  Tesla stays inside the contract.**
- **Two rows are estimates** — Kronos's residual and the optical modules' spread; periodic
  re-ranging is what keeps the second one a constant over temperature. Every other constant is a
  datasheet's or a worksheet's.
- **Holdover is outside the table by design** — a TCXO on its temperature table leaves the
  microsecond within minutes, and the quality byte says so; the contract is for `LOCKED`.
- **The chain below the record delivers better than the contract, and the surplus is margin.** The
  distance terms — cable and fibre propagation, the barrier, the optical modules — are fixed per
  unit and ranged out, so what remains is edge detection plus the residual drift of what was
  ranged: tens of ns a hop, ~100 ns summed on the longest spur. The contract stays at ±1 µs
  anyway: one number no build has to defend is worth more than the margin it hides.

**Measure and cancel, do not chase precision.** Time of arrival uses differences between stations,
so an error common to both cancels. Stronger, and the rule here: **each station measures its own
delays** — the cable by ranging, the PPS quantisation by the sawtooth, the path constants — so an
error need only be known, not identical, and each station subtracts its own. Parts can change over
the years and the network stays coherent because every station calibrates itself. **The floor is
physics**: sferic propagation smears the waveform to ~µs and the network geometry locates to ~km, so
µs is the requirement; sub-µs internal wander never reaches the product.

**Global coherence is automatic** — GPS system time is steered to UTC and carried by every
satellite, so two stations need no common satellite to share time, to tens of ns. Per-event
coherence is regional anyway, a sferic being heard within VLF range: the worldwide network is a
mosaic of overlapping regional solutions, each stitched by GPS time. All stamps are GPS time in the Unix epoch — zone-free, leap-blind — and become UTC at export
(`../archive/HMC.md`).

## The GNSS receiver — two named types

**Standard NMEA is a common minimum, not a common contract.** Position and UTC read the same
everywhere; the configuration dialogue, the leap-second message and the delay constant do not —
u-blox speaks UBX beside NMEA and Unicore speaks its own. A firmware that admitted any receiver
would carry protocols for chips nobody fitted, so the station names two:

| type | where it sits | vendor protocol |
|---|---|---|
| **`M8N`** | **Polaris**, a bought u-blox NEO-M8N on a carrier on Kronos's socket; **Kronos checks and configures it at every boot** | UBX |
| **`UM980`** | **Sputnik**, which serves TEC and time from one receiver and configures it itself | Unicore |

**The type is a typed setting on Kronos, never a detection.** It decides the boot dialogue, the
leap-second source and the `INT` term of the delay table — which is therefore **a constant of the
type**, not an unknown of the build. It decides nothing about reading position and UTC. A build
that fits another receiver types it and brings its dialogue.

- **A station without Sputnik takes Polaris.** Its PPS is tens of ns to UTC and the GPSDO loop
  averages what is left, which is what a seismic or weather station needs; sawtooth and survey-in
  are refinements, not requirements. **A UM980 is not put there** — it is the ionosphere receiver,
  and a station with no ionosphere tier does not pay for it.
- **A station with Sputnik takes the UM980's time and adds nothing.** The UM980 has three COM ports,
  each with its own message configuration: COM1 serves Sputnik's own GNSS processing, **COM2 feeds
  Kronos a lean time stream** (`RMC`/`GGA`), COM3 is spare. Its PPS reaches Kronos's socket off the
  net Sputnik's H523 captures, **through a buffer and never through the processor**, and COM2 is
  relayed by that H523 — which gives the H523 the run's `RXD`/`TXD` and with them the ranging
  turnaround.
- **One receiver per station; more buy nothing.**

**What a receiver must be, to stand on Kronos's socket:** a hardware 1PPS within ~50 ns of UTC;
standard NMEA for position and UTC; the broadcast ionosphere model at least. Timing mode,
survey-in and sawtooth are refinements the loop already averages out.

**The antenna is part of the receiver**: a decent active antenna — ideally ~28 dB of LNA, a ground
plane, clear sky, multi-band if the receiver is — fed over its coax by the module board's bias tee.
**A lower gain costs signal-to-noise and nothing else**: time wants a fix, a fix wants four
satellites, and a multi-constellation receiver tracks several times that under a sky far worse than
a mast-top gives. A good chip on a bundled ceramic patch is still a ruined station.

## The receiver lives in the enclosure — always

**The antenna is sited for the sky; the receiver is not sited at all.** It sits in the head
enclosure and the only long thing is the coax, as the timing world does it: the mast carries the
antenna, the receiver is metres away in a cabinet, the cable is calibrated once. **A site whose own
mast cannot see the sky is sited wrong**, and the answer is to move the station, not the receiver.

- **No Sputnik:** Polaris on Kronos's `TIME IN 1`, centimetres of ribbon.
- **With Sputnik:** the node stands in the enclosure too; its COM2 and its PPS reach `TIME IN 1` on
  an in-box cable — the same data body Polaris plugs into (`TXD` · `RXD` · `CLK/PPS` · `ID` ·
  3,3 V · `GND`). **Kronos tells the two apart by the typed setting, not by the wire.** At ~20 cm
  only basic protection is needed; it is the board's.
- **Pip is the one exception**: its antennas are ferrite and cannot be in the box, so it stands
  where the band is quiet, on `TIME IN 2`, and **its run is ranged**.

**The coax stays short — ≤ 4 m is the target** (a low mast and a shallow vault): its delay is ~20 ns,
typed once as `CAB`, and the active antenna covers the loss. **No PPS or UART is ever run raw over
distance**; only the RF is long, and RF does not care. **No optocoupler on a PPS inside the box** —
one adds tens of ns of jitter and an isolated supply for nothing; a PPS leaves the enclosure only on
a Galvani board's channel B, isolated there like every other line.

**The capability for a remote receiver stays fitted, and the deployment rule is what leaves it
unused.** Every NOD link is the same link, so nothing is deleted for being unused in the base build:
both of Kronos's sockets range, Sputnik keeps its turnaround. A receiver behind a Galvani pair sends
its UART on channel A and **its PPS on channel B, reversed by the socket** — the sending end's socket
ties `B_DIR` to 3,3 V and Kronos's ties it to ground, so the board configures itself and no
processor pin reads it (`../../galvani/README.md`, *The reversed channel*); copper over metres, glass
over kilometres, the same pin and the same host firmware. On glass it costs a transmitter at each
end — both module sections, or a BiDi part at 2 Mb/s — and no laser is ever turned round. **Its
delay is then subtracted, and that is not optional**: at ~5 ns/m a kilometre is 5 µs, five times the
contract. **Kronos ranges the run itself**, as a card ranges a spur: an edge from a compare channel
of its PPS timer on `TXD`, turned round by capture and compare at the far H523, back on a capture of
the same timer — `2 × route + a constant`, ±1 tick of 2²⁷, no CPU in the path. The PPS crosses on
channel B of the same cable, so the measured route is its route; the two pairs' modules differ by
nanoseconds, accepted. Where a build cannot range, the run's length is surveyed once, or the ranged
number of a NodBus spur pulled alongside is taken. **A remoted receiver without a route in the
handset's corrections table reads NO-GO** (`../../mayak/handset/README.md`).

## The coax, the arresters and the bracket

**The coax takes two arresters, both DC-pass** — the bias-tee current reaches the LNA through both:
one at the antenna, one at the enclosure entry, bonded to the entry earth. The coax is the one
conductor on the station that cannot be isolated — its shield bonds the masthead antenna straight to
receiver ground — so the arresters are what define where a strike's potential rise equalises. The
upper one saves the LNA in the induced and nearby case; the entry one protects the receiver and the
station side. **The shield belongs to the internal system, top to bottom, and it is not the earth
anchor** — the anchor is the internal system's own bonding conductor, **hard-bonded to the common
earthing point**, one tie, no gap and no part in the path (`../../daedalus/CONSTRUCTION.md`). A few
square millimetres of braid is not a kA path and is never asked to be one. **So the antenna is
mounted on an insulating bracket**: a metal mount bonds the shield to the mast through the antenna
body, and a plastic enclosure has no feeder entry plate to throw that current off at. The arresters'
part class is `../../sputnik/HARDWARE.md`'s; every station carries this coax, so the rule is the
station's.

## Kronos — the clock, beside the head

**The network clock is made on Kronos**, a small H523 board beside the head (`../../kronos/`): the
TCXO, the disciplining PLL and the PPS capture live there, and the clock leaves on **the M-LVDS time
bus** to the cards and the Mayak. **A dedicated clock processor and not the head**: the head juggles
storage, the uplink and the Modbus tunnel under Wi-Fi and BLE jitter, and a separate processor
isolates the one hard-real-time job — PPS-disciplined clock synthesis — from that load. The heavy
GNSS work — RAW, corrections, satellite filtering — stays on Sputnik.

**The discipline:**

- **The oscillator is a TCXO at a power-of-two frequency** — 2²⁴, ±1 ppm (`../../kronos/HARDWARE.md`).
  With PPS present its absolute accuracy barely matters: `FRACN` steers out the offset and the PPS
  re-anchors every second, so a mediocre but measured oscillator ends as accurate as a precise one.
  **The value is what matters.** `FRACN` moves the VCO and every PLL output with it; what it cannot
  touch is the ratio between two outputs, which the integer dividers fix, and the timebase and the
  capture timer leave on two of them. A crystal's own odd factors sit in that ratio permanently —
  16,384 MHz is 2¹⁷ × 125, and the 125 forces a 25 into the divider chain — so the part is a power
  of two and every ratio on the board is a shift. **What the TCXO buys is holdover**: when the source
  is absent or coarse, the oscillator carries the time between corrections, an order and a half
  better than a plain crystal's ±50 ppm. A headline "1 ppm" is usually one line of the sheet; the
  real figure — initial, temperature, ageing, load pull, supply — is often two or three times it.
- **One 32-bit timer on the post-PLL clock, PPS-GNSS (or Pip's pulse) on input capture.** Ticks
  between two edges are the frequency error → the loop → `FRACN`. **The gate sets the resolution,
  not the tick**: one edge a second may be averaged as long as the loop likes, and a minute of it
  resolves far finer than the oscillator drifts. The capture's position is the **phase offset**,
  measured — the edge is never forced onto the PPS. **Measuring after the PLL steers what is
  actually distributed**, and needs no second timer.
- **The receiver's checks run on Kronos**, where the stream lands and the holdover decision lives —
  the station's one NMEA parser: ① the **interval**, PPS to PPS ≈ 1,000 s within the oscillator's
  ppm (a missing, extra or jumped pulse); ② **fix valid**, `GGA` quality > 0 or `RMC` status `A` (a
  no-fix, free-running PPS); ③ **the second increments by one** (rollover, spoofing, a sentence
  against the wrong pulse). A wrong or frozen date drops Kronos to holdover. The sentence for second
  *T* arrives tens of ms after that second's PPS, so it only **labels** the edge; the edge is the
  instant.
- **Holdover:** when the source pulse goes, Kronos **does not pretend to be locked** — `FRACN`
  follows its temperature table from the last good code, the time is flagged, and the record
  carries the flag and the trust tier, so no data is silently mistimed. The return to `LOCKED` is a
  slew, never a step (`../../kronos/FIRMWARE.md`).

**Kronos hands the station three things, on one 10-pin ribbon** — `GND · CLK+ · CLK− · GND · PPS+ ·
PPS− · GND · SDA · SCL · ATTN`, a tap per card and one for the Mayak, the last tap terminated by its
jumper:

- **PPS-K** — the second Kronos derives by integer division from its steered oscillator, on the PPS
  pair to **a capture input on every card and on the Mayak**, each behind its own `THVD1450`. A
  hardware edge latched in hardware at every customer and **never forwarded across an inter-chip
  link**. In holdover PPS-K keeps coming; the raw GNSS pulse, which stops, is nobody's input but
  Kronos's. The pulses are named by source: **PPS-GNSS** into Kronos, **PPS-K** out of it.
- **The clock, 2²²**, on the CLK pair — into every card's `OSC_IN` and out on a Bifrost's spurs
  undivided; the Mayak receives it and its firmware does not use it.
- **The label** — the full Unix second with its date, on the **I²C time bus**, multi-master, one
  bus along the ribbon. Kronos writes it to all once a minute and on a change of quality; a receiver
  counts the seconds in between on PPS-K, and a label that disagrees with the count is reported,
  not applied. The head writes when it has something — the coarse seed, the position, a change —
  and reads quality (locked · holdover · seeded) as a register. The cards only listen. Nothing
  time-critical rides it.

**`ATTN`**, tied to 3,3 V on Kronos and pulled down on a card, makes a card on the ribbon a Bifrost
and one off it an Argus. The in-box distances are ≤ ~0,5 m, so their delays are constants from the
datasheets — **four ticks of 2²⁷** for the time bus — and only the runs that leave the box are
ranged.

```
THE THREE NAMED SECONDS, AND WHICH WAY EACH ONE POINTS

  IN, to Kronos ─────────────────────────────────────────────────────────────────────
    GNSS receiver ─ lean NMEA ─▶ UART 1 ┐  the station's ONE parser lives here
                   └ PPS-GNSS ─────────▶│ capture ch 1 ┐ both edges land on ONE 32-bit
    Pip, where fitted ─ NMEA ──▶ UART 2 ┤              ├ timer, so ranking the sources
                       └ its PPS ──────▶│ capture ch 2 ┘ is a subtraction
                                        KRONOS

  OUT of Kronos — ONE 10-pin time bus ribbon, GND · CLK± · GND · PPS± · GND · SDA · SCL · ATTN,
                  a tap per card and for the Mayak, the last tap terminated by jumper ────────
    PPS-K ─ DS91C176, PPS pair ─┬─▶ the Mayak's capture pin          the head's time
                                └─▶ a capture input on EVERY card    is PPS-K + the label
    CLK 2²² ─ DS91C176, CLK pair ─┬─▶ every card's OSC_IN, and out on a Bifrost's spurs
                                  └─▶ the Mayak — received, unused by its firmware
    ATTN ─ 3V3 ─▶ a pull-down on every card: high = a Bifrost, off the ribbon = an Argus
    I²C — THE LABEL BUS, one, along the ribbon, multi-master:
        Kronos, once a minute and on a change of quality, to all — the Unix second, the date in it;
        the Mayak, when it has something — the coarse seed, the position, a change;
        quality (locked / holdover / seeded) is a register it reads. Cards only listen.
```

**Kronos emulates a GPS to the station**: an edge that says nothing and a slow label that says which
second it was. Boards derive network time by counting the distributed clock — 2²² on the wire, 2²³
on the board — never their own oscillators.

## The head's time

**The Mayak's time is PPS-K and the label, and nothing else.** PPS-K lands on a hardware timer
capture, immune to the head's load and its radio's jitter, and a software loop disciplines the
head's own UTC for its own stamps — the uplink, the logs, the second and position bookkeeping.
**Record time is not the head's job**: the card stamps the absolute second on the trunk record, and
the head does no index-to-second arithmetic. **The head carries no GNSS code at all** and gates its
own stamps on Kronos's quality register, not on NMEA it never sees.

**Liveness is PPS-K itself.** A presence watchdog on each receiver of PPS-K catches a lost edge, and
the head marks the time untrusted. The state of health tells two states apart: **"GPS lost"** —
holdover, data flowing, labelled; **"clock dead"** — the station mute, units on RC never transmit,
and the head reports it over the modem position, which runs off the head and not off the network
clock. **There is no takeover of the clock by the head**: its oscillator is tens of ppm, a takeover
path adds hardware and a failure mode for the least likely death in the box, and the head is not the
clock's parent. The answer to a dead Kronos is the spare board in the drawer.

## Without a receiver

**Seeded over the bus, never by a pulse.** A station with no GNSS and no Pip but coarse time — NTP
over the head's Wi-Fi or an LTE-M modem's session, or the network time an LTE-M modem reports — is seeded by **a write on the time bus**: the
head writes the coarse second, Kronos labels by it, holds its frequency on the TCXO alone and **tags
the time seeded**, a lower trust tier than either receiver. **The seed comes from something that
was itself synchronised**, never a free-running counter, or the label is a guess wearing a
timestamp. Kronos always keeps the frequency; a seed labels and never steers. No pulse crosses from
the head: an edge the head derived from NTP is not one to discipline a TCXO against. A station with
nothing at all runs relative-only — the shared clock still gives sub-µs between units, and only the
absolute anchor is missing. **A dead time bus link and a head reboot leave no label source until one
returns**, and the station is UNSYNCED and says so.

**A unit's absolute time rides the clock, never a PPS run of its own**: the distributed clock plus
the second the card completes. Its one distance term is the run's propagation and the barrier — a
fixed per-unit offset the card ranges and writes as `0xFF11 ROUTE`, and **the unit applies it
once**, taking its `SYNC` edge `ROUTE` early, so its frames arrive in their nominal slots and
neither the card nor the head does delay arithmetic (`../PROTOCOL.md` §7). An uncorrected 100 m
spur would sit ~0,5 µs off.

## No RTC — a drifted clock is worse than an honest gap

**There is no battery-backed real-time clock in the station, and no external clock part in any bill
of materials.** The absolute second arrives from a synchronised source — GNSS on Kronos, or the
longwave time where Pip is fitted — and until one is available the station is **UNSYNCED and says
so**.

A stopped clock is obvious; a drifted one is not. The station stands dead for some days, restarts
in winter on weak charging, and reports data stamped with a time that has quietly wandered, in
records that look completely normal. A commodity 32,768 kHz crystal runs about ±20 ppm; a
TCXO-backed part only slows that down. **Store only what is certain**: an honest gap can be filled
from the archive later, a wrong timestamp poisons the record permanently and silently.

**The on-chip RTC peripherals** — the S31's and the H523's — are counters for housekeeping: roughly
how long power was out, uptime, scheduling. **Nothing is stamped from them**, and nothing keeps them
alive across a power cut.

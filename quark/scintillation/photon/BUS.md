★ N.I.C. ★

# The scintillation record — Photon and Positron on the NodBus

> **Design-stage concept — nothing built.** The figures are verified against the parts' sheets and
> the parts before a board is made.

> What the units are: [`../../README.md`](../../README.md), [`../positron/README.md`](../positron/README.md).
> The bus itself — framing, addressing, TDMA — is [`../../../core/blocks/nodbus.md`](../../../core/blocks/nodbus.md),
> which wins on any bus question; the tube unit's mini frame is [`../../tubes/BUS.md`](../../tubes/BUS.md).

**Photon (NodBus type 9) · Positron (11)** are classic NODs behind a Bifrost — counts *and*
energy do not fit an 8 B payload. **One unit, one NUMBER, one 32 B record a frame.** The contract,
decided:

| bytes | field | Photon | Positron |
|---|---|---|---|
| 0–1 | **count**, `uint16` | ✓ | ✓ |
| 2–5 | **the sum of the events' energies, keV**, `uint32` | ✓ | ✓ |
| 6–7 | **the largest single event, keV**, `uint16` | ✓ | ✓ |
| 8–9 | **the thermal neutron count**, `uint16` — the `⁶LiF/ZnS(Ag)` screen | — | **✓** |
| 10–11 | **the epithermal neutron count**, `uint16` — the second screen behind the PMMA | — | **✓** |
| the rest | **energy bands, counts, `uint16` each** | **12** | **10** |

- **Every field is an ACCUMULATOR reset on the second boundary.** Frame 127 carries the whole
  second, the difference of two neighbouring frames carries one frame, and a lost frame costs
  nothing because the next still holds the running total. **No frame is sent to close a second** —
  the second is already there.
- **Only what can be ADDED rides a frame.** A count, a maximum, a sum and band counts all add. The
  **mean** is that sum ÷ that count and the **median** is interpolated in the band crossing the
  50 % mark, both downstream and exact; the **dose is the same sum** × a calibration constant. A
  mean cannot be re-averaged over 128 frames and a median cannot be combined at all, so neither is
  sent. **Σ(bands) must equal the count** — the bands span the whole range, under- and over-range
  included, so that identity is an integrity check that costs nothing.
- **No flags in the payload.** The frame header already carries a `status` byte, one code at the
  instant of the frame, and `6 CLIPPED` is exactly *a value hit its range* — which is what pile-up
  makes of a count. The codes are `../../../core/PROTOCOL.md` §1's.
- **On Photon the two channels are ONE measurement, switched per event by its energy** — the
  SiPM's below 500 keV, the PIN's above 700 keV, a linear blend between (`FIRMWARE.md`, *The
  handover*). Two instruments with overlapping windows give one reading, not two series: below its
  ceiling the SiPM is the better instrument and above it distorts, and the PIN is the reverse. The
  ratio of the two areas is measured on every event that crosses both thresholds and watched
  against the persisted constant; a departure over 10 % is `HEALTH` DEGRADED, which is how a
  saturating channel reports itself without a second address.
- **On `Quark-Neutron/Positron` there is nothing to switch and no handover**: the board carries two
  charge-stage channels — the block's SiPM and the photomultiplier's anode — and no PIN at all. Its **25 mm plastic block** contains every beta in the band —
  Y-90's 2,28 MeV ranges 10,7 mm — and a contained Y-90 beta sits at **~2 % cell occupancy**
  against the ~30 % where the response bends, so there is nothing to hand over to
  (`../positron/HARDWARE.md`).
- **`Quark-Neutron/Positron` is ONE unit and one NUMBER**, `Positron`. The neutron channel has no
  energy to send — two screens, a threshold and two height windows give two counts, thermal and epithermal
  (`../neutron/HARDWARE.md`) — so it is four bytes of the record and not an address that would
  carry twenty-eight empty ones. **NodBus type 10 is reserved for `Neutron`**: a build that wants
  the channel on a NUMBER of its own takes type 10.
- **A build that wants a finer spectrum may take a SECOND NUMBER carrying bands alone**: 32 B of
  `uint16`, **16 more bands**, for 28 on Photon and 26 on Positron. It is not the base build and
  the firmware need not implement it — **the reason it is not default is the counting statistics,
  not the bytes**: at ~500 events/s a second spreads ~18 events over 28 bands, ±23 % Poisson,
  against ~42 and ±15 % over twelve. Finer bands earn their keep only integrated over minutes, and
  downstream can integrate twelve as well as twenty-eight, where a slot once spent is spent.
- **`List` stays, for calibration and commissioning only**: up to 16 particle energies `uint16` a
  frame in order of arrival, a ceiling of 2048 particles/s, the whole payload. The `MODE` register
  picks it.
- A TGF is counted in the same stream as ordinary particles — no special channel.

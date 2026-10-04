★ N.I.C. ★

# Ceres and Sakura — the ModBus contract

> **Design-stage concept.** What the units are: `README.md`, `sakura/README.md`; the board:
> `HARDWARE.md`; the firmware: `FIRMWARE.md`. The bus itself — framing, the ModBus arm,
> addressing — is `../core/blocks/modbus.md`, which wins on any bus question.

**Address:** the computed one, `TYPE«2 | NUMBER` — ModBus type **6** for Ceres, **0x18** for unit 0;
type **7** for Sakura, **0x1C** for unit 0 (`../core/PROTOCOL.md` §2, which owns the three type
spaces and their packings).

**The `0xFF00+` house block is the same on every MOD** — `VERSION` · `IDENT` · `TAG` · `HEALTH` ·
`NUMBER` rw · `BOOT_COUNT` · `STATUS` · `RATE` rw · `SENSORS` (`../babel/MODBUS.md`); no time
register, the arm carries no time. The map below is the house map every MOD answers
(`../core/blocks/modbus.md`, *The house map*): the reading at `0x0000`, the second value at
`0x0001`.

| Reg | Name | Meaning |
|---|---|---|
| 0x0000 | VWC ro · **WETNESS** on Sakura | **uint16, 0,1 %/LSB** — Ceres: volumetric water content; Sakura: leaf-surface wetness as a fraction of full scale. Compensated, the finished value |
| 0x0001 | TEMP ro | **0,1 °C signed** — Ceres: the soil temperature at the depth; Sakura: the temperature at the plate, a leaf temperature and not the air's. The second value; `0x8000` = absent |
| 0x0002 | STATUS ro | bit0 VALID · bit1 SENSOR_FAULT · bit2 COMP_STALE — the temperature compensation running on a reading older than a minute |
| 0x0003 | RAW ro | **uint16**, the corrected capacitance `C` in fF — bench and calibration diagnostics |
| 0x0010 | INTERVAL rw | seconds between read cycles, 10–3600, default 60; persisted |
| 0x0011 | CURVE rw | 16 × (uint16 `C` in fF, uint16 value in 0,1 %) — the calibration table, written at the bench; persisted. Ceres's against gravimetric samples of the soil class; Sakura's between the dry plate and the plate flooded |
| 0x0012 | KAPPA rw | int16, 2⁻¹⁶/K — the temperature coefficient of the curve |
| 0x0013 | OFFSET rw | the open-pad `C` in fF, and the series `R` in Ω — the bench writes them |
| 0x0014 | YQ ro | the six `(G, C)` pairs of the last cycle — bench and diagnostics |

**What Palatine polls and ships is the run `VWC · TEMP · STATUS · RAW`**, one contiguous block
(`../core/PROTOCOL.md` §5); the register numbers live here and never ride a frame. **The scale**:
0,1 %/LSB spans 0–100 % in 1000 counts against a capacitive sensor whose own accuracy is
percent-class — the LSB an order under the instrument, like every encoding in the house.

**One unit is one probe at one depth**, and it reports both of that depth's quantities; a
patrona's depths are separate units with their own NUMBERs, so depth never needs a channel.
**Sakura publishes a wetness, not a conclusion**: the wet and dry durations the disease models
want are the archive's arithmetic on this value, never a register.

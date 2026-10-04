★ N.I.C. ★

# Sakura — the firmware, as it differs from Ceres's

> **Design-stage concept.** The figures are verified against the parts' sheets and the parts
> before the build is written. The registers of both units: `../MODBUS.md`.

**The firmware is Ceres's, whole, and `../FIRMWARE.md` is the description** — the boot, the
60 s cycle in Stop mode on the LPTIM, the read at three frequencies, the de-embedding against
`C_ref` and the open pad, the compensation, the registers, the house block, what is tested. What
differs is the type, the name of the finished value, what the curve was written against, and one
correction.

| | Ceres | Sakura |
|---|---|---|
| the slave | type **6**, `6«2 \| NUMBER` — 0x18 for NUMBER 0 | type **7**, `7«2 \| NUMBER` — **0x1C** for NUMBER 0 |
| `0x0000` | `VWC`, uint16, 0,1 % — volumetric water content | **`WETNESS`**, uint16, 0,1 % of full scale — leaf-surface wetness |
| `0x0001 TEMP` | the soil temperature at the depth | the temperature at the plate |
| the curve | 16 points, `C` in fF against VWC, written at the bench against gravimetric samples of the soil class | 16 points, `C` in fF against wetness, written at the bench **between the dry plate and the plate flooded** |
| `G` at 1,05 MHz | the salinity correction of the curve | **the correction for a conducting film** — dew with dissolved salts reads in `G` |
| the bench | the probe in water, dry sand and a saline solution | the plate dry, misted and flooded, and a salted film read in `G` |

`STATUS`, `RAW`, the `0x0010+` cells — `INTERVAL`, the curve, κ, the offsets — and the `0xFF00+`
house block are Ceres's, answered on type 7.

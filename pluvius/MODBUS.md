★ N.I.C. ★

# Pluvius — the ModBus contract

> **Design-stage concept.** What Pluvius is: `README.md`; the board: `HARDWARE.md`; the firmware:
> `FIRMWARE.md`. The bus itself — framing, the ModBus arm, addressing — is
> `../core/blocks/modbus.md`, which wins on any bus question.

**Address:** the computed one, `TYPE«2 | NUMBER` — ModBus type **5**, so **0x14** for unit 0
(`../core/PROTOCOL.md` §2, which owns the three type spaces and their packings).

**The `0xFF00+` house block is the same on every MOD** — `VERSION` · `IDENT` · `TAG` · `HEALTH` ·
`NUMBER` rw · `BOOT_COUNT` · `STATUS` · `RATE` rw · `SENSORS` (`../babel/MODBUS.md`); no time
register, the arm carries no time. Only the block below is Pluvius's.

| Reg | Name | Meaning |
|---|---|---|
| 0x0000 | RATE ro | **uint16, 0,1 mm — the reading**: the running total since the last boundary. Bit 15 = UNSETTLED |
| 0x0001 | HOUR ro | **uint16**, the last complete hour, 0,1 mm — the second value. Bit 15 = UNSETTLED |
| 0x0002 | STATUS ro | bit0 VALID · **bit1 PUMP — the request: the unit wants the head; Palatine reads it and switches the `EXT` body** · bit2 ADC_FAULT · bit3 DRAIN_TIMEOUT · **bit4 FROZEN — the vessel's thermometer below 0 °C: the values are melt, not rate, and the catch may be capped** |
| 0x0003 | WEIGHT ro | **uint16, 1 g/LSB, direct** — what is in the vessel, on the house `RAW` position as the ingredient the total is made from; 65,5 kg of span against a 30 kg cell |
| 0x0004 | HOUR_PREV ro | **uint16**, the hour before that |
| 0x0005 | DAY ro | **uint16**, the last complete day, 0,1 mm. Bit 15 = UNSETTLED |
| 0x0006 | DAY_PREV ro | **uint16**, the day before that |
| 0x0007–8 | DRAINED ro | **uint32, ml, free-running** — the volume the head has counted out since boot; the reverse run's revolutions subtract and the next cycle's refill adds them back (`HARDWARE.md`, *The head's tail*). Accumulation is `weight + Δ DRAINED`, which is what lets the drain run during rainfall |
| 0x0009 | PUMP_REV ro | **uint16**, revolutions in the current cycle — what the pickup actually saw, for diagnosing a slipped or stalled head against what the drive was asked for |
| 0x000A | TEMP1 ro | 0,1 °C signed; `0x8000` = absent |
| 0x000B | TEMP2 ro | 0,1 °C signed; `0x8000` = absent |
| 0x000C–D | COUNT ro | signed 24-bit ADC count — bench and calibration diagnostics |
| 0x0010 | TARE rw | write ≠ 0 = re-tare now |
| 0x0011 | DRAIN rw | write 1 start / 0 abort; reads 1 while a cycle runs. **The remote pre-drain writes here** — server → master → Palatine → the tunnel — to empty the vessel ahead of a large incoming front ([`README.md`](README.md)) |
| 0x0012 | THRESH rw | **uint16**, the fill that starts a drain, 1 g — set low, so the cell carries little: a litre above `EMPTY` is the kind of figure (`HARDWARE.md`, *Detecting a dead drain*) |
| 0x0013 | EMPTY rw | **uint16**, the base level a drain runs down to, 1 g — water left standing so the tubes stay submerged; about 100 ml is the kind of figure |
| 0x0014 | HEAD rw | **written by the host**: 1 while the `EXT` body is on and the head runs, 0 when off — the unit's dead-drain window runs from 1, and a 0 mid-cycle ends the cycle. A slave cannot speak first, so the request is bit 1 in a register the host reads anyway and this is the host's answer (`../palatine/FIRMWARE.md` §6) |
| 0x0020–0x0027 | CAL rw | **span** (2⁻²⁴ g per count, uint32) · **buoyancy multiplier** (2⁻¹⁶, uint16) · **creep constant** (2⁻¹⁶ per decade, uint16) · **the hour boundary** (the second of the hour a period closes on, uint16) · **the day boundary** (the hour of the day, uint16) · **`DEAD_G`** (the fall the 5 s dead-drain window must show, g, 20 default) · reserve — plain holding registers, written from the bench and by the host, persisted (`FIRMWARE.md` §6, §9) |
| 0x0028 | FLUSH_REV rw | **uint16, revolutions** the head runs in reverse at the end of a cycle, a few seconds, to empty the tube — set at commissioning; 0 on a head with two wires, which leaves its hose full (`HARDWARE.md`, *The head's tail*) |

**`RATE` sits at `0x0000` because it is the reading** — the house map puts the reading first on
every unit we build and the second value beside it (`../core/blocks/modbus.md`, *The house map*);
`WEIGHT` takes the `RAW` position as the ingredient, and the rest of the table is the unit's own.

**What Palatine polls and ships up is the run `RATE · HOUR · STATUS`**, `0x0000`–`0x0002` — the
running total since the last boundary with its `UNSETTLED` bit, which is the two things anyone asks
a rain gauge: how much, and whether the level has settled; the last complete hour beside it; and
`STATUS`, whose `PUMP` bit is the request for the head, so the request rides the poll Palatine
makes anyway (`../palatine/FIRMWARE.md` §6). On the wire that is an 8 B block,
`[address][6][RATE · HOUR · STATUS]` (`../core/PROTOCOL.md` §5), and the archive reads 1,5 mm, 2,5 mm, then the hour closes and 0, 0, 3,0 mm — the total
resets at the boundary, and a drain inside the hour moves nothing in it. **Everything else in the table is state, read on demand**
through the tunnel — weight, the raw count, the temperatures, the drained volume, the revolution
count, the hour before and the day accumulations — never polled round after round: the air temperature is
known from the site's own thermometers, so freezing and snow are known without asking the gauge.
The register numbers stay here, in the profile; they never ride a frame.

## What is actually read — totals, not ingredients

**Pluvius carries every correction itself** — tare, span, the buoyancy multiplier, creep, and the
volume the head removed. **So it publishes totals.** The arithmetic never leaves the unit: what
went into the vessel minus what was pumped out, done once, here.

| Reg | Name | what it is |
|---|---|---|
| **0x0000** | **RATE** | the running total since the last boundary — what is falling *now* |
| **0x0001** | **HOUR** | the **last complete hour**. Rewritten on the hour |
| **0x0004** | **HOUR_PREV** | the hour before that — one more step of history, nothing cleverer |
| **0x0005** | **DAY** | the **last complete day**. Rewritten at the configured day boundary |
| **0x0006** | **DAY_PREV** | the day before it |

**The current hour and the current day are not published**: a partial total is not a
measurement. **The hour and the day are boundaries the unit is set to**, not periods anything
mandates — which second of the hour and which hour of the day a period closes on are configured
per unit like every other interval on this bus (`../core/blocks/modbus.md`). Two registers deep is
what the board holds; the rest of the history is the archive's.

**The `_PREV` pair is history, not insurance.** `HOUR` already holds the hour that finished and
keeps holding it until the next one closes, so a master polling at any sane rate cannot miss it.
What `_PREV` buys is one more step back — enough to see a boundary land badly and read the hour on
either side of it.

**A boundary can land badly and there is nothing to do about it.** A day boundary in a
thunderstorm cuts the 24 h total mid-event; the day is honest but unlovely, and the settled bit
says so. **The
measurement is not lost either way** — the unit runs drain to drain, weighed in against pumped
out, and every hour that ever finished is in the record. A total is a convenience for whoever
reads it, not the thing being measured.

### 15 + 1 — the settled bit rides the value

**Bit 15 of every total is `UNSETTLED`.** If the boundary landed mid-downpour the vessel was still
moving when the total was cut, and that reading is worth less than the next one; the unit says so
in the value itself.

**One bit, not a register**, and the reason is not the byte: **a flag in another register is a
second read at a different instant, and it can disagree with the value it describes.** In the same
sixteen bits it cannot — the flag and the number are one read, always consistent. Splitting a bit
on the module is cheaper than making the master fetch and pair two things.

Fifteen bits at 0,1 mm is **3276 mm in an hour**, which no hour has ever produced.

### The rest is state, read when something looks wrong

`WEIGHT` — what is in the vessel now: a blocked funnel, a vessel filling toward overflow.
`DRAINED` and `PUMP_REV` — a slipped tube or a stalled head, one against the other.
`COUNT` — bench and calibration only.

**None of these is the reading**: a total reassembled from them upstream would be two numbers
fetched at two instants, added by someone who measured neither.

**`WEIGHT` is uint16 at 1 g/LSB**: 65,5 kg of span against a 30 kg cell, and on the 200 cm² catch
1 g is 0,05 mm of rain, half the step precipitation is reported in. The cell and its wiring resolve
a gram (`HARDWARE.md`); the converter resolves far below it, so a finer step would describe nothing
that is there. `THRESH` and `EMPTY` are uint16 for the same reason.

**So every published value on this unit is one register**, which is what lets Palatine ship a
plain contiguous run and keeps the house map uniform (`../core/PROTOCOL.md` §5). `DRAINED` stays
32-bit because it is a free-running accumulator, not a reading.

**`WEIGHT` and `DRAINED` are the ingredients, not the reading** — *What is actually read*, above:
the reading is `RATE` and its neighbours, and both of these stay on the tunnel as unit state. A
peristaltic head with a pulse output, one a revolution, counts what it removes, so
**the accumulation is `weight + the change in DRAINED`**, computed here and published as a total —
and the drain runs while it rains. What the parent node packs into the 32 B payload is settled by
its own profile; pump revolutions, thresholds and the drain command stay on the ModBus tunnel as
unit registers.

**Where the pickup is not fitted, `DRAINED` and `PUMP_REV` read zero and stay there**, and the
accumulation falls back to weight alone. That is the **plain fit**, where the head — on
the switched 24 V at the station like every build — empties the vessel in a minute or two ([`HARDWARE.md`](HARDWARE.md)). The
registers do not move and a reader tells the two apart by `DRAINED` never advancing across a cycle
in which `PUMP` was asserted — **a fitted pickup that has stopped counting is a stall, and an
absent one reads the same way**. A dead head is caught by the weight not falling on either, and `DRAIN_TIMEOUT` is set the same way; what a build without the count gives up is the ml-per-revolution coefficient and the slipped-tube diagnosis (`HARDWARE.md`, *Detecting a dead drain*).

**DRAINED free-runs and never resets.** A head with control wires runs in reverse at the end of
every cycle and empties its tube, so nothing stands in it to freeze; those revolutions are
subtracted and the next cycle's refill adds them back, so what is counted is what left the vessel
(`HARDWARE.md`, *The head's tail*). A counter that wraps at 2³² ml is 4,3 million litres, decades of
a 200 cm² catch, and free-running means it survives a master restart where a per-cycle counter
would lose the volume it was carrying.

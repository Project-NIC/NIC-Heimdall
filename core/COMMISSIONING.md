★ N.I.C. ★

# NIC-Heimdall — commissioning (bench → field → bring-up)

> **Design-stage concept.** The end-to-end procedure for putting a station in the ground and turning
> it on. It ties together the physical build ([`../daedalus/CONSTRUCTION.md`](../daedalus/CONSTRUCTION.md)), the power/thermal case
> ([`POWER.md`](POWER.md)), the timing survey ([`blocks/gps-pps.md`](blocks/gps-pps.md)) and the
> master's GO/NO-GO readout (`../mayak/FIRMWARE.md` §10, `../mayak/handset/`). Written for an experienced builder:
> it is the **sequence and the intent**, not a hand-holding checklist.

The guiding rule, everywhere below: **fail in the garage, not in the bog.** As much as possible is
verified on the bench before anything is driven to a remote site — because the site may be a day's hike
away and there is **no laptop and no flashing cable** in the field; the only field tool is the **phone app
over BLE**.

## Phase 1 — Bench (before you leave)

Everything that can be got right indoors:

1. **Provision each Modbus sensor's address** — one unit at a time on the bench (they ship identical), set
   its address and the common 19 200 in Palatine's learning mode, driven from the phone
   (`blocks/modbus.md`). A unit provisioned wrong
   here is a silent zero later.
2. **Flash each board's firmware** — the last time a cable touches it; a later image comes over the
   bus (`LOAD` and `APPLY`, `PROTOCOL.md` §7). Nothing else is set: a unit carries its TYPE and takes its NUMBER from the sweep (`PROTOCOL.md` §2).
3. **Dry power-up on the bench** — assemble the station as it will run, power the master, and read the
   **GO/NO-GO on the phone** (press the commissioning button; BLE advertises only while it is live). The
   master's discover sweep + each node's `HEALTH`, pulled after it, lists **who registered** and **what is
   unprovisioned or failing, by name**. **All green → it may go to site. Anything red → fix it here.**

## Phase 2 — Field install (at the site)

The physical build, in the order the ground dictates:

1. **The vault** — dig, drop the shaft (ring / thick pipe), set the drainage per the **water table**
   (low → open bottom / French drain, boxes raised; high → sealed plug + IP68), lid + polystyrene slab
   (`../daedalus/CONSTRUCTION.md`, `POWER.md` §4).
2. **The mast** — pipe ~1,5 m down / ~2 m up; run the cabling **inside** it, insulated from the metal
   (`../daedalus/CONSTRUCTION.md`, *The mast*), and land every SPD common and the internal system's bonding conductor on the
   **one common earthing point** (`../daedalus/CONSTRUCTION.md`).
3. **Mount the heads** — met sensors in the shield, the GNSS + lightning antennas, the radiation heads,
   the solar panel — on the mast brackets. Only the sensors are exposed; the head, the battery and Quake go
   in the vault.
4. **Cable + seal** — pull the runs through the PG glands, seal them (bog-proof), land the data cable,
   the feed's own 2-core cable and the antenna coax. Fit the battery, the BMS and the fuse field, one fuse to a board (`../daedalus/CONSTRUCTION.md`).

## Phase 3 — Bring-up (at the site, on the phone)

Power on and walk the station up on the app — no laptop:

1. **Discover + GO/NO-GO** — same readout as the bench, now with the real cabling: every expected node and
   sensor **green by name**, or fix the one that is red before sealing up.
2. **GNSS fix + antenna aim** — confirm the receiver reaches a **valid fix** (`GGA` quality, sat count);
   nudge the antenna for clear sky if it is marginal. The clock's absolute anchor depends on it.
3. **Clock lock** — confirm Kronos reports **LOCKED**, every card **LABELLED**, and every unit
   **synced** — the shared clock is up end to end.
4. **Level Quake** — send the **calibrate** command; virtual levelling records the current gravity
   vector as the reference zero (a bubble level to ±1–2° is plenty — the matrix takes the rest,
   `../quake/FIRMWARE.md` §7). Only needed where a Quake is fitted.
5. **Survey the delays** — enter the **antenna coax** delay (≤ 4 m → ~20 ns, negligible — usually just
   note it). The spur cable delays, which are what keep the cross-node budget at distance, are not
   entered at all: they are measured on the card (below).
   - **A NodBus spur measures itself — there is nothing to type in.** The Bifrost ranges each of its
     runs at floor-up and again every `RANGE_INTERVAL`, and writes the route to the unit (`../bifrost/HARDWARE.md`), so a remote unit's
     cable delay never passes through a person. What is left for this step is the **antenna coax**
     above, which no bridge can see.
6. **Power check** — read **SoC / charge state / solar-in** off the BMS through Hermes (`POWER.md`); confirm
   the pack is charging and the source is sized for the site before you close the lid.
7. **Seal and leave** — lid on, glands sealed, reflective finish intact. The button times out, BLE
   goes quiet (radio off = power + security), and the station is autonomous: card is recording, the
   modem is sending its markers and heartbeat, and it phones home on whatever uplink the site has
   (`UPLINK_TRANSPORT.md`).

## Later — re-commissioning without a dig

Anything that does not need the ground reopened is a **phone job**: press the button, connect, and re-run
the pass (`{"cmd":"recommission"}`) or re-provision one node (`{"cmd":"provision","node":N}`) — e.g. after
swapping a Modbus unit or adding a head. The genuinely physical service (battery swap — the one
consumable) opens the vault lid; everything else is BLE + the app.

## Repair is replacement

A failed block — a unit, a card, a Bifrost — is **swapped, not repaired in place**: power down,
exchange, power up; the start-up sequence re-enrolls everything as on any boot. The outage costs
minutes of data, and that is accepted: the network is the instrument, and coverage during one
station's downtime comes from the stations that are up. A station does not promise continuity; it
promises to come back and report every unit present and healthy (the bring-up contract above).

## The through-line

**Green in the garage, green on the phone at the hole, seal it, walk away.** The whole procedure is built
so a remote, potted, buried station is committed only once every node and sensor has been called out **by
name** as present and healthy — never a silent box logging zeros for a season.

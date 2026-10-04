★ N.I.C. ★

# Handset — the station's bring-up and service companion (BLE)

The laptop-free bring-up companion for the **Mayak**, the station's head (`../FIRMWARE.md` §10,
the service window). You assemble a station, press the Mayak's commissioning
button, open this app, and it shows **who registered** and **which sensor is
unprovisioned / failing, by name** — a **GO / NO-GO** you read on the phone. It can also
send **provision / re-commission** commands.

> **Design-stage concept — no app is written.** This file is the app's description and the GATT
> contract the Mayak keeps; the app is built against it once the Mayak's firmware exposes the
> contract below.

**Stack — a recommendation, not a mandate:** one codebase for Android **and** iOS — Flutter with
`flutter_blue_plus` as the BLE central is the worked choice; React Native with
`react-native-ble-plx`, or native Kotlin and Swift, do the same. BLE, not classic BT: low power, and the
Mayak's ESP32-S31 is BLE-only anyway (BT 5.4 LE).

---

## GATT contract — the Mayak's firmware matches this

Mnemonic: the bytes `4e 49 43 20` spell "NIC " in the service base.

| Role | UUID | Property |
|---|---|---|
| Service | `9a1e0001-4e49-4320-b000-000000000000` | — |
| **Status** | `9a1e0002-4e49-4320-b000-000000000000` | **notify** |
| **Command** | `9a1e0003-4e49-4320-b000-000000000000` | **write** |

**Advertising is button-gated.** The Mayak advertises this service **only** while its
commissioning button / timeout is live (radio off in normal running — power + security).
A scan that finds nothing means "press the button on the box".

### Status (notify) — newline-delimited JSON

One full document per line; a document **may span several notifications** (reassemble on
`\n`). This mirrors the HEALTH/SOH the Mayak already aggregates:

```json
{"go":false,"nodes":[
  {"n":1,"bus":"nodbus","type":5,"label":"Quake","ok":false,"sensors":[
    {"id":0,"name":"ICM","h":0},
    {"id":1,"name":"ADXL355","h":2},
    {"id":2,"name":"SCL3300","h":0},
    {"id":3,"name":"RM3100","h":0}
  ]},
  {"n":2,"bus":"nodbus","type":6,"label":"Palatine","ok":true,"sensors":[
    {"id":1,"name":"AIR1","h":0},{"id":9,"name":"SOIL1","h":0}
  ]}
]}
```

- `go` — overall GO/NO-GO (all fitted sensors OK).
- `n` node NUMBER, `type` node TYPE — **the code of the bus's own type table**
  (`../../core/PROTOCOL.md` §2: NodBus 1 Mayak · 2 Bifrost · 3 Argus · 4 Marconi · 5 Quake ·
  6 Palatine · 7 Tesla · 8 Sputnik · 9 Photon · 10 reserved (Neutron) · 11 Positron · 12 Pip · 13 Steinmetz; mini 1 Gauss · 2 Quark-Tubes ·
  3 Pascal; ModBus 4..61, one per quantity at a position), and `bus` says which table. The
  optional `label` field is a debug courtesy only — the code, never the
  name, is the identity.
- per sensor: `id` = the node-local sensor # (Modbus roster address, or the SPI
  sensor index), `name` human label, `h` = **the sensor's health code**: `0` OK, `1`
  SELFTEST_FAIL, `2` NO_RESPONSE, `3` DEGRADED.

### The corrections table — the delays nothing can measure

**Everything the station subtracts is in this table, and all of it is Kronos's.** One term the
station measured itself — a remote receiver's clock run, ranged by Kronos — comes back
**read-only**, so the operator can see what was found rather than guess. The rest cannot be measured and has to be typed, and there are **two numbers**: the antenna
cable, as metres, and the antenna-plus-receiver constant, as nanoseconds from the receiver's
datasheet or one bench measurement — the shape the timing world's CGGTTS convention uses
(`../../kronos/FIRMWARE.md` §7, the delay table). They are **fixed biases**, so they are subtracted
rather than tolerated. The phone is where a human enters them, because a human is the only
thing on site that knows how much cable was actually pulled.

**Kronos holds its own copy in flash and offers it at start-up; the head is the authority.**
So the app shows what the station believes, and a write replaces it — a replaced Kronos is
handed the right numbers without anyone typing them twice.

Read (in the status document):

```json
{"corr":{"total_ns":13150,"terms":[
  {"id":"cab","label":"antenna cable","m":20,"ns_per_m":5,"ns":100,"src":"typed"},
  {"id":"int","label":"antenna + receiver","ns":50,"src":"typed"},
  {"id":"route","label":"remoted receiver, clock run","ns":13000,"src":"ranged"}
]}}
```

- **`src` says who owns the number:** `typed` is editable, `ranged` is what the station measured
  and is read-only — a write to one is refused, not silently dropped.
- `m` is what the operator types; `ns` is what the station computed from it. A term the build
  does not have is absent, not zero — `route` exists only where the receiver is remoted on a
  Galvani link (`../../galvani/README.md`, *The reversed channel*).
- The typed terms are summed and rounded on Kronos to whole capture cycles, 7,45 ns
  (`../../kronos/FIRMWARE.md` §7, the delay table); the ranged one is applied there too.
- **A spur's route is not a correction and is not in this table.** The card ranges it and
  writes it to the unit, which places its own grid on it (`../../core/PROTOCOL.md` §7); the
  routes are read from the cards and shown on the phone as diagnostics, one per slot.
- **`route` is the term that decides whether a remoted build meets the contract at all.**
  Glass runs ~5 ns/m, so a kilometre is ~5 µs against a 1 µs delivered precision. Kronos ranges
  it; a remoted receiver whose run has not been ranged is **NO-GO** in the app, in the same way
  an unprovisioned sensor is.

### The climatology tables — set by country, and the country is part of the answer

**A normality class is meaningless without the tables that produced it.** The seven probability
bands are WMO's and the same everywhere; **the boundary values are a national service's fit to
its own climate**, so a station carries the tables of the country it stands in
(`../../palatine/CLIMATE.md`). None of it is a firmware constant — it is
station configuration, typed once here.

What the app sets:

- **the country and the designation of the service whose tables these are** — stored with them,
  because anything published with a class attached has to say whose tables produced it;
- **the normal in force** — a 30-year block counted from 1901, so **1991–2020** today. Stated
  with the numbers the same way a long-term average has to state its period;
- **the boundary tables themselves** — temperature in K from the normal, precipitation in % of
  it, per month, per half-year (**IV–IX** and **X–III**) and per year.

**The Mayak holds them; the phone is only where they are typed.** It is the largest table the
head carries and it is the one that is *not* about the station's own hardware — everything else
in this contract describes the box, this describes where the box is standing.

**A station with no tables loaded still measures and still archives.** It simply does not
classify: the class is an interpretation added on top of a value, never a condition for
recording one. So a missing table is not NO-GO, unlike an unranged `route`.

### Command (write) — newline-terminated JSON

```json
{"cmd":"refresh"} // push a fresh status snapshot now
{"cmd":"recommission"} // re-run the whole commissioning pass
{"cmd":"provision","node":1} // (re)provision one node's sensors
{"cmd":"corr","set":[{"id":"cab","m":25}]}     // write one TYPED term; the station recomputes ns
                                               // a ranged term is read-only and a write is refused
```

> JSON was chosen for legibility and because commissioning is low-rate / occasional. If
> you want it tighter later, swap both sides to a packed TLV — the app touches it only in its
> status parser and its BLE layer.

---

## What a phone build needs

**BLE permissions**: on Android `BLUETOOTH_SCAN` (with `neverForLocation`) and `BLUETOOTH_CONNECT`,
and on API < 31 `ACCESS_FINE_LOCATION`; on iOS `NSBluetoothAlwaysUsageDescription` (a short "used
to commission NIC stations" string). BLE needs a real phone, not an emulator.

The app is four parts: the GATT contract (the UUIDs above), the BLE layer (scan, connect, subscribe
to status, write a command), the status model (station · node · sensor, the JSON parse, the health
vocabulary) and the one screen.

## Design — Volkov Commander

**Norton / Volkov Commander look:** DOS-blue background, monospace, box-drawn cyan panel,
and the signature **bottom function-key bar** (F2 Refresh · F3 Connect · F5 Recomm · F10
Quit). No modern colourful chrome — colour is the **16-colour DOS palette used
functionally**: sensor health (green / yellow / red), a reverse-video GO / NO-GO highlight
bar. It reads like a diagnostic tool, not a toy.

## Licence

Software: MIT (`../../LICENSE`) — Copyright (c) 2026 NIC — Native Intellect Community

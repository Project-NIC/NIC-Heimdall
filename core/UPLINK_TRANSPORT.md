★ N.I.C. ★

# NIC-Heimdall — uplink transport (how a station reaches the server)

> **Design-stage concept.** This is the **physical link** a station uses to send data home —
> the layer *below* the uplink **session** protocol (HELLO / CURSOR / ACK, the archive forward from the CURSOR, windows on demand —
> `PROTOCOL.md` §6). The session decides *what* and *when*; this decides *over what radio*.

## The principle — the card is the truth, the uplink is thin and layered

Every station is **autonomous**: it records everything to its **card (HMC)** locally, always. The
uplink is **not** the data path of record — it is a **thin, best-effort, layered** convenience on
top. This is only possible because the volume rides where it is cheap: the station sends the
**archive forward from its `CURSOR` over Wi-Fi** when a window opens, **markers and the hourly
heartbeat on the modem**, and **windows on demand** on a thin link, so what must leave at any moment
is **minuscule**. A tiny data rate is what lets the transport be a **low-power, bursty, even
opportunistic** radio — no always-on broadband, no Starlink-class dish (the satellite note below).

## The layers — card always; radios by what the site can reach

Ordered from always-present to site-dependent. A station uses whatever it can, and **falls back down
the list**; the card underneath never depends on any of them.

| Layer | Role | When | Rough cost |
|---|---|---|---|
| **Card (HMC)** | the durable local truth — everything, always | always | — |
| **The modem** | the always-on **thin backbone**, one module in the modem position on the backup cell's rail — LoRa, the cheapest, where a gateway is in range: the 16 B values frame **at once on a marker** and **once an hour as the heartbeat**, one frame for both (`../mayak/FIRMWARE.md` §9). Carries the **16 B values frame, never the archive** — the frame stands alone; and where a site sets it, **the live packet**: a mode-B Palatine's hourly row and daily row, 32 B each (`../palatine/WMO.md`). **On the `LR1121`'s satellite path**, about six messages a day, the heartbeat stretches to ~4 h and the hourly live packet does not fit | everywhere | lowest on LoRa |
| **Wi-Fi (directional, opportunistic)** | the **archive** in archive packets forward from the `CURSOR`, and any asked window, when an access point is reachable | site near an AP | low, **duty-cycled** |
| **Satellite (small-burst)** | the values frame where there is **nothing** — the `LR1121` in the modem position, or an Iridium / Kinéis module on a freed trunk UART (`../mayak/HARDWARE.md`) | no gateway, no AP | bursty |

### The modem — the always-on backbone AND the independent-path resilience layer

The modem carries the base uplink everywhere: one module in the Mayak's modem position, chosen per
site and fed from the rail the backup cell carries, so the last report leaves when the 12 V goes.
**LoRa is the default and the cheapest where a gateway/peer is in range**; the `LR1121` for LR-FHSS
straight to a satellite, an Iridium or Kinéis module, or an LTE-M / NB-IoT modem where none is:

- **A marker — at once.** Something happened (an ALARM, a `FAULT` of the marker set, a supply edge,
  a lost clock or store): the 16 B values frame goes out now, rate-limited to one a minute.
- **The heartbeat — once an hour**, restarted by every marker: the same 16 B frame — the second,
  the pack voltage, the board temperature, the event count, the alive mask, the flags with the
  clock's state and any shed. Nothing that is merely OK is sent; the heartbeat exists because
  silence cannot be told from death.

**Keep the modem even when there is Wi-Fi — it is a *different network on a different band*, so it
is the resilience path.** The fat links degrade exactly when you most want the event out: in severe
weather even rooftop satellite broadband drops to a trickle or out (the satellite note below), and
Wi-Fi fades too. The modem's tiny packet — LoRa's sub-GHz one where a gateway is in range — gets
through when the broadband does not, over a path that shares no infrastructure with the others — so
**if the fat link drops you still know "something is happening."** Events on the modem, bulk on the
fat link: if the bulk path dies, you have lost throughput, not awareness.

The full high-rate data stays on the card until a fatter layer below carries it — the archive
forward from the `CURSOR` over Wi-Fi, a window on demand elsewhere — or the card is pulled.

### How big is the archive, and the uplink — counted from what the head writes

**Every NUMBER on the station fills its slot every frame, and the head keeps what its recording
rule selects** (`archive/HMC.md`): under `ALL`, 128 frames a second, each its 32 B payload plus
the `status` and `kind` bytes — what the units deliver; codec 0's record adds 7 B of its own, and
HCC takes most of it back. A slot with nothing in it carries a filler. So the ceiling is the
same for every unit, whatever it measures:

| | raw under `ALL`, before HCC |
|---|---|
| one NUMBER | 128 × 34 B = **4,35 kB/s · ~376 MB a day** |
| a station of eight NUMBERs — one card full | ~35 kB/s · **~3 GB a day** |
| the full station, 32 NUMBERs on four cards | ~139 kB/s · **~12 GB a day** |

**What separates a seismic channel from a slow one is the recording rule.** A Quake under `ALL`
keeps every frame and HCC codes it column by column (`archive/HCC.md`); a Palatine under `CHANGE`
keeps a ModBus block only when its bytes change — a few hundred bytes an hour for a weather set;
a Tesla under `NONZERO` keeps only the frames that carry something. The table is the ceiling a
card is sized against; the retention a station actually gets is its own rules' mix after HCC.

**The card:** at the raw ceiling a 512 GB card holds ~6 weeks of the full station and ~6 months of
one full card; a station of slow quantities under their default rules holds decades. **The uplink:**
the modem carries the values frame and, where a site sets it, the live packet — nothing of the
archive. The archive leaves in archive packets over Wi-Fi or on a pulled card, and even the full
station's raw ceiling, ~1,1 Mb/s, is a small load on a Wi-Fi link that is up for a window. **The
live packet** — what each unit's send rule selects, at a cadence per link — rides whatever link the
site has: a station with no Wi-Fi sends a mode-B Palatine's hourly row over the modem, 32 B, and its
cards are read when somebody comes (`archive/HMC.md`, *The uplink*).

### Wi-Fi — opportunistic bulk over a directional antenna

**A directional antenna reaches an AP kilometres away**, and the station's ~1 Mb/s at most is a
small load on that network. A station a few km from a village (or a lone building) points a
**directional Wi-Fi antenna** (panel / grid / Yagi, line-of-sight) at a **reachable AP**, and
**every ~30–60 min turns the radio on, sends the archive forward from its `CURSOR` and any asked
window, and turns it off** again. Wi-Fi is on the Mayak's **ESP32-S31 for free** (Wi-Fi 6); the only
adds are the directional antenna and a **reachable AP** (a cooperating/known/guest network —
arranged, not leeched). Duty-cycling it (on only for the dump) keeps the power draw down.

**What the antenna buys.** The S31 is 2,4 GHz only, 20 dBm out, −99 dBm at 1 Mb/s and −94 dBm at
MCS0 (module datasheet v0.7, tables 7-2 and 7-4). **The Czech limit on 2400–2483,5 MHz is 100 mW
e.i.r.p. — 20 dBm including the antenna's gain** (ČTÚ VO-R/12), so a high-gain antenna is paid for
by turning the transmitter down: the gain counts in full on receive and not at all on transmit, and
a long link wants the same antenna at BOTH ends. The table takes 20 dBm e.i.r.p., the same antenna
at each end, 1,5 dB of cable and connectors and a 10 dB fade margin:

| antenna | gain | beam (−3 dB) | 1 Mb/s | MCS0, 6,5 Mb/s | MCS2, 19,5 Mb/s |
|---|---|---|---|---|---|
| log-periodic | ~10 dBi | ~50–60° | ~7 km | ~4 km | ~2,3 km |
| helix, 1 m, circular | ~17–18 dBi | ~18–20° | ~18 km | ~10 km | ~6 km |
| Yagi or log-Yagi, 1 m | ~17–18 dBi | ~20–25° | ~18 km | ~10 km | ~6 km |
| four Yagis, 2×2 | ~23 dBi | ~10–12° | ~33 km | ~18 km | ~10 km |
| grid dish, 1 m | ~25 dBi | ~9° | ~40 km | ~23 km | ~13 km |
| grid dish, 2 m | ~31 dBi | ~4–5° | — | — | ~26 km |
| grid dish, 3 m | ~34 dBi | ~3° | — | — | ~37 km |

- **The beam is set by the aperture in wavelengths**, θ ≈ 70 λ/D degrees; at 12,3 cm a 1 m dish is
  eight wavelengths across. An end-fire antenna — Yagi, helix, log-periodic — gains only with its
  length, so its beam narrows with the square root of the length; a dish narrows with its diameter.
  Polarization does not narrow a beam. A log-periodic cut to the one band does not gain either: as
  its element ratio goes to 1 it becomes a row of alike dipoles and stops near 10 dBi, and the
  narrow-band end-fire antenna is the Yagi. **A circular antenna needs a circular one at the far
  end** — against a linear one it loses 3 dB — and rejects a reflection, which comes back with the
  opposite hand.
- **The archive needs MCS0 to MCS2** — the full station's raw ceiling is ~1,1 Mb/s and HCC takes it
  lower. **The base is a 1 m grid dish, ~15–20 km; a 1 m Yagi, ~6–10 km, where the path is
  shorter.** Past ~20 km the path, not the antenna, decides: a larger dish adds receive gain the
  transmit limit cannot use, and a 3° beam holds only on a mast that twists less than ±1° in a gale
  where the 1 m dish's 9° tolerates ±3–4°.
- **The path must clear 60 % of the first Fresnel zone** plus the earth's bulge at k = 4/3: ~12 m
  plus 2,5 m at mid-path on 13 km, ~15 m plus 6 m on 20 km — both antennas stand that high above
  whatever lies between them. **At these distances the link's acknowledgement timeout is longer than
  Wi-Fi's default**, and the firmware sets it for the distance.
- **Rain does not limit this band.** Specific attenuation at 50 mm/h (ITU-R P.838): ~0,01 dB/km at
  2,4 GHz, ~0,16 at 5 GHz, ~1,7 at 10 GHz, ~8 at 24 GHz — on 20 km that is 0,2 dB, 3 dB, 33 dB and
  150 dB. Below ~6 GHz the weather on the path costs nothing; **wet snow and ice on the feed cost
  more, and the feed is covered.** A grid dish lets snow through and can hang feed-down.

### Cellular for the live tier, satellite beyond it

Where neither a LoRa gateway nor a reachable AP exists:

- **Cellular carries the live tier, never the archive.** An archive of gigabytes a day wants an
  unlimited tariff at roughly **600 Kč a month per SIM**, which a few hundred stations turn into
  over a hundred thousand crowns a month; the live tier — the live packet, the markers, the command
  channel, the live board — is tens of megabytes a month at most, and that is what an **IoT tariff
  on LTE-M or NB-IoT** is sold for: O2's *Machine 100 MB* at **110 Kč a month**, Vodafone's *IoT
  Easy Connect* at **449 Kč once for 1 GB and ten years**, Miotiq's roaming tariff at **2,59 € a
  year for 1 GB** in 45 countries. **The cellular path is an LTE-M / NB-IoT modem in the Mayak's
  modem position**, on a freed trunk UART as the satellite module is, and **fed from the rail the
  backup cell carries**, so the last report leaves when the 12 V goes — Proteus carries one. A
  bought router on the Ethernet carries the live tier too, but it dies with the 12 V and is never
  the modem. The archive still goes by card, by Wi-Fi where there is one, or by the
  bounded windows the server asks for. **The local link is Wi-Fi**, which the S31 carries: a
  directional antenna and an agreement with a nearby access point cost less than one month of one
  unlimited SIM.
- **Satellite, small-burst** — **Iridium SBD** is the safe global default (works to the poles, small
  reliable bursts) at roughly **400 Kč a month per module** for the line plus **~4 Kč per message**
  of up to 50 B — ~80 000 Kč per megabyte; **smallsat IoT** (Astrocast / Kinéis / Myriota) is
  cheaper — Kinéis ~**25–125 Kč a month per device** for up to 96 messages of 19 B a day — but
  store-and-forward (latency — a satellite passes overhead), fine for markers. Satellite carries
  **the values frame only** — the markers, and the heartbeat as its message budget allows; full
  windows wait for a fatter link or a card pull.
- **LoRaWAN straight to a satellite — the `LR1121` as the modem, the same radio, no second
  transmitter.** Operators exist whose spacecraft receive a LoRa-family uplink directly from an
  ordinary node (Lacuna's *LoneWhisper*, on **LR-FHSS** — Semtech's satellite adaptation of LoRa,
  compatible with LoRaWAN devices; six satellites operational in polar orbit as of mid-2026). This
  fits the station's own rule that nothing new gets designed: **one transmitter, two antennas** —
  terrestrial where a gateway exists, spaceborne where none does. **But size it honestly — it is an
  EXISTENCE channel, not a data channel.** The published service is on the order of **six 50-byte
  messages per day across three passes = ~300 bytes/day**: three hundred bytes, not three hundred
  kilobytes. That buys *alive / in trouble / here is the alarm code* from anywhere on the planet,
  and nothing more, at roughly **250 Kč a year per device** where the service is priced.
  **On it the heartbeat stretches to ~4 h and the hourly live packet does not fit.** The archive
  stays on the card and waits for a real link, exactly as everywhere else in this document.
- *(Avoid Swarm — being wound down. **Avoid Starlink — doubly wrong here** (measured figures, not
  marketing): the **Standard/Gen 3 dish is ~75–100 W active, ~20–50 W idle**, and even the low-power
  **Mini** (what drone crews use) is **~15–25 W typical, ~60 W startup** — a **continuous** load that
  **dwarfs the whole station's own draw** (not a burst you can duty-cycle), **and** it is weather-fragile
  (drops from ~50–100 Mb/s to a trickle in heavy rain). A low-power station wants small-burst sat + LoRa,
  not a broadband dish.)*

### Clustered deployment — many nodes, one gateway (islands / remote fields)

Where a group of stations sit within radio range of each other but far from any backhaul (an island
archipelago, a valley of boreholes), don't give each one an expensive uplink — **mesh them to one
gateway:**

- **LoRa gathers the cluster** → a single **gateway** station carries the one fatter link out. The
  leaves stay cheap and low-power; only the gateway spends the uplink watts/cost.
- **The ratio is limited by LoRa, not the backhaul.** What the cluster gathers over LoRa is each
  leaf's 16 B values frame, an hour apart and on a marker — a concentrator carries **30–100** leaves
  on that, and the archive stays on each leaf's card until a fatter link or a visit takes it. Push
  the ratio high: the gateway's uplink is the dominant cost and power, so **minimise gateways**, and
  let the mesh **re-route** if one dies (don't over-concentrate).
- **Over open water the limit is the horizon, not interference** (`d ≈ 4.12(√h1 + √h2)` km) — antenna
  height sets the hop, so hilly/volcanic islands reach far and flat atolls stay short.
- **Starlink is the one place it fits:** the per-station "avoid Starlink" rule stands (the satellite
  note above), but a **shared cluster gateway** is a bigger, better-powered site where a broadband dish
  aggregating many leaves *can* justify its draw — the exception that proves the per-node rule. A
  small-burst sat link still serves a gateway with no power to spare.

## Modular — the transport is a swappable radio, like a sensor is a Modbus unit

Wi-Fi is built into the ESP32-S31; its Ethernet MAC is brought out only on Proteus and Mimir
(`../mayak/HARDWARE.md`, *Ethernet*), the Mayak board's uplink being Wi-Fi and the modem; **the modem is one module in
the modem position, chosen per site** — LoRa, a satellite module or an LTE-M / NB-IoT modem, the
UART ones on a freed trunk UART. A site **populates the radio it needs** and leaves the rest off —
the same "one board, populate for the job" rule as the sensors. No transport is wired-in-forever.

**Radios and antennas on the Mayak (ESP32-S31).** The `-3U` module brings the 2,4 GHz radio out to
one **external antenna connector**, and the modem has its own, so **the modem and Wi-Fi run to their
own real antennas** (the Wi-Fi one directional, per above; the modem its own — sub-GHz for LoRa).
**Bluetooth (BLE) stays purely local** — it is only the commissioning / control link for the Handset
(`../mayak/handset/`); on the S31 it is the same radio as Wi-Fi and shares the module's one external
antenna connector, which is enough for a phone at the open enclosure. So the master exposes: **the
modem → its antenna, Wi-Fi + BLE → the one 2,4 GHz (directional) antenna**.

## Couples back to power

Every radio has a very different power cost, and the choice **feeds straight back into the solar/battery
sizing** (`POWER.md`): Wi-Fi and satellite are **duty-cycled** — on only for the periodic dump,
off otherwise, so the *average* draw stays low. A satellite modem's burst
current and a directional Wi-Fi session are line items in the power budget; size the source for the
transport the site actually uses. (This is why Starlink is out per station — a continuous load,
weather-fragile on top; the satellite note above has the figures.)

## Where this connects

- The session it carries: `PROTOCOL.md` §6 (HELLO/CURSOR/ACK, the spool, the archive and
  live packets, windows on demand, the server contract).
- The power it draws: [`POWER.md`](POWER.md) (duty-cycled radios in the budget; solar/battery sizing).
- The modem is chosen per deployment — LoRa, a satellite service or an IoT tariff; the station
  fixes only that it is fed from the backup cell's rail and never carries the archive.

★ N.I.C. ★

# EXPORTERS — what the world takes, written from the archive

> **Design-stage concept — nothing built.** What each exporter takes and writes is fixed here; the
> code is written against this page when the reference library runs, and checked as each entry says.

**An exporter is a reader.** It opens HMC files (`HMC.md`) through the one reference library,
takes the records of the addresses and the window it is asked for, and writes a format someone
else defines. **It never touches the station, the bus or the archive**: a new format is a new
reader, and an export is re-written from the same bytes whenever a layout or a model improves.
Who takes each format, and whether they take us, is `../INTEROP.md`; this page is how each one is
written.

## The rules every exporter keeps

- **Time is converted here and nowhere else.** A record carries its second and its frame index; the
  format's time is the second plus `frame × 2²⁰ / rows` project ticks — 8 192 a frame at 128 rows —
  made decimal by the one constant 10⁶/2²⁰ (`../blocks/nodbus.md`). A record front carries the
  instant its own `BUS.md` gives it: Sputnik's epochs carry the receiver's tag. **The archive's
  seconds are GPS time in the Unix epoch, leap-blind** (`HMC.md`); an exporter subtracts the GPS −
  UTC offset of the `TIME` section in force at the record's second, and writes UTC throughout — a
  leap second the format's own business, from the `TIME` section's announcement.
- **Values follow the one calibration rule** (`../INTEROP.md`): a format built for counts — miniSEED
  — takes the counts, its response beside them; every other takes the physical value of the field's
  layout, `physical = (raw + add) × mantissa × 10^exp10`, from the config in force at the record's
  second.
- **A missing record is read through the rule in force**, never guessed:

  | rule in force | a second with no record means |
  |---|---|
  | `ALL` | a gap — written as the format's gap |
  | `CHANGE` | the last kept value holds, up to its `at least every s`; past that, a gap |
  | `NONZERO` | zero, up to its `at least every s`; past that, a gap |
  | `DECIMATE n` · `INTERVAL s` | nothing — the series has the rule's own rate, and is exported at it |
  | `NONE` | the address is not exported |

- **A flagged frame never leaves as a clean number** (`../README.md`, the second principle):
  `status` 6 CLIPPED and 7 DISTURBED are carried as the format's flag where it has one, and as its
  missing value where it has none. The flagged frame itself stays in the archive.
- **The metadata comes from the header**: the station's name and position from `STATION`, the
  unit from `UNIT`, the names, units and scales from `LAYOUT` and `PROFILE`, a response from
  `RESPONSE`. **A registered code** — an FDSN network, an IAGA code, a CWOP ID — is the server's
  configuration, never the station's.
- **An export is a function of the archive and its request**: the same files and the same request
  write the same bytes. An exporter keeps no state of its own; what it last sent is the server's
  catalogue (`HMC.md`, *On the server*).

**Each entry below says the same seven things**: what it takes, what it writes, the time, the
values, the metadata, the gaps and flags, and what it is checked with.

## The exporters

| exporter | from | for |
|---|---|---|
| [miniSEED + StationXML](#miniseed--stationxml) | Quake; any series under `ALL` — Gauss, Pascal | FDSN, EarthScope, SeisComP, ObsPy |
| [IAGA-2002](#iaga-2002) | Gauss; any field in nT | SuperMAG |
| [RINEX](#rinex) | Sputnik, Tier B | Madrigal, NGL, RTKLIB |
| [BUFR](#bufr) | a mode-B Palatine's `HOUR` and `TEN` rows | WIS2 / a national service |
| [CWOP](#cwop) | Palatine | CWOP → MADIS |
| [IOC sea level](#ioc-sea-level) | Pascal | the IOC monitoring facility |
| [Air quality](#air-quality) | Chinook's units on Palatine's arms | sensor.community, OpenAQ |
| [Blitzortung](#blitzortung) | Tesla | Blitzortung, by agreement |
| [CSV](#csv) | any address | anyone, a spreadsheet |
| [SQL](#sql) | any address | SQLite on one machine, PostgreSQL for a network |

### miniSEED + StationXML

| | |
|---|---|
| **takes** | one channel's series of an address recorded under `ALL` — a Quake axis, a Gauss component, Pascal's pressure |
| **writes** | miniSEED 2.4, Steim-2, 512 B records (miniSEED 3 where the data centre takes it); one StationXML document a station |
| **time** | a record's start is its first sample's instant; the rate is the `UNIT`'s rows, or the layout's samples a payload times the rows |
| **values** | the counts as recorded; the codes from the layout — the SEED band by the rate, the instrument by the quantity, the orientation by the channel |
| **metadata** | StationXML from `STATION`, `UNIT` and `LAYOUT`: a flat `InstrumentSensitivity` from the layout's scale, poles and zeros only from a `RESPONSE` section |
| **gaps and flags** | a gap ends a record and the next begins after it; a record holding a CLIPPED frame carries the *digitizer clipping* quality bit, one holding a DISTURBED frame the *glitches* bit |
| **checked with** | libmseed and ObsPy read the series back sample for sample; the Steim-2 frames against `ref/steim2.py`; StationXML against the FDSN schema |

### IAGA-2002

| | |
|---|---|
| **takes** | Gauss's components, and any series a layout gives in nT |
| **writes** | IAGA-2002 text, `Data Type: variation`, one file a station a day; minute values, or second values on request |
| **time** | a minute value is centred on the minute, by the Gaussian filter INTERMAGNET specifies for minute data, named in a comment line |
| **values** | nT from the layout's scale, to the format's 0,01 nT; the components as the layout's names give them |
| **metadata** | the header from `STATION` — name, latitude, longitude, elevation — and the IAGA code from the server's registration |
| **gaps and flags** | flagged samples are left out of the filter; a minute the filter cannot fill, or any gap, is `99999.00` |
| **checked with** | an independent IAGA-2002 parser reads the file back; the minute values against the same filter run in the reference library on the raw series |

### RINEX

| | |
|---|---|
| **takes** | Sputnik's Tier B, decoded as `../../sputnik/BUS.md` defines it: the epochs, the arc bases, the geometry records |
| **writes** | a RINEX 3 observation file — pseudorange, carrier phase and SNR on the three slots of each system; navigation is taken from the broadcast products, not written here |
| **time** | the epoch is the receiver's tag, in GNSS time as RINEX wants it; the station's second only orders the records |
| **values** | each pseudorange and phase restored by its arc base to the full value; the observation codes from Sputnik's slot table |
| **metadata** | the header from `STATION` — the approximate position — and the receiver, antenna and site from the server's site log |
| **gaps and flags** | an epoch never received is absent; a cycle slip in the flags byte is RINEX's loss-of-lock indicator; an arc whose base was not yet seen is not written |
| **checked with** | RTKLIB and georinex read the file; a PPP solution from it lands on the station's surveyed position |

### BUFR

| | |
|---|---|
| **takes** | the `HOUR` and `TEN` table series of a mode-B Palatine, addresses `0xF8`–`0xFB` (`../../palatine/WMO.md`) |
| **writes** | BUFR edition 4: template 307096 from the `HOUR` rows, 307092 from the `TEN` rows, encoded with ecCodes |
| **time** | the row's own observation time, as the table defines it |
| **values** | the row's values, scaled to the template's units; nothing is derived that the row does not carry |
| **metadata** | the WIGOS station identifier and the position from the server's registration and `STATION` |
| **gaps and flags** | a value the row marks missing is BUFR's missing value; a row never received is not encoded |
| **checked with** | ecCodes decodes the message back to the row; the national service's validator, where it has one |

### CWOP

| | |
|---|---|
| **takes** | Palatine's air temperature, humidity, pressure, wind and rain |
| **writes** | the CWOP / APRS weather packet, every 5 to 10 minutes, to the CWOP servers |
| **time** | the packet's time is the newest value's second |
| **values** | the units the protocol asks — °F, inches, mph, tenths of millibar — converted here from the layout's |
| **metadata** | the CWOP ID and the position from the server's registration |
| **gaps and flags** | a value missing or flagged is left out of the packet, which the protocol allows field by field |
| **checked with** | the packet parsed back; the station's page at CWOP against the archive |

### IOC sea level

| | |
|---|---|
| **takes** | Pascal's bottom pressure, under `ALL` |
| **writes** | the series the facility asks — one value a minute — pushed over HTTP, as agreed at registration |
| **time** | a minute value is the mean centred on the minute |
| **values** | the pressure in the layout's unit; the conversion to a water level is the facility's, from the site's density, or ours where it asks for level |
| **metadata** | the station code and position from the registration and `STATION`; the depth from the site log |
| **gaps and flags** | a minute with a flagged or missing sample is not sent |
| **checked with** | the facility's plot of the station against the archive; a tide fitted to a month of it |

### Air quality

| | |
|---|---|
| **takes** | the blocks of Chinook's units on Palatine's arms, read through their `PROFILE` |
| **writes** | the JSON each network takes, pushed over HTTP — sensor.community per sensor, OpenAQ per location |
| **time** | the reading's second; a mean where the network asks for one, over the window it names |
| **values** | µg/m³ and ppb as the profile scales them |
| **metadata** | the sensor's model from the profile; the network's IDs from the server's registration |
| **gaps and flags** | a missing or flagged reading is not sent |
| **checked with** | the network's map against the archive |

### Blitzortung

| | |
|---|---|
| **takes** | Tesla's stroke records |
| **writes** | what Blitzortung's receiver protocol asks — the protocol is theirs and is agreed with them, so this entry is the slot, not the specification |
| **time** | the stroke's instant from its record, to the grid's 2⁻²⁰ s |
| **values** | as the protocol asks |
| **metadata** | as the protocol asks |
| **gaps and flags** | a DISTURBED frame sends nothing |
| **checked with** | Blitzortung's own location of our strokes against Tesla's |

### CSV

| | |
|---|---|
| **takes** | any address, any window |
| **writes** | one file an address: a header line with each field's name and unit, then one line a record — the UTC time, the frame, `kind`, `status`, then the values |
| **time** | ISO 8601 UTC to the microsecond, and the second and frame beside it |
| **values** | physical, from the layout in force; raw on request, one column a field as the unit wrote it |
| **metadata** | a comment block above the header: station, address, TYPE, the layout's revision, the rule in force |
| **gaps and flags** | only records that exist are written — the `status` column carries the flag, and the rule in the comment says what a missing line means |
| **checked with** | the file read back and compared value for value with the reference library |

### SQL

**Not an archive — a query tool beside it**, for whoever wants to ask a few years of a station with
a query instead of a program. The tables are written from the files and dropped and rebuilt at
will; the catalogue of *On the server* (`HMC.md`) is the same schema without the values.

| | |
|---|---|
| **takes** | any addresses, any window |
| **writes** | SQLite, one file — the light build for a station, a laptop, a small operator; PostgreSQL for a network, the same schema |
| **time** | the second and the frame index as integers, and the UTC time as the database's own type beside them |
| **values** | one row a record and a field: physical and raw both, so neither question needs the other |
| **metadata** | a table each of stations, units, layouts and configs, as the headers and configs give them |
| **gaps and flags** | the `status` of each record in its row; the rules in force in the configs table |
| **checked with** | a query on the tables against the same window read through the reference library |

```
stations (station, name, latitude, longitude, elevation)
units    (station, address, type, rows, first_second)
configs  (station, address, from_second, rule, layout_revision, sections)
fields   (station, address, from_second, channel, name, unit, exp10, mantissa, add)
values   (station, address, second, frame, kind, status, channel, raw, value)
segments (station, address, file, offset, length, first_second, last_second, codec)
```

`values` is keyed by station, address, second, frame and channel; a record kept twice — a late
frame repaired — keeps the later written, as the reader does.

## The viewer

**The viewer is a reader too**: it shows a window of any address as a plot, through the same
library, the same time and the same rules — a missing record drawn as the rule in force reads it,
a flagged one drawn as flagged. It writes nothing.

## Owed

The reference library, C and Python, that reads HMC, and on it each exporter as entered here.
**No exporter is written before the library reads a real station's files**, and none is registered
anywhere before the hardware is validated (`../INTEROP.md`, the gate).

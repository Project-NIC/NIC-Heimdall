★ N.I.C. ★

# The exporters

**English** · [Čeština](README.cs.md) · [Русский](README.ru.md)

What the world takes, written from the archive after the fact, as [`../EXPORTERS.md`](../EXPORTERS.md)
enters each one. **Nothing here is the station's record**: the record is the MLF files and the
library in [`../ref/`](../ref/) that writes and reads them; an exporter only reads them, on the
server or a laptop, and the same files and the same request always write the same bytes.
**miniSEED with StationXML and IAGA-2002 are NIC-SDK's** (`tools/mlf2mseed.*`, `tools/mlf2iaga.*`),
for any profile; Heimdall gives them its glue, the reading of its records.

| file | what |
|---|---|
| `glue.py` | Heimdall's glue for NIC-SDK's exporters: a channel of the `LAYOUT` by its name or as `#c`, at n · rows a second; the station from `STATION`; GPS − UTC from `TIME`; a segment's samples through the reference library, CLIPPED and DISTURBED as SEED's quality bits; SEED's orientation by the name, else the channel's number |
| `tables.py` | every record of an address with the layout, the rule and GPS − UTC in force at it; one CSV file an address; SQLite tables PostgreSQL takes as they are, the files' catalogue beside them |
| `tests/test_mseed.py` | a synthetic Quake's file through NIC-SDK and the glue, read back by ObsPy sample for sample, a gap and the quality bits where they belong, the three axes' codes, the StationXML valid against the FDSN schema |
| `tests/test_iaga.py` | a synthetic 32 Hz magnetometer's day through NIC-SDK and the glue, read back by its fixed columns: the station, the gap's minute, a minute where the filters say |
| `tests/test_tables.py` | CSV and SQL value for value against the reference library's own reading of every test file, a late frame's later record kept, the catalogue one row a segment and a config |

```
python3 ../../../../NIC-SDK/tools/mlf2mseed.py glue.py OUT.mseed NET STA LOC '#0' FILE…
python3 ../../../../NIC-SDK/tools/mlf2iaga.py glue.py DIR CODE SOURCE minute magX,magY,magZ FILE…
python3 tests/test_mseed.py          # needs ObsPy
python3 tests/test_iaga.py
python3 tests/test_tables.py
```

NIC-SDK is found beside this repository or where `NIC_SDK` points (`../ref/sdk.py`); its own tests
hold the formats — the Steim-2 frames, the filters against SciPy's, Budkov's minutes.

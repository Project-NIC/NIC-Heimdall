★ N.I.C. ★

# The exporters

What the world takes, written from the archive after the fact, as [`../EXPORTERS.md`](../EXPORTERS.md)
enters each one. **Nothing here is the station's record**: the record is the HMC files and the
library in [`../ref/`](../ref/) that writes and reads them; an exporter only reads them, on the
server or a laptop, and the same files and the same request always write the same bytes.

| file | what |
|---|---|
| `export.py` | a channel's series read out of an address's files at its UTC instants; miniSEED 2.4 with Steim-2 in 512 B records and FDSN StationXML 1.2; IAGA-2002 with INTERMAGNET's minute filter and USGS's low-pass to 1 Hz scaled to the rate |
| `tables.py` | every record of an address with the layout, the rule and GPS − UTC in force at it; one CSV file an address; SQLite tables PostgreSQL takes as they are, the files' catalogue beside them |
| `tests/test_mseed.py` | a synthetic Quake's files exported and read back by ObsPy sample for sample, a gap and the quality bits where they belong, the frames against `../ref/steim2.py`, the StationXML valid against the FDSN schema |
| `tests/test_iaga.py` | the filters against SciPy's; Budkov's own minutes from its own seconds to their rounding; a synthetic 32 Hz magnetometer's day read back by its fixed columns |
| `tests/test_tables.py` | CSV and SQL value for value against the reference library's own reading of every test file, a late frame's later record kept, the catalogue one row a segment and a config |

```
python3 tests/test_mseed.py          # needs ObsPy
python3 tests/test_iaga.py           # needs SciPy; BDV_SEC and BDV_MIN name Budkov's day, if fetched
python3 tests/test_tables.py
```

The test files are the reference's (`../ref/tests/test_hmc.py`); fetching Budkov's day is in
`../ref/README.md`.

★ N.I.C. ★

# The archive's reference code — Python and C

**English** · [Čeština](README.cs.md) · [Русский](README.ru.md)

The reference library `HMC.md` names, in Python and in C: HCC as `HCC.md` fixes it, the two
writing the same bytes, and the measurement that settles its ratios against Steim-2 on
recorded series. Nothing here runs on the head yet; the head will write with the C.

| file | what |
|---|---|
| `hcc.py` | HCC coder and decoder: the bit stream, the five modes, Rice partitions, the reference channel, a layout composed from fields and the bytes none covers, an Argus NUMBER's layout from its positions |
| `steim2.py` | a Steim-2 packer and unpacker — the miniSEED codec HCC is measured against, checked against libmseed's frame count |
| `measure.py` | bits per sample of Steim-2 and of every HCC mode on recorded traces, per segment length; `--ref` codes paired channels against each other |
| `tests/test_hcc.py` | the cases `HCC.md` lists, on synthetic records; every one round-trips byte for byte |
| `tests/test_damaged.py` | streams cut, extended, bit-flipped, overwritten and invented: every one is refused with `ValueError` or decodes, never crashes or hangs; a cut or extended one always reads as damaged. `HCC_FUZZ_SEED` and `HCC_FUZZ_ROUNDS` widen it |
| `c/hcc.h` · `c/hcc.c` | the same coder and decoder in C99, malloc and free its only dependencies; built with `-ffp-contract=off`, so the predictor rounds every double operation as the Python does, and with exact 128-bit autocorrelation sums |
| `c/hcc_tool.c` · `c/Makefile` | the C driven over stdin and stdout, for `tests/test_c.py` |
| `tests/test_c.py` | the C against the Python: every case above and random segments — every field width, both byte orders, reference channels, extreme and wrapping values, seconds anywhere in 32 bits — coded by both and compared byte for byte, each stream read back by the C, and every damaged stream given the same verdict and the same records by both. `HCC_C_SEED` and `HCC_C_ROUNDS` widen it |

```
python3 tests/test_hcc.py
python3 tests/test_damaged.py
python3 tests/test_c.py                 # builds c/ with $CC, which may carry flags
python3 measure.py recording.mseed --seconds 1,2,4,8,16,32
python3 measure.py obs.min --seconds 60,300,900,3600      # IAGA-2002, nT × 100
```

`measure.py` reads miniSEED through obspy and IAGA-2002 itself; a trace is one integer series
and the field width is the smallest whole-byte width its values fit.

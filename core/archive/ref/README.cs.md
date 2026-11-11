★ N.I.C. ★

# Referenční kód archivu — Python a C

[English](README.md) · **Čeština** · [Русский](README.ru.md)

Referenční knihovna, kterou jmenuje `HMC.md`, v Pythonu a v C: HCC tak, jak ho určuje `HCC.md`,
obě zapisují tytéž bajty, a měření, které rozhoduje o jeho kompresních poměrech
proti Steim-2 na zaznamenaných řadách. Nic z toho zatím neběží v centrále; centrála bude zapisovat přes C.

| soubor | co |
|---|---|
| `hcc.py` | kodér a dekodér HCC: bitový proud, pět režimů, Riceovy oddíly, referenční kanál, rozvržení složené z polí a z bajtů, které žádné pole nepokrývá, rozvržení NUMBER Argusu z jeho pozic |
| `steim2.py` | packer a unpacker Steim-2 — kodek miniSEED, proti kterému se HCC měří, ověřený proti počtu rámců z libmseed |
| `measure.py` | bity na vzorek pro Steim-2 a pro každý režim HCC na zaznamenaných stopách, podle délky segmentu; `--ref` kóduje párové kanály proti sobě navzájem |
| `tests/test_hcc.py` | případy, které uvádí `HCC.md`, na syntetických záznamech; každý projde tam a zpět bajt po bajtu |
| `tests/test_damaged.py` | proudy zkrácené, prodloužené, s převrácenými bity, přepsané a vymyšlené: každý je buď odmítnut s `ValueError`, nebo se dekóduje, nikdy nespadne ani se nezasekne; zkrácený nebo prodloužený se vždy čte jako poškozený. `HCC_FUZZ_SEED` a `HCC_FUZZ_ROUNDS` test rozšiřují |
| `c/hcc.h` · `c/hcc.c` | tentýž kodér a dekodér v C99, závislý jen na malloc a free; překládá se s `-ffp-contract=off`, aby prediktor zaokrouhloval každou operaci v double jako Python, a autokorelaci sčítá přesně ve 128 bitech |
| `c/hcc_tool.c` · `c/Makefile` | C ovládané přes stdin a stdout, pro `tests/test_c.py` |
| `tests/test_c.py` | C proti Pythonu: všechny případy výše a náhodné segmenty — každá šířka pole, obě pořadí bajtů, referenční kanály, krajní a přetékající hodnoty, sekundy kdekoli ve 32 bitech — zakódované oběma a porovnané bajt po bajtu, každý proud přečtený zpět v C a každý poškozený proud posouzený oběma stejně, se stejnými záznamy. `HCC_C_SEED` a `HCC_C_ROUNDS` ho rozšíří |

```
python3 tests/test_hcc.py
python3 tests/test_damaged.py
python3 tests/test_c.py                                   # builds c/ with $CC
python3 measure.py recording.mseed --seconds 1,2,4,8,16,32
python3 measure.py obs.min --seconds 60,300,900,3600      # IAGA-2002, nT × 100
```

`measure.py` čte miniSEED přes obspy a IAGA-2002 sám; stopa je jedna celočíselná řada a šířka pole
je nejmenší šířka v celých bajtech, do které se její hodnoty vejdou.

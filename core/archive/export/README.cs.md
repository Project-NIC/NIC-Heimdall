★ N.I.C. ★

# Exportéry

[English](README.md) · **Čeština** · [Русский](README.ru.md)

To, co si svět bere, zapsané z archivu dodatečně, jak každý z nich zavádí [`../EXPORTERS.md`](../EXPORTERS.md).
**Nic tady není záznamem stanice**: záznamem jsou soubory MLF a knihovna v [`../ref/`](../ref/), která
je zapisuje a čte; exportér je jen čte, na serveru nebo na notebooku, a tytéž soubory a týž požadavek
zapíší vždy tytéž bajty. **miniSEED se StationXML a IAGA-2002 jsou z NIC-SDK** (`tools/mlf2mseed.*`,
`tools/mlf2iaga.*`), pro jakýkoli profil; Heimdall jim dává své lepidlo, čtení svých záznamů.

| soubor | co |
|---|---|
| `glue.py` | lepidlo Heimdallu pro exportéry NIC-SDK: kanál z `LAYOUT` podle jména nebo jako `#c`, n · rows za sekundu; stanice ze `STATION`; GPS − UTC z `TIME`; vzorky segmentu přes referenční knihovnu, CLIPPED a DISTURBED jako bity kvality SEED; orientace SEED podle jména, jinak číslo kanálu |
| `tables.py` | každý záznam adresy s rozložením, pravidlem a GPS − UTC platnými v jeho čase; jeden soubor CSV na adresu; tabulky SQLite, které PostgreSQL převezme, jak jsou, a vedle nich katalog souborů |
| `tests/test_mseed.py` | soubor syntetického Quake přes NIC-SDK a lepidlo, zpět načtený ObsPy vzorek po vzorku, mezera a bity kvality na svém místě, kódy tří os, StationXML platné podle schématu FDSN |
| `tests/test_iaga.py` | den syntetického magnetometru na 32 Hz přes NIC-SDK a lepidlo, zpět načtený podle pevných sloupců: stanice, minuta mezery, minuta tam, kde ji dávají filtry |
| `tests/test_tables.py` | CSV a SQL hodnotu po hodnotě proti vlastnímu čtení referenční knihovny každého testovacího souboru, pozdější záznam pozdního rámce zachován, katalog jeden řádek na segment a konfiguraci |

```
python3 ../../../../NIC-SDK/tools/mlf2mseed.py glue.py OUT.mseed NET STA LOC '#0' FILE…
python3 ../../../../NIC-SDK/tools/mlf2iaga.py glue.py DIR CODE SOURCE minute magX,magY,magZ FILE…
python3 tests/test_mseed.py          # needs ObsPy
python3 tests/test_iaga.py
python3 tests/test_tables.py
```

NIC-SDK se najde vedle tohoto repozitáře nebo tam, kam ukazuje `NIC_SDK` (`../ref/sdk.py`); formáty
drží jeho vlastní testy — rámce Steim-2, filtry proti SciPy, minuty z Budkova.

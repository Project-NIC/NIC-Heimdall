★ N.I.C. ★

# Exportéry

[English](README.md) · **Čeština** · [Русский](README.ru.md)

To, co si svět bere, zapsané z archivu dodatečně, jak každý z nich zavádí [`../EXPORTERS.md`](../EXPORTERS.md).
**Nic tady není záznamem stanice**: záznamem jsou soubory MLF a knihovna v [`../ref/`](../ref/), která
je zapisuje a čte; exportér je jen čte, na serveru nebo na notebooku, a tytéž soubory a týž požadavek
zapíší vždy tytéž bajty.

| soubor | co |
|---|---|
| `export.py` | řada jednoho kanálu vyčtená ze souborů jedné adresy v jejích okamžicích UTC; miniSEED 2.4 se Steim-2 v záznamech po 512 B a FDSN StationXML 1.2; IAGA-2002 s minutovým filtrem INTERMAGNET a dolní propustí USGS na 1 Hz, škálovanou podle vzorkování |
| `tables.py` | každý záznam adresy s rozložením, pravidlem a GPS − UTC platnými v jeho čase; jeden soubor CSV na adresu; tabulky SQLite, které PostgreSQL převezme, jak jsou, a vedle nich katalog souborů |
| `tests/test_mseed.py` | soubory syntetického Quake exportované a zpět načtené ObsPy vzorek po vzorku, mezera a bity kvality na svém místě, rámce proti `../ref/steim2.py`, StationXML platné podle schématu FDSN |
| `tests/test_iaga.py` | filtry proti filtrům SciPy; vlastní minuty Budkova z jeho vlastních sekund až na zaokrouhlení; den syntetického magnetometru na 32 Hz zpět načtený podle pevných sloupců |
| `tests/test_tables.py` | CSV a SQL hodnotu po hodnotě proti vlastnímu čtení referenční knihovny každého testovacího souboru, pozdější záznam pozdního rámce zachován, katalog jeden řádek na segment a konfiguraci |

```
python3 tests/test_mseed.py          # needs ObsPy
python3 tests/test_iaga.py           # needs SciPy; BDV_SEC and BDV_MIN name Budkov's day, if fetched
python3 tests/test_tables.py
```

Testovací soubory jsou ty z reference (`../ref/tests/test_hmlf.py`); jak stáhnout den z Budkova, je v
`../ref/README.md`.

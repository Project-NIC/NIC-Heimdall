★ N.I.C. ★

# marconi/models — orientace smyčky

[English](README.md) · **Čeština** · [Русский](README.ru.md)

Aritmetika za oddílem *One loop, standing, and its two nulls are a siting condition* v
[`../CONSTRUCTION.md`](../CONSTRUCTION.md): azimut po hlavní kružnici ze stanice ke každému
sledovanému vysílači a rovina prstence natočená na úhel, který maximalizuje `|cos(bearing − plane)|`
nejhoršího cíle. Prstenec se přišroubuje pod vytištěným úhlem dřív, než se postaví stožár; za
provozu se nic neotáčí.

- [`pointing.py`](pointing.py) — čistý Python, žádné balíčky. Spusťte ho se souřadnicemi stanice:

```
python3 pointing.py 50.08 14.43
```

vypíše rovinu a nuly diagramu, nejhorší cíl a tabulku azimutu, úhlu od roviny a odezvy pro každý
vysílač. Vestavěná tabulka obsahuje stálé časové stanice a značky z [`../BAND.md`](../BAND.md), jejichž
polohy jsou zveřejněny; lokalita předá vlastní seznam jako CSV (`name,kHz,lat,lon`) třetím
argumentem. Propočtená tabulka v `../CONSTRUCTION.md` je výstupem tohoto programu pro
50,08° N 14,43° E.

★ N.I.C. ★

# Njord — vzdušná část Heimdallu

[English](README.md) · **Čeština** · [Русский](README.ru.md)

> **Koncept ve fázi návrhu — nic není postaveno.** Čísla se ověřují proti katalogovým listům a součástkám, než vznikne deska.

**Njörðr je ta část Heimdallu, která dosáhne rádiem.** Midgard — stanice na kabelech a skle — měří
tam, kam dosáhne kabel; Njord měří za ním: pomalé veličiny, vzorek každých deset minut, z malých
jednotek na vlastním panelu a článku, připojených k Heimdallu nebo rovnou do internetu přes LoRa na
868 MHz. Nic rychlého po něm nejede: vzduch EU 868 unese v průměru desítky bitů za sekundu, kdežto
seismometr potřebuje kilobity.

| jednotka | co to je | kde stojí | MCU |
|---|---|---|---|
| **[Iris](iris/)** | přijímač: MOD na větvi Palatine, ModBus typ 8, každá polní jednotka jako multičidlo za její adresou | ve stanici, anténa na stožáru | STM32H523 (LQFP48) |
| **[Arke](iris/arke/)** | **deska Iris ve druhé roli** — vlastní větev ModBus v terénu: kupovaná čidla RS-485 a vlastní MODy, vlastní 12V pack a panel, GNSS pro čas | stožár v terénu | STM32H523 (LQFP48) |
| **[Ratatoskr](ratatoskr/)** | malá jednotka: teplota a vlhkost ve 2 m a v 5 cm, tlak, srážky tam, kde je Imber; jeden článek, malý skleněný panel | strom nebo 3m tyč, rohy a střed sadu | STM32U535 (LQFP48) |
| **[Huginn](huginn/)** | brána do internetu úplně bez Heimdallu — LoRa dovnitř, Wi-Fi ven, čas z NTP | okno, balkon, napájení z USB | ESP32-S3 |

```
   MIDGARD (the station)                                   NJORD (the field)
   Palatine ─── ModBus arm ─── IRIS  ))) 868 MHz (((  ARKE ── its own arm ── bought sensors · Ceres · Sakura
                                     │                (12 V pack, BMS, MPPT, panel, GNSS)
                                     ))) 868 MHz (((  RATATOSKR × n  (one cell, a small panel)
   the internet ─── Wi-Fi ── HUGINN  ))) 868 MHz (((  the same units, with no station
   a LoRaWAN network ─ its gateways  ))) 868 MHz (((  the same units, hourly, with neither
```

**Jedno spojení pro všechny čtyři** ([`LINK.md`](LINK.md)): paket každých deset minut, potvrzený;
hodnoty jako registry domácí mapy, big-endian, takže přijímač předá dál přesně ty bajty, které
přiletěly; 8B tag na každém paketu i potvrzení; čas razítkovaný tam, kam paket dorazí — u Iris
dotazem Palatine, u Huginna z NTP — jednotka udává jen, jak dlouho před odesláním měřila. Jedno či
dvě nastavení se mohou poslat zpět v potvrzení; nikdy registry, nikdy firmware.
**Každá jednotka jede i v LoRaWAN**, veřejné nebo vlastní síti místa — stejný paket jako payload,
šest vzorků za hodinu tam, kde platí fair use veřejné sítě (`LINK.md` §7).

**Co která jednotka měří, se řídí tím, co potřebují rozhodnutí na poli.** Ratatoskr dává mráz, rosu
a hodiny ovlhčení listu, tlak a srážky — výstrahu před mrazem a modely houbových chorob. Arke přidává
vítr ve 2 m (při větru se nestříká), globální záření, půdu (Ceres) a měřené ovlhčení listu (Sakura) —
s nimi evapotranspiraci a závlahu. UV, ozon a prachové částice jsou věcí stanice.

| soubor | co obsahuje |
|---|---|
| [`LINK.md`](LINK.md) | rádiové spojení — pásma, kadence, pakety bajt po bajtu, potvrzení, tag a klíče, sloty, LoRaWAN, dosah |
| [`iris/`](iris/) | Iris a její deska — hardware a firmware obou rolí |
| [`iris/arke/`](iris/arke/) | Arke — čím se liší od Iris |
| [`ratatoskr/`](ratatoskr/) | Ratatoskr — deska, sondy, firmware |
| [`huginn/`](huginn/) | Huginn — brána |
| [`calc/`](calc/) | energetické bilance (`power.py`) a dosah (`link.py`), spustitelné tak, jak jsou |

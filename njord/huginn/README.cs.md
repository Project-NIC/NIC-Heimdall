★ N.I.C. ★

# Huginn — brána Njordu do internetu

[English](README.md) · **Čeština** · [Русский](README.ru.md)

> **Koncept ve fázi návrhu — nic není postaveno.** Čísla se ověřují proti katalogovým listům a součástkám, než vznikne deska.

**Huginn přijímá jednotky Njordu tam, kde žádná stanice nestojí, a jejich data posílá dál přes
internet** — malá jednotka s LoRa a Wi-Fi u okna nebo na balkoně, napájená z USB. Pro jednotky je to
přijímač jako Iris: potvrzuje, drží takt jejich slotů, nese zpět nastavení; pro internet je to klient,
který každý záznam jednotky odešle ve formátu polní jednotky na server, který má nastavený. Brána
LoRaWAN dělá totéž se síťovým serverem za sebou; Huginn žádný nepotřebuje.

| | |
|---|---|
| deska | ESP32-S3 s Wio-SX1262 (kit Seeed XIAO ESP32-S3 & Wio-SX1262, kupovaný) |
| rádio | jeden kanál a jeden spreading factor najednou, přeladěný ve slotu každé jednotky — vlastní paket Njordu nepotřebuje osmikanálový koncentrátor LoRaWAN |
| uplink | Wi-Fi; záznam se posílá na server nastavený na jeho stránce |
| čas | NTP přes jeho Wi-Fi — milisekundy až desítky — razítkuje příchod každého paketu; stáří od jednotky dává čas měření |
| nastavení | vlastní webová stránka na vlastním přístupovém bodě WPA2, otevřeném jen pro nastavení: síť, server, klíče jednotek a jejich sloty |
| napájení | 5 V z USB |
| spojení | [`../LINK.md`](../LINK.md) |

**Jeden Huginn unese mnoho jednotek:** jeho vzduchem jsou potvrzení, ~7 800 za hodinu při SF7 v rámci
10 % pásma h1.7. Počet určuje dosah, ne vzduch: jednotka metr nad trávou dosáhne na Huginna u okna
na několik kilometrů v otevřené krajině, méně přes stromy a zdi (`LINK.md` §1).

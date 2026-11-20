★ N.I.C. ★

# Iris — přijímač Njordu ve stanici

[English](README.md) · **Čeština** · [Русский](README.ru.md)

> **Koncept ve fázi návrhu — nic není postaveno.** Čísla se ověřují proti katalogovým listům a součástkám, než vznikne deska.

**Iris přivádí pole do stanice: MOD na větvi Palatine, ModBus typ 8, rádiová brána.** Přijímá každou
jednotku Njordu v dosahu, každou potvrzuje, drží takt jejich slotů a nejnovější záznam každé jednotky
drží v okně registrů, které Palatine čte jako každé čidlo. **Každá polní jednotka je multičidlo za
adresou Iris** — tak jako Palatine už čte kupovanou krabici mnoha hodnot v jednom bloku (plyny
Chinooku): první registr bloku je číslo jednotky, zbytek její hodnoty, a profil jednotky — klíčovaný
typem Iris a tím číslem — říká, co který registr je.

**Jedna deska, dvě role.** Tatáž deska v terénu je **[Arke](arke/)**; kterou z nich je, zjistí při
prvním startu z proudu, který odebírá její spínaný výstup, a drží si to, dokud ji magnet neresetuje.

| | |
|---|---|
| třída | **MOD** — ModBus typ **8**, rádiová brána; jeden slave na větvi Palatine, až čtyři Iris na stanici |
| napájení | izolovaných 12 V větve na samčím vstupu M12, s párem větve |
| rádio | Wio-SX1262 na 868 MHz, anténa ~3 dBi na izolačním držáku stožáru, ~8 m vysoko |
| MCU | STM32H523, LQFP48 |
| spojení | [`../LINK.md`](../LINK.md) |

```
  Palatine arm ── M12 male: 12 V · 0 V · A · B ──▶ IRIS ── IPEX ── N ── coax ── antenna on the mast
                                                   (H523 · SX1262 · 485 · the beat's crystal)
                                                   M12 female: capped
```

| soubor | |
|---|---|
| [`HARDWARE.md`](HARDWARE.md) | deska obou rolí — součástky, piny, konektory, krabice |
| [`FIRMWARE.md`](FIRMWARE.md) | firmware obou rolí — role, přijímač, okna a mapa registrů |
| [`WHY.md`](WHY.md) | co se zvažovalo a zůstalo stranou |
| [`arke/`](arke/) | druhá role desky |

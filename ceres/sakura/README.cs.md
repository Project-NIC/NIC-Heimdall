<p align="center">
  <img src="NIC-Sakura.svg" width="200"/>
</p>

★ N.I.C. ★

# Sakura — čidlo ovlhčení listů

[English](README.md) · **Čeština** · [Русский](README.ru.md)

> **Koncept ve fázi návrhu — nic není postaveno.** Čísla se ověřují proti katalogovým listům a součástkám, než vznikne deska.

**Sakura měří ovlhčení povrchu listů — stav uvnitř porostu, který řídí modely houbových chorob a
načasování rosy a jinovatky — a teplotu destičky, která ho nese: ModBus typ 7, MOD na větvi
ModBus (arm) Palatine.** Je to deska jednotky Ceres v borosilikátové misce jednotky Ceres, hřeben
měří skrz 1 mm skla jako admitanci na třech frekvencích (`../HARDWARE.md`); liší se zadní strana,
místo, kde visí, a křivka v její paměti (`HARDWARE.md`, `FIRMWARE.md`).

**Kde visí, to rozhoduje, co měří.** Na tenkém rameni v porostu, **sklem k obloze pod úhlem 45°**,
aby voda stékala, **bez radiačního krytu**: destička musí vyzařováním chladnout, aby se na ní
vytvořila rosa, a na slunci se ohřát, aby oschla, stejně jako list. Nad ní je jen hrubá nerezová
síťka proti kroupám, nikdy plná deska, protože déšť je ovlhčení a výhled na oblohu je to, co tvoří
rosu. Žádný standardní list ani referenční přístroj neexistuje; modely chorob byly sestaveny vůči
destičkám, jako je tato, takže standardem je destička a druh rostliny je kalibrace agronoma až dál
po proudu.

| | |
|---|---|
| třída | **vlastní MOD** — Modbus RTU slave na větvi Palatine, **ModBus typ 7**, adresa `7«2 \| NUMBER`, 0x1C pro jednotku 0; bez hodin |
| co hlásí | ovlhčení na `0x0000`, teplotu destičky na `0x0001` — teplotu listu, ne vzduchu (`../MODBUS.md`) |
| snímací prvek | vlastní hřeben desky pod sklem misky, sklem k obloze pod úhlem 45° |
| čtení | synchronní detektor na 2²⁰ / 2²² / 2²⁴ Hz — 1,05 / 4,19 / 16,8 MHz (`../HARDWARE.md`) |
| součástky | `STM32H523VE` · `THS4541` · 3× `74LVC1G3157` · `74LVC1G17` · `TMP117` nebo `STS35` · `THVD1450` · `LMR43610` — celá deska jednotky Ceres |
| napájení | izolovaných 12 V z větve na čtyřvodičovém kabelu; deska si vyrábí vlastních 3,3 V |
| kabel | plochý, čtyři vodiče, připájený k desce a zalitý skrz kanálek misky; na jednotce žádný konektor |

## Blokové schéma

```
   a Palatine arm ── A · B · 12 V · GND ── the flat cable
        │
   ┌────┴────────────────────────────────────────────────────────────┐
   │ SAKURA — Ceres's board potted in the same bowl                  │
   │  the comb on the underside ── 1 mm of glass ── the sky, at 45°  │
   │  the thermometer on the board ── the plate's temperature        │
   └─────────────────────────────────────────────────────────────────┘
     held on a thin arm in the canopy by the holder on its back;
     a coarse stainless mesh ~5 cm above the glass, against hail
```

## Soubory

| soubor | obsah |
|---|---|
| [`HARDWARE.md`](HARDWARE.md) | čím se liší od desky jednotky Ceres: prstenec, zadní strana a její držák, rameno a síťka; samotná deska je v `../HARDWARE.md` |
| [`FIRMWARE.md`](FIRMWARE.md) | čím se liší od firmwaru jednotky Ceres: typ, registr, koncové body křivky, korekce na vodní film; samotný firmware je v `../FIRMWARE.md` |
| [`../MODBUS.md`](../MODBUS.md) | kontrakt ModBus obou jednotek |
| [`WHY.md`](WHY.md) | hřbitov — co se zkusilo, co padlo a proč, vlastní pro Sakura; hřbitov desky je v `../WHY.md` |

## Licence

Hardware: CERN-OHL-S v2 (`../../LICENSE-HW`) · Software: MIT (`../../LICENSE`) — Copyright (c) 2026 NIC — Native Intellect Community

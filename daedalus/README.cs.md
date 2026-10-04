<p align="center">
  <img src="NIC-Daedalus.svg" width="200"/>
</p>

★ N.I.C. ★

# Daedalus — stanice jako stavba

[English](README.md) · **Čeština** · [Русский](README.ru.md)

> **Koncept ve fázi návrhu — nic není postaveno.** Čísla se ověřují proti katalogovým listům a součástkám, než vznikne deska.

**Daedalus je fyzická stanice**: stožár, uzemnění, vedení kabelů, skříň, podzemní komora a povrchová
úprava — **všechno, co stanice má a co není deska ani měření.** Je pojmenovaný po mýtickém
staviteli, který z toho, co měl po ruce, stavěl fungující stroje — včetně křídel — a pravidlo, které
tu platí všude, je jeho: **klasické, levné, z běžné nabídky, přizpůsobené stavitelem.**

**Obsahuje jen to, co sdílí každá stanice.** Usazení, sonda nebo trubka čidla patří jednotce, která
je používá, a deska patří svému vlastnímu projektu — nic zde nejmenuje desku ani součástku.

## Blokové schéma

```
   the mast — an isolated air termination where it must be the tallest thing          the antenna on an insulating bracket
        │                                                                             the coax to the enclosure
   ONE common earthing point ◀── every SPD common · the internal system's own bond, hard, no gap in it
        │
   the enclosure: the head, Kronos, the cards, the Galvani boards ══ cables in conduit, one trench ══▶ the units on their posts · the plate · the vault
```

## Soubory

| Soubor | Obsah |
|---|---|
| [`CONSTRUCTION.md`](CONSTRUCTION.md) | stavba: konstrukce a co na nich visí, proti čemu je stanice chráněna, uzemnění, vedení kabelů a nakreslené kabely, skříně, skříň centrály, podzemní komora, povrchová úprava, servis |
| [`WHY.md`](WHY.md) | hřbitov — co se zkusilo, co padlo a proč |
| [`drawings/`](drawings/) | strojní výkresy, doplňované tak, jak vznikají |

## Licence

Hardware: CERN-OHL-S v2 (`../LICENSE-HW`) · Software: MIT (`../LICENSE`) — Copyright (c) 2026 NIC — Native Intellect Community

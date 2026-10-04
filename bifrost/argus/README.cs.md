<p align="center">
  <img src="NIC-Argus.svg" width="200"/>
</p>

★ N.I.C. ★

# Argus — karta nesoucí čtyři segmenty NodBus mini

[English](README.md) · **Čeština** · [Русский](README.ru.md)

> **Koncept ve fázi návrhu — nic není postaveno.** Čísla se ověřují proti katalogovým listům a součástkám, než vznikne deska.

**Argus je karta Bifrost, která nemá nic na pinu `ATTN`** — tatáž deska, tentýž firmware, a nízká
úroveň při startu z ní dělá Argus (`../`). Nemá žádný vlastní senzor: je to **jednotka NodBus na
portu Bifrostu, jejímž jediným úkolem je nést malé taktované jednotky**, mini-NODy — Gauss,
Quark-Tubes, Pascal, typy 1–3 prostoru mini. Čtyři z jejích šesti USART budí každý jeden segment
**NodBus mini**, pátý je její spoj nahoru k Bifrostu, šestý přijímač echa tohoto spoje.

**NodBus mini je NodBus s payloadem (užitečnými daty) 8 B** — stejné rámcování, stejné TDMA, stejné
hodiny na vodiči, stejné adresování `TYPE«4 | NUMBER` — na vlastním stupni Argusu, **2¹⁹**, na všech
čtyřech segmentech. Je synchronní, takže slot je čas: záznam se sestaví proto, že dorazil, když měl,
bez značky a bez pole stáří. **Segment stojí celý UART**, protože TDMA potřebuje nepřetržitý
poslech — to je cena časování a právě proto ModBus, který sdílí jeden UART mezi mnoha zařízeními,
zůstává ve stanici pro to, co žádné hodiny nepotřebuje.

**Osm sond na kartu, stejných osm jako u Bifrostu.** Sklo je bod–bod a měď lze řetězit, takže osm
optických sond jsou dva Argusy. **Směrem nahoru Argus skládá čtyři mini payloady po 8 B do jednoho
payloadu 32 B** na pozicích pevně určených při registraci a bere si **⌈sondy/4⌉ NUMBERů** svého
vlastního typu, 3: čtyři Gaussy na plnou rychlost jsou jeden slot, osm jsou dva. Tabulka segmentů,
kterou karta hlásí, pozice dekóduje; žádný bajt se nepřidává.

**Stojí ve skříni na kříženém kabelu k portu Bifrostu, nebo ve vzdálenosti za deskami Galvani.**
Každý segment odchází přes desky Galvani, protože mini-NOD stojí venku.

```
   BIFROST port ══ the up port, 2²² ══▶ ┌─────────────────────────┐ ══ segment 1, 2¹⁹ ══▶ mini-NODs
   no time bus: ATTN low                │ ARGUS — the card, H523  │ ══ segment 2 ══▶      Gauss · Quark-Tubes · Pascal
                                        │ 1 up + its echo,        │ ══ segment 3 ══▶      each segment a whole USART,
                                        │ 4 segments              │ ══ segment 4 ══▶      8 B TDMA
                                        └─────────────────────────┘
        four 8 B payloads tiled into one 32 B payload · ⌈sondes/4⌉ NUMBERs up the port
```

## Soubory

| soubor | obsah |
|---|---|
| [`HARDWARE.md`](HARDWARE.md) | čím se liší od Bifrostu, a Argus ve vzdálenosti — optika lokality, jeho napájení a jeho vodič 12 V |
| [`../HARDWARE.md`](../HARDWARE.md) | samotná karta — její piny, zásuvky, napájení a součástky, které jsou zároveň Argusovy |
| [`../FIRMWARE.md`](../FIRMWARE.md) | firmware — jeden obraz pro obě role; vrstva Argus v §8, karta jako jednotka v §9 |
| [`WHY.md`](WHY.md) | hřbitov: broadcast zmrazení, formáty agregace, které prohrály, šestnáct mini a zbytek |

## Licence

Hardware: CERN-OHL-S v2 (`../../LICENSE-HW`) · Software: MIT (`../../LICENSE`) — Copyright (c) 2026 NIC — Native Intellect Community

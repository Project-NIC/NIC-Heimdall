<p align="center">
  <img src="NIC-Bifrost.svg" width="200"/>
</p>

★ N.I.C. ★

# Bifrost — karta mezi centrálou a jednotkami

[English](README.md) · **Čeština** · [Русский](README.ru.md)

> **Koncept ve fázi návrhu — nic není postaveno.** Čísla se ověřují proti katalogovým listům a součástkám, než vznikne deska.

**Jedna karta s STM32H523: jeden port nahoru k Mayaku, čtyři porty dolů k jednotkám a absolutní
sekunda vložená do každého rámce, který projde.** Každý ze čtyř UART páteřních linek Mayaku je spoj
MasterNOD bod–bod k jedné kartě; vlastní NodBus začíná až za kartami. Karta je masterem a časovou
autoritou každé odbočky, kterou vlastní, a předává centrále jeden homogenní, absolutně časovaný
proud. **Bez Bifrostu žádná stanice není**: Mayak nemá port pro odbočku a neformátuje žádnou
sekundu, takže i stanice s jedinou jednotkou má mezi centrálou a touto jednotkou kartu.

**Karta formátuje, nepřeposílá.** Každá odbočka je samostatná sběrnice s vlastní časovou doménou
a vlastní plnou kadencí; karta přeskládá to, co říkají její jednotky, do slotů na páteřní lince,
bajt po bajtu shodně, a předřadí tři horní bajty sekundy — takže Mayak jen alokuje místo v souboru
a zapisuje, zcela bez časové aritmetiky. **Každý záznam dorazí k centrále přesně jednu sekundu poté,
co byl naměřen**: karta drží rámce každé jednotky v kruhovém bufferu o hloubce 96 rámců, jednotka
32 rámců, než je odešle.

**Jedna deska, dvě role.** Karta nasazená na plochý kabel Kronosu čte `ATTN` ve vysoké úrovni a je
**Bifrost**: bere 2²² od Kronosu a předává je svým portům nedělené. Mimo plochý kabel čte `ATTN` v nízké úrovni a je
**Argus** (`argus/README.md`): bere 2²² ze svého horního portu a předává svým čtyřem segmentům mini
2¹⁹. Jeden firmware; úroveň na jednom pinu při startu je celý rozdíl.

```
          KRONOS ──the time bus: CLK 2²² · PPS_K · the label──┐
                                                               ▼
       ┌───────────┐                              ┌───────────────────────┐
       │   MAYAK   │══ the trunk, one per card ══▶│ THE CARD — H523       │══▶ port 1 ─┐
       └───────────┘   in-box crossed cable       │ 6 USARTs of 7:        │══▶ port 2  │ a data body and
                                                  │ 1 up + its echo,      │══▶ port 3  │ a power body each;
                                                  │ 4 down                │══▶ port 4 ─┘ Galvani boards
                                                  └───────────────────────┘              or an in-box cable
        up to four cards, one per trunk · at most eight units on any card
```

| | |
|---|---|
| porty | **jeden nahoru, čtyři dolů** — šest USART ze sedmi v H523: horní port, jeho přijímač echa, čtyři dolní |
| jednotek na kartu | **8** při jakékoli kombinaci portů — strop, pro který jsou dimenzovány TDMA páteřní linky a kruhový buffer |
| karet na stanici | až **4**, jedna na páteřní linku centrály — **16 jednotek bod–bod, 32 v libovolné kombinaci**, přičemž čtyři jednotky nad rámec jedné na port pocházejí z měděného segmentu multidrop |
| stupeň odbočky | **2²⁰** pro osamocenou jednotku na portu, **2²¹** pro dvě až osm na řetězeném měděném segmentu, nastavený při registraci podle počtu (`../core/blocks/nodbus.md`) |
| co port přijme | libovolnou desku z rodiny Galvani nebo křížený kabel uvnitř skříně; karta přečte `ID` každého konektoru dřív, než cokoli napájí |
| MCU | **STM32H523**, domácí součástka a pouzdro — záměrně předimenzovaná: vzácnými zdroji jsou UARTy, kanály DMA a záchyty časovačů, a jedno objednací číslo napříč stanicí za ten křemík stojí |

**Karta dělá pouze NodBus.** Větev ModBus (arm) visí na Palatine; jakákoli koncová sběrnice, kterou
lokalita potřebuje, visí na jednotce za odbočkou.

## Strom

```
bifrost/    Bifrost — the card, NodBus master: one link up, four spurs down  NodBus 2
└── argus/  Argus — the same card off Kronos's ribbon, NodBus mini master    NodBus 3
```

## Soubory

| soubor | obsah |
|---|---|
| [`HARDWARE.md`](HARDWARE.md) | karta: hodiny dovnitř a ven, piny, časovače a strom hodin, zásuvky, páteřní linka, napájení, cesta rámce, měření vzdálenosti, součástky, zkoušky na stole |
| [`FIRMWARE.md`](FIRMWARE.md) | popis firmwaru: start, `FLOOR`, časová značka, kruhový buffer, vrstva Argus, řídicí rovina, žebříky stupňů, registry |
| [`WHY.md`](WHY.md) | hřbitov: zamítnuté alternativy a překonané stavy, s odůvodněním |
| [`argus/`](argus/) | Argus — tatáž karta v druhé roli: čtyři segmenty NodBus mini pro malé taktované jednotky |

## Licence

Hardware: CERN-OHL-S v2 (`../LICENSE-HW`) · Software: MIT (`../LICENSE`) — Copyright (c) 2026 NIC — Native Intellect Community

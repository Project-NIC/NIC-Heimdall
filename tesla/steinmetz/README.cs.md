★ N.I.C. ★

# Steinmetz — poruchy na elektrických vedeních, z vozidla nebo z lokality

[English](README.md) · **Čeština** · [Русский](README.ru.md)

> **Koncept ve fázi návrhu — nic není postaveno.** Čísla se ověřují proti katalogovým listům a součástkám, než vznikne deska.

**Steinmetz je deska Tesla s vlastním obrazem: NodBus typ 13, NOD.** Obraz dělá jedinou věc: hledá
a klasifikuje impulzní zdroje vázané na síťovou frekvenci na elektrickém vedení — oblouky, korónu,
částečné výboje — a celý H7A3 je věnován tomuto úkolu a ničemu jinému. Jednotka je stejná na střeše
vozidla při 80–100 km/h i na stožáru na pevném stanovišti; mezi oběma variantami se nic neosazuje
ani neodebírá.

- **Deska je Teslova, totožná do poslední součástky** — tři tyče, řetězec, převodník, napájecí
  linky, oba datové konektory. Tři pásma zůstávají; **vstupní obvod je utlumen o 20 dB** na vlastních útlumových svorkách Tesly, takže
  venkovní vedení nad vozem leží uvnitř rozsahu a horizont blesků klesne na 350–700 km
  (`HARDWARE.md`); obraz je dimenzován pro vedení, ne pro bouři za horizontem
  (`FIRMWARE.md`).
- **Na stanici visí jako každá jednotka**, na konci trasy na straně jednotky: komunikační deska pro
  jeho médium a 12 V na jeho svorkách. Ve vozidle je stanicí Proteus v kabině
  (`../../proteus/`), trasa je **jeden hybridní kabel na střechu** přes jednu průchodku; anténa GNSS
  sedí na čelním skle, přijímač na Kronosu jako v každé stanici.
- **Záznam je událost Tesly o 4 B**, offset a amplituda beze změny, dva typové bity znamenají
  **0 oblouk · 1 koróna · 2 částečný výboj · 3 UFO**; šumové dno a stavy zdrojů jedou v druhém NODu
  jako u Tesly. **Co z toho udělá server**: TOA napříč jednotkami podél vedení, jehož poloha je
  známá, je jedna neznámá souřadnice; při časování náběžné hrany dá ~1 000 výbojů za epizodu
  30–35 dB a dvojice jednotek obkročující poruchu rozliší ~150 m — krátký seznam stožárů, a poplachem
  je zdroj, který během několika dnů změní třídu z UFO na oblouk.

Pojmenován po **Charlesi Proteovi Steinmetzovi**, který sepsal aritmetiku přechodových jevů na
přenosových vedeních a v laboratoři vyrobil blesk, aby je otestoval.

## Jak je připojen ke stanici

```
  roof / mast                                       cabin / enclosure
  ┌─────────────────────────────┐   hybrid cable    ┌──────────────────────────────────────────────────────┐
  │ Tesla's board, Steinmetz's  │ 4 pairs + 2 cores │ PROTEUS — Mayak · Kronos · Bifrost                   │
  │ image, type 13              │◀═════════════════▶│ port 1 — the 485 island, 4 pairs on terminals        │
  │   NB IN  ◀─ communication   │  one gland each   │ PWR OUT 1 — the 12 V across a barrier                │
  │           board             │                   │ Polaris on Kronos — coax — antenna on the windscreen │
  │   12V    ◀─ terminals       │                   │ Wi-Fi · modem · Ethernet · USB                       │
  │   3 rods                    │                   └──────────────────────────────────────────────────────┘
  └─────────────────────────────┘
```

## Soubory

| soubor | obsah |
|---|---|
| [`HARDWARE.md`](HARDWARE.md) | čím se liší od sestavy Tesla: spoj na střechu, napájení trasy, napájení z vozidla, anténa, skříň. Deska je `../HARDWARE.md`, celá |
| [`CONSTRUCTION.md`](CONSTRUCTION.md) | montáž na vozidlo — krabice na střešních nosičích, střecha pod tyčemi, kabel, vlastní rušení vozidla; pevné stanoviště se staví jako Tesla |
| [`FIRMWARE.md`](FIRMWARE.md) | obraz — čtyři třídy záznamu, registry, které pevně nastavuje, co centrála dělá pro pohybující se jednotku, co obraz nechává nečinné |
| [`WHY.md`](WHY.md) | vlastní hřbitov Steinmetz; hřbitov desky je `../WHY.md` |

## Licence

Hardware: CERN-OHL-S v2 (`../../LICENSE-HW`) · Software: MIT (`../../LICENSE`) — Copyright (c) 2026 NIC — Native Intellect Community

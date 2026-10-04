<p align="center">
  <img src="NIC-Quark.svg" width="200"/>
</p>

★ N.I.C. ★

# Quark — radiační část

[English](README.md) · **Čeština** · [Русский](README.ru.md)

> **Koncept ve fázi návrhu — nic není postaveno.** Čísla se ověřují proti katalogovým listům a součástkám, než vznikne deska.

> **Quark je radiační část stanice: co měří a dvě sestavy, které to měří.** Fotony, beta a neutrony, čtené **dvěma způsoby, které nesdílejí jedinou součástku**:
>
> - **[Quark-Tubes](tubes/)** — **vysoké napětí.** GM a proporcionální trubice, všechny počítané na jedné desce H523, **jednotka `Quark-Tubes`, NodBus mini typ 2**, pouze počty. Hlavice trubic — GM trubice Photonu, Helion, Gadolin/Rhodion — nenesou žádný MCU.
> - **[Quark-Scintillation](scintillation/)** — **nízké napětí.** Scintilátory čtené elektronikou na dvou deskách H7A3, `Quark-Photon` a `Quark-Neutron/Positron`: **NODy Photon (typ 9) a Positron (typ 11)**, přičemž `Neutron` je kanálem té druhé; počty a energie.
>
> **Samotný `Quark` je tato složka a část jako celek — nikdy jednotka a nikdy deska.** Každá deska nese prefix rodiny, takže se její sestava pozná z názvu. Photon stojí v obou sestavách: GM trubice za odstupňovaným olovem v Quark-Tubes, krystal v Quark-Scintillation — jedna veličina, měřená oběma způsoby.

| jednotka | veličina | Quark-Tubes — vysoké napětí | Quark-Scintillation — nízké napětí |
|---|---|---|---|
| **Photon** | γ + rentgenové záření | tři GM trubice, K1 holá · K2 se zastavenou β · K3 za Pb ([`tubes/HEADS.md`](tubes/HEADS.md)) | CsI(Tl) + SiPM + PIN na `Quark-Photon`, typ 9 ([`scintillation/photon/`](scintillation/photon/)) |
| **Positron** | β, obě znaménka | K1 − K2, holá trubice nad depoziční deskou | plastový blok 25 mm + SiPM na `Quark-Neutron/Positron`, typ 11 ([`scintillation/positron/`](scintillation/positron/)) |
| **Helion · Neutron** | neutrony | Helion, proporcionální trubice He³ / BF₃ na K4 ([`tubes/helion/`](tubes/helion/)) | stoh dvou stínítek (`⁶LiF/ZnS(Ag)` · PMMA · tenké druhé `⁶LiF/ZnS(Ag)`) čtený fotonásobičem, kanál 1 téže desky, dva počty v záznamu Positronu — `Neutron` ([`scintillation/neutron/`](scintillation/neutron/)) |
| **Gadolin / Rhodion** | neutrony | třináct trubic SI-22G kolem konvertoru z Gd nebo Rh, sloučených na K4 ([`tubes/gadolin/`](tubes/gadolin/)) | — |

Neutronová fyzika, na které stojí každá neutronová hlavice, je v [`NEUTRONS.md`](NEUTRONS.md). Jednotky dohromady tvoří relativní monitor *občanské vědy* — nikoli kalibrovanou dozimetrii — jehož hodnotou je **hustá, levná, hodinami synchronizovaná mřížka**, která napříč sítí koreluje radiační záblesky s konkrétními údery blesku (TGF).

## Dvě sestavy a jednotky, kterým slouží

**Radiační jednotky jsou pojmenovány podle veličiny — Photon (γ), Neutron a Helion (n), Positron (β) — a technologie je sestava.** Trubicová strana je **Quark-Tubes**: jeden celek, jeden mini-NOD, který počítá každou trubici. Scintilační strana, **Quark-Scintillation**, jsou **dva klasické NODy na NodBusu — Photon typ 9 a Positron typ 11, dva neutronové počty jsou čtyři bajty záznamu Positronu; typ 10 je rezervován pro `Neutron`** — stojící na **dvou** deskách, jejichž fyzika a vstupní obvody jsou popsány v [`scintillation/photon/SCINTILLATION.md`](scintillation/photon/SCINTILLATION.md).

| | **Quark-Tubes — mini-NOD typ 2** | **Quark-Scintillation — Photon a Positron, NodBus 9 a 11** |
|---|---|---|
| detekuje pomocí | ionizace plynu, počítané jako impulzy | scintilačního světla, digitalizovaného jako průběh |
| hlavice | GM za odstupňovaným Pb (trubicová sestava Photonu) · He³ / BF₃ (Helion) · Gadolin/Rhodion | CsI(Tl)+SiPM+PIN (γ) · plastový blok 25 mm+SiPM (β) · stoh dvou stínítek + fotonásobič (n, `Neutron`, `scintillation/neutron/HARDWARE.md`) |
| vysoké napětí | **ano** — ~400 V pro GM, 1300–1600 V pro trubici He³ ze zdroje 1300–2000 V | **nic nad ~30 V na SiPM a PIN**; fotonásobič `Neutron` na kV zdroji, ~1,2 kV (`tubes/helion/HARDWARE.md`) |
| MCU | **STM32H523** | **STM32H7A3IIT6**, běžné provedení **−40…+85 °C** — a vyžaduje ho **převodník**, nikoli výpočetní výkon ani paměť |
| převodník | žádný — impulzy jdou přímo na vstupy časovačů a EXTI | **`AD9251-80`** — dvoukanálový 14bitový, 80 Msps, prokládaný na PSSI: **88,080384 MHz na pinech, 21 × 2²¹ = 44,040192 MSPS na kanál** |
| výstup | **počty** — čtyři odvozené kanály: beta · měkké γ · tvrdé γ · neutrony | **počty a energie** — plocha impulzu je deponovaná energie, takže z ní plyne skutečná dávka |
| payload | 8 B mini payload (užitečná data): 4× `uint16`, **akumulátory nulované na hranici sekundy** ([`tubes/BUS.md`](tubes/BUS.md)) | jediný 32 B scintilační záznam v každém rámci — počet · součet energií · největší událost · 12 energetických pásem u Photonu, 10 a dva neutronové počty u Positronu, žádné příznaky — každé pole je akumulátor nulovaný na sekundě ([`scintillation/photon/BUS.md`](scintillation/photon/BUS.md)) |
| cena | nízká | vysoká; krystalová sestava tvoří ~60 % kusovníku hlavice |

**Spektrum zůstává v každém případě lokálně** — scintilační jednotky počítají své histogramy na desce a odesílají záznam (počet · součet energií · největší událost · pásma, v každém rámci — průměr, medián a dávka plynou ze součtu až dále v řetězci), nebo, po přepnutí registrem, seznam energií částic pro kalibraci a výzkum; histogram odchází jen zřídka nebo na vyžádání (`scintillation/photon/BUS.md`).

**Hodiny v každém případě přicházejí se sběrnicí.** Quark-Tubes, mini-NOD, zachytává stupeň svého segmentu na **časovači, nikoli na HSE** (`../core/blocks/nodbus.md`). Scintilační NODy stojí za Bifrostem a berou hodiny z vedení jako každý uzel; vzorkovací (encode) kmitočet je **21 × 2²¹**, celočíselná PLL ze stejných hodin vedení, takže převodník běží na vlastním hodinovém stromu stanice a na desce není žádný oscilátor ani zlomkový dělič.

## Proč síť — přiřazení TGF

To je důvod, proč levná mřížka existuje. Protože všechny uzly sdílejí jedny síťové hodiny, nesou úder (bleskový uzel) i počty gama/neutronů (tento uzel) **stejné časové razítko rámce**. Síť pak každý úder **lokalizuje metodou TOA** — mikrosekunda je 300 m — a záblesk **koreluje podle amplitudy** — slabý úder nevytvoří tvrdé gama, silný ano — takže TGF v rámci s několika údery je **přiřazen silnému úderu** a rozlehlá síť určí, *kterému*. Úplná mechanika je v [`../tesla/DETECTION.md`](../tesla/DETECTION.md), *TGF correlation*.

## Bezpečnost

**Detektor, ne zdroj** — žádný štěpný ani regulovaný materiál, žádný neutronový zdroj; nic nevyzařuje. Kalibrace proti Am-Be nebo Cf-252 je volitelná a je věcí instituce, nikdy dílny.

- **Vysoké napětí** — ~400 V na GM trubicích, 1300–1600 V na trubici He³ ze zdroje s odbočkami do 2000 V, koróna do 2400 V: v každém případě smrtelné. Izolace, vybíjecí cesta (kondenzátory drží náboj i po vypnutí), uzavřená skříň, žádná odkrytá svorka; nad ~2 kV povrchové vzdálenosti a zalité spoje.
- **Berylium se nikdy neobrábí** — jeho prach způsobuje beryliózu, senzibilizaci po letech, už při µg/m³. Reflektorem je grafit; berylium jen jako zapouzdřený hotový díl, pokud vůbec.
- **Olovo** — navíjené z fólie, nikdy netavené v interiéru; ruce umýt, nedávat k jídlu.
- **Prášek Gd₂O₃** — nízká toxicita, ale při míchání se nesmí vdechovat; vázaný ve vosku nebo polyethylenu je bezpečný.
- **Aktivační fólie** — po ozáření slabě a krátce aktivní (sekundy až minuty, mangan hodiny); samotné kovy jsou inertní.
- **Plyny** — He³ je inertní; **BF₃ je vysoce toxický**, a proto se pevné vrstvě B-10 dává přednost před plněním BF₃.

## Blok

```
   QUARK-TUBES — HIGH VOLTAGE
   K1 · K2 · K3 GM tubes ───────────┐
   the He³ tube, or the Gd/Rh ring ─┴──▶ QUARK-TUBES (H523) ──▶ mini type 2
   one 400 V module for every GM tube; the kV source for the tube

   QUARK-SCINTILLATION — LOW VOLTAGE
   CsI(Tl) + SiPM + PIN ─────────────────────────▶ QUARK-PHOTON (H7A3 · AD9251) ──▶ Photon, type 9
   the plastic block + SiPM ─────────────────────┐
   the two-screen stack + the photomultiplier ───┴──▶ QUARK-NEUTRON/POSITRON ──▶ Positron, type 11
```

## Strom

```
quark/                Quark — the radiation part
├── NEUTRONS.md       the neutron physics both builds share
├── tubes/            Quark-Tubes — high voltage, counts only                       mini 2
│   ├── HEADS.md      Photon — the GM tubes K1–K3 behind graded lead
│   ├── helion/       Helion — the He³ / BF₃ tube on K4, and the kV source
│   └── gadolin/      Gadolin — the Gd ring on K4, with Rhodion
└── scintillation/    Quark-Scintillation — low voltage, counts and energy
    ├── photon/       Photon — CsI(Tl) + SiPM + PIN, on Quark-Photon                NodBus 9
    ├── positron/     Positron — plastic block + SiPM, on Quark-Neutron/Positron    NodBus 11
    └── neutron/      Neutron — ⁶LiF/ZnS(Ag) + photomultiplier, on the same board   type 10 reserved
```

## Soubory

| Složka / soubor | Obsah |
|---|---|
| [`tubes/`](tubes/) | **Quark-Tubes, vysoké napětí** — jednotka a její jediná deska (mini typ 2), GM hlavice, hlavice He³, prstenec Gadolin/Rhodion |
| [`scintillation/`](scintillation/) | **Quark-Scintillation, nízké napětí** — skupina: Photon, Positron, Neutron |
| [`scintillation/photon/`](scintillation/photon/) | Quark-Scintillation — gama jednotka, `Quark-Photon`; také vrstva H7A3 společná oběma nízkonapěťovým deskám, scintilační fyzika, kontrakt záznamu a jediný obraz firmwaru |
| [`scintillation/positron/`](scintillation/positron/) | Quark-Scintillation — beta jednotka na `Quark-Neutron/Positron` |
| [`scintillation/neutron/`](scintillation/neutron/) | Quark-Scintillation — `Neutron`, fotonásobičový kanál na desce Positronu, typ 10 rezervován |
| [`tubes/helion/`](tubes/helion/) | Quark-Tubes — Helion, trubice He³ / BF₃ a kV zdroj |
| [`tubes/gadolin/`](tubes/gadolin/) | Quark-Tubes — Gadolin a Rhodion, prstenec třinácti trubic na K4 |
| [`WHY.md`](WHY.md) | hřbitov — co se zkusilo, co padlo a proč |
| [`NEUTRONS.md`](NEUTRONS.md) | neutronová fyzika společná třem neutronovým hlavicím — rodiny, řetězec účinnosti, pravidlo krátkého dosahu, účinné průřezy, aktivační terče, trubice podle náplně a kde je koupit |

## Licence

Hardware: CERN-OHL-S v2 (`../LICENSE-HW`) · Software: MIT (`../LICENSE`) — Copyright (c) 2026 NIC — Native Intellect Community

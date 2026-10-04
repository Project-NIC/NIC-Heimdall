<p align="center">
  <img src="NIC-Gadolin.svg" width="200"/>
</p>

★ N.I.C. ★

# Gadolin — Gd neutronová hlavice

[English](README.md) · **Čeština** · [Русский](README.ru.md)

> **Koncept ve fázi návrhu — nic není postaveno.** Společná neutronová fyzika → [`../../NEUTRONS.md`](../../NEUTRONS.md); stavba detektoru → [`CONSTRUCTION.md`](CONSTRUCTION.md); čtecí elektronika → [`HARDWARE.md`](HARDWARE.md); hřbitov — co se zkusilo, co padlo a proč → [`WHY.md`](WHY.md).

**Gadolin** je **levný** způsob, jak chytat **neutrony**. Je pojmenován po **Johanu Gadolinovi**, finském chemikovi, který stojí za gadoliniem — prvkem, o který se opírá. Je rozpočtovým protějškem **Helionu** (prémiové trubice He³): oba jsou neutronové hlavice na **Quark-Tubes** ([`../`](../)) a žádný nenese MCU — impulzy trubic se počítají na H523 Quark-Tubes. Oba počítají stejnou částici, jen jinými triky.

## Jak detekuje

Gadolinium je **neutronový magnet** — `¹⁵⁷Gd(n,γ)` má jeden z největších známých záchytových apetitů (~254 000 barnů). Neutron sám nic neionizuje, takže ho Gadolin nechá spolknout gadoliniem; záchyt pak vyplivne **spršku gama (~7,9 MeV)** a *tu* vidí **Geigerovy–Müllerovy trubice** (rodina **(b)**, záchytové gama — `../../NEUTRONS.md`).

**Konstrukce — vrstvený moderátor + reflektor, ne plochý „Gd/plastový střed“.** Úplná stavba detektoru (proč to vynucuje neutralita neutronu, volba trubic, umístění konvertoru, hvězdicové jádro a varianta s Rh) je v **[`CONSTRUCTION.md`](CONSTRUCTION.md)**. Ve zkratce, zvenku dovnitř:

```
graphite reflector           (encapsulated — conductive dust vs. the 400 V)
 → thin outer moderator       (poly, no Gd) — thermalise + feed neutrons inward
 → 13× SI-22G tubes           (1 central + 12 in a ring)
 → central moderator core     (~1 % Gd₂O₃ dispersed, multi-point star)
```

Moderace je náhodná procházka **s driftem vpřed** (rozptyl n–p udržuje neutrony při zpomalování v pohybu dovnitř), takže **centrální 13. trubice** chytá to, co dodriftuje do středu; hvězdicové jádro rozprostírá odezvu na **energii** neutronů a vazbu Gd↔trubice. **Stejná kostra** nese i **rhodiovou variantu — „Rhodion“** (`¹⁰³Rh(n,γ)¹⁰⁴Rh` → tvrdá beta 2,44 MeV) — viz CONSTRUCTION.md §6: její aktivační zpoždění 42 s **není problém**, protože čas úderu přichází z Tesly, takže se odděluje **čas** (Tesla) od **počtu** (Rh) a integruje se od T0.

- **Účinnost ~1 %** (gama se zachytí jen slabě) — ale díly jsou **levné, odolné a snášejí gama**: žádné He³, žádný toxický BF₃, žádný fotonásobič. Jako vždy je přístrojem **síť**, ne jednotlivá trubice.
- **Varianta:** **aktivační fólie** (`¹⁰³Rh(n,γ)¹⁰⁴Rh` → zpožděná tvrdá beta; rodina **(c)**) vyměňuje rychlost za ~30× víc záchytů na událost. Stejné čtení.
- **Vyhýbá se vzácnému He³.**

## V síti

- Počet jede v **neutronovém kanálu K4 Quark-Tubes** — tomtéž kanálu, který plní trubice Helionu v sestavě, jež nese místo prstence ji (rozložení vlastní `../BUS.md`) — se společným razítkem s **Photonem** (γ) a **Teslou** (blesky) pro korelaci TGF.
- **Sloučení běží na `Quark-Tubes`, ne tady.** Tato deska nenese žádný MCU: třináct katodových vedení přechází přes sestavu na vstupy EXTI H523 a sloučení i počítání patří té desce (`../FIRMWARE.md` §A5).
- **Deska:** vstupní obvody pro každou trubici a třináct vedení ven, jedna deska bez vlastního zdroje — ~400 V přichází po sběrnici z modulu `Quark-Tubes`, jediného zdroje pro všechny GM trubice sestavy ([`HARDWARE.md`](HARDWARE.md)).

## Čtení — 13 trubic, žádný MCU, 13 vodičů ven

**Třináct trubic `SI-22G`, čtených na katodě, každá vlastním vodičem do EXTI `Quark-Tubes`.** Deska není uzel a nenese žádný MCU; `Quark-Tubes` těch třináct slučuje softwarově porovnáním časů příchodu — každá hrana v **okně 16 µs** od první je táž částice, takže jedna událost, která rozsvítila několik trubic, se započítá jednou a dvě oddělené události dvakrát — a výsledek počítá do **K4** (`../FIRMWARE.md` §A5, okno je registr).

Pro **Rh** je čtení stejné; aktivační dozvuk se čte dále v řetězci, kde jsou k dispozici časy úderů z Tesly (`CONSTRUCTION.md` §6).

**Vstupní obvod, pro každou trubici:** anodový řetězec ze sběrnice 400 V, katodový rezistor, na kterém vzniká impulz, a sériový rezistor do pinu, jehož vlastní ochranná dioda ořízne překmit; na impulzním vedení žádný kondenzátor ([`HARDWARE.md`](HARDWARE.md) §3).

## Soubory

| Soubor | Obsah |
|---|---|
| [`HARDWARE.md`](HARDWARE.md) | čtecí elektronika — RC sběrnice, vstupní obvod každé trubice, třináct vedení |
| [`CONSTRUCTION.md`](CONSTRUCTION.md) | konstrukce detektoru — moderátor, reflektor, konvertor |
| [`WHY.md`](WHY.md) | hřbitov — co se zkusilo, co padlo a proč |

## Licence

Hardware: CERN-OHL-S v2 (`../../../LICENSE-HW`) · Software: MIT (`../../../LICENSE`) — Copyright (c) 2026 NIC — Native Intellect Community

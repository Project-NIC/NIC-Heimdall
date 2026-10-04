<p align="center">
  <img src="NIC-Helion.svg" width="200"/>
</p>

★ N.I.C. ★

# Helion — neutronová hlavice He³ / BF₃

[English](README.md) · **Čeština** · [Русский](README.ru.md)

> **Koncept ve fázi návrhu — nic není postaveno.** Hlavice a její kV zdroj → [`HARDWARE.md`](HARDWARE.md); hřbitov — co se zkusilo, co padlo a proč → [`WHY.md`](WHY.md).

**Helion** počítá **neutrony** v trubicové variantě: **proporcionální trubice ³He** — nebo BF₃, nebo trubice s bórovým povlakem, nastavená podle katalogového listu osazené trubice — na neutronovém kanálu `Quark-Tubes` **`K4`**, s **`K4A`** vedle jako ukazatelem zdraví trubice. Název je doslovný: jádro helia-3 je **helion**. Je to prémiová ze dvou neutronových hlavic trubicové varianty; **Gadolin** (`../gadolin/`) je ta levná a sestava nese jednu nebo druhou na stejném `K4`.

**Vlastní kV zdroj**: flyback `LT8331` do Cockcroftova–Waltonova násobiče se stupni po 400 V, regulovaný přes `FBX` ze skutečného výstupu, samostatný modul vedle hlavice — s odbočkami 1300–2000 V pro trubici. Fotonásobič nízkonapěťové varianty, `Neutron`, běží ve své vlastní sestavě na stejném modulu.

## V síti

- **Počet jede v mini rámci Quark-Tubes na `K4`** — tomtéž kanálu, který plní Gadolin v sestavě, jež místo toho nese prstenec (rozložení vlastní `../BUS.md`), se společným razítkem s gama kanály a s údery z Tesly pro korelaci TGF.
- **Umístění a přívody** jako u každé exponované hlavice: vlastní sloupek vedle stanice, nikdy stožár, přívody zakopané.

```
   kV source (LT8331 → ladder, 1300–2000 V) ──▶ He³ / BF₃ tube ──▶ LTC6268 → TLV9061 → two comparators ──▶ K4 · K4A on Quark-Tubes
```

## Soubory

| Soubor | Obsah |
|---|---|
| [`HARDWARE.md`](HARDWARE.md) | trubice, její nábojové vstupní obvody, dva prahy a kV zdroj |
| [`WHY.md`](WHY.md) | hřbitov — co se zkusilo, co padlo a proč |

## Licence

Hardware: CERN-OHL-S v2 (`../../../LICENSE-HW`) · Software: MIT (`../../../LICENSE`) — Copyright (c) 2026 NIC — Native Intellect Community

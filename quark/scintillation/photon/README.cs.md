<p align="center">
  <img src="NIC-Photon.svg" width="200"/>
</p>

★ N.I.C. ★

# Photon — gama jednotka

[English](README.md) · **Čeština** · [Русский](README.ru.md)

> **Koncept ve fázi návrhu — nic není postaveno.** Čísla se ověřují proti katalogovým listům a součástkám, než vznikne deska.

> Deska a vrstva H7A3 společná oběma scintilačním deskám: [`HARDWARE.md`](HARDWARE.md); součástky napájecích napětí jsou převzaté z rodiny (`../../../galvani/HARDWARE.md`). **GM trubicová sestava Photonu je VN varianta**, počítaná na `Quark-Tubes`: [`../../tubes/HEADS.md`](../../tubes/HEADS.md).

**Photon** sleduje **gama a rentgenové záření** — energetické světlo, které vyzařuje radioaktivní rozpad, kosmické záření a gama záblesky doprovázející velké bouřky (TGF). Gama i rentgenové záření jsou **fotony** — jedna veličina, rozlišená původem, nikoli energií — a Photon je jednotka, která ji měří, **ve dvou sestavách**:

- **Scintilace — NOD, NodBus typ 9 `Photon`, jeden NUMBER**: kostka CsI(Tl) + SiPM + PIN na společné desce H7A3/`AD9251`, počty *i* energie každé události. Návrh je v [`SCINTILLATION.md`](SCINTILLATION.md); kontrakt sběrnice v [`BUS.md`](BUS.md).

  **V každém rámci publikuje jeden 32 B záznam**, celý akumulovaný od začátku sekundy, takže rámec 127 nese celou sekundu a rozdíl dvou rámců nese jeden rámec: **počet `uint16` · součet energií událostí `uint32` v keV · největší jednotlivá událost `uint16` · dvanáct energetických pásem**. Průměr je ten součet dělený tím počtem, medián se interpoluje v pásmu, které překračuje 50 %, a dávka je tentýž součet × konstanta — vše dále v řetězci a přesně. Nic, co nelze sčítat, jednotku neopouští, a příznaky jsou bajt `status` v hlavičce rámce, nikoli payload (užitečná data).

  **SiPM a PIN jsou JEDNO měření, přepínané u každé události podle její energie** — SiPM pod 500 keV, PIN nad 700 keV, mezi tím lineární prolínání. Dva přístroje s překrývajícími se okny dávají jedno čtení, ne dvě řady: pod svým stropem je SiPM lepším přístrojem a nad ním zkresluje, u PIN je to naopak. Dvojici hlídá **poměr** obou ploch u každé události, která překročí oba prahy, vůči uložené konstantě — odchylka přes 10 % znamená `HEALTH` DEGRADED, takže kanál, který přechází do saturace, se ohlásí sám, aniž by to stálo adresu.

- **Trubice — VN varianta**: GM trubice za odstupňovaným Pb, K1 · K2 · K3, počítané na `Quark-Tubes` a jedoucí v jeho mini rámci ([`../../tubes/HEADS.md`](../../tubes/HEADS.md)). Vedle scintilační sestavy stojí neutronová strana (**`Neutron`**, kanál v záznamu Positronu, typ 10 je pro něj rezervován) a beta jednotka (**Positron**).

## Převodník a napájecí napětí

`AD9251-80` — dvoukanálový 14bitový, 80 Msps, `DRVDD` 1,8–3,3 V, konfigurace přes SPI, interní dělič hodin 1–8 a pin `SYNC`, takže **bere vzorkovací hodiny, které mu předá sběrnice**, místo aby si vyráběl vlastní (součástku vlastní [`HARDWARE.md`](HARDWARE.md); `AD9265-80` z Marconi je jeho jednokanálový 16bitový bratranec). Dva kanály jsou přesně to, co chce **jedna hlavice**: gama krystal čtou SiPM a PIN zároveň a jejich poměr má smysl jen u téže události, takže tato dvojice vlastní jeden převodník. **Jeden `AD9251-80`, dva kanály, na desku** ([`HARDWARE.md`](HARDWARE.md)). Součástka je taktována slovní rychlostí, 88,08 MHz, a uvnitř dělí dvěma, protože její prokládaný výstup je DDR a PSSI čte jednu hranu; tytéž 88,08 MHz jsou čtecími hodinami PSSI. *(Pouzdro a mapa registrů jsou společné celé rodině — `AD9648`, `AD9258`, `AD9268`, `AD9650` jsou přímou náhradou; žádný z nich není požadován.)*

**Všechno jede na 1,8 V, přesně jako to dělá Marconi.** `DRVDD` zvládne i 3,3 V, ale není k tomu důvod. Přechod na 3,3 V nic nepřináší a stojí druhé schéma napájení. Rozhraní ke stanici zůstává na 3,3 V přes převodníky úrovní.

**Rychlost je `21 × 2²¹` = 44,040192 MSPS na kanál**, 88,080384 MHz na pinech. Prokládání stojí polovinu vzorkovacího kmitočtu, takže binární stupeň by byl 2²⁵; toto je nejvyšší rychlost, která zachovává celočíselný počet vzorků **za sekundu i za rámec**, dá se vyrobit celočíselnou PLL a nechává piny pod 100 MHz PSSI.

**Co dodává, na kanál:**

| kanál | šířka elektrického impulzu | vzorky |
|---|---|---|
| gama CsI(Tl) | 1 µs | **44,0** |
| plastová beta — dosvit 285 ns `EJ-240` na nábojovém stupni 603 ns Positronu | ~600 ns | **~26** |
| náběh jedné mikrobuňky | 54 ns | **2,38** |

První dva jsou to, z čeho se čte energie. **Třetí se neměří**: kalibrační pravítko, krok jedné buňky, je kumulantová statistika proudu temných impulzů mezi událostmi (`SCINTILLATION.md`, *The calibration chain*) a od počtu vzorků nic nepožaduje; další stupeň nahoru by na pinu PSSI nechal méně než 8 % rezervy a nic ho nepotřebuje.

**Na scintilační straně se nesleduje žádné vysoké napětí.** Nic na obou deskách nepřekračuje ~30 V; jediný kilovolt na této straně patří fotonásobiči, na kV zdroji Helionu.

## Soubory

| Soubor | Obsah |
|---|---|
| [`HARDWARE.md`](HARDWARE.md) | deska `Quark-Photon` — a vrstva H7A3 společná oběma scintilačním deskám |
| [`FIRMWARE.md`](FIRMWARE.md) | popis firmwaru — jeden obraz pro tuto desku i desku Positronu |
| [`SCINTILLATION.md`](SCINTILLATION.md) | fyzika a vstupní obvody gama hlavice, kalibrační řetězec, referenční tabulky, kusovník |
| [`BUS.md`](BUS.md) | 32 B záznam, Photonu i Positronu |

## Licence

Hardware: CERN-OHL-S v2 (`../../../LICENSE-HW`) · Software: MIT (`../../../LICENSE`) — Copyright (c) 2026 NIC — Native Intellect Community

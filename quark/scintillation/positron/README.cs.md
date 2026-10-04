★ N.I.C. ★

# Positron — beta jednotka

[English](README.md) · **Čeština** · [Русский](README.ru.md)

> **Koncept ve fázi návrhu — nic není postaveno.** Fyzika, vstupní okénko, dlaždice a celé vstupní obvody jsou v [`../photon/SCINTILLATION.md`](../photon/SCINTILLATION.md); záznam v [`../photon/BUS.md`](../photon/BUS.md). Tento soubor je identitou jednotky.

**Positron** detekuje **beta záření obou znamének** — β⁻ i β⁺ mají stejnou hmotnost a stejnou velikost náboje, ionizují stejně a jedna hlavice čte obě. Název je částice β⁺ a není to jen název: fotonukleární řetězec TGF (`¹⁴N(γ,n)¹³N`, β⁺) vytváří nad bouřkami skutečné pozitrony, na týchž hodinách, kterými tato stanice razítkuje.

**NOD, NodBus typ 11 `Positron` — jedna sestava.** **Plastový scintilační blok 25 × 25 × 25 mm** (`EJ-240`, pomalý plast — `HARDWARE.md`), zabalený do PTFE, se SiPM přímo na jedné stěně — bez vlákna — na společné desce **`Quark-Neutron/Positron`**; jedna skříň ho nese spolu se stínítky kanálu **`Neutron`** a odpovídá jako **JEDEN** NOD, tento. **Nemá žádnou vysokonapěťovou část**: na stanici, která provozuje trubice, je beta kanálem Quark-Tubes, holá trubice K1 proti K2 se zastavenou β, obě 0,5–1 m nad depoziční deskou — nebo surové počty (`../../tubes/HARDWARE.md`, `../../tubes/BUS.md`).

**V každém rámci publikuje jediný scintilační záznam stanice**, celý akumulovaný od začátku sekundy, takže rámec 127 nese celou sekundu a rozdíl dvou rámců nese jeden rámec: **počet `uint16` · součet energií událostí `uint32` v keV · největší jednotlivá událost `uint16` · dva počty `Neutron`, tepelné a epitermální, každý `uint16` · deset energetických pásem**; příznaky jsou bajt `status` v hlavičce rámce, nikoli payload (užitečná data). Průměr je součet dělený počtem a medián se odečte z pásem, obojí dále v řetězci a přesně; dávka je tentýž součet × konstanta. Registr přepne záznam na seznam energií částic, 16 na rámec, pro kalibraci (`../photon/BUS.md`).

**Blok měří energii.** 25 mm plastu je 2,56 g/cm² a nejtvrdší beta, pro kterou je tato jednotka stavěna — **Y-90 při 2,28 MeV** — má dolet 1,1 g/cm², takže se zastaví uvnitř a odevzdá všechnu energii; úplné pohlcení platí zhruba do 5 MeV.

**Na této desce není žádné předávání mezi kanály a nic k přepínání.** Nese jeden kanál SiPM, vedle něj kanál fotonásobiče a vůbec žádný PIN. Plně pohlcená beta Y-90 leží na **~2 % obsazenosti buněk** na `EJ-240` proti ~30 %, kde se odezva začíná ohýbat, a **mion přistává na ~5,1 MeV** — s dvojnásobnou rezervou nad beta pásmem, což z něj dělá pravítko a veto, nikoli kontaminant.

**Neutron jede tady, protože nemá nic jiného k odeslání.** Dvě stínítka za jedním fotonásobičem, práh a dvě výšková okna dávají dva počty a žádnou energii, takže čtyři bajty unesou celý kanál; vlastní NUMBER by stál adresu jen kvůli 28 prázdným bajtům. **NodBus typ 10 je rezervován pro `Neutron`**, kdyby ho nějaká sestava chtěla na vlastním NUMBER ([`../neutron/`](../neutron/)).

**Proč jednotka vůbec existuje: je jediným okem na čisté beta zářiče.** `Sr-90`/`Y-90` se rozpadá **bez jakékoli gama linky** — gama spektrometr jakékoli kvality nevidí nic, dokud je přítomen — a `Kr-85` je tentýž případ ve vzácném plynu. Třicetiletý poločas, analog vápníku, který se ukládá v kostech. Žádný jiný kanál ve stanici tuto třídu událostí nevidí.

Hlavice se dívá dolů na **plastovou depoziční desku 1 m × 1 m**, okénkem dolů, 0,5–1 m nad ní — deska je v každé sestavě, trubicové i scintilační — a geometrii, rozpočet okénka (≤ 30 mg/cm²) a tabulky dosahu izotopů vlastní sekce o nasazení v [`../photon/SCINTILLATION.md`](../photon/SCINTILLATION.md).

## Soubory

| Soubor | Obsah |
|---|---|
| [`HARDWARE.md`](HARDWARE.md) | blok a SiPM, vstupní okénko a deska, na kterou přistávají |
| [`../neutron/`](../neutron/) | `Neutron`, fotonásobičový kanál na téže desce |
| [`WHY.md`](WHY.md) | hřbitov — co se zkusilo, co padlo a proč |

## Licence

Hardware: CERN-OHL-S v2 (`../../../LICENSE-HW`) · Software: MIT (`../../../LICENSE`) — Copyright (c) 2026 NIC — Native Intellect Community

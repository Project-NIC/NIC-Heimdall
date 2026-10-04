<p align="center">
  <img src="NIC-Pluvius.svg" width="200"/>
</p>

★ N.I.C. ★

# Pluvius — vážicí srážkoměr

[English](README.md) · **Čeština** · [Русский](README.ru.md)

> **Koncept ve fázi návrhu — nic není postaveno.** Čísla se ověřují proti katalogovým listům a součástkám, než vznikne deska.

**Pluvius váží srážky: ModBus typ 5, MOD na větvi ModBus (arm) Palatine.** Záchytná plocha 200 cm²
odvádí vodu do nádoby, která **visí na tenzometrickém snímači**; snímač čte ratiometricky převodník
pro váhy a jednotka odpovídá **v gramech** — 1 mm deště je 20 g. Nádobu vypouští peristaltická
hlavice s počítanými otáčkami a každé vypuštění vrátí čerstvou nulu, takže údaj je přírůstek vůči
nule obnovované v každém cyklu a absolutní přesnost snímače do něj nikdy nevstupuje. **Sníh se
neváží** — měří ho radar na větvi Palatine; kroupy padají dovnitř, roztají a zváží se.

**Hmotnost je jediná míra, ve které přichází každá fáze srážek.** Led váží tolik co jeho voda, takže
srážkoměr měří i v mrazu, kde se člunkový srážkoměr zastaví na 0 °C; mrholení, kroupy a rosa
dopadají jako hmotnost; výpar se projeví jako úbytek, viditelný a započtený; a v měřicí cestě se nic
nepohybuje. Záchytná plocha ⌀160 mm překračuje minimum WMO 150 mm, pod nímž ve vzorkování převládají
velké jednotlivé kapky.

| | |
|---|---|
| třída | **vlastní MOD** — Modbus RTU slave na větvi Palatine, **ModBus typ 5**, adresa `5«2 \| NUMBER`, 0x14 pro jednotku 0; bez hodin, údaj označí časem hostitel, když se dotazuje |
| záchytná plocha | **200 cm²**, ⌀160 mm — 1 mm = 20 g |
| snímač | **30 kg, jednobodový, nádoba zavěšená**, třída `C5` — referenční sestava; dimenzování a menší varianty viz `HARDWARE.md`, *Sizing* |
| převodník | `ADS1235`, 24bitový, ratiometrický, PGA 64, střídavé buzení |
| vypouštění | **peristaltická hlavice 24 V na spínaných 24 V z konektoru `PWR EXT` Palatine**, spínaná Palatine podle bitu `PUMP` této jednotky — **sestavou je Kamoer `KPHM600` s pětivodičovým motorem**: 12 W, pulzní výstup, jeden pulz na otáčku, a vodiče chodu a směru, všechny tři přes optočleny na malé desce ve skříňce hlavice; `KPHM900`, dva vodiče a žádná deska, je alternativa (`HARDWARE.md`, *The head*) |
| napájení | izolovaných 12 V z větve na čtyřvodičovém kabelu, jako každé čidlo na větvi; deska si vyrábí vlastních 3,3 V a 5,0 V pro převodník |
| MCU | `STM32H523VE`, LQFP100 |
| co publikuje | úhrny — `RATE`, poslední úplnou `HOUR` a `DAY` spolu s předchozí, každou s bitem ustálení; hmotnost, vypuštěný objem a otáčky jsou stav dostupný přes tunel (`MODBUS.md`) |

## Jak visí na stanici

- **Na větvi Palatine jako kupované čidlo** — pár větve a její izolovaných 12 V na čtyřech
  vodičích, vlastní `THVD1450` a základní sada ochran na desce. Palatine se každých 10 s dotazuje
  na `RATE · HOUR · STATUS` a odešle tento rozsah jako jeden blok, s požadavkem `PUMP` v jeho
  `STATUS`; zbytek se čte na vyžádání přes tunel.
- **Hlavice je zátěž stanice, ne součást této desky.** Peristaltická hlavice musí nejprve překonat
  tření při stlačení hadičky, než cokoli dopraví, a 0,4 A z větve na to nestačí, takže visí na
  spínaných 24 V na stanici po vlastním dvoužilovém kabelu. Slave nemůže promluvit první, proto
  požadavek jede v dotazu: tato jednotka nastaví v `STATUS` bit 1 `PUMP`, Palatine zvedne `ENABLE`
  desky a zpětně zapíše `HEAD`; jednotka pustí hlavici, sleduje pokles hmotnosti, zastaví na
  základní hladině a odečte nulu. `ENABLE` je na obou koncích stažený k zemi, takže reset,
  přerušený plochý kabel nebo port bez napájení nechají hlavici vypnutou (`HARDWARE.md`,
  *The switch*).
- **Objem se počítá a nefunkční vypouštění odhalí hmotnost**; počet otáček a ampérmetr zdrojové
  desky řeknou, co selhalo (`HARDWARE.md`, *The pickup*, *Detecting a dead drain*).
  Akumulace je `weight + counted volume`, což dovoluje vypouštět i během deště.
- **`DRAIN` je zapisovaný registr**, dosažitelný ze serveru přes tunel, takže stanice, která ví, že
  se blíží fronta, si před ní vyprázdní nádobu; započítané vypuštění nic nestojí.

## Kde stojí

**Vedle skříně Palatine, u stožáru**: 24 V pro hlavici opouští skříň po dvoužilovém kabelu a hadice
vede od nádoby k hlavici, obojí dlouhé několik metrů. Na vzdálené ploše stojí srážkoměr stejně
vedle vzdálené Palatine, na jejím konektoru `PWR EXT`. Záchytná plocha je ve výšce 1 m; lokalita s
hlubokým sněhem nebo horská lokalita zvedne celou jednotku na sloupku nad maximální výšku sněhu.
Nádoba, plášť, trubka nálevky a hadice viz `HARDWARE.md`, *The vessel, the shell and the
tubes*.

## Blokové schéma

```
   a Palatine arm ── A · B · 12 V · GND ──▶ ┌────────────────────────────────────────┐
                                            │ PLUVIUS — H523 · THVD1450, MOD type 5  │── the load cell ── ADS1235 ── grams
                                            └────────────────────────────────────────┘
   Palatine's EXT body ══ switched 24 V, its own 2-core ══▶ the head's tail board ── the head — switched by ENABLE on this unit's PUMP status bit
   PLUVIUS ── 3,3 V · GND · PULSE · RUN · DIR, a thin cable ──▶ the tail board's three optocouplers ── yellow · white · green — the pulse in, run and direction out
```

## Soubory

| soubor | obsah |
|---|---|
| [`HARDWARE.md`](HARDWARE.md) | deska, snímač a jeho dimenzování, nádoba a plášť, hlavice a její koncová deska, snímání otáček, pravidlo nefunkčního vypouštění, spínač, každý pin, součástky |
| [`FIRMWARE.md`](FIRMWARE.md) | popis firmwaru — hmotnost, stavový automat vypouštění, tárování, období a úhrny, registry, co je otestováno |
| [`MODBUS.md`](MODBUS.md) | kontrakt ModBus |
| [`WHY.md`](WHY.md) | hřbitov — co se zkusilo, co padlo a proč |

## Licence

Hardware: CERN-OHL-S v2 (`../LICENSE-HW`) · Software: MIT (`../LICENSE`) — Copyright (c) 2026 NIC — Native Intellect Community

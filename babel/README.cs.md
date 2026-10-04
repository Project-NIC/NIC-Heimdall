<p align="center">
  <img src="NIC-Babel.svg" width="200"/>
</p>

★ N.I.C. ★

# Babel — převodník protokolů

[English](README.md) · **Čeština** · [Русский](README.ru.md)

> **Koncept ve fázi návrhu — nic není postaveno.** Čísla se ověřují proti katalogovým listům a součástkám, než vznikne deska.

**Babel dostane na větev ModBus (arm) čidlo, které se neprodává jako jednotka Modbus.** Vlastní
rozhraní čidla — I²C, SPI, UART nebo 1-Wire — vstupuje z jedné strany; z druhé vychází Modbus RTU
a hostitel čte hotové číslo v registrové podobě, která je v projektu standardem. Deska je pro každé
čidlo stejná; mění se profil firmwaru zkompilovaný pro danou pozici.

**Po stanici necestuje žádná cizí sběrnice.** Čidlo, které nemluví Modbus, se převádí u čidla, na
této desce, a nikdy se nepřenáší přes stanici, aby se rozplétalo až na druhém konci.

**Univerzální není deska, ale procesor.** Deska, která nese ovladač pro každé čidlo, není nikdy
hotová, takže tahle nenese žádný: je to `STM32H523` s vyvedenými periferiemi — SPI do 2²⁶ bit/s,
USART do 2²³ baud, I²C, hodinový výstup, 59 volných pinů — na stejném krystalu a se stejným
snižujícím měničem jako každá deska H523 ve stanici, a stavitel ji nakonfiguruje pro jedno čidlo.
Základní firmware běží, i když není nic připojeno: deska odpovídá na sběrnici svou identitou, svým
standardním blokem a nulami a k tomu se přidává profil pro každé osazené čidlo.

| | |
|---|---|
| třída | vlastní MOD — Modbus RTU slave na větvi **Palatine**; varianta pro NodBus neexistuje |
| kód typu | **žádný.** Každá osazená pozice odpovídá jako samostatný slave na typu **své veličiny**, `TYPE«2 \| NUMBER`; `0xFF01 IDENT` říká, že tyto slavy jsou jedna deska. **Deska bez osazených čidel odpovídá jednou na ModBus typu 4, holá**, se svou identitou a nulami (`MODBUS.md`) |
| pozice | až **čtyři** čidla, každé s jednou adresou |
| rozhraní čidel | I²C · SPI · UART · 1-Wire · 2× GPIO · hodinový výstup, vše 3,3 V |
| MCU | `STM32H523VE`, LQFP100 |
| napájení | izolovaných 12 V z větve na čtyřvodičovém kabelu, jako u kupovaného čidla; deska si vyrábí 3,3 V na `LMR43610` |
| vstup linky | `THVD1450`, základní sada na desce (`HARDWARE.md`) |
| firmware | standardní jádro MOD a jeden profil na každé osazené čidlo (`FIRMWARE.md`) |

## Blokové schéma

```
   the ModBus arm — A · B · 12 V · GND
        │
   ┌────┴────────────────────────────────────────┐
   │ BABEL — H523 · LMR43610 → 3,3 V             │──▶ I²C · SPI · UART · 1-Wire · 2× GPIO · a clock ──▶ up to four sensors
   │ THVD1450 · 2× 10 Ω + SM712 · 2× 5.0SMDJ18A  │    one ModBus address per fitted position,
   └─────────────────────────────────────────────┘    0xFF01 IDENT says they are one board
```

## Soubory

| soubor | obsah |
|---|---|
| [`HARDWARE.md`](HARDWARE.md) | deska — napájení, vstup linky, svorkovnice, konektor čidel a jeho ochrana, každý pin, součástky |
| [`MODBUS.md`](MODBUS.md) | identita, adresování, registry, které čte hostitel |
| [`FIRMWARE.md`](FIRMWARE.md) | popis jádra a profilů |
| [`WHY.md`](WHY.md) | hřbitov — co se zkusilo, co padlo a proč |

## Licence

Hardware: CERN-OHL-S v2 (`../LICENSE-HW`) · Software: MIT (`../LICENSE`) — Copyright (c) 2026 NIC — Native Intellect Community

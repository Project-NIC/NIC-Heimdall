★ N.I.C. ★

# Proteus — centrála, hodiny, Bifrost a Hermes na jedné malé DPS

[English](README.md) · **Čeština** · [Русский](README.ru.md)

> **Koncept ve fázi návrhu — nic není postaveno.** Čísla se ověřují proti katalogovým listům a součástkám, než vznikne deska.

**Proteus je nejmenší DPS, která je celou centrálou stanice: Mayak, Kronos, Bifrost a Hermes na jedné
desce plošných spojů, Polaris do ní zasunutý, s jednou měděnou trasou NodBus přímo na desce a ničím
dalším.** Žádný datový konektor, žádný volný port, žádný plochý kabel: čtyři datové páry jediné trasy
končí na svorkách, napájení trasy 12 V je ta jediná zásuvná deska — izolovaný zdrojový modul 12 V na svém
napájecím konektoru, takže přepětí spálí modul, a ne desku — a deska je napájena přes izolovaný modul
DC/DC typu brick na DPS z 12 V vozidla nebo z akumulátoru lokality. Vejde se do šachty rádia 1-DIN
nebo do malé krabice.
Každá sekce je rozvržena tak, jak ji kreslí její vlastní dokument, piny i firmware beze změny —
klasická struktura stanice na jedné DPS. **Modem je modem LTE-M** a spolu s Mayakem a Hermesem je
tím, co nese záložní článek: když zmizí 12 V, poslední hlášení přesto odejde.

Pojmenován po prostředním jménu, které nesl **Charles Proteus Steinmetz**.

```
   vehicle 12 V / pack ──▶ [isolated DC/DC brick] ──▶ the 12 V node ── INA238 (LP I²C)
                                                        │
      ┌─────────────────────────────────────────────────┴─────────────────────────────────────────────────────────────────────┐
      │  KRONOS ◀ TIME IN 1 ◀ POLARIS ── CLK · PPS_K · label as traces ──▶ BIFROST                                            │
      │                                                  ▲        port 1 ──▶ 485 island on the board ──▶ 4 pairs on terminals │
      │  MAYAK ── one MasterNOD as traces ───────────────┘        PWR OUT 1 ──▶ the plug-in 12 V cell ──▶ feed pair           │
      │    ├── LP I²C ──▶ HERMES ── 485 · CAN · UART · I²C out                                                                │
      │    ├── the modem, LTE-M ── on the backup cell's rail with the Mayak and Hermes                                        │
      │    └── SD ×2 · Wi-Fi · BLE · USB-C · Ethernet                                                                         │
      └───────────────────────────────────────────────────────────────────────────────────────────────────────────────────────┘
```

## Soubory

| soubor | obsah |
|---|---|
| [`HARDWARE.md`](HARDWARE.md) | sekce, co se stane vodivou cestou, trasa na desce, napájení a modul DC/DC, dva snižující měniče, co je osazeno, tvar |
| [`WHY.md`](WHY.md) | hřbitov |

## Licence

Hardware: CERN-OHL-S v2 (`../LICENSE-HW`) · Software: MIT (`../LICENSE`) — Copyright (c) 2026 NIC — Native Intellect Community

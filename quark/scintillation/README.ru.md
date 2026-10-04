<p align="center">
  <img src="NIC-QuarkScintillation.svg" width="200"/>
</p>

★ N.I.C. ★

# Quark-Scintillation — низковольтные радиационные блоки

[English](README.md) · [Čeština](README.cs.md) · **Русский**

> **Концепция на стадии проектирования — ничего не построено.** **Низковольтная сборка Quark**, радиационной части ([`../README.md`](../README.md)); высоковольтная сборка — [`Quark-Tubes`](../tubes/). Этот файл — карта группы: что в неё входит и где что описано.

**Сцинтилляторы, считываемые электроникой, на двух платах.** Кристалл, пластиковый блок или экран превращают частицу во вспышку света; SiPM, PIN-диод или фотоумножитель превращают свет в импульс; плата оцифровывает импульс как форму сигнала, и его площадь есть выделенная энергия. Поэтому эта сторона даёт **счёт и энергию**, тогда как трубки дают только счёт. Ничто на ней не работает выше ~30 V, кроме фотоумножителя `Neutron`.

| блок | величина | чувствительный элемент | плата | на шине |
|---|---|---|---|---|
| **[Photon](photon/)** | γ + рентген | куб CsI(Tl), считываемый одновременно SiPM и PIN, одно измерение с переключением по энергии | `Quark-Photon` | **NOD, NodBus тип 9** |
| **[Positron](positron/)** | β, обоих знаков | пластиковый сцинтилляционный блок 25 mm + SiPM | `Quark-Neutron/Positron` | **NOD, NodBus тип 11** |
| **[Neutron](neutron/)** | нейтроны, тепловые и эпитепловые | два экрана `⁶LiF/ZnS(Ag)` с PMMA между ними, считываемые фотоумножителем | `Quark-Neutron/Positron`, канал 1 | **канал Positron** — его два счёта идут в записи Positron; тип 10 зарезервирован |

**Две платы, одна конструкция.** Обе — платы `STM32H7A3IIT6` с `AD9251-80` на PSSI — одна и та же цифровая половина, единый образ прошивки. `Quark-Photon` несёт SiPM и PIN, оцифровываемые одновременно, с охранным кольцом; `Quark-Neutron/Positron` — один канал SiPM и канал фотоумножителя, без `LTC6268` и без охранного кольца. **~1,2 kV для фотоумножителя даёт kV-источник Helion** (`../tubes/helion/HARDWARE.md`), собранный для этой головки.

**Одна запись для обоих блоков**: 32 B в каждом кадре, каждое поле — аккумулятор, сбрасываемый на секунде, — счёт · сумма энергий · наибольшее событие · 12 энергетических полос у Photon, 10 и два нейтронных счёта у Positron.

## Дерево

```
scintillation/  Quark-Scintillation — low voltage
├── photon/     Photon — γ / X-ray, on Quark-Photon                      NodBus 9
├── positron/   Positron — β, on Quark-Neutron/Positron                  NodBus 11
└── neutron/    Neutron — the photomultiplier channel on the same board  type 10 reserved
```

## Где что описано

Общие документы группы лежат у Photon, первой из двух плат:

| | |
|---|---|
| физика сцинтилляции и весь входной тракт | [`photon/SCINTILLATION.md`](photon/SCINTILLATION.md) |
| слой H7A3, общий для обеих плат, и `Quark-Photon` | [`photon/HARDWARE.md`](photon/HARDWARE.md) |
| контракт записи | [`photon/BUS.md`](photon/BUS.md) |
| единый образ прошивки | [`photon/FIRMWARE.md`](photon/FIRMWARE.md) |
| `Quark-Neutron/Positron` и бета-блок | [`positron/`](positron/) |
| канал фотоумножителя | [`neutron/`](neutron/) |
| нейтронная физика, общая для обеих сборок | [`../NEUTRONS.md`](../NEUTRONS.md) |

## Лицензия

Аппаратная часть: CERN-OHL-S v2 (`../../LICENSE-HW`) · Программы: MIT (`../../LICENSE`) — Copyright (c) 2026 NIC — Native Intellect Community

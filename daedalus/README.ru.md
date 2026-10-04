<p align="center">
  <img src="NIC-Daedalus.svg" width="200"/>
</p>

★ N.I.C. ★

# Daedalus — станция как сооружение

[English](README.md) · [Čeština](README.cs.md) · **Русский**

> **Концепция на стадии проектирования — ничего не построено.** Цифры проверяются по даташитам и самим компонентам, прежде чем будет сделана плата.

**Daedalus — это физическая станция**: мачта, заземление, прокладка кабелей, шкаф, подземная камера
и финишная отделка — **всё, что есть у станции и что не является ни платой, ни измерением.** Назван в
честь мифического мастера, который строил работающие машины — включая крылья — из того, что было
под рукой, и правило здесь везде его: **классика, дёшево, из готового, доработано строителем.**

**Здесь только то, что общее для всех станций.** Основание, зонд или труба датчика принадлежат
блоку, который их использует, а плата принадлежит своему собственному проекту — здесь не названа
ни одна плата и ни один компонент.

## Структурная схема

```
   the mast — an isolated air termination where it must be the tallest thing          the antenna on an insulating bracket
        │                                                                             the coax to the enclosure
   ONE common earthing point ◀── every SPD common · the internal system's own bond, hard, no gap in it
        │
   the enclosure: the head, Kronos, the cards, the Galvani boards ══ cables in conduit, one trench ══▶ the units on their posts · the plate · the vault
```

## Файлы

| Файл | Содержание |
|---|---|
| [`CONSTRUCTION.md`](CONSTRUCTION.md) | постройка: сооружения и то, что на них висит, от чего защищена станция, заземление, прокладка кабелей и нарисованные кабели, шкафы, шкаф центрального блока, подземная камера, финишная отделка, обслуживание |
| [`WHY.md`](WHY.md) | кладбище — что пробовали, что отвергли и почему |
| [`drawings/`](drawings/) | механические чертежи, пополняются по мере создания |

## Лицензия

Аппаратная часть: CERN-OHL-S v2 (`../LICENSE-HW`) · Программы: MIT (`../LICENSE`) — Copyright (c) 2026 NIC — Native Intellect Community

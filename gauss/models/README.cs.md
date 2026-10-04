★ N.I.C. ★

# gauss/models — model umístění v mělké vodě

[English](README.md) · **Čeština** · [Русский](README.ru.md)

Výpočet, na kterém stojí **pravidlo hloubkového umístění** v [`../ARRAY.md`](../ARRAY.md)
(*Where along the slope*): lineární 2D mělkovodní simulace průchodu kolem
kruhového ostrova (šelf → svah → hluboká pláň, přicházející rovinná tsunami), sledující
**max |H·u|** — transport = rychlost × hloubka, zástupná veličina pro pohybovou indukci (magnetometr).

- [`slope_toe_sweep.py`](slope_toe_sweep.py) — model (čisté numpy + matplotlib;
  posunutá C-mřížka, schéma forward–backward, tlumicí okraje). Spuštění: `python3 slope_toe_sweep.py`.
- [`slope_toe_sweep.png`](slope_toe_sweep.png) — výstup uložený v repozitáři. Vlevo: 2D pole špičkového
  transportu (všimněte si zjasnění refrakcí na bocích a mrtvého závětří). Vpravo:
  radiální křivka — signál ~0,30× na šelfu, stoupá po svahu, **~1,00× u paty svahu,
  dál plochá plošina** (kabel navíc směrem k hlubokomořskému příkopu přinese ~0 %).

**Poctivě o rozsahu:** lineární, bez tření, schematická batymetrie — určuje *tvar*
(signál sleduje hloubku; pata svahu je optimum; plošina je plochá) a optimum u paty,
ne druhé desetinné místo. Nelineární výběh vlny na břeh a tření o dno jsou záměrně vynechány.

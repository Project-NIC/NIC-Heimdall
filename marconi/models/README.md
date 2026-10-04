★ N.I.C. ★

# marconi/models — the loop's orientation

The arithmetic behind *One loop, standing, and its two nulls are a siting condition* in
[`../CONSTRUCTION.md`](../CONSTRUCTION.md): the great-circle bearing from the station to every
monitored transmitter, and the plane of the ring turned to the angle that maximises the worst
target's `|cos(bearing − plane)|`. The ring is bolted at the printed angle before the mast goes
up; nothing rotates in service.

- [`pointing.py`](pointing.py) — pure Python, no packages. Run it with the station's coordinates:

```
python3 pointing.py 50.08 14.43
```

prints the plane and the nulls, the worst target, and a table of bearing, angle off the plane
and response per transmitter. The built-in table holds the permanent time stations and markers
of [`../BAND.md`](../BAND.md) whose positions are published; a site passes its own list as a CSV
(`name,kHz,lat,lon`) as the third argument. The worked table in `../CONSTRUCTION.md` is this
program's output for 50,08° N 14,43° E.

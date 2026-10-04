#!/usr/bin/env python3
"""Marconi — the loop's orientation from the station's coordinates.

The loop is one vertical ring, a figure of eight in azimuth: response |cos(bearing - plane)|,
full in the plane of the ring, zero broadside. Every target is a fixed transmitter at a
published position, so the bearing to each is a great-circle calculation and the plane is turned
to the angle that maximises the WORST target's response. Nothing is measured on site; the ring is
bolted at the angle printed here (CONSTRUCTION.md, *One loop, standing*).

    python3 pointing.py LAT LON [transmitters.csv]

LAT and LON in decimal degrees, north and east positive. The optional CSV has one transmitter a
line, "name,kHz,lat,lon"; without it the built-in table below is used — the permanent time
stations and markers of BAND.md whose positions are published. Add the ones your site hears.
"""
import math, sys, csv

TRANSMITTERS = [
    # name, kHz, lat, lon  (north, east positive)
    ("WWV Fort Collins",   5000,  40.6781, -105.0400),
    ("WWVH Kauai",         5000,  21.9872, -159.7629),
    ("CHU Ottawa",         7850,  45.2950,  -75.7580),
    ("RWM Taldom",         4996,  56.7330,   37.6630),
    ("BPM Pucheng",        5000,  34.9500,  109.5600),
    # ("YVTO Caracas",     5000,  10.5000,  -66.9300),   # a target of BAND.md, seldom heard in Europe — uncomment where it is
    ("UVB-76 Buzzer",      4625,  60.3100,   30.2800),
]

def bearing(lat1, lon1, lat2, lon2):
    """Initial great-circle bearing from point 1 to point 2, degrees 0..360."""
    p1, p2 = math.radians(lat1), math.radians(lat2)
    dl = math.radians(lon2 - lon1)
    y = math.sin(dl) * math.cos(p2)
    x = math.cos(p1) * math.sin(p2) - math.sin(p1) * math.cos(p2) * math.cos(dl)
    return (math.degrees(math.atan2(y, x)) + 360.0) % 360.0

def response_db(bearing_deg, plane_deg):
    c = abs(math.cos(math.radians(bearing_deg - plane_deg)))
    return 20 * math.log10(c) if c > 1e-9 else -180.0

def best_plane(bearings, step=0.1):
    """The plane angle (0..180, the figure of eight is symmetric) that maximises the worst response."""
    best, best_worst = 0.0, -1e9
    n = int(round(180.0 / step))
    for i in range(n):
        plane = i * step
        worst = min(response_db(b, plane) for b in bearings)
        if worst > best_worst:
            best, best_worst = plane, worst
    return best, best_worst

def main():
    if len(sys.argv) < 3:
        print(__doc__); sys.exit(1)
    lat, lon = float(sys.argv[1]), float(sys.argv[2])
    tx = TRANSMITTERS
    if len(sys.argv) > 3:
        with open(sys.argv[3]) as f:
            tx = [(r[0], float(r[1]), float(r[2]), float(r[3])) for r in csv.reader(f) if r and not r[0].startswith('#')]
    rows = [(name, khz, bearing(lat, lon, tlat, tlon)) for name, khz, tlat, tlon in tx]
    plane, worst = best_plane([b for _, _, b in rows])
    print(f"station {lat:.4f} N {lon:.4f} E")
    print(f"plane of the ring: {plane:.1f} deg / {plane+180:.1f} deg   nulls: {(plane+90)%360:.1f} / {(plane+270)%360:.1f} deg")
    print(f"worst target: {worst:.1f} dB\n")
    print(f"{'transmitter':<20}{'kHz':>7}{'bearing':>10}{'off plane':>11}{'response':>10}")
    for name, khz, b in sorted(rows, key=lambda r: -response_db(r[2], plane)):
        off = abs(((b - plane) + 90) % 180 - 90)
        print(f"{name:<20}{khz:>7.0f}{b:>9.1f}°{off:>10.1f}°{response_db(b, plane):>9.1f} dB")

if __name__ == "__main__":
    main()

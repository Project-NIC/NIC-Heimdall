#!/usr/bin/env python3
"""Generate storm-network.geojson and cyclone-basins.geojson.

The basins: the seven areas where tropical cyclones form and run, outlined by hand, each
with one typical track and an arrowhead — a sketch of where and which way, not a
climatology.

The storm network is COMPUTED from the transmitters the station hears (SITING.md, *The
storm network*). Marconi and Pip see the ionosphere along the path between a transmitter
and the station, and Tesla locates a stroke by the stations around it. So a candidate
place scores by
  - the paths from it to every transmitter in reach that cross a basin, counted per basin
    and per orientation of the crossing (four bins of 45°), up to PATH_NEED a bin; and
  - for Tesla, the side of each basin it stands on (eight bearings from the basin's
    centre, within TESLA_REACH_KM), one station a side; and
  - for Sputnik and the strongest sferics, the side of each basin it stands NEAR — within
    NEAR_KM of the basin's edge, eight sides, one station a side — so every basin is
    watched close by from the north and the south; and
  - for the globe, the grid point it covers — a point every 20° of latitude from 50° S to
    60° N and every 30° of longitude, covered by a station within COVER_KM, wherever a
    candidate can reach it — so no quarter of the Earth is left without a station.
A greedy pass takes the candidate that adds most, keeping MIN_SPACING_KM between stations,
until nothing is added. The candidates are places with people, a road and power — hills,
observatories, towns, islands — none on a summit swept by hurricane-force wind, none in a
basin but the inner stations at its edge, where a storm is still weak.
simplestyle-spec; coordinates [lon, lat], representative."""
import json, math, os

OUT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
R = 6371.0
PATH_NEED, TESLA_REACH_KM, NEAR_KM, COVER_KM, MIN_SPACING_KM, MAX_STATIONS = 4, 5000, 1500, 2000, 700, 70
GRID = [(la, lo) for la in range(-50, 61, 20) for lo in range(-180, 180, 30)]

# HF time stations — Marconi's Doppler (reach: multi-hop sky wave)
HF = [("WWV", 40.68, -105.04), ("WWVH", 21.99, -159.76), ("CHU", 45.30, -75.75),
      ("RWM", 56.73, 37.63), ("BPM", 34.95, 109.55)]
# HF broadcast sites — Marconi's carrier levels: any fixed transmitter is a path
HF += [("Noblejas", 39.95, -3.43), ("Nauen", 52.65, 12.91), ("Woofferton", 52.31, -2.72),
       ("Issoudun", 46.95, 1.98), ("Abis", 31.13, 30.08), ("Dhabbaya", 24.18, 54.23),
       ("Tashkent", 41.22, 69.15), ("Kashi", 39.36, 75.76), ("Kranji", 1.42, 103.73),
       ("Tinang", 15.35, 120.62), ("Yamata", 36.17, 139.82), ("Shepparton", -36.33, 145.42),
       ("Meyerton", -26.58, 28.17), ("Talata-Volonondry", -18.75, 47.61),
       ("Okeechobee", 27.46, -80.93), ("Greenville", 35.47, -77.20), ("Sackville", 45.88, -64.35)]
HF_REACH_KM = 12000
# VLF transmitters — Pip's SID carriers (reach: the Earth–ionosphere waveguide)
VLF = [("NAA", 44.65, -67.28), ("NLK", 48.20, -121.92), ("NPM", 21.42, -158.15),
       ("NML", 46.37, -98.34), ("NAU", 18.40, -67.18), ("NWC", -21.82, 114.17),
       ("JJI", 32.07, 130.83), ("VTX", 8.39, 77.75), ("DHO38", 53.08, 7.62),
       ("GQD", 54.73, -2.88), ("ICV", 40.92, 9.73), ("NRK", 63.85, -22.47),
       ("HWU", 46.71, 1.25), ("NSY", 37.13, 14.44)]
VLF_REACH_KM = 10000
LF_REACH_KM = {"time": 2500, "eLoran": 1500}   # the longwave layer's own transmitters

# candidates: (name, lat, lon, ring) — ring as placed: equator, north, south, inner
CANDIDATES = [
    ("Sao Tome", 0.34, 6.73, "equator"), ("Gabon, Libreville", 0.39, 9.45, "equator"),
    ("DR Congo, Kisangani", 0.52, 25.19, "equator"), ("Kenya, Nairobi", -1.29, 36.82, "equator"),
    ("Seychelles, Mahe", -4.62, 55.45, "equator"), ("Maldives, Addu", -0.63, 73.16, "equator"),
    ("Indonesia, Padang", -0.95, 100.35, "equator"), ("Singapore", 1.29, 103.85, "equator"),
    ("Indonesia, Ternate", 0.79, 127.38, "equator"), ("Papua New Guinea, Manus", -2.03, 147.27, "equator"),
    ("Nauru", -0.52, 166.93, "equator"), ("Kiribati, Tarawa", 1.42, 173.02, "equator"),
    ("Kiribati, Kiritimati", 1.87, -157.43, "equator"), ("Marquesas, Nuku Hiva", -8.91, -140.10, "equator"),
    ("Galapagos, Santa Cruz", -0.74, -90.31, "equator"), ("Ecuador, Quito", -0.18, -78.47, "equator"),
    ("Brazil, Manaus", -3.12, -60.02, "equator"), ("Brazil, Macapa", 0.03, -51.07, "equator"),
    ("Brazil, Fernando de Noronha", -3.85, -32.42, "equator"), ("Ascension", -7.93, -14.41, "equator"),
    ("Ghana, Accra", 5.60, -0.19, "equator"), ("Cote d'Ivoire, Abidjan", 5.32, -4.03, "equator"),
    ("Liberia, Monrovia", 6.30, -10.80, "equator"), ("Guinea, Conakry", 9.64, -13.58, "equator"),
    ("Portugal, Sagres", 37.01, -8.94, "north"), ("Portugal, Sintra", 38.78, -9.39, "north"),
    ("Morocco, Agadir", 30.42, -9.60, "north"), ("Mauritania, Nouakchott", 18.09, -15.98, "north"),
    ("Spain, Calar Alto Observatory", 37.22, -2.55, "north"), ("Morocco, Oukaimeden Observatory", 31.21, -7.87, "north"),
    ("Iceland, Reykjavik", 64.15, -21.94, "north"), ("Egypt, Kottamia Observatory", 29.93, 31.83, "north"),
    ("Armenia, Byurakan Observatory", 40.33, 44.27, "north"), ("Russia, Zelenchukskaya (SAO)", 43.65, 41.44, "north"),
    ("Iran, Iranian National Observatory", 33.67, 51.32, "north"), ("Uzbekistan, Maidanak Observatory", 38.67, 66.90, "north"),
    ("India, Mount Abu Observatory", 24.65, 72.78, "north"), ("India, Devasthal (Nainital)", 29.36, 79.68, "north"),
    ("India, Hanle Observatory", 32.78, 78.96, "north"), ("China, Lijiang Observatory (Gaomeigu)", 26.69, 100.03, "north"),
    ("China, Delingha Observatory", 37.38, 97.73, "north"), ("China, Xinglong Observatory", 40.39, 117.58, "north"),
    ("Japan, Nayoro Observatory", 44.37, 142.48, "north"), ("Alaska, Adak", 51.88, -176.66, "north"),
    ("Alaska, Poker Flat", 65.12, -147.43, "north"), ("Canada, DRAO Penticton", 49.32, -119.62, "north"),
    ("Mexico, San Pedro Martir Observatory", 31.04, -115.46, "north"), ("USA, Kitt Peak", 31.96, -111.60, "north"),
    ("USA, Apache Point Observatory", 32.78, -105.82, "north"), ("USA, McDonald Observatory", 30.67, -104.02, "north"),
    ("Mexico, Tonantzintla", 19.03, -98.31, "north"), ("USA, Green Bank Observatory", 38.43, -79.84, "north"),
    ("Canada, Mont Megantic Observatory", 45.46, -71.15, "north"),
    ("Tristan da Cunha", -37.07, -12.31, "south"), ("Gough Island", -40.35, -9.88, "south"),
    ("Namibia, Gamsberg (H.E.S.S.)", -23.27, 16.50, "south"), ("South Africa, Sutherland (SAAO)", -32.38, 20.81, "south"),
    ("Marion Island", -46.88, 37.86, "south"), ("Amsterdam Island", -37.83, 77.57, "south"),
    ("Australia, Perth Observatory", -32.01, 116.14, "south"), ("Australia, Albany", -35.02, 117.88, "south"),
    ("Australia, Siding Spring Observatory", -31.27, 149.07, "south"), ("Australia, Hobart", -42.88, 147.33, "south"),
    ("New Zealand, Mount John Observatory", -43.99, 170.46, "south"), ("Easter Island", -27.11, -109.35, "south"),
    ("Juan Fernandez", -33.64, -78.83, "south"), ("Chile, Paranal", -24.63, -70.40, "south"),
    ("Chile, Cerro Tololo", -30.17, -70.81, "south"), ("Argentina, CASLEO", -31.80, -69.30, "south"),
    ("Chile, Puerto Montt", -41.47, -72.94, "south"), ("Falkland Islands, Stanley", -51.70, -57.85, "south"),
    ("Brazil, Pico dos Dias Observatory", -22.53, -45.58, "south"),
    ("Uruguay, Montevideo", -34.90, -56.16, "south"), ("Argentina, Bahia Blanca", -38.72, -62.27, "south"),
    ("Argentina, Comodoro Rivadavia", -45.86, -67.48, "south"), ("Chile, Punta Arenas", -53.16, -70.91, "south"),
    ("Cape Verde, Praia", 14.93, -23.51, "inner"), ("Senegal, Dakar", 14.69, -17.44, "inner"),
    ("Trinidad, Port of Spain", 10.66, -61.51, "inner"), ("Curacao, Willemstad", 12.11, -68.93, "inner"),
    ("Panama, Panama City", 8.98, -79.52, "inner"), ("Palau, Koror", 7.34, 134.48, "inner"),
    ("Micronesia, Pohnpei", 6.96, 158.21, "inner"), ("Sri Lanka, Colombo", 6.93, 79.85, "inner"),
    ("Cocos (Keeling) Islands", -12.19, 96.83, "inner"), ("Timor-Leste, Dili", -8.56, 125.57, "inner"),
    ("Solomon Islands, Honiara", -9.43, 159.95, "inner"),
    ("Hawaii, Mauna Loa Observatory", 19.54, -155.58, "inner"),
]

RINGS = {
    "equator": ("#2b6cb0", "the equatorial ring: no cyclone forms within ~5° of the equator"),
    "north":   ("#2f855a", "the high ring, north: storms arrive only as extratropical gales"),
    "south":   ("#805ad5", "the high ring, south: storms arrive only as extratropical gales"),
    "inner":   ("#dd6b20", "an inner station: the basin's edge, where a storm is still weak; built for its worst"),
}

# the seven basins: (name, outline [lon, lat], typical track [lon, lat])
BASINS = [
    ("North Atlantic and Caribbean",
     [[-98,16],[-97,26],[-90,30],[-81,32],[-76,38],[-66,44],[-50,44],[-35,36],[-18,20],
      [-18,10],[-40,9],[-60,11],[-70,13],[-82,12],[-88,15],[-98,16]],
     [[-25,13],[-45,15],[-62,19],[-74,25],[-75,32],[-68,39],[-55,44]]),
    ("North-east Pacific",
     [[-88,12],[-95,16],[-105,21],[-112,26],[-118,29],[-135,24],[-150,17],[-140,10],[-110,9],[-88,12]],
     [[-95,12],[-105,15],[-118,18],[-133,20]]),
    ("North-west Pacific",
     [[105,11],[110,22],[118,30],[128,40],[145,42],[162,32],[178,20],[178,8],[150,5],[128,6],[105,11]],
     [[162,11],[143,13],[130,18],[126,25],[131,33],[142,40]]),
    ("North Indian Ocean",
     [[52,12],[57,23],[67,25],[73,19],[77,10],[80,8],[82,16],[90,23],[95,19],[98,10],[85,6],[62,7],[52,12]],
     [[89,11],[87,16],[88,21]]),
    ("South-west Indian Ocean",
     [[35,-11],[37,-25],[45,-32],[62,-32],[82,-26],[90,-12],[75,-7],[55,-7],[40,-8],[35,-11]],
     [[82,-10],[66,-14],[55,-18],[51,-24],[57,-30]]),
    ("Australian region",
     [[90,-9],[96,-21],[110,-31],[118,-25],[126,-15],[136,-12],[142,-11],[146,-20],[154,-28],[160,-20],
      [156,-10],[142,-8],[122,-8],[105,-7],[90,-9]],
     [[124,-10],[117,-15],[113,-21],[115,-28]]),
    ("South-west Pacific",
     [[156,-9],[160,-25],[172,-35],[180,-35],[180,-6],[166,-6],[156,-9]],
     [[166,-11],[171,-17],[177,-24]]),
    ("South-west Pacific, east of the date line",
     [[-180,-35],[-155,-30],[-138,-20],[-140,-10],[-160,-7],[-180,-6],[-180,-35]],
     [[-172,-12],[-166,-18],[-158,-25]]),
]

def arrowhead(track, length=3.0, spread=25):
    (x0, y0), (x1, y1) = track[-2], track[-1]
    a = math.atan2(y1 - y0, x1 - x0)
    wing = lambda d: [x1 - length * math.cos(a + math.radians(d)),
                      y1 - length * math.sin(a + math.radians(d))]
    return [wing(spread), [x1, y1], wing(-spread)]

def basins():
    feats = []
    for name, outline, track in BASINS:
        why = "tropical cyclones form and run here; the storm network stands round it"
        feats.append({"type": "Feature",
                      "geometry": {"type": "Polygon", "coordinates": [outline]},
                      "properties": {"name": name, "why": why, "stroke": "#c53030",
                                     "stroke-width": 1, "stroke-opacity": 0.6,
                                     "fill": "#e53e3e", "fill-opacity": 0.15}})
        feats.append({"type": "Feature",
                      "geometry": {"type": "MultiLineString",
                                   "coordinates": [track, arrowhead(track)]},
                      "properties": {"name": name + " — a typical track", "why": "which way a storm runs",
                                     "stroke": "#9b2c2c", "stroke-width": 2, "stroke-opacity": 0.9}})
    return {"type": "FeatureCollection",
            "properties": {"title": "Tropical-cyclone basins",
                           "note": "outlined by hand, a typical track each — a sketch, not a climatology (SITING.md)"},
            "features": feats}


def xyz(lat, lon):
    a, b = math.radians(lat), math.radians(lon)
    return (math.cos(a) * math.cos(b), math.cos(a) * math.sin(b), math.sin(a))

def latlon(v):
    x, y, z = v
    return math.degrees(math.atan2(z, math.hypot(x, y))), math.degrees(math.atan2(y, x))

def dist(a, b):
    va, vb = xyz(*a), xyz(*b)
    d = sum(p * q for p, q in zip(va, vb))
    return R * math.acos(max(-1.0, min(1.0, d)))

def bearing(a, b):
    (p1, l1), (p2, l2) = [(math.radians(x), math.radians(y)) for x, y in (a, b)]
    y = math.sin(l2 - l1) * math.cos(p2)
    x = math.cos(p1) * math.sin(p2) - math.sin(p1) * math.cos(p2) * math.cos(l2 - l1)
    return math.degrees(math.atan2(y, x)) % 360

def gc_points(a, b, step=100.0):
    d = dist(a, b); n = max(2, int(d / step) + 1); w = d / R
    va, vb = xyz(*a), xyz(*b)
    pts = []
    for i in range(n + 1):
        t = i / n
        if w < 1e-9:
            pts.append(a); continue
        s1, s2 = math.sin((1 - t) * w) / math.sin(w), math.sin(t * w) / math.sin(w)
        pts.append(latlon(tuple(s1 * p + s2 * q for p, q in zip(va, vb))))
    return pts

def inside(lat, lon, poly):
    c = False
    for (x1, y1), (x2, y2) in zip(poly, poly[1:]):
        if (y1 > lat) != (y2 > lat) and lon < (x2 - x1) * (lat - y1) / (y2 - y1) + x1:
            c = not c
    return c

def basin_key(name):
    return "South-west Pacific" if name.startswith("South-west Pacific") else name

def basin_centres():
    acc = {}
    for name, outline, _ in BASINS:
        for lon, lat in outline:
            acc.setdefault(basin_key(name), []).append((lat, lon % 360))
    return {k: (sum(p[0] for p in v) / len(v), ((sum(p[1] for p in v) / len(v) + 180) % 360) - 180)
            for k, v in acc.items()}

def transmitters():
    tx = [(n, la, lo, HF_REACH_KM, "HF") for n, la, lo in HF]
    tx += [(n, la, lo, VLF_REACH_KM, "VLF") for n, la, lo in VLF]
    lw = json.load(open(os.path.join(OUT, "longwave-transmitters.geojson")))
    for f in lw["features"]:
        if f["geometry"]["type"] != "Point":
            continue
        lo, la = f["geometry"]["coordinates"]
        name = f["properties"].get("name", "")
        reach = LF_REACH_KM["eLoran"] if name.startswith("eLoran") else LF_REACH_KM["time"]
        tx.append((name, la, lo, reach, "LF"))
    return tx

def contributions(cand, tx, centres):
    """What one candidate adds: {(basin, 'path', bin): n} and {(basin, 'tesla', octant): 1}."""
    _, la, lo, _ = cand
    got = {}
    for _, tla, tlo, reach, _ in tx:
        if dist((la, lo), (tla, tlo)) > reach:
            continue
        pts = gc_points((la, lo), (tla, tlo))
        seen = set()
        for i in range(len(pts) - 1):
            p = pts[i]
            for name, outline, _ in BASINS:
                k = basin_key(name)
                if k in seen or not inside(p[0], p[1], outline):
                    continue
                seen.add(k)
                b = int((bearing(p, pts[i + 1]) % 180) // 45)
                got[(k, "path", b)] = got.get((k, "path", b), 0) + 1
    for k, c in centres.items():
        if dist((la, lo), c) <= TESLA_REACH_KM:
            got[(k, "tesla", int(bearing(c, (la, lo)) // 45))] = 1
        if edge_dist(la, lo, k) <= NEAR_KM:
            got[(k, "near", int(bearing(c, (la, lo)) // 45))] = 1
    for g in GRID:
        if dist((la, lo), g) <= COVER_KM:
            got[("globe", "cover", g)] = 1
    return got

_EDGES = {}
def edge_dist(la, lo, key):
    """Great-circle distance to a basin's edge, 0 inside it."""
    if key not in _EDGES:
        pts = []
        for name, outline, _ in BASINS:
            if basin_key(name) != key:
                continue
            for (x1, y1), (x2, y2) in zip(outline, outline[1:]):
                pts += gc_points((y1, x1), (y2, x2), 150)
        _EDGES[key] = pts
    for name, outline, _ in BASINS:
        if basin_key(name) == key and inside(la, lo, outline):
            return 0.0
    return min(dist((la, lo), p) for p in _EDGES[key])

def select():
    tx, centres = transmitters(), basin_centres()
    contrib = {c[0]: contributions(c, tx, centres) for c in CANDIDATES}
    need = lambda key: PATH_NEED if key[1] == "path" else 1
    have, chosen = {}, []
    while len(chosen) < MAX_STATIONS:
        best, gain = None, 0
        for c in CANDIDATES:
            if c in chosen or any(dist(c[1:3], d[1:3]) < MIN_SPACING_KM for d in chosen):
                continue
            g = sum(min(need(k), have.get(k, 0) + n) - min(need(k), have.get(k, 0))
                    for k, n in contrib[c[0]].items())
            if g > gain:
                best, gain = c, g
        if not best:
            break
        chosen.append(best)
        for k, n in contrib[best[0]].items():
            have[k] = have.get(k, 0) + n
    targets = ({(k, "path", b) for k in centres for b in range(4)}
               | {(k, kind, o) for k in centres for o in range(8) for kind in ("tesla", "near")}
               | {("globe", "cover", g) for g in GRID
                  if any(dist(c[1:3], g) <= COVER_KM for c in CANDIDATES)})
    met = sum(1 for t in targets if have.get(t, 0) >= need(t))
    return chosen, contrib, have, targets, met, tx

def main():
    chosen, contrib, have, targets, met, tx = select()
    feats = []
    for name, lat, lon, ring in chosen:
        color, rule = RINGS[ring]
        c = contrib[name]
        crossings = {}
        for (k, kind, _), n in c.items():
            if kind == "path":
                crossings[k] = crossings.get(k, 0) + n
        what = "; ".join(f"{k}: {n}" for k, n in sorted(crossings.items(), key=lambda x: -x[1]))
        feats.append({"type": "Feature",
                      "geometry": {"type": "Point", "coordinates": [lon, lat]},
                      "properties": {"name": name, "ring": ring,
                                     "why": f"{rule}. Paths across basins — {what or 'none; placed for Tesla'}",
                                     "marker-color": color, "marker-symbol": "", "marker-size": "small"}})
    fc = {"type": "FeatureCollection",
          "properties": {"title": "NIC storm network",
                         "note": f"computed from {len(tx)} transmitters and {len(CANDIDATES)} candidates: "
                                 f"{len(chosen)} stations, {met} of {len(targets)} targets met (SITING.md)"},
          "features": feats}
    with open(os.path.join(OUT, "storm-network.geojson"), "w") as f:
        json.dump(fc, f, indent=1, ensure_ascii=False)
    with open(os.path.join(OUT, "cyclone-basins.geojson"), "w") as f:
        json.dump(basins(), f, indent=1, ensure_ascii=False)
    print(fc["properties"]["note"])
    for t in sorted(targets):
        if have.get(t, 0) < (PATH_NEED if t[1] == "path" else 1):
            print("  short:", t, have.get(t, 0))
    return chosen

if __name__ == "__main__":
    main()

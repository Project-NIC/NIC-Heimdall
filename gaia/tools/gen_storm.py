#!/usr/bin/env python3
"""Generate storm-network.geojson and cyclone-basins.geojson.

The basins: the seven areas where tropical cyclones form and run, outlined by hand, each
with one typical track and an arrowhead — a sketch of where and which way, not a
climatology. The storm network is COMPUTED against what each instrument sees of a storm,
over the basins and a margin of MARGIN_KM round them, where storms are born and die —
nothing elsewhere (SITING.md, *The storm network*):

  - GNSS (Sputnik): the ionosphere is pierced at ~350 km, 1 200 km from the station at 10°
    of elevation and ~500 km at 30°; the precipitable water is the column over the station.
    Every point of a basin and its margin is wanted within GNSS_NEAR_KM of a station, and at
    worst within GNSS_FAR_KM. With all four systems tracked the sky is always full, so the
    satellites' tracks do not enter.
  - Pressure (Palatine, Pascal): a hurricane's fall reaches ~300–500 km, the surge only the
    coast. Each basin wants a coastal station on each of its eight sides, within PRESS_KM of
    its edge or inside it.
  - Marconi (HF): the ionosphere is read where the path reflects — one hop to HOP_KM, the
    reflections at the hops' midpoints; a reflection over a basin and its margin counts, per
    basin and orientation (four bins of 45°), up to PATH_NEED a bin. The broadcasters' curtain
    antennas are aimed; their azimuths (HFCC's schedules) are not in the model, so their
    paths are counted as if they radiated all round.
  - Pip (VLF, LF): the Earth–ionosphere waveguide carries the signal under the D-region the
    whole way; a path through a basin and its margin counts, the same bins.
  - Tesla: one station on each of a basin's eight sides within TESLA_REACH_KM.

A greedy pass takes the candidate that adds most, keeping MIN_SPACING_KM between stations,
until a station adds less than MIN_GAIN. Candidates are places with people, a road and power;
those inside a basin are reached by storms and built for the worst (core/POWER.md §2).
simplestyle-spec; coordinates [lon, lat], representative."""
import json, math, os

OUT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
R = 6371.0
MARGIN_KM, GNSS_NEAR_KM, GNSS_FAR_KM, PRESS_KM = 500, 500, 1200, 300
HOP_KM, PATH_NEED, TESLA_REACH_KM = 3500, 4, 5000
MIN_SPACING_KM, MIN_GAIN, MAX_STATIONS = 500, 1.0, 80
W_GNSS = 0.06          # weight of one sample point of a basin's area (a 2,5° grid)

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
CANDIDATES += [   # islands and coasts inside the basins: reached by storms, built for the worst
    ("Hawaii, Hilo", 19.72, -155.08, "inner"), ("Tahiti, Papeete", -17.53, -149.57, "inner"),
    ("Cook Islands, Rarotonga", -21.23, -159.78, "inner"), ("Samoa, Apia", -13.83, -171.76, "inner"),
    ("Tonga, Nuku'alofa", -21.14, -175.20, "inner"), ("Fiji, Nadi", -17.76, 177.44, "inner"),
    ("Vanuatu, Port Vila", -17.73, 168.32, "inner"), ("New Caledonia, Noumea", -22.27, 166.44, "inner"),
    ("Guam", 13.44, 144.79, "inner"), ("Japan, Naha", 26.21, 127.68, "inner"),
    ("Taiwan, Taipei", 25.03, 121.56, "inner"), ("Hong Kong", 22.30, 114.17, "inner"),
    ("China, Haikou", 20.04, 110.34, "inner"), ("Vietnam, Da Nang", 16.05, 108.20, "inner"),
    ("Philippines, Manila", 14.60, 120.98, "inner"), ("Japan, Tokyo", 35.68, 139.69, "inner"),
    ("China, Shanghai", 31.23, 121.47, "inner"), ("Korea, Busan", 35.18, 129.08, "inner"),
    ("Bangladesh, Chittagong", 22.34, 91.83, "inner"), ("India, Chennai", 13.08, 80.27, "inner"),
    ("India, Visakhapatnam", 17.69, 83.22, "inner"), ("India, Mumbai", 19.07, 72.88, "inner"),
    ("Pakistan, Karachi", 24.86, 67.00, "inner"), ("Oman, Muscat", 23.59, 58.41, "inner"),
    ("Mauritius, Port Louis", -20.16, 57.50, "inner"), ("Reunion, Saint-Denis", -20.88, 55.45, "inner"),
    ("Madagascar, Toamasina", -18.15, 49.40, "inner"), ("Mozambique, Beira", -19.84, 34.84, "inner"),
    ("Australia, Darwin", -12.46, 130.84, "inner"), ("Australia, Broome", -17.96, 122.24, "inner"),
    ("Australia, Port Hedland", -20.31, 118.60, "inner"), ("Australia, Townsville", -19.26, 146.80, "inner"),
    ("Australia, Brisbane", -27.47, 153.03, "inner"), ("Bermuda", 32.30, -64.78, "inner"),
    ("Bahamas, Nassau", 25.05, -77.35, "inner"), ("USA, Miami", 25.76, -80.19, "inner"),
    ("USA, Houston", 29.76, -95.37, "inner"), ("USA, Charleston", 32.78, -79.93, "inner"),
    ("USA, Cape Hatteras", 35.25, -75.53, "inner"), ("Canada, Halifax", 44.65, -63.57, "inner"),
    ("Puerto Rico, San Juan", 18.47, -66.10, "inner"), ("Barbados, Bridgetown", 13.10, -59.60, "inner"),
    ("Jamaica, Kingston", 18.00, -76.80, "inner"), ("Mexico, Cancun", 21.16, -86.85, "inner"),
    ("Mexico, Acapulco", 16.85, -99.88, "inner"), ("Mexico, Cabo San Lucas", 22.89, -109.91, "inner"),
    ("Azores, Ponta Delgada", 37.74, -25.67, "inner"),
]
INLAND = {"DR Congo, Kisangani", "Kenya, Nairobi", "Ecuador, Quito", "Brazil, Manaus",
          "Egypt, Kottamia Observatory", "Armenia, Byurakan Observatory", "Russia, Zelenchukskaya (SAO)",
          "Iran, Iranian National Observatory", "Uzbekistan, Maidanak Observatory", "India, Mount Abu Observatory",
          "India, Devasthal (Nainital)", "India, Hanle Observatory", "China, Lijiang Observatory (Gaomeigu)",
          "China, Delingha Observatory", "China, Xinglong Observatory", "Japan, Nayoro Observatory",
          "Alaska, Poker Flat", "Canada, DRAO Penticton", "Mexico, San Pedro Martir Observatory",
          "USA, Kitt Peak", "USA, Apache Point Observatory", "USA, McDonald Observatory", "Mexico, Tonantzintla",
          "USA, Green Bank Observatory", "Canada, Mont Megantic Observatory", "Spain, Calar Alto Observatory",
          "Morocco, Oukaimeden Observatory", "Namibia, Gamsberg (H.E.S.S.)", "South Africa, Sutherland (SAAO)",
          "Australia, Perth Observatory", "Australia, Siding Spring Observatory", "New Zealand, Mount John Observatory",
          "Chile, Paranal", "Chile, Cerro Tololo", "Argentina, CASLEO", "Brazil, Pico dos Dias Observatory",
          "Hawaii, Mauna Loa Observatory", "Mali, Bamako"}

RINGS = {
    "equator": ("#2b6cb0", "the equatorial ring: no cyclone forms within ~5° of the equator"),
    "north":   ("#2f855a", "the high ring, north: storms arrive only as extratropical gales"),
    "south":   ("#805ad5", "the high ring, south: storms arrive only as extratropical gales"),
    "inner":   ("#dd6b20", "inside or at the edge of a basin: reached by storms, built for its worst"),
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

def in_zone(la, lo, key, margin):
    return edge_dist(la, lo, key) <= margin

def zone_points():
    """The basins and their margin on a 2,5° grid: {basin: [(lat, lon), ...]}."""
    keys = sorted({basin_key(n) for n, _, _ in BASINS})
    pts = {k: [] for k in keys}
    la = -47.5
    while la <= 52.5:
        lo = -180.0
        while lo < 180.0:
            for k in keys:
                if in_zone(la, lo, k, MARGIN_KM):
                    pts[k].append((la, lo))
            lo += 2.5
        la += 2.5
    return pts

def path_hits(a, b, kind):
    """{(basin, bin)} the path a–b reads: HF at its reflections, VLF and LF along it."""
    d = dist(a, b)
    hits = set()
    if kind == "HF":
        n = max(1, math.ceil(d / HOP_KM))
        pts = gc_points(a, b, 50.0)
        for h in range(n):
            i = min(len(pts) - 2, int((2 * h + 1) / (2 * n) * (len(pts) - 1)))
            p, q = pts[i], pts[i + 1]
            for k in {basin_key(x) for x, _, _ in BASINS}:
                if in_zone(p[0], p[1], k, MARGIN_KM):
                    hits.add((k, int((bearing(p, q) % 180) // 45)))
    else:
        pts = gc_points(a, b, 150.0)
        for i in range(len(pts) - 1):
            p = pts[i]
            for k in {basin_key(x) for x, _, _ in BASINS}:
                if all(h[0] != k for h in hits) and in_zone(p[0], p[1], k, MARGIN_KM):
                    hits.add((k, int((bearing(p, pts[i + 1]) % 180) // 45)))
    return hits

def contributions(cand, tx, centres, zones):
    name, la, lo, ring = cand
    got = {}
    for _, tla, tlo, reach, kind in tx:
        if dist((la, lo), (tla, tlo)) > reach:
            continue
        for k, b in path_hits((la, lo), (tla, tlo), kind):
            got[(k, "path", b)] = got.get((k, "path", b), 0) + 1
    for k, c in centres.items():
        side = int(bearing(c, (la, lo)) // 45)
        if dist((la, lo), c) <= TESLA_REACH_KM:
            got[(k, "tesla", side)] = 1
        if name not in INLAND and edge_dist(la, lo, k) <= PRESS_KM:
            got[(k, "press", side)] = 1
    for k, pts in zones.items():
        for p in pts:
            d = dist((la, lo), p)
            if d <= GNSS_FAR_KM:
                got[(k, "gnss_far", p)] = 1
                if d <= GNSS_NEAR_KM:
                    got[(k, "gnss_near", p)] = 1
    return got

def need(key):
    return PATH_NEED if key[1] == "path" else 1

def weight(key):
    return W_GNSS if key[1].startswith("gnss") else 1.0

def select():
    tx, centres, zones = transmitters(), basin_centres(), zone_points()
    contrib = {c[0]: contributions(c, tx, centres, zones) for c in CANDIDATES}
    have, chosen = {}, []
    while len(chosen) < MAX_STATIONS:
        best, gain = None, MIN_GAIN
        for c in CANDIDATES:
            if c in chosen or any(dist(c[1:3], d[1:3]) < MIN_SPACING_KM for d in chosen):
                continue
            g = sum(weight(k) * (min(need(k), have.get(k, 0) + n) - min(need(k), have.get(k, 0)))
                    for k, n in contrib[c[0]].items())
            if g > gain:
                best, gain = c, g
        if not best:
            break
        chosen.append(best)
        for k, n in contrib[best[0]].items():
            have[k] = have.get(k, 0) + n
    return chosen, contrib, have, zones, centres, tx

def report(chosen, have, zones, centres):
    rows = []
    for k in sorted(zones):
        pts = zones[k]
        near = sum(1 for p in pts if have.get((k, "gnss_near", p)))
        far = sum(1 for p in pts if have.get((k, "gnss_far", p)))
        paths = sum(1 for b in range(4) if have.get((k, "path", b), 0) >= PATH_NEED)
        tesla = sum(1 for o in range(8) if have.get((k, "tesla", o)))
        press = sum(1 for o in range(8) if have.get((k, "press", o)))
        rows.append((k, round(100 * near / len(pts)), round(100 * far / len(pts)), paths, tesla, press))
    return rows

def main():
    chosen, contrib, have, zones, centres, tx = select()
    feats = []
    for name, lat, lon, ring in chosen:
        color, rule = RINGS[ring]
        c = contrib[name]
        reads = sorted({k for (k, kind, _) in c if kind in ("path", "gnss_near", "press")})
        feats.append({"type": "Feature",
                      "geometry": {"type": "Point", "coordinates": [lon, lat]},
                      "properties": {"name": name, "ring": ring,
                                     "why": f"{rule}. Reads: {', '.join(reads) or 'Tesla only'}",
                                     "marker-color": color, "marker-symbol": "", "marker-size": "small"}})
    rows = report(chosen, have, zones, centres)
    fc = {"type": "FeatureCollection",
          "properties": {"title": "NIC storm network",
                         "note": f"computed from {len(tx)} transmitters and {len(CANDIDATES)} candidates: "
                                 f"{len(chosen)} stations (SITING.md)",
                         "coverage": [dict(zip(("basin", "gnss_500_pct", "gnss_1200_pct", "path_bins_of_4",
                                                "tesla_sides_of_8", "pressure_sides_of_8"), r)) for r in rows]},
          "features": feats}
    with open(os.path.join(OUT, "storm-network.geojson"), "w") as f:
        json.dump(fc, f, indent=1, ensure_ascii=False)
    with open(os.path.join(OUT, "cyclone-basins.geojson"), "w") as f:
        json.dump(basins(), f, indent=1, ensure_ascii=False)
    print(fc["properties"]["note"])
    print(f"{'basin':42} GNSS≤500 GNSS≤1200 paths tesla press")
    for r in rows:
        print(f"{r[0]:42} {r[1]:6}% {r[2]:7}% {r[3]:4}/4 {r[4]:4}/8 {r[5]:4}/8")

if __name__ == "__main__":
    main()

#!/usr/bin/env python3
"""Generate storm-network.geojson and cyclone-basins.geojson: the storm network, three rings of stations around
the seven tropical-cyclone basins (SITING.md, *The storm network*) — the equatorial
islands, the high ring on both hemispheres, and the inner stations where a storm is
still weak. A first draw by hand, placed for the geometry, people or none. The basins:
the seven areas where tropical cyclones form and run, outlined by hand, each with
one typical track and an arrowhead — a sketch of where and which way, not a climatology.
simplestyle-spec; coordinates [lon, lat], representative."""
import json, math, os

OUT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))

RINGS = {
    "equator": ("#2b6cb0", "the equatorial ring: no cyclone forms within ~5° of the equator"),
    "north":   ("#2f855a", "the high ring, north: storms arrive only as extratropical gales"),
    "south":   ("#805ad5", "the high ring, south: storms arrive only as extratropical gales"),
    "inner":   ("#dd6b20", "an inner station: the basin's edge, where a storm is still weak; built for its worst"),
}

# (ring, name, lat, lon, why)
SITES = [
    ("equator", "Sao Tome", 0.34, 6.73, "the Gulf of Guinea"),
    ("equator", "Kenya, Nairobi", -1.29, 36.82, "East Africa, 1 700 m"),
    ("equator", "Seychelles, Mahe", -4.62, 55.45, "the western Indian Ocean, on a cable"),
    ("equator", "Maldives, Addu", -0.63, 73.16, "the central Indian Ocean, on a cable"),
    ("equator", "Singapore", 1.29, 103.85, "the cable hub between the Indian Ocean and the Pacific"),
    ("equator", "Indonesia, Ternate", 0.79, 127.38, "the Moluccas, south of the typhoons' genesis"),
    ("equator", "Papua New Guinea, Manus", -2.03, 147.27, "the western Pacific"),
    ("equator", "Kiribati, Tarawa", 1.42, 173.02, "the western equatorial Pacific"),
    ("equator", "Kiribati, Kiritimati", 1.87, -157.43, "the central equatorial Pacific"),
    ("equator", "Marquesas, Nuku Hiva", -8.91, -140.10, "the eastern Pacific's gap; cyclones very rare"),
    ("equator", "Galapagos, Santa Cruz", -0.74, -90.31, "the eastern equatorial Pacific"),
    ("equator", "Brazil, Macapa", 0.03, -51.07, "the Amazon's mouth"),
    ("equator", "Brazil, Fernando de Noronha", -3.85, -32.42, "the equatorial Atlantic, west"),
    ("equator", "Ascension", -7.93, -14.41, "the equatorial Atlantic, east; the South Atlantic has almost no cyclones"),
    ("north", "Iceland, Reykjavik", 64.15, -21.94, "north of the Atlantic basin"),
    ("north", "Ukraine, Kyiv", 50.45, 30.52, "eastern Europe"),
    ("north", "Russia, Yekaterinburg", 56.84, 60.61, "north of the North Indian basin"),
    ("north", "Russia, Irkutsk", 52.29, 104.30, "inner Asia"),
    ("north", "Russia, Petropavlovsk-Kamchatsky", 53.02, 158.65, "north of the north-west Pacific basin"),
    ("north", "Alaska, Adak", 51.88, -176.66, "the Aleutians"),
    ("north", "Alaska, Anchorage", 61.22, -149.90, "north of the north-east Pacific basin"),
    ("north", "Canada, Saskatoon", 52.13, -106.67, "the prairies"),
    ("north", "Canada, Goose Bay", 53.30, -60.33, "Labrador, north of the Atlantic's recurving storms"),
    ("south", "Tristan da Cunha", -37.07, -12.31, "the middle of the South Atlantic"),
    ("south", "South Africa, Cape Town", -33.92, 18.42, "south-west of the south-west Indian basin"),
    ("south", "Marion Island", -46.88, 37.86, "south of the south-west Indian basin; the research base"),
    ("south", "Amsterdam Island", -37.83, 77.57, "the southern Indian Ocean; the research base"),
    ("south", "Australia, Albany", -35.02, 117.88, "south of the north-west Australian basin"),
    ("south", "Australia, Hobart", -42.88, 147.33, "south of the Coral Sea"),
    ("south", "New Zealand, Christchurch", -43.53, 172.64, "south of the south-west Pacific basin"),
    ("south", "Easter Island", -27.11, -109.35, "the south-east Pacific, no cyclones"),
    ("south", "Chile, Puerto Montt", -41.47, -72.94, "the south-east Pacific's coast"),
    ("south", "Falkland Islands, Stanley", -51.70, -57.85, "the south-west Atlantic"),
    ("inner", "Cape Verde, Praia", 14.93, -23.51, "where the Atlantic's storms are born, still weak"),
    ("inner", "Trinidad, Port of Spain", 10.66, -61.51, "the Caribbean's southern edge"),
    ("inner", "Curacao, Willemstad", 12.11, -68.93, "south of the Caribbean's storm track"),
    ("inner", "Panama, Panama City", 8.98, -79.52, "between the Caribbean and the north-east Pacific basin"),
    ("inner", "Palau, Koror", 7.34, 134.48, "the north-west Pacific's genesis region"),
    ("inner", "Micronesia, Pohnpei", 6.96, 158.21, "the north-west Pacific's genesis region, east"),
    ("inner", "Sri Lanka, Colombo", 6.93, 79.85, "the North Indian basin's southern edge"),
    ("inner", "Cocos (Keeling) Islands", -12.19, 96.83, "the south-east Indian Ocean's genesis region"),
    ("inner", "Timor-Leste, Dili", -8.56, 125.57, "the Australian basin's northern edge"),
    ("inner", "Solomon Islands, Honiara", -9.43, 159.95, "the south-west Pacific's genesis region"),
]

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

def main():
    feats = []
    for ring, n, lat, lon, w in SITES:
        color, rule = RINGS[ring]
        feats.append({"type": "Feature",
                      "geometry": {"type": "Point", "coordinates": [lon, lat]},
                      "properties": {"name": n, "why": f"{w} — {rule}", "ring": ring,
                                     "marker-color": color, "marker-symbol": "",
                                     "marker-size": "small"}})
    fc = {"type": "FeatureCollection",
          "properties": {"title": "NIC storm network",
                         "note": "three rings around the cyclone basins, a first draw by hand (SITING.md)"},
          "features": feats}
    with open(os.path.join(OUT, "storm-network.geojson"), "w") as f:
        json.dump(fc, f, indent=1, ensure_ascii=False)
    with open(os.path.join(OUT, "cyclone-basins.geojson"), "w") as f:
        json.dump(basins(), f, indent=1, ensure_ascii=False)
    print(len(feats), "sites,", len(BASINS), "basin outlines")

if __name__ == "__main__":
    main()

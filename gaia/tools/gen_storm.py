#!/usr/bin/env python3
"""Generate storm-network.geojson: the storm network, a few dozen stations off the
tropical-cyclone belts that watch lightning, the ionosphere and the water from afar
(SITING.md, *The storm network*). A first draw by hand, placed for the geometry,
people or none, ~2 900 km apart. simplestyle-spec; coordinates [lon, lat], representative."""
import json, os

OUT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))

# (name, lat, lon, why)
SITES = [
    ("Czechia, central Bohemia", 50.1, 14.4, "the reference station"),
    ("Spain, Almeria", 37.0, -2.5, "the sunniest corner of Europe; the western Mediterranean"),
    ("Iceland, Reykjavik", 64.1, -21.9, "the North Atlantic paths"),
    ("Greenland, Narsarsuaq", 61.2, -45.4, "the Labrador Sea; paths to North America"),
    ("Morocco, Ouarzazate", 30.9, -6.9, "the Sahara's edge, sun all year"),
    ("Algeria, Tamanrasset", 22.8, 5.5, "the central Sahara, 1 400 m"),
    ("Mali, Bamako", 12.6, -8.0, "the Sahel, inland of the Atlantic's genesis region"),
    ("Chad, N'Djamena", 12.1, 15.0, "the middle of the continent"),
    ("Egypt, Aswan", 24.1, 32.9, "the Nile, among the sunniest places on Earth"),
    ("Ethiopia, Addis Ababa", 9.0, 38.7, "the highlands, 2 350 m; the Horn"),
    ("Saudi Arabia, Riyadh", 24.7, 46.7, "inland of the Arabian Sea's cyclones"),
    ("Gabon, Libreville", 0.4, 9.4, "the equator on the Gulf of Guinea"),
    ("Kenya, Nairobi", -1.3, 36.8, "the equator, 1 700 m"),
    ("Zambia, Lusaka", -15.4, 28.3, "inland, west of the Mozambique Channel's cyclones"),
    ("Namibia, Windhoek", -22.6, 17.1, "the desert plateau, sun all year"),
    ("South Africa, Sutherland", -32.4, 20.8, "the Karoo, beside the observatory"),
    ("St Helena", -15.9, -5.7, "mid-ocean, on the Equiano cable"),
    ("Tristan da Cunha", -37.1, -12.3, "the middle of the South Atlantic"),
    ("Turkey, Ankara", 39.9, 32.9, "Anatolia; the eastern Mediterranean"),
    ("Kazakhstan, Almaty", 43.2, 76.9, "Central Asia"),
    ("Mongolia, Ulaanbaatar", 47.9, 106.9, "north-east Asia"),
    ("Nepal, Kathmandu", 27.7, 85.3, "the Himalaya's south side, inland of the Bay of Bengal"),
    ("India, Bengaluru", 13.0, 77.6, "the Deccan, 900 m, inland of both coasts' cyclones"),
    ("Maldives, Addu", -0.6, 73.1, "the equator, on a cable"),
    ("Seychelles, Mahe", -4.6, 55.5, "north of the south-west Indian belt, on a cable"),
    ("Singapore", 1.3, 103.8, "the equator, the cable hub of the region"),
    ("Indonesia, Manado", 1.5, 124.8, "south of the typhoons' reach"),
    ("Japan, Sapporo", 43.1, 141.4, "the north-west Pacific's northern side; built for typhoon remnants"),
    ("Amsterdam Island", -37.8, 77.6, "the southern Indian Ocean; the research base"),
    ("Australia, Alice Springs", -23.7, 133.9, "the continent's middle, sun all year"),
    ("Australia, Perth", -31.9, 115.9, "south of the north-west coast's cyclones"),
    ("Australia, Canberra", -35.3, 149.1, "the south-east, inland"),
    ("Papua New Guinea, Manus", -2.0, 147.3, "the equator, north of the Coral Sea"),
    ("New Zealand, Christchurch", -43.5, 172.6, "the South Island, south of the ex-tropical storms' usual track"),
    ("Kiribati, Tarawa", 1.4, 173.0, "the western equatorial Pacific"),
    ("Kiribati, Kiritimati", 1.9, -157.4, "the central equatorial Pacific"),
    ("Alaska, Adak", 51.9, -176.6, "the North Pacific, the Aleutians"),
    ("Pitcairn", -25.1, -130.1, "the south-east Pacific"),
    ("Easter Island", -27.1, -109.4, "the south-east Pacific, no cyclones"),
    ("Galapagos, Santa Cruz", -0.7, -90.3, "the equator off South America"),
    ("Alaska, Anchorage", 61.2, -149.9, "the Gulf of Alaska"),
    ("Canada, Victoria", 48.4, -123.4, "the Pacific north-west"),
    ("Canada, Saskatoon", 52.1, -106.6, "the prairies"),
    ("USA, Boulder", 40.0, -105.3, "the Rockies, beside WWV"),
    ("Canada, Ottawa", 45.4, -75.7, "the east, inland of the Atlantic coast; built for hurricane remnants"),
    ("Mexico, Mexico City", 19.4, -99.1, "the plateau, 2 240 m, between both coasts' hurricanes; built for their remnants"),
    ("Colombia, Bogota", 4.7, -74.1, "the Andes, south of the Caribbean"),
    ("Brazil, Manaus", -3.1, -60.0, "the Amazon, the equator"),
    ("Brazil, Fernando de Noronha", -3.9, -32.4, "the equatorial Atlantic"),
    ("Brazil, Brasilia", -15.8, -47.9, "the plateau; the South Atlantic has almost no cyclones"),
    ("Chile, San Pedro de Atacama", -22.9, -68.2, "the driest and sunniest place on Earth"),
    ("Argentina, Cordoba", -31.4, -64.2, "the pampas"),
    ("Falkland Islands, Stanley", -51.7, -57.9, "the south-west Atlantic"),
    ("South Georgia, King Edward Point", -54.3, -36.5, "the Southern Ocean's edge; the research base"),
]

def main():
    feats = [{"type": "Feature",
              "geometry": {"type": "Point", "coordinates": [lon, lat]},
              "properties": {"name": n, "why": w, "marker-color": "#2b6cb0",
                             "marker-symbol": "", "marker-size": "small"}}
             for n, lat, lon, w in SITES]
    fc = {"type": "FeatureCollection",
          "properties": {"title": "NIC storm network",
                         "note": "a first draw by hand: off the cyclone belts, ~2 900 km apart (SITING.md)"},
          "features": feats}
    with open(os.path.join(OUT, "storm-network.geojson"), "w") as f:
        json.dump(fc, f, indent=1, ensure_ascii=False)
    print(len(feats), "sites")

if __name__ == "__main__":
    main()

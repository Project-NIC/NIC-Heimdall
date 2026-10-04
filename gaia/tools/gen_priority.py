#!/usr/bin/env python3
"""Generate NIC priority-zone GeoJSON layers, one file per phenomenon.
simplestyle-spec so GitHub renders them natively. Coordinates are [lon, lat],
approximate / representative (siting guide, not survey data)."""
import json, os

import os as _os
OUT = _os.path.dirname(_os.path.dirname(_os.path.abspath(__file__)))

def fc(title, note, sources, features):
    return {"type": "FeatureCollection",
            "properties": {"title": title, "note": note, "sources": sources},
            "features": features}

def line(coords, name, why, color, width=3, opacity=0.9):
    return {"type": "Feature",
            "geometry": {"type": "LineString", "coordinates": coords},
            "properties": {"name": name, "why": why,
                           "stroke": color, "stroke-width": width,
                           "stroke-opacity": opacity}}

def pt(lon, lat, name, why, color, symbol, size="small"):
    return {"type": "Feature",
            "geometry": {"type": "Point", "coordinates": [lon, lat]},
            "properties": {"name": name, "why": why,
                           "marker-color": color, "marker-symbol": symbol,
                           "marker-size": size}}

def write(fname, obj):
    p = os.path.join(OUT, fname)
    with open(p, "w") as f:
        json.dump(obj, f, indent=1, ensure_ascii=False)
    print(f"{fname:34s} {len(obj['features']):3d} features")

# ─────────────────────────────────────────────────────────────────────────
# 1. TSUNAMI — subduction trenches (lines) + recommended island array sites
# ─────────────────────────────────────────────────────────────────────────
RED = "#c53030"
tren = [
 (line([[162,56],[160,54],[157,51],[154,48],[151,46],[148,44],[145,41],[143,39],[142,37],[141,35],[141,33]],
   "Japan–Kuril–Kamchatka Trench","M9-class megathrust (Tohoku 2011); dense island arc alongside","#c53030")),
 (line([[141,33],[138,33],[136,33],[134,32],[132,31],[130,29],[128,27],[126,25],[124,23],[122,22]],
   "Nankai–Ryukyu Trench","Anticipated Nankai megathrust; Ryukyu island chain as ready aperture","#c53030")),
 (line([[-146,58],[-150,57.5],[-155,56],[-160,55],[-165,54.5],[-170,53.5],[-175,52.5],[-179,52]],
   "Aleutian Trench","1946/1957/1964 tsunamigenic; the Aleutians ARE the array","#c53030")),
 (line([[-128,50.5],[-127.5,48.5],[-126.5,46.5],[-125.5,44],[-125,42],[-124.5,40.5]],
   "Cascadia Subduction Zone","M9 overdue (1700); Vancouver Is. + offshore islets","#c53030")),
 (line([[-105,20],[-103,18],[-100,17],[-96,16],[-93,14],[-90,13],[-88,12],[-86,11]],
   "Middle America Trench","Frequent tsunamigenic thrust off Mexico/Central America","#c53030")),
 (line([[-77,-5],[-78,-9],[-77,-13],[-75,-17],[-73,-21],[-72,-25],[-71,-29],[-72,-33],[-73,-37],[-74,-41],[-75,-44],[-76,-47]],
   "Peru–Chile Trench","Largest quakes on Earth (1960 M9.5); long straight coast","#c53030")),
 (line([[92,8],[93,5],[95,2],[97,-1],[100,-4],[103,-6],[106,-8],[110,-9],[113,-10],[116,-10.5],[119,-11]],
   "Sunda Trench (Sumatra–Java)","2004 M9.1 Indian Ocean tsunami; Mentawai/Nias arc","#c53030")),
 (line([[120,19],[120,16],[121,14],[123,12],[125,10],[126,8],[127,6]],
   "Philippine / Manila Trench","Dense archipelago over an active thrust","#c53030")),
 (line([[187,-15],[186,-18],[185,-21],[184,-24],[183,-27],[182,-29],[180,-31],[179,-33],[178,-35],[177,-37]],
   "Tonga–Kermadec Trench","Fastest convergence on Earth; Tonga/Kermadec islets","#c53030")),
 (line([[166,-12],[167,-14],[168,-16],[169,-18],[170,-20],[171,-22],[172,-23]],
   "New Hebrides Trench (Vanuatu)","Very high seismicity; island chain aperture","#c53030")),
 (line([[-59,10],[-59,12],[-59,14],[-60,16],[-61,17],[-61,18]],
   "Lesser Antilles Trench","Caribbean thrust; string of small islands","#c53030")),
 (line([[179,-37],[178.5,-39],[178,-40],[177,-41.5],[176,-42.5]],
   "Hikurangi Trench (New Zealand)","Slow-slip + megathrust; North Island coast","#c53030")),
]
tsites = [
 pt(-176.6,51.9,"Aleutian Islands (Adak-class)","Slope toe within ~km; the arc itself is the aperture",RED,"star"),
 pt(151,46,"Kuril Islands","Steep deep flanks close in — short cable to full-transport water",RED,"star"),
 pt(141.5,38,"Japan Pacific islet","Co-sited with the DART/GSN backbone; magnetometer + borehole seismo",RED,"star"),
 pt(128,26,"Ryukyu (Okinawa arc)","Nankai/Ryukyu front; island chain spans the strike",RED,"star"),
 pt(125,14,"Luzon east coast","Manila Trench; deep water near shore",RED,"star"),
 pt(98,-1,"Mentawai / Nias (Sumatra)","2004 source region; outer-arc islands over the toe",RED,"star"),
 pt(110,-8,"Java south coast islet","Sunda Trench; steep shelf drop",RED,"star"),
 pt(168,-17,"Vanuatu","New Hebrides arc; dense small islands, deep between them",RED,"star"),
 pt(185,-21,"Tonga","Kermadec convergence; atoll line resolves speed",RED,"star"),
 pt(178,-38.5,"Gisborne / Hikurangi (NZ)","Slow-slip laboratory; land + shelf sondes",RED,"star"),
 pt(-73.5,-37,"South-central Chile islet","1960 rupture zone; long straight trench parallel",RED,"star"),
 pt(-88,12.5,"Central America Pacific","Middle America Trench; short deep cable",RED,"star"),
 pt(-126,49,"Vancouver Island","Cascadia; islets over the slope toe",RED,"star"),
 pt(-61,14.7,"Martinique (Lesser Antilles)","Caribbean arc; island-network aperture",RED,"star"),
]
write("priority-tsunami.geojson", fc(
 "NIC priority — tsunami (subduction zones + island array sites)",
 "Red lines = major subduction trenches (tsunami sources). Red stars = recommended island-arc array sites: "
 "the magnetometer sits at the toe of the island slope (deep water within ~km), the borehole seismo drops in a "
 "pipe, the radar watches the surface — short cables, no 50 km haul to the trench. Representative, not exhaustive.",
 "USGS/Global CMT · NOAA NCEI · gauss/README.md (The array) · daedalus/README.md (offshore node)",
 tren + tsites))

# ─────────────────────────────────────────────────────────────────────────
# 2. EARTHQUAKE — the populated CONTINENTAL belts (complement to tsunami)
# ─────────────────────────────────────────────────────────────────────────
ORA = "#dd6b20"
ebelts = [
 line([[-9,36],[-4,36],[2,37],[7,39],[12,42],[16,42],[20,41],[24,40],[28,39],[33,37],[37,37],[42,38],[47,38],[52,36],[57,35],[62,34],[67,34],[71,34],[75,33],[79,32],[83,30],[87,28],[91,27],[95,26]],
   "Alpide belt (Mediterranean → Himalaya)","2nd-most-seismic belt; dense population, weak coverage — cheap grid pays off","#dd6b20"),
 line([[41,40.7],[38,40.8],[35,40.7],[32,40.6],[29,40.6],[27,40.5]],
   "North Anatolian Fault","Istanbul gap; classic dense-network early-warning case","#dd6b20"),
 line([[-124,40],[-122.5,38],[-121,36.5],[-119.5,35],[-118,34],[-116.5,33.5]],
   "San Andreas (California)","ShakeAlert territory; the density argument proven","#dd6b20"),
 line([[-24,66],[-28,60],[-30,52],[-33,44],[-35,36],[-32,26],[-25,14],[-18,4],[-13,-2],[-12,-12],[-13,-22],[-14,-32],[-15,-42],[-16,-50]],
   "Mid-Atlantic Ridge","Oceanic spreading seismicity (context; sparse land)","#dd6b20"),
 line([[36,15],[38,11],[39,7],[37,3],[36,-1],[35,-5],[34,-9],[33,-13],[32,-16]],
   "East African Rift","Continental rifting; almost no dense monitoring","#dd6b20"),
 line([[142,44],[141,41],[140,38],[138,36],[136,35],[134,34],[132,34]],
   "Japan inland belt","Densest existing net — the benchmark to match cheaply","#dd6b20"),
]
espots = [
 pt(28.97,41.01,"Istanbul","North Anatolian gap; millions exposed","#dd6b20","circle"),
 pt(85.32,27.71,"Kathmandu","2015 M7.8; Himalayan front, thin coverage","#dd6b20","circle"),
 pt(51.39,35.69,"Tehran","Multiple active faults under a megacity","#dd6b20","circle"),
 pt(-122.42,37.77,"San Francisco Bay","Dense-array early-warning reference","#dd6b20","circle"),
 pt(13.4,42.35,"Central Italy (L'Aquila/Amatrice)","Repeated deadly moderate quakes","#dd6b20","circle"),
 pt(-70.65,-33.45,"Santiago","Andean thrust behind a capital","#dd6b20","circle"),
 pt(69.2,34.5,"Kabul / Hindu Kush","Deep intermediate seismicity, no net","#dd6b20","circle"),
 pt(174.78,-41.29,"Wellington","On-fault capital; NZ early warning","#dd6b20","circle"),
]
write("priority-earthquake.geojson", fc(
 "NIC priority — earthquake (continental seismic belts, populated & under-covered)",
 "Orange lines = major seismic belts; orange circles = high-exposure, under-instrumented cities. This layer is the "
 "INLAND complement to the tsunami layer: dense cheap seismographs where people live and national networks are thin — "
 "the earthquake-early-warning density argument (Japan/California) taken to places that cannot afford it.",
 "USGS · Global CMT · GEM seismic hazard · quake/README.md",
 ebelts + espots))

# ─────────────────────────────────────────────────────────────────────────
# 3. IONOSPHERE / TEC — where GNSS-TEC (Sputnik) pays off most
# ─────────────────────────────────────────────────────────────────────────
PUR = "#805ad5"
def band(lat, name, why, jitter=0):
    return line([[lon, lat + jitter*__import__('math').sin(lon/30.0)] for lon in range(-180,181,20)],
                name, why, PUR, width=6, opacity=0.5)
tec = [
 band(12,"Equatorial Ionization Anomaly — north crest","EIA crest: strongest TEC + scintillation; GNSS-TEC science hotspot"),
 band(-12,"Equatorial Ionization Anomaly — south crest","EIA crest: strongest TEC + scintillation; GNSS-TEC science hotspot"),
 band(0,"Magnetic dip equator (approx)","Equatorial electrojet / plasma bubbles seed here"),
 band(67,"Auroral oval — north (approx)","Substorm currents, GIC driver; dB/dt + TEC together"),
 band(-67,"Auroral oval — south (approx)","Southern auroral zone counterpart"),
]
tspots = [
 pt(-60,-15,"South America (Andes–Amazon)","EIA crest + the magnetic anomaly; premier scintillation zone",PUR,"circle"),
 pt(105,10,"SE Asia","EIA crest, dense population, few open TEC nodes",PUR,"circle"),
 pt(20,5,"West/Central Africa","EIA crest, almost no coverage",PUR,"circle"),
 pt(25,69,"Fennoscandia (auroral)","EISCAT/IMAGE latitude; substorm + GIC",PUR,"circle"),
 pt(-147,65,"Alaska (auroral)","Auroral TEC + GIC on the grid",PUR,"circle"),
]
write("priority-tec.geojson", fc(
 "NIC priority — ionosphere / TEC (Sputnik GNSS)",
 "Purple bands = where GNSS-TEC (NIC-Sputnik) earns most: the two Equatorial Ionization Anomaly crests (~±10–15° of the "
 "magnetic dip equator — strongest TEC, scintillation, plasma bubbles) and the auroral ovals (~67° magnetic — substorm "
 "currents, geomagnetically-induced currents, dB/dt). Bands are approximate; the dip equator and ovals move with activity.",
 "IGS · Sputnik/README.md · space-weather literature (EIA, auroral electrojet)",
 tec + tspots))

# ─────────────────────────────────────────────────────────────────────────
# 4. RADIATION — cosmic-ray neutron monitoring (high latitude + altitude)
# ─────────────────────────────────────────────────────────────────────────
GRN = "#2f855a"
rspots = [
 pt(25.47,65.05,"Oulu (Finland)","Low cutoff rigidity (~0.8 GV): sensitive to cosmic-ray flux",GRN,"circle"),
 pt(7.98,46.55,"Jungfraujoch (Alps, 3475 m)","High altitude → high count rate; the classic monitor",GRN,"circle"),
 pt(-68.13,-16.35,"Chacaltaya (Andes, 5240 m)","Very high altitude, low latitude — extreme neutron flux",GRN,"circle"),
 pt(166.67,-77.85,"McMurdo / polar plateau","Polar: near-zero cutoff, full galactic-ray sensitivity",GRN,"circle"),
 pt(15.5,78.2,"Svalbard","High-Arctic, low cutoff; space-weather forecasting",GRN,"circle"),
 pt(90.1,30.0,"Tibetan Plateau (~4500 m)","Altitude + mid-latitude; huge open gap",GRN,"circle"),
]
rband = [
 line([[lon,66] for lon in range(-180,181,20)],
   "High-latitude cosmic-ray zone (north)","Low geomagnetic cutoff → strongest galactic-ray + solar-event signal",GRN,4,0.4),
 line([[lon,-66] for lon in range(-180,181,20)],
   "High-latitude cosmic-ray zone (south)","Southern counterpart",GRN,4,0.4),
]
write("priority-radiation.geojson", fc(
 "NIC priority — radiation / cosmic-ray neutron monitoring",
 "Green circles = best sites for the neutron heads (Helion/Gadolin) as a cosmic-ray / space-weather monitor: LOW "
 "geomagnetic cutoff (high latitude) and/or HIGH altitude, where galactic-ray flux and solar-event ground enhancements "
 "are strongest. Green bands mark the high-latitude zones. (Honest note: this is the weakest 'priority geography' — the "
 "radiation heads are mainly local/relative alarms; the space-weather case is the one place a global siting map applies.)",
 "NMDB neutron monitor network · Sputnik/space-weather context · quark/README.md",
 rband + rspots))

print("done")

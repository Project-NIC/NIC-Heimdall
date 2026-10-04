#!/usr/bin/env python3
"""Generates ../proposed-network.geojson — data-driven placement, kept deliberately simple.

Three rules (owner, 2026-07-16):
  1. don't pile on where networks already exist  -> covered countries: only >1M cities
  2. don't put stations where they make no sense -> every station snaps to a real town
     (someone has to build and maintain it); no lattice points in the wilderness
  3. don't solve everything -> one algorithm: cities x hazard proximity; backbone fills gaps

The directional rule (owner, 2026-07-18): the 50-year tsunami source map is
closed (the known trenches), so a coastal array is a 4-5-sonde fan on the
source-facing sector only — never a ring; the lee shore is wave shadow.
Cabled seas (S-net/DONET, NEPTUNE, GeoNet) keep only interop hubs.

City data: GeoNames via the `all-the-cities` npm package. Regenerate the input with
    node -e "const c=require('all-the-cities');require('fs').writeFileSync('cities20k.json',
      JSON.stringify(c.filter(x=>x.population>=20000).map(x=>({n:x.name,c:x.country,
      p:x.population,lon:x.loc.coordinates[0],lat:x.loc.coordinates[1]}))))"
and drop cities20k.json next to this script.
"""
import json, math, os
from collections import Counter

HERE = os.path.dirname(os.path.abspath(__file__))
GAIA = os.path.dirname(HERE)

cities = json.load(open(os.path.join(HERE, "cities20k.json")))

# countries with dense existing seismic/EEW networks -> only major hubs there
COVERED = {"JP","TW","NZ","IT","CH","US","CL","MX","GR","IS"}

# hazard line vertex clouds (from the committed priority layers)
# For the tsunami arcs each vertex also keeps its trench name and the local
# line tangent azimuth — the directional rule below needs both.
def verts(fname, tagged=False):
    d=json.load(open(f"{GAIA}/{fname}")); out=[]
    for ft in d["features"]:
        if ft["geometry"]["type"]=="LineString":
            c=ft["geometry"]["coordinates"]; name=ft["properties"].get("name","")
            for (x0,y0),(x1,y1) in zip(c[:-1],c[1:]):
                n=max(1,int(math.hypot(x1-x0,y1-y0)))
                for k in range(n+1):
                    t=k/max(n,1)
                    x=((x0+(x1-x0)*t)+180)%360-180; y=y0+(y1-y0)*t
                    if tagged:
                        az=math.degrees(math.atan2((x1-x0)*math.cos(math.radians(y)),y1-y0))%360
                        out.append((x,y,name,az))
                    else:
                        out.append((x,y))
    return out
ARC_T = verts("priority-tsunami.geojson", tagged=True)
ARC  = [(x,y) for x,y,_,_ in ARC_T]
BELT = verts("priority-earthquake.geojson")
HAZ  = ARC + BELT

def dmin(lon,lat,cloud):
    return min(math.hypot(lon-x,lat-y) for x,y in cloud)

# ---- 1. seismic stations = towns near hazard lines ------------------------
# within ~4 deg of an arc/belt; dedupe conurbations by 0.4-deg cell (keep biggest)
cand={}
for c in sorted(cities,key=lambda c:-c["p"]):
    if c["c"] in COVERED and c["p"]<1_000_000: continue
    if dmin(c["lon"],c["lat"],HAZ)>4.0: continue
    key=(round(c["lon"]/0.4),round(c["lat"]/0.4))
    if key not in cand: cand[key]=c
seis=list(cand.values())

# ---- 2. coastal / tsunami array sites along the arcs ----------------------
# The directional rule (owner, 2026-07-18): the 50-year tsunami source map is
# CLOSED — the sources are the known subduction arcs (the historical record);
# no new source geography will appear on that horizon. Consequences:
#   a) every site's threat bearing is computable NOW from trench geometry:
#      the wave arrives from the trench side, so instrument ONLY the
#      source-facing sector — an island's lee shore sees diffraction (a swirl
#      behind the island, not a front) and correctly gets nothing. A 5-sonde
#      sector fan replaces the 10-sonde ring (gauss/README.md budget:
#      4 = common-mode reference + bearing, +1 margin);
#   b) where a DART buoy already watches the deep-water leg upstream
#      (<= ~6 deg), 4 sondes suffice — far-field detection exists, the fan
#      confirms and bearings the wave locally;
#   c) arcs already under cabled national tsunami nets keep only every 3rd
#      site as an interop hub (siting rule 1: don't pile on).
CABLED = {"Nankai–Ryukyu Trench":        "S-net/DONET",
          "Cascadia Subduction Zone":    "ONC NEPTUNE",
          "Hikurangi Trench (New Zealand)": "GeoNet"}
def cabled_net(trench, lat):
    if trench in CABLED: return CABLED[trench]
    # Japan segment of the Japan–Kuril–Kamchatka line is S-net water;
    # the Kuril/Kamchatka stretch further north runs bare.
    if trench.startswith("Japan–Kuril") and lat < 44.5: return "S-net"
    return None

darts=[tuple(f["geometry"]["coordinates"]) for f in
       json.load(open(f"{GAIA}/existing-systems.geojson"))["features"]
       if f["properties"].get("network","").startswith("NOAA DART")]
def dart_near(lon,lat,r=6.0):
    for dx,dy in darts:
        dl=abs(lon-dx)%360; dl=min(dl,360-dl)
        if math.hypot(dl,lat-dy)<=r: return True
    return False

DIRS=["N","NNE","NE","ENE","E","ESE","SE","SSE","S","SSW","SW","WSW","W","WNW","NW","NNW"]
def compass(b): return DIRS[int((b+11.25)//22.5)%16]
def angsep(a,b): d=abs(a-b)%360; return min(d,360-d)

# Which side of each trench the open ocean (= the incoming wave) is on — a
# geographic fact, hand-tagged once per trench as a rough compass bearing.
# The local line tangent then gives the exact normal; the tag only picks
# which of the two normals is seaward. (A nearest-town heuristic fails on
# island arcs — towns sit on every side of an open-ocean arc.)
OCEAN = {"Japan–Kuril–Kamchatka Trench": 135, "Nankai–Ryukyu Trench": 135,
         "Aleutian Trench": 180, "Cascadia Subduction Zone": 270,
         "Middle America Trench": 225, "Peru–Chile Trench": 270,
         "Sunda Trench (Sumatra–Java)": 225, "Tonga–Kermadec Trench": 90,
         "New Hebrides Trench (Vanuatu)": 225, "Lesser Antilles Trench": 90,
         "Hikurangi Trench (New Zealand)": 135}
def ocean_bearing(trench, lat):
    if trench.startswith("Philippine"):     # one line, two ocean sides:
        return 270 if lat >= 13 else 90     # Manila Trench (W) / Philippine Trench (E)
    return OCEAN[trench]

def threat_bearing(trench,y,az):
    """Of the two trench normals, the one on the tagged ocean side."""
    ob=ocean_bearing(trench,y)
    n1,n2=(az+90)%360,(az-90)%360
    return n1 if angsep(n1,ob)<=angsep(n2,ob) else n2

# Two honest exceptions (owner, 2026-07-18):
#   - MULTI-THREAT: an island within reach of a SECOND arc whose threat
#     bearing differs by > 90 deg can be hit from both sides — no debate,
#     it keeps the ring (8 sondes: two fans back-to-back sharing the
#     common-mode core).
#   - RADAR ONLY WHERE THE MAST SURVIVES: a surface sea-gauge on the bare
#     exposed shore gets drowned or de-masted by the very wave it waits
#     for. The radar goes only where a real town (~<= 100 km) can host and
#     keep the mast (a harbour structure, someone to fix it); everywhere
#     else the slope-toe magnetometer IS the gauge — it reads transport
#     (velocity x depth) from the seafloor, potted, below the violence.
by_trench={}
for x,y,name,az in ARC_T: by_trench.setdefault(name,[]).append((x,y,az))
def second_threat(x,y,trench,tb,r=8.0):
    for name,vs in by_trench.items():
        if name==trench: continue
        vx,vy,vaz=min(vs,key=lambda v:math.hypot(x-v[0],y-v[1]))
        if math.hypot(x-vx,y-vy)<=r and angsep(threat_bearing(name,vy,vaz),tb)>90:
            return name
    return None
def town_near(x,y,r=1.0):
    return any(math.hypot(x-c["lon"],y-c["lat"])<=r for c in cities)

# Per-arc cable & depth budget (GAIA.md: the cable & depth budget —
# geography-grade reference, not survey data). cable = shore -> slope toe.
def cable_depth(trench, lon, lat):
    if trench.startswith("Philippine"):
        return ("50–80","4–6") if lat>=13 else ("20–40","4–6")   # Manila / E Mindanao
    if trench.startswith("Sunda"):
        return ("40–70","4.5–5.5") if lon<105 else ("60–100","4.5–5.5")  # Mentawai / Java
    return {"Aleutian Trench":("15–35","3.5–4.5"),
            "Japan–Kuril–Kamchatka Trench":("20–40","4–5"),
            "Tonga–Kermadec Trench":("15–40","4–6"),
            "New Hebrides Trench (Vanuatu)":("15–30","3–5"),
            "Lesser Antilles Trench":("30–60","4–5"),
            "Nankai–Ryukyu Trench":("30–60","3.5–4.5"),
            "Middle America Trench":("40–70","3.5–4.5"),
            "Peru–Chile Trench":("40–80","4–6"),
            "Cascadia Subduction Zone":("70–110","2.5–2.9"),
            "Hikurangi Trench (New Zealand)":("60–100","2.5–3.5")}[trench]

# The islet-first lever (GAIA.md, owner 2026-07-18): where a real islet sits
# seaward on the slope, the shore station goes ON the islet — the mainland
# haul collapses to ~10–30 km. Named real islets on the expensive tail; the
# nearest computed site snaps to each. Lobos de Afuera (PE) was here and failed
# the slope test — shelf-locked, 210 km to the toe (../WHY.md).
ISLETS = [("Isla Mocha (CL)",        -73.90, -38.37),
          ("Isla Santa María (CL)",  -73.53, -37.03),
          ("Islas Marías (MX)",     -106.53,  21.63)]

sites=[]; seen=set(); cab_ct=Counter()
for x,y,trench,az in ARC_T:
    k=(round(x/1.5),round(y/1.5))
    if k in seen: continue
    seen.add(k)
    net=cabled_net(trench,y)
    if net:
        if cab_ct[trench]%3: cab_ct[trench]+=1; continue   # cabled sea: hubs only
        cab_ct[trench]+=1
    tb=threat_bearing(trench,y,az)
    multi=second_threat(x,y,trench,tb)
    if multi:            sondes=8            # hit from both sides: keep the ring
    elif dart_near(x,y): sondes=4
    else:                sondes=5
    sites.append((x,y,trench,tb,sondes,net,multi,town_near(x,y)))

# ---- 3. backbone = biggest town in each empty 8-deg cell -------------------
taken={(round(c["lon"]/8),round(c["lat"]/8)) for c in seis}
back={}
for c in sorted(cities,key=lambda c:-c["p"]):
    if c["c"] in COVERED and c["p"]<1_000_000: continue
    key=(round(c["lon"]/8),round(c["lat"]/8))
    if key in taken or key in back: continue
    back[key]=c
backbone=list(back.values())

# ---- loadout per site (populate for the place, not full-kit) ---------------
def units_for(pop):
    """EEW wants aperture across a city: extra nodes per ~1M people, capped."""
    return 1 + min(pop // 1_000_000, 5)

def loadout_seismic(c):
    L = ["seismo (dense EEW)", "baro/temp"]
    if abs(c["lat"]) >= 45: L.append("snow radar")
    if c["p"] >= 100_000:   L.append("air quality / CO")
    return L

def loadout_coastal(radar):
    L = (["radar sea-gauge"] if radar else []) + \
        ["magnetometer (slope-toe array)", "seismo (borehole)", "baro"]
    return L

def loadout_backbone(c):
    L = ["magnetometer", "GNSS-TEC", "seismo (regional)"]
    if abs(c["lat"]) >= 45: L.append("snow radar")
    if abs(c["lat"]) >= 60: L.append("neutron (cosmic-ray)")
    if abs(c["lat"]) < 20:  L.append("[TEC priority: EIA band]")
    return L

# ---- write ----------------------------------------------------------------
feats=[]
def pt(lon,lat,color,cls,name,loadout,units=1,extra="",props=None):
    feats.append({"type":"Feature","geometry":{"type":"Point","coordinates":[round(lon,3),round(lat,3)]},
      "properties":{"name":name,"class":cls,"units":units,
                    "loadout":", ".join(loadout),"why":extra,
                    **(props or {}),
                    "marker-color":color,"marker-size":"small","marker-symbol":""}})

u_seis=0
for c in seis:
    u=units_for(c["p"]); u_seis+=u
    pt(c["lon"],c["lat"],"#dd6b20","seismic",c["n"],loadout_seismic(c),u,
       f"pop {c['p']:,} — {u} node(s); near an active belt/arc")
# snap the nearest site to each named islet (one site per islet)
islet_at={}
for iname,ix,iy in ISLETS:
    best=min(range(len(sites)),key=lambda i:math.hypot(sites[i][0]-ix,sites[i][1]-iy))
    # generous radius: the schematic trench lines wander a degree or two off
    # the real bathymetry — the named islet is the truer position of the two
    if math.hypot(sites[best][0]-ix,sites[best][1]-iy)<=3.5 and best not in islet_at:
        islet_at[best]=(iname,ix,iy)

u_coast=0
for i,(x,y,trench,tb,sondes,net,multi,radar) in enumerate(sites):
    u_coast+=sondes
    islet=islet_at.get(i)
    name="arc site"
    cab,dep=cable_depth(trench,x,y)
    if islet:
        name,x,y=islet; cab="10–30 (islet-first)"
    sector=f"{compass(tb)} {tb:03.0f}°"
    if multi:
        why=(f"island/coast array — {sondes} sondes as a RING: reachable from two arcs "
             f"({trench} + {multi}, bearings > 90° apart), waves can arrive from both sides "
             f"(GAIA.md: the directional rule, multi-threat exception)")
    else:
        why=(f"island/coast array — {sondes} sondes in a {sector} sector fan facing {trench}; "
             f"the lee shore is wave shadow, no sondes (GAIA.md: the directional rule; "
             f"gauss/README.md: The array)")
        if sondes==4: why+=" — DART watches the deep-water leg upstream, 4 suffice"
    if net: why+=f" — interop hub only, {net} already cables this sea"
    if not radar: why+=(" — no radar sea-gauge: bare exposed shore drowns/de-masts a surface "
                        "gauge; the slope-toe magnetometer IS the gauge here")
    if islet: why+=(" — islet-first: the shore station sits on this named islet, deep water "
                    "next door, the mainland haul collapses (GAIA.md: the islet-first lever)")
    why+=f" — cable ~{cab} km to the toe at {dep} km depth"
    pt(x,y,"#c53030","coastal array",name,loadout_coastal(radar),sondes,why,
       props={"threat-sector":sector,"trench":trench,
              "cable-km":cab,"toe-depth-km":dep,
              **({"hub-of":net} if net else {}),
              **({"multi-threat":multi} if multi else {}),
              **({"islet-first":True} if islet else {})})
u_back=0
for c in backbone:
    u_back+=1
    pt(c["lon"],c["lat"],"#718096","backbone",c["n"],loadout_backbone(c),1,
       f"pop {c['p']:,} — geomag/TEC gap fill")

n_seis,n_coast,n_back=len(seis),len(sites),len(backbone)
sites_total=n_seis+n_coast+n_back
units_total=u_seis+u_coast+u_back
n_hub=sum(1 for s in sites if s[5]); n_dart=sum(1 for s in sites if s[4]==4)
n_multi=sum(1 for s in sites if s[6]); n_radar=sum(1 for s in sites if s[7])
obj={"type":"FeatureCollection","properties":{
 "title":"NIC proposed network — data-driven placement (towns x hazard), loadout per site",
 "note":(f"COMPUTED placement, every marker a real town (GeoNames) or a real arc site — no lattice. "
   f"Rules: stations go where people can build and maintain them; countries with dense existing networks "
   f"(JP/TW/US/NZ/IT/CH/CL/MX/GR/IS) get only major hubs; backbone fills empty ~800 km cells with the "
   f"biggest town in the cell. Each marker carries its LOADOUT (populate for the place, not full-kit: no "
   f"snow radar in the tropics, cosmic-ray heads only at high latitude, air-quality only in real cities) "
   f"and its UNITS (a big city on a fault gets extra nodes for EEW aperture — +1 per ~1M people, cap 6). "
   f"Coastal arrays follow the DIRECTIONAL RULE: the 50-year source map is the known trenches, so each "
   f"site is a 5-sonde fan on the source-facing sector only (4 where DART watches the deep leg upstream); "
   f"the lee shore is wave shadow and gets nothing; an island reachable from TWO arcs keeps the ring "
   f"(8 sondes); cabled seas (S-net/DONET, NEPTUNE, GeoNet) keep only interop hubs. The radar sea-gauge "
   f"goes only where a real town (<= ~100 km) can host and keep the mast — a bare exposed shore drowns or "
   f"de-masts a surface gauge, so elsewhere the slope-toe magnetometer is the gauge. Each coastal marker "
   f"carries its CABLE-KM (shore -> slope toe, per-arc budget) and TOE-DEPTH-KM; where a real islet sits "
   f"seaward on the slope the station snaps to it by name (islet-first: the mainland haul collapses to "
   f"~10-30 km). "
   f"Totals: {n_seis:,} seismic towns ({u_seis:,} nodes) + {n_coast:,} coastal arrays "
   f"({u_coast:,} sondes, {n_hub} of the sites hubs on cabled seas, {n_multi} multi-threat rings, "
   f"radar at {n_radar}) + {n_back:,} backbone = "
   f"{sites_total:,} sites, ~{units_total:,} deployed units. "
   f"Orange=seismic (belt towns) · red=coastal array · grey=backbone (geomag/TEC)."),
 "sources":"GeoNames via all-the-cities (pop>=20k) · priority-tsunami/earthquake.geojson · existing-systems.geojson (DART) · GAIA.md"},
 "features":feats}
json.dump(obj,open(f"{GAIA}/proposed-network.geojson","w"),ensure_ascii=False)
print(f"sites: seismic {n_seis:,} · coastal {n_coast:,} ({n_hub} hubs, {n_dart} DART-trimmed, "
      f"{n_multi} multi-threat rings, radar at {n_radar}) · backbone {n_back:,} = {sites_total:,}")
print(f"units: seismic {u_seis:,} · sondes {u_coast:,} · backbone {u_back:,} = {units_total:,}")
print("countries (seismic top10):", Counter(c['c'] for c in seis).most_common(10))
print("bytes:", os.path.getsize(f"{GAIA}/proposed-network.geojson"))

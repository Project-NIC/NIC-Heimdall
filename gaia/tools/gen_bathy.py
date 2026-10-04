#!/usr/bin/env python3
"""Per-site cable & depth from real bathymetry (ETOPO via NOAA ERDDAP).

For every coastal-array site in ../proposed-network.geojson:
  1. fetch an ETOPO1 tile around the site (2-arc-min stride ~= 3.7 km,
     cached in bathy_cache/ so reruns are free),
  2. walk LANDWARD (against the threat bearing) to find the real shore,
  3. walk SEAWARD from that shore along the threat bearing, profile the
     depth, and find the toe of the slope (the gauss siting rule: full
     transport signal at the toe, the plateau beyond buys nothing) —
     toe = first crossing of min(4000 m, 0.85 x the profile's max depth),
  4. write cable length (shore -> toe) and depth at the sonde.

Output: ../bathy-per-site.csv (committed) + a per-arc summary on stdout.
Needs outbound HTTPS to coastwatch.pfeg.noaa.gov (ERDDAP) — allow the
domain in the environment's network policy. On 403/timeouts the script
says so and stops; partial progress is kept in the cache.

The doctrine this implements: GAIA.md "The cable & depth budget" — this
script replaces the geography-grade per-arc ranges with measured
per-site numbers. Coordinates are still schematic (the siting guide),
so treat results as planning-grade, not survey.
"""
import csv, json, math, os, sys, time, urllib.request, urllib.error

HERE  = os.path.dirname(os.path.abspath(__file__))
GAIA  = os.path.dirname(HERE)
CACHE = os.path.join(HERE, "bathy_cache")
os.makedirs(CACHE, exist_ok=True)

ERDDAP = ("https://coastwatch.pfeg.noaa.gov/erddap/griddap/etopo180.csv"
          "?altitude%5B({la0}):2:({la1})%5D%5B({lo0}):2:({lo1})%5D")

BOX_DEG      = 3.2     # half-size of the fetched tile
LAND_MAX_KM  = 320.0   # how far landward we search for a shore
SEA_MAX_KM   = 260.0   # how far seaward we profile
STEP_KM      = 2.0     # profile step
TOE_CAP_M    = 4000.0  # depth that counts as "the deep plain reached"
TOE_FRAC     = 0.85    # ...or this fraction of the profile's max depth

def fetch(url, tries=4):
    for i in range(tries):
        try:
            with urllib.request.urlopen(url, timeout=90) as r:
                return r.read().decode()
        except (urllib.error.URLError, urllib.error.HTTPError, OSError) as e:
            if i == tries - 1: raise
            time.sleep(2 ** (i + 1))

def tile(idx, lon, lat):
    """One cached ETOPO tile around (lon,lat); dateline-split when needed.
    Returns (lats_sorted, lons_sorted, {(lat,lon): alt})."""
    fn = os.path.join(CACHE, f"site_{idx:03d}.csv")
    if not os.path.exists(fn):
        la0, la1 = max(-89.9, lat - BOX_DEG), min(89.9, lat + BOX_DEG)
        spans = []
        lo0, lo1 = lon - BOX_DEG, lon + BOX_DEG
        if lo0 < -180: spans = [(-180, lo1), (lo0 + 360, 180)]
        elif lo1 > 180: spans = [(lo0, 180), (-180, lo1 - 360)]
        else: spans = [(lo0, lo1)]
        chunks = []
        for s0, s1 in spans:
            chunks.append(fetch(ERDDAP.format(la0=la0, la1=la1, lo0=s0, lo1=s1)))
            time.sleep(0.4)                    # be polite to ERDDAP
        with open(fn, "w", encoding="utf-8") as f: f.write("\n".join(chunks))
    grid = {}
    for line in open(fn, encoding="utf-8"):
        p = line.strip().split(",")
        if len(p) != 3: continue
        try: la, lo, al = float(p[0]), float(p[1]), float(p[2])
        except ValueError: continue
        grid[(round(la, 4), round(lo, 4))] = al
    lats = sorted({k[0] for k in grid}); lons = sorted({k[1] for k in grid})
    return lats, lons, grid

def nearest(vals, x):
    lo, hi = 0, len(vals) - 1
    while hi - lo > 1:
        m = (lo + hi) // 2
        if vals[m] < x: lo = m
        else: hi = m
    return vals[lo] if abs(vals[lo] - x) <= abs(vals[hi] - x) else vals[hi]

def sample(lats, lons, grid, lon, lat):
    lon = (lon + 180) % 360 - 180
    if not lats or not lons: return None
    return grid.get((nearest(lats, round(lat, 4)), nearest(lons, round(lon, 4))))

def walk(lon, lat, bearing_deg, km):
    b = math.radians(bearing_deg)
    dlat = km * math.cos(b) / 111.32
    dlon = km * math.sin(b) / (111.32 * math.cos(math.radians(lat)) or 1e-9)
    return lon + dlon, lat + dlat

def site_numbers(idx, lon, lat, tb):
    lats, lons, grid = tile(idx, lon, lat)
    # 1. landward: find the real shore (first land sample against the threat)
    shore = None
    d = 0.0
    while d <= LAND_MAX_KM:
        x, y = walk(lon, lat, (tb + 180) % 360, d)
        a = sample(lats, lons, grid, x, y)
        if a is not None and a >= 0: shore = (x, y, d); break
        d += STEP_KM
    if shore is None:
        return None                            # no land within reach — flagged
    sx, sy, _ = shore
    # 2. seaward profile from the shore along the threat bearing
    prof = []
    d = STEP_KM
    while d <= SEA_MAX_KM:
        x, y = walk(sx, sy, tb, d)
        a = sample(lats, lons, grid, x, y)
        if a is not None and a < 0: prof.append((d, -a))
        d += STEP_KM
    if not prof: return None
    dmax = max(p[1] for p in prof)
    target = min(TOE_CAP_M, TOE_FRAC * dmax)
    # what a fixed cable budget buys: depth reached at 10/20/50/100 km from shore
    # (signal tracks depth — 100 km is the module's reach)
    def depth_at(km):
        c = [dep for d, dep in prof if d <= km]
        return round(max(c)) if c else 0
    ats = {"depth_at_10km": depth_at(10), "depth_at_20km": depth_at(20),
           "depth_at_50km": depth_at(50), "depth_at_100km": depth_at(100)}
    for d, dep in prof:
        if dep >= target:
            return {"shore_lon": round(sx, 3), "shore_lat": round(sy, 3),
                    "cable_km": round(d), "sonde_depth_m": round(dep),
                    "profile_max_m": round(dmax), **ats}
    d, dep = prof[-1]
    return {"shore_lon": round(sx, 3), "shore_lat": round(sy, 3),
            "cable_km": round(d), "sonde_depth_m": round(dep),
            "profile_max_m": round(dmax), **ats, "note": "toe beyond profile reach"}

def main():
    # repo layout first (gaia/), then next to the script — so a bare download
    # of gen_bathy.py + proposed-network.geojson into one folder works too
    global GAIA
    for base in (GAIA, HERE):
        src = os.path.join(base, "proposed-network.geojson")
        if os.path.exists(src): break
    else:
        sys.exit("proposed-network.geojson not found — put it in the same "
                 "folder as gen_bathy.py (download it from the repo's gaia/ dir)")
    GAIA = base
    d = json.load(open(src, encoding="utf-8"))
    coast = [(i, f) for i, f in enumerate(d["features"])
             if f["properties"]["class"] == "coastal array"]
    rows, misses = [], 0
    for n, (i, f) in enumerate(coast):
        p = f["properties"]; lon, lat = f["geometry"]["coordinates"]
        tb = float(p["threat-sector"].split()[1].rstrip("°"))
        try:
            r = site_numbers(n, lon, lat, tb)
        except Exception as e:
            print(f"[{n:3d}] FETCH FAILED ({e}) — is coastwatch.pfeg.noaa.gov "
                  f"allowed in the network policy?", file=sys.stderr)
            sys.exit(1)
        row = {"site": n, "name": p["name"], "trench": p["trench"],
               "lon": lon, "lat": lat, "threat_deg": tb,
               "sondes": p["units"]}
        if r: row.update(r)
        else: row["note"] = "no shore/profile resolved (schematic position?)"; misses += 1
        rows.append(row)
        if n % 20 == 0: print(f"{n}/{len(coast)} sites done")
    cols = ["site","name","trench","lon","lat","threat_deg","sondes",
            "shore_lon","shore_lat","cable_km","sonde_depth_m","profile_max_m",
            "depth_at_10km","depth_at_20km","depth_at_50km","depth_at_100km","note"]
    out = os.path.join(GAIA, "bathy-per-site.csv")
    with open(out, "w", newline="", encoding="utf-8") as f:
        w = csv.DictWriter(f, fieldnames=cols); w.writeheader()
        for r in rows: w.writerow(r)
    ok = [r for r in rows if "cable_km" in r]
    tot = sum(r["cable_km"] for r in ok)
    print(f"\nresolved {len(ok)}/{len(rows)} sites ({misses} unresolved), "
          f"total cable ~{tot:,} km")
    per = {}
    for r in ok: per.setdefault(r["trench"], []).append(r)
    for t, rs in sorted(per.items(), key=lambda kv: -len(kv[1])):
        cab = sorted(x["cable_km"] for x in rs)
        dep = sorted(x["sonde_depth_m"] for x in rs)
        print(f"{t:40s} n={len(rs):3d}  cable med {cab[len(cab)//2]:4d} km "
              f"(min {cab[0]}, max {cab[-1]})  depth med {dep[len(dep)//2]:5d} m")
    print(f"wrote {out}")

if __name__ == "__main__":
    main()

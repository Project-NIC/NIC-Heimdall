★ N.I.C. ★

# GAIA — the public data shelf

The datasets the maps stand on are **public, free and already categorized** — nobody has to survey
an ocean to plan the network. What each one is for:

## Bathymetry & relief (cable lengths, sonde depths, slope toes)

| dataset | what | where |
|---|---|---|
| **GEBCO** | the global bathymetry grid (~15 arc-sec ≈ 450 m), one free download, whole planet | gebco.net |
| **ETOPO 2022** (NOAA) | global relief, 15/30/60 arc-sec; also served in slices via ERDDAP — what `tools/gen_bathy.py` uses | ncei.noaa.gov / coastwatch.pfeg.noaa.gov ERDDAP |
| **GMRT** | multibeam-backed compilation, higher resolution where ships actually mapped | gmrt.org |

## Tsunami & earthquake history (the closed source map of siting rule 4)

| dataset | what | where |
|---|---|---|
| **NOAA/NCEI historical tsunami database** | every recorded tsunami: source, magnitude, **runups per coastline** — the historical record the directional rule stands on | ncei.noaa.gov/hazard |
| **USGS ComCat** | the global earthquake catalogue, queryable API | earthquake.usgs.gov |
| **USGS Slab2** | 3-D geometry of every subduction slab — the *real* trench lines (ours are schematic) | USGS ScienceBase |
| **Global CMT** | focal mechanisms (which faults thrust, where wave-making quakes live) | globalcmt.org |
| **ISC-GEM** | the long, homogenised instrumental catalogue (1904–) | isc.ac.uk |

## Existing networks (rule 1: don't pile on)

| dataset | what | where |
|---|---|---|
| **NOAA NDBC** | live DART buoy positions & status | ndbc.noaa.gov |
| **IOC sea-level monitoring** | the world's tide/tsunami gauge stations, live | ioc-sealevelmonitoring.org |
| **FDSN station registry** | every registered seismic station/network on Earth | fdsn.org |
| **INTERMAGNET / SuperMAG** | the magnetometer observatories | intermagnet.org / supermag.jhuapl.edu |
| **PSMSL** | long-term tide-gauge records (what a coast's sea level has done for a century) | psmsl.org |

## Geography (rule 2: a named real place)

| dataset | what | where |
|---|---|---|
| **GeoNames** | every town on Earth with population — already the input of `tools/gen_network.py` | geonames.org |
| **Natural Earth** | coastlines, islands, land polygons, public domain | naturalearthdata.com |
| **OpenStreetMap** | harbours, piers, roads — whether an islet has anywhere to land a cable | openstreetmap.org |

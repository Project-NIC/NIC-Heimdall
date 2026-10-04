#!/usr/bin/env python3
"""Generates the unit coverage maps (tesla/pip/, quake/, gauss/ coverage.svg) from the layers here.

One projection, one palette, one legend/caveat pattern for every unit map: equirectangular
1000x500 + a 60 px footer, dark GitHub palette, land from vendor/land-110m.geojson,
30-degree graticule. The GeoJSON layers stay the authority — these SVGs are rendered views
for the unit READMEs; rerun this after editing a layer. Label positions are hand-placed in
the per-map lists below.
"""
import json, os

REPO = os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
W, H, MAPH = 1000, 560, 500

def norm(lon):
    while lon > 180: lon -= 360
    while lon < -180: lon += 360
    return lon

def xy(lon, lat):
    return ((norm(lon) + 180) / 360 * W, (90 - lat) / 180 * MAPH)

def fmt(v):
    return f'{v:.1f}'.rstrip('0').rstrip('.')

def land_path():
    d = json.load(open(f'{REPO}/gaia/vendor/land-110m.geojson'))
    parts = []
    for f in d['features']:
        g = f['geometry']
        polys = g['coordinates'] if g['type'] == 'MultiPolygon' else [g['coordinates']]
        for poly in polys:
            for ring in poly:
                pts = [xy(c[0], c[1]) for c in ring]
                parts.append('M' + 'L'.join(f'{fmt(x)} {fmt(y)}' for x, y in pts) + 'Z')
    return ''.join(parts)

def graticule():
    s = ['<g stroke="#262c36" stroke-width="1" fill="none">']
    for lat in (60, 30, -30, -60):
        y = fmt((90 - lat) / 180 * MAPH)
        s.append(f'<line x1="0" y1="{y}" x2="{W}" y2="{y}"/>')
    for lon in (-150, -120, -90, -60, -30, 30, 60, 90, 120, 150):
        x = fmt((lon + 180) / 360 * W)
        s.append(f'<line x1="{x}" y1="0" x2="{x}" y2="{MAPH}"/>')
    s.append('</g>')
    s.append(f'<line x1="0" y1="250" x2="{W}" y2="250" stroke="#3a424d" stroke-width="1.5"/>')
    s.append(f'<line x1="500" y1="0" x2="500" y2="{MAPH}" stroke="#3a424d" stroke-width="1.5"/>')
    s.append('<g fill="#6e7681" font-size="10">')
    for lat in (60, 30, 0, -30, -60):
        y = (90 - lat) / 180 * MAPH
        lbl = '0°' if lat == 0 else f'{abs(lat)}°{"N" if lat > 0 else "S"}'
        s.append(f'<text x="4" y="{fmt(y - 4)}">{lbl}</text>')
    for lon in (-90, 0, 90):
        x = (lon + 180) / 360 * W
        lbl = '0°' if lon == 0 else f'{abs(lon)}°{"W" if lon < 0 else "E"}'
        s.append(f'<text x="{fmt(x + 4)}" y="{MAPH - 6}">{lbl}</text>')
    s.append('</g>')
    return ''.join(s)

def polyline(coords, stroke, width=2.5, dash=None, opacity=0.9):
    # split at the antimeridian so a wrapped segment never streaks across the map
    runs, run = [], [xy(*coords[0][:2])]
    for c in coords[1:]:
        p = xy(*c[:2])
        if abs(p[0] - run[-1][0]) > 500:
            runs.append(run); run = [p]
        else:
            run.append(p)
    runs.append(run)
    d = 'stroke-dasharray="6 4" ' if dash else ''
    out = []
    for r in runs:
        if len(r) < 2: continue
        pts = ' '.join(f'{fmt(x)},{fmt(y)}' for x, y in r)
        out.append(f'<polyline points="{pts}" fill="none" stroke="{stroke}" '
                   f'stroke-width="{width}" stroke-opacity="{opacity}" '
                   f'stroke-linecap="round" stroke-linejoin="round" {d}/>')
    return ''.join(out)

def ring_path(coords):
    pts = [xy(c[0], c[1]) for c in coords]
    return 'M' + 'L'.join(f'{fmt(x)} {fmt(y)}' for x, y in pts) + 'Z'

def head(title):
    return (f'<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 {W} {H}" '
            f'width="{W}" height="{H}" font-family="DejaVu Sans, Verdana, sans-serif">'
            f'<title>{title}</title>'
            f'<rect width="{W}" height="{H}" fill="#0d1117"/>'
            f'<path d="{LAND}" fill="#1b2430"/>' + graticule())

def legend(items, caveat):
    s = ['<g transform="translate(20,522)" font-size="12">']
    x = 0
    for kind, color, text, dash in items:
        if kind == 'dot':
            s.append(f'<circle cx="{x + 6}" cy="-4" r="4" fill="{color}"/>')
        else:
            d = 'stroke-dasharray="6 4" ' if dash else ''
            s.append(f'<line x1="{x}" y1="-4" x2="{x + 14}" y2="-4" stroke="{color}" stroke-width="3" {d}/>')
        s.append(f'<text x="{x + 20}" y="0" fill="#c9d1d9">{text}</text>')
        x += 20 + len(text) * 6.2 + 26
    s.append('</g>')
    s.append(f'<text x="20" y="548" fill="#8b949e" font-size="11">{caveat}</text>')
    return ''.join(s)

LAND = land_path()

# ---------------------------------------------------------------- pip
def gen_pip():
    d = json.load(open(f'{REPO}/gaia/longwave-transmitters.geojson'))
    circles, dots = [], []
    for f in d['features']:
        g, p = f['geometry'], f['properties']
        if g['type'] == 'Polygon':
            cls = p.get('kind')
            if cls == 'time-code':
                circles.append(f'<path d="{ring_path(g["coordinates"][0])}" fill="#1f6feb" '
                               'fill-opacity="0.14" stroke="#58a6ff" stroke-width="1.2"/>')
            else:
                circles.append(f'<path d="{ring_path(g["coordinates"][0])}" fill="#d29922" '
                               'fill-opacity="0.14" stroke="#e3b341" stroke-width="1.2" stroke-dasharray="5 3"/>')
        else:
            c = p['marker-color']
            x, y = xy(*g['coordinates'][:2])
            dots.append(f'<circle cx="{fmt(x)}" cy="{fmt(y)}" r="3.5" fill="{c}"/>')
    labels = [  # text, x, y, color, anchor
        ('DCF77 · 77.5', 533, 100, '#c9d1d9', 'start'),
        ('MSF · 60', 484, 84, '#c9d1d9', 'end'),
        ('RBU · 66.66', 612, 82, '#c9d1d9', 'start'),
        ('WWVB · 60', 252, 170, '#c9d1d9', 'start'),
        ('BPC · 68.5', 812, 168, '#c9d1d9', 'end'),
        ('JJY · 40', 900, 138, '#c9d1d9', 'start'),
        ('JJY · 60', 858, 172, '#c9d1d9', 'start'),
        ('eLoran 100', 128, 82, '#e3b341', 'middle'),
        ('eLoran 100', 786, 132, '#e3b341', 'end'),
        ('eLoran 100', 872, 116, '#e3b341', 'start'),
    ]
    ltxt = ''.join(f'<text x="{x}" y="{y}" fill="{c}" font-size="11" font-weight="bold" '
                   f'text-anchor="{a}">{t}</text>' for t, x, y, c, a in labels)
    svg = (head('Pip — nominal longwave coverage') + ''.join(circles) + ''.join(dots) + ltxt +
           legend([('dot', '#58a6ff', 'time-code station — date + carrier', 0),
                   ('dot', '#e3b341', 'eLoran — time to ~100 ns, no date of its own', 0)],
                  'SCHEMATIC, equirectangular. Circles are NOMINAL free-field reach — the real limit is the '
                  'receiver&#8217;s surroundings, not the transmitter&#8217;s power. Interactive layer: gaia/longwave-transmitters.geojson.')
           + '</svg>')
    open(f'{REPO}/tesla/pip/coverage.svg', 'w').write(svg)

# ---------------------------------------------------------------- quake
def gen_quake():
    d = json.load(open(f'{REPO}/gaia/priority-earthquake.geojson'))
    lines, dots = [], []
    for f in d['features']:
        g, p = f['geometry'], f['properties']
        if g['type'] == 'LineString':
            lines.append(polyline(g['coordinates'], '#dd6b20'))
        else:
            x, y = xy(*g['coordinates'][:2])
            dots.append(f'<circle cx="{fmt(x)}" cy="{fmt(y)}" r="3.5" fill="#f0883e"/>')
    labels = [
        ('Alpide belt', 620, 158, 'middle'),
        ('North Anatolian', 622, 106, 'start'),
        ('San Andreas', 148, 158, 'middle'),
        ('Mid-Atlantic Ridge', 456, 300, 'start'),
        ('East African Rift', 606, 300, 'start'),
        ('Japan inland', 890, 118, 'middle'),
        ('Istanbul', 588, 130, 'start'),
        ('Kathmandu', 748, 186, 'start'),
        ('Tehran', 646, 144, 'start'),
        ('San Francisco Bay', 152, 132, 'middle'),
        ('Santiago', 296, 348, 'end'),
        ('Kabul', 700, 148, 'start'),
        ('Wellington', 980, 386, 'end'),
        ("L'Aquila", 520, 108, 'end'),
    ]
    ltxt = ''.join(f'<text x="{x}" y="{y}" fill="#e6a06f" font-size="10" font-weight="bold" '
                   f'text-anchor="{a}">{t}</text>' for t, x, y, a in labels)
    svg = (head('Quake — priority belts and cities') + ''.join(lines) + ''.join(dots) + ltxt +
           legend([('line', '#dd6b20', 'priority belt — dense, weakly covered, a cheap grid pays off', 0),
                   ('dot', '#f0883e', 'priority city (EEW-class density)', 0)],
                  'SCHEMATIC, equirectangular. Priority by global blindness, not exclusivity — the rules are '
                  'GAIA.md&#8217;s; interactive layer: gaia/priority-earthquake.geojson.')
           + '</svg>')
    open(f'{REPO}/quake/coverage.svg', 'w').write(svg)

# ---------------------------------------------------------------- gauss
def gen_gauss():
    d = json.load(open(f'{REPO}/gaia/priority-tsunami.geojson'))
    lines, dots = [], []
    for f in d['features']:
        g, p = f['geometry'], f['properties']
        if g['type'] == 'LineString':
            lines.append(polyline(g['coordinates'], '#c53030'))
        else:
            x, y = xy(*g['coordinates'][:2])
            dots.append(f'<circle cx="{fmt(x)}" cy="{fmt(y)}" r="3.5" fill="#fca5a5"/>')
    labels = [
        ('Kuril–Kamchatka', 936, 102, 'end'),
        ('Nankai–Ryukyu', 826, 184, 'end'),
        ('Aleutian', 72, 76, 'start'),
        ('Cascadia', 138, 128, 'end'),
        ('Middle America', 200, 236, 'end'),
        ('Peru–Chile', 268, 328, 'start'),
        ('Sunda', 762, 288, 'middle'),
        ('Manila', 828, 226, 'end'),
        ('Tonga–Kermadec', 984, 340, 'end'),
        ('New Hebrides', 952, 278, 'end'),
        ('Antilles', 342, 200, 'start'),
        ('Hikurangi', 980, 392, 'end'),
    ]
    ltxt = ''.join(f'<text x="{x}" y="{y}" fill="#e88989" font-size="10" font-weight="bold" '
                   f'text-anchor="{a}">{t}</text>' for t, x, y, a in labels)
    svg = (head('Gauss — trench roster for the tsunami array') + ''.join(lines) + ''.join(dots) + ltxt +
           legend([('line', '#c53030', 'megathrust trench — the tsunami source', 0),
                   ('dot', '#fca5a5', 'shore / islet sonde site — the sector fan faces the trench', 0)],
                  'SCHEMATIC, equirectangular. Sites are the computed first wave, not a survey — the rules are '
                  'GAIA.md&#8217;s; interactive layer: gaia/priority-tsunami.geojson.')
           + '</svg>')
    open(f'{REPO}/gauss/coverage.svg', 'w').write(svg)

gen_pip(); gen_quake(); gen_gauss()
for f in ('tesla/pip/coverage.svg', 'quake/coverage.svg', 'gauss/coverage.svg'):
    print(f, os.path.getsize(f'{REPO}/{f}'), 'bytes')

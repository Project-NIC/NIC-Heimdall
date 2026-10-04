★ N.I.C. ★

# Palatine — construction

> **Design-stage concept — nothing built.** The figures are verified against the parts' sheets and
> the parts before a board is made.

The mechanical build Palatine owns: the soil column and the snow radar's shroud. The exposure of
each instrument is `SITING.md`.

## The soil column — a patrona, packed on the bench

The soil sensors are not installed in a hole. They are **stacked into a tube in the workshop, capped,
driven to the site and knocked into the ground as one piece** — the *patrona*. The reason is depth:
soil readings only compare between stations if every station's sensor sits at the same depth, and
centimetre placement is something achieved on a bench, never standing over an open hole with a shovel.
Move the precise work indoors and the field job collapses to bore, drive, backfill, leave.

- **Perforated wall, geotextile lining.** The wall is perforated at the sensor depths so water and
  salts exchange with the surrounding ground — an unperforated tube measures its own contents forever.
  The inside is lined with **geotextile** so the packed soil does not sift out through the perforations
  while it is filled, levelled and tamped. The caps are for transport and come off at the hole.
- **The packed soil is a starting state, not a permanent one.** Filling every patrona from the same
  stock makes all stations begin on identical material. They will not stay identical — the column
  salinises and leaches over the years, and roots find it. That is the ground behaving normally, not
  the tube failing.
- **It is not serviceable, and that is the trade.** A dead sensor at depth means digging the tube
  out. A serviceable column would cost exactly the depth accuracy the patrona exists for, so it is not
  built that way.
- **A bundle of cables surfaces at the head**, one per sensor. A Ceres is a sealed glass dish at its
  depth with its board inside, lying flat in the packed sand; what surfaces is its flat four-wire
  cable (`../ceres/HARDWARE.md`, *The bowl*).
- **Undisturbed ground only.** A mast footing or the vault shaft changes drainage and heat for a radius
  around it; the patrona goes clear of both.
- **The surface over it follows the purpose.** For the climatological profile WMO (Vol. I, chapter 2,
  §2.1.4.2.2) wants soil temperatures at 20 cm and shallower under **a level patch of bare ground,
  about 2 × 2 m**, typical of the surrounding soil; a farm's profile goes under the crop it is asked
  about. The patrona's head is set flush, and the patch is kept bare or cropped as the site decides.

Depths are picked from how far each thermal cycle actually reaches. Amplitude falls as e^(−z/d):

| cycle | damping depth d | amplitude at 2,5 m |
|---|---|---|
| daily | ~0,12 m | nil — gone by ~0,4 m |
| annual | ~2,24 m | ~⅓ of the surface swing |

Moist soil, α ≈ 0,5×10⁻⁶ m²/s; dry sand damps shallower, saturated clay deeper. That is the
background; **the depths themselves are from the standard series 5 · 10 · 20 · 50 · 100 cm — −10 and
−50 cm, and a farm's profile −10 · −20 · −50 · −100 cm — and the tube runs about 30 cm below its deepest sensor for its footing.** A sensor
wanted deeper (three metres, say) is a longer tube in an augered hole, backfilled and tamped —
an option, not a roster. A centimetre of seating error is nothing against what roots and
settling do to the surface in two seasons; the tube is set against the terrain as well as a
collar allows. The tube material is the **builder's call**; this is the concept, not a parts list.

## The snow radar's shroud

**The radar sits recessed at the top of a short smooth-bore plastic tube**, ~0,3 m long, Ø 110 or
125 mm, aimed straight down and truly vertical, **its mouth flared with a concentric PVC increaser
to Ø 160 mm**. Snow and rime build on the rim and never on the lens, and the bore keeps the near
field clear. It works because the beam is 3°: at the 0,3 m mouth it is ~1,6 cm wide
(`r = 0,3 · tan 1,5°`), so it clears the bore many times over — a 26 GHz radar's 8–10° beam would
scrape it.

- **The flare is a small horn**: it eases the launch into free space with less reflection than a
  sharp pipe end, and a wide mouth opening downward resists bridging and drips meltwater clear,
  which is what lets the tube be short.
- **Concentric and conical**, never an eccentric reducer, which tilts the axis; **no step or socket
  shoulder in the beam path**, which is a reflector at 80 GHz.
- **Plastic, not metal**: it runs warmer, frosts less, sheds water, and reflects the antenna's side
  lobes far less than a cold metal bore.
- **What it costs**: ~0,3 m of top range, the antenna being recessed — the mast stands high enough
  that the deepest snow never climbs into the bore. With a heated lens in severe rime, neither bore
  nor mouth ices shut.

The shroud guards against snow on the rim and a fouled lens; a weak return from dry powder is the
unit's quality register's.

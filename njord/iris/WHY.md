★ N.I.C. ★

# Iris and Arke — why

The graveyard of the board and of Njord's link: what was weighed and left, and why.

## Galvani's boards for the switching

Arke's sensors' feed switched by `G-12-S` and their line by `G-I-M-005` was proposed first. Galvani
is built for the station's feeds: isolated converters and `INA237`s that draw, converters sized for
watts, connectors that would make a small unit large. Left for one small board with the MOD's front
twice and an eFuse.

## Five terminals, and a sixth wire for the role

A board with five terminals — 12 V, 0 V, A, B and a switched 12 V — and a sixth wire to say which role
it was: superseded by a male input and a female output, the genders keying the cables, and the role
read from the output's current.

## A role found at every start

Found anew at each start, an Arke whose arm a strike had killed would come up as Iris and fall silent.
The role is kept from the first start until a magnet resets it.

## A NOD for Iris

Iris as a NodBus unit would carry a fleet's zeros on every frame. Left for one ModBus slave.

## Each field sensor as its own slave

Iris answering for every field sensor at its own ModBus address, invisible to the station: a ModBus
type has four NUMBERs, so four units of one kind fill it. Left for one slave carrying multi-sensor
blocks keyed by the unit's number.

## A queue that Palatine empties

A queue read with removal loses its record when a read is lost on the bus. Left for a window a unit
holding its latest record.

## Iris emulated in software

The gateway to the internet as Iris's role in software on another board: left — Huginn is a unit of
its own.

## LoRaWAN's 4 B MIC

The professionals' AES-CMAC MIC of 4 B was proposed; the station's own 8 B HMAC-SHA-256 tag was kept —
one construction and one library with the station, 4 B more on the air.

## The downlink refused whole

The head's modem stays out-only (`../../core/WHY.md`); Njord's acknowledgement may carry a value or
two, signed, and nothing more — never the registers, never the firmware over a public band.

## Slowing the packets by polling through the air

Palatine polling each field sensor through the radio: ten sensors every minute at SF9 would pass the
1 %; packed and pushed, the same in ~48 B is 0,5 %.

## Links without LoRa

Meteor-burst (as the US snow network SNOTEL reported its mountain stations on ~40–50 MHz) and HF data
modems carry a few dozen bytes an hour over hundreds of km without a satellite — on licensed
frequencies with a master station's antennas; the licence-free bands near give ~10 mW. Islands keep
the Mayak's satellite IoT (`LR1121`, Kinéis).

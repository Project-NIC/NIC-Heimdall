★ N.I.C. ★

# The protocol's core

The bytes every board of the station speaks, as [`../PROTOCOL.md`](../PROTOCOL.md) §1, §2 and §9
fix them — the first of the portable libraries §8 names: C99, no allocation, no hardware, reaching
nothing but the bytes it is given, compiled into the head's, the cards' and every unit's firmware
alike and unit-tested on a PC.

| file | what |
|---|---|
| `nic_proto.h` · `nic_proto.c` | the CRC; a frame told by its length; the three buses' addresses packed and checked; DATA on a spur, on mini and on the trunk, the card's completion of a spur frame in place; CONTROL with `SUB` and an Argus's swap of the last hop; a wide register's `kind` 5 payload; the master's roster keyed by (bus, TYPE, NUMBER) in 256-entry tables |
| `test/test_proto.c` | every rule above against the document's own examples — Quake #2 is `0x52`, Gauss #2 `0x12`, Pluvius #2 `0x16`, Modbus's ceiling 247 — every bit flip caught, the trunk frame never moved, the second across a wrap, a sonde seeing its own address, a host dropped with what hangs under it |

```
make check                     # with $CC; CC=clang, or CFLAGS with -fsanitize=address,undefined
```

**Owed here**: the `LOAD` stream and the `APPLY` trial boot (§7), then each layer the firmware
pages list — a card's per-spur masters, the head's ingest — each beside this one, on it.

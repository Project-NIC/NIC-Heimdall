★ N.I.C. ★

# The protocol's core

The bytes every board of the station speaks, as [`../PROTOCOL.md`](../PROTOCOL.md) §1, §2 and §9
fix them — the first of the portable libraries §8 names: C99, no allocation, no hardware, reaching
nothing but the bytes it is given, compiled into the head's, the cards' and every unit's firmware
alike and unit-tested on a PC.

| file | what |
|---|---|
| `nic_proto.h` · `nic_proto.c` | the CRC; a frame told by its length; the three buses' addresses packed and checked; DATA on a spur, on mini and on the trunk, the card's completion of a spur frame in place; CONTROL with `SUB` and an Argus's swap of the last hop; a wide register's `kind` 5 payload; the master's roster keyed by (bus, TYPE, NUMBER) in 256-entry tables |
| `nic_load.h` · `nic_load.c` | `LOAD`, the `kind` 9 stream and `APPLY` (§7): the image header, the unit's receiver writing a slot through the firmware's flash callbacks and checking count, TYPE, length and CRC-32, the master's frames on a spur or cut for an Argus, and the trial boot — an untried image started once, confirmed by its first answered `GET VERSION`, else the old slot back; ERROR's codes (§9) |
| `nic_card.h` · `nic_card.c` | the card's data path above its pins (`../../bifrost/FIRMWARE.md` §6–§8): the slot window, a unit's circular buffer — written as frames arrive, read at their period plus the card's delay, one `RESEND` after a CRC miss and none after the second, a FILLER with its reason for every period that has no whole frame, the ladder after three in the wrong place — the segment table, the ranging rule (the median's route, kept within 2 ticks, rewritten past them, two agreeing measurements past 128), the ladder's five steps (eight rounds, the feed off a second and four to rejoin, three times, then dead), the `PORTS` frame, an Argus's tiling of four sondes under one NUMBER |
| `nic_ingest.h` · `nic_ingest.c` | the head's ingest above its pins (`../../mayak/FIRMWARE.md` §6), on the archive library in `../archive/ref/c/`: every good trunk frame into its second's bucket, two open; a second closed one second after its end, a hole a FILLER, *no data arrived*; each address through its recording rule into its buffer; a buffer a segment at 16 kB or 8 s; a frame repaired after its second closed kept as a record with an older second |
| `nic_cells.h` · `nic_cells.c` | the configuration cells (`../PROTOCOL.md` §7): 16 B cells appended in one 8 KB sector through the firmware's flash callbacks, the last good per id, the live set rewritten when full and once a year, the persisted set's one check |
| `nic_sweep.h` · `nic_sweep.c` | the sweep (`../PROTOCOL.md` §2): the tag, a unit's wait of tag × 10 µs, listening before it talks and waiting again when it hears the bus, a new wait after a collision on the UID turned by the attempt, the second tag in the reserve bytes; the host's windows closed by 700 ms of silence and the round by a second silent one; `ASSIGN_ADDR` by both tags |
| `nic_cmd.h` · `nic_cmd.c` | the head's check of a command from outside (`../PROTOCOL.md` §6, `../../mayak/FIRMWARE.md` §8): the counter above the stored one, checked before any MAC; the tag over the station's 3 B and the message under K0‖K1, or K0 alone for `SET K1`; the signer's side; on the archive library's SHA-256 |
| `nic_tree.h` · `nic_tree.c` | the head's station tree (`../../mayak/FIRMWARE.md` §7): the cards' and the Arguses' `kind` 4 tables composed, the tags read per slot, a sonde found by its NUMBER and position; each row's NUMBER decided against the mirror — confirmed, moved back, renumbered, a stranger, new, a replacement put to the human — the `ASSIGN_ADDR` by both tags, the roster written from the tree |
| `test/test_proto.c` | every rule above against the document's own examples — Quake #2 is `0x52`, Gauss #2 `0x12`, Pluvius #2 `0x16`, Modbus's ceiling 247 — every bit flip caught, the trunk frame never moved, the second across a wrap, a sonde seeing its own address, a host dropped with what hangs under it |
| `test/test_load.c` | an image streamed into a slot on a spur and through an Argus, a frame left out or failing its CRC, another TYPE's image, a wrong count, a bad CRC-32, a table past its slot, a failing erase; a trial confirmed, and one that resets unconfirmed handing the boot back |
| `test/test_card.c` | the window's edges; a frame a second late and stamped with its own second, across the second and around the ring; every filler reason; one `RESEND` repaired, two given up; the ladder and its reset, the ranging rule's cases, the ladder's three power cycles to dead; the table's and the `PORTS` frame's bytes; a tile's flags, worst code and a silent seated sonde |
| `test/test_ingest.c` | three units for twenty seconds — `ALL`, `CHANGE` per block, `NONZERO` — through the buckets, the close, the rules and the spool into HMC files read back: the holes as FILLERs, a late frame repaired right behind the second it came in, every segment within 16 kB and 8 s, each rule keeping what it says |
| `test/test_cells.c` | a simulated H5 sector: the last good value, a torn cell skipped, the full sector rewritten with every last value, the set's check failing when one member moves alone, the yearly refresh, an erase with nothing written behind it read as no set |
| `test/test_sweep.c` | a shared segment simulated at 38 400 with its 3 µs collision window: eight fresh units on one default byte found once each by their tags, an enrolled one silent, two units of one tag parted after one collision and told apart by their second tags, an empty segment closed by two silent windows, `ASSIGN_ADDR` taken by its tags alone; the round's length measured |
| `test/test_cmd.c` | a `WINDOW` made by the session's code taken, its replay dropped; one signed here read by the session's parser; a changed byte, another station, the wrong key, a stale counter with no MAC computed; `SET K1` under K0 alone, the old K1 refused after it |
| `test/test_tree.c` | three cards and an Argus on two NUMBERs: a table sent again replacing the old, positions 0–3 and 4–7; every verdict of §7 against a mirror — a confirmed box of two NUMBERs, a collision, a fresh unit offered a missing number and the next fresh one new, a moved unit, a stranger — the `ASSIGN_ADDR` taken by the unit; the roster's parents; two Teslas #5 leaving as #1 and #2; a type full |

```
make check                     # with $CC; CC=clang, or CFLAGS with -fsanitize=address,undefined
make CC="arm-linux-gnueabihf-gcc -static" test/test_proto test/test_load      # a 32-bit ARM,
qemu-arm test/test_proto; qemu-arm test/test_load                               # run under QEMU
make cross                     # the Cortex-M33 and the head's RV32 — arm-none-eabi, riscv64-unknown-elf
```

**Owed here**: each layer the firmware pages list — a card's sweep, ranging and ladder, the head's tree and control plane — each beside this one, on it.

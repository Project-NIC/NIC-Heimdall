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
| `nic_card.h` · `nic_card.c` | the card's data path above its pins (`../../bifrost/FIRMWARE.md` §6–§8): the slot window, a unit's circular buffer — written as frames arrive, read at their period plus the card's delay, one `RESEND` after a CRC miss and none after the second, a FILLER with its reason for every period that has no whole frame, the ladder after three in the wrong place — the segment table, the `PORTS` frame, an Argus's tiling of four sondes under one NUMBER |
| `nic_ingest.h` · `nic_ingest.c` | the head's ingest above its pins (`../../mayak/FIRMWARE.md` §6), on the archive library in `../archive/ref/c/`: every good trunk frame into its second's bucket, two open; a second closed one second after its end, a hole a FILLER, *no data arrived*; each address through its recording rule into its buffer; a buffer a segment at 16 kB or 8 s; a frame repaired after its second closed kept as a record with an older second |
| `test/test_proto.c` | every rule above against the document's own examples — Quake #2 is `0x52`, Gauss #2 `0x12`, Pluvius #2 `0x16`, Modbus's ceiling 247 — every bit flip caught, the trunk frame never moved, the second across a wrap, a sonde seeing its own address, a host dropped with what hangs under it |
| `test/test_load.c` | an image streamed into a slot on a spur and through an Argus, a frame left out or failing its CRC, another TYPE's image, a wrong count, a bad CRC-32, a table past its slot, a failing erase; a trial confirmed, and one that resets unconfirmed handing the boot back |
| `test/test_card.c` | the window's edges; a frame a second late and stamped with its own second, across the second and around the ring; every filler reason; one `RESEND` repaired, two given up; the ladder and its reset; the table's and the `PORTS` frame's bytes; a tile's flags, worst code and a silent seated sonde |
| `test/test_ingest.c` | three units for twenty seconds — `ALL`, `CHANGE` per block, `NONZERO` — through the buckets, the close, the rules and the spool into HMC files read back: the holes as FILLERs, a late frame repaired right behind the second it came in, every segment within 16 kB and 8 s, each rule keeping what it says |

```
make check                     # with $CC; CC=clang, or CFLAGS with -fsanitize=address,undefined
make CC="arm-linux-gnueabihf-gcc -static" test/test_proto test/test_load      # a 32-bit ARM,
qemu-arm test/test_proto; qemu-arm test/test_load                               # run under QEMU
make cross                     # the Cortex-M33 and the head's RV32 — arm-none-eabi, riscv64-unknown-elf
```

**Owed here**: each layer the firmware pages list — a card's sweep, ranging and ladder, the head's tree and control plane — each beside this one, on it.

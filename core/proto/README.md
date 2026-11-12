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
| `test/test_proto.c` | every rule above against the document's own examples — Quake #2 is `0x52`, Gauss #2 `0x12`, Pluvius #2 `0x16`, Modbus's ceiling 247 — every bit flip caught, the trunk frame never moved, the second across a wrap, a sonde seeing its own address, a host dropped with what hangs under it |
| `test/test_load.c` | an image streamed into a slot on a spur and through an Argus, a frame left out or failing its CRC, another TYPE's image, a wrong count, a bad CRC-32, a table past its slot, a failing erase; a trial confirmed, and one that resets unconfirmed handing the boot back |

```
make check                     # with $CC; CC=clang, or CFLAGS with -fsanitize=address,undefined
make CC="arm-linux-gnueabihf-gcc -static" test/test_proto test/test_load      # a 32-bit ARM,
qemu-arm test/test_proto; qemu-arm test/test_load                               # run under QEMU
make cross                     # the Cortex-M33 and the head's RV32 — arm-none-eabi, riscv64-unknown-elf
```

**Owed here**: each layer the firmware pages list — a card's per-spur masters, the head's ingest — each beside this one, on it.

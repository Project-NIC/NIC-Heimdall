# The protocol core on a PC. The firmware compiles the same nic_proto.c for its MCU.
CC     ?= cc
CFLAGS ?= -std=c99 -O2 -Wall -Wextra -Wpedantic -Werror

test/test_proto: nic_proto.c nic_proto.h test/test_proto.c
	$(CC) $(CFLAGS) -o $@ nic_proto.c test/test_proto.c

test/test_load: nic_proto.c nic_load.c nic_proto.h nic_load.h test/test_load.c
	$(CC) $(CFLAGS) -o $@ nic_proto.c nic_load.c test/test_load.c

test/test_card: nic_proto.c nic_card.c nic_proto.h nic_card.h test/test_card.c
	$(CC) $(CFLAGS) -o $@ nic_proto.c nic_card.c test/test_card.c

ARCH = ../archive/ref/c

test/test_cells: nic_proto.c nic_load.c nic_cells.c nic_cells.h test/test_cells.c
	$(CC) $(CFLAGS) -o $@ nic_proto.c nic_load.c nic_cells.c test/test_cells.c

test/test_ingest: nic_proto.c nic_ingest.c nic_ingest.h test/test_ingest.c $(ARCH)/hcc.c $(ARCH)/hmc.c
	$(CC) $(CFLAGS) -ffp-contract=off -I$(ARCH) -o $@ nic_proto.c nic_ingest.c $(ARCH)/hcc.c $(ARCH)/hmc.c test/test_ingest.c -lm

check: test/test_proto test/test_load test/test_card test/test_ingest test/test_cells
	./test/test_proto
	./test/test_load
	./test/test_card
	./test/test_ingest
	./test/test_cells

# the same sources for the MCUs: the H523's and H7A3's Cortex-M, the head's 32-bit RISC-V
cross:
	arm-none-eabi-gcc -mcpu=cortex-m33 -mthumb -mfloat-abi=hard -mfpu=fpv5-sp-d16 $(CFLAGS) -Os -c nic_proto.c -o /dev/null
	arm-none-eabi-gcc -mcpu=cortex-m33 -mthumb -mfloat-abi=hard -mfpu=fpv5-sp-d16 $(CFLAGS) -Os -c nic_load.c -o /dev/null
	riscv64-unknown-elf-gcc -march=rv32imafc_zicsr -mabi=ilp32f --specs=picolibc.specs $(CFLAGS) -Os -c nic_proto.c -o /dev/null
	riscv64-unknown-elf-gcc -march=rv32imafc_zicsr -mabi=ilp32f --specs=picolibc.specs $(CFLAGS) -Os -c nic_load.c -o /dev/null
	arm-none-eabi-gcc -mcpu=cortex-m33 -mthumb -mfloat-abi=hard -mfpu=fpv5-sp-d16 $(CFLAGS) -Os -c nic_card.c -o /dev/null
	arm-none-eabi-gcc -mcpu=cortex-m33 -mthumb -mfloat-abi=hard -mfpu=fpv5-sp-d16 $(CFLAGS) -Os -c nic_cells.c -o /dev/null
	riscv64-unknown-elf-gcc -march=rv32imafc_zicsr -mabi=ilp32f --specs=picolibc.specs $(CFLAGS) -Os -I$(ARCH) -c nic_ingest.c -o /dev/null

clean:
	rm -f test/test_proto test/test_load test/test_card test/test_ingest test/test_cells

.PHONY: check cross clean

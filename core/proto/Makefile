# The protocol core on a PC. The firmware compiles the same nic_proto.c for its MCU.
CC     ?= cc
CFLAGS ?= -std=c99 -O2 -Wall -Wextra -Wpedantic -Werror

test/test_proto: nic_proto.c nic_proto.h test/test_proto.c
	$(CC) $(CFLAGS) -o $@ nic_proto.c test/test_proto.c

test/test_load: nic_proto.c nic_load.c nic_proto.h nic_load.h test/test_load.c
	$(CC) $(CFLAGS) -o $@ nic_proto.c nic_load.c test/test_load.c

check: test/test_proto test/test_load
	./test/test_proto
	./test/test_load

clean:
	rm -f test/test_proto test/test_load

.PHONY: check clean

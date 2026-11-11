# The C reference. -ffp-contract=off: the predictor rounds every double operation as the Python
# does, never fused (HCC.md, *The coder's choices*).
CC      ?= cc
CFLAGS  ?= -std=c99 -O2 -Wall -Wextra -Wpedantic
CFLAGS  += -ffp-contract=off

hcc_tool: hcc.c hmc.c hcc_tool.c hcc.h hmc.h
	$(CC) $(CFLAGS) -o $@ hcc.c hmc.c hcc_tool.c -lm

clean:
	rm -f hcc_tool

.PHONY: clean

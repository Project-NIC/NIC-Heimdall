# The C reference: Heimdall's profiles on MLF's and NRC's cores from NIC-SDK, beside this
# repository or where NIC_SDK points. -ffp-contract=off: the predictor rounds every double
# operation as the Python does, never fused (NRC.md, *The coder's choices*).
CC      ?= cc
NIC_SDK ?= ../../../../../NIC-SDK
CFLAGS  ?= -std=c99 -O2 -Wall -Wextra -Wpedantic
CFLAGS  += -ffp-contract=off -I$(NIC_SDK)/mlf -I$(NIC_SDK)/nrc

SRC = $(NIC_SDK)/nrc/nrc.c $(NIC_SDK)/mlf/mlf.c hnrc.c hmlf.c session.c hnrc_tool.c

hnrc_tool: $(SRC) hnrc.h hmlf.h session.h $(NIC_SDK)/nrc/nrc.h $(NIC_SDK)/mlf/mlf.h
	$(CC) $(CFLAGS) -o $@ $(SRC) -lm

clean:
	rm -f hnrc_tool

.PHONY: clean

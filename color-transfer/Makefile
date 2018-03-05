ROOT = ../bcl
ROOT = ../bcl

CPPFLAGS = -I$(ROOT)/include -I.
CFLAGS   = -Wall -Wextra -std=c99 -g
LDFLAGS  = -L$(ROOT)/lib
LDLIBS   = -lbcl -lm

BIN = color-transfer

.PHONY: all
all: $(BIN)


.PHONY: clean cleanall
clean:
	$(RM) *.o
cleanall: clean
	$(RM) $(BIN)

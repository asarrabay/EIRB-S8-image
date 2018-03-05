ROOT = ../bcl
ROOT = ../bcl

CPPFLAGS = -I$(ROOT)/include -I.
CFLAGS   = -Wall -Wextra -std=c99 -g
LDFLAGS  = -L$(ROOT)/lib
LDLIBS   = -lbcl -lm

BIN = colorization
.PHONY: all
all: $(BIN)

test: all
	./colorization ../data/tree-c.ppm ../data/tree-g.ppm a.ppm && pvisu a.ppm

testall: test
	./colorization ../data/insect-c.ppm ../data/insect-g.ppm a.ppm && pvisu a.ppm
	./colorization ../data/sat-c.ppm ../data/sat-g.ppm a.ppm && pvisu a.ppm


.PHONY: clean cleanall
clean:
	$(RM) *.o
cleanall: clean
	$(RM) $(BIN)

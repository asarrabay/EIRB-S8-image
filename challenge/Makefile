ROOT=../bcl

CPPFLAGS = -I$(ROOT)/include -I.
CFLAGS   = -Wall -Wextra -Werror -pedantic -std=c99 -g
LDFLAGS  = -L$(ROOT)/lib
LDLIBS   = -lbcl -lm -lfftw3

BIN=pipeline

.PHONY: all
all: $(BIN)

pipeline: domain.o fft.o

.PHONY: clean cleanall
clean:
	$(RM) *.o *.ppm
cleanall: clean
	$(RM) $(BIN)

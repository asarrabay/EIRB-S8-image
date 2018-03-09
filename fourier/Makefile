ROOT=../bcl

CPPFLAGS = -I. -I$(ROOT)/include
CFLAGS   = -Wall -Wextra  -pedantic -std=c99 -g
LDFLAGS  = -L$(ROOT)/lib
LDLIBS   = -lbcl -lfftw3 -lm

BIN = test-fft

.PHONY: all
all: $(BIN)

test-fft: test-fft.o fft.o

test: all
	./test-fft ../data/lena-gray.ppm && pvisu FB-lena-gray.ppm && pvisu FAS-lena-gray.ppm && pvisu FREQ-lena-gray.ppm

.PHONY: clean cleanall
clean:
	$(RM) *.o *~ *.ppm

cleanall: clean
	$(RM) $(BIN)

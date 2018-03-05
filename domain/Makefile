ROOT = ../bcl
ROOT = ../bcl

CPPFLAGS = -I$(ROOT)/include -I.
CFLAGS   = -Wall -Wextra -Werror -std=c99 -g
LDFLAGS  = -L$(ROOT)/lib
LDLIBS   = -lbcl -lm

BIN = \
	move\
	rotate\
	wave\
	scroll\
	flip\
	shear\
	sphere

.PHONY: all
all: $(BIN)

shear: domain.o
rotate: domain.o
wave: domain.o
sphere: domain.o

.PHONY: clean cleanall
clean:
	$(RM) *.o
cleanall: clean
	$(RM) $(BIN)

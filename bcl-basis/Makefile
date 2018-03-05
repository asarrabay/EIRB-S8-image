ROOT=../bcl

CPPFLAGS = -I$(ROOT)/include
CFLAGS   = -Wall -Wextra -Werror -g -pedantic -std=c99
LDFLAGS  = -L$(ROOT)/lib
LDLIBS   = -lbcl

BIN=\
	test-bcl\
	extract-subimage\
	gray2color\
	color2mean\
	normalize\
	extract-channel

.PHONY: all
all:$(BIN)

.PHONY: clean cleanall
clean:
	$(RM) *.o *~ *.ppm
cleanall: clean
	$(RM) $(BIN)

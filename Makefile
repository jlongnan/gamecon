CC = gcc
AR = ar
RM = rm -f
CFLAGS ?= -std=c99 -Wall -Wextra -Wstrict-prototypes -O2
# Retain the original override name for existing users.
CCFLAGS ?= $(CFLAGS)
RUNNER ?=
SAMPLES = animation animation2 animation3 control display text

.PHONY: all samples test clean $(SAMPLES)
all: libgamecon.a samples

libgamecon.a: gamecon.o
	$(AR) rcs $@ $^

gamecon.o: gamecon.c gamecon.h
	$(CC) $(CPPFLAGS) $(CCFLAGS) -c $< -o $@

samples: $(SAMPLES)

$(SAMPLES): libgamecon.a
	$(MAKE) -C samples/$@ CC="$(CC)" AR="$(AR)" CCFLAGS="$(CCFLAGS)"

tests/regression.exe: tests/regression.c gamecon.h libgamecon.a
	$(CC) $(CPPFLAGS) $(CCFLAGS) -I. $< -L. -lgamecon -lgdi32 $(LDFLAGS) -o $@

tests/samples.exe: tests/samples.c
	$(CC) $(CPPFLAGS) $(CCFLAGS) $< $(LDFLAGS) -o $@

test: samples tests/regression.exe tests/samples.exe
	$(RUNNER) ./tests/regression.exe
	$(RUNNER) ./tests/samples.exe

clean:
	$(RM) libgamecon.a gamecon.o tests/regression.exe tests/samples.exe
	for sample in $(SAMPLES); do $(MAKE) -C samples/$$sample clean RM="$(RM)" || exit $$?; done

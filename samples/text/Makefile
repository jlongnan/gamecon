CC = gcc
AR = ar
RM = rm -f
CFLAGS ?= -std=c99 -Wall -Wextra -Wstrict-prototypes -O2
CCFLAGS ?= $(CFLAGS)

text.exe: text.o ../../libgamecon.a
	$(CC) $< -o $@ -L../.. -lgamecon -lgdi32 -mwindows $(LDFLAGS)

text.o: text.c ../../gamecon.h
	$(CC) $(CPPFLAGS) $(CCFLAGS) -I../.. -c $< -o $@

../../libgamecon.a: ../../gamecon.c ../../gamecon.h
	$(MAKE) -C ../.. libgamecon.a CC="$(CC)" AR="$(AR)" CCFLAGS="$(CCFLAGS)"

.PHONY: clean
clean:
	$(RM) text.exe text.o

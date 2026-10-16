CC = gcc
AR = ar
RM = rm -f
CFLAGS ?= -std=c99 -Wall -Wextra -Wstrict-prototypes -O2
CCFLAGS ?= $(CFLAGS)

animation3.exe: animation3.o ../../libgamecon.a
	$(CC) $< -o $@ -L../.. -lgamecon -lgdi32 -mwindows $(LDFLAGS)

animation3.o: animation3.c ../../gamecon.h
	$(CC) $(CPPFLAGS) $(CCFLAGS) -I../.. -c $< -o $@

../../libgamecon.a: ../../gamecon.c ../../gamecon.h
	$(MAKE) -C ../.. libgamecon.a CC="$(CC)" AR="$(AR)" CCFLAGS="$(CCFLAGS)"

.PHONY: clean
clean:
	$(RM) animation3.exe animation3.o

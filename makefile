# lackey - curses calendar program
# See COPYING file for license details.

-include config.mk

# Settings
VERSION   ?= 0.1-rc1
PREFIX    ?= /usr/local
MANPREFIX ?= $(PREFIX)/share/man

# Compiler
GCC       ?= gcc
CFLAGS    ?= -Wall --std=c99
CPPFLAGS  ?= -Isrc
LDFLAGS   ?= -lncursesw -lical

# Sources
PROG      ?= lackey
PROG_SRC  ?= main view date cal conf util
TEST      ?= test
TEST_SRC  ?= test date cal conf util
VIEWS     ?= day week month year events todo settings help edit
CALS      ?= dummy ical

# For ncursesw
CPPFLAGS  += $(strip $(shell pkg-config --cflags ncursesw))

# Targets
all: $(PROG)

clean:
	rm -f src/*.o views/*.o cals/*.o $(PROG) $(TEST)

dist:
	tar -czf $(PROG)-$(VERSION).tar.gz --transform s::$(PROG)-$(VERSION)/: \
		README COPYING config.mk.example makefile */*.txt */*.1 */*.c */*.h

install: all
	install -m 755 -D $(PROG) $(DESTDIR)$(PREFIX)/bin/$(PROG)
	install -m 644 -D doc/$(PROG).1 $(DESTDIR)$(MANPREFIX)/man1/$(PROG).1

uninstall:
	rm -f $(DESTDIR)$(PREFIX)/bin/$(PROG)
	rm -f $(DESTDIR)$(MANPREFIX)/man1/$(PROG).1

memcheck: $(PROG)
	valgrind --log-file=valgrind.out \
	         --track-origins=yes     \
	         --leak-check=full       \
	         --leak-resolution=high  \
	         ./$(PROG)

# Rules
$(PROG): $(PROG_SRC:%=src/%.o) $(VIEWS:%=views/%.o) $(CALS:%=cals/%.o)
	$(GCC) $(CFLAGS) -o $@ $+ $(LDFLAGS)

$(TEST): $(TEST_SRC:%=src/%.o) $(CALS:%=cals/%.o)
	$(GCC) $(CFLAGS) -o $@ $+ $(LDFLAGS)

%.o: %.c $(wildcard src/*.h makefile config.mk)
	$(GCC) $(CFLAGS) $(CPPFLAGS) -c -o $@ $<

.PHONY: all clean dist install uninstall

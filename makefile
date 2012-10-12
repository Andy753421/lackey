-include config.mk

# Settings
CC       ?= gcc
CFLAGS   ?= -Wall --std=c99
CPPFLAGS ?= -Isrc
LDFLAGS  ?= -lncursesw

# Sources
PROG      = lackey
TEST      = test
SOURCES   = main screen event util
TESTS     = test util
VIEWS     = day week month year todo notes settings help
CALS      = dummy

# Targets
all: $(PROG) $(TEST)

run-$(PROG): $(PROG)
	@urxvt -e ./$<
	@cat /tmp/lackey.log

run-$(TEST): $(TEST)
	./$<

clean:
	rm -f src/*.o view/*.o $(PROG) $(TEST) 

# Rules
$(PROG): $(SOURCES:%=src/%.o) $(VIEWS:%=view/%.o) $(CALS:%=cal/%.o)
	$(CC) $(CFLAGS) -o $@ $+ $(LDFLAGS)

$(TEST): $(TESTS:%=src/%.o) $(VIEWS:%=view/%.o) $(CALS:%=cal/%.o)
	$(CC) $(CFLAGS) -o $@ $+ $(LDFLAGS)

%.o: %.c $(SOURCES:%=src/%.h) makefile
	$(CC) $(CFLAGS) $(CPPFLAGS) -c -o $@ $<

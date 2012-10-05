-include config.mk

# Settings
CC       ?= gcc
CFLAGS   ?= -Wall --std=c99
CPPFLAGS ?= -Isrc
LDFLAGS  ?= -lncursesw

# Sources
PROG      = lackey
TEST      = test
SOURCES   = main screen util
TESTS     = test util
VIEWS     = day week month year todo notes settings help

# Targets
all: $(PROG) $(TEST)

run-$(PROG): $(PROG)
	@urxvt -e ./$<
	@cat lackey.log

run-$(TEST): $(TEST)
	./$<

clean:
	rm -f src/*.o view/*.o $(PROG) $(TEST) 

# Rules
$(PROG): $(SOURCES:%=src/%.o) $(VIEWS:%=view/%.o)
	$(CC) $(CLFAGS) -o $@ $+ $(LDFLAGS)

$(TEST): $(TESTS:%=src/%.o) $(VIEWS:%=view/%.o)
	$(CC) $(CLFAGS) -o $@ $+ $(LDFLAGS)

%.o: %.c $(SOURCES:%=%.h) makefile
	$(CC) $(CFLAGS) $(CPPFLAGS) -c -o $@ $<

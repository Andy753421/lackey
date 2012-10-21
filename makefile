-include config.mk

# Settings
CC       ?= gcc
CFLAGS   ?= -Wall --std=c99
CPPFLAGS ?= -Isrc
LDFLAGS  ?= -lncursesw -lical

# Sources
PROG      = lackey
PROG_SRC  = main view date cal util 
TEST      = test
TEST_SRC  = test date util
VIEWS     = day week month year events todo settings help
CALS      = dummy ical

# Targets
all: $(PROG) $(TEST)

run-$(PROG): $(PROG)
	@urxvt -e ./$<
	@cat /tmp/lackey.log

run-$(TEST): $(TEST)
	./$<

clean:
	rm -f src/*.o views/*.o cals/*.o $(PROG) $(TEST) 

# Rules
$(PROG): $(PROG_SRC:%=src/%.o) $(VIEWS:%=views/%.o) $(CALS:%=cals/%.o)
	$(CC) $(CFLAGS) -o $@ $+ $(LDFLAGS)

$(TEST): $(TEST_SRC:%=src/%.o) $(CALS:%=cals/%.o)
	$(CC) $(CFLAGS) -o $@ $+ $(LDFLAGS)

%.o: %.c $(wildcard src/*.h) makefile
	$(CC) $(CFLAGS) $(CPPFLAGS) -c -o $@ $<

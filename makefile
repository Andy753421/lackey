-include config.mk

# Settings
CC       ?= gcc
CFLAGS   ?= -Wall --std=c99
CPPFLAGS ?= -Isrc
LDFLAGS  ?= -lncursesw -lical

# Sources
PROG      = lackey
PROG_SRC  = main screen date event util 
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
	rm -f src/*.o view/*.o cal/*.o $(PROG) $(TEST) 

# Rules
$(PROG): $(PROG_SRC:%=src/%.o) $(VIEWS:%=view/%.o) $(CALS:%=cal/%.o)
	$(CC) $(CFLAGS) -o $@ $+ $(LDFLAGS)

$(TEST): $(TEST_SRC:%=src/%.o) $(CALS:%=cal/%.o)
	$(CC) $(CFLAGS) -o $@ $+ $(LDFLAGS)

%.o: %.c $(wildcard src/*.h) makefile
	$(CC) $(CFLAGS) $(CPPFLAGS) -c -o $@ $<

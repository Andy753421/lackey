# Settings
CC       = gcc
CFLAGS   = -Wall --std=c99
CPPFLAGS = -I.
LDFLAGS  = -lncursesw
PROG     = acal
TEST     = test

# Views
SOURCES  = main screen util
TESTS    = test util
VIEWS    = day week month year todo notes settings help

default: run-$(PROG)

# Targets
all: $(PROG) $(TEST)

run-$(PROG): $(PROG)
	@urxvt -e ./$<
	@cat acal.log

run-$(TEST): $(TEST)
	./$<

clean:
	rm -f *.o view/*.o $(PROG) $(TEST)

# Rules
$(PROG): $(SOURCES:%=%.o) $(VIEWS:%=view/%.o)
	$(CC) $(CLFAGS) -o $@ $+ $(LDFLAGS)

$(TEST): $(TESTS:%=%.o) $(VIEWS:%=view/%.o)
	$(CC) $(CLFAGS) -o $@ $+ $(LDFLAGS)

%.o: %.c $(SOURCES:%=%.h) makefile
	$(CC) $(CFLAGS) $(CPPFLAGS) -c -o $@ $<

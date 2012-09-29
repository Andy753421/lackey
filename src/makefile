# Settings
CC       = gcc
CFLAGS   = -Wall --std=c99
CPPFLAGS =
LDFLAGS  = -lncursesw
PROG     = acal

# Views
SOURCES  = main screen
VIEWS    = day week month year todo notes settings help

default: test

# Targets
all: $(PROG)

test: $(PROG)
	@urxvt -e ./$<
	@cat acal.log

clean:
	rm -f *.o view/*.o $(PROG)

# Rules
$(PROG): $(SOURCES:%=%.o) $(VIEWS:%=view/%.o)
	$(CC) $(CLFAGS) -o $@ $+ $(LDFLAGS)

%.o: %.c $(SOURCES:%=%.h) makefile
	$(CC) $(CFLAGS) $(CPPFLAGS) -c -o $@ $<

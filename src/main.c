#define _POSIX_C_SOURCE 1
#include <stdarg.h>
#include <stdlib.h>
#include <signal.h>
#include <ncurses.h>

#include "main.h"
#include "screen.h"

/* Global data */
int win_rows = 0;
int win_cols = 0;

/* Static data */
static WINDOW *win = NULL;
static FILE *debug_fd = NULL;

/* Control-C handler, so we don't hose the therminal */
static void on_sigint(int signum)
{
	endwin();
	debug("got sigint\n");
	exit(0);
}

/* Window change */
static void update(void)
{
	getmaxyx(win, win_rows, win_cols);
	win_rows++;
	win_cols++;
	screen_draw();
}

/* Window change */
static void on_sigwinch(int signum)
{
	endwin();
	refresh();
	update();
}

/* Debugging functions */
int debug(char *fmt, ...)
{
	int rval;
	va_list ap;
	va_start(ap, fmt);
	vfprintf(debug_fd, "debug: ", ap);
	rval = vfprintf(debug_fd, fmt, ap);
	va_end(ap);
	return rval;
}

/* Main */
int main(int argc, char **argv)
{
	/* Misc setup */
	debug_fd = fopen("acal.log", "w+");
	struct sigaction act;
	sigemptyset(&act.sa_mask);
	act.sa_flags   = 0;
	act.sa_handler = on_sigint;
	if (sigaction(SIGINT, &act, NULL) < 0)
		debug("sigint error\n");
	act.sa_handler = on_sigwinch;
	if (sigaction(SIGWINCH, &act, NULL) < 0)
		debug("sigwinch error\n");

	/* Curses setup */
	win = initscr();
	cbreak();
	noecho();
	start_color();
	curs_set(false);
	screen_init();

	/* Run */
	while (1) {
		int chr = getch();
		if (chr == 'q')
			break;
		switch (chr) {
			case 'L':
				clear();
			case 'l':
				update();
				break;
			default:
				screen_run(chr);
				break;
		}
	}

	/* Cleanup, see also on_sigint */
	endwin();
	debug("cleanup");
	return 0;
}

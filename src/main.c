#define _POSIX_C_SOURCE 1
#include <stdarg.h>
#include <stdlib.h>
#include <signal.h>
#include <ncurses.h>

#include "main.h"
#include "screen.h"

/* Static data */
static FILE *debug_fd = NULL;

/* Control-C handler, so we don't hose the therminal */
static void on_sigint(int signum)
{
	endwin();
	debug("got sigint\n");
	exit(0);
}

/* Window change */
static void on_sigwinch(int signum)
{
	endwin();
	refresh();
	screen_draw();
}

/* Debugging functions */
int debug(char *fmt, ...)
{
	int rval;
	va_list ap;

	/* Log to debug file */
	va_start(ap, fmt);
	vfprintf(debug_fd, "debug: ", ap);
	rval = vfprintf(debug_fd, fmt, ap);

	/* Log to status bar */
	va_start(ap, fmt);
	mvhline(LINES-2, 0, ACS_HLINE, COLS);
	move(LINES-1, 0);
	attron(COLOR_PAIR(COLOR_ERROR));
	vwprintw(stdscr, fmt, ap);
	attroff(COLOR_PAIR(COLOR_ERROR));
	clrtoeol();

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
	initscr();
	cbreak();
	noecho();
	keypad(stdscr, TRUE);
	start_color();
	curs_set(false);
	mousemask(ALL_MOUSE_EVENTS, NULL);
	init_pair(COLOR_TITLE, COLOR_GREEN, COLOR_BLACK);
	init_pair(COLOR_ERROR, COLOR_RED,   COLOR_BLACK);
	screen_init();

	/* Run */
	while (1) {
		MEVENT btn;
		int chr = getch();
		if (chr == KEY_MOUSE)
			if (getmouse(&btn) != OK)
				continue;
		if (chr == 'q')
			break;
		if (KEY_MOUSE)
			//debug("mouse xyz=%d,%d,%d id=%hd state=%lx\n",
			//	btn.x, btn.y, btn.z, btn.id, btn.bstate);
		switch (chr) {
			case 'L':
				clear();
			case 'l':
				screen_draw();
				break;
			default:
				screen_run(chr, btn.bstate, btn.y, btn.x);
				break;
		}
	}

	/* Cleanup, see also on_sigint */
	endwin();
	debug("cleanup");
	return 0;
}

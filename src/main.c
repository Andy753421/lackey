/*
 * Copyright (C) 2012 Andy Spencer <andy753421@gmail.com>
 * 
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#define _POSIX_C_SOURCE 1
#include <stdarg.h>
#include <stdlib.h>
#include <signal.h>
#include <time.h>
#include <ncurses.h>

#include "main.h"
#include "screen.h"

/* Debugging */
int YEAR  = 2012;
int MONTH = 8;
int DAY   = 29;

/* Static data */
static FILE *debug_fd = NULL;

/* Control-C handler, so we don't hose the therminal */
static void on_sigint(int signum)
{
	endwin();
	exit(0);
}

/* Window change */
static void on_sigwinch(int signum)
{
	endwin();
	refresh();
	screen_resize();
	screen_draw();
}

/* Debugging functions */
int debug(char *fmt, ...)
{
	int rval;
	va_list ap;

	/* Open log file */
	if (!debug_fd)
		debug_fd = fopen("lackey.log", "w+");

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
	struct sigaction act;
	sigemptyset(&act.sa_mask);
	act.sa_flags   = 0;
	act.sa_handler = on_sigint;
	sigaction(SIGINT, &act, NULL);
	act.sa_handler = on_sigwinch;
	sigaction(SIGWINCH, &act, NULL);

	/* Time setup */
	time_t sec = time(NULL);
	struct tm *tm = localtime(&sec);
	YEAR  = tm->tm_year+1900;
	MONTH = tm->tm_mon;
	DAY   = tm->tm_mday-1;

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
	screen_draw();

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
	return 0;
}

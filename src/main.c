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

#include <stdarg.h>
#include <stdlib.h>
#include <signal.h>
#include <time.h>
#include <locale.h>
#include <ncurses.h>

#include "main.h"
#include "screen.h"

/* Debugging */
year_t   YEAR   = 2012;
month_t  MONTH  = 8;
day_t    DAY    = 29;

event_t *EVENTS = NULL;

/* Static data */
static FILE *debug_fd = NULL;

/* Control-C handler, so we don't hose the therminal */
static void on_sigint(int signum)
{
	endwin();
	exit(0);
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
	signal(SIGINT, on_sigint);
	debug_fd = fopen("/tmp/lackey.log", "w+");

	/* Time setup */
	time_t sec = time(NULL);
	struct tm *tm = localtime(&sec);
	YEAR   = tm->tm_year+1900;
	MONTH  = tm->tm_mon;
	DAY    = tm->tm_mday-1;

	EVENTS = event_get(2012, JAN, 0, 366);

	/* Curses setup */
	setlocale(LC_ALL, "");
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

	/* Debug */
	for (event_t *e = EVENTS; e; e = e->next)
		debug("event: %04d-%02d-%02d %02d:%02d: %s - %s\n",
				e->start.year, e->start.month, e->start.day,
				e->start.hour, e->start.min, e->name, e->desc);

	/* Run */
	while (1) {
		MEVENT btn;
		int chr = getch();
		if (chr == 'q')
			break;
		if (chr == KEY_MOUSE)
			if (getmouse(&btn) != OK)
				continue;
		switch (chr) {
			case ERR:
				continue;
			case KEY_RESIZE:
				endwin();
				refresh();
				screen_resize();
				screen_draw();
				continue;
			case '\14':
				clear();
			case '\7':
				screen_draw();
				continue;
		}
		if (screen_run(chr, btn.bstate, btn.y, btn.x))
			continue;
		//debug("Unhandled key: Dec %3d,  Hex %02x,  Oct %03o,  Chr <%c>\n",
		//		chr, chr, chr, chr);
	}

	/* Cleanup, see also on_sigint */
	endwin();
	return 0;
}

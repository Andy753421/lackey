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

#include <stdlib.h>
#include <signal.h>
#include <locale.h>
#include <ncurses.h>

#include "util.h"
#include "date.h"
#include "cal.h"
#include "view.h"

/* Control-C handler, so we don't hose the therminal */
static void on_sigint(int signum)
{
	endwin();
	exit(0);
}

/* Main */
int main(int argc, char **argv)
{
	/* Misc setup */
	signal(SIGINT, on_sigint);

	/* Setup Curses */
	setlocale(LC_ALL, "");
	initscr();
	cbreak();
	noecho();
	keypad(stdscr, TRUE);
	start_color();
	curs_set(false);
	use_default_colors();
	mousemask(ALL_MOUSE_EVENTS, NULL);

	init_pair(COLOR_TITLE, COLOR_GREEN,   -1);
	init_pair(COLOR_ERROR, COLOR_RED,     -1);

	init_pair(COLOR_NEW,   COLOR_RED,     -1);
	init_pair(COLOR_WIP,   COLOR_YELLOW,  -1);
	init_pair(COLOR_DONE,  COLOR_GREEN,   -1);

	init_pair(COLOR_CLASS, COLOR_BLUE,    -1);
	init_pair(COLOR_EC,    COLOR_GREEN,   -1);
	init_pair(COLOR_WORK,  COLOR_MAGENTA, -1);
	init_pair(COLOR_OTHER, COLOR_RED,     -1);

	/* Initialize */
	util_init();
	date_init();
	cal_init();
	view_init();

	/* Draw initial view */
	view_draw();

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
				view_resize();
				view_draw();
				continue;
			case '\14':
				clear();
			case '\7':
				view_draw();
				continue;
		}
		if (view_run(chr, btn.bstate, btn.y, btn.x))
			continue;
		debug("main: Unhandled key - Dec %3d,  Hex %02x,  Oct %03o,  Chr <%c>\n",
				chr, chr, chr, chr);
	}

	/* Cleanup, see also on_sigint */
	endwin();
	return 0;
}

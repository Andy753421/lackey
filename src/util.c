/*
#include <string.h>
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ncurses.h>

#include "date.h"
#include "event.h"
#include "screen.h"

/* Static data */
static FILE *debug_fd = NULL;

/* Initialize */
void util_init(void)
{
	debug_fd = fopen("/tmp/lackey.log", "w+");
}

/* Debugging functions */
int debug(char *fmt, ...)
{
	int rval;
	va_list ap;

	/* Log to debug file */
	if (debug_fd) {
		va_start(ap, fmt);
		vfprintf(debug_fd, "debug: ", ap);
		rval = vfprintf(debug_fd, fmt, ap);
	}

	/* Log to status bar */
	if (stdscr) {
		va_start(ap, fmt);
		mvhline(LINES-2, 0, ACS_HLINE, COLS);
		move(LINES-1, 0);
		attron(COLOR_PAIR(COLOR_ERROR));
		vwprintw(stdscr, fmt, ap);
		attroff(COLOR_PAIR(COLOR_ERROR));
		clrtoeol();
	}

	va_end(ap);
	return rval;
}

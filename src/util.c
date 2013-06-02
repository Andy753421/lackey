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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ncurses.h>

#include "date.h"
#include "cal.h"
#include "view.h"
#include "util.h"

/* Static data */
static FILE *debug_fd = NULL;

/* Helper functions */
static void message(FILE *output_fd, const char *prefix, const char *fmt, va_list ap)
{
	va_list tmp;

	/* Log to standard out */
	if (output_fd) {
		va_copy(tmp, ap);
		fprintf(output_fd, "%s: ", prefix);
		vfprintf(output_fd, fmt, tmp);
		fprintf(output_fd, "\n");
	}

	/* Log to debug file */
	if (debug_fd) {
		va_copy(tmp, ap);
		fprintf(debug_fd, "%s: ", prefix);
		vfprintf(debug_fd, fmt, tmp);
		fprintf(debug_fd, "\n");
	}

	/* Log to status bar */
	if (stdscr) {
		va_copy(tmp, ap);
		mvhline(LINES-2, 0, ACS_HLINE, COLS);
		move(LINES-1, 0);
		attron(COLOR_PAIR(COLOR_ERROR));
		vwprintw(stdscr, fmt, tmp);
		attroff(COLOR_PAIR(COLOR_ERROR));
		clrtoeol();
	}
}

/* Initialize */
void util_init(void)
{
	debug_fd = fopen("/tmp/lackey.log", "w+");
}

/* String functions */
void strsub(char *str, char find, char repl)
{
	for (char *cur = str; *cur; cur++)
		if (*cur == find)
			*cur = repl;
}

/* Debugging functions */
void debug(char *fmt, ...)
{
	va_list ap;
	va_start(ap, fmt);
	message(NULL, "debug", fmt, ap);
	va_end(ap);
}

void error(char *fmt, ...)
{
	va_list ap;
	va_start(ap, fmt);
	fflush(stdout);
	fflush(stderr);
	message(stderr, "error", fmt, ap);
	va_end(ap);
	if (stdscr) {
		getch();
		endwin();
	}
	exit(-1);
}

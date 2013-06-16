/*
 * Copyright (C) 2012-2013 Andy Spencer <andy753421@gmail.com>
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

#define _XOPEN_SOURCE
#define _XOPEN_SOURCE_EXTENDED

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ncurses.h>

#include "date.h"
#include "cal.h"
#include "view.h"
#include "util.h"

/* For testing */
#pragma weak COMPACT

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
	if (&COMPACT && stdscr) {
		int rev = COMPACT ? A_BOLD : 0;
		va_copy(tmp, ap);
		if (!COMPACT)
			mvhline(LINES-2, 0, ACS_HLINE, COLS);
		move(LINES-1, 0);
		attron(COLOR_PAIR(COLOR_ERROR) | rev);
		vwprintw(stdscr, fmt, tmp);
		attroff(COLOR_PAIR(COLOR_ERROR) | rev);
		if (!COMPACT)
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

char *strcopy(const char *str)
{
	if (str == NULL)
		return NULL;
	return strdup(str);
}

int match(const char *a, const char *b)
{
	if (a == b)
		return 1;
	if (!a || !b)
		return 0;
	return !strcmp(a, b);
}

/* Memory functions */
void *alloc0(int size)
{
	void *data = calloc(1, size);
	if (!data)
		error("memory allocation failed");
	return data;
}

/* File functions */
char *read_file(const char *path, int *len)
{
	/* we could use stat, but we'll try to be portable */
	FILE *fd = fopen(path, "rt+");
	if (!fd)
		return NULL;

	int   block = 512; // read size
	int   size  = 512; // buffer size
	int   slen  = 0;   // string length
	char *buf   = malloc(size);
	if (!buf)
		goto err;

	while (!feof(fd)) {
		if (slen + block + 1 > size) {
			size *= 2;
			buf   = realloc(buf, size);
			if (!buf)
				goto err;
		}
		slen += fread(&buf[slen], 1, block, fd);
		buf[slen] = '\0';
	}

err:
	if (len)
		*len = slen;
	fclose(fd);
	return buf;
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

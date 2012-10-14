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

#define _XOPEN_SOURCE_EXTENDED

#include <string.h>
#include <ncurses.h>

#include "util.h"
#include "date.h"
#include "event.h"
#include "screen.h"

/* Static data */
static int     line;
static WINDOW *win;
static WINDOW *head;
static WINDOW *times;
static WINDOW *body;

/* Day init */
void day_init(WINDOW *_win)
{
	win   = _win; //  lines  cols  y  x
	head  = derwin(win,         1, COLS,   0, 0);
	times = derwin(win, LINES-2-2,      5, 2, 0);
	body  = derwin(win, LINES-2-2, COLS-6, 2, 6);
	line  = 10*4; // 10:00
}

/* Day size */
void day_size(int rows, int cols)
{
	wresize(head,       1, cols  );
	wresize(times, rows-2,      5);
	wresize(body,  rows-2, cols-6);
}

/* Day draw */
void day_draw(void)
{
	const char *mstr = month_to_string(MONTH);
	const char *dstr = day_to_string(day_of_week(YEAR, MONTH, DAY));

	/* Print Header */
	mvwprintw(head, 0, 0, "%s, %s %d", dstr, mstr, DAY+1);
	mvwprintw(head, 0, COLS-10, "%d-%02d-%02d", YEAR, MONTH, DAY+1);

	/* Print times */
	mvwprintw(times, 0, 0, "%02d:%02d", ((line/4)-1)%12+1, (line*15)%60);
	for (int h = 0; h < 24; h++)
		mvwprintw(times, h*4-line, 0, "%02d:%02d", (h-1)%12+1, 0);

	/* Print events */
	event_t *event = EVENTS;
	for (int h = 0; h < 24; h++)
	for (int m = 0; m < 60; m+=15)
	while (event && before(&event->start,
			YEAR, MONTH, DAY, h+(m+15)/60, (m+15)%60)) {
		if (!before(&event->start, YEAR, MONTH, DAY, h, m)) {
			int y = h*4 + m/15 - line;
			int x = 0;
			int h = (get_mins(&event->start, &event->end)-1)/15+1;
			int w = COLS-6;
			event_box(body, event, y, x, h, w);
		}
		event = event->next;
	}

	/* Print lines */
	mvwhline(win, 1, 0, ACS_HLINE, COLS);
	mvwvline(win, 2, 5, ACS_VLINE, LINES-4);
}

/* Day run */
int day_run(int key, mmask_t btn, int row, int col)
{
	int days = 0, ref = 0;
	switch (key)
	{
		case 'h': ref = 1; days = -1; break;
		case 'l': ref = 1; days =  1; break;
		case 'i': ref = 1; days = -7; break;
		case 'o': ref = 1; days =  7; break;
		case 'k': ref = 1; line--;    break;
		case 'j': ref = 1; line++;    break;
	}
	line = CLAMP(line, 0, 24*4);
	if (days)
		add_days(&YEAR, &MONTH, &DAY, days);
	if (ref) {
		werase(win);
		day_draw();
		wrefresh(win);
	}
	return 0;
}

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

#include <string.h>
#include <ncurses.h>

#include "util.h"
#include "date.h"
#include "cal.h"
#include "view.h"

/* Static data */
static WINDOW *win;
static int     line;
static int     rows;

/* Events init */
void events_init(WINDOW *_win)
{
	win = _win;
}

/* Events size */
void events_size(int rows, int cols)
{
}

/* Events draw */
void events_draw(void)
{
	int days = 2*7;
	int min  = 12;

	date_t start = {SEL.year, SEL.month, SEL.day,    0, 0};
	date_t cur   = {SEL.year, SEL.month, SEL.day-1,  0, 0};
	date_t end   = {SEL.year, SEL.month, SEL.day,   24, 0};
	add_days(&end.year, &end.month, &end.day, days);
	cal_load(SEL.year, SEL.month, SEL.day, days);

	int      row   = 0;
	int      count = 0;
	event_t *event = EVENTS;
	while (event && (count < min || compare(&event->start, &end) < 0)) {
		if (compare(&start, &event->start) <= 0) {
			date_t next = event->start;
			int newdate = cur.year  != next.year  ||
			              cur.month != next.month ||
			              cur.day   != next.day   ;
			int newtime = cur.hour  != next.hour  ||
			              cur.min   != next.min   ;

			/* Update events */
			if (EVENT == NULL || compare(&EVENT->start, &start) < 0)
				EVENT = event;

			/* Spacing before the date */
			if ((newdate || newtime) && row != 0)
				row++;
			if (newdate && row != 0)
				row++;

			/* Print date */
			if (newdate) {
				wday_t wday = day_of_week(next.year, next.month, next.day);
				wattron(win, A_BOLD);
				mvwprintw(win, row-line, 0,  "%04d-%02d-%02d",
					next.year, next.month+1, next.day+1);
				mvwprintw(win, row-line, 13, "%s, %s %d",
					day_to_string(wday), month_to_string(next.month), next.day+1);
				wattroff(win, A_BOLD);
				row++;
			}

			/* Print event info */
			event_line(win, event, row++-line, 4, COLS-4,
					SHOW_DETAILS | SHOW_ACTIVE);
			if (event->name && event->desc) {
				int n = MIN(COLS-14, strcspn(event->desc, "\n"));
				mvwprintw(win, row++-line, 14, "%.*s", n, event->desc);
			}

			cur = next;
			count += 1;
		}
		event  = event->next;
	}
	rows = row;
}

/* Events run */
int events_run(int key, mmask_t btn, int row, int col)
{
	int scroll = 0, move = 0;
	switch (key)
	{
		case 'g':    scroll = -line;    break;
		case 'G':    scroll =  rows;    break;
		case '\005': scroll =  1;       break; // ctrl-e
		case '\031': scroll = -1;       break; // ctrl-y
		case 'd':    scroll =  LINES/2; break;
		case 'u':    scroll = -LINES/2; break;
		case 'D':    scroll =  LINES;   break;
		case 'U':    scroll = -LINES;   break;
		case 'j':    move   =  1;       break;
		case 'k':    move   = -1;       break;
		case 'e':    view_edit(EDIT_EVENT); return 1;
		case '\012': view_edit(EDIT_EVENT); return 1; // enter
	}
	line   = CLAMP(line+scroll, 0, rows-1);
	for (int i=0; i<move && EVENT && EVENT->next; i++)
		EVENT = EVENT->next;
	for (int i=0; i>move && EVENT && EVENT->prev; i--)
		EVENT = EVENT->prev;
	if (scroll || move) {
		werase(win);
		events_draw();
		wrefresh(win);
	}
	return scroll || move;
}

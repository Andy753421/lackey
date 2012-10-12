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

#include <ncurses.h>

#include "util.h"
#include "date.h"
#include "event.h"

/* Static data */
static WINDOW *win;

/* Local functions */
static void print_event(event_t *event, wday_t day, hour_t hour, min_t min, float hstep)
{
	int x = 6+ROUND(day*hstep);
	int y = 3+hour*4;
	int l = (event->end.min - event->start.min)/15;
	mvwprintw(win, y, x, "%s", event->name);
	debug("week: event = %s\n", event->name);
	(void)l;
}

static int before(date_t *start, int year, int month, int day, int hour, int min)
{
	int rval = start->year  < year  ? 1 : start->year  > year ? 0 :
	           start->month < month ? 1 : start->month > month? 0 :
	           start->day   < day   ? 1 : start->day   > day  ? 0 :
	           start->hour  < hour  ? 1 : start->hour  > hour ? 0 :
	           start->min   < min   ? 1 : start->min   > min  ? 0 : 0;
	debug("week: %04d-%02d-%02d %02d:%02d < %04d-%02d-%02d %02d:%02d == %d\n",
			start->year, start->month, start->day, start->hour, start->min,
			year, month, day, hour, min, rval);
	return rval;
}

/* Week init */
void week_init(WINDOW *_win)
{
	win = _win;
}

/* Week draw */
void week_draw(void)
{
	int x = 6;
	int y = 3;
	const float hstep = (float)(COLS-x)/7.0;

	/* Clear */
	werase(win);

	/* Get start of week */
	year_t  year  = YEAR;
	month_t month = MONTH;
	day_t   day   = DAY;
	int shift = day_of_week(year, month, day);
	add_days(&year, &month, &day, -shift);

	/* Print Header */
	mvwprintw(win, 1, 0, "%s", month_to_str(MONTH));
	for (int d = 0; d < 7; d++) {
		const char *str = hstep >= 10 ? day_to_string(d) : day_to_str(d);
		if (d == shift) wattron(win, A_BOLD);
		mvwprintw(win, 0, x+ROUND(d*hstep), "%02d/%02d", month+1, day+1);
		mvwprintw(win, 1, x+ROUND(d*hstep), "%s", str);
		if (d == shift) wattroff(win, A_BOLD);
		add_days(&year, &month, &day, 1);
	}

	/* Print times */
	hour_t start = 8;
	for (int h = 0; h < (LINES-6)/4+1; h++)
		mvwprintw(win, 3+h*4, 0,"%02d:%02d", (start+h)%12, 0);

	/* Print lines */
	mvwhline(win, y-1, 0, ACS_HLINE, COLS);
	for (int d = 0; d < 7; d++)
		mvwvline(win, y, x+ROUND(d*hstep)-1, ACS_VLINE, LINES-y-2);

	/* Print events */
	event_t *event = EVENTS;
	add_days(&year, &month, &day, -7);
	for (int d = 0; d <  7; d++) {
		for (int h = 0; h < (LINES-6)/4+1; h++) {
			for (int m = 0; m < 60; m+=15) {
				while (event && before(&event->start, year, month, day, start+h+(m+15)/60, (m+15)%60)) {
					if (!before(&event->start, year, month, day, start+h, m))
						print_event(event, d, h, m, hstep);
					event = event->next;
				}
			}
		}
		add_days(&year, &month, &day, 1);
	}

	/* Draw today */
	int l = x+ROUND((shift+0)*hstep)-1;
	int r = x+ROUND((shift+1)*hstep)-1;
	mvwhline    (win, y-1, l, ACS_BLOCK, r-l+1);
	mvwvline_set(win, y,   l, WACS_T_VLINE, LINES-y-2);
	mvwvline_set(win, y,   r, WACS_T_VLINE, LINES-y-2);
}

/* Week run */
int week_run(int key, mmask_t btn, int row, int col)
{
	int days = 0;
	switch (key)
	{
		case 'h': days = -1; break;
		case 'l': days =  1; break;
		case 'i': days = -7; break;
		case 'o': days =  7; break;
	}
	if (days) {
		add_days(&YEAR, &MONTH, &DAY, days);
		week_draw();
		wrefresh(win);
	}
	return 0;
}

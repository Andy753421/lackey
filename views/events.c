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

#define DEBUG

#include <ncurses.h>

#include <util.h>
#include <date.h>
#include <cal.h>
#include <view.h>

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

	date_t start = {YEAR, MONTH, DAY,    0, 0};
	date_t cur   = {YEAR, MONTH, DAY-1,  0, 0};
	date_t end   = {YEAR, MONTH, DAY,   24, 0};
	add_days(&end.year, &end.month, &end.day, days);

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
			if ((newdate || newtime) && row != 0)
				row++;
			if (newdate && row != 0)
				row++;
			if (newdate) {
				wday_t wday = day_of_week(next.year, next.month, next.day);
				wattron(win, A_UNDERLINE);
				mvwprintw(win, row-line, 0,  "%04d-%02d-%02d",
					next.year, next.month+1, next.day+1);
				mvwprintw(win, row-line, 13, "%s, %s %d",
					day_to_string(wday), month_to_string(next.month), next.day);
				wattroff(win, A_UNDERLINE);
				row++;
			}
			event_line(win, event, row++-line, 3, COLS-2, 1);
			if (event->name && event->desc)
				mvwprintw(win, row++-line, 13, "%s", event->desc);
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
	int scroll = 0;
	switch (key)
	{
		case 'g': scroll = -line;    break;
		case 'G': scroll =  rows;    break;
		case 'j': scroll =  1;       break;
		case 'k': scroll = -1;       break;
		case 'd': scroll =  LINES/2; break;
		case 'u': scroll = -LINES/2; break;
		case 'D': scroll =  LINES;   break;
		case 'U': scroll = -LINES;   break;
	}
	line = CLAMP(line+scroll, 0, rows-1);
	if (scroll) {
		werase(win);
		events_draw();
		wrefresh(win);
	}
	return 0;
}

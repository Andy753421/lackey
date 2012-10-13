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

/* Local functions */
static void print_event(event_t *event, wday_t day, hour_t hour, min_t min, float hstep)
{
	int x = ROUND(day*hstep);
	int y = hour*4 + min/15 - line;
	int w = ROUND((day+1)*hstep) - 1 - x;
	int h = (get_mins(&event->start, &event->end)-1)/15+1;
	int l = 0;
	int s = y < 0 ? -y-1 : 0;

	int color = event->cat == NULL           ? 0           :
	            !strcmp(event->cat, "class") ? COLOR_CLASS :
	            !strcmp(event->cat, "ec")    ? COLOR_EC    :
	            !strcmp(event->cat, "work")  ? COLOR_WORK  : COLOR_OTHER ;

	if (color) wattron(body, COLOR_PAIR(color));

        if (h <= 1) mvwadd_wch(body,   y,     x, WACS_BULLET);
	if (h >= 2) mvwadd_wch(body,   y,     x, WACS_T_ULCORNER);
	if (h >= 3) mvwvline_set(body, y+1+s, x, WACS_T_VLINE, h-2-s);
	if (h >= 2) mvwadd_wch(body,   y+h-1, x, WACS_T_LLCORNER);

	if (color) wattroff(body, COLOR_PAIR(color));

	if (l<h && event->name) mvwprintw(body, y+l++, x+1, "%-*.*s",   w-1, w-1, event->name);
	if (l<h && event->loc)  mvwprintw(body, y+l++, x+1, "@ %-*.*s", w-3, w-3, event->loc);
	if (l<h && event->desc) mvwprintw(body, y+l++, x+1, "%-*.*s",   w-1, w-1, event->desc);

	debug("week: event = %s\n", event->name);
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
	win   = _win; //  lines  cols  y  x
	head  = derwin(win,         2, COLS,   0, 0);
	times = derwin(win, LINES-2-3,      5, 3, 0);
	body  = derwin(win, LINES-2-3, COLS-6, 3, 6);
	line  = 10*4; // 10:00
}

/* Week size */
void week_size(int rows, int cols)
{
	wresize(head,       2, cols  );
	wresize(times, rows-3,      5);
	wresize(body,  rows-3, cols-6);
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
	mvwprintw(head, 0, 0, "%s",   month_to_str(MONTH));
	mvwprintw(head, 1, 0, "%04d", YEAR);
	for (int d = 0; d < 7; d++) {
		const char *str = hstep >= 10 ? day_to_string(d) : day_to_str(d);
		if (d == shift) wattron(head, A_BOLD);
		mvwprintw(head, 0, x+ROUND(d*hstep), "%s", str);
		mvwprintw(head, 1, x+ROUND(d*hstep), "%02d/%02d", month+1, day+1);
		if (d == shift) wattroff(head, A_BOLD);
		add_days(&year, &month, &day, 1);
	}

	/* Print times */
	mvwprintw(times, 0, 0, "%02d:%02d", ((line/4)-1)%12+1, (line*15)%60);
	for (int h = 0; h < 24; h++)
		mvwprintw(times, h*4-line, 0, "%02d:%02d", (h-1)%12+1, 0);

	/* Print events */
	event_t *event = EVENTS;
	add_days(&year, &month, &day, -7);
	for (int d = 0; d <  7; d++, add_days(&year,&month,&day,1))
	for (int h = 0; h < 24; h++)
	for (int m = 0; m < 60; m+=15)
	while (event && before(&event->start,
			year, month, day, h+(m+15)/60, (m+15)%60)) {
		if (!before(&event->start, year, month, day, h, m))
			print_event(event, d, h, m, hstep);
		event = event->next;
	}

	/* Print lines */
	mvwhline(win, y-1, 0, ACS_HLINE, COLS);
	for (int d = 0; d < 7; d++)
		mvwvline(win, y, x+ROUND(d*hstep)-1, ACS_VLINE, LINES-y-2);

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
		week_draw();
		wrefresh(win);
	}
	return 0;
}

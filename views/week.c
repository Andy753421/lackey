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
#include "cal.h"
#include "view.h"

/* Static data */
static int     line;
static WINDOW *win;
static WINDOW *times;
static WINDOW *body;

/* Week init */
void week_init(WINDOW *_win)
{
	win   = _win; //    lines    cols    y  x
	times = derwin(win, LINES-2,      5, 0, 0);
	body  = derwin(win, LINES-2, COLS-5, 0, 5);
	line  = 10*4; // 10:00
}

/* Week size */
void week_size(int rows, int cols)
{
	int hdr = 3-COMPACT;
	wmvresize(times, hdr, 0, rows-hdr,      5);
	wmvresize(body,  hdr, 5, rows-hdr, cols-5);
}

/* Week draw */
void week_draw(void)
{
	int x = 6;
	int y = 3 - COMPACT;
	const float hstep = (float)(COLS-x)/7.0;
	event_t *event;

	/* Get start of week */
	year_t  year  = YEAR;
	month_t month = MONTH;
	day_t   day   = DAY;
	int shift = day_of_week(year, month, day);
	add_days(&year, &month, &day, -shift);

	/* Load cal data */
	cal_load(year, month, day, 7);

	/* For today */
	int l = ROUND((shift+0)*hstep);
	int r = ROUND((shift+1)*hstep);

	/* Print Header */
	int rev = COMPACT ? A_REVERSE | A_BOLD : 0;
	wattron(win, rev);
	mvwprintw(win, 0, 0, "%-*s",  COLS, month_to_str(MONTH));
	mvwprintw(win, 1, 0, "%-0*d", COLS, YEAR);
	wattroff(win, rev);
	mvwhline(win, 0, x+l, ' ', r-l-1);
	mvwhline(win, 1, x+l, ' ', r-l-1);
	wattron(win, rev);
	for (int d = 0; d < 7; d++) {
		const char *str = hstep >= 10 ? day_to_string(d) : day_to_str(d);
		if (d == shift) wattrset(win, A_BOLD);
		mvwprintw(win, 0, x+ROUND(d*hstep), "%s", str);
		mvwprintw(win, 1, x+ROUND(d*hstep), "%02d/%02d", month+1, day+1);
		if (d == shift) wattrset(win, rev);
		add_days(&year, &month, &day, 1);
	}
	wattroff(win, rev);

	/* Print all day events */
	int allday = 0;
	event = EVENTS;
	add_days(&year, &month, &day, -7);
	for (int d = 0; d <  7; d++) {
		int n = 0;
		while (event && before(&event->start, year, month, day, 24, 0)) {
			if (!before(&event->end, year, month, day, 0, 1) &&
			    get_mins(&event->start, &event->end) > 23*60) {
				int s = ROUND(d*hstep);
				int w = ROUND((d+1)*hstep) - 1 - s;
				event_line(win, event, y+n++, x+s, w, 0);
			}
			event = event->next;
			if (n > allday)
				allday = n;
		}
		add_days(&year,&month,&day,1);
	}
	if (allday && !COMPACT)
		allday++;

	/* Resize body */
	wshrink(times, y+allday-!COMPACT);
	wshrink(body,  y+allday-!COMPACT);

	/* Print times */
	mvwprintw(times, !COMPACT, 0, "%02d:%02d", ((line/4)-1)%12+1, (line*15)%60);
	for (int h = 0; h < 24; h++)
		mvwprintw(times, !COMPACT+h*4-line, 0, "%02d:%02d", (h-1)%12+1, 0);

	/* Print events */
	event = EVENTS;
	add_days(&year, &month, &day, -7);
	for (int d = 0; d <  7; d++, add_days(&year,&month,&day,1))
	for (int h = 0; h < 24; h++)
	for (int m = 0; m < 60; m+=15)
	while (event && before(&event->start,
			year, month, day, h+(m+15)/60, (m+15)%60)) {
		if (!before(&event->start, year, month, day, h, m) &&
		    get_mins(&event->start, &event->end) <= 23*60) {
			int y = h*4 + m/15 - line + !COMPACT;
			int x = ROUND(d*hstep) + 1;
			int h = (get_mins(&event->start, &event->end)-1)/15+1;
			int w = ROUND((d+1)*hstep) - x;
			event_box(body, event, y, x, h, w);
		}
		event = event->next;
	}

	/* Print header lines */
	if (!COMPACT)
		mvwhline(win, y-1, 0, ACS_HLINE, COLS);
	if (!COMPACT && allday)
		mvwhline(win, y-1+allday, 0, ACS_HLINE, COLS);

	/* Print day lines */
	for (int d = 0; d < 7; d++)
		mvwvline(body, !COMPACT, ROUND(d*hstep), ACS_VLINE, LINES-y-2+COMPACT-allday);
	mvwvline_set(body, 0, l, WACS_T_VLINE, LINES-y-1+COMPACT);
	mvwvline_set(body, 0, r, WACS_T_VLINE, LINES-y-1+COMPACT);
	for (int h = (line+3)/4; h < 24; h++) {
		mvwadd_wch(body, h*4-line+!COMPACT, l, WACS_T_LTEE);
		mvwadd_wch(body, h*4-line+!COMPACT, r, WACS_T_RTEE);
	}
	if (!COMPACT)
		mvwhline(body, 0, l, ACS_BLOCK, r-l+1);
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
		werase(win);
		week_draw();
		wrefresh(win);
	}
	return ref;
}

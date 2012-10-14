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
static WINDOW *win;

/* Local functions */
static void print_event(event_t *event, int y, int x, int w)
{
	int color = event->cat == NULL           ? 0           :
	            !strcmp(event->cat, "class") ? COLOR_CLASS :
	            !strcmp(event->cat, "ec")    ? COLOR_EC    :
	            !strcmp(event->cat, "work")  ? COLOR_WORK  : COLOR_OTHER ;

	if (color) wattron(win, COLOR_PAIR(color));
	mvwaddch(win, y, x+0, ACS_BLOCK);
	if (color) wattroff(win, COLOR_PAIR(color));

	mvwprintw(win, y, x+1, "%-*.*s", w-1, w-1, event->name);
}

/* Month init */
void month_init(WINDOW *_win)
{
	win = _win;
}

/* Month size */
void month_size(int rows, int cols)
{
}

/* Month draw */
void month_draw(void)
{
	const char *name  = month_to_string(MONTH);
	const int   start = start_of_month(YEAR, MONTH);
	const int   days  = days_in_month(YEAR, MONTH);
	const int   weeks = weeks_in_month(YEAR, MONTH);
	const float midpt = (float)COLS/2.0 - (strlen(name) + 1 + 4)/2.0;
	const float hstep = (float)(COLS-1)/7.0;
	const float vstep = (float)(LINES-6)/weeks;

	/* Clear */
	werase(win);

	/* Print Header */
	mvwprintw(win, 0, midpt, "%s %d", name, YEAR);
	for (int d = 0; d < 7; d++) {
		const char *str = hstep >= 10 ? day_to_string(d+SUN) : day_to_str(d+SUN);
		mvwprintw(win, 1, ROUND(1+d*hstep), "%s", str);
	}
	mvwhline(win, 2, 0, ACS_HLINE, COLS);

	/* Print days */
	for (int d = 0; d < days; d++) {
		int row = (start + d) / 7;
		int col = (start + d) % 7;
		if (d == DAY) wattron(win, A_BOLD);
		mvwprintw(win, ROUND(4+row*vstep), ROUND(1+col*hstep), "%d", d+1);
		if (d == DAY) wattroff(win, A_BOLD);
	}

	/* Print events */
	event_t *event = EVENTS;
	for (int d = 0; d < days; d++) {
		int y = ROUND(4+(((start + d) / 7)  )*vstep);
		int e = ROUND(4+(((start + d) / 7)+1)*vstep)-2;
		int x = ROUND(1+(((start + d) % 7)  )*hstep)+3;
		int w = ROUND(1+(((start + d) % 7)+1)*hstep)-x-1;
		while (event && before(&event->start, YEAR, MONTH, d, 24, 0)) {
			if (!before(&event->start, YEAR, MONTH, d, 0, 0)){
				if (y == e) mvwhline(win, y, x-3, ACS_DARROW, 2);
				if (y <= e) print_event(event, y, x, w);
				y++;
			}
			event = event->next;
		}
	}

	/* Print lines */
	for (int w = 1; w < weeks; w++)
		mvwhline(win, ROUND(3+w*vstep), 1, ACS_HLINE, COLS-2);
	for (int d = 1; d < 7; d++) {
		int top = d >=  start             ? 0     : 1;
		int bot = d <= (start+days-1)%7+1 ? weeks : weeks-1;
		mvwvline(win, ROUND(4+top*vstep), ROUND(d*hstep),
				ACS_VLINE, (bot-top)*vstep);
		for (int w = 1; w < weeks; w++) {
			int chr = w == top ? ACS_TTEE :
				  w == bot ? ACS_BTEE : ACS_PLUS;
			mvwaddch(win, ROUND(3+w*vstep), ROUND(d*hstep), chr);
		}
	}

	/* Draw today */
	int col = day_of_week(YEAR, MONTH, DAY);
	int row = (start+DAY) / 7;
	int l = ROUND((col+0)*hstep);
	int r = ROUND((col+1)*hstep);
	int t = ROUND((row+0)*vstep+3);
	int b = ROUND((row+1)*vstep+3);
	mvwvline_set(win, t, l, WACS_T_VLINE, b-t);
	mvwvline_set(win, t, r, WACS_T_VLINE, b-t);
	mvwhline_set(win, t, l, WACS_T_HLINE, r-l);
	mvwhline_set(win, b, l, WACS_T_HLINE, r-l);
	mvwadd_wch(win, t, l, WACS_T_ULCORNER);
	mvwadd_wch(win, t, r, WACS_T_URCORNER);
	mvwadd_wch(win, b, l, WACS_T_LLCORNER);
	mvwadd_wch(win, b, r, WACS_T_LRCORNER);
}

/* Month run */
int month_run(int key, mmask_t btn, int row, int col)
{
	int days = 0, months = 0;
	switch (key)
	{
		case 'h': days   = -1; break;
		case 'j': days   =  7; break;
		case 'k': days   = -7; break;
		case 'l': days   =  1; break;
		case 'i': months = -1; break;
		case 'o': months =  1; break;
	}
	if (days || months) {
		add_days(&YEAR, &MONTH, &DAY, days);
		add_months(&YEAR, &MONTH, months);
		month_draw();
		wrefresh(win);
	}
	return 0;
}

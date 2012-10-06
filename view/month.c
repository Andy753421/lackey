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

#include "main.h"
#include "util.h"

/* Static data */
static WINDOW *win;

/* Month init */
void month_init(WINDOW *_win)
{
	win = _win;
}

/* Month draw */
void month_draw(void)
{
	const char *name  = month_to_string(MONTH);
	const int   start = start_of_month(YEAR, MONTH);
	const int   days  = days_in_month(YEAR, MONTH);
	const int   weeks = weeks_in_month(YEAR, MONTH);
	const float midpt = (float)COLS/2.0 - (strlen(name) + 1 + 4)/2.0;
	const float hstep = (float)COLS/7.0;
	const float vstep = (float)(LINES-4)/weeks;

	/* Clear */
	werase(win);

	/* Print Header */
	mvwprintw(win, 0, midpt, "%s %d", name, YEAR);
	for (int d = 0; d < 7; d++) {
		const char *str = hstep >= 10 ? day_to_string(d+SUN) : day_to_str(d+SUN);
		mvwprintw(win, 1, ROUND(d*hstep), "%s", str);
	}
	mvwhline(win, 2, 0, ACS_HLINE, COLS);

	/* Print days */
	for (int d = 0; d < days; d++) {
		int row = (start + d) / 7;
		int col = (start + d) % 7;
		mvwprintw(win, ROUND(3+row*vstep), ROUND(col*hstep), "%d", d+1);
	}

	/* Print lines */
	for (int w = 1; w < weeks; w++)
		mvwhline(win, ROUND(2+w*vstep), 0, ACS_HLINE, COLS);
	for (int d = 1; d < 7; d++) {
		int top = d >=  start         ? 0     : 1;
		int bot = d <= (start+days)%7 ? weeks : weeks-1;
		mvwvline(win, ROUND(3+top*vstep), ROUND(d*hstep-1),
				ACS_VLINE, (bot-top)*vstep);
		for (int w = 1; w < weeks; w++) {
			int chr = w == top ? ACS_TTEE :
				  w == bot ? ACS_BTEE : ACS_PLUS;
			mvwaddch(win, ROUND(2+w*vstep), ROUND(d*hstep-1), chr);
		}
	}

	/* Draw today */
	int col = day_of_week(YEAR, MONTH, DAY);
	int row = (start+DAY) / 7;
	int l = ROUND((col+0)*hstep-1);
	int r = ROUND((col+1)*hstep-1);
	int t = ROUND((row+0)*vstep+2);
	int b = ROUND((row+1)*vstep+2);
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

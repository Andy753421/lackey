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
}

/* Month run */
int month_run(int key, mmask_t btn, int row, int col)
{
	return 0;
}

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

#include "util.h"
#include "date.h"
#include "cal.h"
#include "view.h"

/* Constants */
#define MW (2*7+6)

/* Static data */
static WINDOW *win;

/* Helper functions */
static void print_month(month_t month, int y, int x)
{
	event_t    *event = EVENTS;
	const char *name  = month_to_string(month);
	const int   start = start_of_month(YEAR, month);
	const char  days  = days_in_month(YEAR, month);
	mvwprintw(win, y, x+MW/2-strlen(name)/2, "%s", name);
	wmove(win, y+1, x);
	for (int d = 0; d < 7; d++)
		wprintw(win, "%-3s", day_to_st(d));
	for (int d = 0; d < days; d++) {
		int row = (start + d) / 7;
		int col = (start + d) % 7;

		int busy = 0;
		while (event && before(&event->start, YEAR, month, d, 24, 0)) {
			if (!before(&event->start, YEAR, month, d, 0, 0))
				busy = 1;
			event = event->next;
		}

		int today = month == MONTH && d == DAY;

		int attr  = (busy  ? A_BOLD|A_UNDERLINE : A_DIM)
		          | (today ? A_REVERSE          : 0    );

		wattron(win, attr);
		mvwprintw(win, y+2+row, x+col*3, "%2d", d+1);
		wattroff(win, attr);
	}
}

/* Year init */
void year_init(WINDOW *_win)
{
	win = _win;
}

/* Year size */
void year_size(int rows, int cols)
{
}

/* Year draw */
void year_draw(void)
{
	int w = MW*3 + 2*3;
	int x = COLS/2 - w/2;
	int y = 0;
	int h[4] = {};

	/* Determine heights */
	for (int m = 0; m < 12; m++) {
		int weeks = weeks_in_month(YEAR, m);
		h[m/3] = MAX(h[m/3], weeks+2);
	}
	int sum = h[0]+h[1]+h[2]+h[3];

	/* Print Header */
	mvwprintw(win, y++, COLS/2-2, "%d", YEAR);

	/* Print Months */
	for (int m = 0; m < 12; m++) {
		print_month(m, y, x);
		if (m % 3 == 2) {
			x  = COLS/2 - w/2;
			y += h[m/3]+1;
		} else {
			x += 3+MW;
		}
	}

	/* Print Lines */
	y = 1;
	mvwvline(win, y, x+(MW+3)*1-2, ACS_VLINE, sum+3);
	mvwvline(win, y, x+(MW+3)*2-2, ACS_VLINE, sum+3);
	for (int i = 0; i < 3; i++) {
		y += h[i];
		mvwhline(win, y, x,        ACS_HLINE, w);
		mvwaddch(win, y, x+(MW+3)*1-2, ACS_PLUS);
		mvwaddch(win, y, x+(MW+3)*2-2, ACS_PLUS);
		y++;
	}
}

/* Year run */
int year_run(int key, mmask_t btn, int row, int col)
{
	day_t d = DAY;
	month_t m = MONTH;
	year_t y = YEAR;
	wday_t day = day_of_week(YEAR, MONTH, DAY);
	int week = (start_of_month(y, m) + d) / 7;
	int dir = 0;

	/* Step years */
	if (key == 'i')
		YEAR--;
	if (key == 'o')
		YEAR++;

	/* Get direction */
	if (key == 'h' || key == 'k')
		dir = -1;
	if (key == 'j' || key == 'l')
		dir =  1;

	/* Step up/down */
	if (key == 'j' || key == 'k') {
		for (int i = 0; i < 90/7; i++) {
			add_days(&y, &m, &d, dir*7);
			if (day_of_week(y, m, d) == day &&
			    y == YEAR && m%3 == MONTH%3) {
				MONTH = m;
				DAY = d;
				break;
			}
		}
	}

	/* Step left/right */
	if (key == 'h' || key == 'l') {
		for (int i = 0; i < 90; i++) {
			add_days(&y, &m, &d, dir);
			if ((start_of_month(y, m) + d) / 7 == week &&
			    y == YEAR && m/3 == MONTH/3) {
				MONTH = m;
				DAY = d;
				break;
			}
		}
	}

	/* Refresh */
	if (dir || y != YEAR) {
		werase(win);
		year_draw();
		wrefresh(win);
	}
	return dir || y != YEAR;
}

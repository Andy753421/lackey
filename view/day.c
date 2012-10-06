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

/* Day init */
void day_init(WINDOW *_win)
{
	win = _win;
}

/* Day draw */
void day_draw(void)
{
	const char *mstr = month_to_string(MONTH);
	const char *dstr = day_to_string(day_of_week(YEAR, MONTH, DAY));

	/* Clear */
	werase(win);

	/* Print Header */
	mvwprintw(win, 0, 0, "%s, %s %d", dstr, mstr, DAY+1);
	mvwprintw(win, 0, COLS-10, "%d-%02d-%02d", YEAR, MONTH, DAY+1);
	mvwhline(win, 1, 0, ACS_HLINE, COLS);

	/* Print times */
	int start = 8;
	for (int h = 0; h < (LINES-5)/4+1; h++)
		mvwprintw(win, 2+h*4, 0,"%02d:%02d", (start+h)%12, 0);
	mvwvline(win, 2, 5, ACS_VLINE, LINES-4);
}

/* Day run */
int day_run(int key, mmask_t btn, int row, int col)
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
		day_draw();
		wrefresh(win);
	}
	return 0;
}

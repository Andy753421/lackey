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
#include "screen.h"

/* Types */
typedef struct {
	char   *name;
	void  (*init)(WINDOW*);
	void  (*size)(int,int);
	void  (*draw)(void);
	int   (*run)(int,mmask_t,int,int);
	int     keys[8];
	WINDOW *win;
} view_t;

/* Data */
view_t views[] = {
	{ "Day",      day_init,      day_size,      day_draw,      day_run,      {KEY_F(1), '1',    } },
	{ "Week",     week_init,     week_size,     week_draw,     week_run,     {KEY_F(2), '2',    } },
	{ "Month",    month_init,    month_size,    month_draw,    month_run,    {KEY_F(3), '3',    } },
	{ "Year",     year_init,     year_size,     year_draw,     year_run,     {KEY_F(4), '4',    } },
	{ "|",        NULL,          NULL,          NULL,          NULL,         {                  } },
	{ "Todo",     todo_init,     todo_size,     todo_draw,     todo_run,     {KEY_F(5), '5',    } },
	{ "Notes",    notes_init,    notes_size,    notes_draw,    notes_run,    {KEY_F(6), '6',    } },
	{ "|",        NULL,          NULL,          NULL,          NULL,         {                  } },
	{ "Settings", settings_init, settings_size, settings_draw, settings_run, {KEY_F(7), '7',    } },
	{ "Help",     help_init,     help_size,     help_draw,     help_run,     {KEY_F(8), '8', '?'} },
};

int active = 1;

/* Local functions */
void draw_header(void)
{
	move(0, 0);
	attron(COLOR_PAIR(COLOR_TITLE));
	for (int i = 0; i < N_ELEMENTS(views); i++) {
		if (i == active)
			attron(A_BOLD);
		printw("%s ", views[i].name);
		if (i == active)
			attroff(A_BOLD);
	}
	attroff(COLOR_PAIR(COLOR_TITLE));
	mvhline(1, 0, ACS_HLINE, COLS);
	refresh();
}

/* Screen init */
void screen_init(void)
{
	for (int i = 0; i < N_ELEMENTS(views); i++) {
		if (views[i].init) {
			views[i].win = newwin(LINES-2, COLS, 2, 0);
			views[i].init(views[i].win);
		}
	}
}

/* Screen draw */
void screen_resize(void)
{
	for (int i = 0; i < N_ELEMENTS(views); i++) {
		if (views[i].win)
			wresize(views[i].win, LINES-2, COLS);
		if (views[i].size)
			views[i].size(LINES-2, COLS);
	}
}

/* Screen draw */
void screen_draw(void)
{
	draw_header();
	werase(views[active].win);
	views[active].draw();
	wrefresh(views[active].win);
}

/* Screen set */
int screen_set(int num)
{
	if (active != num) {
		active = num;
		screen_draw();
	}
	return 1;
}

/* Screen run */
int screen_run(int key, mmask_t btn, int row, int col)
{
	/* Check for mouse events */
	if (key == KEY_MOUSE && row == 0) {
		int start = 1;
		for (int i = 0; i < N_ELEMENTS(views); i++) {
			int end = start + strlen(views[i].name) - 1;
			if (start <= col && col <= end && views[i].draw)
				return screen_set(i);
			start = end + 2;
		}
	}

	/* Check for view change */
	for (int i = 0; i < N_ELEMENTS(views); i++) {
		if (i == active)
			continue;
		for (int j = 0; j < N_ELEMENTS(views[i].keys); j++)
			if (views[i].keys[j] == key)
				return screen_set(i);
	}

	/* Shift windows */
	int num   = active;
	int shift = key == KEY_RIGHT ? +1 :
		    key == KEY_LEFT  ? -1 : 0;
	while (shift) {
		num += shift;
		num += N_ELEMENTS(views);
		num %= N_ELEMENTS(views);
		if (views[num].run)
			return screen_set(num);
	}

	/* Pass key to active view */
	return views[active].run(key, btn, row, col);
}

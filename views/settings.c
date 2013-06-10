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
#include "conf.h"
#include "date.h"
#include "cal.h"
#include "view.h"

/* Constant data */
#define PAD "    "

/* Static data */
static WINDOW *win;

/* Helper functions */
static void heading(WINDOW *win, char *label)
{
	wattron(win, A_BOLD);
	wprintw(win, "%s", label);
	wattroff(win, A_BOLD);
}

static void checkbox(WINDOW *win, char *label, int *value)
{
	wprintw(win, PAD "[%c] %s", *value?'x':' ', label);
}

/* Settings init */
void settings_init(WINDOW *_win)
{
	win = _win;
}

/* Settings init */
void settings_size(int rows, int cols)
{
}

/* Settings draw */
void settings_draw(void)
{
	int wtype = 0, wname = 0;

	/* Figure out cal widths */
	for (cal_t *cal = CALS; cal; cal = cal->next) {
		if (strlen(cal->type) > wtype)
			wtype = strlen(cal->type);
		if (strlen(cal->name) > wname)
			wname = strlen(cal->name);
	}

	heading(win, "Current Settings\n");
	checkbox(win, "Compact layout\n", &COMPACT);

	heading(win, "\nLoaded Calendars\n");
	for (cal_t *cal = CALS; cal; cal = cal->next)
		wprintw(win, PAD "%s:%*s \"%s\"%*s - %s\n",
			cal->type, wtype-strlen(cal->type), "",
			cal->name, wname-strlen(cal->name), "",
			cal->desc ?: "(no description)");
}

/* Settings run */
int settings_run(int key, mmask_t btn, int row, int col)
{
	return 0;
}

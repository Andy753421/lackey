/*
 * Copyright (C) 2013 Andy Spencer <andy753421@gmail.com>
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

/* Static data */
static WINDOW *win;

/* Edit init */
void edit_init(WINDOW *_win)
{
	win = _win;
}

/* Edit init */
void edit_size(int rows, int cols)
{
}

/* Edit draw */
void edit_draw(void)
{
	mvwprintw(win, 1, 1, "Edit\n");
}

/* Edit run */
int edit_run(int key, mmask_t btn, int row, int col)
{
	return 0;
}

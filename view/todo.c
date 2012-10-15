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

#include <ncurses.h>

/* Static data */
static WINDOW *win;

/* Todo init */
void todo_init(WINDOW *_win)
{
	win = _win;
}

/* Todo size */
void todo_size(int rows, int cols)
{
}

/* Todo draw */
void todo_draw(void)
{
	mvwprintw(win, 0, 1, "%s\n", "todo");
}

/* Todo run */
int todo_run(int key, mmask_t btn, int row, int col)
{
	return 0;
}

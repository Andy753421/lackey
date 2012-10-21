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

#include "util.h"
#include "date.h"
#include "cal.h"
#include "view.h"

/* Static data */
static WINDOW *win;

static int show_new      = 1;
static int show_started  = 1;
static int show_finished = 1;

/* Helper functions */
static int print_todos(WINDOW *win, int y, todo_t *todos, status_t low, status_t high)
{
	int n = 0;
	for (todo_t *cur = todos; cur; cur = cur->next)
		if (low <= cur->status && cur->status <= high)
			todo_line(win, cur, y+n++, 2, COLS-2, 1);
	return n;
}

static int print_group(WINDOW *win, int y, todo_t *todos,
		int show, const char *label, status_t low, status_t high)
{
	int n = 1;

	/* Label */
	mvwprintw(win, y, 0, "%s", label);

	/* Todos */
	if (show)
		n = print_todos(win, y+1, todos, low, high);

	/* Status */
	if (!show)
		mvwprintw(win, y+1, 4, "[hidden]");
	if (n == 0)
		mvwprintw(win, y+1, 4, "[no tasks]");

	return y+1+MAX(n,1)+1;
}

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
	int y = 0;

	y = print_group(win, y, TODOS,
		show_new, "New Tasks", NEW, NEW);

	y = print_group(win, y, TODOS,
		show_started, "Started Tasks", NEW+1, DONE-1);

	y = print_group(win, y, TODOS,
		show_finished, "Finished Tasks", DONE, DONE);
}

/* Todo run */
int todo_run(int key, mmask_t btn, int row, int col)
{
	int ref = 0;
	switch (key)
	{
		case 'n': ref = 1; show_new      ^= 1; break;
		case 's': ref = 1; show_started  ^= 1; break;
		case 'f': ref = 1; show_finished ^= 1; break;
	}
	if (ref) {
		werase(win);
		todo_draw();
		wrefresh(win);
	}
	return 0;
}

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

/* Helper functions */
static const char *timestr(date_t date)
{
	static char buf[64];
	if (date.year)
		sprintf(buf, "%04d-%02d-%02d %02d:%02d",
			date.year, date.month+1, date.day+1,
			date.hour, date.min);
	else
		sprintf(buf, "[none]");
	return buf;
}

/* Event editing */
static void draw_event(event_t *event)
{
	wmove(win, 0, 0);
	wprintw(win, "Edit Event\n");
	wprintw(win, "    Name:        %s\n",  event->name      ?: "[none]");
	wprintw(win, "    Description: %s\n",  event->desc      ?: "[none]");
	wprintw(win, "    Location:    %s\n",  event->loc       ?: "[none]");
	wprintw(win, "    Category:    %s\n",  event->cat       ?: "[none]");
	wprintw(win, "    Start Time:  %s\n",  timestr(event->start));
	wprintw(win, "    End Time:    %s\n",  timestr(event->end));
	wprintw(win, "    Calendar:    %s\n",  event->cal->name ?: "[none]");
}

static int edit_event(event_t *event, int key, mmask_t btn, int row, int col)
{
	return 0;
}

/* Todo editing */
static void draw_todo(todo_t *todo)
{
	wmove(win, 0, 0);
	wprintw(win, "Edit Todo\n");
	wprintw(win, "    Name:        %s\n",   todo->name      ?: "[none]");
	wprintw(win, "    Description: %s\n",   todo->desc      ?: "[none]");
	wprintw(win, "    Category:    %s\n",   todo->cat       ?: "[none]");
	wprintw(win, "    Completed:   %d%%\n", todo->status);
	wprintw(win, "    Start Time:  %s\n",   timestr(todo->start));
	wprintw(win, "    Due Date:    %s\n",   timestr(todo->due));
	wprintw(win, "    Calendar:    %s\n",   todo->cal->name ?: "[none]");
}

static int edit_todo(todo_t *todo, int key, mmask_t btn, int row, int col)
{
	return 0;
}

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
	switch (EDIT) {
		case EDIT_CAL:   break;
		case EDIT_EVENT: draw_event(EVENT); break;
		case EDIT_TODO:  draw_todo(TODO);   break;
		default:         break;
	}
}

/* Edit run */
int edit_run(int key, mmask_t btn, int row, int col)
{
	switch (EDIT) {
		case EDIT_CAL:   return 1;
		case EDIT_EVENT: return edit_event(EVENT, key, btn, row, col);
		case EDIT_TODO:  return edit_todo(TODO, key, btn, row, col);
		default:         return 0;
	}
}

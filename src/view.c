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

#include "util.h"
#include "date.h"
#include "cal.h"
#include "view.h"

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

/* View data */
view_t views[] = {
	{ "Day",      day_init,      day_size,      day_draw,      day_run,      {KEY_F(1), '1',    } },
	{ "Week",     week_init,     week_size,     week_draw,     week_run,     {KEY_F(2), '2',    } },
	{ "Month",    month_init,    month_size,    month_draw,    month_run,    {KEY_F(3), '3',    } },
	{ "Year",     year_init,     year_size,     year_draw,     year_run,     {KEY_F(4), '4',    } },
	{ "|",        NULL,          NULL,          NULL,          NULL,         {                  } },
	{ "Events",   events_init,   events_size,   events_draw,   events_run,   {KEY_F(5), '5',    } },
	{ "Todo",     todo_init,     todo_size,     todo_draw,     todo_run,     {KEY_F(6), '6',    } },
	{ "|",        NULL,          NULL,          NULL,          NULL,         {                  } },
	{ "Settings", settings_init, settings_size, settings_draw, settings_run, {KEY_F(7), '7',    } },
	{ "Help",     help_init,     help_size,     help_draw,     help_run,     {KEY_F(8), '8', '?'} },
};

/* Config data */
int COMPACT = 0;
int ACTIVE  = 0;

/* Local functions */
static void draw_header(void)
{
	move(0, 0);
	attron(COLOR_PAIR(COLOR_TITLE));
	for (int i = 0; i < N_ELEMENTS(views); i++) {
		if (i == ACTIVE)
			attron(A_BOLD);
		printw("%s ", views[i].name);
		if (i == ACTIVE)
			attroff(A_BOLD);
	}
	attroff(COLOR_PAIR(COLOR_TITLE));
	if (!COMPACT)
		mvhline(1, 0, ACS_HLINE, COLS);
	refresh();
}

static int get_color(const char *cat)
{
	return cat == NULL           ? 0           :
	       match(cat, "class") ? COLOR_CLASS :
	       match(cat, "ec")    ? COLOR_EC    :
	       match(cat, "work")  ? COLOR_WORK  : COLOR_OTHER ;
}

/* Helper functions */
void event_box(WINDOW *win, event_t *event, int y, int x, int h, int w)
{
	int l = 0;
	int s = y < 0 ? -y-1 : 0;

	int color = get_color(event->cat);

	if (color) wattron(win, COLOR_PAIR(color));

	if (h >= 2) mvwhline_set(win, y,     x+1,   WACS_T_HLINE, w-2);
	if (h <= 1) mvwadd_wch(win,   y,     x,     WACS_BULLET);
	if (h >= 2) mvwadd_wch(win,   y,     x,     WACS_T_ULCORNER);
	if (h >= 2) mvwadd_wch(win,   y,     x+w-1, WACS_T_URCORNER);
	if (h >= 3) mvwvline_set(win, y+1+s, x,     WACS_T_VLINE, h-2-s);
	if (h >= 3) mvwvline_set(win, y+1+s, x+w-1, WACS_T_VLINE, h-2-s);
	if (h >= 2) mvwadd_wch(win,   y+h-1, x,     WACS_T_LLCORNER);
	if (h >= 2) mvwadd_wch(win,   y+h-1, x+w-1, WACS_T_LRCORNER);
	if (h >= 2) mvwhline_set(win, y+h-1, x+1,   WACS_T_HLINE, w-2);

	if (color) wattroff(win, COLOR_PAIR(color));

	if (l<h && event->name) mvwprintw(win, y+l++, x+1, "%.*s",   w-2, event->name);
	if (l<h && event->loc)  mvwprintw(win, y+l++, x+1, "@ %.*s", w-4, event->loc);
	if (l<h && event->desc) mvwprintw(win, y+l++, x+1, "%.*s",   w-2, event->desc);
}

void event_line(WINDOW *win, event_t *event, int y, int x, int w, int full)
{
	int color = get_color(event->cat);

	if (color) wattron(win, COLOR_PAIR(color));
	mvwaddch(win, y, x++, ACS_BLOCK);
	if (color) wattroff(win, COLOR_PAIR(color));

	if (full) {
		if (all_day(&event->start, &event->end))
			mvwprintw(win, y, x, " [all day]   -");
		else
			mvwprintw(win, y, x, " %2d:%02d-%2d:%02d -",
					event->start.hour, event->start.min,
					event->end.hour,   event->end.min);
		x += 15;
	}
	if (event->name) {
		const char *label = event->name ?: event->desc;
		mvwprintw(win, y, x, "%-*.*s", w-1, w-1, label);
		x += MIN(strlen(label), w-1);
	}
	if (full && event->loc) {
		mvwprintw(win, y, x, " @ %s", event->loc);
	}
}

void todo_line(WINDOW *win, todo_t *todo, int y, int x, int w, int full)
{
	char perc[16];
	char desc[LINES];
	sprintf(perc, "%2d%%", todo->status);

	int cat    = get_color(todo->cat);
	int status = todo->status == NEW  ? COLOR_NEW  :
		     todo->status == DONE ? COLOR_DONE : COLOR_WIP;

	sprintf(desc, "%s", todo->name ?: todo->desc ?: "");
	strsub(desc, '\n', ';');

	/* Print category */
	if (cat) wattron(win, COLOR_PAIR(cat));
	mvwaddch(win, y, x, ACS_BLOCK);
	if (cat) wattroff(win, COLOR_PAIR(cat));
	x += 2;

	/* Print time */
	if (no_date(&todo->due))
		mvwprintw(win, y, x, "[no due date]");
	else
		mvwprintw(win, y, x, "%04d-%02d-%02d %2d:%02d",
				todo->due.year, todo->due.month+1, todo->due.day+1,
				todo->due.hour, todo->due.min);
	x += 18;

	/* Print status */
	if (status) wattron(win, COLOR_PAIR(status));
	mvwprintw(win, y, x, "%s",
		todo->status == NEW    ? "new"  :
		todo->status == DONE   ? "done" : perc);
	if (status) wattroff(win, COLOR_PAIR(status));
	x += 6;

	/* Print description */
	mvwprintw(win, y, x, "%s", desc);
}

/* View init */
void view_init(void)
{
	int hdr = COMPACT ? 1 : 2;
	for (int i = 0; i < N_ELEMENTS(views); i++) {
		if (views[i].init) {
			views[i].win = newwin(LINES-hdr, COLS, hdr, 0);
			views[i].init(views[i].win);
		}
	}
}

/* View draw */
void view_resize(void)
{
	int hdr = COMPACT ? 1 : 2;
	for (int i = 0; i < N_ELEMENTS(views); i++) {
		if (views[i].win) {
			wresize(views[i].win, LINES-hdr, COLS);
			mvwin(views[i].win, hdr, 0);
		}
		if (views[i].size)
			views[i].size(LINES-hdr, COLS);
	}
}

/* View draw */
void view_draw(void)
{
	draw_header();
	werase(views[ACTIVE].win);
	views[ACTIVE].draw();
	wrefresh(views[ACTIVE].win);
}

/* View set */
int view_set(int num)
{
	if (ACTIVE != num) {
		ACTIVE = num;
		view_draw();
	}
	return 1;
}

/* View run */
int view_run(int key, mmask_t btn, int row, int col)
{
	/* Check for compact mode toggle */
	if (key == 'c') {
		COMPACT ^= 1;
		view_resize();
		view_draw();
		return 1;
	}

	/* Check for mouse events */
	if (key == KEY_MOUSE && row == 0) {
		int start = 1;
		for (int i = 0; i < N_ELEMENTS(views); i++) {
			int end = start + strlen(views[i].name) - 1;
			if (start <= col && col <= end && views[i].draw)
				return view_set(i);
			start = end + 2;
		}
	}

	/* Check for view change */
	for (int i = 0; i < N_ELEMENTS(views); i++) {
		if (i == ACTIVE)
			continue;
		for (int j = 0; j < N_ELEMENTS(views[i].keys); j++)
			if (views[i].keys[j] == key)
				return view_set(i);
	}

	/* Shift windows */
	int num   = ACTIVE;
	int shift = key == KEY_RIGHT ? +1 :
		    key == KEY_LEFT  ? -1 : 0;
	while (shift) {
		num += shift;
		num += N_ELEMENTS(views);
		num %= N_ELEMENTS(views);
		if (views[num].run)
			return view_set(num);
	}

	/* Pass key to active view */
	return views[ACTIVE].run(key, btn, row, col);
}

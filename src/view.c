/*
 * Copyright (C) 2012-2013 Andy Spencer <andy753421@gmail.com>
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
#include "conf.h"
#include "date.h"
#include "cal.h"
#include "view.h"

/* Types */
typedef struct {
	const char *name;
	const char *title;
	void      (*init)(WINDOW*);
	void      (*size)(int,int);
	void      (*draw)(void);
	int       (*run)(int,mmask_t,int,int);
	int         keys[8];
	WINDOW     *win;
} view_t;

/* Macros */
#define VIEW(name, title, ...)                \
	void name##_init(WINDOW *win);        \
	void name##_size(int,int);            \
	void name##_draw(void);               \
	int  name##_run(int,mmask_t,int,int); \
	view_t name##_view = {                \
		#name,                        \
		title,                        \
		name##_init,                  \
		name##_size,                  \
		name##_draw,                  \
		name##_run,                   \
		{ __VA_ARGS__ }               \
	}

/* Views */
VIEW(day,      "Day",      KEY_F(1), '1');
VIEW(week,     "Week",     KEY_F(2), '2');
VIEW(month,    "Month",    KEY_F(3), '3');
VIEW(year,     "Year",     KEY_F(4), '4');
VIEW(events,   "Events",   KEY_F(5), '5');
VIEW(todo,     "Todo",     KEY_F(6), '6');
VIEW(settings, "Settings", KEY_F(7), '7');
VIEW(help,     "Help",     KEY_F(8), '8');
VIEW(edit,     "Edit");

/* View data */
view_t  spacer = { "|", "|" };

view_t *views[] = {
	&day_view, &week_view, &month_view, &year_view,
	&events_view, &todo_view,
	&settings_view, &help_view,
	&edit_view
};

view_t *menu[] = {
	&day_view, &week_view, &month_view, &year_view,
	&spacer, &events_view, &todo_view,
	&spacer, &settings_view, &help_view
};

/* Config data */
int COMPACT = 0;

/* Global data */
edit_t EDIT = EDIT_NONE;

/* Local data */
view_t *view   = &day_view;
view_t *active = &day_view;
view_t *popup  = NULL;

/* Local functions */
static void draw_header(void)
{
	move(0, 0);
	attron(COLOR_PAIR(COLOR_TITLE));
	clrtoeol();

	/* Draw menu */
	for (int i = 0; i < N_ELEMENTS(menu); i++) {
		if (menu[i] == active)
			attron(A_BOLD);
		printw("%s ", menu[i]->title);
		if (menu[i] == active)
			attroff(A_BOLD);
	}

	/* Draw popup window */
	if (popup) {
		printw("| ");
		attron(A_BOLD);
		printw("[%s]", popup->title);
		attroff(A_BOLD);
	}

	/* Draw date */
	move(0, COLS-19);
	printw("%04d-%02d-%02d %02d:%02d:%02d",
			NOW.year, NOW.month, NOW.day,
			NOW.hour, NOW.min,   NOW.sec);

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

static int set_view(view_t *_active, view_t *_popup)
{
	view = _popup ?: _active;
	if (active != _active) {
		active = _active;
		set_string("view", 0, "active", active->name);
		view_draw();
	}
	if (popup != _popup) {
		popup = _popup;
		view_draw();
	}
	return 1;
}

/* Curses functions */
void wmvresize(WINDOW *win, int top, int left, int rows, int cols)
{
	int y = getpary(win);
	if (top < y)
		mvderwin(win, top, left);
	wresize(win, rows, cols);
	if (top > y)
		mvderwin(win, top, left);
}

void wshrink(WINDOW *win, int top)
{
	int x    = getparx(win);
	int y    = getpary(win);
	int r    = getmaxy(win);
	int c    = getmaxx(win);
	int rows = r + (y - top);
	if (top  <  y) mvderwin(win, top, x);
	if (rows != r) wresize(win, rows, c);
	if (top  >  y) mvderwin(win, top, x);
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

void event_line(WINDOW *win, event_t *event, int y, int x, int w, int flags)
{
	int color = get_color(event->cat);

	if (color) wattron(win, COLOR_PAIR(color));
	mvwaddch(win, y, x++, ACS_BLOCK);
	if (color) wattroff(win, COLOR_PAIR(color));

	if (flags & SHOW_ACTIVE && event == EVENT)
		wattron(win, A_REVERSE | A_BOLD);
	if (flags & SHOW_DETAILS) {
		if (all_day(&event->start, &event->end))
			mvwprintw(win, y, x+1, "[all day]   - ");
		else
			mvwprintw(win, y, x+1, "%2d:%02d-%2d:%02d - ",
					event->start.hour, event->start.min,
					event->end.hour,   event->end.min);
		x += 15;
		w -= 15;
	}
	if (event->name) {
		const char *label = event->name ?: event->desc;
		mvwprintw(win, y, x, "%-*.*s", w-1, w-1, label);
		x += MIN(strlen(label), w-1);
	}
	if (flags & SHOW_DETAILS && event->loc) {
		mvwprintw(win, y, x, " @ %s", event->loc);
	}
	if (flags & SHOW_ACTIVE && event == EVENT)
		wattroff(win, A_REVERSE | A_BOLD);
}

void todo_line(WINDOW *win, todo_t *todo, int y, int x, int w, int flags)
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

	/* Set background */
	if (flags & SHOW_ACTIVE && todo == TODO)
		wattron(win, A_REVERSE | A_BOLD);
	mvwhline(win, y, x, ' ', COLS-x);

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

	/* Reset flags */
	if (flags & SHOW_ACTIVE && todo == TODO)
		wattroff(win, A_REVERSE | A_BOLD);
}

/* View init */
void view_init(void)
{
	int hdr = COMPACT ? 1 : 2;
	for (int i = 0; i < N_ELEMENTS(views); i++) {
		views[i]->win = newwin(LINES-hdr, COLS, hdr, 0);
		views[i]->init(views[i]->win);
		views[i]->size(LINES-hdr, COLS);
	}
}

/* Config parser */
void view_config(const char *group, const char *name, const char *key, const char *value)
{
	if (match(group, "view")) {
		if (match(key, "compact")) {
			COMPACT = get_bool(value);
		} else if (match(key, "active")) {
			for (int i = 0; i < N_ELEMENTS(views); i++) {
				if (match(value, views[i]->name)) {
					get_string(value);
					view = active = views[i];
					break;
				}
			}
		}
	}
}

/* View draw */
void view_resize(void)
{
	int hdr = COMPACT ? 1 : 2;
	for (int i = 0; i < N_ELEMENTS(views); i++) {
		wresize(views[i]->win, LINES-hdr, COLS);
		mvwin(views[i]->win, hdr, 0);
		views[i]->size(LINES-hdr, COLS);
	}
}

/* View draw */
void view_draw(void)
{
	draw_header();
	werase(view->win);
	view->draw();
	wrefresh(view->win);
}

/* View run */
int view_run(int key, mmask_t btn, int row, int col)
{
	/* Refresh timestamp */
	draw_header();

	/* Check for mouse events on the menu */
	if (key == KEY_MOUSE && row == 0) {
		int start = 1;
		for (int i = 0; i < N_ELEMENTS(menu); i++) {
			int end = start + strlen(menu[i]->name) - 1;
			if (start <= col && col <= end && menu[i]->draw)
				return set_view(menu[i], NULL);
			start = end + 2;
		}
	}

	/* Look though menu for hotkeys */
	for (int i = 0; i < N_ELEMENTS(menu); i++) {
		for (int j = 0; j < N_ELEMENTS(menu[i]->keys); j++)
			if (menu[i]->keys[j] == key)
				return set_view(menu[i], NULL);
	}

	/* Shift windows with left/right keys */
	int shift = key == KEY_RIGHT ? +1 :
		    key == KEY_LEFT  ? -1 : 0;
	if (shift) {
		int num = 0;
		for (int i = 0; i < N_ELEMENTS(menu); i++)
			if (menu[i] == active)
				num = i;
		do  {
			num += shift;
			num += N_ELEMENTS(menu);
			num %= N_ELEMENTS(menu);
		} while (menu[num] == &spacer);
		return set_view(menu[num], NULL);
	}

	/* Handle other keys */
	switch (key) {
		case 'c':
			COMPACT ^= 1;
			set_bool("view", 0, "compact", COMPACT);
			view_resize();
			view_draw();
			return 1;
		case '\033': // escape
			return set_view(active, NULL);
		case '?':    // help
			return set_view(active, &help_view);
		case 'e':    // edit
			return set_view(active, &edit_view);
	}

	/* Pass key to active view */
	return view->run(key, btn, row, col);
}

/* View event */
void view_edit(edit_t mode)
{
	EDIT = mode;
	set_view(active, &edit_view);
}

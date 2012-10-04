#include <string.h>
#include <ncurses.h>
#include "main.h"
#include "screen.h"

/* Types */
typedef struct {
	char   *name;
	void  (*init)(WINDOW*);
	void  (*draw)(void);
	int   (*run)(int,mmask_t,int,int);
	int     keys[8];
	WINDOW *win;
} view_t;

/* Data */
view_t views[] = {
	{ "Day",      day_init,      day_draw,      day_run,      {KEY_F(1), '1', 'd',    } },
	{ "Week",     week_init,     week_draw,     week_run,     {KEY_F(2), '2', 'w',    } },
	{ "Month",    month_init,    month_draw,    month_run,    {KEY_F(3), '3', 'm',    } },
	{ "Year",     year_init,     year_draw,     year_run,     {KEY_F(4), '4', 'y',    } },
	{ "|",        NULL,          NULL,          NULL,         {                       } },
	{ "Todo",     todo_init,     todo_draw,     todo_run,     {KEY_F(5), '5', 't',    } },
	{ "Notes",    notes_init,    notes_draw,    notes_run,    {KEY_F(6), '6', 'n',    } },
	{ "|",        NULL,          NULL,          NULL,         {                       } },
	{ "Settings", settings_init, settings_draw, settings_run, {KEY_F(7), '7', 's',    } },
	{ "Help",     help_init,     help_draw,     help_run,     {KEY_F(8), '8', 'h', '?'} },
};

int active = 2;

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
	for (int i = 0; i < N_ELEMENTS(views); i++)
		if (views[i].init)
			wresize(views[i].win, LINES-2, COLS);
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
			if (start <= col && col <= end)
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

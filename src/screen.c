#include <ncurses.h>
#include "main.h"
#include "screen.h"

/* Types */
typedef struct {
	char  *name;
	void (*init)(void);
	void (*draw)(void);
	int  (*run)(int);
	int    keys[8];
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

view_t *active = &views[0];

/* Local functions */
void draw_header(void)
{
	move(0, 0);
	attron(COLOR_PAIR(1));
	for (int i = 0; i < N_ELEMENTS(views); i++) {
		if (active == &views[i])
			attron(A_BOLD);
		printw(" %s", views[i].name);
		if (active == &views[i])
			attroff(A_BOLD);
	}
	attroff(COLOR_PAIR(1));
	mvhline(1, 0, ACS_HLINE, win_cols);
}

/* Screen init */
void screen_init(void)
{
	init_pair(1, COLOR_GREEN, COLOR_BLACK);
}

/* Scren draw */
void screen_draw(void)
{
	draw_header();
	active->draw();
}

/* Screen run */
int screen_run(int chr)
{
	/* Check for view change */
	for (int i = 0; i < N_ELEMENTS(views); i++) {
		view_t *view = &views[i];
		if (view == active)
			continue;
		for (int j = 0; j < N_ELEMENTS(view->keys); j++)
			if (view->keys[j] == chr) {
				active = view;
				screen_draw();
			}
	}

	/* Pass key to active view */
	return active->run(chr);
}

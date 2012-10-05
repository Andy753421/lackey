#include <string.h>
#include <ncurses.h>

#include "main.h"
#include "util.h"

/* Static data */
static WINDOW *win;

/* Day init */
void day_init(WINDOW *_win)
{
	win = _win;
}

/* Day draw */
void day_draw(void)
{
	const char *mstr = month_to_string(MONTH);
	const char *dstr = day_to_string(day_of_week(YEAR, MONTH, DAY));

	/* Print Header */
	mvwprintw(win, 0, 0, "%s, %s %d", dstr, mstr, DAY);
	mvwprintw(win, 0, COLS-10, "%d-%02d-%02d", YEAR, MONTH, DAY);
	mvwhline(win, 1, 0, ACS_HLINE, COLS);

	/* Print times */
	int start = 8;
	for (int h = 0; h < (LINES-5)/4+1; h++)
		mvwprintw(win, 2+h*4, 0,"%02d:%02d", (start+h)%12, 0);
	mvwvline(win, 2, 5, ACS_VLINE, LINES-4);
}

/* Day run */
int day_run(int key, mmask_t btn, int row, int col)
{
	return 0;
}

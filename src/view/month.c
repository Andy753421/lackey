#include <string.h>
#include <ncurses.h>

#include "util.h"

/* Macros */
#define ROUND(x) ((int)((x)+0.5))

/* Static data */
static WINDOW *win;

/* Test data */
const static int YEAR  = 2012;
const static int MONTH = SEP;
const static int DAY   = 29;

/* Month init */
void month_init(WINDOW *_win)
{
	win = _win;
}

/* Month draw */
void month_draw(void)
{
	const char *name  = month_to_string(MONTH);
	const int   start = start_of_month(YEAR, MONTH);
	const int   days  = days_in_month(YEAR, MONTH);
	const int   weeks = weeks_in_month(YEAR, MONTH);
	const float midpt = (float)COLS/2.0 - (strlen(name) + 1 + 4)/2.0;
	const float hstep = (float)COLS/7.0;
	const float vstep = (float)(LINES-4)/weeks;

	/* Print Header */
	mvwprintw(win, 0, midpt, "%s %d", name, YEAR);
	for (int d = 0; d < 7; d++)
		mvwprintw(win, 1, ROUND(d*hstep), "%s", day_to_str(d+SUN));
	mvwhline(win, 2, 0, ACS_HLINE, COLS);

	/* Print days */
	for (int d = 0; d < days; d++) {
		int row = (start + d) / 7;
		int col = (start + d) % 7;
		mvwprintw(win, ROUND(3+row*vstep), ROUND(col*hstep), "%d", d+1);
	}

	/* Print lines */
	for (int w = 1; w < weeks; w++)
		mvwhline(win, ROUND(2+w*vstep), 0, ACS_HLINE, COLS);
	for (int d = 1; d < 7; d++) {
		int top = d >=  start         ? 0     : 1;
		int bot = d <= (start+days)%7 ? weeks : weeks-1;
		mvwvline(win, ROUND(3+top*vstep), ROUND(d*hstep-1),
				ACS_VLINE, (bot-top)*vstep);
		for (int w = 1; w < weeks; w++) {
			int chr = w == top ? ACS_TTEE :
				  w == bot ? ACS_BTEE : ACS_PLUS;
			mvwaddch(win, ROUND(2+w*vstep), ROUND(d*hstep-1), chr);
		}
	}
}

/* Month run */
int month_run(int key, mmask_t btn, int row, int col)
{
	return 0;
}

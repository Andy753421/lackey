#include <string.h>
#include <ncurses.h>

#include "main.h"
#include "util.h"

/* Static data */
static WINDOW *win;

/* Week init */
void week_init(WINDOW *_win)
{
	win = _win;
}

/* Week draw */
void week_draw(void)
{
	int x = 6;
	int y = 3;
	const float hstep = (float)(COLS-x)/5.0;

	/* Get start of week */
	year_t  year  = YEAR;
	month_t month = MONTH;
	day_t   day   = DAY;
	int shift = day_of_week(year, month, day);
	add_days(&year, &month, &day, -shift+MON);

	/* Print Header */
	mvwprintw(win, 1, 0, "%s", month_to_str(MONTH));
	for (int d = 0; d < 5; d++) {
		// FIXME..
		const char *str = hstep >= 10 ? day_to_string(d+MON) : day_to_str(d+MON);
		mvwprintw(win, 0, x+ROUND(d*hstep), "%02d/%02d", month, day);
		mvwprintw(win, 1, x+ROUND(d*hstep), "%s", str);
		add_days(&year, &month, &day, 1);
	}

	/* Print times */
	int start = 8;
	for (int h = 0; h < (LINES-6)/4+1; h++)
		mvwprintw(win, 3+h*4, 0,"%02d:%02d", (start+h)%12, 0);

	/* Print lines */
	mvwhline(win, y-1, 0, ACS_HLINE, COLS);
	for (int d = 0; d < 5; d++)
		mvwvline(win, y, x+ROUND(d*hstep)-1, ACS_VLINE, LINES-y-2);
}

/* Week run */
int week_run(int key, mmask_t btn, int row, int col)
{
	return 0;
}

#include <string.h>
#include <ncurses.h>

#include "main.h"
#include "util.h"

/* Constants */
#define MW (2*7+6)

/* Static data */
static WINDOW *win;

/* Helper functions */
static void print_month(month_t month, int y, int x)
{
	const char *name  = month_to_string(month);
	const int   start = start_of_month(YEAR, month);
	const char  days  = days_in_month(YEAR, month);
	mvwprintw(win, y, x+MW/2-strlen(name)/2, "%s", name);
	wmove(win, y+1, x);
	for (int d = 0; d < 7; d++)
		wprintw(win, "%-3s", day_to_st(d));
	for (int d = 0; d < days; d++) {
		int row = (start + d) / 7;
		int col = (start + d) % 7;
		mvwprintw(win, y+2+row, x+col*3, "%d", d+1);
	}
}

/* Year init */
void year_init(WINDOW *_win)
{
	win = _win;
}

/* Year draw */
void year_draw(void)
{
	int w = MW*3 + 2*3;
	int x = COLS/2 - w/2;
	int y = 0;
	int h[4] = {};

	/* Determine heights */
	for (int m = 0; m < 12; m++) {
		int weeks = weeks_in_month(YEAR, m);
		h[m/3] = MAX(h[m/3], weeks+2);
	}
	int sum = h[0]+h[1]+h[2]+h[3];

	/* Print Header */
	mvwprintw(win, y++, COLS/2-2, "%d", YEAR);

	/* Print Months */
	for (int m = 0; m < 12; m++) {
		print_month(m, y, x);
		if (m % 3 == 2) {
			x  = COLS/2 - w/2;
			y += h[m/3]+1;
		} else {
			x += 3+MW;
		}
	}

	/* Print Lines */
	y = 1;
	mvwvline(win, y, x+(MW+3)*1-2, ACS_VLINE, sum+3);
	mvwvline(win, y, x+(MW+3)*2-2, ACS_VLINE, sum+3);
	for (int i = 0; i < 3; i++) {
		y += h[i];
		mvwhline(win, y, x,        ACS_HLINE, w);
		mvwaddch(win, y, x+(MW+3)*1-2, ACS_PLUS);
		mvwaddch(win, y, x+(MW+3)*2-2, ACS_PLUS);
		y++;
	}
}

/* Year run */
int year_run(int key, mmask_t btn, int row, int col)
{
	return 0;
}

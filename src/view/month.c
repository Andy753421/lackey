#include <ncurses.h>

/* Static data */
static WINDOW *win;

/* Month init */
void month_init(WINDOW *_win)
{
	win = _win;
}

/* Month draw */
void month_draw(void)
{
	mvwprintw(win, 0, 1, "%s\n", "month");
	wrefresh(win);
}

/* Month run */
int month_run(int key, mmask_t btn, int row, int col)
{
	return 0;
}

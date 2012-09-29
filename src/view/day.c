#include <ncurses.h>

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
	mvwprintw(win, 0, 1, "%s\n", "day");
	wrefresh(win);
}

/* Day run */
int day_run(int key, mmask_t btn, int row, int col)
{
	return 0;
}

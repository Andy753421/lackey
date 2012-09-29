#include <ncurses.h>

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
	mvwprintw(win, 0, 1, "%s\n", "week");
	wrefresh(win);
}

/* Week run */
int week_run(int key, mmask_t btn, int row, int col)
{
	return 0;
}

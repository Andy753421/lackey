#include <ncurses.h>

/* Static data */
static WINDOW *win;

/* Year init */
void year_init(WINDOW *_win)
{
	win = _win;
}

/* Year draw */
void year_draw(void)
{
	mvwprintw(win, 0, 1, "%s\n", "year");
	wrefresh(win);
}

/* Year run */
int year_run(int key, mmask_t btn, int row, int col)
{
	return 0;
}

#include <ncurses.h>

/* Static data */
static WINDOW *win;

/* Help init */
void help_init(WINDOW *_win)
{
	win = _win;
}

/* Help draw */
void help_draw(void)
{
	mvwprintw(win, 0, 1, "%s\n", "help");
	wrefresh(win);
}

/* Help run */
int help_run(int key, mmask_t btn, int row, int col)
{
	return 0;
}

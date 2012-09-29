#include <ncurses.h>

/* Static data */
static WINDOW *win;

/* Todo init */
void todo_init(WINDOW *_win)
{
	win = _win;
}

/* Todo draw */
void todo_draw(void)
{
	mvwprintw(win, 0, 1, "%s\n", "todo");
	wrefresh(win);
}

/* Todo run */
int todo_run(int key, mmask_t btn, int row, int col)
{
	return 0;
}

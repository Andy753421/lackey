#include <ncurses.h>

/* Static data */
static WINDOW *win;

/* Notes init */
void notes_init(WINDOW *_win)
{
	win = _win;
}

/* Notes draw */
void notes_draw(void)
{
	mvwprintw(win, 0, 1, "%s\n", "notes");
	wrefresh(win);
}

/* Notes run */
int notes_run(int key, mmask_t btn, int row, int col)
{
	return 0;
}

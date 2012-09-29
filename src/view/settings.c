#include <ncurses.h>

/* Static data */
static WINDOW *win;

/* Settings init */
void settings_init(WINDOW *_win)
{
	win = _win;
}

/* Settings draw */
void settings_draw(void)
{
	mvwprintw(win, 0, 1, "%s\n", "settings");
	wrefresh(win);
}

/* Settings run */
int settings_run(int key, mmask_t btn, int row, int col)
{
	return 0;
}

#!/bin/bash

for file; do
	func=$(basename ${file%.c})
	name=$(echo $func | sed -r 's/./\U&/')
	tab="	"
	cat > $file <<-EOF
		#include <ncurses.h>

		/* Static data */
		static WINDOW *win;

		/* $name init */
		void ${func}_init(WINDOW *_win)
		{
		${tab}win = _win;
		}

		/* $name draw */
		void ${func}_draw(void)
		{
		${tab}mvwprintw(win, 0, 1, "%s\n", "${func}");
		${tab}wrefresh(win);
		}

		/* $name run */
		int ${func}_run(int key, mmask_t btn, int row, int col)
		{
		${tab}return 0;
		}
	EOF
done

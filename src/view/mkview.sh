#!/bin/bash

for file; do
	name=${file%.c}
	tab="	"
	cat > $file <<-EOF
		#include <ncurses.h>

		/* $name init */
		void ${name}_init(void)
		{
		}

		/* $name draw */
		void ${name}_draw(void)
		{
		}

		/* $name run */
		int ${name}_run(int key, mmask_t btn, int row, int col)
		{
		${tab}return 0;
		}
	EOF
done

#!/bin/bash

for file; do
	name=${file%.c}
	tab="	"
	cat > $file <<-EOF
		/* $name init */
		void ${name}_init(void)
		{
		}

		/* $name draw */
		void ${name}_draw(void)
		{
		}

		/* $name run */
		int ${name}_run(int chr)
		{
		${tab}return 0;
		}
	EOF
done

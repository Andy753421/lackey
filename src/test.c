/*
 * Copyright (C) 2012 Andy Spencer <andy753421@gmail.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "date.h"
#include "cal.h"
#include "conf.h"
#include "util.h"

void date_test(void);
void ical_test(void);
void conf_test(void);

int main(int argc, char **argv)
{
	for (int i = 1; i < argc; i++) {
		if (match(argv[i], "date")) date_test();
		if (match(argv[i], "ical")) ical_test();
		if (match(argv[i], "conf")) conf_test();
	}
	return 0;
}

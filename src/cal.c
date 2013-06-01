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

#include "util.h"
#include "date.h"
#include "cal.h"

/* Global data */
event_t *EVENTS;
todo_t  *TODOS;

/* Initialize */
void cal_init(void)
{
	EVENTS = cal_events(2012, JAN, 0, 366);
	TODOS  = cal_todos(2012, JAN, 0, 366);

	/* Debug */
	for (event_t *e = EVENTS; e; e = e->next)
		debug("event: %04d-%02d-%02d %02d:%02d: %s - %s\n",
				e->start.year, e->start.month, e->start.day,
				e->start.hour, e->start.min, e->name, e->desc);
	for (todo_t *e = TODOS; e; e = e->next)
		debug("todo: %04d-%02d-%02d %02d:%02d: %s - %s\n",
				e->start.year, e->start.month, e->start.day,
				e->start.hour, e->start.min, e->name, e->desc);
}

/* Get events */
event_t *cal_events(year_t year, month_t month, day_t day, int days)
{
	return ical_events(0, year, month, day, days)
	   ?: dummy_events(0, year, month, day, days);
}

/* Get todos */
todo_t *cal_todos(year_t year, month_t month, day_t day, int days)
{
	return ical_todos(0, year, month, day, days)
	   ?: dummy_todos(0, year, month, day, days);
}

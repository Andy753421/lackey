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

#include <stdlib.h>

#include "util.h"
#include "conf.h"
#include "date.h"
#include "cal.h"

/* Test data */
static cal_t cal = {
	.name  = "dummy",
	.desc  = "dummy calendar",
	.data  = NULL,
};

static event_t event = {
	.cal   = &cal,
	.start = {2012, OCT, 0, 12, 0},
	.end   = {2012, OCT, 0, 13, 0},
	.name  = "dummy event",
	.desc  = "this event is random and does not exist",
};

static todo_t todo = {
	.cal     = &cal,
	.name    = "dummy todo",
	.desc    = "this todo is random and does not exist",
	.due     = {2012, OCT, 0, 13, 0},
	.status  = 50,
};

static int     enable;

/* Config parser */
void dummy_config(const char *group, const char *name, const char *key, const char *value)
{
	if (match(group, "dummy") && match(key, "enable"))
		enable = get_bool(value);
}

/* Event functions */
event_t *dummy_events(date_t start, date_t end)
{
	event_t *last = &event;
	for (int i = 0; i < 8; i++) {
		last->next        = new0(event_t);
		last->next->cal   = event.cal;
		last->next->start = event.start;
		last->next->end   = event.end;
		last->next->name  = strcopy(event.name);
		last->next->desc  = strcopy(event.desc);

		date_t *s = &last->next->start;
		date_t *e = &last->next->end;
		add_days(&s->year, &s->month, &s->day, 7*i);
		add_days(&e->year, &e->month, &e->day, 7*i);

		last = last->next;
		last->next = NULL;
	}
	return enable ? event.next : 0;
}

/* Todo functions */
todo_t *dummy_todos(date_t start, date_t end)
{
	todo_t *last = &todo;
	for (int i = 0; i < 6; i++) {
		last->next = new0(event_t);
		last->next->cal    = todo.cal;
		last->next->name   = strcopy(todo.name);
		last->next->desc   = strcopy(todo.desc);
		last->next->due    = todo.due;
		last->next->status = todo.status;
		last = last->next;
		last->next = NULL;
	}
	return enable ? todo.next : 0;
}

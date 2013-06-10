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
#include "date.h"
#include "cal.h"

/* Macros */
#define CAL(name) \
	event_t *name##_events(cal_t *cal, year_t year, month_t month, day_t day, int days); \
	todo_t  *name##_todos(cal_t *cal, year_t year, month_t month, day_t day, int days)

/* Prototypes */
CAL(dummy);
CAL(ical);

event_t *cal_events(year_t year, month_t month, day_t day, int days);
todo_t  *cal_todos(year_t year, month_t month, day_t day, int days);

/* Global data */
event_t *EVENTS;
todo_t  *TODOS;

/* Merge events and todos */
static void add_event(event_t **first, event_t **last, event_t **next)
{
	if (*last)
		(*last)->next = *next;
	else
		(*first) = *next;
	(*last) = (*next);
	(*next) = (*next)->next;
}

static void add_todo(todo_t **first, todo_t **last, todo_t **next)
{
	if (*last)
		(*last)->next = *next;
	else
		(*first) = *next;
	(*last) = (*next);
	(*next) = (*next)->next;
}

static event_t *merge_events(event_t *a, event_t *b)
{
	event_t *first = NULL, *last = NULL;
	while (a && b)
		if (compare(&a->start, &b->start) <= 0)
			add_event(&first, &last, &a);
		else
			add_event(&first, &last, &b);
	while (a) add_event(&first, &last, &a);
	while (b) add_event(&first, &last, &b);
	return first;
}

static todo_t *merge_todos(todo_t *a, todo_t *b)
{
	todo_t *first = NULL, *last = NULL;
	while (a && b)
		if (compare(&a->start, &b->start) <= 0)
			add_todo(&first, &last, &a);
		else
			add_todo(&first, &last, &b);
	while (a) add_todo(&first, &last, &a);
	while (b) add_todo(&first, &last, &b);
	return first;
}

/* Initialize */
void cal_init(void)
{
	EVENTS = cal_events(2012, JAN, 0, 366);
	TODOS  = cal_todos(2012, JAN, 0, 366);

	/* Debug */
	for (event_t *e = EVENTS; e; e = e->next)
		debug("event: %04d-%02d-%02d %02d:%02d: %s - %s",
				e->start.year, e->start.month, e->start.day,
				e->start.hour, e->start.min, e->name, e->desc);
	for (todo_t *e = TODOS; e; e = e->next)
		debug("todo: %04d-%02d-%02d %02d:%02d: %s - %s",
				e->start.year, e->start.month, e->start.day,
				e->start.hour, e->start.min, e->name, e->desc);
}

/* Get events */
event_t *cal_events(year_t year, month_t month, day_t day, int days)
{
	event_t *dummy = dummy_events(0, year, month, day, days);
	event_t *ical  =  ical_events(0, year, month, day, days);
	return merge_events(dummy, ical);
}

/* Get todos */
todo_t *cal_todos(year_t year, month_t month, day_t day, int days)
{
	todo_t *dummy = dummy_todos(0, year, month, day, days);
	todo_t *ical  =  ical_todos(0, year, month, day, days);
	return merge_todos(dummy, ical);
}

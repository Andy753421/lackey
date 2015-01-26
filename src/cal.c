/*
 * Copyright (C) 2012-2013 Andy Spencer <andy753421@gmail.com>
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
	void     name##_config(const char *group, const char *name, const char *key, const char *value); \
	cal_t   *name##_cals(void); \
	event_t *name##_events(date_t start, date_t end); \
	todo_t  *name##_todos(date_t start, date_t end); \
	void     name##_edit(edit_t mode); \
	void     name##_save(edit_t mode)

/* Prototypes */
CAL(dummy);
CAL(ical);

/* Global data */
cal_t   *CAL,   *CALS;
event_t *EVENT, *EVENTS;
todo_t  *TODO,  *TODOS;

/* Local data */
static date_t start;
static date_t end;

/* Merge events and todos */
static void add_event(event_t **first, event_t **last, event_t **next)
{
	if (*last)
		(*last)->next = *next;
	else
		(*first) = *next;
	(*next)->prev = *last;
	(*last) = (*next);
	(*next) = (*next)->next;
}

static void add_todo(todo_t **first, todo_t **last, todo_t **next)
{
	if (*last)
		(*last)->next = *next;
	else
		(*first) = *next;
	(*next)->prev = *last;
	(*last) = (*next);
	(*next) = (*next)->next;
}

static cal_t *merge_cals(cal_t *a, cal_t *b)
{
	// TODO - we should sort these
	if (!a) return b;
	if (!b) return a;
	cal_t *last = a;
	while (last->next)
		last = last->next;
	last->next = b;
	return a;
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
	/* Load calendars */
	CALS = merge_cals(
		dummy_cals(),
		 ical_cals());

	/* Load data */
	cal_load(SEL.year, SEL.month, SEL.day, 1);

	/* Debug */
#ifdef CAL_DEBUG
	for (event_t *e = EVENTS; e; e = e->next)
		debug("event: %04d-%02d-%02d %02d:%02d: %s - %s",
				e->start.year, e->start.month, e->start.day,
				e->start.hour, e->start.min, e->name, e->desc);
	for (todo_t *e = TODOS; e; e = e->next)
		debug("todo: %04d-%02d-%02d %02d:%02d: %s - %s",
				e->start.year, e->start.month, e->start.day,
				e->start.hour, e->start.min, e->name, e->desc);
#endif
}

/* Load events and todos */
void cal_load(year_t year, month_t month, day_t day, int days)
{
	year_t  eyear  = year;
	month_t emonth = month;
	day_t   eday   = day;
	add_days(&eyear, &emonth, &eday, days);

	/* Skip if we already loaded enough info */
	if (before(&start, year,  month,  day,  0, 0) &&
	   !before(&end,  eyear, emonth, eday, 24, 0))
	     	return;

	/* Free uneeded data */
	for (event_t *next, *cur = EVENTS; cur; cur = next) {
		next = cur->next;
		if (cur->name) free(cur->name);
		if (cur->desc) free(cur->desc);
		if (cur->loc)  free(cur->loc);
		if (cur->cat)  free(cur->cat);
		free(cur);
	}
	for (todo_t *next, *cur = TODOS; cur; cur = next) {
		next = cur->next;
		if (cur->name) free(cur->name);
		if (cur->desc) free(cur->desc);
		if (cur->cat)  free(cur->cat);
		free(cur);
	}

	/* Push dates out a bit to avoid reloading,
	 * enough to at least cover the current year */
	add_days(&year,  &month,  &day, -366);
	add_days(&eyear, &emonth, &eday, 366);
	start = (date_t){year,  month,  day};
	end   = (date_t){eyear, emonth, eday};

	/* Load events */
	EVENTS = merge_events(
		dummy_events(start, end),
		 ical_events(start, end));

	/* Load todos */
	TODOS  = merge_todos(
		dummy_todos(start, end),
		 ical_todos(start, end));

	/* Verify events and todos*/
#ifdef CAL_ERROR
	for (event_t *cur = EVENTS; cur; cur = cur->next) {
		if (!cur->cal)
			error("Missing cal in event '%s'", cur->name);
		if ((cur->next && cur->next->prev != cur) ||
		    (cur->prev && cur->prev->next != cur))
			error("Broken link in event '%s'", cur->name);
	}
	for (todo_t *cur = TODOS; cur; cur = cur->next) {
		if (!cur->cal)
			error("Missing cal in todo '%s'", cur->name);
		if ((cur->next && cur->next->prev != cur) ||
		    (cur->prev && cur->prev->next != cur))
			error("Broken link in todo '%s'", cur->name);
	}
#endif
}

/* Config parser */
void cal_config(const char *group, const char *name, const char *key, const char *value)
{
	if (match(group, "dummy"))
		dummy_config(group, name, key, value);
	else if (match(group, "ical"))
		ical_config(group, name, key, value);
}

/* Edit functions */
void cal_edit(edit_t mode)
{
	const cal_t *cal =
		mode == EDIT_EVENT ? EVENT->cal :
		mode == EDIT_TODO  ? TODO->cal  : CAL;
	debug("cal_edit");
	if (match(cal->type, "dummy")) dummy_edit(mode);
	if (match(cal->type, "ical"))  ical_edit(mode);
}

void cal_save(edit_t mode)
{
	const cal_t *cal =
		mode == EDIT_EVENT ? EVENT->cal :
		mode == EDIT_TODO  ? TODO->cal  : CAL;
	if (match(cal->type, "dummy")) dummy_save(mode);
	if (match(cal->type, "ical"))  ical_save(mode);
}

/* Find event for matching target date */
event_t *find_event(date_t *target)
{
	int      min   = 0;
	event_t *event = NULL;

	if (EVENT && compare(&EVENT->start, target) == 0)
		return EVENT;

	for (event_t *cur = EVENTS; cur; cur = cur->next) {
		// Skip events that are on the wrong day
		if ((target->year  != cur->start.year) ||
		    (target->month != cur->start.month) ||
		    (target->day   != cur->start.day))
		    	continue;

		// We don't want time change or leap seconds here
		int diff = (target->hour - cur->start.hour) * 60 * 24 +
			   (target->min  - cur->start.min)  * 60 +
			   (target->sec  - cur->start.sec);

		if (event == NULL || ABS(diff) < min) {
			min   = ABS(diff);
			event = cur;
		}
	}

	return event;
}

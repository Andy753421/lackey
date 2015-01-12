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

#define _XOPEN_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <wordexp.h>
#include <libical/ical.h>

#include "util.h"
#include "conf.h"
#include "date.h"
#include "cal.h"

/* Local types */
typedef struct {
	cal_t *cal;
	icalcomponent *comp;
	struct icaltimetype start;
	struct icaltimetype end;
} ical_inst;

typedef struct ical_t {
	cal_t          cal;
	char          *location;
	char          *username;
	char          *password;
	icalcomponent *comp;
	struct ical_t *next;
} ical_t;

/* Static data */
static ical_t *calendars;

/* Helper functions */
static int ical_compare(const void *_a, const void *_b)
{
	const ical_inst *a = _a;
	const ical_inst *b = _b;
	int scomp = icaltime_compare(a->start, b->start);
	int ecomp = icaltime_compare(a->end,   b->end);
	return scomp != 0 ? scomp :
	       ecomp != 0 ? ecomp : 0 ;
}

static date_t to_date(struct icaltimetype time)
{
	return (date_t){
		.year  = time.year,
		.month = time.month ? time.month-1 : 0,
		.day   = time.day   ? time.day  -1 : 0,
		.hour  = time.hour,
		.min   = time.minute,
	};
}

static icaltimetype to_itime(date_t time)
{
	return (struct icaltimetype){
		.year   = time.year,
		.month  = time.month + 1,
		.day    = time.day   + 1,
		.hour   = time.hour,
		.minute = time.min
	};
}

static void add_recur(cal_t *cal,
		icalarray *array, icalcomponent *comp,
		icaltimetype start, icaltimetype end,
		icalcomponent_kind which)
{
	icalcomponent_kind kind = icalcomponent_isa(comp);

	if (kind == which) {
		/* Get recurrence data */
		struct icaltimetype cstart, cend; // Component times
		struct icaltimetype istart, iend; // Instance times
		struct icaldurationtype length;   // Duration

		icalproperty             *rrule;
		struct icalrecurrencetype recur;
		icalrecur_iterator       *iter;

		cstart = icalcomponent_get_dtstart(comp);
		cend   = icalcomponent_get_dtend(comp);
		length = icaltime_subtract(cend, cstart);

		/* Full day event */
		if (icaltime_is_null_time(cstart) ||
		    which == ICAL_VTODO_COMPONENT) {
			icalarray_append(array, &(ical_inst){
				.cal   = cal,
				.comp  = comp,
				.start = cstart,
				.end   = cend,
			});
		}

		/* Add all recurrences */
		rrule = icalcomponent_get_first_property(comp, ICAL_RRULE_PROPERTY);
		while (rrule) {
			recur = icalproperty_get_rrule(rrule);
			iter  = icalrecur_iterator_new(recur, cstart);

			/* Add recurrence for this rrule */
			while (1) {
				istart = iend = icalrecur_iterator_next(iter);
				if (icaltime_is_null_time(istart))
					break;    // no more instances
				if (!icaltime_is_null_time(cend))
					iend = icaltime_add(iend, length);

				if (icaltime_compare(iend, start) <= 0)
					continue; // instance ends before start time
				if (icaltime_compare(istart, end) >= 0)
					break;    // instance begins after stop time

				icalarray_append(array, &(ical_inst){
					.cal   = cal,
					.comp  = comp,
					.start = istart,
					.end   = iend,
				});
			}

			icalrecur_iterator_free(iter);
			rrule = icalcomponent_get_next_property(comp, ICAL_RRULE_PROPERTY);
		}
	}

	/* Add children */
	icalcomponent_kind find = ICAL_ANY_COMPONENT;
	icalcomponent *child = icalcomponent_get_first_component(comp, find);
	while (child) {
		add_recur(cal, array, child, start, end, which);
		child = icalcomponent_get_next_component(comp, find);
	}
}

static void read_icals(void)
{
	for (ical_t *cal = calendars; cal; cal = cal->next) {
		if (cal->comp == NULL && cal->location) {
			wordexp_t wexp;
			wordexp(cal->location, &wexp, WRDE_NOCMD);
			icalparser *parser = icalparser_new();
			for (int i = 0; i < wexp.we_wordc; i++) {
				FILE *file = fopen(wexp.we_wordv[i], "r");
				if (!file)
					continue;
				icalparser_set_gen_data(parser, file);
			}
			cal->comp = icalparser_parse(parser, (void*)fgets);
			icalparser_free(parser);
			wordfree(&wexp);
		}
	}
}

/* Event functions */
static event_t *to_event(ical_inst *inst)
{
	icalproperty *prop = icalcomponent_get_first_property(inst->comp, ICAL_CATEGORIES_PROPERTY);

	event_t *event = new0(event_t);
	event->name  = strcopy(icalcomponent_get_summary(inst->comp));
	event->desc  = strcopy(icalcomponent_get_description(inst->comp));
	event->loc   = strcopy(icalcomponent_get_location(inst->comp));
	event->cat   = icalproperty_get_value_as_string_r(prop);
	event->start = to_date(inst->start);
	event->end   = to_date(inst->end);
	event->cal   = inst->cal;
	return event;
}

static event_t *to_events(icalarray *array)
{
	event_t  list = {};
	event_t *tail = &list;
	for (int i = 0; i < array->num_elements; i++) {
		 ical_inst *inst = icalarray_element_at(array, i);
		 tail->next = to_event(inst);
		 tail = tail->next;
	}
	return list.next;
}

static void print_events(event_t *start)
{
	for (event_t *cur = start; cur; cur = cur->next)
		printf("%04d-%02d-%02d %02d:%02d - %s\n",
			cur->start.year, cur->start.month, cur->start.day,
			cur->start.hour, cur->start.min,
			cur->name ?: cur->desc ?: "[no summary]");
}

/* Todo functions */
static todo_t *to_todo(ical_inst *inst)
{
	icalproperty *cat  = icalcomponent_get_first_property(inst->comp, ICAL_CATEGORIES_PROPERTY);
	icalproperty *perc = icalcomponent_get_first_property(inst->comp, ICAL_PERCENTCOMPLETE_PROPERTY);

	todo_t *todo = new0(todo_t);
	todo->name   = strcopy(icalcomponent_get_summary(inst->comp));
	todo->desc   = strcopy(icalcomponent_get_description(inst->comp));
	todo->cat    = strcopy(icalproperty_get_value_as_string(cat));
	todo->status = icalcomponent_get_status(inst->comp) == ICAL_STATUS_COMPLETED ? 100 :
	               perc ? icalproperty_get_percentcomplete(perc) : 0;
	todo->start  = to_date(inst->start);
	todo->due    = to_date(icalcomponent_get_due(inst->comp));
	todo->cal    = inst->cal;
	return todo;
}

static todo_t *to_todos(icalarray *array)
{
	todo_t  list = {};
	todo_t *tail = &list;
	for (int i = 0; i < array->num_elements; i++) {
		 ical_inst *inst = icalarray_element_at(array, i);
		 tail->next = to_todo(inst);
		 tail = tail->next;
	}
	return list.next;
}

static void print_todos(todo_t *start)
{
	for (todo_t *cur = start; cur; cur = cur->next)
		printf("%04d-%02d-%02d %02d:%02d - %d%% - %s\n",
			cur->due.year, cur->due.month, cur->due.day,
			cur->due.hour, cur->due.min,   cur->status,
			cur->name ?: cur->desc ?: "[no summary]");
}

/* Config parser */
void ical_config(const char *group, const char *name, const char *key, const char *value)
{
	ical_t *cal = NULL, *last = NULL;

	/* Make sure it's valid */
	if (!match(group, "ical") || !name)
		return;

	/* Find existing calendar */
	for (cal = calendars; cal; last = cal, cal = cal->next)
		if (match(cal->cal.name, name))
			break;

	/* Create new calendar */
	if (!cal) {
		cal = new0(ical_t);
		cal->cal.type = "ical";
		cal->cal.name = get_name(name);
		if (last)
			last->next = cal;
		else
			calendars = cal;
		return;
	}

	/* Set calendar values */
	if (match(key, "location"))
		cal->location = get_string(value);
	else if (match(key, "username"))
		cal->username = get_string(value);
	else if (match(key, "password"))
		cal->password = get_string(value);
}

/* Cal functions */
cal_t *ical_cals(void)
{
	read_icals();

	for (ical_t *cal = calendars; cal; cal = cal->next)
		cal->cal.next = &cal->next->cal;

	return &calendars->cal;
}

/* Event functions */
event_t *ical_events(date_t _start, date_t _end)
{
	read_icals();

	icaltimetype start = to_itime(_start);
	icaltimetype end   = to_itime(_end);
	icalarray *array = icalarray_new(sizeof(ical_inst), 1);
	for (ical_t *cal = calendars; cal; cal = cal->next)
		add_recur(&cal->cal, array, cal->comp, start, end, ICAL_VEVENT_COMPONENT);
	icalarray_sort(array, ical_compare);
	event_t *events = to_events(array);
	icalarray_free(array);

	return events;
}

/* Todo functions */
todo_t *ical_todos(date_t _start, date_t _end)
{
	read_icals();

	icaltimetype start = to_itime(_start);
	icaltimetype end   = to_itime(_end);
	icalarray *array = icalarray_new(sizeof(ical_inst), 1);
	for (ical_t *cal = calendars; cal; cal = cal->next)
		add_recur(&cal->cal, array, cal->comp, start, end, ICAL_VTODO_COMPONENT);
	icalarray_sort(array, ical_compare);
	todo_t *todos = to_todos(array);
	icalarray_free(array);

	return todos;
}

/* Test functions */
void ical_printr(icalcomponent *comp, int depth)
{
	/* Print component */
	icalcomponent_kind kind = icalcomponent_isa(comp);
	printf("%*s", depth, "");
	printf("%s",  icalcomponent_kind_to_string(kind));
	if (kind == ICAL_VEVENT_COMPONENT ||
	    kind == ICAL_VTODO_COMPONENT)
		printf(" - %s", icalcomponent_get_summary(comp) ?: "[no summary]");
	printf("\n");

	/* Print children */
	icalcomponent_kind find = ICAL_ANY_COMPONENT;
	icalcomponent *child = icalcomponent_get_first_component(comp, find);
	while (child) {
		ical_printr(child, depth+2);
		child = icalcomponent_get_next_component(comp, find);
	}
}

void ical_test(void)
{
	/* Load ical */
	FILE *file = fopen("data/all.ics", "r");
	icalparser *parser = icalparser_new();
	icalparser_set_gen_data(parser, file);
	icalcomponent *comp = icalparser_parse(parser, (void*)fgets);

	/* Misc */
	icalarray *array;
	icaltimetype start = {.year = 2000};
	icaltimetype end   = {.year = 2020};

	/* Find events */
	array = icalarray_new(sizeof(ical_inst), 1);
	add_recur(NULL, array, comp, start, end, ICAL_VEVENT_COMPONENT);
	icalarray_sort(array, ical_compare);
	event_t *events = to_events(array);
	icalarray_free(array);

	/* Find Todos */
	array = icalarray_new(sizeof(ical_inst), 1);
	add_recur(NULL, array, comp, start, end, ICAL_VTODO_COMPONENT);
	icalarray_sort(array, ical_compare);
	todo_t *todos = to_todos(array);
	icalarray_free(array);

	/* Print */
	ical_printr(comp, 0);
	print_events(events);
	print_todos(todos);

	(void)print_events;
	(void)print_todos;
	(void)events;
	(void)todos;

	/* Cleanup */
	icalparser_free(parser);
}

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

#include <stdio.h>
#include <stdlib.h>
#include <libical/ical.h>

#include "util.h"
#include "date.h"
#include "cal.h"

/* Local types */
typedef struct {
	icalcomponent *comp;
	struct icaltimetype start;
	struct icaltimetype end;
} ical_inst;

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
		.month = time.month-1,
		.day   = time.day-1,
		.hour  = time.hour,
		.min   = time.minute,
	};
}

static event_t *to_event(ical_inst *inst)
{
	icalproperty *prop = icalcomponent_get_first_property(inst->comp, ICAL_CATEGORIES_PROPERTY);

	event_t *event = calloc(1, sizeof(event_t));
	event->name  = icalcomponent_get_summary(inst->comp);
	event->desc  = icalcomponent_get_description(inst->comp);
	event->loc   = icalcomponent_get_location(inst->comp);
	event->cat   = icalproperty_get_value_as_string(prop);
	event->start = to_date(inst->start);
	event->end   = to_date(inst->end);
	return event;
}

static event_t *to_list(icalarray *array)
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

static void print_list(event_t *start)
{
	for (event_t *cur = start; cur; cur = cur->next)
		printf("%04d-%02d-%02d %02d:%02d - %s\n",
			cur->start.year, cur->start.month, cur->start.day,
			cur->start.hour, cur->start.min,
			cur->name ?: cur->desc ?: "[no summary]");
}

static void add_events(icalarray *array, icalcomponent *comp,
		icaltimetype start, icaltimetype end)
{
	icalcomponent_kind kind = icalcomponent_isa(comp);

	if (kind == ICAL_VEVENT_COMPONENT ||
	    kind == ICAL_VTODO_COMPONENT) {
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

		rrule  = icalcomponent_get_first_property(comp, ICAL_RRULE_PROPERTY);
		recur  = icalproperty_get_rrule(rrule);
		iter   = icalrecur_iterator_new(recur, cstart);

		/* Add recurrences */
		while (1) {
			istart = icalrecur_iterator_next(iter);
			if (icaltime_is_null_time(istart))
				break;    // no more instances
			iend   = icaltime_add(istart, length);
			if (icaltime_compare(iend, start) <= 0)
				continue; // instance ends before start time
			if (icaltime_compare(istart, end) >= 0)
				break;    // instance begins after stop time
			icalarray_append(array, &(ical_inst){
				.comp  = comp,
				.start = istart,
				.end   = iend,
			});
		}
	}

	/* Add children */
	icalcomponent_kind find = ICAL_ANY_COMPONENT;
	icalcomponent *child = icalcomponent_get_first_component(comp, find);
	while (child) {
		add_events(array, child, start, end);
		child = icalcomponent_get_next_component(comp, find);
	}
}

/* Event functions */
event_t *ical_events(cal_t *cal, year_t year, month_t month, day_t day, int days)
{
	/* Load ical */
	FILE *file = fopen("data/all.ics", "r");
	if (!file)
		return NULL;
	icalparser *parser = icalparser_new();
	icalparser_set_gen_data(parser, file);
	icalcomponent *ical = icalparser_parse(parser, (void*)fgets);

	/* Add events */
	icalarray *array = icalarray_new(sizeof(ical_inst), 1);
	icaltimetype start = {.year = 2000};
	icaltimetype end   = {.year = 2020};
	add_events(array, ical, start, end);
	icalarray_sort(array, ical_compare);
	return to_list(array);

	/* Todo, memory management */
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
	icalcomponent *ical = icalparser_parse(parser, (void*)fgets);

	/* Test print */
	ical_printr(ical, 0);

	/* Test Add */
	icalarray *array = icalarray_new(sizeof(ical_inst), 1);
	icaltimetype start = {.year = 2000};
	icaltimetype end   = {.year = 2020};
	add_events(array, ical, start, end);
	icalarray_sort(array, ical_compare);
	event_t *events = to_list(array);
	print_list(events);

	/* Cleanup */
	icalparser_free(parser);
}

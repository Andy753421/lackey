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
#include "event.h"

/* Helper functions */
static event_t *to_event(icalcomponent *comp)
{
	event_t *event = calloc(1, sizeof(event));
	return event;
}

/* Event functions */
event_t *ical_get(cal_t *cal, year_t year, month_t month, day_t day, int days)
{
	(void)to_event;
	return NULL;
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
	FILE *file = fopen("data/all.ics", "r");
	icalparser *parser = icalparser_new();
	icalparser_set_gen_data(parser, file);
	icalcomponent *ical = icalparser_parse(parser, (void*)fgets);
	ical_printr(ical, 0);
	icalparser_free(parser);
}

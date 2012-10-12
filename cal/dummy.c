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

#include "main.h"
#include "event.h"

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
	.next  = NULL,
};

static event_t events[8];

/* Event functions */
event_t *dummy_get(cal_t *cal, year_t year, month_t month, day_t day, int days)
{
	for (int i = 0; i < N_ELEMENTS(events); i++) {
		datetime_t *s = &events[i].start;
		datetime_t *e = &events[i].end;
		events[i] = event;
		add_days(&s->year, &s->month, &s->day, 7*i);
		add_days(&e->year, &e->month, &e->day, 7*i);
		if (i+1 < N_ELEMENTS(events))
			events[i].next = &events[i+1];
	}
	return &events[0];
}

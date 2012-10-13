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

/* Calendar types */
typedef struct {
	char *name;
	char *desc;
	void *data;
} cal_t;

/* Event types */
typedef struct event_t {
	const cal_t    *cal;
	const char     *name;
	const char     *desc;
	const char     *loc;
	const char     *cat;
	date_t          start;
	date_t          end;
	struct event_t *next;
} event_t;

/* Global data */
extern event_t *EVENTS;

/* Event functions */
void event_init(void);
event_t *event_get(year_t year, month_t month, day_t day, int days);

/* Calendar implementation functions */
event_t *dummy_get(cal_t *cal, year_t year, month_t month, day_t day, int days);
event_t *ical_get(cal_t *cal, year_t year, month_t month, day_t day, int days);

/* Test fuctions */
void ical_test(void);

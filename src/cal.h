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

/* Cal status types */
typedef enum {
	NEW  = 0,
	DONE = 100,
} status_t;

/* Calendar type */
typedef struct cal_t {
	char         *type;
	char         *name;
	char         *desc;
	struct cal_t *next;
} cal_t;

/* Calendar item types */
typedef struct event_t {
	char           *name;
	char           *desc;
	char           *loc;
	char           *cat;
	date_t          start;
	date_t          end;
	const cal_t    *cal;
	struct event_t *next;
	struct event_t *prev;
} event_t;

typedef struct todo_t {
	char          *name;
	char          *desc;
	char          *cat;
	status_t       status;
	date_t         start;
	date_t         due;
	cal_t         *cal;
	struct todo_t *next;
	struct todo_t *prev;
} todo_t;

/* Global data */
extern cal_t   *CAL,   *CALS;
extern event_t *EVENT, *EVENTS;
extern todo_t  *TODO,  *TODOS;

/* Calendar functions */
void cal_init(void);
void cal_load(year_t year, month_t month, day_t day, int days);
void cal_config(const char *group, const char *name, const char *key, const char *value);

/* Event functions */
event_t *find_event(date_t *target);

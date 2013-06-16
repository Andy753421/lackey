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

/* Time types */
typedef long long stamp_t;

typedef int year_t;
typedef int day_t;
typedef int hour_t;
typedef int min_t;
typedef int sec_t;

typedef enum {
	JAN =  0,
	FEB =  1,
	MAR =  2,
	APR =  3,
	MAY =  4,
	JUN =  5,
	JUL =  6,
	AUG =  7,
	SEP =  8,
	OCT =  9,
	NOV = 10,
	DEC = 11,
} month_t;

typedef enum {
	SUN = 0,
	MON = 1,
	TUE = 2,
	WED = 3,
	THU = 4,
	FRI = 5,
	SAT = 6,
} wday_t;

typedef struct {
	year_t  year;
	month_t month;
	day_t   day;
	hour_t  hour;
	min_t   min;
	sec_t   sec;
} date_t;

/* Global data */
extern date_t NOW; // current wall clock time, refreshed at 10 Hz
extern date_t SEL; // date and time the user is looking at

/* Initialize */
void date_init(void);
void date_sync(void);

/* Time functions */
int is_leap_year(year_t year);
int days_in_year(year_t year);
int days_in_month(year_t year, month_t month);
int weeks_in_month(year_t year, month_t month);
wday_t day_of_week(year_t year, month_t month, day_t day);
wday_t start_of_month(year_t year, month_t month);
day_t start_of_week(year_t year, month_t month, day_t day);
void add_days(year_t *year, month_t *month, day_t *day, int days);
void add_months(year_t *year, month_t *month, int months);

stamp_t get_stamp(date_t *date);
int get_mins(date_t *start, date_t *end);
int compare(date_t *a, date_t *b);
int before(date_t *start, int year, int month, int day, int hour, int min);

int all_day(date_t *start, date_t *end);
int no_date(date_t *date);

/* Time to string functions */
const char *month_to_str(month_t month);
const char *month_to_string(month_t month);
const char *day_to_st(wday_t day);
const char *day_to_str(wday_t day);
const char *day_to_string(wday_t day);

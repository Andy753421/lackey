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

/* Time Keeping Bugs Abound! */

#include <stdio.h>
#include <time.h>

#include "date.h"

/* Global data */
year_t  YEAR;
month_t MONTH;
day_t   DAY;

/* Initialize */
void date_init(void)
{
	time_t     sec = time(NULL);
	struct tm *tm  = localtime(&sec);

	YEAR  = tm->tm_year+1900;
	MONTH = tm->tm_mon;
	DAY   = tm->tm_mday-1;

	/* Testing */
	//YEAR  = 2008;
	//MONTH = OCT;
	//DAY   = 21;
}

/* Time functions */
int is_leap_year(year_t year)
{
	return (year % 400 == 0) ? 1 :
	       (year % 100 == 0) ? 0 :
	       (year % 4   == 0) ? 1 : 0;
}

int days_in_year(year_t year)
{
	return 365 + is_leap_year(year);
}

int days_in_month(year_t year, month_t month)
{
	static int mdays[] = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };
	int days = mdays[month];
	if (month == FEB)
		days += is_leap_year(year);
	return days;
}

int weeks_in_month(year_t year, month_t month)
{
	int start = start_of_month(year, month);
	int days  = days_in_month(year, month);
	return ((start + days)-1) / 7 + 1;
}

wday_t day_of_week(year_t year, month_t month, day_t day)
{
	static int tmp[] = {0, 3, 2, 5, 0, 3,
		            5, 1, 4, 6, 2, 4};
	if (month < MAR)
		year--;
	int start = year + year / 4
		         - year / 100
		         + year / 400
		         + tmp[month];
	return (start + day + 1) % 7;
}

wday_t start_of_month(year_t year, month_t month)
{
	return day_of_week(year, month, 0);
}

void add_days(year_t *year, month_t *month, day_t *day, int days)
{
	time_t time = mktime(&(struct tm){
			.tm_year = *year-1900,
			.tm_mon  = *month,
			.tm_mday = *day+1,
			.tm_hour = 12});
	time  += days*24*60*60;
	struct tm *tm = localtime(&time);
	*year  = tm->tm_year+1900;
	*month = tm->tm_mon;
	*day   = tm->tm_mday-1;
}

void add_months(year_t *year, month_t *month, int months)
{
	int total = *year*12 + *month + months;
	*year  = total / 12;
	*month = total % 12;
}

stamp_t get_time(date_t *date)
{
	return mktime(&(struct tm){
		.tm_year = date->year-1900,
		.tm_mon  = date->month,
		.tm_mday = date->day+1,
		.tm_hour = date->hour,
		.tm_min  = date->min});
}

int get_mins(date_t *start, date_t *end)
{
	return (get_time(end)-get_time(start))/60;
}

int before(date_t *start, int year, int month, int day, int hour, int min)
{
	int rval = start->year  < year  ? 1 : start->year  > year ? 0 :
	           start->month < month ? 1 : start->month > month? 0 :
	           start->day   < day   ? 1 : start->day   > day  ? 0 :
	           start->hour  < hour  ? 1 : start->hour  > hour ? 0 :
	           start->min   < min   ? 1 : start->min   > min  ? 0 : 0;
	return rval;
}

/* Debug functions */
const char *month_to_str(month_t month)
{
	static const char *map[] =
		{ "Jan", "Feb", "Mar", "Apr", "May", "Jun",
		  "Jul", "Aug", "Sep", "Oct", "Nov", "Dec", };
	return map[month%12];
}
const char *month_to_string(month_t month)
{
	static const char *map[] =
		{ "January",   "February", "March",    "April",
		  "May",       "June",     "July",     "August",
		  "September", "October",  "November", "December" };
	return map[month%12];
}

const char *day_to_st(wday_t day)
{
	static const char *map[] =
		{ "Su","Mo", "Tu", "We", "Th", "Fr", "Sa" };
	return map[day%7];
}
const char *day_to_str(wday_t day)
{
	static const char *map[] =
		{ "Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat" };
	return map[day%7];
}
const char *day_to_string(wday_t day)
{
	static const char *map[] =
		{ "Sunday",   "Monday", "Tuesday", "Wednesday",
		  "Thursday", "Friday", "Saturday" };
	return map[day%7];
}

/* Test functions */
void date_test(void)
{
	printf("Info\n");
	printf("  Year Month     Start Weeks Days\n");
	for (int y = 2012; y <= 2012; y++)
	for (int m = JAN;  m <= DEC;  m++) {
		printf("  %-5d",  y);
		printf("  %-10s", month_to_string(m));
		printf("  %-6s",  day_to_str(start_of_month(y,m)));
		printf("  %-6d",  weeks_in_month(y,m));
		printf("  %-2d",  days_in_month(y,m));
		printf("\n");
	}
}

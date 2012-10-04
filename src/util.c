/* Time Keeping Bugs Abound! */

#include <stdio.h>

#include "util.h"

/* Helper functions */
static int is_leap_year(year_t year)
{
	return (year % 400 == 0) ? 1 :
	       (year % 100 == 0) ? 0 :
	       (year % 4   == 0) ? 1 : 0;
}

static wday_t day_of_week(year_t year, month_t month, day_t day)
{
	static int tmp[] = {0, 3, 2, 5, 0, 3,
		            5, 1, 4, 6, 2, 4};
	if (month < 3)
		year--;
	int start = year + year / 4
		         - year / 100
		         + year / 400
		         + tmp[month];
	return (start + day) % 7;
}

/* Time functions */
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

wday_t start_of_month(year_t year, month_t month)
{
	return day_of_week(year, month, 1);
}

/* Debug functions */
const char *month_to_str(month_t month)
{
	static const char *map[] =
		{ "Jan", "Feb", "Mar", "Apr", "May", "Jun",
		  "Jul", "Aug", "Sep", "Oct", "Nov", "Dec", };
	return map[month];
}
const char *month_to_string(month_t month)
{
	static const char *map[] =
		{ "January",   "February", "March",    "April",
		  "May",       "June",     "July",     "August",
		  "September", "October",  "November", "December" };
	return map[month];
}

const char *day_to_st(wday_t day)
{
	static const char *map[] =
		{ "Su","Mo", "Tu", "We", "Th", "Fr", "Sa" };
	return map[day];
}
const char *day_to_str(wday_t day)
{
	static const char *map[] =
		{ "Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat" };
	return map[day];
}
const char *day_to_string(wday_t day)
{
	static const char *map[] =
		{ "Sunday",   "Monday", "Tuesday", "Wednesday",
		  "Thursday", "Friday", "Saturday" };
	return map[day];
}

/* Test functions */
void test_time(void)
{
	printf("Year Month     Start Weeks Days\n");
	for (int y = 2012; y <= 2012; y++)
	for (int m = JAN;  m <= DEC;  m++) {
		printf("%-5d",  y);
		printf("%-10s", month_to_string(m));
		printf("%-6s",  day_to_str(start_of_month(y,m)));
		printf("%-6d",  weeks_in_month(y,m));
		printf("%-2d",  days_in_month(y,m));
		printf("\n");
	}
}

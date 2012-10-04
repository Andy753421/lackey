/* Types */
typedef int year_t;
typedef int day_t;

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

/* Time functions */
int days_in_year(year_t year);
int days_in_month(year_t year, month_t month);
int weeks_in_month(year_t year, month_t month);
wday_t start_of_month(year_t year, month_t month);

/* String functions */
const char *month_to_str(month_t month);
const char *month_to_string(month_t month);
const char *day_to_st(wday_t day);
const char *day_to_str(wday_t day);
const char *day_to_string(wday_t day);

/* Tests */
void test_time(void);

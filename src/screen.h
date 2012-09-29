/* Screen functions */
void screen_init(void);
void screen_draw(void);
int  screen_run(int);

/* View init functions */
void day_init(void);
void week_init(void);
void month_init(void);
void year_init(void);
void todo_init(void);
void notes_init(void);
void settings_init(void);
void help_init(void);

/* View draw functions */
void day_draw(void);
void week_draw(void);
void month_draw(void);
void year_draw(void);
void todo_draw(void);
void notes_draw(void);
void settings_draw(void);
void help_draw(void);

/* View run functions */
int day_run(int);
int week_run(int);
int month_run(int);
int year_run(int);
int todo_run(int);
int notes_run(int);
int settings_run(int);
int help_run(int);

/* Screen functions */
void screen_init(void);
void screen_resize(void);
void screen_draw(void);
int  screen_run(int key, mmask_t btn, int row, int col);

/* View init functions */
void day_init(WINDOW *win);
void week_init(WINDOW *win);
void month_init(WINDOW *win);
void year_init(WINDOW *win);
void todo_init(WINDOW *win);
void notes_init(WINDOW *win);
void settings_init(WINDOW *win);
void help_init(WINDOW *win);

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
int day_run(int,mmask_t,int,int);
int week_run(int,mmask_t,int,int);
int month_run(int,mmask_t,int,int);
int year_run(int,mmask_t,int,int);
int todo_run(int,mmask_t,int,int);
int notes_run(int,mmask_t,int,int);
int settings_run(int,mmask_t,int,int);
int help_run(int,mmask_t,int,int);

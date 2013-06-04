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

/* Configuration */
#define COLOR_TITLE 1
#define COLOR_ERROR 2

#define COLOR_NEW   3
#define COLOR_WIP   4
#define COLOR_DONE  5

#define COLOR_CLASS 6
#define COLOR_EC    7
#define COLOR_WORK  8
#define COLOR_OTHER 9

/* Config data */
extern int COMPACT;
extern int ACTIVE;

/* Helper functions */
void event_box(WINDOW *win, event_t *event, int y, int x, int h, int w);
void event_line(WINDOW *win, event_t *event, int y, int x, int w, int full);
void todo_line(WINDOW *win, todo_t *todo, int y, int x, int w, int full);

/* View functions */
void view_init(void);
void view_resize(void);
void view_draw(void);
int  view_run(int key, mmask_t btn, int row, int col);

/* View init functions */
void day_init(WINDOW *win);
void week_init(WINDOW *win);
void month_init(WINDOW *win);
void year_init(WINDOW *win);
void events_init(WINDOW *win);
void todo_init(WINDOW *win);
void settings_init(WINDOW *win);
void help_init(WINDOW *win);

/* View size functions */
void day_size(int,int);
void week_size(int,int);
void month_size(int,int);
void year_size(int,int);
void events_size(int,int);
void todo_size(int,int);
void settings_size(int,int);
void help_size(int,int);

/* View draw functions */
void day_draw(void);
void week_draw(void);
void month_draw(void);
void year_draw(void);
void events_draw(void);
void todo_draw(void);
void settings_draw(void);
void help_draw(void);

/* View run functions */
int day_run(int,mmask_t,int,int);
int week_run(int,mmask_t,int,int);
int month_run(int,mmask_t,int,int);
int year_run(int,mmask_t,int,int);
int events_run(int,mmask_t,int,int);
int todo_run(int,mmask_t,int,int);
int settings_run(int,mmask_t,int,int);
int help_run(int,mmask_t,int,int);

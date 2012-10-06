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

#ifndef SCREEN_H
#define SCREEN_H

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

#endif

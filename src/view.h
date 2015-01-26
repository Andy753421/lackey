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

/* Display flags */
#define SHOW_DETAILS 0x1
#define SHOW_ACTIVE  0x2

/* Config data */
extern int COMPACT; // reduce layout spacing
extern int MORNING; // first line to display

/* Global data */
extern edit_t EDIT; // edit mode 0=cal 1=event 3=todo

/* Curses functions */
void wmvresize(WINDOW *win, int top, int left, int rows, int cols);
void wshrink(WINDOW *win, int top);

/* Helper functions */
void event_box(WINDOW *win, event_t *event, int y, int x, int h, int w);
void event_line(WINDOW *win, event_t *event, int y, int x, int w, int flags);
void todo_line(WINDOW *win, todo_t *todo, int y, int x, int w, int flags);

/* View functions */
void view_init(void);
void view_config(const char *group, const char *name, const char *key, const char *value);
void view_resize(void);
void view_draw(void);
int  view_run(int key, mmask_t btn, int row, int col);
void view_edit(edit_t mode);

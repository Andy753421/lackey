/*
 * Copyright (C) 2015 Andy Spencer <andy753421@gmail.com>
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

/* Widget Macros */
#define TEXT(k,...)    {.f.type=FORM_TEXT,   .f.hotkey=k, __VA_ARGS__}
#define NUMBER(k,...)  {.f.type=FORM_NUMBER, .f.hotkey=k, __VA_ARGS__}
#define DATE(k,...)    {.f.type=FORM_DATE,   .f.hotkey=k, __VA_ARGS__}
#define BUTTON(k,...)  {.f.type=FORM_BUTTON, .f.hotkey=k, __VA_ARGS__}
#define LIST(k,...)    {.f.type=FORM_LIST,   .f.hotkey=k, __VA_ARGS__}

#define HEAD(t,...)    &(form_field_t){.type=FORM_LABEL, .label=t, .attr.bold=1}
#define LABEL(t,...)   &(form_field_t){.type=FORM_LABEL, .label=t, .after="  "}
#define TAB            &(form_field_t){.type=FORM_LABEL, .label="    "}

#define CHECK(t,...)   {.f.type=FORM_BUTTON, .f.label=t}
#define TOGGLE(t,...)  {.f.type=FORM_BUTTON, .f.label=t}
#define RADIO(t,...)   {.f.type=FORM_BUTTON, .f.label=t}
#define BUTTONS(t,...) {.f.type=FORM_BUTTON, .f.label=t} // LIST[buttons]

/* Widget types */
typedef enum {
	FORM_LABEL,
	FORM_TEXT,
	FORM_DATE,
	FORM_NUMBER,
	FORM_BUTTON,
	FORM_LIST
} form_type_t;

/* Text attributes */
typedef struct {
	int bold : 1;
} form_attr_t;

/* Form header */
typedef struct {
	form_type_t type;
	form_attr_t attr;
	int         hotkey;
	char       *before;
	char       *label;
	char       *after;
} form_field_t;

/* Form types */
typedef struct {
	form_field_t f;
	char   *text;
} form_text_t;

typedef struct {
	form_field_t f;
	date_t  date;
} form_date_t;

typedef struct {
	form_field_t f;
	int     number;
} form_number_t;

typedef struct {
	form_field_t f;
	int     button;
} form_button_t;

typedef struct {
	form_field_t f;
	char **map;
	int    num;
	int    idx;
} form_list_t;

/* Form structure */
typedef struct {
	int rows;
	int cols;
	form_field_t *fields[][10];
} form_t;

/* Form functions */
void form_show(form_t *form);
void form_draw(WINDOW *win);
int  form_run(int key, mmask_t btn, int row, int col);

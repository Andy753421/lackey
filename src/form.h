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
#define TEXT(k,...)    {.type=FORM_TEXT,   .hotkey=k, __VA_ARGS__}
#define NUMBER(k,...)  {.type=FORM_NUMBER, .hotkey=k, __VA_ARGS__}
#define DATE(k,...)    {.type=FORM_DATE,   .hotkey=k, __VA_ARGS__}
#define BUTTON(k,...)  {.type=FORM_BUTTON, .hotkey=k, __VA_ARGS__}
#define LIST(k,...)    {.type=FORM_LIST,   .hotkey=k, __VA_ARGS__}

#define HEAD(t,...)    &(form_field_t){.type=FORM_LABEL, .label=t, .attr.bold=1}
#define LABEL(t,...)   &(form_field_t){.type=FORM_LABEL, .label=t}

#define CHECK(t,...)     {.type=FORM_BUTTON, .label=t}
#define TOGGLE(t,...)    {.type=FORM_BUTTON, .label=t}
#define RADIO(t,...)     {.type=FORM_BUTTON, .label=t}
#define BUTTONS(t,...)   {.type=FORM_BUTTON, .label=t} // LIST[buttons]

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

/* Form data type */
typedef struct {
	form_type_t type;
	form_attr_t attr;
	int         hotkey;
	char       *before;
	char       *label;
	char       *after;
	union {
		char    text[256];
		date_t  date;
		int     number;
		int     button;
		struct {
			char **map;
			int    num;
			int    idx;
		} list;
	};
} form_field_t;

typedef struct {
	int rows;
	int cols;
	form_field_t *fields[][10];
} form_t;

void form_init(void);

int form_run(form_t *form, int key, mmask_t btn, int row, int col);

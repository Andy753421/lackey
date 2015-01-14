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

#define _XOPEN_SOURCE
#define _XOPEN_SOURCE_EXTENDED

#include <string.h>
#include <ncurses.h>

#include "date.h"
#include "form.h"

/* Constants */
#define TEXT_WIDTH   (20)
#define NUMBER_WIDTH (10)
#define DATE_WIDTH   (4+1+2+1+2 +1+ 2+1+2)

/* Local variables */
static WINDOW *form_win;
static int     form_row;
static int     form_col;

/* Helpeer functions */
int label_width(const char *label)
{
	int len = 0;
	for (int i = 0; label[i]; i++)
		if (label[i] != '_')
			len++;
	return len;
}

void label_print(WINDOW *win, const char *label)
{
	for (int i = 0; label[i]; i++) {
		if (label[i] == '_' && label[i+1])
			waddch(form_win, label[++i]
				| A_UNDERLINE | A_BOLD);
		else
			waddch(form_win, label[i]);
	}
}

int field_width(form_field_t *field)
{
	// Calculate list width
	int list_size = 0;
	if (field->type == FORM_LIST) {
		for (int i = 0; i < field->list.num; i++) {
			int width = strlen(field->list.map[i]);
			if (width > list_size)
				list_size = width;
		}
	}

	// Calculate field size
	int width = 0;
	if (field->before)
		width += strlen(field->before);
	switch (field->type) {
		case FORM_LABEL:  width += label_width(field->label); break;
		case FORM_TEXT:   width += TEXT_WIDTH;                break;
		case FORM_DATE:   width += DATE_WIDTH;                break;
		case FORM_NUMBER: width += NUMBER_WIDTH;              break;
		case FORM_BUTTON: width += label_width(field->label); break;
		case FORM_LIST:   width += list_size;                 break;
	}
	if (field->after)
		width += strlen(field->after);
	return width;
}

void field_draw(WINDOW *win, form_field_t *field, int width, int hover)
{
	char **map   = field->list.map;
	int    idx   = field->list.idx;

	char *before = field->before ?: "";
	char *after  = field->after  ?: "";

	int boxed    = field->type == FORM_TEXT ||
	               field->type == FORM_NUMBER;
	int under    = 0;

	int begin    = getcurx(win);
	int maxstr   = width - strlen(before) - strlen(after);

	if (hover)  wattron(win, A_REVERSE);
	if (under)  wattron(win, A_UNDERLINE);
	if (boxed)  waddch(win, '[');
	if (boxed)  maxstr -= 2;

	wprintw(win, "%s", before);
	switch (field->type) {
		case FORM_LABEL:
			label_print(win, field->label);
			break;
		case FORM_TEXT:
			wprintw(win, "%-.*s",
				maxstr, field->text);
			break;
		case FORM_DATE:
			if (no_date(&field->date))
				wprintw(win, "%s", "undefined");
			else
				wprintw(win, "%04d-%02d-%02d %02d:%02d",
					field->date.year,  field->date.month+1,
					field->date.day+1, field->date.hour,
					field->date.min);
			break;
		case FORM_NUMBER:
			wprintw(win, "%d", field->number);
			break;
		case FORM_BUTTON:
			wprintw(win, "%s", field->label);
			break;
		case FORM_LIST:
			if (map)
				wprintw(win, "%s", map[idx]);
			else
				wprintw(win, "%s", "undefined");
			break;
	}
	int pad = width-(getcurx(win)-begin)-boxed;
	wprintw(win, "%-*s", pad, after);

	if (boxed)  waddch(win, ']');
	if (under)  wattroff(win, A_UNDERLINE);
	if (hover)  wattroff(win, A_REVERSE);
}

int is_active(form_t *form, int r, int c)
{
	if (r == form_row && c == form_col)
		return 1;
	for (int i = c+1; i < form->cols && !form->fields[r][i]; i++)
		if (r == form_row && i == form_col)
			return 1;
	return 0;
}

int can_active(form_t *form, int r, int c)
{
	for (int i = c; i >= 0; i--) {
		if (!form->fields[r][i])
			continue;
		if (form->fields[r][i]->type != FORM_LABEL)
			return 1;
		else
			return 0;
	}
	return 0;
}

void set_active(form_t *form, int ro, int co)
{
	// Set first field
	if (ro==0 && co==0) {
		for (int r = 0; r < form->rows; r++)
		for (int c = 0; c < form->cols; c++) {
			if (can_active(form, r, c)) {
				form_row = r;
				form_col = c;
				return;
			}
		}
	}

	// Move up/down
	if (ro) {
		for (int ri = form_row+ro; ri>=0 && ri<form->rows; ri+=ro) {
			if (can_active(form, ri, form_col)) {
				form_row = ri;
				return;
			}
		}
	}

	// Move left/right
	if (co) {
		for (int ci = form_col+co; ci>=0 && ci<form->cols; ci+=co) {
			for (int ri = form_row; ri < form->rows; ri++)
				if (can_active(form, form_row, ci)) {
					form_row = ri;
					form_col = ci;
					return;
				}
			for (int ri = form_row; ri >= 0; ri--)
				if (can_active(form, form_row, ci)) {
					form_row = ri;
					form_col = ci;
					return;
				}
		}
	}
}

/* Initialize */
void form_init(void)
{
	form_win = newwin(LINES, COLS, 0, 0);
}

/* Resize */
void form_resize(void)
{
	mvwin(form_win, 0, 0);
	wresize(form_win, LINES, COLS);
}

/* Run */
int form_draw(form_t *form)
{
	// Calculate column width
	//    do this by column, and right to left so that
	//    we can add in the extra space available for
	//    blank spaces.
	int col_size[form->cols];
	for (int c = form->cols-1; c >= 0; c--) {
		col_size[c] = 0;
		for (int r = 0; r < form->rows; r++) {
			form_field_t *field = form->fields[r][c];
			if (form->fields[r][c]) {
				int width = field_width(field);
				for (int i = c+1; i < form->cols; i++) {
					if (form->fields[r][i])
						break;
					width -= col_size[i];
				}
				if (width > col_size[c])
					col_size[c] = width;
			}
		}
	}

	// Make sure we have an active field
	if (!can_active(form, form_row, form_col))
		set_active(form, 0, 0);

	// Display form
	for (int r = 0; r < form->rows; r++) {
		for (int c = 0; c < form->cols; c++) {
			form_field_t *field = form->fields[r][c];
			// Calculate form field size
			int width = col_size[c];
			for (int i = c+1; i < form->cols; i++) {
				if (form->fields[r][i])
					break;
				width += col_size[i];
			}
			// Draw the field
			if (field)
				field_draw(form_win, field, width,
					is_active(form, r, c));
			else if (c == 0)
				wprintw(form_win, "%*s", width, "");
		}
		wprintw(form_win, "\n");
	}
	return 0;
}

int form_run(form_t *form, int key, mmask_t btn, int row, int col)
{
	// Check movement keys
	switch (key) {
		case 'h': set_active(form,  0, -1); goto redraw;
		case 'j': set_active(form,  1,  0); goto redraw;
		case 'k': set_active(form, -1,  0); goto redraw;
		case 'l': set_active(form,  0,  1); goto redraw;
	}

	// Search for hotkeys
	for (int r = 0; r < form->rows; r++)
	for (int c = 0; c < form->cols; c++)
		if (form->fields[r][c] &&
		    form->fields[r][c]->hotkey == key) {
			form_row = r;
			form_col = c;
			goto redraw;
		}
	return 0;

redraw:
	werase(form_win);
	form_draw(form);
	wrefresh(form_win);
	return 1;
}

/* Test functions */
static form_field_t title     = TEXT('t');
static form_field_t location  = TEXT('o');
static form_field_t start     = DATE('s');
static form_field_t end       = DATE('e');
static form_field_t due_date  = DATE('u');
static form_field_t completed = NUMBER('p', .after="%");
static form_field_t calendar  = LIST('c');
static form_field_t category  = LIST('g');
static form_field_t repeat    = LIST('r');
static form_field_t frequency = NUMBER(0);
static form_field_t weekdays  = BUTTONS("Su Mo Tu We Th Fr Sa");
static form_field_t details   = TEXT('d');

static form_t edit = { 12, 4, {
	{ LABEL("_Title: "),    &title                                         },
	{ LABEL("L_ocation: "), &location                                      },
	{                                                                      },
	{ LABEL("_Start: "),    &start,    LABEL("  _End: "),       &end       },
	{ LABEL("D_ue Date: "), &due_date, LABEL("  Com_pleted: "), &completed },
	{ LABEL("_Calendar: "), &calendar, LABEL("  Cate_gory: "),  &category  },
	{                                                                      },
	{ LABEL("_Repeat: "),   &repeat,   LABEL("  Every: "),      &frequency },
	{                                                                      },
	{ NULL,                 &weekdays                                      },
	{                                                                      },
	{ LABEL("_Details: "),  &details                                       },
} };

void form_test(void)
{
	/* Setup Curses */
	initscr();
	cbreak();
	noecho();
	keypad(stdscr, TRUE);
	start_color();
	curs_set(false);
	timeout(100);
	use_default_colors();
	mousemask(ALL_MOUSE_EVENTS, NULL);

	/* Init */
	form_init();

	/* Run */
	while (1) {
		MEVENT btn;
		int chr = getch();
		if (chr == 'q')
			break;
		if (chr == KEY_MOUSE)
			if (getmouse(&btn) != OK)
				continue;
		switch (chr) {
			case KEY_RESIZE:
				form_resize();
				refresh();
				werase(form_win);
				form_draw(&edit);
				wrefresh(form_win);
				continue;
			case '\14': // Ctrl-L
				clear();
			case '\7':  // Ctrl-G
				refresh();
				werase(form_win);
				form_draw(&edit);
				wrefresh(form_win);
				continue;
		}
		if (form_run(&edit, chr, btn.bstate, btn.y, btn.x))
			continue;
		if (chr == ERR) // timeout
			continue;
	}

	/* Finish */
	endwin();
}

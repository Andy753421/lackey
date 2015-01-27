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
#include "cal.h"
#include "view.h"
#include "form.h"

/* Constants */
#define DATE_WIDTH   (4+1+2+1+2 +1+ 2+1+2)
#define TEXT_WIDTH   (DATE_WIDTH)
#define NUMBER_WIDTH (10)

/* Widget accessors */
#define FF_TEXT(f)   (((form_text_t  *)f)->text)
#define FF_DATE(f)   (((form_date_t  *)f)->date)
#define FF_NUMBER(f) (((form_number_t*)f)->number)
#define FF_BUTTON(f) (((form_button_t*)f)->button)
#define FF_LIST(f)   (((form_list_t  *)f))

/* Local variables */
static form_t *form;    // current form
static WINDOW *win;     // current window
static int     arow;	// active row
static int     acol;	// active row

/* Helper functions */
static int label_width(const char *label)
{
	int len = 0;
	for (int i = 0; label[i]; i++)
		if (label[i] != '_')
			len++;
	return len;
}

static void label_print(const char *label)
{
	for (int i = 0; label[i]; i++) {
		if (label[i] == '_' && label[i+1])
			waddch(win, label[++i]
				| A_UNDERLINE | A_BOLD);
		else
			waddch(win, label[i]);
	}
}

static int field_width(form_field_t *field)
{
	// Calculate list width
	int list_size = 0;
	if (field->type == FORM_LIST) {
		for (int i = 0; i < FF_LIST(field)->num; i++) {
			int width = strlen(FF_LIST(field)->map[i]);
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

static void field_sizes(form_t *form, int *col_size)
{
	// Do this by column, and right to left so that we can add
	// in the extra space available for blank spaces.
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
}

static void field_draw(form_field_t *field, int width, int hover)
{
	char **map   = FF_LIST(field)->map;
	int    idx   = FF_LIST(field)->idx;

	char *before = field->before ?: "";
	char *after  = field->after  ?: "";

	int boxed    = field->type == FORM_TEXT ||
	               field->type == FORM_NUMBER;
	int bold     = field->attr.bold;
	int under    = 0;

	int begin    = getcurx(win);
	int maxstr   = width - strlen(before) - strlen(after);

	if (bold)   wattron(win, A_BOLD);
	if (hover)  wattron(win, A_REVERSE);
	if (under)  wattron(win, A_UNDERLINE);
	if (boxed)  waddch(win, '[');
	if (boxed)  maxstr -= 2;

	wprintw(win, "%s", before);
	switch (field->type) {
		case FORM_LABEL:
			label_print(field->label);
			break;
		case FORM_TEXT:
			if (FF_TEXT(field))
				wprintw(win, "%-.*s",
						maxstr, FF_TEXT(field));
			break;
		case FORM_DATE:
			if (no_date(&FF_DATE(field)))
				wprintw(win, "%s", "{undefined}");
			else
				wprintw(win, "%04d-%02d-%02d %02d:%02d",
					FF_DATE(field).year,  FF_DATE(field).month+1,
					FF_DATE(field).day+1, FF_DATE(field).hour,
					FF_DATE(field).min);
			break;
		case FORM_NUMBER:
			wprintw(win, "%d", FF_NUMBER(field));
			break;
		case FORM_BUTTON:
			wprintw(win, "%s", field->label);
			break;
		case FORM_LIST:
			if (map)
				wprintw(win, "%s", map[idx]);
			else
				wprintw(win, "%s", "{undefined}");
			break;
	}
	int pad = width-(getcurx(win)-begin)-boxed;
	wprintw(win, "%-*s", pad, after);

	if (boxed)  waddch(win, ']');
	if (under)  wattroff(win, A_UNDERLINE);
	if (hover)  wattroff(win, A_REVERSE);
	if (bold)   wattroff(win, A_BOLD);
}

static int is_active(form_t *form, int r, int c)
{
	for (int i = c; i >= 0; i--) {
		if (r == arow && i == acol)
			return 1;
		if (form->fields[r][i])
			break;
	}
	for (int i = c+1; i < form->cols; i++) {
		if (form->fields[r][i])
			break;
		if (r == arow && i == acol)
			return 1;
	}
	return 0;
}

static int can_active(form_t *form, int r, int c)
{
	for (int i = c; i >= 0; i--) {
		if (!form->fields[r][i])
			continue;
		if (form->fields[r][i]->type == FORM_LABEL)
			return 0;
		return 1;
	}
	return 0;
}

static int set_active(form_t *form, int r, int c)
{
	if (can_active(form, r, c)) {
		arow = r;
		acol = c;
		return 1;
	}
	return 0;
}

static void move_active(form_t *form, int ro, int co)
{
	// Set first field
	if (ro==0 && co==0)
		for (int r = 0; r < form->rows; r++)
		for (int c = 0; c < form->cols; c++)
			if (set_active(form, r, c))
				return;

	// Move up/down
	if (ro) {
		for (int ri = arow+ro; ri>=0 && ri<form->rows; ri+=ro) {
			if (is_active(form, ri, acol))
				continue;
			// Search for a row
			for (int ci = acol; ci < form->cols; ci++)
				if (set_active(form, ri, ci))
					return;
			for (int ci = acol; ci >= 0; ci--)
				if (set_active(form, ri, ci))
					return;
		}
	}

	// Move left/right
	if (co) {
		for (int ci = acol+co; ci>=0 && ci<form->cols; ci+=co) {
			if (is_active(form, arow, ci))
				continue;
			// Simple move
			if (set_active(form, arow, ci))
				return;
		}
	}
}

/* Form functions */
void form_show(form_t *_form)
{
	// Save form
	form = _form;
}

void form_draw(WINDOW *_win)
{
	// Save window
	win = _win;

	// Validate everything
	if (!win || !form)
		return;

	// Calculate column width
	int col_size[form->cols];
	field_sizes(form, col_size);

	// Make sure we have an active field
	if (!can_active(form, arow, acol))
		move_active(form, 0, 0);

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
				field_draw(field, width, is_active(form, r, c));
			else if (c == 0)
				wprintw(win, "%*s", width, "");
		}
		wprintw(win, "\n");
	}
}

int form_run(int key, mmask_t btn, int row, int col)
{
	// Validate everything
	if (!form)
		return 0;

	// Check movement keys
	switch (key) {
		case 'h': move_active(form,  0, -1); goto redraw;
		case 'j': move_active(form,  1,  0); goto redraw;
		case 'k': move_active(form, -1,  0); goto redraw;
		case 'l': move_active(form,  0,  1); goto redraw;
	}

	// Handle mouse movement
	if (key == KEY_MOUSE && row < form->rows) {
		int pos=0, col_size[form->cols];
		field_sizes(form, col_size);
		for (int c = 0; c < form->cols; c++) {
			if (pos < col && col < pos+col_size[c]) {
				if (can_active(form, row, c)) {
					arow = row;
					acol = c;
					goto redraw;
				}
			}
			pos += col_size[c];
		}
	}

	// Search for hotkeys
	for (int r = 0; r < form->rows; r++)
	for (int c = 0; c < form->cols; c++)
		if (form->fields[r][c] &&
		    form->fields[r][c]->hotkey == key) {
			arow = r;
			acol = c;
			goto redraw;
		}
	return 0;

redraw:
	werase(win);
	form_draw(win);
	wrefresh(win);
	return 1;
}

/* Test functions */
static form_text_t   title     = TEXT('t');
static form_text_t   location  = TEXT('o');
static form_date_t   start     = DATE('s');
static form_date_t   end       = DATE('e');
static form_date_t   due_date  = DATE('u');
static form_number_t completed = NUMBER('p', .f.after="%");
static form_list_t   calendar  = LIST('c');
static form_list_t   category  = LIST('g');
static form_list_t   repeat    = LIST('r');
static form_number_t frequency = NUMBER(0);
static form_button_t weekdays  = BUTTONS("Su Mo Tu We Th Fr Sa");
static form_text_t   details   = TEXT('d');

static form_t edit = { 12, 4, {
	{ LABEL("_Title: "),    &title.f                                           },
	{ LABEL("L_ocation: "), &location.f                                        },
	{                                                                          },
	{ LABEL("_Start: "),    &start.f,    LABEL("  _End: "),       &end.f       },
	{ LABEL("D_ue Date: "), &due_date.f, LABEL("  Com_pleted: "), &completed.f },
	{ LABEL("_Calendar: "), &calendar.f, LABEL("  Cate_gory: "),  &category.f  },
	{                                                                          },
	{ LABEL("_Repeat: "),   &repeat.f,   LABEL("  Every: "),      &frequency.f },
	{                                                                          },
	{ NULL,                 &weekdays.f                                        },
	{                                                                          },
	{ LABEL("_Details: "),  &details.f                                         },
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
	win = newwin(LINES, COLS, 0, 0);
	form_show(&edit);

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
				mvwin(win, 0, 0);
				wresize(win, LINES, COLS);
				refresh();
				werase(win);
				form_draw(win);
				wrefresh(win);
				continue;
			case '\14': // Ctrl-L
				clear();
			case '\7':  // Ctrl-G
				refresh();
				werase(win);
				form_draw(win);
				wrefresh(win);
				continue;
		}
		if (form_run(chr, btn.bstate, btn.y, btn.x))
			continue;
		if (chr == ERR) // timeout
			continue;
	}

	/* Finish */
	endwin();
}

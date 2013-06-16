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

#define _XOPEN_SOURCE_EXTENDED

#include <string.h>
#include <ncurses.h>

#include "util.h"
#include "date.h"
#include "cal.h"
#include "view.h"

/* Static data */
static int     line;
static WINDOW *win;
static WINDOW *times;
static WINDOW *body;

/* Box packing helpers */
static void clear_old(event_t **list, int n, event_t *cur)
{
	for (int i = 0; i < n; i++)
		if (list[i] && compare(&list[i]->end, &cur->start) <= 0)
			list[i] = NULL;
}

static int next_free(event_t **list, int n)
{
	for (int i = 0; i < n; i++)
		if (!list[i])
			return i;
	return n-1;
}

static int count_busy(event_t **list, int n)
{
	int sum = 0;
	for (int i = 0; i < n; i++)
		if (list[i])
			sum++;
	return sum;
}

static int get_ncols(event_t *event, int n)
{
	int ncols = 0;
	event_t *preview[n];
	memset(preview, 0, sizeof(preview));
	for (event_t *cur = event; cur; cur = cur->next) {
		int col = next_free(preview, n);
		preview[col] = cur;
		ncols = MAX(ncols, col+1);
		if (cur->next)
			clear_old(preview, n, cur->next);
		if (count_busy(preview, n) == 0)
			break;
	}
	return ncols;
}

static int get_col(event_t **list, int n, event_t *event, int *ncols)
{
	clear_old(list, n, event);

	/* If there are current events, then recalculate
	 * ncols for the next series of events */
	if (count_busy(list, n) == 0)
		*ncols = get_ncols(event, n);

	/* Find next open slot */
	int col = next_free(list, n);
	list[col] = event;
	return col;
}

/* Day init */
void day_init(WINDOW *_win)
{
	win   = _win; //    lines    cols    y  x
	times = derwin(win, LINES-2,      5, 0, 0);
	body  = derwin(win, LINES-2, COLS-5, 0, 5);
	line  = 10*4; // 10:00
}

/* Day size */
void day_size(int rows, int cols)
{
	int hdr = 2-COMPACT;
	wmvresize(times, hdr, 0, rows-hdr,      5);
	wmvresize(body,  hdr, 5, rows-hdr, cols-5);
}

/* Day draw */
void day_draw(void)
{
	const char *mstr = month_to_string(SEL.month);
	const char *dstr = day_to_string(day_of_week(SEL.year, SEL.month, SEL.day));

	int y = !COMPACT+1;
	event_t *event;

	/* Load cal data */
	cal_load(SEL.year, SEL.month, SEL.day, 1);

	/* Print Header */
	if (COMPACT) wattron(win, A_REVERSE | A_BOLD);
	mvwhline(win, 0, 0, ' ', COLS);
	mvwprintw(win, 0, 0, "%s, %s %d", dstr, mstr, SEL.day+1);
	mvwprintw(win, 0, COLS-10, "%d-%02d-%02d", SEL.year, SEL.month, SEL.day+1);
	if (COMPACT) wattroff(win, A_REVERSE | A_BOLD);

	/* Print all day events */
	event = EVENTS;
	int allday = 0;
	while (event && before(&event->start, SEL.year, SEL.month, SEL.day, 24, 0)) {
		if (!before(&event->end, SEL.year, SEL.month, SEL.day, 0, 1) &&
		    get_mins(&event->start, &event->end) > 23*60)
			event_line(win, event, y+allday++, 6, COLS, SHOW_DETAILS);
		event = event->next;
	}
	if (allday && !COMPACT)
		allday++;

	/* Resize body */
	wshrink(times, y+allday);
	wshrink(body,  y+allday);

	/* Print times */
	mvwprintw(times, 0, 0, "%02d:%02d", ((line/4)-1)%12+1, (line*15)%60);
	for (int h = 0; h < 24; h++)
		mvwprintw(times, h*4-line, 0, "%02d:%02d", (h-1)%12+1, 0);

	/* Print events */
	event = EVENTS;
	event_t *active[10] = {};
	int ncols = 0;
	for (int h = 0; h < 24; h++)
	for (int m = 0; m < 60; m+=15)
	while (event && before(&event->start,
			SEL.year, SEL.month, SEL.day, h+(m+15)/60, (m+15)%60)) {
		if (!before(&event->start, SEL.year, SEL.month, SEL.day, h, m) &&
		    get_mins(&event->start, &event->end) <= 23*60) {
			int col    = get_col(active, N_ELEMENTS(active), event, &ncols);
			int left   = ROUND((col+0.0)*(COLS-6)/ncols) + 1;
			int right  = ROUND((col+1.0)*(COLS-6)/ncols) + 1;
			int row    = h*4 + m/15 - line;
			int height = (get_mins(&event->start, &event->end)-1)/15+1;
			event_box(body, event, row, left, height, right-left);
		}
		event = event->next;
	}

	/* Print lines */
	if (!COMPACT)
		mvwhline(win, 1, 0, ACS_HLINE, COLS);
	if (!COMPACT && allday)
		mvwhline(win, allday+1, 0, ACS_HLINE, COLS);
	mvwvline(body, 0, 0, ACS_VLINE, LINES-4+COMPACT+COMPACT);
}

/* Day run */
int day_run(int key, mmask_t btn, int row, int col)
{
	int days = 0, ref = 0;
	switch (key)
	{
		case 'h': ref = 1; days = -1; break;
		case 'l': ref = 1; days =  1; break;
		case 'i': ref = 1; days = -7; break;
		case 'o': ref = 1; days =  7; break;
		case 'k': ref = 1; line--;    break;
		case 'j': ref = 1; line++;    break;
	}
	line = CLAMP(line, 0, 24*4);
	if (days)
		add_days(&SEL.year, &SEL.month, &SEL.day, days);
	if (ref) {
		werase(win);
		day_draw();
		wrefresh(win);
	}
	return ref;
}

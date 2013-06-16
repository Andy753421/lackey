/*
 * Copyright (C) 2013 Andy Spencer <andy753421@gmail.com>
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

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>

#include "util.h"
#include "conf.h"

/* Saved formattnig struct */
typedef struct line_t {
	char *group;
	char *name;
	char *key;
	char *value;
	char *text;
	int   dirty;
	struct line_t *next;
} line_t;

/* Constat data */
static const char *booleans[] = {
	"false",
	"true"
};

/* Setup info */
static char     *filename;
static parser_t  parser;
static int       argc;
static char    **argv;

/* Static data */
static line_t   *settings;

/* Parsing and saving */
static int       wasfound;
static int       needsave;

static char     *lastgroup;
static char     *lastname;
static char     *lastkey;

/* Helper functions */
static void conf_error(const char *value)
{
	if (lastname)
		error("invalid value '%s' for %s.%s.%s",
				value, lastgroup, lastname, lastkey);
	else
		error("invalid value '%s' for %s.%s",
				value, lastgroup, lastkey);
}

static void set_value(const char *group, const char *name,
		const char *key, const char *value)
{
	int     ingroup  = 0;
	line_t *groupend = NULL;
	line_t *fileend  = NULL;

	/* Queue a save next time around */
	needsave = 1;

	/* Look though for existing items */
	for (line_t *line = settings; line; line = line->next) {
		/* Search for the correct group */
		if (line->group)
			ingroup = match(line->group, group) &&
			          match(line->name,  name);
		/* Set value */
		if (ingroup && match(line->key, key)) {
			free(line->value);
			line->value = strcopy(value);
			line->dirty = 1;
			return;
		}
		/* Record positions for new keys */
		if (ingroup && line->key && line->value)
			groupend = line;
		else
			fileend = line;
	}

	/* Create new items */
	if (groupend) {
		/* Append to group */
		line_t *line = new0(line_t);
		line->key      = strcopy(key);
		line->value    = strcopy(value);
		line->next     = groupend->next;
		groupend->next = line;
	} else if (fileend)  {
		/* Create new group */
		line_t *blank  = new0(line_t);
		line_t *header = new0(line_t);
		line_t *line   = new0(line_t);
		fileend->next = blank;
		blank->next   = header;
		header->group = strcopy(group);
		header->name  = strcopy(name);
		header->next  = line;
		line->key     = strcopy(key);
		line->value   = strcopy(value);
	} else {
		/* Create new file */
		line_t *header = new0(line_t);
		line_t *line   = new0(line_t);
		settings      = header;
		header->group = strcopy(group);
		header->name  = strcopy(name);
		header->next  = line;
		line->key     = strcopy(key);
		line->value   = strcopy(value);
	}
}

/* Parsing functions */
static char *scan_white(char *src)
{
	while (*src == ' ' || *src == '\t')
		src++;
	return src;
}

static char *scan_word(char *dst, char *src)
{
	while (islower(*src))
		*dst++ = *src++;
	*dst = '\0';
	return src;
}

static char *scan_value(char *dst, char *src)
{
	char *start = dst;
	while (*src != '#' && *src != '\0')
		*dst++ = *src++;
	do
		*dst-- = '\0';
	while (dst > start && (*dst == ' ' || *dst == '\t'));
	return src;
}

static char *scan_quote(char *dst, char *src)
{
	while (*src != '"' && *src != '\0') {
		if (*src == '\\') {
			switch (*++src) {
				case '\0': *dst++ = '\0';        break;
				case '\\': *dst++ = '\\'; src++; break;
				case 't':  *dst++ = '\t'; src++; break;
				case 'r':  *dst++ = '\r'; src++; break;
				case 'n':  *dst++ = '\n'; src++; break;
				default:   *dst++ = *src; src++; break;
			}
		} else {
			*dst++ = *src++;
		}
	}
	if (*src == '"')
		src++;
	*dst = '\0';
	return src;
}

static void parse_line(line_t *dst, char *text, const char *file, int lnum)
{
	char *chr = scan_white(text);
	if (*chr == '[') {
		dst->group = malloc(strlen(chr)+1);
		chr = scan_white(chr+1);
		chr = scan_word(dst->group, chr);
		chr = scan_white(chr);
		if (*chr == '"') {
			dst->name = malloc(strlen(chr)+1);
			chr = scan_quote(dst->name, chr+1);
			chr = scan_white(chr);
		}
		if (*chr != ']')
			error("parsing group at %s:%d,%d -- '%.8s..'",
					file, lnum, 1+chr-text, chr);
	}
	else if (islower(*chr)) {
		dst->key = malloc(strlen(chr)+1);
		chr = scan_white(chr);
		chr = scan_word(dst->key, chr);
		chr = scan_white(chr);
		if (*chr != '=')
			error("parsing key at %s:%d,%d -- '%.8s..'",
					file, lnum, 1+chr-text, chr);
		else
			chr++;
		chr = scan_white(chr);
		dst->value = malloc(strlen(chr)+1);
		if (*chr == '"')
			chr = scan_quote(dst->value, chr+1);
		else
			chr = scan_value(dst->value, chr);
	} else if (*chr != '#' && *chr != '\n' && *chr != '\0') {
		error("parsing file at %s:%d,%d -- '%.8s..'",
				file, lnum, 1+chr-text, chr);
	}
}

/* File I/O functions */
static void conf_load(const char *path, parser_t parser)
{
	line_t *prev = NULL;
	char *group = NULL, *name = NULL;

	/* read the whole file */
	int   len;
	char *start = read_file(path, &len);
	if (!start)
		return;

	/* run parser */
	int lnum;
	char *sol, *eol;
	for (lnum = 1, sol = start; sol < (start+len); lnum++, sol = eol+1) {
		eol = strchr(sol, '\n') ?: &start[len];
		eol[0] = '\0';

		/* update current group info */
		line_t *line = new0(line_t);
		line->text = strcopy(sol);
		parse_line(line, sol, path, lnum);
		group = line->group ? line->group : group;
		name  = line->group ? line->name  : name;

		/* Parse dynamic groups */
		if (line->group && line->name) {
			wasfound  = 0;
			lastgroup = line->group;
			lastname  = line->name;
			lastkey   = NULL;
			parser(line->group, line->name, "", "");
			if (!wasfound)
				error("unknown group: line %d - [%s \"%s\"]",
						lnum, group, name ?: "");
		}

		/* Parse static key/value pairs */
		if (group && line->key && line->value) {
			wasfound  = 0;
			lastgroup = group;
			lastname  = name;
			lastkey   = line->key;
			parser(group, name?:"", line->key, line->value);
			if (!wasfound)
				error("unknown setting: line %d - %s.%s.%s = '%s'\n",
						lnum, group, name?:"", line->key, line->value);
		}

		/* debug printout */
		printf("parse: %s.%s.%s = '%s'\n", group, name, line->key, line->value);

		/* save line formatting for the next write */
		if (prev == NULL)
			settings = line;
		else
			prev->next = line;
		prev = line;

	}
	free(start);
}

void conf_save(const char *path)
{
	FILE *fd = fopen(path, "wt+");
	if (!fd)
		return;

	for (line_t *cur = settings; cur; cur = cur->next) {
		/* Output existing items */
		if (cur->text && !cur->dirty)
			fprintf(fd, "%s\n", cur->text);

		/* Output group and name headers */
		else if (cur->group && cur->name)
			fprintf(fd, "[%s \"%s\"]\n", cur->group, cur->name);

		/* Output group only headers */
		else if (cur->group)
			fprintf(fd, "[%s]\n", cur->group);

		/* Output key/value pairs - todo: add quotes */
		else if (cur->key && cur->value)
			fprintf(fd, "\t%s = %s\n", cur->key, cur->value);

		/* Output blank lines */
		else
			fprintf(fd, "\n");
	}

	fclose(fd);
}

/* Initialize */
void conf_setup(int _argc, char **_argv, const char *_name, parser_t _parser)
{
	const char *home = getenv("HOME");
	filename = alloc0(strlen(home) + 1 + strlen(_name) + 1);
	sprintf(filename, "%s/%s", home, _name);
	parser   = _parser;
	argc     = _argc;
	argv     = _argv;
}

/* Initialize */
void conf_init(void)
{
	conf_load(filename, parser);
}

/* Update */
void conf_sync(void)
{
	if (needsave)
		conf_save(filename);
	needsave = 0;
}

/* Getters */
int get_enum(const char *value, const char **map, int n)
{
	wasfound = 1;
	for (int i = 0; i < n; i++)
		if (match(map[i], value))
			return i;
	conf_error(value);
	return 0;
}

int get_bool(const char *value)
{
	wasfound = 1;
	return get_enum(value, booleans, N_ELEMENTS(booleans));
}

int get_number(const char *value)
{
	wasfound = 1;
	errno = 0;
	int rval = atoi(value);
	if (errno)
		conf_error(value);
	return rval;
}

char *get_string(const char *value)
{
	wasfound = 1;
	return (char*)value;
}

char *get_name(const char *name)
{
	wasfound = 1;
	return (char*)name;
}

/* Setters */
void set_enum(const char *group, const char *name,
		const char *key, int value,
		const char **map, int n)
{
	if (value >= 0 && value < n)
		set_value(group, name, key, map[value]);
}

void set_bool(const char *group, const char *name,
		const char *key, int value)
{
	set_enum(group, name, key, value,
			booleans, N_ELEMENTS(booleans));
}

void set_number(const char *group, const char *name,
		const char *key, int value)
{
	char buf[32];
	snprintf(buf, sizeof(buf), "%d", value);
	set_value(group, name, key, buf);
}

void set_string(const char *group, const char *name,
		const char *key, const char *value)
{
	set_value(group, name, key, value);
}

void set_name(const char *group, const char *name, const char *value)
{
	for (line_t *line = settings; line; line = line->next) {
		if (match(line->group, group) &&
		    match(line->name,  name)) {
			free(line->name);
			line->name = strcopy(value);
			line->dirty = 1;
			break;
		}
	}
}

/* Config parser */
static const char *colors[] = {"red", "green", "blue"};

static int   test_bin = 1;
static int   test_num = 42;
static char *test_str = "str";
static int   test_clr = 0;

static void  test_parser(const char *group, const char *name,
		const char *key, const char *value)
{
	if (match(group, "test")) {
		if (match(key, "bin"))
			test_bin = get_bool(value);
		else if (match(key, "clr"))
			test_clr = get_enum(value, colors, N_ELEMENTS(colors));
		else if (match(key, "num"))
			test_num = get_number(value);
		else if (match(key, "str"))
			test_str = get_string(value);
	}
}

void conf_test(void)
{
	printf("conf_test:\n");

	/* Read values from a file */
	conf_load("data/test.rc", test_parser);

	printf("\nload:\n");
	printf("  bin: %-8d\n", test_bin);
	printf("  clr: %-8s\n", colors[test_clr]);
	printf("  num: %-8d\n", test_num);
	printf("  str: %-8s\n", test_str);

	/* Update values */
	set_bool  ("test", 0,      "bin",  1);
	set_enum  ("test", 0,      "clr",  2, colors, N_ELEMENTS(colors));
	set_number("test", 0,      "num",  -9999);
	set_string("test", 0,      "str",  "hello");

	set_string("test", 0,      "new",  "new0");
	set_string("test", "new",  "new",  "new1");
	set_string("new",  0,      "newa", "new2");
	set_string("new",  0,      "newb", "new3");
	set_string("new",  "new",  "newa", "new4");
	set_string("new",  "new",  "newb", "new5");

	set_name  ("func", "name", "test");

	/* Write back to file */
	conf_save("data/test_out.rc");
}

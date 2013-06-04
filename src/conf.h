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

/* Handlers */
typedef void (*parser_t)(const char *group, const char *name,
		         const char *key, const char *value);

/* Getters */
int get_enum(const char *value, const char **map, int n);
int get_bool(const char *value);
int get_number(const char *value);
char *get_string(const char *value);
char *get_name(const char *name);

/* Setters */
void set_enum(const char *group, const char *name,
		const char *key, int value,
		const char **map, int n);
void set_bool(const char *group, const char *name,
		const char *key, int value);
void set_number(const char *group, const char *name,
		const char *key, int value);
void set_string(const char *group, const char *name,
		const char *key, const char *value);
void set_name(const char *group, const char *name,
		const char *value);

/* Functions */
void conf_setup(int argc, char **argv, const char *name, parser_t parser);
void conf_init(void);
void conf_sync(void);

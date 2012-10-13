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

/* Macros */
#define MIN(a,b) ((a) < (b) ? (a) : (b))
#define MAX(a,b) ((a) > (b) ? (a) : (b))
#define CLAMP(x,l,h) MIN(MAX(x,l),h)
#define ROUND(x) ((int)((x)+0.5))
#define N_ELEMENTS(x) (sizeof(x)/sizeof((x)[0]))

/* Debug functions */
void util_init(void);

/* Misc functions */
char *sdup(const char *str);

/* Debug functions */
#ifdef DEBUG
int debug(char *fmt, ...);
#else
#define debug(...)
#endif

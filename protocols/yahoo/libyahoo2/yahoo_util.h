/*
 * libyahoo2: yahoo_util.h
 *
 * Copyright (C) 2002, Philip S Tellis <philip . tellis AT gmx . net>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */

#ifndef __YAHOO_UTIL_H__
#define __YAHOO_UTIL_H__

#include <stdlib.h>

#ifdef FREE
#undef FREE
#endif

#define FREE(x)	if(x) {free(x); x=NULL;}

#ifndef TRUE
#define TRUE 1
#endif

#ifndef FALSE
#define FALSE 0
#endif

#ifndef MIN
#define MIN(x,y) ((x)<(y)?(x):(y))
#endif

#ifndef MAX
#define MAX(x,y) ((x)>(y)?(x):(y))
#endif

#define y_new(type, n)		(type *)malloc(sizeof(type) * (n))
#define y_new0(type, n)		(type *)calloc((n), sizeof(type))
#define y_renew(type, mem, n)	(type *)realloc(mem, n)

void * y_memdup(const void * addr, int n);
char ** y_strsplit(char * str, char * sep, int nelem);
void y_strfreev(char ** vector);

char * y_string_append(char * str, char * append);

#endif

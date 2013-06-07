/*
 * gaim
 *
 * Copyright (C) 2003
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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */

#ifndef YAHOO_FN_H
#define YAHOO_FN_H

#define IDENT  1 /* identify function */
#define XOR    2 /* xor with arg1 */
#define MULADD 3 /* multipy by arg1 then add arg2 */
#define LOOKUP 4 /* lookup each byte in the table pointed to by arg1 */
#define BITFLD 5 /* reorder bits according to table pointed to by arg1 */

struct yahoo_fn
{
	int type; 
	long arg1, arg2;
};

int yahoo_xfrm( int table, int depth, int seed );

#endif /* YAHOO_FN_H */


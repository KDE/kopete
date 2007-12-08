/**
 * The QQ2003C protocol plugin
 *
 * for gaim
 *
 * Copyright (C) 2004 Puzzlebird
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
 *
 **************************************************
 * Reorganized by Minmin <csdengxm@hotmail.com>, 2005-3-27
 * Refactored by blueangel <blueangel.jin@gmail.com>, 2006-07
 **************************************************
 */

#include "crypt.h"
#include <arpa/inet.h>

void TEA::encipher(unsigned int *const v, const unsigned int *const k, 
			unsigned int *const w)
{
	register unsigned int 
		y     = ntohl(v[0]),
		z     = ntohl(v[1]),
		a     = ntohl(k[0]),
		b     = ntohl(k[1]),
		c     = ntohl(k[2]),
		d     = ntohl(k[3]),
		n     = 0x10,       /* do encrypt 16 (0x10) times */
		sum   = 0,
		delta = 0x9E3779B9; /*  0x9E3779B9 - 0x100000000 = -0x61C88647 */

	while (n-- > 0) {
		sum += delta;
		y += ((z << 4) + a) ^ (z + sum) ^ ((z >> 5) + b);
		z += ((y << 4) + c) ^ (y + sum) ^ ((y >> 5) + d);
	}

	w[0] = htonl(y); w[1] = htonl(z);
}

void TEA::decipher(unsigned int *const v, const unsigned int *const k, 
			unsigned int *const w)
{
	register unsigned int
		y     = ntohl(v[0]),
		z     = ntohl(v[1]),
		a     = ntohl(k[0]),
		b     = ntohl(k[1]),
		c     = ntohl(k[2]),
		d     = ntohl(k[3]),
		n     = 0x10,
		sum   = 0xE3779B90, 
		/* why this ? must be related with n value*/
		delta = 0x9E3779B9;

	/* sum = delta<<5, in general sum = delta * n */
	while (n-- > 0) {
		z -= ((y << 4) + c) ^ (y + sum) ^ ((y >> 5) + d);
		y -= ((z << 4) + a) ^ (z + sum) ^ ((z >> 5) + b);
		sum -= delta;
	}

	w[0] = htonl(y); w[1] = htonl(z);
}


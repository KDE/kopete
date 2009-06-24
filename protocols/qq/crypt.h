#ifndef CRYPT_H
#define CRYPT_H

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

class TEA 
{
public:
	static void encipher(unsigned int *const v, const unsigned int *const k, 
			unsigned int *const w);
	static void decipher(unsigned int *const v, const unsigned int *const k, 
			unsigned int *const w);
};

#endif


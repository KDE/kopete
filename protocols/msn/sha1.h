/*
 * sha1.h - Secure Hash Algorithm 1
 * Copyright (C) 2003  Justin Karneges
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */

#ifndef CS_SHA1_H
#define CS_SHA1_H

#include<qstring.h>

class SHA1
{
public:
	static QByteArray hash(const QByteArray &);
	static QByteArray hashString(const QCString &);
	static QString digest(const QString &);

private:
	SHA1();

	struct SHA1_CONTEXT
	{
		Q_UINT32 state[5];
		Q_UINT32 count[2];
		unsigned char buffer[64];
	};

	typedef union {
		unsigned char c[64];
		Q_UINT32 l[16];
	} CHAR64LONG16;

	void transform(Q_UINT32 state[5], unsigned char buffer[64]);
	void init(SHA1_CONTEXT* context);
	void update(SHA1_CONTEXT* context, unsigned char* data, Q_UINT32 len);
	void final(unsigned char digest[20], SHA1_CONTEXT* context);

	unsigned long blk0(Q_UINT32 i);
	bool bigEndian;

	CHAR64LONG16* block;
};

#endif

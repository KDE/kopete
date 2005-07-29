/*
 * base64.h - Base64 converting functions
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
 * Foundation, Inc., 51 Franklin Steet, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */

#ifndef CS_BASE64_H
#define CS_BASE64_H

#include<qstring.h>

// CS_NAMESPACE_BEGIN

class Base64
{
public:
	static QByteArray encode(const QByteArray &);
	static QByteArray decode(const QByteArray &);
	static QString arrayToString(const QByteArray &);
	static QByteArray stringToArray(const QString &);
	static QString encodeString(const QString &);
};

// CS_NAMESPACE_END

#endif

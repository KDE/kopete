/*
    kbytearrayescaper.cpp -  A small class that allow to (un)escape a QByteArray using custom escape sequences.

    Copyright (c) 2003-2007 by Michel Hermier <michel.hermier@gmail.com>

    Kopete    (c) 2003-2007 by the Kopete developers <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/

#include "kircbytearrayescaper.h"

class KIrc::ByteArrayEscaper::Private
{
public:
	char escape_char;
	char escape[256];
	char reverse[256];
};

using namespace KIrc;

ByteArrayEscaper::ByteArrayEscaper(char escape_char, const ByteArrayEscaper::EscapeList &escapes)
	: d(new ByteArrayEscaper::Private)
{
	reset(escape_char);
	addEscape(escapes);
}

ByteArrayEscaper::~ByteArrayEscaper()
{
	delete d;
}

void ByteArrayEscaper::reset(char escape_char)
{
	d->escape_char = escape_char;
	for (int i=0; i<256; ++i)
		removeEscape(i);
}

void ByteArrayEscaper::addEscape(char escape, char replacement)
{
	d->escape[(uchar)escape] = replacement;
	d->reverse[(uchar)replacement] = escape;
}

void ByteArrayEscaper::addEscape(const ByteArrayEscaper::EscapeList &escapeds)
{
	Q_FOREACH(const ByteArrayEscaper::Escape &escaped, escapeds)
		addEscape(escaped.first, escaped.second);
}

void ByteArrayEscaper::removeEscape(char escape)
{
	addEscape(escape, escape);
}

QByteArray ByteArrayEscaper::escape(const QByteArray &buffer) const
{
#ifdef __GNUC__
	#warning implement me
#endif
	return buffer;

#if 0
	QByteArray quoted;
//	ret.resize(buffer.size()*1.10);

	char escaped;
	foreach (char c, buffer)
	{
		escaped = d->escaped[(uchar)c];
		if(c != escaped || c == d->escape)
			quoted += d->escape;
		quoted += escaped;
	}
	return quoted;
#endif
}

QList<QByteArray> ByteArrayEscaper::escape(const QList<QByteArray> &buffers) const
{
	QList<QByteArray> ret;
	Q_FOREACH(const QByteArray &buffer, buffers)
		ret.append(escape(buffer));
	return ret;
}

QByteArray ByteArrayEscaper::unescape(const QByteArray &buffer) const
{
#ifdef __GNUC__
	#warning implement me
#endif
	return buffer;

#if 0
	QByteArray quoted;
//	ret.resize(buffer.size()*1.10);

	bool escaped = false;
	foreach (char c, buffer)
	{
		if (escaped)
			quoted += d->reverse[(uchar)c];
		escaped = !escaped && c == d->escape;
		if (!escaped)
			quoted += c;
	}
	if (quoted)
		ret += d->escape;

	return quoted;
#endif
}

QList<QByteArray> ByteArrayEscaper::unescape(const QList<QByteArray> &buffers) const
{
	QList<QByteArray> ret;
	Q_FOREACH(const QByteArray &buffer, buffers)
		ret.append(unescape(buffer));
	return ret;
}

QByteArray ByteArrayEscaper::join(const QList<QByteArray> &buffers, char sep) const
{
	QByteArray ret;
	int size = buffers.size();

	if (size > 0)
	{
		ret.append(buffers.at(0));
		for(int i=1; i < size; ++i)
		{
			ret.append(sep);
			ret.append(buffers.at(i));
		}
	}

	return ret;
}

#if 0
QList<QByteArray> ByteArrayEscaper::split(const QByteArray &buffer, char sep) const
{
}
#endif


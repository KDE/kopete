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

#include "kbytearrayescaper.h"

class KByteArrayEscaper::Private
{
public:
	char escape_char;
	char escape[256];
	char reverse[256];
};

KByteArrayEscaper::KByteArrayEscaper(char escape_char, const KByteArrayEscaper::EscapeList &escapes)
	: d(new KByteArrayEscaper::Private)
{
	reset(escape_char);
	addEscape(escapes);
}

KByteArrayEscaper::~KByteArrayEscaper()
{
	delete d;
}

void KByteArrayEscaper::reset(char escape_char)
{
	d->escape_char = escape_char;
	for (int i=0; i<256; ++i)
		removeEscape((char)i);
}

void KByteArrayEscaper::addEscape(char escape, char replacement)
{
	d->escape[escape] = replacement;
	d->reverse[replacement] = escape;
}

void KByteArrayEscaper::addEscape(const KByteArrayEscaper::EscapeList &escapeds)
{
	Q_FOREACH(const KByteArrayEscaper::Escape &escaped, escapeds)
		addEscape(escaped.first, escaped.second);
}

void KByteArrayEscaper::removeEscape(char escape)
{
	addEscape(escape, escape);
}

QByteArray KByteArrayEscaper::escape(const QByteArray &buffer) const
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
		escaped = d->escaped[c];
		if(c != escaped || c == d->escape)
			quoted += d->escape;
		quoted += escaped;
	}
	return quoted;
#endif
}

QList<QByteArray> KByteArrayEscaper::escape(const QList<QByteArray> &buffers) const
{
	QList<QByteArray> ret;
	Q_FOREACH(const QByteArray &buffer, buffers)
		ret.append(escape(buffer));
	return ret;
}

QByteArray KByteArrayEscaper::unescape(const QByteArray &buffer) const
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
			quoted += d->reverse[c];
		escaped = !escaped && c == d->escape;
		if (!escaped)
			quoted += c;
	}
	if (quoted)
		ret += d->escape;

	return quoted;
#endif
}

QList<QByteArray> KByteArrayEscaper::unescape(const QList<QByteArray> &buffers) const
{
	QList<QByteArray> ret;
	Q_FOREACH(const QByteArray &buffer, buffers)
		ret.append(unescape(buffer));
	return ret;
}

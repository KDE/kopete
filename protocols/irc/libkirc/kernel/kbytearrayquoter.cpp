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

KByteArrayEscaper::Private
{
public:
	Private()
		: escape(0)
	{
		reset();
	}

	void reset();
	void setEscape(char escape, char escaped);

	char escape;
	char escaped[256];
	char reverse[256];
};

void KByteArrayEscape::Private::reset(char escape = '\0')
{
	this->escape = escape;
	for (int i=0; i<256; ++i)
	{
	      escaped[i] = reverse[i] = (char)i;
	}
}

void KByteArrayEscape::Private::setEscape(char escape, char escaped)
{
	this->escaped[escape] = escaped;
	this->reverse[escaped] = escape;
}

KByteArrayQuoter::KByteArrayQuoter()
	: d(new KByteArrayEscaper::Private)
{
}
#if 0
KByteArrayEscaper::KByteArrayEscaper(char escaped)
	: d(new KByteArrayEscaper::Private)
{
}
#endif
KByteArrayEscaper::~KByteArrayEscaper()
{
	delete d;
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

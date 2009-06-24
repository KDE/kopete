/*
    kircbytearrayescaper.h - A small class that allow to (un)escape a QByteArray using custom escape sequences.

    Copyright (c) 2005-2007 by Michel Hermier <michel.hermier@gmail.com>

    Kopete    (c) 2005-2007 by the Kopete developers <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/

#ifndef KIRCBYTEARRAYESCAPER_H
#define KIRCBYTEARRAYESCAPER_H

#include "kirc_export.h"

#include <QtCore/QByteArray>
#include <QtCore/QList>
#include <QtCore/QPair>

namespace KIrc
{

class KIRC_EXPORT ByteArrayEscaper
{
public:
	typedef QPair<char, char> Escape;
	typedef QList<Escape> EscapeList;

	explicit ByteArrayEscaper(char escape_char = '\0',
		const ByteArrayEscaper::EscapeList &escapes = ByteArrayEscaper::EscapeList());
	~ByteArrayEscaper();

	void reset(char escape_char = '\0');
	void addEscape(char escaped, char replacement);
	void addEscape(const KIrc::ByteArrayEscaper::EscapeList &escapes);
	void removeEscape(char escape);

	QByteArray escape(const QByteArray &buffer) const;
	QList<QByteArray> escape(const QList<QByteArray> &buffers) const;

	QByteArray unescape(const QByteArray &buffer) const;
	QList<QByteArray> unescape(const QList<QByteArray> &buffers) const;

	QByteArray join(const QList<QByteArray> &buffers, char sep) const;
//	QByteArray join(const QList<QByteArray> &buffers, const char *sep) const;
//	QByteArray join(const QList<QByteArray> &buffers, const QByteArray &sep) const;

//	QList<QByteArray> split(const QByteArray &buffer, char sep) const;

private:
	class Private;
	Private * const d;
};

};

#endif


/*
    kbytearrayescaper.h - A small class that allow to (un)escape a QByteArray using custom escape sequences.

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

#ifndef KBYTEARRAYESCAPER_H
#define KBYTEARRAYESCAPER_H

#include <QByteArray>

class KByteArrayEscaper {

public:
	KByteArrayEscaper();
	~KByteArrayEscaper();

	QByteArray escape(const QByteArray &buffer) const;
	QByteArray unescape(const QByteArray &buffer) const;

private:
	class Private;
	Private * const d;
};

#endif


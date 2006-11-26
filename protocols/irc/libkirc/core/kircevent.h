/*
    kircevent.h - IRC Event.

    Copyright (c) 2003-2005 by Michel Hermier <michel.hermier@wanadoo.fr>

    Kopete    (c) 2003-2005 by the Kopete developers <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/

#ifndef KIRCEVENT_H
#define KIRCEVENT_H

#include "kircmessage.h"

#include "kdemacros.h"

//#include <QList> // From QStringList
#include <QSharedDataPointer>
#include <QStringList>

namespace KIrc
{

class Socket;

class Event
//	: QEvent // Use QEvent interface here ???
{
public:
	Event();
	Event(const Event &o);
	virtual ~Event();

	Event &operator = (const Event &o);

public:
	Message message() const;
	Event &setMessage(const KIrc::Message &message);
/*
	Socket *socket() const;
	Event &setSocket(Socket *socket);
*/
	const QByteArray &from() const;
	Event &setFrom(const QByteArray &from);

	const QList<QByteArray> &to() const;
	Event &setTo(const QList<QByteArray> &to);
/*
	const QList<QByteArray> &cc() const;
	Event &setCc(const QList<QByteArray> &cc);
*/
	const QList<QByteArray> &victims() const;
	Event &setVictims(const QList<QByteArray> &cc);

	QString text() const;
	Event &setText(const QString &text);

private:
	class Private;
	QSharedDataPointer<Private> d;
};

}

#endif

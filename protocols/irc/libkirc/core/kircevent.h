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

#include "kircentity.h"

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
        enum MessageType
        {
		ErrorMessage = -1,

		JoinMessage,
		PartMessage,

		PrivateMessage,
		InfoMessage,
		NoticeMessage,

		WhoMessage,

		MOTDMessage,
		MOTDCondensedMessage
	};

	Event();
	Event(const Event &o);
	virtual ~Event();

	Event &operator = (const Event &o);

public:
/*
	Socket *socket() const;
	Event &setSocket(Socket *socket);
*/
	Event::MessageType messageType() const;
	Event &setMessageType(Event::MessageType messageType);

	Entity::Ptr from() const;
	Event &setFrom(const Entity::Ptr &from);

	Entity::Ptr to() const;
	Event &setTo(const Entity::Ptr &to);

	Entity::List cc() const;
	Event &setCc(const Entity::List &cc);

	QString message() const;
	Event &setMessage(const QString &message);

private:
	class Private;
	QSharedDataPointer<Private> d;
};

}

#endif

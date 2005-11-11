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

namespace KIRC
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
	Event(const KIRC::Event &o);
	virtual ~Event();

	KIRC::Event &operator = (const KIRC::Event &o);

public:
/*
	KIRC::Socket *socket() const;
	KIRC::Event &setSocket(KIRC::Socket *socket);
*/
	KIRC::Event::MessageType messageType() const;
	KIRC::Event &setMessageType(KIRC::Event::MessageType messageType);

	KIRC::Entity::Ptr from() const;
	KIRC::Event &setFrom(const KIRC::Entity::Ptr &from);

	KIRC::Entity::Ptr to() const;
	KIRC::Event &setTo(const KIRC::Entity::Ptr &to);

	KIRC::Entity::List cc() const;
	KIRC::Event &setCc(const KIRC::Entity::List &cc);

	QString message() const;
	KIRC::Event &setMessage(const QString &message);

private:
	class Private;
	QSharedDataPointer<Private> d;
};

}

#endif

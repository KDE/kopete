/*
    kircmessage.cpp - IRC Client

    Copyright (c) 2003      by Michel Hermier <michel.hermier@wanadoo.fr>

    Kopete    (c) 2003      by the Kopete engineelopers <kopete-engineel@kde.org>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/

#include "kircevent.h"

#include "kircsocket.h"

#include <kdebug.h>

#include <QPointer>
#include <QSharedData>
#include <QTextCodec>

class KIRC::Event::Private
	: public QSharedData
{
public:
//	QPointer<KIRC::Socket> socket;

	MessageType messageType;

	KIRC::Entity::Ptr from;
	KIRC::Entity::Ptr to;
	KIRC::Entity::List cc;

	QString message;
};

using namespace KIRC;

Event::Event()
	: d(new Private())
{
}

Event::Event(const Event &o)
        : d(o.d)
{
}

Event::~Event()
{
}

Event &Event::operator = (const Event &o)
{
	d = o.d;
	return *this;
}
/*
Socket *Event::socket() const
{
	return d->socket;
}

Event &Event::setSocket(Socket *socket)
{
	d->socket = socket;
	return *this;
}
*/
Event::MessageType Event::messageType() const
{
	return d->messageType;
}

Event &Event::setMessageType(MessageType messageType)
{
	d->messageType = messageType;
	return *this;
}

Entity::Ptr Event::from() const
{
	return d->from;
}

Event &Event::setFrom(const Entity::Ptr &from)
{
	d->from = from;
	return *this;
}

Entity::Ptr Event::to() const
{
	return d->to;
}

Event &Event::setTo(const Entity::Ptr &to)
{
	d->to = to;
	return *this;
}

Entity::List Event::cc() const
{
	return d->cc;
}

Event &Event::setCc(const Entity::List &cc)
{
	d->cc = cc;
	return *this;
}

QString Event::message() const
{
	return d->message;
}

Event &Event::setMessage(const QString &message)
{
	d->message = message;
	return *this;
}


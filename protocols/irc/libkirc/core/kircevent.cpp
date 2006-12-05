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

class KIrc::Event::Private
	: public QSharedData
{
public:
	Message msg;
//	QPointer<KIrc::Socket> socket;

	MessageType messageType;

	QByteArray from;
	QList<QByteArray> to;
//	QList<QByteArray> cc;
	QList<QByteArray> victims;

	QString text;
};

using namespace KIrc;

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

Message Event::message() const
{
	return d->msg;
}

Event &Event::setMessage(const Message &msg)
{
	d->msg = msg;
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
#if 0
Event::MessageType Event::messageType() const
{
	return d->messageType;
}

Event &Event::setMessageType(MessageType messageType)
{
	d->messageType = messageType;
	return *this;
}
#endif
const QByteArray &Event::from() const
{
	return d->from;
}

Event &Event::setFrom(const QByteArray &from)
{
	d->from = from;
	return *this;
}

const QList<QByteArray> &Event::to() const
{
	return d->to;
}

Event &Event::setTo(const QList<QByteArray> &to)
{
	d->to = to;
	return *this;
}
/*
const QList<QByteArray> &Event::cc() const
{
	return d->cc;
}

Event &Event::setCc(const QList<QByteArray> &cc)
{
	d->cc = cc;
	return *this;
}
*/
const QList<QByteArray> &Event::victims() const
{
	return d->victims;
}

Event &Event::setVictims(const QList<QByteArray> &victims)
{
	d->victims = victims;
	return *this;
}
QString Event::text() const
{
	return d->text;
}

Event &Event::setText(const QString &text)
{
	d->text = text;
	return *this;
}


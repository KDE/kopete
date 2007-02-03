/*
    kircmessage.cpp - IRC Client

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

//	MessageType messageType;

	KIrc::Entity::Ptr from;
	KIrc::Entity::List to;
	KIrc::Entity::List victims;
	KIrc::Entity::List cc;

	bool visible;
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
const KIrc::Entity::Ptr &Event::from() const
{
	return d->from;
}

Event &Event::setFrom(const KIrc::Entity::Ptr &from)
{
	d->from = from;
	return *this;
}

const KIrc::Entity::List &Event::to() const
{
	return d->to;
}

Event &Event::setTo(const KIrc::Entity::List &to)
{
	d->to = to;
	return *this;
}

const KIrc::Entity::List &Event::victims() const
{
	return d->victims;
}

Event &Event::setVictims(const KIrc::Entity::List &victims)
{
	d->victims = victims;
	return *this;
}

const KIrc::Entity::List &Event::cc() const
{
	return d->cc;
}

Event &Event::setCc(const KIrc::Entity::List &cc)
{
	d->cc = cc;
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


/*
    kircevent.h - IRC Event.

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

#ifndef KIRCEVENT_H
#define KIRCEVENT_H

#include "kircmessage.h"

#include "kdemacros.h"

//#include <QList> // From QStringList
#include <QSharedDataPointer>
#include <QStringList>

namespace KIrc
{

class KIRC_EXPORT Event
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

	const KIrc::Entity::Ptr &from() const;
	Event &setFrom(const KIrc::Entity::Ptr &from);

	const KIrc::Entity::List &to() const;
	Event &setTo(const KIrc::Entity::List &to);

	const KIrc::Entity::List &victims() const;
	Event &setVictims(const KIrc::Entity::List &cc);

	const KIrc::Entity::List &cc() const;
	Event &setCc(const KIrc::Entity::List &cc);

	QString text() const;
	Event &setText(const QString &text);

private:
	class Private;
	QSharedDataPointer<Private> d;
};

}

#endif

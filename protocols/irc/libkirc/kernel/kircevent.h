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

#include <QtCore/QVariant>

namespace KIrc
{

class KIRC_EXPORT Event
//	: QEvent // TODO: Use QEvent interface
{
public:
	Event(const QString &name,
		const QString &text, const QList<QVariant> &args,
		const KIrc::Entity::Ptr &from,
		const KIrc::Entity::List &to,
		const KIrc::Entity::List &cc = KIrc::Entity::List());

	/**
	 * The name of the event.
	 */
	const QString &name() const;

	/**
	 * The text associed with this event.
	 *
	 * This text is used to build the displayed text using the arguments.
	 */
	const QString &text() const;

	/**
	 * The arguments of the event.
	 */
	const QList<QVariant> &args() const;

	const KIrc::Entity::Ptr &from() const;

	const KIrc::Entity::List &to() const;

	const KIrc::Entity::List &cc() const;

private:
	QString m_name;
	QString m_text;
	QList<QVariant> m_args;
	KIrc::Entity::Ptr m_from;
	KIrc::Entity::List m_to;
	KIrc::Entity::List m_cc;

};

}

#endif

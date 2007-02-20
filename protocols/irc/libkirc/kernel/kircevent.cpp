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

using namespace KIrc;

Event::Event(const QString &name,
		const QString &text, const QList<QVariant> &args,
		const KIrc::Entity::Ptr &from,
		const KIrc::Entity::List &to,
		const KIrc::Entity::List &cc)
	: m_name(name)
	, m_text(text)
	, m_args(args)
	, m_from(from)
	, m_to(to)
	, m_cc(cc)
{
}

const QString &Event::name() const
{
	return m_name;
}

const QString &Event::text() const
{
	return m_text;
}

const QList<QVariant> &Event::args() const
{
	return m_args;
}

const KIrc::Entity::Ptr &Event::from() const
{
	return m_from;
}

const KIrc::Entity::List &Event::to() const
{
	return m_to;
}

const KIrc::Entity::List &Event::cc() const
{
	return m_cc;
}


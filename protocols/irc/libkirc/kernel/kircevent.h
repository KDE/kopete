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

#ifndef KIRC_EVENTS_H
#define KIRC_EVENTS_H

#include "kircentity.h"
#include "kircmessage.h"

#include <QtCore/QEvent>

namespace KIrc
{

class Socket;

class KIRC_EXPORT CommandEvent
	: public QEvent
{
	static const QEvent::Type Type;
public:
//	explicit CommandEvent();

private:

};

class KIRC_EXPORT MessageEvent
	: public QEvent
{
	static const QEvent::Type Type;

public:
	MessageEvent(const KIrc::Message &message, KIrc::Socket *socket)
		: QEvent(Type), m_message(message), m_socket(socket)
	{ }

	inline const KIrc::Message &message() const { return m_message; }
	inline KIrc::Socket *socket() { return m_socket; }

private:
	KIrc::Message m_message;
	KIrc::Socket *m_socket; // Use QPointer instead?
};

class KIRC_EXPORT TextEvent
	: public QEvent
{
public:
	static const QEvent::Type Type;
/*
	enum Verbosity
	{
		Error,
		Warning,
		Normal,
		Verbose,
		Debug
	};
*/
	TextEvent(const QString &eventId, const KIrc::EntityPtr from, const KIrc::EntityPtr to, const QString &text)
		: QEvent(Type), m_eventId(eventId), m_from(from), m_to(KIrc::EntityList() << to), m_text(text)
	{ }

	TextEvent(const QString &eventId, const KIrc::EntityPtr &from, const KIrc::EntityList &to, const QString &text)
		: QEvent(Type), m_eventId(eventId), m_from(from), m_to(to), m_text(text)
	{ }

	QString eventId() const { return m_eventId; }
	KIrc::EntityPtr from() const { return m_from; }
	KIrc::EntityList to() const { return m_to; }
	QString text() const { return m_text; }

private:
	QString m_eventId;
	KIrc::EntityPtr m_from;
	KIrc::EntityList m_to;
	QString m_text;
};

}

#endif

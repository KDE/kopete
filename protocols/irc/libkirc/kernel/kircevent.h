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

#ifndef KIRC_EVENT_H
#define KIRC_EVENT_H

#include "kircmessage.h"

#include <QtCore/QEvent>

namespace KIrc
{

class Socket;

class KIRC_EXPORT Event
	: public QEvent
{
public:
	enum Type
	{
		Command                 = 10, // Text command

		MessageReceived         = 20, // Generic message, usually a command message
		MessageSending		= 21,
//		MessageDispatch		= 22, // Message as to be redispatched to other sockets.

//		ClientServerReply       = 21, // Numeric reply message in the 001-099 range
//		CommandReply            = 22, // Numeric reply message in the 200-399 range
//		ErrorReply              = 23  // Numeric reply message in the 400-599 range

		Text
	};

public:
	explicit Event(Type type)
		: QEvent(static_cast<QEvent::Type>(type))
	{ }

	Event(const Event &o);
};

class KIRC_EXPORT CommandEvent
	: public KIrc::Event
{
public:
//	explicit CommandEvent();

private:
};

class KIRC_EXPORT MessageEvent
	: public Event
{
public:
	MessageEvent(Type type, const KIrc::Message &message, KIrc::Socket *socket)
		: KIrc::Event(type), m_message(message), m_socket(socket)
	{ }

	inline const KIrc::Message &message() const { return m_message; }
	inline KIrc::Socket *socket() { return m_socket; }

private:
	KIrc::Message m_message;
	KIrc::Socket *m_socket; // Use QPointer instead?
};

class KIRC_EXPORT TextEvent
	: public KIrc::Event
{
public:
	enum Verbosity
	{
		Error,
		Warning,
		Normal,
		Verbose,
		Debug
	};


	TextEvent(const QString &text, Verbosity verbosity = Normal)
		: Event(Text), m_text(text), m_verbosity(verbosity)
	{ }

private:
	QString m_text;
	Verbosity m_verbosity;
};

}

#endif

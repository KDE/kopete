/*
    kircmessageredirector.cpp - IRC Client

    Copyright (c) 2004      by Michel Hermier <michel.hermier@wanadoo.fr>

    Kopete    (c) 2004      by the Kopete developers <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/

#include "kircengine.h"
#include "kircmessage.h"
#include "kircmessageredirector.h"

using namespace KIRC;

MessageRedirector::MessageRedirector(KIRC::Engine *engine,
	int argsSize_min, int argsSize_max, const QString &helpMessage)
	: QObject(engine, "KIRC::MessageRedirector"),
	  m_argsSize_min(argsSize_min),
	  m_argsSize_max(argsSize_max),
	  m_helpMessage(helpMessage)
{
}

bool MessageRedirector::connect(QObject *object, const char *member)
{
	return QObject::connect(this, SIGNAL(redirect(KIRC::Message &)),
					object, member);
}

QStringList MessageRedirector::operator () (Message &msg)
{
	m_errors.clear();

//	if (m_connectedObjects == 0)
//		m_errors.append(i18n("Internal error: no more connected object, triggered by:")+msg);

	if (checkValidity(msg))
		emit redirect(msg);

	return m_errors;
}

QString MessageRedirector::helpMessage()
{
	return m_helpMessage;
}

void MessageRedirector::error(QString &message)
{
	m_errors.append(message);
}

bool MessageRedirector::checkValidity(const Message &msg)
{
	bool success = true;
	int argsSize = msg.argsSize();

	if (m_argsSize_min >= 0 && argsSize < m_argsSize_min)
	{
//		m_errors.append(i18n("Not enougth arguments in message:")+msg);
		success = false;
	}

#ifdef _IRC_STRICTNESS_
	if (m_argsSize_max >= 0 && argsSize > m_argsSize_max)
	{
//		m_errors.append(i18n("Too many arguments in message:")+msg);
		success = false;
	}
#endif
/*
	if ( msg.isNumeric() &&
		( msg.argsSize() > 0 && (
			msg.arg(0) == m_Nickname ||
			msg.arg(0) == m_PendingNick ||
			msg.arg(0) == QString::fromLatin1("*")
			)
		)
	)
	{
//		m_errors.append(i18n("Too many arguments in message:")+msg);
		success = false;
	}
*/
	return success;
}

#include "kircmessageredirector.moc"

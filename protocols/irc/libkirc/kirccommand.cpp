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

#include "kirccommand.h"

#include "kircengine.h"

using namespace KIRC;

Command::Command(QObject *parent)
	: QObject(parent)
{
}

int Command::connected() const
{
	return receivers(SIGNAL(redirect(KIRC::Message &)));
}

QStringList Command::operator () (Message &msg)
{
	m_errors.clear();

//	if (m_connectedObjects == 0)
//		m_errors.append(i18n("Internal error: no more connected object, triggered by:")+msg);

	if (checkValidity(msg))
		emit redirect(msg);

	return m_errors;
}

QString Command::helpMessage()
{
	return m_helpMessage;
}

void Command::error(QString &message)
{
	m_errors.append(message);
}

bool Command::checkValidity(const Message &msg)
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

#include "kirccommand.moc"


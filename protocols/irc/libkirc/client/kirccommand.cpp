/*
    kircmessageredirector.cpp - IRC Client

    Copyright (c) 2007      by Michel Hermier <michel.hermier@gmail.com>

    Kopete    (c) 2007      by the Kopete developers <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/

#include <QStringList>

class KIrc::Command::Private
{
public:
	int argsSize_min;
	int argsSize_max;
	QString helpMessage;
};

using namespace KIrc;

QString Command::expand(QString command, QString args)
{
#ifdef __GNUC__
        #warning IMPLEMENT ME
#endif
        return command;
}

Command::Command(QObject *parent)
	: QObject(parent),
	  d(new Private)
{
}

Command::~Command()
{
	delete d;
}

QString Command::help() const
{
        return d->helpMessage;
}

void Command::setHelp(const QString &helpMessage)
{
	d->helpMessage = helpMessage;
}

void Command::handleMessage(Message msg)
{
	if (checkValidity(msg))
		emit redirect(msg);
}

bool Command::checkValidity(const Message &msg)
{
	int argsSize = msg.argsSize();
/*
//	if (m_connectedObjects == 0)
//		m_errors.append(i18n("Internal error: no more connected object, triggered by:")+msg);

	if (m_argsSize_min >= 0 && argsSize < m_argsSize_min)
	{
//		m_errors.append(i18n("Not enougth arguments in message:")+msg);
		return false;
	}

#ifdef _IRC_STRICTNESS_
	if (m_argsSize_max >= 0 && argsSize > m_argsSize_max)
	{
//		m_errors.append(i18n("Too many arguments in message:")+msg);
		return false;
	}
#endif

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
		return false;
	}
*/
	return true;
}


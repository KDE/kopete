/*
    kircmessageredirector.h - IRC Client

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

#ifndef KIRC_MESSAGEREDIRECTOR_H
#define KIRC_MESSAGEREDIRECTOR_H

#include <qobject.h>
#include <qstring.h>

namespace KIRC
{

class Engine;

class Message;

class MessageRedirector
	: public QObject
{
	Q_OBJECT

public:
	enum {
		Unknown = -1,
		Unlimited = -2
	};

	MessageRedirector(KIRC::Engine *engine,
		int argsSize_min = KIRC::MessageRedirector::Unknown,
		int argsSize_max = KIRC::MessageRedirector::Unknown,
		const QString &helpMessage = QString::null);

	/**
	 * Connects the given object member signal/slot to this message redirector.
	 * The member signal slot should be looking like:
	 * SIGNAL(mysignal(KIRC::Message &msg))
	 * or
	 * SIGNAL(myslot(KIRC::Message &msg))
	 */
	bool connect(QObject *object, const char *member);

	/**
	 * Attempt to send the message.
	 * @return a not empty QStringList on errors or no slots connected.
	 * 	The returned string list contains all the errors.
	 */
	QStringList operator()(KIRC::Message &msg);

	void error(QString &errorMessage);

	QString helpMessage();

signals:
	void redirect(KIRC::Message &);

private:
	/**
	 * Check that the given message as the correct number of args
	 * and do some message format checks.
	 */
	bool checkValidity(const KIRC::Message &msg);

	QStringList m_errors;

	int m_argsSize_min;
	int m_argsSize_max;
	QString m_helpMessage;
};

}

#endif

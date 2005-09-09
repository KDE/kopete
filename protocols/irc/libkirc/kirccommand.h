/*
    kircmessageredirector.h - IRC Client

    Copyright (c) 2004-2005 by Michel Hermier <michel.hermier@wanadoo.fr>

    Kopete    (c) 2004-2005 by the Kopete developers <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/

#ifndef KIRC_COMMAND_H
#define KIRC_COMMAND_H

#include <QObject>

namespace KIRC
{

class Engine;

class Message;

class Command
	: public QObject
{
	Q_OBJECT

//	Q_PROPERTY(int min READ min WRITE setMin)
//	Q_PROPERTY(int max READ max WRITE setMax)
//	Q_PROPERTY(QString help READ help WRITE setHelp)

public:
	enum {
		Unknown = -1,
		Unlimited = -2
	};

	Command(KIRC::Engine *engine,
		int argsSize_min = KIRC::MessageRedirector::Unknown,
		int argsSize_max = KIRC::MessageRedirector::Unknown,
		const QString &helpMessage = QString::null);

public: // READ properties accessors.
//	int min()
//	int max()
	QString help();

public slots: // WRITE properties accessors.
//	Command &setMin();
//	Command &setMax();
	Command &setHelp(const QString &help);

public:

//	Command &setMinMax();

	/**
	 * Attempt to send the message.
	 * @return a not empty QStringList on errors or no slots connected.
	 * 	The returned string list contains all the errors.
	 */
	QStringList invoke(KIRC::Message &msg);

	void error(QString &errorMessage);

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

class CommandManager
	: public QObject
{
	Q_OBJECT

public:
	CommandManager(QObject *parent);
	~CommandManager();

	Command *registerCommand(Command *command);

	/**
	 * Connects the given object member signal/slot to this message redirector.
	 * The member signal slot should be looking like:
	 * SIGNAL(mysignal(KIRC::Message &msg))
	 * or
	 * SIGNAL(myslot(KIRC::Message &msg))
	 */
	Command *registerCommand(QObject *object, const char *member);

	void unregisterCommand(Command *command);

private:
//	QMultiMap<QByteArray name, Command *> commands;
};

}

#endif

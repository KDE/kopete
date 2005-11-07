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

#ifndef KIRC_COMMANDHANDLER_H
#define KIRC_COMMANDHANDLER_H

#include <QObject>

namespace KIRC
{

class Command;

class CommandHandler
	: public QObject
{
	Q_OBJECT

public:
	CommandHandler(QObject *parent = 0);
	~CommandHandler();

public slots:
	Command *registerCommand(const QString &name, Command *command);

	/**
	 * Connects the given object member signal/slot to this message redirector.
	 * The member signal slot should be looking like:
	 * SIGNAL(mysignal(KIRC::Message &msg))
	 * or
	 * SIGNAL(myslot(KIRC::Message &msg))
	 */
	Command *registerCommand(const QString &name, QObject *object, const char *member);

//	virtual void handleMessage(KIRC::Message &msg);

	void unregisterCommand(Command *command);

private:
	Q_DISABLE_COPY(CommandHandler)

	class Private;
	Private * const d;
};

}

#endif

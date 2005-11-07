/*
    kirccommandhandler.cpp - IRC Client command hanler.

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

#include "kirccommandhandler.moc"

#include "kirccommand.h"
#include "kircsocket.h"

#include <QMultiHash>

class KIRC::CommandHandler::Private 
{
public:
	QMultiHash<QString, Command *> commands;
};

using namespace KIRC;

CommandHandler::CommandHandler(QObject *parent)
	: QObject(parent)
	, d(new Private)
{
}

CommandHandler::~CommandHandler()
{
	delete d;
}

Command *CommandHandler::registerCommand(const QString &name, Command *command)
{
}

Command *CommandHandler::registerCommand(const QString &name, QObject *object, const char *member)
{
}

void CommandHandler::handleMessage(Message msg)
{
	QList<Command *> commands = d->commands.values(msg.command());
	if (commands.isEmpty())
	{
//		emit unhandledMessage(msg);
	}
	else
	{
		foreach(Command *command, commands)
			command->handleMessage(msg);
	}
}

void CommandHandler::unregisterCommand(Command *command)
{
}


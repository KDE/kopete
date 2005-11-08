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

using namespace KIRC;

CommandHandler::CommandHandler(QObject *parent)
	: QObject(parent)
{
}

CommandHandler::~CommandHandler()
{
}

Command *CommandHandler::registerCommand(const QString &name, Command *command)
{
	if (name.isEmpty() || !command)
		return 0;

	if (!m_commands.values(name).contains(command))
	{
		m_commands.insertMulti(name, command);
		connect(command, SIGNAL(destroyed()),
			this, SLOT(cleanup()));
	}
	return command;
}

Command *CommandHandler::registerCommand(const QString &name, QObject *object, const char *member)
{
//	Command command = new Command()
//	return registerCommand(name, new Command);
}

void CommandHandler::handleMessage(Message msg)
{
	QList<Command *> commands = m_commands.values(msg.command());
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


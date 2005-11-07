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

#include "kircsocket.h"

#include <QMultiMap>

class KIRC::CommandHandler::Private 
{
public:
	QMultiMap<QString, Command *> registry;
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
/*
void CommandHandler::handleMessage(Message &msg)
{
}
*/
void CommandHandler::unregisterCommand(Command *command)
{
}


/*
    kircsimpletask.cpp - IRC Simple Task.

    Copyright (c) 2006      by Michel Hermier <michel.hermier@wanadoo.fr>

    Kopete    (c) 2006      by the Kopete developers <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/

#include "kircsimpletask.moc"

#include <kdebug.h>

using namespace KIrc;

// TODO ?? populate hash with the method instead of relying on qt invocation method
SimpleTask::SimpleTask(QObject *parent)
	: Task(parent)
	, d(0)
{
}

SimpleTask::~SimpleTask()
{
//	delete d;
}

Task::Status SimpleTask::doCommand(Message msg)
{
        return doMessage("doCommand_", msg);
}

Task::Status SimpleTask::doMessage(Message msg)
{
        return doMessage("doMessage_", msg);
}

Task::Status SimpleTask::doMessage(const char *commandPrefix, Message msg)
{
        if (!msg.isValid())
        {
                return NotHandled;
        }

        Status handled;
        QByteArray methodName = commandPrefix + msg.rawCommand();
        if (QMetaObject::invokeMethod(this, methodName.constData(), Qt::DirectConnection, Q_RETURN_ARG(KIrc::Task::Status, handled), Q_ARG(KIrc::Message, msg)))
        {
                return handled;
        }

        return NotHandled;
}


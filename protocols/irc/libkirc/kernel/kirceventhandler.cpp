/*
    kirceventhandler.cpp - IRC event handler.

    Copyright (c) 2008      by Michel Hermier <michel.hermier@gmail.com>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/

#include "kirceventhandler.moc"

#include "kircevent.h"

class KIrc::EventHandlerPrivate
{
public:
	bool enabled;
};

using namespace KIrc;

// TODO ?? populate hash with the method instead of relying on qt invocation method
EventHandler::EventHandler(QObject *parent)
	: QObject(parent)
	, d_ptr(new EventHandlerPrivate)
{
	Q_D(EventHandler);
	d->enabled = false;
}

EventHandler::~EventHandler()
{
	delete d_ptr;
}

bool EventHandler::isEnabled() const
{
	Q_D(const EventHandler);

	return d->enabled;
}

void EventHandler::setEnabled(bool enabled)
{
	Q_D(EventHandler);

	d->enabled = enabled;
}

bool EventHandler::eventFilter(QObject *watched, QEvent *event)
{
	Q_D(EventHandler);

	if (d->enabled)
	{
	}
	return false;
}

bool EventHandler::commandEvent(KIrc::CommandEvent *ev)
{
#if 0
	bool consumed = false;
	QByteArray methodName = msg->arg(0)->upper();
	if (QMetaObject::invokeMethod(this, message->args(0)), Qt::DirectConnection,
			Q_RETURN_ARG(bool, consumed),
			Q_ARG(KIrc::Contect *, context),
			Q_ARG(KIrc::Message *, command))
		return consumed;
#endif

	return false;
}

bool EventHandler::messageEvent(KIrc::MessageEvent *ev)
{
#if 0
	bool consumed = false;
	QByteArray methodName = msg->arg(0)->upper();
	if (QMetaObject::invokeMethod(this, message->args(0)), Qt::DirectConnection,
			Q_RETURN_ARG(bool, consumed),
			Q_ARG(KIrc::Contect *, context),
			Q_ARG(KIrc::Message *, message))
		return consumed;
#endif

	return false;
}

/*
Command *EventHandler::registerCommand(const QString &name, Command *command)
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

Command *EventHandler::registerCommand(const QString &name, QObject *object, const char *member)
{
//      Command command = new Command();
//	return registerCommand(name, new Command);
}

void EventHandler::handleMessage(Message msg)
{
	QList<Command *> commands = m_commands.values(msg.command());
	if (commands.isEmpty())
	{
		// emit unhandledMessage(msg);
	}
	else
	{
		foreach(Command *command, commands)
			command->handleMessage(msg);
	}
}

void EventHandler::unregisterCommand(Command *command)
{
}
*/

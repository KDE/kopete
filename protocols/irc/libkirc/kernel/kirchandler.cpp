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

#include "kirchandler.moc"

#include "kircevent.h"

class KIrc::HandlerPrivate
{
public:
	bool enabled;
};

using namespace KIrc;

// TODO ?? populate hash with the method instead of relying on qt invocation method
Handler::Handler(QObject *parent)
	: QObject(parent)
	, d_ptr(new HandlerPrivate)
{
	Q_D(Handler);
	d->enabled = false;
}

Handler::~Handler()
{
	delete d_ptr;
}

bool Handler::isEnabled() const
{
	Q_D(const Handler);

	return d->enabled;
}

void Handler::setEnabled(bool enabled)
{
	Q_D(Handler);

	d->enabled = enabled;
}

#if 0
Command *Handler::registerCommand(const QString &name, Command *command)
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

Command *Handler::registerCommand(const QString &name, QObject *object, const char *member)
{
//      Command command = new Command();
//	return registerCommand(name, new Command);
}

#endif

Handler::Handled Handler::onCommand(KIrc::Context *context, const QList<QByteArray> &command/*, KIrc::Entity::Ptr from*/)
{
	Q_D(Handler);

	Handled handled = NotHandled;

	if (isEnabled())
	{
#if 0
		QMetaObject::invokeMethod(this, message->args(0)->upper(), Qt::DirectConnection,
			Q_RETURN_ARG(KIrc::Handler::Handled, handled),
			Q_ARG(KIrc::Contect *, context),
			Q_ARG(QList<QByteArray>, command));
//			Q_ARG(KIrc::Entity::Ptr, from));
#endif
	}
	return handled;
}

#if 0
void Handler::unregisterCommand(Command *command)
{
}
#endif

#if 0
void Handler::registerMessage()
{
}
#endif

Handler::Handled Handler::onMessage(KIrc::Context *context, const KIrc::Message &message, KIrc::Socket *socket)
{
	Handled handled = NotHandled;

	if (isEnabled())
	{
#if 0
		QMetaObject::invokeMethod(this, message->args(0)->upper(), Qt::DirectConnection,
			Q_RETURN_ARG(KIrc::Handler::Handled, handled),
			Q_ARG(KIrc::Contect *, context),
			Q_ARG(const KIrc::Message &, message),
			Q_ARG(KIrc::Socket *, socket));
#endif
	}
	return handled;
}

#if 0
void Handler::unregisterMessage()
{
}
#endif
/*
void Handler::handleMessage(Message msg)
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

*/

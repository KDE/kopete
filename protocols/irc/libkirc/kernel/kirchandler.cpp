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

#include "kirchandler_p.h"
#include "kirchandler.moc"

#include <QtCore/QMultiHash>
#include <kdebug.h>

using namespace KIrc;

// TODO ?? populate hash with the method instead of relying on qt invocation method
Handler::Handler(QObject *parent)
	: QObject(parent)
	, d_ptr(new HandlerPrivate)
{
}

Handler::Handler(Handler *parent)
	: QObject(parent)
	, d_ptr(new HandlerPrivate)
{
	parent->addEventHandler(this);
}

Handler::Handler(HandlerPrivate* d,Handler *parent)
	: QObject(parent)
	, d_ptr(d)
{
	parent->addEventHandler(this);	
}

Handler::Handler(HandlerPrivate *d, QObject *parent)
	: QObject(parent)
	, d_ptr(d)
{
}

Handler::~Handler()
{
	delete d_ptr;
}

void Handler::addEventHandler(Handler *handler)
{
	Q_D(Handler);
	d->eventHandlers.append(handler);
}

void Handler::removeEventHandler(Handler *handler)
{
	Q_D(Handler);
	d->eventHandlers.removeAll(handler);
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
	//Enable/Disable also the children
	foreach(Handler * handler, d->eventHandlers)
	{
		handler->setEnabled(enabled);
	}
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

void Handler::unregisterCommand(Command *command)
{
}
#endif

void Handler::registerCommandAlias(const QByteArray &alias, const QByteArray &command)
{
	Q_D(Handler);
	d->commandAliases.insert(alias.toUpper(), command.toUpper());
}

Handler::Handled Handler::onCommand(KIrc::Context *context, const QList<QByteArray> &command/*, KIrc::Entity::Ptr from*/)
{
	if (!isEnabled())
		return NotHandled;

	Q_D(Handler);
	Handled handled = NotHandled;

	foreach(KIrc::Handler * handler, d->eventHandlers)
	{
		handled = handler->onCommand(context, command/*, from*/);
		if (handled != NotHandled)
			return handled;
	}

	QGenericReturnArgument ret = Q_RETURN_ARG(KIrc::Handler::Handled, handled);
	QGenericArgument arg0 = Q_ARG(KIrc::Context *, context);
	QGenericArgument arg1 = Q_ARG(QList<QByteArray>, command); // Should be implemented as (const QList<QByteArray> &)
//	QGenericArgument arg2 = Q_ARG(KIrc::Entity *);

	QByteArray cmd = command.value(0).toUpper();
	if (QMetaObject::invokeMethod(this, cmd, Qt::DirectConnection, ret, arg0, arg1/*, arg2*/))
		if (handled != NotHandled)
			return handled;

	foreach(const QByteArray &alias, d->commandAliases.values(cmd))
	{
		if (QMetaObject::invokeMethod(this, alias, Qt::DirectConnection, ret, arg0, arg1/*, arg2*/))
			if (handled != NotHandled)
				return handled;
	}
	return handled;
}

#if 0
void Handler::registerMessage()
{
}

void Handler::unregisterMessage(Message msg)
{
}
#endif

void Handler::registerMessageAlias(const QByteArray &alias, const QByteArray &message)
{
	Q_D(Handler);
	d->messageAliases.insert(alias.toUpper(), message.toUpper());
}

Handler::Handled Handler::onMessage(KIrc::Context *context, const KIrc::Message &message, KIrc::Socket *socket)
{
	if (!isEnabled())
		return NotHandled;

	Q_D(Handler);
	Handled handled = NotHandled;

	foreach(KIrc::Handler * handler, d->eventHandlers)
	{
		handled = handler->onMessage(context, message, socket);
		if (handled != NotHandled)
			return handled;
	}

	QGenericReturnArgument ret = Q_RETURN_ARG(Handler::Handled, handled);
	QGenericArgument arg0 = Q_ARG(KIrc::Context *, context);
	QGenericArgument arg1 = Q_ARG(KIrc::Message, message); // Should be implemented as (const KIrc::Message &)
	QGenericArgument arg2 = Q_ARG(KIrc::Socket *, socket);

	QByteArray msg = message.argAt(0).toUpper();

	//Check if it's a numeric reply
	// FIXME: This is an old temporary solution that was backported for simplicity. One should port such 
	//        code to use the alias system to use named numbers that can found in RFC and servers implementations
	QByteArray msgToExecute=msg;
	bool isNumeric=false;
	int reply=msg.toInt( &isNumeric );
	if ( isNumeric )
		msgToExecute.prepend( "numericReply_" ); //add a prefix, because a slot name cannot be just a number

	if (QMetaObject::invokeMethod(this, msgToExecute, Qt::DirectConnection, ret, arg0, arg1, arg2))
		if (handled != NotHandled)
			return handled;

	foreach(const QByteArray &alias, d->messageAliases.values(msg))
	{
		if (QMetaObject::invokeMethod(this, alias, Qt::DirectConnection, ret, arg0, arg1, arg2))
			if (handled != NotHandled)
				return handled;
	}
	return handled;
}


/*
    kopetemessagefilter.cpp - Kopete Message Filtering

    Copyright (c) 2004      by Richard Smith         <kde@metafoo.co.uk>
    Kopete    (c) 2002-2004 by the Kopete developers <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This library is free software; you can redistribute it and/or         *
    * modify it under the terms of the GNU Lesser General Public            *
    * License as published by the Free Software Foundation; either          *
    * version 2 of the License, or (at your option) any later version.      *
    *                                                                       *
    *************************************************************************
*/

#include "kopetesimplemessagehandler.h"
#include "kopetemessageevent.h"

#include <qpointer.h>

namespace Kopete
{

//BEGIN SimpleMessageHandlerFactory

class SimpleMessageHandlerFactory::Private
{
public:
	Message::MessageDirection direction;
	int position;
	QPointer<QObject> target;
	const char *slot;
};

SimpleMessageHandlerFactory::SimpleMessageHandlerFactory( Message::MessageDirection direction,
	int position, QObject *target, const char *slot )
 : d( new Private )
{
	d->direction = direction;
	d->position = position;
	d->target = target;
	d->slot = slot;
}

SimpleMessageHandlerFactory::~SimpleMessageHandlerFactory()
{
	delete d;
}

MessageHandler *SimpleMessageHandlerFactory::create( ChatSession *manager, Message::MessageDirection direction )
{
	Q_UNUSED( manager )
	if ( direction != d->direction )
		return 0;
	MessageHandler *handler = new SimpleMessageHandler;
	QObject::connect( handler, SIGNAL(handle(Kopete::Message&)), d->target, d->slot );
	return handler;
}

int SimpleMessageHandlerFactory::filterPosition( ChatSession *manager, Message::MessageDirection direction )
{
	Q_UNUSED( manager )
	if ( direction != d->direction )
		return StageDoNotCreate;
	return d->position;
}

//END SimpleMessageHandlerFactory

//BEGIN SimpleMessageHandler

class SimpleMessageHandler::Private
{
};

SimpleMessageHandler::SimpleMessageHandler()
	: d(0)
{
}

SimpleMessageHandler::~SimpleMessageHandler()
{
	delete d;
}

void SimpleMessageHandler::handleMessage( MessageEvent *event )
{
	Message message = event->message();
	emit handle( message );
	event->setMessage( message );
	MessageHandler::handleMessage( event );
}

//END SimpleMessageHandler

}

#include "kopetesimplemessagehandler.moc"

// vim: set noet ts=4 sts=4 sw=4:

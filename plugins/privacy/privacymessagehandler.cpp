/*
    privacymessagehandler.cpp - Kopete Message Filtering

    Copyright (c) 2006 by Andre Duffeck <duffeck@kde.org>
    Kopete    (c) 2002-2006 by the Kopete developers <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This library is free software; you can redistribute it and/or         *
    * modify it under the terms of the GNU Lesser General Public            *
    * License as published by the Free Software Foundation; either          *
    * version 2 of the License, or (at your option) any later version.      *
    *                                                                       *
    *************************************************************************
*/

#include "privacymessagehandler.h"
#include "kopetemessageevent.h"

#include <kdebug.h>
#include <QPointer>

class PrivacyMessageHandlerFactory::Private
{
public:
	Message::MessageDirection direction;
	int position;
	QPointer<QObject> target;
	const char *slot;
};

PrivacyMessageHandlerFactory::PrivacyMessageHandlerFactory( Message::MessageDirection direction,
	int position, QObject *target, const char *slot )
 : d( new Private )
{
	d->direction = direction;
	d->position = position;
	d->target = target;
	d->slot = slot;
}

PrivacyMessageHandlerFactory::~PrivacyMessageHandlerFactory()
{
	delete d;
}

MessageHandler *PrivacyMessageHandlerFactory::create( ChatSession *manager, Message::MessageDirection direction )
{
	Q_UNUSED( manager )
	if ( direction != d->direction )
		return 0;
	MessageHandler *handler = new PrivacyMessageHandler;
	QObject::connect( handler, SIGNAL(handle(Kopete::MessageEvent*)), d->target, d->slot );
	return handler;
}

int PrivacyMessageHandlerFactory::filterPosition( ChatSession *manager, Message::MessageDirection direction )
{
	Q_UNUSED( manager )
	if ( direction != d->direction )
		return StageDoNotCreate;
	return d->position;
}

PrivacyMessageHandler::PrivacyMessageHandler()
{
}

PrivacyMessageHandler::~PrivacyMessageHandler()
{
}

void PrivacyMessageHandler::handleMessage( MessageEvent *e )
{
	QPointer< MessageEvent > event = e;
	emit handle( e );
	if( event )
	{
		kDebug(14313) << "MessageEvent still there!";
		MessageHandler::handleMessage( event );
	}
	else
		kDebug(14313) << "MessageEvent destroyed!";
}

#include "privacymessagehandler.moc"

// vim: set noet ts=4 sts=4 sw=4:

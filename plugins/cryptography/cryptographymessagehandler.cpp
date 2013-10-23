/*
    cryptographymessagehandler.cpp - Kopete Message Filtering

    This file is borrowed from privacymessagefilter.cpp in the Kopete Privacy plugin

    Copyright (c) 2006 by Andre Duffeck <andre@duffeck.de>
    Kopete    (c) 2002-2007 by the Kopete developers <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This library is free software; you can redistribute it and/or         *
    * modify it under the terms of the GNU Lesser General Public            *
    * License as published by the Free Software Foundation; either          *
    * version 2 of the License, or (at your option) any later version.      *
    *                                                                       *
    *************************************************************************
*/

#include "cryptographymessagehandler.h"
#include "kopetemessageevent.h"

#include <kdebug.h>
#include <QPointer>

class CryptographyMessageHandlerFactory::Private
{
public:
	Message::MessageDirection direction;
	int position;
	QPointer<QObject> target;
	const char *slot;
};

CryptographyMessageHandlerFactory::CryptographyMessageHandlerFactory( Message::MessageDirection direction,
	int position, QObject *target, const char *slot )
 : d( new Private )
{
	d->direction = direction;
	d->position = position;
	d->target = target;
	d->slot = slot;
}

CryptographyMessageHandlerFactory::~CryptographyMessageHandlerFactory()
{
	delete d;
}

MessageHandler *CryptographyMessageHandlerFactory::create( ChatSession *manager, Message::MessageDirection direction )
{
	Q_UNUSED( manager )
	if ( direction != d->direction )
		return 0;
	MessageHandler *handler = new CryptographyMessageHandler;
	QObject::connect( handler, SIGNAL(handle(Kopete::MessageEvent*)), d->target, d->slot );
	return handler;
}

int CryptographyMessageHandlerFactory::filterPosition( ChatSession *manager, Message::MessageDirection direction )
{
	Q_UNUSED( manager )
	if ( direction != d->direction )
		return StageDoNotCreate;
	return d->position;
}

CryptographyMessageHandler::CryptographyMessageHandler()
{
}

CryptographyMessageHandler::~CryptographyMessageHandler()
{
}

void CryptographyMessageHandler::handleMessage( MessageEvent *e )
{
	QPointer< MessageEvent > event = e;
	emit handle( e );
	if( event )
	{
		kDebug(14303) << "MessageEvent still there!";
		MessageHandler::handleMessage( event );
	}
	else
		kDebug(14303) << "MessageEvent destroyed!";
}

#include "cryptographymessagehandler.moc"

// vim: set noet ts=4 sts=4 sw=4:

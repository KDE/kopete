/*
    kopetemessagehandlerchain.h - Kopete Message Handler Chain

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

#include "kopetemessagehandlerchain.h"
#include "kopetemessagehandler.h"
#include "kopetemessageevent.h"

#include <kdebug.h>

namespace Kopete
{

class MessageHandlerChain::Private
{
public:
	MessageHandler *first;
	MessageHandler *last;
};

class MessageHandlerChainTerminator : public MessageHandler
{
public:
	MessageHandlerChainTerminator() : MessageHandler(0) {}
	void handleMessage( MessageEvent *event )
	{
		kdError( 14000 ) << k_funcinfo << "message got to end of chain!" << endl;
	}
	int capabilities()
	{
		kdError( 14000 ) << k_funcinfo << "request got to end of chain!" << endl;
		return 0;
	}
};

MessageHandlerChain::MessageHandlerChain( QObject *parent, const char *name )
 : QObject( parent, name ), d( new Private )
{
	d->first = d->last = new MessageHandlerChainTerminator;
}

MessageHandlerChain::~MessageHandlerChain()
{
	MessageHandler *handler = d->first;
	while( handler )
	{
		MessageHandler *next = handler->next();
		delete handler;
		handler = next;
	}
	delete d;
}


void MessageHandlerChain::addHandler( MessageHandler *handler )
{
	handler->setNext( d->first );
	d->first = handler;
}

void MessageHandlerChain::processMessage( const Message &message )
{
	MessageEvent *event = new MessageEvent( message );
	d->first->handleMessage( event );
}

int MessageHandlerChain::capabilities()
{
	return d->first->capabilities();
}

}

#include "kopetemessagehandlerchain.moc"

// vim: set noet ts=4 sts=4 sw=4:

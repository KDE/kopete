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

namespace Kopete
{

class MessageHandlerChain::Private
{
public:
	MessageHandler *first;
	MessageHandler *last;
};

MessageHandlerChain::MessageHandlerChain( QObject *parent, const char *name )
 : QObject( parent, name ), d( new Private )
{
}

MessageHandlerChain::~MessageHandlerChain()
{
	delete d;
}


void MessageHandlerChain::addHandler( MessageHandler *handler )
{
	handler->setNext( d->last->next() );
	d->last->setNext( handler );
	d->last = handler;
}

void MessageHandlerChain::processMessage( const Message &message )
{
	MessageEvent *event = new MessageEvent( message );
	d->first->handleMessage( event );
}

Protocol::RichTextCapabilities MessageHandlerChain::capabilities()
{
	return d->first->capabilities();
}

}

#include "kopetemessagehandlerchain.moc"

// vim: set noet ts=4 sts=4 sw=4:

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

#include <qmap.h>
#include <qvaluelist.h>

namespace Kopete
{

class MessageHandlerChain::Private
{
public:
	Private() : first(0) {}
	MessageHandler *first;
};

class MessageHandlerChainTerminator : public MessageHandler
{
public:
	void handleMessage( MessageEvent * )
	{
		kdError( 14010 ) << k_funcinfo << "message got to end of chain!" << endl;
	}
	int capabilities()
	{
		kdError( 14010 ) << k_funcinfo << "request got to end of chain!" << endl;
		return 0;
	}
};

MessageHandlerChain::Ptr MessageHandlerChain::create( MessageManager *manager, Message::MessageDirection direction )
{
	// create the handler chain
	MessageHandlerChain *chain = new MessageHandlerChain;
	
	// grab the list of handler factories
	typedef MessageHandlerFactory::FactoryList FactoryList;
	FactoryList factories = MessageHandlerFactory::messageHandlerFactories();
	
	// create a sorted list of handlers
	typedef QValueList<MessageHandler*> HandlerList;
	typedef QMap<int,HandlerList> HandlerMap;
	HandlerMap handlers;
	uint count = 0;
	for( FactoryList::Iterator it = factories.begin(); it != factories.end(); ++it )
	{
		int position = (*it)->filterPosition( manager, direction );
		MessageHandler *handler = (*it)->create( manager, direction );
		if ( handler )
		{
			++count;
			handlers[ position ].append( handler );
		}
	}
	
	kdDebug(14010) << k_funcinfo << "got " << count << " handlers for chain" << endl;
	
	// add the handlers to the chain
	MessageHandler *curr = 0;
	for( HandlerMap::Iterator it = handlers.begin(); it != handlers.end(); ++it )
	{
		for ( HandlerList::Iterator handlerIt = (*it).begin(); handlerIt != (*it).end(); ++handlerIt )
		{
			if ( curr )
				curr->setNext( *handlerIt );
			else
				chain->d->first = *handlerIt;
			curr = *handlerIt;
		}
	}
	
	// add a terminator to avoid crashes if the message somehow manages to get to the
	// end of the chain. maybe we should use a MessageHandlerFactory for this too?
	MessageHandler *terminator = new MessageHandlerChainTerminator;
	if ( curr )
		curr->setNext( terminator );
	else // empty chain: might happen for dir == Internal
		chain->d->first = terminator;
	
	return chain;
}

MessageHandlerChain::MessageHandlerChain()
 : QObject( 0 ), d( new Private )
{
}

MessageHandlerChain::~MessageHandlerChain()
{
	kdDebug(14010) << k_funcinfo << endl;
	MessageHandler *handler = d->first;
	while( handler )
	{
		MessageHandler *next = handler->next();
		delete handler;
		handler = next;
	}
	delete d;
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

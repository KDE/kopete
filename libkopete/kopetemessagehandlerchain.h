/*
    kopetefilterchain.h - Kopete Message Filter Chain

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

#ifndef KOPETEFILTERCHAIN_H
#define KOPETEFILTERCHAIN_H

#include <qobject.h>
#include <kdemacros.h>

#include "kopetemessage.h"

namespace Kopete
{

class MessageHandler;
class MessageHandlerChainRef;

/**
 * @brief A chain of message handlers; the processing layer between protocol and chat view
 * @author Richard Smith       <kde@metafoo.co.uk>
 *
 * This class represents a chain of connected message handlers.
 *
 * This class is the client of the chain of responsibility formed by the
 * MessageHandlers, and acts as a facade for that chain, presenting a
 * more convenient interface.
 */
class MessageHandlerChain : public QObject
{
	Q_OBJECT
public:
	typedef MessageHandlerChainRef Ref;
	
	/**
	 * Create a new MessageHandlerChain object with the appropriate handlers for
	 * processing messages entering @p manager in direction @p direction.
	 */
	static Ref create( MessageManager *manager, Message::MessageDirection direction );

	void processMessage( const Message &message );
	int capabilities();
private:
	MessageHandlerChain();
	~MessageHandlerChain();
	
	friend class MessageHandlerChainRef;
	void incRef();
	void decRef();
	
	class Private;
	Private *d;
};

/**
 * A smart pointer to MessageHandlerChains implementing reference counting.
 */
class MessageHandlerChainRef
{
	MessageHandlerChain *chain;
public:
	MessageHandlerChainRef( MessageHandlerChain *chain = 0 ) : chain(chain) { incRef(); }
	MessageHandlerChainRef( const MessageHandlerChainRef &o ) : chain(o.chain) { incRef(); }
	~MessageHandlerChainRef() { decRef(); }
	MessageHandlerChainRef &operator=(const MessageHandlerChainRef &o)
	{
		o.incRef();
		decRef();
		chain = o.chain;
		return *this;
	}
	MessageHandlerChain *get() const { return chain; }
	MessageHandlerChain *operator->() const { return chain; }
	/**
	 * Implicit conversion to QObject * for connect() calls
	 */
	operator QObject *() { return chain; }
	
private:
	void incRef() const
	{
		if(chain)
			chain->incRef();
	}
	void decRef() const
	{
		if(chain)
			chain->decRef();
	}
};

}

#endif

// vim: set noet ts=4 sts=4 sw=4:

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

namespace Kopete
{

class Message;
class MessageHandler;

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
	/**
	 * Create a new Kopete::MessageHandlerChain object with no
	 * handlers in the chain.
	 */
	MessageHandlerChain( QObject *parent, const char *name );
	~MessageHandlerChain();

	//FIXME: remove - this is for testing.
	void addHandler( MessageHandler *handler );

	void processMessage( const Message &message );
	int capabilities();

private:
	class Private;
	Private *d;
};

}

#endif

// vim: set noet ts=4 sts=4 sw=4:

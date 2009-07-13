/*
    kopetesimplemessagehandler.h - Kopete Message Filtering - simple interface

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

#ifndef KOPETESIMPLEMESSAGEHANDLER_H
#define KOPETESIMPLEMESSAGEHANDLER_H

#include "kopete_export.h"
#include "kopetemessagehandler.h"

namespace Kopete
{

/**
 * @brief A MessageHandlerFactory that creates synchronous MessageHandlers that just call a slot
 *
 * A concrete MessageHandlerFactory. This class is intended to make writing MessageHandlers simpler;
 * all that is required is to implement a message processing function and place an instance of this
 * class in your Plugin-derived class.
 *
 * Whenever a message passes through a handler created by this factory, the slot passed to the
 * constructor will be called. The slot should take a single argument of type (non-@p const)
 * <tt>Message &</tt>.
 */
class KOPETE_EXPORT SimpleMessageHandlerFactory : public MessageHandlerFactory
{
public:
	/**
	 * @param direction The direction this factory should create message handlers for
	 * @param position Where in the chain the handler should be installed
	 * @param target The object to call back to when handling a message
	 * @param slot The slot on @p target to call when handling a message
	 * @see Kopete::MessageHandlerFactory::filterPosition
	 */
	SimpleMessageHandlerFactory( Message::MessageDirection direction, int position,
	                             QObject *target, const char *slot );
	~SimpleMessageHandlerFactory();
	
	/**
	 * Creates and returns a SimpleMessageHandler object.
	 */
	MessageHandler *create( ChatSession *manager, Message::MessageDirection direction );
	/**
	 * Returns the filter position passed to the constructor if @p direction matches the
	 * direction passed to the constructor, otherwise returns @c StageDoNotCreate.
	 */
	int filterPosition( ChatSession *manager, Message::MessageDirection direction );
	
private:
	class Private;
	Private * const d;
};

/**
 * @internal This class is used to implement SimpleMessageHandlerFactory.
 */
class SimpleMessageHandler : public MessageHandler
{
	Q_OBJECT
public:
	SimpleMessageHandler();
	~SimpleMessageHandler();
	
	void handleMessage( MessageEvent *event );
	
signals:
	void handle( Kopete::Message &message );
	
private:
	class Private;
	Private * const d;
};

}

#endif

// vim: set noet ts=4 sts=4 sw=4:

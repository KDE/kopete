/*
    kopetemessagehandler.h - Kopete Message Filtering

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

#ifndef KOPETEMESSAGEHANDLER_H
#define KOPETEMESSAGEHANDLER_H

#include <qobject.h>
//#include <kdemacros.h>

//FIXME: Message::MessageDirection could be moved into namespace Kopete
// to avoid this being included everywhere
#include "kopetemessage.h"

#include <qvaluelist.h>

namespace Kopete
{

class MessageEvent;
class MessageManager;

/**
 * @author Richard Smith       <kde@metafoo.co.uk>
 *
 * An object which sits between the protocol and the chat window which
 * intercepts and processes messages on their way through.
 *
 * This class implements Handler role in the Chain of Responsibility pattern.
 * The Client role will be filled by the Kopete::MessageHandlerChain class.
 */
class MessageHandler : public QObject
{
	Q_OBJECT
public:
	MessageHandler();
	virtual ~MessageHandler() = 0;

	/**
	 * @return the next handler in the chain
	 */
	MessageHandler *next();
	// FIXME: remove?
	void setNext( MessageHandler *next );

	/**
	 * @brief Gets the rich-text capabilities of this message handling object
	 *
	 * The default implementation returns next()->capabilities().
	 */
	virtual int capabilities();

	/**
	 * @brief Performs any processing necessary on the message
	 *
	 * @param e The message event to process. Should not be null.
	 */
	virtual void handleMessage( MessageEvent *event );

private:
	class Private;
	Private *d;
};

/**
 * @author Richard Smith       <kde@metafoo.co.uk>
 *
 * A factory for creating MessageHandlers.
 */
class MessageHandlerFactory
{
public:
	/**
	 * Constructs a MessageHandlerFactory, and adds it to the list of factories considered when
	 * creating a MessageHandlerChain for a MessageManager.
	 * 
	 * @note Since the factory is added to the list of possible factories before the object is
	 * finished being constructed, it is not safe to call any function from a derived class's
	 * constructor which may cause a MessageHandlerChain to be created.
	 */
	MessageHandlerFactory();
	/**
	 * Destroys the MessageHandlerFactory and removes it from the list of factories.
	 */
	virtual ~MessageHandlerFactory();
	
	typedef QValueList<MessageHandlerFactory*> FactoryList;
	/**
	 * @return the list of registered message handler factories
	 */
	static FactoryList messageHandlerFactories();
	
	/**
	 * @brief Creates a message handler for a given manager in a given direction.
	 * @param manager The manager whose message handler chain the message handler is for
	 * @param direction The direction of the chain that is being created.
	 * @return the @ref MessageHandler object to put in the chain, or 0 if none is needed.
	 */
	virtual MessageHandler *create( MessageManager *manager, Message::MessageDirection direction ) = 0;
	
	/**
	 * Processing stages for handlers in inbound message handler chains
	 */
	enum InboundStage
	{
		InStageStart = 0,        ///< message was just received
		InStageToSent = 2000,    ///< convert from received format to sent format
		InStageToDesired = 5000, ///< convert to how the user wants the message
		InStageFormat = 7000,    ///< decorate the message without changing the content
		InStageEnd = 10000       ///< message ready for display
	};
	
	/**
	 * Processing stages for handlers in outbound message handler chains
	 */
	enum OutboundStage
	{
		OutStageStart = 0,        ///< user just hit Send
		OutStageParse = 2000,     ///< process commands
		OutStageToDesired = 4000, ///< convert to how the user wanted to send
		OutStageFormat = 6000,    ///< decorate the message without changing the content
		OutStageToSent = 8000,    ///< convert to the format to send in
		OutStageEnd = 10000       ///< message ready for sending
	};
	
	/**
	 * Processing stages for handlers in internal message handler chains
	 */
	enum InternalStage
	{
		IntStageStart = 0,  ///< some component just created the message
		IntStageEnd = 10000 ///< message ready for display
	};
	
	/**
	 * Offsets within a processing stage. Using these values allows finer
	 * control over where in a chain a message handler will be added. Add
	 * one of these values to values from the various Stage enumerations
	 * to form a filter position.
	 */
	enum Offset
	{
		OffsetBefore = -90,
		OffsetVeryEarly = -60,
		OffsetEarly = -30,
		OffsetNormal = 0,
		OffsetLate = 30,
		OffsetVeryLate = 60,
		OffsetAfter = 90
	};
	
	/**
	 * @brief Returns the position in the message handler chain to put this factory's handlers
	 * @param manager The manager whose message handler chain the message handler is for
	 * @param direction The direction of the chain that is being created.
	 * @return a member of the InboundStage, OutboundStage or InternalStage enumeration, as
	 *         appropriate, optionally combined with a member of the Offset enumeration.
	 */
	virtual int filterPosition( MessageManager *manager, Message::MessageDirection direction ) = 0;
	
private:
	// noncopyable
	MessageHandlerFactory(const MessageHandlerFactory &);
	void operator=(const MessageHandlerFactory &);
	
	class Private;
	Private *d;
};

}

#endif

// vim: set noet ts=4 sts=4 sw=4:

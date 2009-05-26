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

#ifndef KOPETEMESSAGEHANDLERCHAIN_H
#define KOPETEMESSAGEHANDLERCHAIN_H

#include <QtCore/QObject>

#include <kdemacros.h>
#include <ksharedptr.h>
#include "kopetemessage.h"
#include "kopetetask.h"

namespace Kopete
{

class MessageEvent;
class MessageHandler;
class ProcessMessageTask;

/**
 * @brief A chain of message handlers; the processing layer between protocol and chat view
 *
 * This class represents a chain of connected message handlers.
 *
 * This class is the client of the chain of responsibility formed by the
 * MessageHandlers, and acts as a facade for that chain, presenting a
 * more convenient interface.
 * 
 * @author Richard Smith       <kde@metafoo.co.uk>
 */
class MessageHandlerChain : public QObject, private KShared
{
	Q_OBJECT
public:
	friend class KSharedPtr<MessageHandlerChain>;
	typedef KSharedPtr<MessageHandlerChain> Ptr;
	
	/**
	 * Create a new MessageHandlerChain object with the appropriate handlers for
	 * processing messages entering @p manager in direction @p direction.
	 */
	static Ptr create( ChatSession *manager, Message::MessageDirection direction );

	ProcessMessageTask *processMessage( const Message &message );
	int capabilities();
	
private:
	MessageHandlerChain();
	~MessageHandlerChain();
	
	friend class ProcessMessageTask;
	class Private;
	Private * const d;
};

/**
 * @brief A task for processing a message
 * @author Richard Smith       <kde@metafoo.co.uk>
 */
class ProcessMessageTask : public Task
{
	Q_OBJECT
public:
	MessageEvent *event();
	
public slots:
	void start();
	void slotDone();
	void kill( bool );
	
protected:
	// Avoid compiler warning about QObject::event
	using Task::event;
private:
	ProcessMessageTask(MessageHandlerChain::Ptr, MessageEvent *event);
	~ProcessMessageTask();
	
	friend class MessageHandlerChain;
	class Private;
	Private * const d;
};

}

#endif // KOPETEMESSAGEHANDLERCHAIN_H

// vim: set noet ts=4 sts=4 sw=4:

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

namespace Kopete
{

class MessageEvent;

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
	MessageHandler( MessageHandler *next, QObject *parent = 0, const char *name = 0 );
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

}

#endif

// vim: set noet ts=4 sts=4 sw=4:

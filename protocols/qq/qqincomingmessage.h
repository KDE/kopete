/*
    qqincomingmessage.h - Kopete QQ Protocol

    Copyright (c) 2003      by Will Stephenson		 <will@stevello.free-online.co.uk>
    Kopete    (c) 2002-2003 by the Kopete developers <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This library is free software; you can redistribute it and/or         *
    * modify it under the terms of the GNU General Public                   *
    * License as published by the Free Software Foundation; either          *
    * version 2 of the License, or (at your option) any later version.      *
    *                                                                       *
    *************************************************************************
*/

#ifndef TESTBEDINCOMINGMESSAGE_H
#define TESTBEDINCOMINGMESSAGE_H

#include <qobject.h>
#include "qqfakeserver.h"

/**
 * A simulated incoming message, that hasn't yet arrived at the
 * Kopete side 'client' of the simulated IM system.
 * @author Will Stephenson
 */
class QQIncomingMessage : public QObject
{
Q_OBJECT
public:
	/**
	 * Create a new incoming message
	 * @param server The simulated Kopete side 'client' of the IM system where the message will arrive when 'delivered'
	 * @param message The simulated message
	 */
	QQIncomingMessage( QQFakeServer* const server , QString message );
	virtual ~QQIncomingMessage();
	/**
	 * Has this message already been delivered?
	 */
	bool delivered() { return m_delivered; }
public slots:
	/**
	 * 'Deliver' the message to Kopete by calling QQFakeServer::incomingMessage().
	 * This marks the message as delivered so it can be purged from the incoming list.
	 */
	void deliver();
protected:
	QString m_message;
	QQFakeServer* m_server;
	bool m_delivered;
};

#endif

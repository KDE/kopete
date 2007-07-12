/*
    ligfakeserver.h - Kopete Lig Protocol

    Copyright (c) 2007      by Cláudio da Silveira Pinheiro	<taupter@gmail.com>
    Kopete    (c) 2002-2007 by the Kopete developers <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This library is free software; you can redistribute it and/or         *
    * modify it under the terms of the GNU General Public                   *
    * License as published by the Free Software Foundation; either          *
    * version 2 of the License, or (at your option) any later version.      *
    *                                                                       *
    *************************************************************************
*/

#ifndef LIGSERVER_H
#define LIGSERVER_H

#include "qobject.h"
#include <qptrlist.h>

class LigIncomingMessage;

/**
 * This is a interface to a dummy IM server
 * @author Will Stephenson
 */
class LigServer : public QObject
{
	Q_OBJECT
public:
	LigServer();
    ~LigServer();
	/**
	 * Called to simulate sending a message to a remote contact
	 */
	void sendMessage( QString contactId, QString message );
	
public slots:
	/**
	 * A message came in off the simulated wire.
	 * In reality, a message on the incoming message list
	 * connects to this slot when it's time to 'arrive'  
	 */
	void incomingMessage( QString message );
	
signals:
	/**
	 * Tells the account that a message arrived
	 */
	void messageReceived( const QString &message );
	
protected:
	/**
	 * Utility method, just clears delivered messages from the 
	 * incoming message list
	 */
	void purgeMessages();
	/**
	 * List of incoming messages
	 */
	QPtrList<LigIncomingMessage> m_incomingMessages;
};

#endif

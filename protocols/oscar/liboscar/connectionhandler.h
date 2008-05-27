/*
    Kopete Oscar Protocol
    Oscar Multiple Connection Handling

    Copyright (c) 2005 Matt Rogers <mattr@kde.org>

    Kopete (c) 2002-2005 by the Kopete developers <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This library is free software; you can redistribute it and/or         *
    * modify it under the terms of the GNU Lesser General Public            *
    * License as published by the Free Software Foundation; either          *
    * version 2 of the License, or (at your option) any later version.      *
    *                                                                       *
    *************************************************************************
*/

#ifndef CONNECTIONHANDLER_H
#define CONNECTIONHANDLER_H

#include "oscartypes.h"
#include <qstring.h>
#include <qpair.h>


class Connection;

typedef QPair<Oscar::WORD, QString> ConnectionRoomInfo;

/**
@author Kopete Developers
*/
class ConnectionHandler
{
public:
	ConnectionHandler();
	~ConnectionHandler();

	/**
	 * Add a connection to the handler so that it can be
	 * tracked and queried for later.
	 * @param c The connection to add to the handler
	 */
	void append( Connection* c );

	/**
	 * Remove a connection from the handler
	 * @param c The connection object to remove
	 */
	void remove( Connection* c );

	/**
	 * Remove a connection from the handler
	 * @param family The SNAC family for the connection to remove
	 */
	void remove( int family );

	/**
	 * Clear all the connections.
	 */
	void clear();

	/**
	 * Get the connection for a particular SNAC family. If there is
	 * more than one connection for a particular family or there is no
	 * connection, then zero is returned.
	 * @return A valid connection object for the family or 0
	 */
	Connection* connectionForFamily( int family ) const;

	/**
	 * Get the default connection. Returns zero when we're handling more than
	 * one connection.
	 * @return The only connection object we're tracking or zero if we have
	 * more than one.
	 */
	Connection* defaultConnection() const;

	/**
	 * Get all connections.
	 * @return all connection objects
	 */
	QList<Connection*> connections() const;

	/**
	 * Add chat room information to a connection so that we can track
	 * connections by chat room
	 * @param c The connection to add information to
	 * @param exchange the exchange the chat room is in
	 * @param room the name of the chat room
	 */
	void addChatInfoForConnection( Connection* c, Oscar::WORD exchange, const QString& room );

	/**
	 * Get the connection for a particular room name and exchange number.
	 * @param exchange the chat room exchange the room is on
	 * @param room the name of the chat room to find a connection for
	 * @return a Connection for the chat room or 0L if no connection for that room
	 */
	Connection* connectionForChatRoom( Oscar::WORD exchange, const QString& room );

    /**
     * Get the room name for the chat room based the connection
     * @return The name of the chat room that this connection is connected to
     * If the connection passed in by @p c is not a chat room connection then
     * QString() is returned.
     */
    QString chatRoomForConnection( Connection* c );

    /**
     * Get the exchange number for the chat room based on the connection
     * @return The exchange of the chat room that this connection is connected
     * to. If the connection passed in by @p c is not a chat room connection
     * then 0xFFFF is returned
     */
    Oscar::WORD exchangeForConnection( Connection* c );

private:
	class Private;
	Private* d;
};

#endif

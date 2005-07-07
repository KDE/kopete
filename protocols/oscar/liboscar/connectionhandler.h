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

#include <qvaluelist.h>
class Connection;

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
	void addConnection( Connection* c );

	/**
	 * Remove a connection from the handler
	 * @param c The connection object to remove
	 */
	void removeConnection( Connection* c );

	/**
	 * Remove a connection from the handler
	 * @param family The SNAC family for the connection to remove
	 */
	void removeConnection( int family );

	/**
	 * Get the connection for a particular SNAC family. If there is
	 * more than one connection for a particular family or there is no
	 * connection, then zero is returned.
	 * @return A valid connection object for the family or 0
	 */
	Connection* connectionForFamily( int family ) const;

private:
	class Private;
	Private* d;
};

#endif

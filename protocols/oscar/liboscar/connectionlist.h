// connectionlist.h

// Copyright (C)  2005  Matt Rogers <mattr@kde.org>

// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2.1 of the License, or (at your option) any later version.

// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Lesser General Public License for more details.

// You should have received a copy of the GNU Lesser General Public
// License along with this library; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
// 02111-1307  USA

#ifndef CONNECTIONLIST_H
#define CONNECTIONLIST_H

#include <qmap.h>
#include <qvaluelist.h>

class Connection;
class QString;

class ConnectionList
{
public:
	ConnectionList();
	~ConnectionList();

	enum ConnectionTypes
	{
		DirectConnection = 0x0001, ///< A generic direct connection
		FileTransfer     = 0x0002, ///< A file transfer connection
		DirectIM         = 0x0004, ///< A direct im connection
		BuddyIcon        = 0x0008, ///< A connection to a buddy icon server
		ChatRoom         = 0x0010  ///< A connection for AOL chatrooms
	};

	/**
	 * \brief Get the BOS connection.
	 *
	 * Get the BOS connection that this class holds.
	 * There can be only one BOS connection open at any time.
	 * \return the connection for the bos server
	 * \return 0 if we're not connected to a bos server yet
	 */
	Connection* bosConnection() const;

	/**
	 * \brief Get the authorizer connection.
	 *
	 * Get the authorizer connection that this class holds.
	 * There is usually only one authorizer connection open at any time.
	 * \return the connection for the authorizer server
	 * \return 0 if we're not connected to an authorizer server anymore
	 */
	Connection* authorizerConnection() const;

	/**
	 * \brief Get the buddy icon connection
	 *
	 * Get the connection to the buddy icon server that this class holds.
	 * There is usually only one icon connection.
	 * \return the connection for the buddy icon server
	 * \return 0 if there's no icon connection available.
	 */
	Connection* iconConnection() const;

	/**
	 * \brief Register a BOS connection with the connection list
	 *
	 * Adds the main BOS connection to the list of connections
	 * \param bosConnection the BOS connection to register
	 */
	void registerBOSConnection( Connection* bosConnection );

	/**
	 * \brief Register an authorizer connection with the connection list
	 *
	 * Adds the authorizer connection to the list of connections
	 * \param authConnection the authorizer connection to register
	 */
	void registerAuthConnection( Connection* authConnection );

	/**
	 * \brief Register an icon connection with the connection list
	 *
	 * Adds the icon connection to the list of connections
	 * \param iconConnection the connection to the icon server
	 */
	void registerIconConnection( Connection* iconConnection );

	/**
	 * \brief Remove a connection from the list
	 *
	 * Removes the specified connection from the list
	 */
	void removeConnection( Connection* connection );

	QValueList<Connection*> getConnections();
	void removeAllConnections();

private:

	//TODO: d-pointerize?
	Connection* m_bosConnection;
	Connection* m_authConnection;
	Connection* m_iconConnection;

};

#endif

// connectionlist.cpp

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


#include "connectionlist.h"

#include <qstring.h>

#include "connection.h"

ConnectionList::ConnectionList()
	: m_bosConnection( 0 ),
		m_authConnection( 0 ),
		m_iconConnection( 0 )
{

}

ConnectionList::~ConnectionList()
{
	delete m_bosConnection;
	delete m_authConnection;
	delete m_iconConnection;
}

QValueList<Connection*> ConnectionList::getConnections()
{
	QValueList<Connection*> list;
	if ( m_bosConnection )
		list.append( m_bosConnection );

	if ( m_authConnection )
		list.append( m_authConnection );

	if ( m_iconConnection )
		list.append( m_iconConnection );

	return list;
}

void ConnectionList::removeAllConnections()
{
	m_bosConnection = 0;
	m_authConnection = 0;
	m_iconConnection = 0;
}

Connection* ConnectionList::bosConnection() const
{
	return m_bosConnection;
}

Connection* ConnectionList::authorizerConnection() const
{
	return m_authConnection;
}

Connection* ConnectionList::iconConnection() const
{
	return m_iconConnection;
}

void ConnectionList::registerBOSConnection( Connection* bosConnection )
{
	m_bosConnection = bosConnection;
}

void ConnectionList::registerAuthConnection( Connection* authConnection )
{
	m_authConnection = authConnection;
}

void ConnectionList::registerIconConnection( Connection* iconConnection )
{
	m_iconConnection = iconConnection;
}

void ConnectionList::removeConnection( Connection* connection )
{
	if ( connection == m_bosConnection )
	{
		m_bosConnection = 0;
		return;
	}

	if ( connection == m_authConnection )
	{
		m_authConnection = 0;
		return;
	}

	if ( connection == m_iconConnection )
	{
		m_iconConnection = 0;
		return;
	}

}

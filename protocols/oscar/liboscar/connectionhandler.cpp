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

#include "connectionhandler.h"
#include <q3valuelist.h>
#include <kdebug.h>
#include "connection.h"
#include "oscartypes.h"

class ConnectionHandler::Private
{
public:
	Q3ValueList<Connection*> connections;
};

ConnectionHandler::ConnectionHandler()
{
	d = new Private;
}


ConnectionHandler::~ConnectionHandler()
{
	delete d;
}

void ConnectionHandler::append( Connection* c )
{
	d->connections.append( c );
}

void ConnectionHandler::remove( Connection* c )
{
	kdDebug(OSCAR_RAW_DEBUG) << k_funcinfo << "Removing connection "
		<< c << endl;
	d->connections.remove( c );
	c->deleteLater();
    c = 0;
}

void ConnectionHandler::remove( int family )
{
	kdDebug(OSCAR_RAW_DEBUG) << k_funcinfo << "Removing all connections " <<
		"supporting family " << family << endl;
	Q3ValueList<Connection*>::iterator it = d->connections.begin();
	Q3ValueList<Connection*>::iterator itEnd = d->connections.end();
	for ( ; it != itEnd; ++it )
	{
		if ( ( *it )->isSupported( family ) )
		{
			Connection* c = ( *it );
			it = d->connections.remove( it );
			c->deleteLater();
			c = 0;
		}
	}
}

void ConnectionHandler::clear()
{
	kdDebug(OSCAR_RAW_DEBUG) << k_funcinfo << "Clearing all connections"
		 << endl;
	while ( !d->connections.isEmpty() )
	{
		Connection *c = d->connections.front();
		d->connections.pop_front();
		c->deleteLater();
		c = 0;
	}
}

Connection* ConnectionHandler::connectionForFamily( int family ) const
{
	Q3ValueList<Connection*>::iterator it = d->connections.begin();
	Q3ValueList<Connection*>::iterator itEnd = d->connections.end();
	int connectionCount = 0;
	Connection* lastConnection = 0;
	for ( ; it != itEnd; ++it )
	{
		if ( ( *it )->isSupported( family ) )
		{
			connectionCount++;
			lastConnection = ( *it );
		}
	}
	if ( connectionCount == 1 )
		return lastConnection;

	return 0;
}

Connection* ConnectionHandler::defaultConnection() const
{
	if ( d->connections.isEmpty() || d->connections.count() > 1 )
		return 0;

	return d->connections.first();
}


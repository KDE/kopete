// -*- Mode: C++; c-basic-offset: 2; indent-tabs-mode: nil; -*-

/* connection.cpp: Qt wrapper for DBusConnection
*
* Copyright (C) 2003  Zack Rusin <zack@kde.org>
*
* Licensed under the Academic Free License version 2.0
*
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation; either version 2 of the License, or
* (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program; if not, write to the Free Software
* Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*
*/

#include <qapplication.h>
#include <qtimer.h>
#include <iostream>

#define DBUS_API_SUBJECT_TO_CHANGE
#include "connection.h"

using namespace DBusQt;

#include "integrator.h"
using Internal::Integrator;

struct Connection::Private
{
	Private( Connection * qq );
	~Private(  );
	void setConnection( DBusConnection * c, bool tmp = FALSE );
	DBusConnection *connection;
	int connectionSlot;
	DBusError error;
	Integrator *integrator;
	int timeout;
	Connection *q;
};

Connection::Private::Private( Connection * qq ):connection( 0 ), connectionSlot( -1 ), integrator( 0 ), timeout( -1 ), q( qq )
{
	dbus_error_init( &error );
}

Connection::Private::~Private(  )
{
	delete integrator;
}

void Connection::Private::setConnection( DBusConnection * c, bool tmp )
{
	if ( !c )
	{
		return;
	}

	connection = c;
	integrator = new Integrator( c, q, tmp );
	connect( integrator, SIGNAL( readReady(  ) ), q, SLOT( dispatchRead(  ) ) );
}

Connection::Connection( QObject * parent ):QObject( parent )
{
	d = new Private( this );

	dbus_error_init( &d->error );
}

Connection::Connection( const QString & host, QObject * parent ):QObject( parent )
{
	d = new Private( this );

	if ( !host.isEmpty(  ) )
	{
		DBusConnection *cn = dbus_connection_open( host.ascii(  ), &d->error );

		if ( error(  ) )
		{
			qDebug( "dbus_connection_open failed." );
			return;
		}

		d->setConnection( cn, TRUE );
	}
}

Connection::Connection( DBusBusType type, QObject * parent, bool temporary ):QObject( parent )
{

	qDebug( "Connection::Connection" );
	d = new Private( this );
	DBusConnection *cn = dbus_bus_get( type, &d->error );

	if ( error(  ) )
	{
		qDebug( "dbus_bus_get failed." );
		return;
	}

	d->setConnection( cn, temporary );

	//QTimer *timer = new QTimer(this);

	//connect(timer, SIGNAL(timeout()), this, SLOT(dispatchRead()));

	//timer->start(500, FALSE);
}

Connection::~Connection(  )
{
	delete d->integrator;
	d->integrator = 0L;

	dbus_connection_unref( d->connection );
	delete d;
}

void Connection::init( const QString & host )
{
	d->setConnection( dbus_connection_open( host.ascii(  ), &d->error ) );
	dbus_connection_allocate_data_slot( &d->connectionSlot );
	dbus_connection_set_data( d->connection, d->connectionSlot, 0, 0 );
}

bool Connection::isConnected(  ) const
{
	return dbus_connection_get_is_connected( d->connection );
}

bool Connection::isAuthenticated(  ) const
{
	return dbus_connection_get_is_authenticated( d->connection );
}

void Connection::open( const QString & host )
{
	if ( host.isEmpty(  ) )
		return;

	init( host );
}

void Connection::close(  )
{
	dbus_connection_disconnect( d->connection );
}

void Connection::flush(  )
{
	dbus_connection_flush( d->connection );
}

bool Connection::event( QEvent * e )
{
	if ( e->type(  ) == DBUS_EVENT_WAKEUP )
	{
		qDebug( "Custom event received." );

		dispatchRead(  );

		return TRUE;
	}
	else
		return QObject::event( e );
}

void Connection::dispatchRead(  )
{
	qDebug( "API: dispatchRead" );

	//dbus_connection_flush(d->connection);

	/*DBusDispatchStatus status = dbus_connection_get_dispatch_status(d->connection);

	   if (status == DBUS_DISPATCH_DATA_REMAINS)
	   dbus_connection_dispatch( d->connection );
	 */

	/*DBusDispatchStatus status = dbus_connection_get_dispatch_status(d->connection);

	   switch(status)
	   {
	   case DBUS_DISPATCH_DATA_REMAINS:
	   qDebug("DBUS_DISPATCH_DATA_REMAINS");
	   break;
	   case DBUS_DISPATCH_COMPLETE:
	   qDebug("DBUS_DISPATCH_COMPLETE");
	   break;
	   case DBUS_DISPATCH_NEED_MEMORY:
	   qDebug("DBUS_DISPATCH_NEED_MEMORY");
	   break;
	   default:
	   qDebug("UNKNOWN");
	   break;
	   } */

	while ( dbus_connection_dispatch( d->connection ) == DBUS_DISPATCH_DATA_REMAINS ) ;

	// status;
	//while(1)
	/*
	   do
	   {
	   status = dbus_connection_dispatch(d->connection);//DBUS_DISPATCH_COMPLETE;
	   //

	   switch(status)
	   {
	   case DBUS_DISPATCH_DATA_REMAINS:
	   qDebug("DBUS_DISPATCH_DATA_REMAINS");
	   break;
	   case DBUS_DISPATCH_COMPLETE:
	   qDebug("DBUS_DISPATCH_COMPLETE");
	   break;
	   case DBUS_DISPATCH_NEED_MEMORY:
	   qDebug("DBUS_DISPATCH_NEED_MEMORY");
	   break;
	   default:
	   qDebug("UNKNOWN");
	   break;
	   }

	   if (status == DBUS_DISPATCH_DATA_REMAINS)
	   {
	   qDebug("dispatching ...");

	   dbus_connection_dispatch(d->connection);
	   }

	   } while (status != DBUS_DISPATCH_COMPLETE);
	 */
}

DBusConnection *Connection::connection(  )
{
	return d->connection;
}

Connection::Connection( DBusConnection * connection, QObject * parent ):QObject( parent )
{
	d = new Private( this );
	d->setConnection( connection );
}

void Connection::send( const Message & m )
{
	dbus_connection_send( d->connection, m.message(  ), 0 );
}

void Connection::sendWithReply( const Message & )
{
}

Message Connection::sendWithReplyAndBlock( const Message & m )
{
	DBusMessage *reply;

	reply = dbus_connection_send_with_reply_and_block( d->connection, m.message(  ), d->timeout, &d->error );

	if ( error(  ) )
		return m;
	else
		return Message( reply );
}

bool Connection::error(  )
{
	return dbus_error_is_set( &d->error );
}

QString Connection::getError(  )
{
	QString err;

	if ( dbus_error_is_set( &d->error ) )
	{
		err = d->error.name;
		dbus_error_free( &d->error );
	}

	return err;
}

void *Connection::virtual_hook( int, void * )
{
	return 0;
}

static DBusHandlerResult nm_message_handler( DBusConnection * connection, DBusMessage * message, void *user_data )
{
	const char *method;
	const char *path;
	const char *sender;
	const char *signature;

	//DBusMessage       *reply_message = NULL;
	//gboolean           handled = TRUE;
	Connection *c = static_cast < Connection * >( user_data );

	//g_return_val_if_fail (connection != NULL, DBUS_HANDLER_RESULT_NOT_YET_HANDLED);
	//g_return_val_if_fail (message != NULL, DBUS_HANDLER_RESULT_NOT_YET_HANDLED);

	qDebug( "nm_message_handler" );

	c->dbusMessage( message );

	method = dbus_message_get_member( message );
	path = dbus_message_get_path( message );
	sender = dbus_message_get_sender( message );
	signature = dbus_message_get_signature( message );

	qDebug( "nm_dbus_nm_message_handler() got method %s for path %s, sender %s", method, path, sender );

	/*
	   if (strcmp("testFunction", method) == 0)
	   return DBUS_HANDLER_RESULT_HANDLED;
	   else
	   return DBUS_HANDLER_RESULT_NOT_YET_HANDLED;
	 */
	//Connection c(connection, NULL);
	//Message in(message);
	//Message out(in);

	//out << "Success.";

	//c->send(out);

	/*
	   if (strcmp ("setKeyForNetwork", method) == 0)
	   set_user_key_for_network (connection, message, user_data);
	   else
	   handled = FALSE;

	   return (handled ? DBUS_HANDLER_RESULT_HANDLED : DBUS_HANDLER_RESULT_NOT_YET_HANDLED);
	 */
	return DBUS_HANDLER_RESULT_HANDLED;
}

static void nm_unregister_handler( DBusConnection * connection, void *user_data )
{
	/* do nothing */
	qDebug( "nm_unregister_handler" );
}

void Connection::dbusMessage( DBusMessage * message )
{
	qDebug( "Connection::dbusMessage" );

	Message *m = new Message( message );

	/*if (m->expectReply())
	   {
	   qDebug("Message expects reply. Generating.");
	   Message* reply = new Message(*m);
	   (*reply) << QString("OLLEH");
	   send(*reply);
	   flush();
	   } */

	emit messageArrived( *m );
}

bool Connection::registerObjectPath( const QString & path, const QString & service )
{
	DBusObjectPathVTable vtable = { &nm_unregister_handler, &nm_message_handler, NULL, NULL, NULL,
		NULL
	};

	//dbus_bus_acquire_service(d->connection, service, 0, &d->error);
	//dbus_bus_activate_service( d->connection, "org.freedesktop.DBus", 0, NULL, &d->error );

	//dbus_bus_set_base_service(d->connection, "com.Skype.API");

	if ( error(  ) )
	{
		return FALSE;
	}

	//qDebug("Connection base name: %s", dbus_bus_get_base_service(d->connection));

	/*dbus_bus_acquire_service(d->connection, "com.Skype.API", 0, &d->error);

	   if (dbus_error_is_set (&d->error))
	   {
	   qDebug("Could not acquire its service.  dbus_bus_acquirebool success = dbus_connection_register_fallback(d->connection, path.ascii(), &vtable, this);_service() says: '%s'", d->error.message);

	   return FALSE;
	   } */

	//dbus_bus_add_match(d->connection, "type='method_call',interface='com.Skype.API'", &d->error);

	//if (dbus_error_is_set (&d->error))
	//{
	//  qDebug("Could not add match. Error is: '%s'", d->error.message);

	//  return FALSE;
	//}

	//bool success = dbus_connection_register_object_path(d->connection, path.ascii(), &vtable, this);
	//success = dbus_connection_register_fallback(d->connection, "/org/freedesktop/DBus", &vtable, this);

	bool success = dbus_connection_register_object_path(d->connection, service.utf8(), &vtable, this);

	//success = dbus_connection_add_filter(d->connection, nm_message_handler, this, NULL);
	if (!success)
	{
	  qDebug("Could not register a handler for NetworkManager.  Not enough memory?");
	  return FALSE;
	}

	return TRUE;
}

#include "connection.moc"

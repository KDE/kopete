// -*- Mode: C++; c-basic-offset: 2; indent-tabs-mode: nil; -*-
/* integrator.h: integrates D-BUS into Qt event loop
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

#define DBUS_API_SUBJECT_TO_CHANGE

#include "integrator.h"
#include "connection.h"

#include <qapplication.h>
#include <qeventloop.h>
#include <qtimer.h>
#include <qsocketnotifier.h>
#include <qintdict.h>
#include <qptrlist.h>

namespace DBusQt
{
namespace Internal {

struct Watch{
	Watch(): readSocket( 0 ), writeSocket( 0 ) { }
	//~Watch();

  DBusWatch *watch;
  QSocketNotifier *readSocket;
  QSocketNotifier *writeSocket;
};

/*Watch::~Watch()
{
	if (readSocket)
		delete readSocket;

	if (writeSocket)
		delete writeSocket;
}*/

//////////////////////////////////////////////////////////////
dbus_bool_t dbusAddWatch(DBusWatch* watch, void* data)
{
	int fd = dbus_watch_get_fd(watch);

	qDebug ("API: fd is %d", fd);

	Integrator *con = static_cast<Integrator*>( data );
	con->addWatch( watch );

	return true;
}

void dbusRemoveWatch( DBusWatch *watch, void *data )
{
	Integrator *con = static_cast<Integrator*>( data );
	con->removeWatch( watch );
}

void dbusToggleWatch( DBusWatch *watch, void *data )
{
	//return;
	Integrator *itg = static_cast<Integrator*>( data );
	//Watch* qw = itg->m_watches.find(dbus_watch_get_fd(watch));

	qDebug("API: dbusToggleWatch");

	int fd = dbus_watch_get_fd(watch);

	qDebug ("API: fd is %d", fd);

	if (dbus_watch_get_enabled(watch))
	{
		//if (qw->readSocket) qw->readSocket->setEnabled(TRUE);
		//if (qw->writeSocket) qw->writeSocket->setEnabled(TRUE);
		itg->addWatch(watch);
		qDebug("enabling");
	}
	else
	{
		//if (qw->readSocket)  qw->readSocket->setEnabled(FALSE);
		//if (qw->writeSocket) qw->writeSocket->setEnabled(FALSE);
		itg->removeWatch(watch);
		qDebug("disabling");
	}

	/*
	bool en = dbus_watch_get_enabled(watch);
	int flags = dbus_watch_get_flags(watch);

	Integrator *itg = static_cast<Integrator*>( data );
	Watch* qw = itg->m_watches.find(dbus_watch_get_fd(watch));


	if ( flags && DBUS_WATCH_READABLE )
	{
		qw->readSocket->setEnabled(en);
	}

	if (flags && DBUS_WATCH_WRITABLE)
	{
		qw->writeSocket->setEnabled(en);
	}
	*/
	/*
	if (dbus_watch_get_enabled(watch))
	{
		//itg->addWatch(watch);

		if (qw)
		{
			qw->readSocket->setEnabled(TRUE);
			qw->writeSocket->setEnabled(TRUE);
		}
		else
			itg->addWatch( watch );
	}
	else
	{
		//itg->removeWatch(watch);

		if (qw)
		{
			qw->readSocket->setEnabled(FALSE);
			qw->writeSocket->setEnabled(FALSE);
		}
		else
			itg->removeWatch( watch );
	}
	*/
}

dbus_bool_t dbusAddTimeout( DBusTimeout *timeout, void *data )
{
  if ( !dbus_timeout_get_enabled(timeout) )
    return true;

  Integrator *itg = static_cast<Integrator*>( data );
  itg->addTimeout( timeout );
  return true;
}

void dbusRemoveTimeout( DBusTimeout *timeout, void *data )
{
  Integrator *itg = static_cast<Integrator*>( data );
  itg->removeTimeout( timeout );
}

void dbusToggleTimeout( DBusTimeout *timeout, void *data )
{
  Integrator *itg = static_cast<Integrator*>( data );

  if ( dbus_timeout_get_enabled( timeout ) )
    itg->addTimeout( timeout );
  else
    itg->removeTimeout( timeout );
}

void dbusWakeupMain(void* c)
{
	qDebug("wake up!");

	//Connection* conn = static_cast<Connection*>(c);

	//conn->flush();

	//if (conn)
	//	QApplication::postEvent(conn, new QCustomEvent(DBUS_EVENT_WAKEUP));

	qApp->eventLoop()->wakeUp();
}

void dbusDispatchStatusChanged(DBusConnection *connection, DBusDispatchStatus new_status, void *data)
{
	qDebug("dbusDispatchStatusChanged invoked");

	switch(new_status)
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
}

static dbus_bool_t dbusUidHandler(DBusConnection* c, unsigned long uid, void* data)
{
	qDebug("API: dbusUidHandler uid is %lu", uid);

	return TRUE;
}

static void dbusNewConnection( DBusServer     *server,
                        DBusConnection *new_connection,
                        void           *data )
{
  qDebug("dbusNewConnection() invoked");

  dbus_connection_set_unix_user_function(new_connection, &dbusUidHandler, data, 0);

  Integrator *itg = static_cast<Integrator*>( data );
  itg->handleConnection( new_connection );
}

/////////////////////////////////////////////////////////////

Timeout::Timeout( QObject *parent, DBusTimeout *t )
  : QObject( parent ),  m_timeout( t )
{
  m_timer = new QTimer( this );
  connect( m_timer,  SIGNAL(timeout()),
           SLOT(slotTimeout()) );
}

void Timeout::slotTimeout()
{
  emit timeout( m_timeout );
}

void Timeout::start()
{
  m_timer->start( dbus_timeout_get_interval( m_timeout ) );
}

Integrator::Integrator(DBusConnection *conn, QObject *parent, bool tmp)
	:QObject(parent),
	m_connection(conn)
{
	qDebug("Integrator::Integrator(DBusConnection*) invoked");

	m_timeouts.setAutoDelete( true );

	dbus_connection_set_watch_functions( m_connection,
										dbusAddWatch,
										dbusRemoveWatch,
										/*dbusToggleWatch*/0,
										this, 0 );
	dbus_connection_set_timeout_functions( m_connection,
											dbusAddTimeout,
											dbusRemoveTimeout,
											dbusToggleTimeout,
											this, 0 );

	//if (!tmp)
	dbus_connection_set_wakeup_main_function(m_connection, dbusWakeupMain, parent, 0);

	//dbus_connection_set_dispatch_status_function(m_connection, dbusDispatchStatusChanged, this, 0);
}

Integrator::Integrator( DBusServer *server, QObject *parent )
  : QObject( parent ), m_server( server )
{
  qDebug("Integrator::Integrator(DBusServer*) invoked");

  //m_connection = reinterpret_cast<DBusConnection*>( m_server );
  m_timeouts.setAutoDelete( true );

  dbus_server_set_watch_functions( m_server,
                                   dbusAddWatch,
                                   dbusRemoveWatch,
								   /*dbusToggleWatch*/0,
                                   this, 0 );
  /*dbus_server_set_timeout_functions( m_server,
                                     dbusAddTimeout,
                                     dbusRemoveTimeout,
                                     dbusToggleTimeout,
  this, 0 );*/
  dbus_server_set_new_connection_function( m_server,
                                           dbusNewConnection,
                                           this,  0 );
}

Integrator::~Integrator()
{
	for (int nC = 0; nC < m_watches.count(); nC++)
		delete m_watches[nC];
}

void Integrator::slotRead(int fd)
{
	qDebug("slotRead");
	QIntDictIterator<Watch> it(m_watches);

	for (; it.current(); ++it)
		dbus_watch_handle(it.current()->watch, DBUS_WATCH_READABLE);

	emit readReady();

	/*if (Watch* ww = m_watches.find(fd))
		dbus_watch_handle(ww->watch, DBUS_WATCH_READABLE);

	if (m_connection)
	{
		//_dbus_connection_acquire_dispatch(m_connection);
		//dbus_connection_dispatch(m_connection);
		//_dbus_connection_release_dispatch(m_connection);

		emit readReady();
}*/
}

void Integrator::slotWrite(int fd)
{
	//qDebug("slotWrite");

	QIntDictIterator<Watch> it(m_watches);

	for (; it.current(); ++it)
		dbus_watch_handle(it.current()->watch, DBUS_WATCH_WRITABLE);

	/*Watch* ww = m_watches.find(fd);

	dbus_watch_handle(ww->watch, DBUS_WATCH_WRITABLE);*/

	//if (m_connection)
	//{
		//dbus_connection_flush(m_connection);

		//emit readReady();
	//}
}

void Integrator::slotTimeout( DBusTimeout *timeout )
{
	qDebug("Integrator::slotTimeout");
  dbus_timeout_handle( timeout );
}

void Integrator::addWatch( DBusWatch *watch )
{
	qDebug("Integrator::addWatch invoked");

	if ( !dbus_watch_get_enabled( watch ) )
	{
    	return;
	}

	int flags = dbus_watch_get_flags(watch);
  	int fd = dbus_watch_get_fd(watch);

	Watch* ww = m_watches.find(fd);

	if (ww && (dbus_watch_get_flags(ww->watch) == flags))
	{
		qDebug("API: not adding duplicate watch");
		return;
	}

  Watch *qtwatch = new Watch;
  qtwatch->watch = watch;

  //qDebug("flags == %d fd == %d", flags, fd);

  if ( flags & DBUS_WATCH_READABLE )
  {
    qtwatch->readSocket = new QSocketNotifier( fd, QSocketNotifier::Read, this);
    QObject::connect( qtwatch->readSocket, SIGNAL(activated(int)), this, SLOT(slotRead(int)) );
  }

  if (flags & DBUS_WATCH_WRITABLE)
  {
    qtwatch->writeSocket = new QSocketNotifier( fd, QSocketNotifier::Write, this);
    QObject::connect( qtwatch->writeSocket, SIGNAL(activated(int)), this, SLOT(slotWrite(int)) );
  }

  m_watches.insert( fd, qtwatch );
}

void Integrator::removeWatch( DBusWatch *watch )
{
  int key = dbus_watch_get_fd( watch );

  Watch *qtwatch = m_watches.take( key );

  if ( qtwatch )
  {
    delete qtwatch->readSocket;
	qtwatch->readSocket = 0;
    delete qtwatch->writeSocket;
	qtwatch->writeSocket = 0;
    delete qtwatch;
  }
}

void Integrator::addTimeout( DBusTimeout *timeout )
{
	qDebug("add timeout");
  Timeout *mt = new Timeout( this, timeout );
  m_timeouts.insert( timeout, mt );
  connect( mt, SIGNAL(timeout(DBusTimeout*)),
           SLOT(slotTimeout(DBusTimeout*)) );
  mt->start();
}

void Integrator::removeTimeout( DBusTimeout *timeout )
{
  m_timeouts.remove( timeout );
}

void Integrator::handleConnection( DBusConnection *c )
{
	//dbus_connection_ref(c);
	//int fd;
	//dbus_connection_get_unix_fd(c, &fd);

	//qDebug("Integrator::handleConnection() invoked, fd is %d", fd);
	//qDebug("Connection base name: %s", dbus_bus_get_base_service(c));

	dbus_connection_ref(c);

	//dbus_bus_set_base_service(c, "/com/Skype/API");

	Connection *con = new Connection( c, this );

	dbus_connection_set_wakeup_main_function(c, &dbusWakeupMain, con, 0);
	//dbus_connection_set_dispatch_status_function(c, dbusDispatchStatusChanged, this, 0);


	emit newConnection( con );
}

}//end namespace Internal
}//end namespace DBusQt

#include "integrator.moc"

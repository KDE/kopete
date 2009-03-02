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
#include "integrator.h"
#include "connection.h"

#include <qtimer.h>
#include <qsocketnotifier.h>
#include <qintdict.h>
#include <qptrlist.h>

namespace DBusQt
{
namespace Internal {

struct Watch {
  Watch(): readSocket( 0 ), writeSocket( 0 ) { }

  DBusWatch *watch;
  QSocketNotifier *readSocket;
  QSocketNotifier *writeSocket;
};

//////////////////////////////////////////////////////////////
dbus_bool_t dbusAddWatch( DBusWatch *watch, void *data )
{
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
  Integrator *itg = static_cast<Integrator*>( data );
  if ( dbus_watch_get_enabled( watch ) )
    itg->addWatch( watch );
  else
    itg->removeWatch( watch );
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

void dbusWakeupMain( void* )
{
}

void dbusNewConnection( DBusServer     *server,
                        DBusConnection *new_connection,
                        void           *data )
{
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

Integrator::Integrator( DBusConnection *conn, QObject *parent )
  : QObject( parent ), m_connection( conn )
{
  m_timeouts.setAutoDelete( true );

  dbus_connection_set_watch_functions( m_connection,
                                       dbusAddWatch,
                                       dbusRemoveWatch,
                                       dbusToggleWatch,
                                       this, 0 );
  dbus_connection_set_timeout_functions( m_connection,
                                         dbusAddTimeout,
                                         dbusRemoveTimeout,
                                         dbusToggleTimeout,
                                         this, 0 );
  dbus_connection_set_wakeup_main_function( m_connection,
					    dbusWakeupMain,
					    this, 0 );
}

Integrator::Integrator( DBusServer *server, QObject *parent )
  : QObject( parent ), m_server( server )
{
  m_connection = reinterpret_cast<DBusConnection*>( m_server );
  m_timeouts.setAutoDelete( true );

  dbus_server_set_watch_functions( m_server,
                                   dbusAddWatch,
                                   dbusRemoveWatch,
                                   dbusToggleWatch,
                                   this, 0 );
  dbus_server_set_timeout_functions( m_server,
                                     dbusAddTimeout,
                                     dbusRemoveTimeout,
                                     dbusToggleTimeout,
                                     this, 0 );
  dbus_server_set_new_connection_function( m_server,
                                           dbusNewConnection,
                                           this,  0 );
}

void Integrator::slotRead( int fd )
{
  QIntDictIterator<Watch>	it( m_watches );
  for ( ; it.current(); ++it )
    dbus_watch_handle ( it.current()->watch, DBUS_WATCH_READABLE );

  emit readReady();
}

void Integrator::slotWrite( int fd )
{
  QIntDictIterator<Watch>       it( m_watches );
  for ( ; it.current(); ++it )
    dbus_watch_handle ( it.current()->watch, DBUS_WATCH_WRITABLE );
}

void Integrator::slotTimeout( DBusTimeout *timeout )
{
  dbus_timeout_handle( timeout );
}

void Integrator::addWatch( DBusWatch *watch )
{
  if ( !dbus_watch_get_enabled( watch ) )
    return;

  Watch *qtwatch = new Watch;
  qtwatch->watch = watch;

  int flags = dbus_watch_get_flags( watch );
  int fd = dbus_watch_get_fd( watch );

  if ( flags & DBUS_WATCH_READABLE ) {
    qtwatch->readSocket = new QSocketNotifier( fd, QSocketNotifier::Read, this );
    QObject::connect( qtwatch->readSocket, SIGNAL(activated(int)), SLOT(slotRead(int)) );
  }

  if (flags & DBUS_WATCH_WRITABLE) {
    qtwatch->writeSocket = new QSocketNotifier( fd, QSocketNotifier::Write, this );
    QObject::connect( qtwatch->writeSocket, SIGNAL(activated(int)), SLOT(slotWrite(int)) );
  }

  m_watches.insert( fd, qtwatch );
}

void Integrator::removeWatch( DBusWatch *watch )
{
  int key = dbus_watch_get_fd( watch );

  Watch *qtwatch = m_watches.take( key );

  if ( qtwatch ) {
    delete qtwatch->readSocket;  qtwatch->readSocket = 0;
    delete qtwatch->writeSocket; qtwatch->writeSocket = 0;
    delete qtwatch;
  }
}

void Integrator::addTimeout( DBusTimeout *timeout )
{
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
  Connection *con = new Connection( c, this );
  emit newConnection( con );
}

}//end namespace Internal
}//end namespace DBusQt

#include "integrator.moc"

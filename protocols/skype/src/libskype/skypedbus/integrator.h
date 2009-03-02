// -*- Mode: C++; c-basic-offset: 2; indent-tabs-mode: nil; -*-
/* integrator.h: integrates D-BUS into Qt event loop
 *
 * Copyright (C) 2003  Zack Rusin <zack@kde.org>
 *
 * Licensed under the Academic Free License version 2.1
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
#ifndef DBUS_QT_INTEGRATOR_H
#define DBUS_QT_INTEGRATOR_H

#include <qobject.h>

#include <qintdict.h>
#include <qptrdict.h>

#include "dbus/dbus.h"

class QTimer;

namespace DBusQt
{
  class Connection;

  namespace Internal
  {
    struct Watch;

    class Timeout : public QObject
    {
      Q_OBJECT
    public:
      Timeout( QObject *parent, DBusTimeout *t );
    public:
      void start();
    signals:
      void timeout( DBusTimeout* );
    protected slots:
      void slotTimeout();
    private:
      QTimer *m_timer;
      DBusTimeout *m_timeout;
    };

    class Integrator : public QObject
    {
      Q_OBJECT
    public:
      Integrator( DBusConnection *connection, QObject *parent );
      Integrator( DBusServer *server, QObject *parent );

    signals:
      void readReady();
      void newConnection( Connection* );

    protected slots:
      void slotRead( int );
      void slotWrite( int );
      void slotTimeout( DBusTimeout *timeout );

    public:
      void addWatch( DBusWatch* );
      void removeWatch( DBusWatch* );

      void addTimeout( DBusTimeout* );
      void removeTimeout( DBusTimeout* );

      void handleConnection( DBusConnection* );
    private:
      QIntDict<Watch> m_watches;
      QPtrDict<Timeout> m_timeouts;
      DBusConnection *m_connection;
      DBusServer *m_server;
    };
  }
}

#endif

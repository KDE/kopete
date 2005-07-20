// -*- Mode: C++; c-basic-offset: 2; indent-tabs-mode: nil; -*-

/* connection.h: Qt wrapper for DBusConnection
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

#ifndef DBUS_QT_CONNECTION_H
#define DBUS_QT_CONNECTION_H

#include "message.h"

#include <qobject.h>
#include <qstring.h>
#include <qevent.h>
#include <dbus/dbus.h>

const int DBUS_EVENT_WAKEUP = QEvent::User + 100;

namespace DBusQt
{
	namespace Internal
	{
		class Integrator;
	}

	class Connection : public QObject
	{
	Q_OBJECT
	public:
		Connection( QObject * parent = 0 );
		Connection( const QString & host, QObject * parent = 0 );
		Connection( DBusBusType type, QObject * parent = 0, bool temporary = FALSE );
		virtual ~ Connection(  );

		bool isConnected(  ) const;
		bool isAuthenticated(  ) const;

		Message borrowMessage(  );
		Message popMessage(  );
		void stealBorrowMessage( const Message & );
		void dbusMessage( DBusMessage * message );
		bool error(  );
		QString getError(  );

	public slots:
		void open( const QString & );
		void close(  );
		void flush(  );
		void send( const Message & );
		void sendWithReply( const Message & );
		Message sendWithReplyAndBlock( const Message & );
		bool registerObjectPath( const QString & path, const QString & service );

	protected slots:
		void dispatchRead(  );

	protected:
		void init( const QString & host );
		virtual void *virtual_hook( int id, void *data );
		bool event( QEvent * );

	private:
		friend class Internal::Integrator;
		DBusConnection *connection(  );
		Connection( DBusConnection * connection, QObject * parent );

	private:
		struct Private;
		Private *d;

	signals:
		void messageArrived( const DBusQt::Message & m );
	};

}

#endif

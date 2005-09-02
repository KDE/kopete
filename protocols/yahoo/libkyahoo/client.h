/*
    Kopete Yahoo Protocol
    
    Copyright (c) 2005 Andre Duffeck <andre.duffeck@kdemail.net>
    Copyright (c) 2004 Duncan Mac-Vicar P. <duncan@kde.org>
    Copyright (c) 2004 Matt Rogers <matt.rogers@kdemail.net>
    Copyright (c) 2004 SuSE Linux AG <http://www.suse.com>
    Copyright (C) 2003  Justin Karneges
    
    Kopete (c) 2002-2004 by the Kopete developers <kopete-devel@kde.org>
 
    *************************************************************************
    *                                                                       *
    * This library is free software; you can redistribute it and/or         *
    * modify it under the terms of the GNU Lesser General Public            *
    * License as published by the Free Software Foundation; either          *
    * version 2 of the License, or (at your option) any later version.      *
    *                                                                       *
    *************************************************************************
*/

#ifndef LIBYAHOO_CLIENT_H
#define LIBYAHOO_CLIENT_H

#include <qobject.h>

#include "rtf2html.h"
#include "transfer.h"


class QString;
class ClientStream;
class KNetworkConnector;
class Task;

class Client : public QObject
{
Q_OBJECT

	public:
	
		/*************
		  EXTERNAL API 
		 *************/
		  
		Client(QObject *parent=0);
		~Client();
		void setUserId( const QString& userName );

		/**
		 * Start a connection to the server using the supplied @ref ClientStream.
		 * This is only a transport layer connection.
		 * Needed for protocol action P1.
		 * @param s initialised client stream to use for the connection.
		 * @param server the server to connect to - but this is also set on the connector used to construct the clientstream??
		 * @param auth indicate whether we're connecting to the authorizer or the bos server
		 */
		void connect( const QString &host, const uint port, const QString &userId, const QString &pass );
		
		/**
		 * Logout and disconnect
		 */
		void close();

		/**
		 * Accessors needed for login
		 */
		QString host();
		int port();
		
		/*************
		  INTERNAL (FOR USE BY TASKS) METHODS 
		 *************/
		/**
		 * Send an outgoing request to the server
		 */
		void send( Transfer *request );
		
		/**
		 * Print a debug statement
		 */
		void debug( const QString &str );
		
		/**
		 * The current user's user ID
		 */
		QString userId();
		
		/**
		 * The current user's password
		 */
		QString password();
		
		/**
		 * Host's IP address
		 */
		QCString ipAddress();
		
		/**
		 * Access the root Task for this client, so tasks may be added to it.
		 */
		Task* rootTask();

	signals:
		/** CONNECTION EVENTS */
		
		/**
		 * Notifies that the login process has succeeded.
		 */
		void loggedIn( int, const QString& );

		/** 
		 * Notifies that the login process has failed 
		 */
		void loginFailed();
		
		/**
		 * Notifies tasks and account so they can react properly
		 */
		void connected();
		/**
		 * Notifies tasks and account so they can react properly
		 */
		void disconnected();
		/**
		 * We were disconnected because we connected elsewhere
		 */
		void connectedElsewhere();
		/**
		 * Notifies about our buddies and groups
		 */
		void gotBuddy( const QString &, const QString &, const QString & );
	protected slots:
		// INTERNAL, FOR USE BY TASKS' finished() SIGNALS //
		void lt_loginFinished();
		void cs_connected();
		void slotGotCookies();
		
		/**
		 * Used by the client stream to notify errors to upper layers.
		 */
		void streamError( int error );
		
		/**
		 * The client stream has data ready to read.
		 */
		void streamReadyRead();

	private:
		void distribute( Transfer *transfer );
		
		class ClientPrivate;
		ClientPrivate* d;
		KNetworkConnector *m_connector;
};

#endif

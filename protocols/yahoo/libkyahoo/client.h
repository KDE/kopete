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
#include "yahootypes.h"


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
		 * Specifies the status we connect with.
		 * May be Online or Invisible.
		 */
		void setStatusOnConnect( Yahoo::Status status );

		/**
		 * Accessors needed for login
		 */
		QString host();
		int port();

		/**
		 * Send a Typing notification
		 */
		void sendTyping( const QString &to, int typ);
		
		/**
		 * Send a Message
		 */
		void sendMessage( const QString &to, const QString &msg );

		/**
		 * Send a Buzz
		 */
		void sendBuzz( const QString &to );

		/**
		 * Change our status
		 */	
		void changeStatus(Yahoo::Status status, const QString &message, Yahoo::StatusType type);

		/**
		 * Set the verification word that is needed for a account verification after
		 * too many wrong login attempts.
		 */
		void setVerificationWord( const QString &word );

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
		 * current Session ID
		 */
		uint sessionID();
		
		/**
		 * return the pictureFlag describing the status of our buddy icon
		 * 0 = no icon, 2 = icon, 1 = avatar (?)
		 */
		int pictureFlag();

		/**
		 * Get our status
		 */
		Yahoo::Status status();

		/**
		 * Set our status
		 */
		void setStatus( Yahoo::Status );

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
		/**
		 * Notifies about the status of online buddies
		 */
		void statusChanged( const QString&, int, const QString&, int );
		/**
		 * Notifies about mails
		 */
		void mailNotify( const QString&, const QString&, int );
		/**
		 * We got a new message
		 */
		void gotIm( const QString&, const QString&, long, int );
		/**
		 * We got a new system message
		 */
		void systemMessage( const QString& );
		/**
		 * The buddy is typing a message
		 */
		void typingNotify( const QString &, int );
		/**
		 * Notifies about a BUZZ notification
		 */
		void gotBuzz( const QString &, long );
	protected slots:
		// INTERNAL, FOR USE BY TASKS' finished() SIGNALS //
		void lt_loginFinished();
		void lt_gotSessionID( uint );
		void cs_connected();
		void slotGotCookies();
		
		/**
		 * Used by tasks to identify a response to a login attempt
		 */
		void slotLoginResponse( int, const QString& );

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
		
		/**
		 * create static tasks and connect their signals
		 */
		void initTasks();

		/**
		 * remove static tasks and their singal connections
		 */
		void deleteTasks();

		class ClientPrivate;
		ClientPrivate* d;
		KNetworkConnector *m_connector;
};

#endif

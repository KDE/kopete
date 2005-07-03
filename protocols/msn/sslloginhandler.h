/*
    sslloginhandler.h - SSL login for MSN protocol

    Copyright (c) 2005      by Michaël Larouche       <shock@shockdev.ca.tc
    Copyright (c) 2002-2005 by Olivier Goffart        <ogoffart at kde.org>

    Kopete    (c) 2002-2005 by the Kopete developers  <kopete-devel@kde.org>

    Portions taken from
    KMess   (c) 2003      by Mike K. Bennett          <mkb137b@hotmail.com>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/


#ifndef SSLLOGINHANDLER_H
#define SSLLOGINHANDLER_H

#include <qobject.h>
#include <qstringlist.h>

// Forward declarations
class KExtendedSocket;
class KSSL;
class MimeMessage;

/**
  * This class handles the SSL portion of the login.
  * @author Mike K. Bennett  <mkb137b@hotmail.com>
  */
class SslLoginHandler : public QObject
{
   Q_OBJECT

public:
	SslLoginHandler();
	
	~SslLoginHandler();

    /**
	 * Start the login process
	 */
	void login( const QString &parameters, const QString &handle, const QString &password );

private:
	/**
	 * Get the authentication data from a string
	 */
	void parseAuthenticationData( const QString &data );
	/**
	 * Parse the HTTP response from the server
	 */
	void parseHttpResponse( const QString &data );
	/**
	 * Get login server data from a string
	 */
	void parseLoginServerData( QString &host, QString &page, QString serverData );
	/**
	 * Read data from the socket via SSL
	 */
    QString readSslData();
	/**
	 * Send the authenticationn request
	 */
	void sendAuthenticationRequest( const QString &loginServer, const QString &page );
	/**
	 * Send a HTTP request to the server
	 */
	void sendHttpRequest( const QString &request, const QString &host, int port );
	/**
	 * Request the name of the login server
	 */
	void sendLoginServerRequest( const QString &hostname );
	/**
	 * Write data to the socket via SSL
	 */
	void writeSslData( const QString &data );

private slots:
	/**
	 * We received data from the socket.
	 */
	void slotDataReceived();

	/**
	 * Detect a socket error
	 */
	void slotSocketError(int error);
	
private:
	/**
	 * The mode of the transfer.
	 */
	enum Mode{ NONE = 0, GETLOGINSERVER = 1, GETAUTHENTICATIONDATA = 2 } m_mode;
	/**
	 * The list of parameters sent by the notification server
	 */
	QString m_authenticationParameters;
	/**
	 * The cookies we received from the server
	 */
	QStringList m_cookies;
	/**
	 * The user's handle
	 */
	QString m_handle;
	/** 
	 * The user's password
	 */
	QString m_password;
	/**
	 * The socket over which the SSL data is written and read
	 */
	KExtendedSocket *m_socket;
	/**
	 * The SSL handler.
	 */
	KSSL *m_ssl;

signals:
	/**
	 * Signal that the login was aborted because an internal error occured
	 */
	void loginFailed();
	/**
	 * Signal that the login failed, username/password was incorrect
	 */
	void loginIncorrect();
	/**
	 * Signal that the login succeeded
	 */
	void loginSucceeded( QString authentication );
};

#endif

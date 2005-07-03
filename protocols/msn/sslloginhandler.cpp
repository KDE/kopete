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
#include "sslloginhandler.h"

#include <qstringlist.h>
#include <qregexp.h>
#include <qsocket.h>
#include <qurl.h>

#include <kdebug.h>
#include <kextsock.h>
#include <kssl.h>
#include <kurl.h>

#if 0
#include "../kmessdebug.h"
#include "mimemessage.h"
#else
//i didn't want to import the whole MimeMessage from Kmess for Kopete so i
// reimplemented the base here  -Olivier

class MimeMessage
{
	public:
		MimeMessage(const QString &msg) : message(msg) {}

		QString getValue(const QString &key)
		{
			QRegExp rx(key+": (.*)\n");
			rx.search(message);
			return rx.cap(1);
		}
	private:
		QString message;
};

#include "sslloginhandler.moc"
#endif
//there is nothing modified from here.  this is exactly the kmerlin code

/*
 * Great documentation about this can be found at
 * http://siebe.bot2k3.net/docs/
 */

// The constructor
SslLoginHandler::SslLoginHandler()
 : m_mode(NONE)
{
  // Create the SSL handler
	m_ssl = new KSSL( true );

	// Create and set up the socket.
	m_socket = new KExtendedSocket( );
	
	//m_socket->setSocketFlags( 0x00 | 0x600000 ); // 0x00 = anySocket | 0x600000 = bufferedSocket
	m_socket->setSocketFlags(0x00); // 0x00 = anySocket | 0x600000 = bufferedSocket
	m_socket->setTimeout(30);
	m_socket->enableRead(true);
	connect( m_socket, SIGNAL( readyRead() ),
			this, SLOT( slotDataReceived() ) );
	connect( m_socket, SIGNAL( connectionFailed(int) ),
			this, SLOT( slotSocketError(int) ) );
}

SslLoginHandler::~SslLoginHandler()
{
  delete m_ssl;
  delete m_socket;
}

// Data was received over the socket
void SslLoginHandler::slotDataReceived()
{
#ifdef DEBUG_SSLLOGIN
	kdDebug(14140) << k_funcinfo << "Data received." << endl;
	kdDebug(14140) << k_funcinfo << m_socket->bytesAvailable() << " bytes available." << endl;
	kdDebug(14140) << k_funcinfo << "SSL says " << m_ssl->pending() << " bytes available." << endl;
#endif

	QString data;
	int timeout = 0;
	const int maxIterations = 1000;

	// Read the data via SSL.
	while( (!data.contains( QRegExp("\r\n") ) ) && ( timeout < maxIterations ) )
	{
		data = readSslData();
		timeout++;
	}	
	
	 // Output the data for debugging
#ifdef DEBUG_SSLLOGIN
	kdDebug(14140) << k_funcinfo << "********************   Contents    ********************" << endl;
	kdDebug(14140) << data << endl;
	kdDebug(14140) << k_funcinfo << "********************  End of data  ********************" << endl;
#endif
	// Warn if timed out
	if ( timeout >= maxIterations )
	{
		kdDebug(14140) << "WARNING - SSL read timed out." << endl;
		emit loginFailed();
		return;
	}

	if ( data.length() > 0 )
	{
		parseHttpResponse(data);
	}
	else
	{
		kdDebug(14140) << "WARNING - Available data wasn't read from the SSL socket." << endl;
		emit loginFailed();
	}
}

// Start the login process
void SslLoginHandler::login( const QString &parameters, const QString &handle, const QString &password )
{
#ifdef DEBUG_SSLLOGIN
	kdDebug(14140) << k_funcinfo << "Starting login with parameters " << parameters << "." << endl;
#endif
	
	// Store the given data
	m_authenticationParameters = parameters;
	m_handle = handle;
	m_password = password;
	
	// Get the login server
	sendLoginServerRequest("nexus.passport.com");
	slotDataReceived();
}

// Get the authentication data from a string
void SslLoginHandler::parseAuthenticationData( const QString &data )
{
	QString twnData;
	
	// Pull TWN data out of the message
	twnData = data.right( data.length() - data.find(QRegExp("from-PP='")) - 9 );
	twnData = twnData.left( twnData.find(QRegExp("',")) );
	
#ifdef DEBUG_SSLLOGIN
	kdDebug(14140) << k_funcinfo << "data for TWN is " << twnData << "." << endl;
#endif
	
	// Notify the MsnNotificationConnection
	emit loginSucceeded(twnData);
}

// Parse the HTTP response from the server
void SslLoginHandler::parseHttpResponse(const QString &data)
{
	KURL location;
	int headerCode, headerEnd;
	QString header, headerText;
	
	// Parse the HTTP status header
	QRegExp re("HTTP/\\d+\\.\\d+ (\\d+) ([^\r\n]+)");
	headerEnd = data.find("\r\n");
	header = data.left( (headerEnd == -1) ? 20 : headerEnd );
	
	re.search(header);
	headerCode = re.cap(1).toUInt();
	headerText = re.cap(2);
	
	// Create a MimeMessage, removing the HTTP status header
	MimeMessage message( data.section( ",", 1 ) );
	
	switch(m_mode)
	{
		case GETLOGINSERVER:
		{
			// Step 1. This data describes the login server to use.
			if(headerCode == 302)
			{
				// HTTP Redirect
				location = KURL( message.getValue( "Location" ) );
				sendLoginServerRequest(location.host());
			}
			else
			{
				// Parse the data
				QString loginServer;
				QString page;
				parseLoginServerData( loginServer, page, message.getValue("PassportURLs") );
		
				// Send the authentication request
				sendAuthenticationRequest( loginServer, page );
			}
			break;
		}
		case GETAUTHENTICATIONDATA:
		{
			// Step 2. Get the authentication data
			if(headerCode == 200)
			{
				// Login success
				parseAuthenticationData(message.getValue("Authentication-Info"));
			}
			else if(headerCode == 302)
			{
				// HTTP Redirect
				location = KURL( message.getValue( "Location" ) );
				sendAuthenticationRequest(location.host(), location.path());
			}
			else if(headerCode == 401)
			{
				// Got a HTTP "401 Unauthorized"; Login failed
				emit loginIncorrect();
			}
			else
			{
				kdDebug(14140) << k_funcinfo << "WARNING "
						<< "- Unhandled response code " << headerCode << " " << headerText << endl;
				emit loginFailed();
			}
			break;
		}
		default:
		{
			kdDebug(14140) << k_funcinfo << "WARNING - Entered illegal state" << endl;
			emit loginFailed();
		}
	}
}

// Get login server data from a string
void SslLoginHandler::parseLoginServerData( QString &host, QString &page, QString serverData )
{
	int slashIndex;
	
	// Get everything between "DLLogin=" and to the comma.
	serverData = serverData.right( serverData.length() - serverData.find( "DALogin=" ) - 8 );
	serverData = serverData.left( serverData.find( "," ) );
	
#ifdef DEBUG_SSLLOGIN
	kdDebug(14140) << k_funcinfo << "host/page=" << serverData << endl;
#endif
	
	// Separate the "host/page" string.
	slashIndex = serverData.find( "/" );
	host = serverData.left( slashIndex );
	page = serverData.right( serverData.length() - slashIndex );
	
#ifdef DEBUG_SSLLOGIN
	kdDebug(14140) << k_funcinfo << "host=" << host << " page=" << page << endl;
#endif
}

// Read data from the socket via SSL
QString SslLoginHandler::readSslData()
{
	char rawblock[1024];
	QCString block;
	QString data = "";
	int noBytesRead = 1;
	
	// Read data from the SSL socket.
	if ( m_ssl != 0L )
	{
		while(noBytesRead > 0)
		{
			noBytesRead = m_ssl->read( rawblock, 1024 );
#ifdef DEBUG_SSLLOGIN
			kdDebug(14140) << k_funcinfo << noBytesRead << " bytes read." << endl;
#endif
			block = rawblock;
			block = block.left( noBytesRead );
			data += QString::fromUtf8( block );
		}
	}
	
	return data;
}

// Send the authenticationn request
void SslLoginHandler::sendAuthenticationRequest( const QString &loginServer, const QString &page )
{
#ifdef DEBUG_SSLLOGIN
	kdDebug(14140) << k_funcinfo << "Step 2. Requesting authentication data." << endl;
#endif
	
	QString request;
	QString encodedHandle = KURL::encode_string(m_handle);
	QString encodedPassword = KURL::encode_string(m_password);
	
	request = "GET " + page + " HTTP/1.1\r\n"
				"Authorization: Passport1.4"
				" OrgVerb=GET"
				",OrgURL=http%3A%2F%2Fmessenger%2Emsn%2Ecom"
				",sign-in=" + encodedHandle +
				",pwd="     + encodedPassword +
				","         + m_authenticationParameters + "\r\n"
				"User-Agent: MSMSGS\r\n"     // Make sure the server won't discriminate
				"Host: " + loginServer + "\r\n"
				"Connection: Keep-Alive\r\n"
				"Cache-Control: no-cache\r\n\r\n";
	
	// Step 2. Send the authorisation request
	m_mode = GETAUTHENTICATIONDATA;
	sendHttpRequest( request, loginServer, 443 );
}

// Send a HTTP request to the server
void SslLoginHandler::sendHttpRequest(const QString &request, const QString &host, int port )
{
	QString response;
	QString responseBody;
	
	if(m_socket == 0L)
	{
		kdDebug(14140) << k_funcinfo << "WARNING "
				<< "- Trying to login using a null socket." << endl;
		return;
	}
	
	// Configure the socket
#ifdef DEBUG_SSLLOGIN
	kdDebug(14140) << k_funcinfo << "Close and reset the socket." << endl;
#endif
	m_ssl->setAutoReconfig(true);
	m_ssl->reInitialize();
	
	m_socket->closeNow();
	m_socket->reset();
	
	// Try to connect
#ifdef DEBUG_SSLLOGIN
	kdDebug(14140) << k_funcinfo << "Connecting to " << host << ":" << port << "." << endl;
#endif
	m_socket->setAddress( host, port );
	m_socket->lookup();
	int connectionSuccess = m_socket->connect();
	if ( connectionSuccess != 0 )
	{
		kdDebug(14140) << k_funcinfo << "WARNING "
				<< "- Connection failed, giving " << connectionSuccess << endl;
		return;
	}
	
	// Try to wrap the SSL handler
#ifdef DEBUG_SSLLOGIN
	kdDebug(14140) << k_funcinfo << "Connection success, binding SSL to socket fd " << m_socket->fd() << endl;
#endif
	int sslConnectionSuccess = m_ssl->connect( m_socket->fd() );
	if ( sslConnectionSuccess != 1 )
	{
		kdDebug(14140) << k_funcinfo << "WARNING "
				<< "- SSL Connection failed, giving " << sslConnectionSuccess << endl;
		return;
	}
	
	// Send the request
#ifdef DEBUG_SSLLOGIN
	kdDebug(14140) << k_funcinfo << "SSL connected OK, sending the request." << endl;
	kdDebug(14140) << request;
#endif
	writeSslData( request );
}

// Request the name of the login server
void SslLoginHandler::sendLoginServerRequest(const QString &hostname)
{
#ifdef DEBUG_SSLLOGIN
	kdDebug(14140) << k_funcinfo << "Step 1. Requesting the login server." << endl;
#endif
	
	// Step 1. Send the login server request
	// The server will respond with the location of the main SSL server.
	m_mode = GETLOGINSERVER;
	sendHttpRequest( "GET /rdr/pprdr.asp\r\n\r\n", hostname, 443 );
}

// Detect a socket error
void SslLoginHandler::slotSocketError(int error)
{
	kdDebug(14140) << k_funcinfo << "WARNING - Received error " << error << " from the socket." << endl;
}

// Write data to the socket via SSL
void SslLoginHandler::writeSslData( const QString &data )
{
	int noBytesWritten;
	
	if(m_socket != 0 && m_ssl != 0)
	{
		noBytesWritten = m_ssl->write( data.latin1(), data.length() );
		if(noBytesWritten != (int)data.length())
		{
			kdDebug(14140) << "WARNING - Wanted to write " << data.length() << " to the socket, "
					<< " wrote " << noBytesWritten << "." << endl;
		}
#ifdef DEBUG_SSLLOGIN
		else
		{
			kdDebug(14140) << k_funcinfo << "Sent " << noBytesWritten << " bytes via SSL." << endl;
		}
#endif
	}
}

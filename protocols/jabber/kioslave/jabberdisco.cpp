
/***************************************************************************
                   Jabber Service Discovery KIO Slave
                             -------------------
    begin                : Wed June 1 2005
    copyright            : (C) 2005 by Till Gerken <till@tantalo.net>

	   Kopete (C) 2001-2005 Kopete developers <kopete-devel@kde.org>
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <kdebug.h>

#include "jabberdisco.h"

#include <stdlib.h>
#include <qcstring.h>
#include <qthread.h>
#include <kdebug.h>
#include <klocale.h>
#include <kapplication.h>
#include <kmessagebox.h>

#include <xmpp_tasks.h>
#include "jabberclient.h"

JabberDiscoProtocol::JabberDiscoProtocol ( const QCString &pool_socket, const QCString &app_socket )
	: KIO::SlaveBase ( "kio_jabberdisco", pool_socket, app_socket )
{
	kdDebug ( JABBER_DISCO_DEBUG ) << k_funcinfo << "Slave launched." << endl;

	m_jabberClient = 0l;
	m_connected = false;

}


JabberDiscoProtocol::~JabberDiscoProtocol ()
{
	kdDebug ( JABBER_DISCO_DEBUG ) << k_funcinfo << "Slave is shutting down." << endl;

	delete m_jabberClient;

}

void JabberDiscoProtocol::setHost ( const QString &host, int port, const QString &user, const QString &pass )
{
	kdDebug ( JABBER_DISCO_DEBUG ) << k_funcinfo << " Host " << host << ", port " << port << ", user " << user << endl;

	m_host = host;
	m_port = !port ? 5222 : port;
	m_user = QString(user).replace ( "%", "@" );
	m_password = pass;

}

void JabberDiscoProtocol::openConnection ()
{
	kdDebug ( JABBER_DISCO_DEBUG ) << k_funcinfo << endl;

	if ( m_connected )
	{
		return;
	}

	// instantiate new client backend or clean up old one
	if ( !m_jabberClient )
	{
		m_jabberClient = new JabberClient;
	
		QObject::connect ( m_jabberClient, SIGNAL ( csDisconnected () ), this, SLOT ( slotCSDisconnected () ) );
		QObject::connect ( m_jabberClient, SIGNAL ( csError ( int ) ), this, SLOT ( slotCSError ( int ) ) );
		QObject::connect ( m_jabberClient, SIGNAL ( tlsWarning ( int ) ), this, SLOT ( slotHandleTLSWarning ( int ) ) );
		QObject::connect ( m_jabberClient, SIGNAL ( connected () ), this, SLOT ( slotConnected () ) );
		QObject::connect ( m_jabberClient, SIGNAL ( error ( JabberClient::ErrorCode ) ), this, SLOT ( slotClientError ( JabberClient::ErrorCode ) ) );

		QObject::connect ( m_jabberClient, SIGNAL ( debugMessage ( const QString & ) ),
				   this, SLOT ( slotClientDebugMessage ( const QString & ) ) );
	}
	else
	{
		m_jabberClient->disconnect ();
	}

	// we need to use the old protocol for now
	m_jabberClient->setUseXMPP09 ( true );

	// set SSL flag (this should be converted to forceTLS when using the new protocol)
	m_jabberClient->setUseSSL ( false );

	// override server and port (this should be dropped when using the new protocol and no direct SSL)
	m_jabberClient->setOverrideHost ( true, m_host, m_port );

	// allow plaintext password authentication or not?
	m_jabberClient->setAllowPlainTextPassword ( false );

	switch ( m_jabberClient->connect ( XMPP::Jid ( m_user + QString("/") + "JabberBrowser" ), m_password ) )
	{
		case JabberClient::NoTLS:
			// no SSL support, at the connecting stage this means the problem is client-side
			error ( KIO::ERR_UPGRADE_REQUIRED, i18n ( "TLS" ) );
			break;

		case JabberClient::Ok:
		default:
			// everything alright!
			kdDebug ( JABBER_DISCO_DEBUG ) << k_funcinfo << "Waiting for socket to open..." << endl;
			break;
	}

	connected ();

}

void JabberDiscoProtocol::closeConnection ()
{
	kdDebug ( JABBER_DISCO_DEBUG ) << k_funcinfo << endl;

	if ( m_jabberClient )
	{
		m_jabberClient->disconnect ();
	}

}

void JabberDiscoProtocol::slave_status ()
{
	kdDebug ( JABBER_DISCO_DEBUG ) << k_funcinfo << endl;

	slaveStatus ( m_host, m_connected );

}

void JabberDiscoProtocol::get ( const KURL &url )
{
	kdDebug ( JABBER_DISCO_DEBUG ) << k_funcinfo << endl;

	m_command = Get;
	m_url = url;

	mimeType ( "inode/directory" );

	finished ();

}

void JabberDiscoProtocol::listDir ( const KURL &url )
{
	kdDebug ( JABBER_DISCO_DEBUG ) << k_funcinfo << endl;

	m_command = ListDir;
	m_url = url;

	openConnection ();

}

void JabberDiscoProtocol::mimetype ( const KURL &/*url*/ )
{
	kdDebug ( JABBER_DISCO_DEBUG ) << k_funcinfo << endl;

	mimeType("inode/directory");

	finished ();

}

void JabberDiscoProtocol::slotClientDebugMessage ( const QString &msg )
{

	kdDebug ( JABBER_DISCO_DEBUG ) << k_funcinfo << msg << endl;

}

void JabberDiscoProtocol::slotHandleTLSWarning ( int validityResult )
{
	kdDebug ( JABBER_DISCO_DEBUG ) << k_funcinfo << "Handling TLS warning..." << endl;

	if ( messageBox ( KIO::SlaveBase::WarningContinueCancel,
					  i18n ( "The server certificate is invalid. Do you want to continue? " ),
					  i18n ( "Certificate Warning" ) ) == KMessageBox::Continue )
	{
		// resume stream
		m_jabberClient->continueAfterTLSWarning ();
	}
	else
	{
		// disconnect stream
		closeConnection ();
	}

}

void JabberDiscoProtocol::slotClientError ( JabberClient::ErrorCode errorCode )
{
	kdDebug ( JABBER_DISCO_DEBUG ) << k_funcinfo << "Handling client error..." << endl;

	switch ( errorCode )
	{
		case JabberClient::NoTLS:
		default:
			error ( KIO::ERR_UPGRADE_REQUIRED, i18n ( "TLS" ) );
			closeConnection ();
			break;
	}

}

void JabberDiscoProtocol::slotConnected ()
{
	kdDebug ( JABBER_DISCO_DEBUG ) << k_funcinfo << "Connected to Jabber server." << endl;

	XMPP::JT_DiscoItems *discoTask;

	m_connected = true;

	// now execute command
	switch ( m_command )
	{
		case ListDir:	// list a directory
						kdDebug ( JABBER_DISCO_DEBUG ) << k_funcinfo << "Listing directory..." << endl;
						discoTask = new XMPP::JT_DiscoItems ( m_jabberClient->rootTask () );
						connect ( discoTask, SIGNAL ( finished () ), this, SLOT ( slotQueryFinished () ) );
						discoTask->get ( m_host );
						discoTask->go ( true );
						break;

		case Get:		// retrieve an item
						kdDebug ( JABBER_DISCO_DEBUG ) << k_funcinfo << "Retrieving item..." << endl;
						break;

		default:		// do nothing by default
						kdDebug ( JABBER_DISCO_DEBUG ) << k_funcinfo << "Unknown command " << m_command << endl;
						break;
	}

}

void JabberDiscoProtocol::slotQueryFinished ()
{
	kdDebug ( JABBER_DISCO_DEBUG ) << "Query task finished" << endl;

	XMPP::JT_DiscoItems * task = (XMPP::JT_DiscoItems *) sender ();

	if (!task->success ())
	{
		error ( KIO::ERR_COULD_NOT_READ, "" );
		return;
	}

	XMPP::DiscoList::const_iterator itemsEnd = task->items().end ();
	for (XMPP::DiscoList::const_iterator it = task->items().begin (); it != itemsEnd; ++it)
	{
		KIO::UDSAtom atom;
		KIO::UDSEntry entry;
		
		atom.m_uds = KIO::UDS_NAME;
		atom.m_str = (*it).jid().userHost ();
		entry.prepend ( atom );

		atom.m_uds = KIO::UDS_SIZE;
		atom.m_long = 0;
		entry.prepend ( atom );

		atom.m_uds = KIO::UDS_LINK_DEST;
		atom.m_str = (*it).name ();
		entry.prepend ( atom );

		atom.m_uds = KIO::UDS_MIME_TYPE;
		atom.m_str = "inode/directory";
		entry.prepend ( atom );

		atom.m_uds = KIO::UDS_SIZE;
		atom.m_long = 0;
		entry.prepend ( atom );

		listEntry ( entry, false );

	}

	listEntry ( KIO::UDSEntry(), true );

	finished ();

}

void JabberDiscoProtocol::slotCSDisconnected ()
{
	kdDebug ( JABBER_DISCO_DEBUG ) << k_funcinfo << "Disconnected from Jabber server." << endl;

	/*
	 * We should delete the JabberClient instance here,
	 * but timers etc prevent us from doing so. Iris does
	 * not like to be deleted from a slot.
	 */
	m_connected = false;

}

void JabberDiscoProtocol::slotCSError ( int errorCode )
{
	kdDebug ( JABBER_DISCO_DEBUG ) << k_funcinfo << "Error in stream signalled." << endl;

	if ( ( errorCode == XMPP::ClientStream::ErrAuth )
		&& ( m_jabberClient->clientStream()->errorCondition () == XMPP::ClientStream::NotAuthorized ) )
	{
		kdDebug ( JABBER_DISCO_DEBUG ) << k_funcinfo << "Incorrect password, retrying." << endl;

		KIO::AuthInfo authInfo;
		authInfo.username = m_user;
		authInfo.password = m_password;
		if ( openPassDlg ( authInfo, i18n ( "The login details are incorrect. Do you want to try again?" ) ) )
		{
			m_user = authInfo.username;
			m_password = authInfo.password;
			closeConnection ();
			openConnection ();
		}
		else
		{
			closeConnection ();
			error ( KIO::ERR_COULD_NOT_AUTHENTICATE, "" );
		}
	}
	else
	{
		closeConnection ();
		error ( KIO::ERR_CONNECTION_BROKEN, "" );
	}

}

bool breakEventLoop = false;

class EventLoopThread : public QThread
{
public:
	void run ();
};

void EventLoopThread::run ()
{

	while ( true )
	{
		qApp->processEvents ();
		msleep ( 100 );

		if ( breakEventLoop )
			break;
	}

}

void JabberDiscoProtocol::dispatchLoop ()
{

	EventLoopThread eventLoopThread;

	eventLoopThread.start ();
	SlaveBase::dispatchLoop ();
	breakEventLoop = true;
	eventLoopThread.wait ();

}

extern "C"
{
	KDE_EXPORT int kdemain(int argc, char **argv);
}


int kdemain ( int argc, char **argv )
{
	KApplication app(argc, argv, "kio_jabberdisco", false, true);

	kdDebug(JABBER_DISCO_DEBUG) << k_funcinfo << endl;

	if ( argc != 4 )
	{
		kdDebug(JABBER_DISCO_DEBUG) << "Usage: kio_jabberdisco protocol domain-socket1 domain-socket2" << endl;
		exit(-1);
	}

	JabberDiscoProtocol slave ( argv[2], argv[3] );
	slave.dispatchLoop ();

	return 0;
}

#include "jabberdisco.moc"

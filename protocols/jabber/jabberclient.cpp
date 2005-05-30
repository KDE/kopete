
/***************************************************************************
                jabberclient.cpp - Generic Jabber Client Class
                             -------------------
    begin                : Sat May 25 2005
    copyright            : (C) 2005 by Till Gerken <till@tantalo.net>

			   Kopete (C) 2001-2005 Kopete developers
			   <kopete-devel@kde.org>.
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "jabberclient.h"

#include <qtimer.h>
#include <qregexp.h>

#include <qca.h>
#include <bsocket.h>
#include <filetransfer.h>
#include <xmpp_tasks.h>

#include "jabberconnector.h"

#define JABBER_PENALTY_TIME	2

XMPP::S5BServer *JabberClient::m_s5bServer = 0L;
QStringList JabberClient::m_s5bAddressList;
int JabberClient::m_s5bServerPort = 8010;

JabberClient::JabberClient ()
{

	m_jabberClient = 0L;
	m_jabberClientStream = 0L;
	m_jabberClientConnector = 0L;
	m_jabberTLS = 0L;
	m_jabberTLSHandler = 0L;

	cleanUp ();

	// initiate penalty timer
	QTimer::singleShot ( JABBER_PENALTY_TIME * 1000, this, SLOT ( slotUpdatePenaltyTime () ) );

}

JabberClient::~JabberClient ()
{

}

void JabberClient::cleanUp ()
{
	if ( m_jabberClient )
	{
		m_jabberClient->close ();
	}
	
	delete m_jabberClient;
	delete m_jabberClientStream;
	delete m_jabberClientConnector;
	delete m_jabberTLSHandler;
	delete m_jabberTLS;

	m_jabberClient = 0L;
	m_jabberClientStream = 0L;
	m_jabberClientConnector = 0L;
	m_jabberTLSHandler = 0L;
	m_jabberTLS = 0L;

	m_currentPenaltyTime = 0;

	m_jid = XMPP::Jid ();
	m_password = QString::null;

	setForceTLS ( false );
	setUseSSL ( false );
	setUseXMPP09 ( false );
	setProbeSSL ( false );

	setOverrideHost ( false );

	setAllowPlainTextPassword ( true );

	setFileTransfersEnabled ( false );
	setS5BServerPort ( 8010 );

	setClientName ( QString::null );
	setClientVersion ( QString::null );
	setOSName ( QString::null );

	setTimeZone ( "UTC", 0 );

	setIgnoreTLSWarnings ( false );

}

void JabberClient::slotUpdatePenaltyTime ()
{

	if ( m_currentPenaltyTime >= JABBER_PENALTY_TIME )
		m_currentPenaltyTime -= JABBER_PENALTY_TIME;
	else
		m_currentPenaltyTime = 0;

	QTimer::singleShot ( JABBER_PENALTY_TIME * 1000, this, SLOT ( slotUpdatePenaltyTime () ) );

}

void JabberClient::setIgnoreTLSWarnings ( bool flag )
{

	m_ignoreTLSWarnings = flag;

}

bool JabberClient::ignoreTLSWarnings ()
{

	return m_ignoreTLSWarnings;

}

bool JabberClient::setS5BServerPort ( int port )
{

	m_s5bServerPort = port;

	if ( fileTransfersEnabled () )
	{
		return s5bServer()->start ( port );
	}

	return true;

}

int JabberClient::s5bServerPort () const
{

	return m_s5bServerPort;

}

XMPP::S5BServer *JabberClient::s5bServer ()
{

	if ( !m_s5bServer )
	{
		m_s5bServer = new XMPP::S5BServer ();
		QObject::connect ( m_s5bServer, SIGNAL ( destroyed () ), this, SLOT ( slotS5BServerGone () ) );

		/*
		 * Try to start the server at the default port here.
		 * We have no way of notifying the caller of an error.
		 * However, since the caller will usually also
		 * use setS5BServerPort() to ensure the correct
		 * port, we can return an error code there.
		 */
		if ( fileTransfersEnabled () )
		{
			s5bServer()->start ( m_s5bServerPort );
		}
	}

	return m_s5bServer;

}

void JabberClient::slotS5BServerGone ()
{

	m_s5bServer = 0L;

	if ( m_jabberClient )
		m_jabberClient->s5bManager()->setServer( 0L );

}

void JabberClient::addS5BServerAddress ( const QString &address )
{
	QStringList newList;

	m_s5bAddressList.append ( address );

	// now filter the list without dupes
	for ( QStringList::Iterator it = m_s5bAddressList.begin (); it != m_s5bAddressList.end (); ++it )
	{
		if ( !newList.contains ( *it ) )
			newList.append ( *it );
	}

	s5bServer()->setHostList ( newList );

}

void JabberClient::removeS5BServerAddress ( const QString &address )
{
	QStringList newList;

	QStringList::iterator it = m_s5bAddressList.find ( address );
	if ( it != m_s5bAddressList.end () )
	{
		m_s5bAddressList.remove ( it );
	}

	if ( m_s5bAddressList.isEmpty () )
	{
		delete m_s5bServer;
		m_s5bServer = 0L;
	}
	else
	{
		// now filter the list without dupes
		for ( QStringList::Iterator it = m_s5bAddressList.begin (); it != m_s5bAddressList.end (); ++it )
		{
			if ( !newList.contains ( *it ) )
				newList.append ( *it );
		}

		s5bServer()->setHostList ( newList );
	}

}

void JabberClient::setForceTLS ( bool flag )
{

	m_forceTLS = flag;

}

bool JabberClient::forceTLS () const
{

	return m_forceTLS;

}

void JabberClient::setUseSSL ( bool flag )
{

	m_useSSL = flag;

}

bool JabberClient::useSSL () const
{

	return m_useSSL;

}

void JabberClient::setUseXMPP09 ( bool flag )
{

	m_useXMPP09 = flag;

}

bool JabberClient::useXMPP09 () const
{

	return m_useXMPP09;

}

void JabberClient::setProbeSSL ( bool flag )
{

	m_probeSSL = flag;

}

bool JabberClient::probeSSL () const
{

	return m_probeSSL;

}

void JabberClient::setOverrideHost ( bool flag, const QString &server, int port )
{

	m_overrideHost = flag;
	m_server = server;
	m_port = port;

}

bool JabberClient::overrideHost () const
{

	return m_overrideHost;

}

void JabberClient::setAllowPlainTextPassword ( bool flag )
{

	m_allowPlainTextPassword = flag;

}

bool JabberClient::allowPlainTextPassword () const
{

	return m_allowPlainTextPassword;

}

void JabberClient::setFileTransfersEnabled ( bool flag, const QString &localAddress )
{

	m_fileTransfersEnabled = flag;
	m_localAddress = localAddress;

}

QString JabberClient::localAddress () const
{

	return m_localAddress;

}

bool JabberClient::fileTransfersEnabled () const
{

	return m_fileTransfersEnabled;

}

void JabberClient::setClientName ( const QString &clientName )
{

	m_clientName = clientName;

}

QString JabberClient::clientName () const
{

	return m_clientName;

}

void JabberClient::setClientVersion ( const QString &clientVersion )
{

	m_clientVersion = clientVersion;

}

QString JabberClient::clientVersion () const
{

	return m_clientVersion;

}

void JabberClient::setOSName ( const QString &osName )
{

	m_osName = osName;

}

QString JabberClient::osName () const
{

	return m_osName;

}

void JabberClient::setTimeZone ( const QString &timeZoneName, int timeZoneOffset )
{

	m_timeZoneName = timeZoneName;
	m_timeZoneOffset = timeZoneOffset;

}

QString JabberClient::timeZoneName () const
{

	return m_timeZoneName;

}

int JabberClient::timeZoneOffset () const
{

	return m_timeZoneOffset;

}

int JabberClient::getPenaltyTime ()
{

	int currentTime = m_currentPenaltyTime;

	m_currentPenaltyTime += JABBER_PENALTY_TIME;

	return currentTime;

}

XMPP::Client *JabberClient::client () const
{

	return m_jabberClient;

}

XMPP::ClientStream *JabberClient::clientStream () const
{

	return m_jabberClientStream;

}

JabberConnector *JabberClient::clientConnector () const
{

	return m_jabberClientConnector;

}

XMPP::Task *JabberClient::rootTask () const
{

	if ( client () )
	{
		return client()->rootTask ();
	}
	else
	{
		return 0l;
	}

}

XMPP::FileTransferManager *JabberClient::fileTransferManager () const
{

	if ( client () )
	{
		return client()->fileTransferManager ();
	}
	else
	{
		return 0L;
	}

}

XMPP::Jid JabberClient::jid () const
{

	return m_jid;

}

JabberClient::ErrorCode JabberClient::connect ( const XMPP::Jid &jid, const QString &password, bool auth )
{
	/*
	 * Close any existing connection.
	 */
	if ( m_jabberClient )
	{
		m_jabberClient->close ();
	}

	m_jid = jid;
	m_password = password;

	/*
	 * Return an error if we should force TLS but it's not available.
	 */
	if ( ( forceTLS () || useSSL () || probeSSL () ) && !QCA::isSupported ( QCA::CAP_TLS ) )
	{
		return NoTLS;
	}

	/*
	 * Instantiate connector, responsible for dealing with the socket.
	 * This class uses KDE's socket code, which in turn makes use of
	 * the global proxy settings.
	 */
	m_jabberClientConnector = new JabberConnector;

	m_jabberClientConnector->setOptSSL ( useSSL () );

	if ( useXMPP09 () )
	{
		if ( overrideHost () )
		{
			m_jabberClientConnector->setOptHostPort ( m_server, m_port );
		}

		m_jabberClientConnector->setOptProbe ( probeSSL () );

	}

	/*
	 * Setup authentication layer
	 */
	if ( QCA::isSupported ( QCA::CAP_TLS ) )
	{
		m_jabberTLS = new QCA::TLS;
		m_jabberTLSHandler = new XMPP::QCATLSHandler ( m_jabberTLS );

		{
			using namespace XMPP;
			QObject::connect ( m_jabberTLSHandler, SIGNAL ( tlsHandshaken() ), this, SLOT ( slotTLSHandshaken () ) );
		}

		QPtrList<QCA::Cert> certStore;
		m_jabberTLS->setCertificateStore ( certStore );
	}

	/*
	 * Instantiate client stream which handles the network communication by referring
	 * to a connector (proxying etc.) and a TLS handler (security layer)
	 */
	m_jabberClientStream = new XMPP::ClientStream ( m_jabberClientConnector, m_jabberTLSHandler );

	{
		using namespace XMPP;
		QObject::connect ( m_jabberClientStream, SIGNAL ( needAuthParams(bool, bool, bool) ),
				   this, SLOT ( slotCSNeedAuthParams (bool, bool, bool) ) );
		QObject::connect ( m_jabberClientStream, SIGNAL ( authenticated () ),
				   this, SLOT ( slotCSAuthenticated () ) );
		QObject::connect ( m_jabberClientStream, SIGNAL ( connectionClosed () ),
				   this, SLOT ( slotCSDisconnected () ) );
		QObject::connect ( m_jabberClientStream, SIGNAL ( delayedCloseFinished () ),
				   this, SLOT ( slotCSDisconnected () ) );
		QObject::connect ( m_jabberClientStream, SIGNAL ( warning (int) ),
				   this, SLOT ( slotCSWarning (int) ) );
		QObject::connect ( m_jabberClientStream, SIGNAL ( error (int) ),
				   this, SLOT ( slotCSError (int) ) );
	}

	m_jabberClientStream->setOldOnly ( useXMPP09 () );

	/*
	 * Initiate anti-idle timer (will be triggered every 55 seconds).
	 */
	m_jabberClientStream->setNoopTime ( 55000 );

	/*
	 * Allow plaintext password authentication or not?
	 */
	m_jabberClientStream->setAllowPlain( allowPlainTextPassword () );

	/*
	 * Setup client layer.
	 */
	m_jabberClient = new XMPP::Client ( this );

	/*
	 * Enable file transfer (IP and server will be set after connection
	 * has been established.
	 */
	if ( fileTransfersEnabled () )
	{
		m_jabberClient->setFileTransferEnabled ( true );

		{
			using namespace XMPP;
			QObject::connect ( m_jabberClient->fileTransferManager(), SIGNAL ( incomingReady() ),
					   this, SLOT ( slotIncomingFileTransfer () ) );
		}
	}

	/* This should only be done here to connect the signals, otherwise it is a
	 * bad idea.
	 */
	{
		using namespace XMPP;
		QObject::connect ( m_jabberClient, SIGNAL ( subscription (const Jid &, const QString &) ),
				   this, SLOT ( slotSubscription (const Jid &, const QString &) ) );
		QObject::connect ( m_jabberClient, SIGNAL ( rosterRequestFinished ( bool, int, const QString & ) ),
				   this, SLOT ( slotRosterRequestFinished ( bool, int, const QString & ) ) );
		QObject::connect ( m_jabberClient, SIGNAL ( rosterItemAdded (const RosterItem &) ),
				   this, SLOT ( slotNewContact (const RosterItem &) ) );
		QObject::connect ( m_jabberClient, SIGNAL ( rosterItemUpdated (const RosterItem &) ),
				   this, SLOT ( slotContactUpdated (const RosterItem &) ) );
		QObject::connect ( m_jabberClient, SIGNAL ( rosterItemRemoved (const RosterItem &) ),
				   this, SLOT ( slotContactDeleted (const RosterItem &) ) );
		QObject::connect ( m_jabberClient, SIGNAL ( resourceAvailable (const Jid &, const Resource &) ),
				   this, SLOT ( slotResourceAvailable (const Jid &, const Resource &) ) );
		QObject::connect ( m_jabberClient, SIGNAL ( resourceUnavailable (const Jid &, const Resource &) ),
				   this, SLOT ( slotResourceUnavailable (const Jid &, const Resource &) ) );
		QObject::connect ( m_jabberClient, SIGNAL ( messageReceived (const Message &) ),
				   this, SLOT ( slotReceivedMessage (const Message &) ) );
		QObject::connect ( m_jabberClient, SIGNAL ( groupChatJoined (const Jid &) ),
				   this, SLOT ( slotGroupChatJoined (const Jid &) ) );
		QObject::connect ( m_jabberClient, SIGNAL ( groupChatLeft (const Jid &) ),
				   this, SLOT ( slotGroupChatLeft (const Jid &) ) );
		QObject::connect ( m_jabberClient, SIGNAL ( groupChatPresence (const Jid &, const Status &) ),
				   this, SLOT ( slotGroupChatPresence (const Jid &, const Status &) ) );
		QObject::connect ( m_jabberClient, SIGNAL ( groupChatError (const Jid &, int, const QString &) ),
				   this, SLOT ( slotGroupChatError (const Jid &, int, const QString &) ) );
		//QObject::connect ( m_jabberClient, SIGNAL (debugText (const QString &) ),
		//		   this, SLOT ( slotPsiDebug (const QString &) ) );
		QObject::connect ( m_jabberClient, SIGNAL ( xmlIncoming(const QString& ) ),
				   this, SLOT ( slotIncomingXML (const QString &) ) );
		QObject::connect ( m_jabberClient, SIGNAL ( xmlOutgoing(const QString& ) ),
				   this, SLOT ( slotOutgoingXML (const QString &) ) );
	}

	m_jabberClient->setClientName ( clientName () );
	m_jabberClient->setClientVersion ( clientVersion () );
	m_jabberClient->setOSName ( osName () );

	m_jabberClient->setTimeZone ( timeZoneName (), timeZoneOffset () );

	m_jabberClient->connectToServer ( m_jabberClientStream, jid, auth );

	return Ok;

}

void JabberClient::disconnect ()
{

	if ( m_jabberClient )
	{
		m_jabberClient->close ();
	}
	else
	{
		cleanUp ();
	}

}

bool JabberClient::isConnected () const
{

	if ( m_jabberClient )
	{
		return m_jabberClient->isActive ();
	}

	return false;

}

void JabberClient::changePassword ( const QString &password )
{

	if ( !isConnected () )
	{
		return;
	}

	XMPP::JT_Register * task = new XMPP::JT_Register ( m_jabberClient->rootTask () );
	QObject::connect ( task, SIGNAL ( finished () ), this, SLOT ( slotChangePasswordDone () ) );

	task->changepw ( password );
	task->go ( true );

}

void JabberClient::slotChangePasswordDone ()
{
	XMPP::JT_Register * task = (XMPP::JT_Register *) sender ();

	emit passwordChanged ( task->success () );
}

void JabberClient::joinGroupChat ( const QString &host, const QString &room, const QString &nick )
{

	client()->groupChatJoin ( host, room, nick );

}

void JabberClient::leaveGroupChat ( const QString &host, const QString &room )
{

	client()->groupChatLeave ( host, room );

}

void JabberClient::sendMessage ( const XMPP::Message &message )
{

	client()->sendMessage ( message );

}

void JabberClient::send ( const QString &packet )
{

	client()->send ( packet );

}

void JabberClient::requestRoster ()
{

	client()->rosterRequest ();

}

void JabberClient::slotPsiDebug ( const QString & _msg )
{
	QString msg = _msg;

	msg = msg.replace( QRegExp( "<password>[^<]*</password>\n" ), "<password>[Filtered]</password>\n" );
	msg = msg.replace( QRegExp( "<digest>[^<]*</digest>\n" ), "<digest>[Filtered]</digest>\n" );

	emit debugMessage ( "Psi: " + msg );

}

void JabberClient::slotIncomingXML ( const QString & _msg )
{
	QString msg = _msg;

	msg = msg.replace( QRegExp( "<password>[^<]*</password>\n" ), "<password>[Filtered]</password>\n" );
	msg = msg.replace( QRegExp( "<digest>[^<]*</digest>\n" ), "<digest>[Filtered]</digest>\n" );

	emit debugMessage ( "XML IN: " + msg );

}

void JabberClient::slotOutgoingXML ( const QString & _msg )
{
	QString msg = _msg;

	msg = msg.replace( QRegExp( "<password>[^<]*</password>\n" ), "<password>[Filtered]</password>\n" );
	msg = msg.replace( QRegExp( "<digest>[^<]*</digest>\n" ), "<digest>[Filtered]</digest>\n" );

	emit debugMessage ( "XML OUT: " + msg );

}

void JabberClient::slotTLSHandshaken ()
{

	emit debugMessage ( "TLS handshake done, testing certificate validity..." );

	// FIXME: in the future, this should be handled by KDE, not QCA
	int validityResult = m_jabberTLS->certificateValidityResult ();

	if ( validityResult == QCA::TLS::Valid )
	{
		emit debugMessage ( "Certificate is valid, continuing." );

		// valid certificate, continue
		m_jabberTLSHandler->continueAfterHandshake ();
	}
	else
	{
		emit debugMessage ( "Certificate is not valid, asking user what to do next." );

		// certificate is not valid, query the user
		if ( ignoreTLSWarnings () )
		{
			emit debugMessage ( "We are supposed to ignore TLS warnings, continuing." );
			m_jabberTLSHandler->continueAfterHandshake ();
		}

		emit tlsWarning ( validityResult );
	}

}

void JabberClient::continueAfterTLSWarning ()
{

	if ( m_jabberTLSHandler )
	{
		m_jabberTLSHandler->continueAfterHandshake ();
	}

}

void JabberClient::slotCSNeedAuthParams ( bool user, bool pass, bool realm )
{
	emit debugMessage ( "Sending auth credentials..." );

	if ( user )
	{
		m_jabberClientStream->setUsername ( jid().node () );
	}

	if ( pass )
	{
		m_jabberClientStream->setPassword ( m_password );
	}

	if ( realm )
	{
		m_jabberClientStream->setRealm ( jid().domain () );
	}

	m_jabberClientStream->continueAfterParams ();

}

void JabberClient::slotCSAuthenticated ()
{
	emit debugMessage ( "Connected to Jabber server." );

	/*
	 * Determine local IP address.
	 * FIXME: This is ugly!
	 */
	if ( localAddress().isEmpty () )
	{
		// code for Iris-type bytestreams
		ByteStream *irisByteStream = m_jabberClientConnector->stream();
		if ( irisByteStream->inherits ( "BSocket" ) || irisByteStream->inherits ( "XMPP::BSocket" ) )
		{
			m_localAddress = ( (BSocket *)irisByteStream )->address().toString ();
		}

		// code for the KDE-type bytestream
		JabberByteStream *kdeByteStream = dynamic_cast<JabberByteStream*>(m_jabberClientConnector->stream());
		if ( kdeByteStream )
		{
			m_localAddress = kdeByteStream->socket()->localAddress().nodeName ();
		}
	}

	if ( fileTransfersEnabled () )
	{
		// setup file transfer
		addS5BServerAddress ( localAddress () );
		m_jabberClient->s5bManager()->setServer ( s5bServer () );
	}

	// start the client operation
	m_jabberClient->start ( jid().domain (), jid().node (), m_password, jid().resource () );

	emit connected ();
}

void JabberClient::slotCSDisconnected ()
{

	/* FIXME:
	 * We should delete the XMPP::Client instance here,
	 * but timers etc prevent us from doing so. (Psi does
	 * not like to be deleted from a slot).
	 */

	emit debugMessage ( "Disconnected, freeing up file transfer port..." );

	// delete local address from S5B server
	removeS5BServerAddress ( localAddress () );

	emit csDisconnected ();

}

void JabberClient::slotCSWarning ( int warning )
{

	emit debugMessage ( "Client stream warning." );

	/*
	 * FIXME: process all other warnings
	 */
	switch ( warning )
	{
		//case XMPP::ClientStream::WarnOldVersion:
		case XMPP::ClientStream::WarnNoTLS:
			if ( forceTLS () )
			{
				disconnect ();
				emit error ( NoTLS );
				return;
			}
			break;
	}

	m_jabberClientStream->continueAfterWarning ();

}

void JabberClient::slotCSError ( int error )
{

	emit debugMessage ( "Client stream error." );

	emit csError ( error );

}

void JabberClient::slotRosterRequestFinished ( bool success, int /*statusCode*/, const QString &/*statusString*/ )
{

	emit rosterRequestFinished ( success );

}

void JabberClient::slotIncomingFileTransfer ()
{

	emit incomingFileTransfer ();

}

void JabberClient::slotNewContact ( const XMPP::RosterItem &item )
{

	emit newContact ( item );

}

void JabberClient::slotContactDeleted ( const RosterItem &item )
{

	emit contactDeleted ( item );

}

void JabberClient::slotContactUpdated ( const RosterItem &item )
{

	emit contactUpdated ( item );

}

void JabberClient::slotResourceAvailable ( const Jid &jid, const Resource &resource )
{

	emit resourceAvailable ( jid, resource );

}

void JabberClient::slotResourceUnavailable ( const Jid &jid, const Resource &resource )
{

	emit resourceUnavailable ( jid, resource );

}

void JabberClient::slotReceivedMessage ( const Message &message )
{

	emit messageReceived ( message );

}

void JabberClient::slotGroupChatJoined ( const Jid &jid )
{

	emit groupChatJoined ( jid );

}

void JabberClient::slotGroupChatLeft ( const Jid &jid )
{

	emit groupChatLeft ( jid );

}

void JabberClient::slotGroupChatPresence ( const Jid &jid, const Status &status)
{

	emit groupChatPresence ( jid, status );

}

void JabberClient::slotGroupChatError ( const Jid &jid, int error, const QString &reason)
{

	emit groupChatError ( jid, error, reason );

}

void JabberClient::slotSubscription ( const Jid &jid, const QString &type )
{

	emit subscription ( jid, type );

}

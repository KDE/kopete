/*
    gwaccount.cpp - Kopete GroupWise Protocol

    Copyright (c) 2004      SUSE Linux AG	 	 http://www.suse.com
    
    Based on Testbed   
    Copyright (c) 2003      by Will Stephenson		 <will@stevello.free-online.co.uk>
    
    Kopete    (c) 2002-2003 by the Kopete developers <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This library is free software; you can redistribute it and/or         *
    * modify it under the terms of the GNU General Public                   *
    * License as published by the Free Software Foundation; either          *
    * version 2 of the License, or (at your option) any later version.      *
    *                                                                       *
    *************************************************************************
*/

#include "gwaccount.h"

#include <kaction.h>
#include <kdebug.h>
#include <klocale.h>
#include <kpopupmenu.h>

#include "kopetemetacontact.h"
#include "kopetepassword.h"

#include "client.h"
#include "qca.h"
#include "gwcontact.h"
#include "gwfakeserver.h"
#include "gwprotocol.h"
#include "gwclientstream.h"
#include "gwconnector.h"
#include "qcatlshandler.h"

#include <sys/utsname.h>

GroupWiseAccount::GroupWiseAccount( GroupWiseProtocol *parent, const QString& accountID, const char *name )
: Kopete::PasswordedAccount ( parent, accountID, 0, name )
{
	// Init the myself contact
	// FIXME: I think we should add a global self metaContact (Olivier)
	KopeteMetaContact *metaContact = new KopeteMetaContact;
	setMyself( new GroupWiseContact( this, accountId(), metaContact, "myself", 0, 0, 0 ) );
	myself()->setOnlineStatus( GroupWiseProtocol::protocol()->groupwiseOffline );
	
	m_connector = 0;
	m_QCATLS = 0;
	m_tlsHandler = 0;
	m_clientStream = 0;
	m_client= 0;
}

GroupWiseAccount::~GroupWiseAccount()
{
}

KActionMenu* GroupWiseAccount::actionMenu()
{
	KActionMenu *theActionMenu = new KActionMenu(accountId(), myself()->onlineStatus().iconFor(this) , this);
	theActionMenu->popupMenu()->insertTitle(myself()->icon(), i18n("GroupWise (%1)").arg(accountId()));
	
	theActionMenu->insert( new KAction (GroupWiseProtocol::protocol()->groupwiseAvailable.caption(),
		GroupWiseProtocol::protocol()->groupwiseAvailable.iconFor(this), 0, this, SLOT ( slotGoOnline() ), this,
		"actionGroupWiseConnect") );
	theActionMenu->insert( new KAction (GroupWiseProtocol::protocol()->groupwiseAway.caption(),
		GroupWiseProtocol::protocol()->groupwiseAway.iconFor(this), 0, this, SLOT ( slotGoAway() ), this,
		"actionGroupWiseAway") );
	// CUSTOMS GO HERE ?
	theActionMenu->insert( new KAction (GroupWiseProtocol::protocol()->groupwiseBusy.caption(),
		GroupWiseProtocol::protocol()->groupwiseBusy.iconFor(this), 0, this, SLOT ( slotGoBusy() ), this,
		"actionGroupWiseBusy") );
	theActionMenu->insert( new KAction ( "A&ppear Offline", "jabber_invisible", 0, this, 
		SLOT( slotGoAppearOffline() ), this, 
		"actionGroupWiseAppearOffline") );
	theActionMenu->insert( new KAction (GroupWiseProtocol::protocol()->groupwiseOffline.caption(),
		GroupWiseProtocol::protocol()->groupwiseOffline.iconFor(this), 0, this, SLOT ( slotGoOffline() ), this,
		"actionGroupWiseOfflineDisconnect") );

	return theActionMenu;
}

bool GroupWiseAccount::addContactToMetaContact(const QString& contactId, const QString& displayName, KopeteMetaContact* parentContact)
{
	kdDebug ( GROUPWISE_DEBUG_GLOBAL ) << k_funcinfo << "contactId: " << contactId << " displayName: " << displayName
			<< endl;
	//GroupWiseContact* newContact = new GroupWiseContact( this, contactId, GroupWiseContact::Echo, displayName, parentContact );
	//return newContact != 0L;
	return false;
}

const int GroupWiseAccount::port() const
{
	return 8300;
	return pluginData( protocol(), "Port" ).toInt();
}

const QString GroupWiseAccount::server() const
{
	return "reiser.suse.de";
	return pluginData( protocol(), "Server" );
}

void GroupWiseAccount::setAway( bool away, const QString & /* reason */ )
{
	if ( away )
		slotGoAway();
	else
		slotGoOnline();
}

void GroupWiseAccount::connectWithPassword( const QString &password )
{
	// set up network classes
	m_connector = new KNetworkConnector( 0 );
	//myConnector->setOptHostPort( "localhost", 8300 );
	m_connector->setOptHostPort( server(), port() );
	m_connector->setOptSSL( true );
	Q_ASSERT( QCA::isSupported(QCA::CAP_TLS) );
	m_QCATLS = new QCA::TLS;
	m_tlsHandler = new QCATLSHandler( m_QCATLS );
	m_clientStream = new ClientStream( m_connector, m_tlsHandler, 0);
	
	QObject::connect( m_connector, SIGNAL( error() ), this, SLOT( slotConnError() ) );
	QObject::connect( m_connector, SIGNAL( connected() ), this, SLOT( slotConnConnected() ) );
	
	QObject::connect (m_clientStream, SIGNAL (connectionClosed()),
				this, SLOT (slotCSDisconnected()));
	QObject::connect (m_clientStream, SIGNAL (delayedCloseFinished()),
				this, SLOT (slotCSDisconnected()));
	// Notify us when the transport layer is connected
	QObject::connect( m_clientStream, SIGNAL( connected() ), SLOT( slotCSConnected() ) );
	// it's necessary to catch this signal and tell the TLS handler to proceed
	// even if we don't check cert validity
	QObject::connect( m_tlsHandler, SIGNAL(tlsHandshaken()), SLOT( slotTLSHandshaken()) );
	// starts the client once the security layer is up, but see below
	QObject::connect( m_clientStream, SIGNAL( securityLayerActivated(int) ), SLOT( slotTLSReady(int) ) );
	// we could handle login etc in start(), in which case we would emit this signal after that
	//QObject::connect (jabberClientStream, SIGNAL (authenticated()),
	//			this, SLOT (slotCSAuthenticated ()));
	// we could also get do the actual login in response to this..
	//QObject::connect (m_clientStream, SIGNAL (needAuthParams(bool, bool, bool)),
	//			this, SLOT (slotCSNeedAuthParams (bool, bool, bool)));
	
	// not implemented: warning 
	QObject::connect( m_clientStream, SIGNAL( warning(int) ), SLOT( slotCSWarning(int) ) );
	// not implemented: error 
	QObject::connect( m_clientStream, SIGNAL( error(int) ), SLOT( slotCSError(int) ) );
	
	m_client = new Client( this );
	
	// TODO: Connect Client signals
	
	struct utsname utsBuf;

	uname (&utsBuf);
	
	/*jabberClient->setClientName ("Kopete");
	jabberClient->setClientVersion (kapp->aboutData ()->version ());
	jabberClient->setOSName (QString ("%1 %2").arg (utsBuf.sysname, 1).arg (utsBuf.release, 2)); */

	kdDebug ( GROUPWISE_DEBUG_GLOBAL ) << k_funcinfo << "Connecting to GroupWise server " << server() << ":" << port() << endl;

	NovellDN dn;
	dn.dn = "maeuschen";
	dn.server = "reiser.suse.de";
	m_client->connectToServer( m_clientStream, dn, true ); 
}

void GroupWiseAccount::disconnect()
{
	kdDebug ( GROUPWISE_DEBUG_GLOBAL ) << k_funcinfo << endl;
	myself()->setOnlineStatus( GroupWiseProtocol::protocol()->groupwiseOffline );
}

void GroupWiseAccount::slotGoOnline()
{
	kdDebug ( GROUPWISE_DEBUG_GLOBAL ) << k_funcinfo << endl;

	if (!isConnected ())
		connect ();
	else
		myself()->setOnlineStatus( GroupWiseProtocol::protocol()->groupwiseAvailable );
}

void GroupWiseAccount::slotGoAway()
{
	kdDebug ( GROUPWISE_DEBUG_GLOBAL ) << k_funcinfo << endl;

	if (!isConnected ())
		connect();
	
	myself()->setOnlineStatus( GroupWiseProtocol::protocol()->groupwiseAway );
}

void GroupWiseAccount::slotGoBusy()
{
	kdDebug ( GROUPWISE_DEBUG_GLOBAL ) << k_funcinfo << endl;

	if (!isConnected ())
		connect();
	
	myself()->setOnlineStatus( GroupWiseProtocol::protocol()->groupwiseBusy );
}

void GroupWiseAccount::slotGoAppearOffline()
{
	kdDebug ( GROUPWISE_DEBUG_GLOBAL ) << k_funcinfo << endl;

	if (!isConnected ())
		connect();
}

void GroupWiseAccount::slotGoOffline()
{
	kdDebug ( GROUPWISE_DEBUG_GLOBAL ) << k_funcinfo << endl;

	if (isConnected ())
		disconnect ();
	updateContactStatus();
}

void GroupWiseAccount::slotConnError()
{
	kdDebug ( GROUPWISE_DEBUG_GLOBAL ) << k_funcinfo << endl;
}

void GroupWiseAccount::slotConnConnected()
{
	kdDebug ( GROUPWISE_DEBUG_GLOBAL ) << k_funcinfo << endl;
}

void GroupWiseAccount::slotCSDisconnected()
{
	kdDebug ( GROUPWISE_DEBUG_GLOBAL ) << k_funcinfo << "Disconnected from Groupwise server." << endl;
}

void GroupWiseAccount::slotCSConnected()
{
	kdDebug ( GROUPWISE_DEBUG_GLOBAL ) << k_funcinfo << "Connected to Groupwise server." << endl;
}

void GroupWiseAccount::slotCSError( int error )
{
	kdDebug ( GROUPWISE_DEBUG_GLOBAL ) << k_funcinfo << "Got error from ClientStream:" << error << endl;
}

void GroupWiseAccount::slotCSWarning( int warning )
{
	kdDebug ( GROUPWISE_DEBUG_GLOBAL ) << k_funcinfo << "Got warning from ClientStream:" << warning << endl;
}

void GroupWiseAccount::slotTLSHandshaken()
{
	kdDebug ( GROUPWISE_DEBUG_GLOBAL ) << k_funcinfo << "TLS handshake complete" << endl;
	int validityResult = m_QCATLS->certificateValidityResult ();

	if( validityResult == QCA::TLS::Valid )
	{
		kdDebug ( GROUPWISE_DEBUG_GLOBAL ) << "Certificate is valid, continuing." << endl;
		// valid certificate, continue
		m_tlsHandler->continueAfterHandshake ();
	}
	else
	{
		kdDebug ( GROUPWISE_DEBUG_GLOBAL ) << "Certificate is not valid, continuing anyway" << endl;
		// certificate is not valid, query the user
		/*			if(handleTLSWarning (validityResult, server (), myself()->contactId ()) == KMessageBox::Continue)
					{*/
		m_tlsHandler->continueAfterHandshake ();
		/*			}
					else
					{
					disconnect ( KopeteAccount::Manual );
					}*/
	}


}

void GroupWiseAccount::slotTLSReady( int secLayerCode )
{
	// i don't know what secLayerCode is for...
	kdDebug( GROUPWISE_DEBUG_GLOBAL ) << k_funcinfo << endl;
	m_client->start( server(), accountId(), password().cachedValue() );
}

void GroupWiseAccount::receivedMessage( const QString &message )
{
	// Look up the contact the message is from
	QString from;
	GroupWiseContact* messageSender;
	
	from = message.section( ':', 0, 0 );
	//from = QString::fromLatin1("echo");
	messageSender = static_cast<GroupWiseContact *>( contacts ()[ from ] );
	
	kdDebug( GROUPWISE_DEBUG_GLOBAL ) << k_funcinfo << " got a message from " << from << ", " << messageSender << ", is: " << message << endl;
	// Pass it on to the contact to process and display via a KMM
	messageSender->receivedMessage( message );
}

void GroupWiseAccount::updateContactStatus()
{
	QDictIterator<KopeteContact> itr( contacts() );
	for ( ; itr.current(); ++itr )
		itr.current()->setOnlineStatus( myself()->onlineStatus() );
}

void GroupWiseAccount::slotGotMyDetails( Field::FieldList & fields )
{
	Field::FieldBase* current = 0;
	QString cn, dn, givenName, surname, fullName, awayMessage, authAttribute;
	int status;
	Field::FieldListIterator it;
	Field::FieldListIterator end = fields.end();
	if ( ( it = fields.find ( NM_A_SZ_AUTH_ATTRIBUTE ) ) != end )
		authAttribute = static_cast<Field::SingleField*>( *it )->value().toString();
	if ( ( it = fields.find ( NM_A_SZ_DN ) ) != end )
		dn = static_cast<Field::SingleField*>( *it )->value().toString();
	if ( ( it = fields.find ( "CN" ) ) != end )
		cn = static_cast<Field::SingleField*>( *it )->value().toString();
	if ( ( it = fields.find ( "Given Name" ) ) != end )
		givenName = static_cast<Field::SingleField*>( *it )->value().toString();
	if ( ( it = fields.find ( "Surname" ) ) != end )
		surname = static_cast<Field::SingleField*>( *it )->value().toString();
	if ( ( it = fields.find ( "Full Name" ) ) != end )
		fullName = static_cast<Field::SingleField*>( *it )->value().toString();
	if ( ( it = fields.find ( NM_A_SZ_STATUS ) ) != end )
		status = static_cast<Field::SingleField*>( *it )->value().toString().toInt();
	if ( ( it = fields.find ( NM_A_SZ_MESSAGE_BODY ) ) != end )
		awayMessage = static_cast<Field::SingleField*>( *it )->value().toString();
	
	myself()->setProperty( GroupWiseProtocol::protocol()->propCN, cn );
	myself()->setProperty( GroupWiseProtocol::protocol()->propGivenName, givenName );
	myself()->setProperty( GroupWiseProtocol::protocol()->propLastName, surname );
	myself()->setProperty( GroupWiseProtocol::protocol()->propFullName, fullName );
	myself()->setProperty( GroupWiseProtocol::protocol()->propAwayMessage, awayMessage );
}
#include "gwaccount.moc"

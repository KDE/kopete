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

#include <qvaluelist.h>

#include <kaction.h>
#include <kdebug.h>
#include <klocale.h>
#include <kpopupmenu.h>

#include "kopeteawayaction.h"
#include "kopetecontactlist.h"
#include "kopetegroup.h"
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
: Kopete::PasswordedAccount ( parent, accountID, 0, "groupwiseaccount" )
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
	theActionMenu->insert( new KopeteAwayAction (GroupWiseProtocol::protocol()->groupwiseAway.caption(),
		GroupWiseProtocol::protocol()->groupwiseAway.iconFor(this), 0, this, SLOT ( slotGoAway( const QString & ) ), this,
		"actionGroupWiseAway") );
	theActionMenu->insert( new KopeteAwayAction (GroupWiseProtocol::protocol()->groupwiseBusy.caption(),
		GroupWiseProtocol::protocol()->groupwiseBusy.iconFor(this), 0, this, SLOT ( slotGoBusy( const QString & ) ), this,
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
	return pluginData( KopeteAccount::protocol(), "Port" ).toInt();
}

const QString GroupWiseAccount::server() const
{
	return "reiser.suse.de";
	return pluginData( KopeteAccount::protocol(), "Server" );
}

GroupWiseProtocol *GroupWiseAccount::protocol()
{
	return static_cast<GroupWiseProtocol *>( KopeteAccount::protocol() );
}

void GroupWiseAccount::setAway( bool away, const QString & reason )
{
	if ( away )
		setStatus( GroupWise::Away, reason);
	else
		setStatus( GroupWise::Available );
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
	// we connected successfully
	QObject::connect( m_client, SIGNAL( loggedIn() ), SLOT( slotLoggedIn() ) );
	// folder listed
	QObject::connect( m_client, SIGNAL( folderReceived( const FolderItem & ) ), SLOT( receiveFolder( const FolderItem & ) ) );
	// contact listed
	QObject::connect( m_client, SIGNAL( contactReceived( const ContactItem & ) ), SLOT( receiveContact( const ContactItem & ) ) );
	// contact details listed
	QObject::connect( m_client, SIGNAL( contactUserDetailsReceived( const ContactDetails & ) ), SLOT( receiveContactUserDetails( const ContactDetails & ) ) );
	// contact status changed
	QObject::connect( m_client, SIGNAL( statusReceived( const QString &, Q_UINT16, const QString & ) ), SLOT( receiveStatus( const QString &, Q_UINT16 , const QString & ) ) );
	// incoming message
	QObject::connect( m_client, SIGNAL( messageReceived( const ConferenceEvent & ) ), SLOT( receiveMessage( const ConferenceEvent & ) ) );
	
	QObject::connect( m_client, SIGNAL( ourStatusChanged( GroupWise::Status, const QString &, const QString & ) ), SLOT( changeOurStatus( GroupWise::Status, const QString &, const QString & ) ) );
	
	QObject::connect( m_client, SIGNAL( conferenceCreated( const int, const QString & ) ), SIGNAL( conferenceCreated( const int, const QString & ) ) );
	
	struct utsname utsBuf;

	uname (&utsBuf);
	
	/*jabberClient->setClientName ("Kopete");
	jabberClient->setClientVersion (kapp->aboutData ()->version ());
	jabberClient->setOSName (QString ("%1 %2").arg (utsBuf.sysname, 1).arg (utsBuf.release, 2)); */

	kdDebug ( GROUPWISE_DEBUG_GLOBAL ) << k_funcinfo << "Connecting to GroupWise server " << server() << ":" << port() << endl;

	NovellDN dn;
	dn.dn = "maeuschen";
	dn.server = "reiser.suse.de";
	myself()->setOnlineStatus( protocol()->groupwiseConnecting );
	m_client->connectToServer( m_clientStream, dn, true ); 
}

void GroupWiseAccount::disconnect()
{
	kdDebug ( GROUPWISE_DEBUG_GLOBAL ) << k_funcinfo << endl;
	myself()->setOnlineStatus( GroupWiseProtocol::protocol()->groupwiseOffline );
}

void GroupWiseAccount::setStatus( GroupWise::Status status, const QString & reason )
{
	if ( !(myself()->onlineStatus() == protocol()->groupwiseConnecting ) )
	{
		if ( isConnected() )
		{
			m_client->setStatus( status, reason );
			//myself()->setOnlineStatus( GroupWiseProtocol::protocol()->groupwiseAway );
		}
		else
		{
			kdDebug ( GROUPWISE_DEBUG_GLOBAL ) << k_funcinfo << "Must be connected before changing status" << endl;
			m_initialStatus = status;
			m_initialReason = reason;
			connect();
		}
	}
}

void GroupWiseAccount::createConference( const int clientId, const QStringList& invitees )
{
	kdDebug ( GROUPWISE_DEBUG_GLOBAL ) << k_funcinfo << endl;
	// TODO: remove this it prevents sending a list of participants with the createconf
	m_client->createConference( clientId , invitees );
}

void GroupWiseAccount::slotGoOnline()
{
	kdDebug ( GROUPWISE_DEBUG_GLOBAL ) << k_funcinfo << endl;
	setStatus( GroupWise::Available );
}

void GroupWiseAccount::slotGoAway( const QString & reason )
{
	kdDebug ( GROUPWISE_DEBUG_GLOBAL ) << k_funcinfo << endl;
	setStatus( GroupWise::Away, reason );
}

void GroupWiseAccount::slotGoBusy( const QString & reason )
{
	kdDebug ( GROUPWISE_DEBUG_GLOBAL ) << k_funcinfo << endl;
	setStatus( GroupWise::Busy, reason );
}

void GroupWiseAccount::slotGoAppearOffline()
{
	kdDebug ( GROUPWISE_DEBUG_GLOBAL ) << k_funcinfo << endl;
	setStatus( GroupWise::Offline );
}

void GroupWiseAccount::slotGoOffline()
{
	kdDebug ( GROUPWISE_DEBUG_GLOBAL ) << k_funcinfo << "NOT IMPLEMENTED" << endl;
	myself()->setOnlineStatus( GroupWiseProtocol::protocol()->groupwiseOffline );
}

void GroupWiseAccount::slotLoggedIn()
{
	kdDebug ( GROUPWISE_DEBUG_GLOBAL ) << k_funcinfo << endl;
	myself()->setOnlineStatus( protocol()->groupwiseAvailable );
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

void GroupWiseAccount::receiveMessage( const ConferenceEvent & event )
{
	kdDebug( GROUPWISE_DEBUG_GLOBAL ) << k_funcinfo << " got a message in conference: " << event.guid << ",  from: " << event.user << ", message is: " << event.message << endl;
	GroupWiseContact * contactFrom = static_cast<GroupWiseContact *>( contacts()[ event.user ] );
	if ( contactFrom )
	{
		contactFrom->handleIncomingMessage( event );
	}
	else
	{
		kdDebug (GROUPWISE_DEBUG_GLOBAL) << k_funcinfo << event.user << " is unknown to us, requesting details so we can create a temporary contact." << endl;
		m_pendingEvents.append( event );
		// get their details
		QStringList userDNsList;
		userDNsList.append( event.user );
		// the client will signal contactUserDetailsReceived when the details arrive, 
		// and we'll add a temporary contact in receiveContactUserDetails, before coming back to this method
		m_client->requestDetails( userDNsList );
	}
}

void GroupWiseAccount::updateContactStatus()
{
	QDictIterator<KopeteContact> itr( contacts() );
	for ( ; itr.current(); ++itr )
		itr.current()->setOnlineStatus( myself()->onlineStatus() );
}

void GroupWiseAccount::slotGotMyDetails( Field::FieldList & fields )
{
/*	Field::FieldBase* current = 0;
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
	myself()->setProperty( GroupWiseProtocol::protocol()->propAwayMessage, awayMessage );*/
}

void GroupWiseAccount::receiveFolder( const FolderItem & folder )
{
	kdDebug( GROUPWISE_DEBUG_GLOBAL ) << k_funcinfo 
			<< " objectId: " << folder.id 
			<< " sequence: " << folder.sequence 
			<< " parentId: " << folder.parentId 
			<< " displayName: " << folder.name << endl;
	if ( folder.parentId != 0 )
	{
		kdWarning( GROUPWISE_DEBUG_GLOBAL ) << " - received a nested folder.  These werre not supported in GroupWise or Kopete as of Sept 2004, aborting! (parentId = " << folder.parentId << ")" << endl;
		return;
	}
	
	bool found = false;
	QPtrList<KopeteGroup> groupList = KopeteContactList::contactList()->groups();
	for ( KopeteGroup *grp = groupList.first(); grp; grp = groupList.next() )
		if ( grp->displayName() == folder.name )
		{
			grp->setPluginData( protocol(), accountId() + " objectId", QString::number( folder.id ) );
			found = true;
			break;
		}

	if ( found )
		kdDebug( GROUPWISE_DEBUG_GLOBAL ) << " - folder already exists locally" << endl;
	else
	{
		kdDebug( GROUPWISE_DEBUG_GLOBAL ) << " - creating local folder" << endl;
		KopeteGroup * grp = new KopeteGroup( folder.name );
		grp->setPluginData( protocol(), accountId() + " objectId", QString::number( folder.id ) );
		KopeteContactList::contactList()->addGroup( grp );
	}
}

void GroupWiseAccount::receiveContact( const ContactItem & contact )
{
	kdDebug( GROUPWISE_DEBUG_GLOBAL ) << k_funcinfo 
			<< " objectId: " << contact.id 
			<< ", sequence: " << contact.sequence 
			<< ", parentId: " << contact.parentId 
			<< ", dn: " << contact.dn 
			<< ", displayName: " << contact.displayName << endl;
			
	// HACK: lowercased DN
	GroupWiseContact * c = static_cast<GroupWiseContact *>( contacts()[ contact.dn.lower() ] );
	if ( c )
	{
		kdDebug( GROUPWISE_DEBUG_GLOBAL ) << " - found contact in list, checking to see it's in this group " << endl;
		// check the metacontact is in the group this listing-of-the-contact is in...
		KopeteMetaContact *metaContact = c->metaContact();
		KopeteGroupList groupList = KopeteContactList::contactList()->groups();
		for ( KopeteGroup *grp = groupList.first(); grp; grp = groupList.next() )
		{
			kdDebug( GROUPWISE_DEBUG_GLOBAL ) << " - seeking in group named: " << grp->displayName() << ", id: " << grp->pluginData( protocol(), accountId() + " objectId" ).toInt() << " our parentId is: " << contact.parentId << endl;
			if ( grp->pluginData( protocol(), accountId() + " objectId" ).toInt() == contact.parentId )
			{
				kdDebug( GROUPWISE_DEBUG_GLOBAL ) << " - matches, adding." << endl;
				metaContact->addToGroup( grp ); //addToGroup() is safe to call if already a member
				break;
			}
		}
		// need to update parentId? and sequence
	}
	else
	{
		KopeteMetaContact *metaContact = new KopeteMetaContact();
		// HACK: lowercased DN
		GroupWiseContact * c = new GroupWiseContact( this, contact.dn.lower(), metaContact, contact.displayName, contact.id, contact.parentId, contact.sequence );
		KopeteGroupList groupList = KopeteContactList::contactList()->groups();
		for ( KopeteGroup *grp = groupList.first(); grp; grp = groupList.next() )
		{
			if ( grp->pluginData( protocol(), accountId() + " objectId" ).toInt() == contact.parentId )
			{
				metaContact->addToGroup( grp );
				break;
			}
		}
		KopeteContactList::contactList()->addMetaContact( metaContact );
	}
}

void GroupWiseAccount::receiveContactUserDetails( const ContactDetails & details )
{
	kdDebug( GROUPWISE_DEBUG_GLOBAL ) << k_funcinfo 
		<< "Auth attribute: " << details.authAttribute
		<< ", Away message: " << details.awayMessage
		<< ", CN" << details.cn
		<< ", DN" << details.dn
		<< ", fullName" << details.fullName
		<< ", surname" << details.surname
		<< ", givenname" << details.givenName
		<< ", status" << details.status
		<< endl;
	// HACK: lowercased DN 
	if ( !details.dn.isNull() )
	{
		GroupWiseContact * detailsOwner= static_cast<GroupWiseContact *>( contacts()[ details.dn ] );
		if( detailsOwner )
		{
			kdDebug( GROUPWISE_DEBUG_GLOBAL ) << " - updating details for " << details.dn << endl;
			detailsOwner->updateDetails( details );
		}
		else
		{
			kdDebug( GROUPWISE_DEBUG_GLOBAL ) << " - got details for " << details.dn << ", now creating a temporary contact and delivering any pending messages" << endl;
			// we asked for the user's details because we got a message from them out of the blue
			// look for any events from this user, add the user and deliver the event(s)
			QValueListIterator< ConferenceEvent > it;
			bool handledDetails = false;
			for ( it = m_pendingEvents.begin(); it != m_pendingEvents.end(); ++it )
			{
				ConferenceEvent event = *it;
				if ( !( event.user.isNull() || event.user != details.dn ) )
				{
					// we may have multiple events for this contact, but only create one temporary contact
					if ( !detailsOwner )
					{
						KopeteMetaContact *metaContact = new KopeteMetaContact ();
		
						metaContact->setTemporary (true);
		
						detailsOwner = new GroupWiseContact( this, details.dn, metaContact, details.fullName, 0, 0, 0 );
						KopeteContactList::contactList ()->addMetaContact (metaContact);
						// the contact details probably don't contain status - but we can ask for it
						if ( details.status == GroupWise::Invalid )
							m_client->requestStatus( details.dn );
					}
					// now the message can be received
					receiveMessage( event );
					handledDetails = true;
				}
				else
					kdDebug( GROUPWISE_DEBUG_GLOBAL ) << " - user DN: " << event.user << " received DN: " << details.dn << endl;
			}
			if ( !handledDetails )
				kdDebug( GROUPWISE_DEBUG_GLOBAL ) << " - received details, but there was no pending event matching them" << endl;
		}
	}
}

void GroupWiseAccount::receiveStatus( const QString & contactId, Q_UINT16 status, const QString &awayMessage )
{
	kdDebug( GROUPWISE_DEBUG_GLOBAL ) << k_funcinfo << "got status for: " << contactId << ", status: " << status << ", away message: " << awayMessage << endl;
	GroupWiseContact * c = static_cast<GroupWiseContact *>( contacts()[ contactId ] );
	if ( c )
	{
		kdDebug( GROUPWISE_DEBUG_GLOBAL ) << " - their KOS is: " << protocol()->gwStatusToKOS( status ).description() << endl;
		c->setOnlineStatus( protocol()->gwStatusToKOS( status ) );
		c->setProperty( protocol()->propAwayMessage, awayMessage );
	}
	else
		kdDebug( GROUPWISE_DEBUG_GLOBAL ) << " couldn't find " << contactId << endl;
}

void GroupWiseAccount::changeOurStatus( GroupWise::Status status, const QString & awayMessage, const QString & autoReply )
{
	myself()->setOnlineStatus( protocol()->gwStatusToKOS( status ) );
	myself()->setProperty( protocol()->propAwayMessage, awayMessage );
	myself()->setProperty( protocol()->propAutoReply, autoReply );
}

void GroupWiseAccount::sendMessage( const QString &guid, const KopeteMessage & message )
{
	// make an outgoing message
	GroupWise::OutgoingMessage outMsg;
	outMsg.guid = guid;
	outMsg.message = message.plainBody();
	// make a list of DNs to send to
	QStringList addresseeDNs;
	KopeteContactPtrList addressees = message.to();
	for ( KopeteContact * contact = addressees.first(); contact; contact = addressees.next() )
		addresseeDNs.append( contact->contactId() );
	// send the message 
	m_client->sendMessage( addresseeDNs, outMsg );
}
#include "gwaccount.moc"

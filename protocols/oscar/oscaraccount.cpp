/*
    oscaraccount.cpp  -  Oscar Account Class

    Copyright (c) 2002 by Tom Linsky <twl6@po.cwru.edu>
    Copyright (c) 2002 by Chris TenHarmsel <tenharmsel@staticmethod.net>
    Copyright (c) 2004 by Matt Rogers <mattr@kde.org>
    Kopete    (c) 2002-2004 by the Kopete developers  <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/

#include "oscaraccount.h"

#include "kopetepassword.h"
#include "kopeteprotocol.h"
#include "kopeteaway.h"
#include "kopetemetacontact.h"
#include "kopetecontactlist.h"
#include "kopeteawaydialog.h"
#include "kopetegroup.h"
#include "kopeteuiglobal.h"
#include "kopetecontactlist.h"
#include "kopetecontact.h"
#include "kopetemessagemanager.h"

#include <assert.h>

#include <qapplication.h>
#include <qregexp.h>
#include <qstylesheet.h>
#include <qtimer.h>
#include <qptrlist.h>

#include <kdebug.h>
#include <kconfig.h>
#include <klocale.h>
#include <kmessagebox.h>

#include "client.h"
#include "connection.h"
#include "oscartypeclasses.h"
#include "oscarutils.h"
#include "oscarclientstream.h"
#include "oscarconnector.h"
#include "ssimanager.h"

class OscarAccountPrivate
{
public:

	//The liboscar hook for the account
	Client* engine;
	
	Q_UINT32 ssiLastModTime;

	//contacts waiting on SSI add ack and their metacontact
	QMap<QString, Kopete::MetaContact*> addContactMap;
	
	//contacts waiting on their group to be added
	QMap<QString, QString> contactAddQueue;
	

};

OscarAccount::OscarAccount(Kopete::Protocol *parent, const QString &accountID, const char *name, bool isICQ)
: Kopete::PasswordedAccount( parent, accountID, isICQ ? 8 : 16, name )
{
	kdDebug(OSCAR_GEN_DEBUG) << k_funcinfo << " accountID='" << accountID <<
		"', isICQ=" << isICQ << endl;

	d = new OscarAccountPrivate;
	d->engine = new Client( this );

	QObject::connect( d->engine, SIGNAL( loggedIn() ), this, SLOT( slotGotSSIList() ) );
	QObject::connect( d->engine, SIGNAL( messageReceived( const Oscar::Message& ) ),
						this, SLOT( messageReceived(const Oscar::Message& ) ) );
	QObject::connect( d->engine, SIGNAL( error( int, int, const QString&  ) ),
	                  this, SLOT( protocolError( int, int, const QString& ) ) );
	QObject::connect( d->engine, SIGNAL( userStartedTyping( const QString& ) ),
	                  this, SLOT( userStartedTyping( const QString& ) ) );
	QObject::connect( d->engine, SIGNAL( userStoppedTyping( const QString& ) ),
	                  this, SLOT( userStoppedTyping( const QString& ) ) );
}

OscarAccount::~OscarAccount()
{
	OscarAccount::disconnect();
	delete d;
}

Client* OscarAccount::engine()
{
	return d->engine;
}

void OscarAccount::disconnect()
{
	kdDebug(OSCAR_GEN_DEBUG) << k_funcinfo << "accountId='" << accountId() << "'" << endl;
		//disconnect the signals
	Kopete::ContactList* kcl = Kopete::ContactList::self();
	QObject::disconnect( kcl, SIGNAL( groupRenamed( Kopete::Group*,  const QString& ) ), 
	                     this, SLOT( kopeteGroupRenamed( Kopete::Group*, const QString& ) ) );
	QObject::disconnect( kcl, SIGNAL( groupRemoved( Kopete::Group* ) ),
	                     this, SLOT( kopeteGroupRemoved( Kopete::Group* ) ) );
	QObject::disconnect( d->engine->ssiManager(), SIGNAL( contactAdded( const Oscar::SSI& ) ),
	                     this, SLOT( ssiContactAdded( const Oscar::SSI& ) ) );
	QObject::disconnect( d->engine->ssiManager(), SIGNAL( groupAdded( const Oscar::SSI& ) ),
	                     this, SLOT( ssiGroupAdded( const Oscar::SSI& ) ) );
	
	d->engine->close();
	myself()->setOnlineStatus( Kopete::OnlineStatus::Offline );
	disconnected( Manual );
}

bool OscarAccount::passwordWasWrong()
{
	return password().isWrong();
}

void OscarAccount::updateContact( Oscar::SSI item )
{
	Kopete::Contact* contact = contacts()[item.name()];
	if ( !contact )
		return;
	else
	{
		kdDebug(OSCAR_RAW_DEBUG) << k_funcinfo << "Updating SSI Item" << endl;
		OscarContact* oc = static_cast<OscarContact*>( contact );
		oc->setSSIItem( item );
	}
}

void OscarAccount::slotGotSSIList()
{
	//login was successful
	password().setWrong( false );
	
	//disconnect signals so we don't attempt to add things to SSI!
	Kopete::ContactList* kcl = Kopete::ContactList::self();
	QObject::disconnect( kcl, SIGNAL( groupRenamed( Kopete::Group*,  const QString& ) ),
	                     this, SLOT( kopeteGroupRenamed( Kopete::Group*, const QString& ) ) );
	QObject::disconnect( kcl, SIGNAL( groupRemoved( Kopete::Group* ) ),
	                     this, SLOT( kopeteGroupRemoved( Kopete::Group* ) ) );
	
	kdDebug(OSCAR_RAW_DEBUG) << k_funcinfo << endl;
	
	SSIManager* listManager = d->engine->ssiManager();
		//first add groups
	QValueList<SSI> groupList = listManager->groupList();
	QValueList<SSI>::const_iterator git = groupList.constBegin();
	QValueList<SSI>::const_iterator listEnd = groupList.constEnd();
		//the protocol dictates that there is at least one group that has contacts
		//so i don't have to check for an empty group list

	kdDebug(OSCAR_GEN_DEBUG) << k_funcinfo << "Adding " << groupList.count() << " groups to contact list" << endl;
	for( ; git != listEnd; ++git )
	{ //add all the groups.
		kdDebug( OSCAR_GEN_DEBUG ) << k_funcinfo << "Adding SSI group'" << ( *git ).name()
			<< "' to the kopete contact list" << endl;
		kcl->findGroup( ( *git ).name() );
	}
	
		//then add contacts
	QValueList<SSI> contactList = listManager->contactList();
	QValueList<SSI>::const_iterator bit = contactList.constBegin();
	QValueList<SSI>::const_iterator blistEnd = contactList.constEnd();
	kdDebug(OSCAR_GEN_DEBUG) << k_funcinfo << "Adding " << contactList.count() << " contacts to contact list" << endl;
	for ( ; bit != blistEnd; ++bit )
	{
		SSI groupForAdd = listManager->findGroup( ( *bit ).gid() );
		Kopete::Group* group;
		if ( groupForAdd.isValid() )
			group = kcl->findGroup( groupForAdd.name() ); //add if not present
		else
			group = kcl->findGroup( i18n( "Buddies" ) );
		
		kdDebug( OSCAR_GEN_DEBUG ) << k_funcinfo << "Adding contact '" << ( *bit ).name() << "' to kopete list in group " <<
			group->displayName() << endl;
		OscarContact* oc = dynamic_cast<OscarContact*>( contacts()[( *bit ).name()] );
		if ( oc )
		{
			Oscar::SSI item = ( *bit );
			oc->setSSIItem( item );
		}
		else
			addContact( ( *bit ).name(), QString::null, group, Kopete::Account::DontChangeKABC );
	}
	QObject::connect( kcl, SIGNAL( groupRenamed( Kopete::Group*,  const QString& ) ),
	                  this, SLOT( kopeteGroupRenamed( Kopete::Group*, const QString& ) ) );
	QObject::connect( kcl, SIGNAL( groupRemoved( Kopete::Group* ) ),
	                  this, SLOT( kopeteGroupRemoved( Kopete::Group* ) ) );
	QObject::connect( listManager, SIGNAL( contactAdded( const Oscar::SSI& ) ),
	                  this, SLOT( ssiContactAdded( const Oscar::SSI& ) ) );
	QObject::connect( listManager, SIGNAL( groupAdded( const Oscar::SSI& ) ),
	                  this, SLOT( ssiGroupAdded( const Oscar::SSI& ) ) );
}


void OscarAccount::slotGoOffline()
{
	OscarAccount::disconnect();
	//setAllContactsStatus( Kopete::OnlineStatus::AccountOffline );
}

void OscarAccount::slotGoOnline()
{
	//do nothing
}

void OscarAccount:: protocolError( int error, int psError, const QString& message )
{
	if ( error == Client::NoError )
		return;
	
	if ( error == Client::NotConnectedError )
	{
		KMessageBox::queuedMessageBox( 0, KMessageBox::Error, message, i18n( "%1 Not Connected to %2" )
		                               .arg( d->engine->userId(), d->engine->isIcq() ? i18n( "ICQ" ) : i18n( "AIM" ) ) );
	}
	
	if ( error == Client::FatalProtocolError )
	{
		kdDebug(OSCAR_GEN_DEBUG) << k_funcinfo << "Received fatal protocol error" << error << ", " 
			<< psError << endl;
		disconnect();
		if ( psError == 5 )
		{
			disconnected( Kopete::Account::BadPassword );
			password().setWrong( true );
			return;
		}
		if ( psError == 0 ) //zero is a generic error when i don't know what's wrong. :/
		{
			disconnected( Kopete::Account::ConnectionReset );
		}
		
		KMessageBox::queuedMessageBox( Kopete::UI::Global::mainWidget(), KMessageBox::Error,
		                               message, i18n( "%1 Disconnected" ).arg( d->engine->userId() ) );
		return;
	}
	
	if ( error == Client::NonFatalProtocolError )
	{
		KMessageBox::queuedMessageBox( Kopete::UI::Global::mainWidget(), KMessageBox::Error,
		                               message, i18n( "account id", "%1" ).arg( d->engine->userId() ) );
	}
}


void OscarAccount::kopeteGroupRemoved( Kopete::Group* group )
{
	if ( isConnected() )
		d->engine->removeGroup( group->displayName() );
}

void OscarAccount::kopeteGroupAdded( Kopete::Group* group )
{
	if ( isConnected() )
		d->engine->addGroup( group->displayName() );
}

void OscarAccount::kopeteGroupRenamed( Kopete::Group* group, const QString& oldName )
{
	if ( isConnected() )
		d->engine->renameGroup( oldName, group->displayName() );
}

void OscarAccount::messageReceived( const Oscar::Message& message )
{
	//the message isn't for us somehow
	if ( Oscar::normalize( message.receiver() ) != Oscar::normalize( accountId() ) )
	{
		kdDebug(OSCAR_RAW_DEBUG) << k_funcinfo << "got a message but we're not the receiver: "
			<< message.text() << endl;
		return;
	}

	/* Logic behind this:
	 * If we don't have the contact yet, create it as a temporary
	 * Create the message manager
	 * Get the sanitized message back
	 * Append to the chat window
	 */
	QString sender = Oscar::normalize( message.sender() );
	if ( !contacts()[ sender ] )
	{
		kdDebug(OSCAR_RAW_DEBUG) << "Adding '" << message.sender() << "' as temporary contact" << endl;
		addContact( message.sender(), QString::null, 0,  Kopete::Account::Temporary );
	}
	
	OscarContact* ocSender = static_cast<OscarContact *> ( contacts()[sender] ); //should exist now
	
	if ( !ocSender )
	{
		kdWarning(OSCAR_RAW_DEBUG) << "Temporary contact creation failed for '" 
			<< message.sender() << "'! Discarding message: " << message.text() << endl;
		return;
	}
	
	Kopete::ChatSession* chatSession = ocSender->manager( Kopete::Contact::CanCreate );
	chatSession->receivedTypingMsg( ocSender, false ); //person is done typing
	
	QString sanitizedMsg = sanitizedMessage( message );
	
	Kopete::ContactPtrList me;
	me.append( myself() );
	Kopete::Message chatMessage( message.timestamp(), ocSender, me, sanitizedMsg,
	                             Kopete::Message::Inbound, Kopete::Message::RichText );
	
	chatSession->appendMessage( chatMessage );
}


void OscarAccount::setServerAddress(const QString &server)
{
	configGroup()->writeEntry( QString::fromLatin1( "Server" ), server );
}

void OscarAccount::setServerPort(int port)
{
	if ( port > 0 )
		configGroup()->writeEntry( QString::fromLatin1( "Port" ), port );
	else //set to default 5190
		configGroup()->writeEntry( QString::fromLatin1( "Port" ), 5190 );
}

void OscarAccount::slotPasswordWrong()
{
	OscarAccount::disconnect();
	password().setWrong();
	QTimer::singleShot(0, this, SLOT(connect()));
}

Connection* OscarAccount::setupConnection( const QString& server, uint port )
{
	//set up the connector
	KNetworkConnector* knc = new KNetworkConnector( 0 );
	knc->setOptHostPort( server, port );
	
	//set up the clientstream
	ClientStream* cs = new ClientStream( knc, knc );
	
	Connection* c = new Connection( knc, cs, "AUTHORIZER" );
	c->setClient( d->engine );
	
	return c;
}


bool OscarAccount::createContact(const QString &contactId,
	Kopete::MetaContact *parentContact)
{
	/* We're not even online or connecting
	 * (when getting server contacts), so don't bother
	 */
	if ( !myself()->isOnline() )
	{
		kdDebug(OSCAR_GEN_DEBUG) << k_funcinfo << "Can't add contact, we are offline!" << endl;
		return false;
	}
	
	/* Logic for SSI additions
	If the contact is temporary, no SSI addition at all. Just create the contact and be done with it
	If the contact is not temporary, we need to do the following:
		1. Check if contact already exists in the SSI manager, if so, just create the contact
		2. If contact doesn't exist:
		2.a. create group on SSI if needed
		2.b. create contact on SSI
		2.c. create kopete contact
	 */

	QValueList<TLV> dummyList;
	if ( parentContact->isTemporary() )
	{
		SSI tempItem( contactId, 0, 0, 0xFFFF, dummyList, 0 );
		return createNewContact( contactId, parentContact, tempItem );
	}
	
	SSI ssiItem = d->engine->ssiManager()->findContact( contactId );
	if ( ssiItem )
	{
		kdDebug(OSCAR_GEN_DEBUG) << k_funcinfo << "Have new SSI entry. Finding contact" << endl;	
		if ( contacts()[ssiItem.name()] )
		{
			kdDebug(OSCAR_GEN_DEBUG) << k_funcinfo << "Found contact in list. Updating SSI item" << endl;
			OscarContact* oc = static_cast<OscarContact*>( contacts()[ssiItem.name()] );
			oc->setSSIItem( ssiItem );
			return true;
		}
		else
		{
			kdDebug(OSCAR_GEN_DEBUG) << k_funcinfo << "Didn't find contact in list, creating new contact" << endl;
			return createNewContact( contactId, parentContact, ssiItem );
		}
	}
	else
	{ //new contact, check temporary, if temporary, don't add to SSI. otherwise, add.
		kdDebug(OSCAR_GEN_DEBUG) << k_funcinfo << "New contact '" << contactId << "' not in SSI."
			<< " Creating new contact" << endl;
		
		kdDebug(OSCAR_GEN_DEBUG) << k_funcinfo << "Adding " << contactId << " to server side list" << endl;
		
		QString groupName;
		Kopete::GroupList kopeteGroups = parentContact->groups(); //get the group list
		
		if ( kopeteGroups.isEmpty() || kopeteGroups.first() == Kopete::Group::topLevel() )
		{
			kdDebug(OSCAR_GEN_DEBUG) << k_funcinfo << "Contact with NO group. " << "Adding to group 'Buddies'" << endl;
			groupName = i18n("Buddies");
		}
		else
		{
				//apparently kopeteGroups.first() can be invalid. Attempt to prevent
				//crashes in SSIData::findGroup(const QString& name)
			groupName = kopeteGroups.first() ? kopeteGroups.first()->displayName() : i18n("Buddies");
			
			kdDebug(OSCAR_GEN_DEBUG) << k_funcinfo << "Contact with group." << " No. of groups = " << kopeteGroups.count() <<
				" Name of first group = " << groupName << endl;
		}
		
		if( groupName.isEmpty() )
		{ // emergency exit, should never occur
			kdWarning(OSCAR_GEN_DEBUG) << k_funcinfo << "Could not add contact because no groupname was given" << endl;
			return false;
		}
		
		if ( !d->engine->ssiManager()->findGroup( groupName ) )
		{ //group isn't on SSI
			d->contactAddQueue[Oscar::normalize( contactId )] = groupName;
			d->addContactMap[Oscar::normalize( contactId )] = parentContact;
			d->engine->addGroup( groupName );
			return true;
		}
		
		d->addContactMap[Oscar::normalize( contactId )] = parentContact;
		d->engine->addContact( Oscar::normalize( contactId ), groupName );
		return true;
	}
}

void OscarAccount::ssiContactAdded( const Oscar::SSI& item )
{
	if ( d->addContactMap.contains( Oscar::normalize( item.name() ) ) )
	{
		kdDebug(OSCAR_GEN_DEBUG) << k_funcinfo << "Received confirmation from server. adding " << item.name()
			<< " to the contact list" << endl;
		createNewContact( item.name(), d->addContactMap[Oscar::normalize( item.name() )], item );
	}
	else
		kdDebug(OSCAR_GEN_DEBUG) << k_funcinfo << "Got addition for contact we weren't waiting on" << endl;
}

void OscarAccount::ssiGroupAdded( const Oscar::SSI& item )
{
	//check the contact add queue for any contacts matching the
	//group name we just added
	kdDebug(OSCAR_GEN_DEBUG) << k_funcinfo << "Looking for contacts to be added in group " << item.name() << endl;
	QMap<QString,QString>::iterator it;
	for ( it = d->contactAddQueue.begin(); it != d->contactAddQueue.end(); ++it )
	{
		if ( Oscar::normalize( it.data() ) == Oscar::normalize( item.name() ) )
		{
			kdDebug(OSCAR_GEN_DEBUG) << k_funcinfo << "starting delayed add of contact '" << it.key() << "' to group "
				<< item.name() << endl;
			d->engine->addContact( Oscar::normalize( it.key() ), item.name() ); //already in the map
		}
	}
}
	
void OscarAccount::userStartedTyping( const QString & contact )
{
	Kopete::Contact * ct = contacts()[ Oscar::normalize( contact ) ];
	if ( ct && contact != accountId() )
	{
		OscarContact * oc = static_cast<OscarContact *>( ct );
		oc->startedTyping();
	}
}

void OscarAccount::userStoppedTyping( const QString & contact )
{
	Kopete::Contact * ct = contacts()[ Oscar::normalize( contact ) ];
	if ( ct && contact != accountId() )
	{
		OscarContact * oc = static_cast<OscarContact *>( ct );
		oc->stoppedTyping();
	}
}


#include "oscaraccount.moc"
//kate: tab-width 4; indent-mode csands;

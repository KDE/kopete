/*
    gwcontact.cpp - Kopete GroupWise Protocol

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

#include <kaction.h>
#include <kdebug.h>
#include <klocale.h>

#include "kopeteaccount.h"
#include "kopetemessagemanagerfactory.h"
#include "kopetemetacontact.h"

#include "gwaccount.h"
#include "gwerror.h"
#include "gwfakeserver.h"
#include "gwprotocol.h"

#include "gwcontact.h"

using namespace GroupWise;

GroupWiseContact::GroupWiseContact( KopeteAccount* account, const QString &dn, 
			KopeteMetaContact *parent, 
			const QString &displayName, const int objectId, const int parentId, const int sequence )
: KopeteContact( account, dn, parent ), m_objectId( objectId ), m_parentId( parentId ),
  m_sequence( sequence )
{
	kdDebug( GROUPWISE_DEBUG_GLOBAL ) << k_funcinfo << " dn: " << dn << endl;
	rename( displayName );
}

GroupWiseContact::~GroupWiseContact()
{
}

void GroupWiseContact::updateDetails( const ContactDetails & details )
{
	kdDebug( GROUPWISE_DEBUG_GLOBAL ) << k_funcinfo << endl;
	if ( !details.cn.isNull() )
		setProperty( protocol()->propCN, details.cn );
	if ( !details.givenName.isNull() )
		setProperty( protocol()->propGivenName, details.givenName );
	if ( !details.surname.isNull() )
		setProperty( protocol()->propLastName, details.surname );
	if ( !details.fullName.isNull() )
		setProperty( protocol()->propFullName, details.fullName );
	//if ( !details.awayMessage.isNull() )
		//setProperty( protocol()->propAwayMessage, details.awayMessage );
	if ( details.status != GroupWise::Invalid )
	{	
		KopeteOnlineStatus status = protocol()->gwStatusToKOS( details.status );
		kdDebug( GROUPWISE_DEBUG_GLOBAL ) << "setting initial status to " << status.description() << endl;
		setOnlineStatus( status );
	}
	else 
		kdDebug( GROUPWISE_DEBUG_GLOBAL ) << "initial status is, not setting " << details.status << endl; 
}

GroupWiseProtocol *GroupWiseContact::protocol()
{
	return static_cast<GroupWiseProtocol *>( KopeteContact::protocol() );
}

bool GroupWiseContact::isReachable()
{
	//TODO: use status to determine reachability
    return true;
}

void GroupWiseContact::serialize( QMap< QString, QString > &serializedData, QMap< QString, QString > & /* addressBookData */ )
{
/*    QString value;
	switch ( m_type )
	{
	case Null:
		value = "null";
	case Echo:
		value = "echo";
	}
	serializedData[ "contactType" ] = value;*/
}

KopeteMessageManager* GroupWiseContact::manager( bool )
{
	kdDebug( GROUPWISE_DEBUG_GLOBAL ) << k_funcinfo << endl;
	if ( m_msgManager )
	{
		return m_msgManager;
	}
	else
	{
		QPtrList<KopeteContact> contacts;
		contacts.append(this);
		m_msgManager = KopeteMessageManagerFactory::factory()->create(account()->myself(), contacts, protocol());
		connect(m_msgManager, SIGNAL(messageSent(KopeteMessage&, KopeteMessageManager*)),
				this, SLOT( sendMessage( KopeteMessage& ) ) );
		connect(m_msgManager, SIGNAL(destroyed()), this, SLOT(slotMessageManagerDestroyed()));
		return m_msgManager;
	}
}


QPtrList<KAction> *GroupWiseContact::customContextMenuActions() //OBSOLETE
{
	//FIXME!!!  this function is obsolete, we should use XMLGUI instead
	/*m_actionCollection = new KActionCollection( this, "userColl" );
	m_actionPrefs = new KAction(i18n( "&Contact Settings" ), 0, this,
			SLOT( showContactSettings( )), m_actionCollection, "contactSettings" );

	return m_actionCollection;*/
	return 0L;
}

void GroupWiseContact::showContactSettings()
{
	//GroupWiseContactSettings* p = new GroupWiseContactSettings( this );
	//p->show();
}

void GroupWiseContact::sendMessage( KopeteMessage &message )
{
	kdDebug( GROUPWISE_DEBUG_GLOBAL ) << k_funcinfo << endl;
	// convert to the what the server wants
	// For this 'protocol', there's nothing to do
	// send it
/*	static_cast<GroupWiseAccount *>( account() )->server()->sendMessage(
			message.to().first()->contactId(),
			message.plainBody() );*/
	// give it back to the manager to display
	manager()->appendMessage( message );
	// tell the manager it was sent successfully
	manager()->messageSucceeded();
}

void GroupWiseContact::receivedMessage( const QString &message )
{
	// Create a KopeteMessage
	KopeteMessage *newMessage;
	KopeteContactPtrList contactList;
	account();
	contactList.append( account()->myself() );
	newMessage = new KopeteMessage( this, contactList, message, KopeteMessage::Inbound );

	// Add it to the manager
	manager()->appendMessage (*newMessage);

	delete newMessage;
}

void GroupWiseContact::slotMessageManagerDestroyed()
{
	//FIXME: the chat window was closed?  Take appropriate steps.
	m_msgManager = 0L;
}

void GroupWiseContact::handleIncomingMessage( const ConferenceEvent & event )
{
	//TODO: implement
}
#include "gwcontact.moc"

// vim: set noet ts=4 sts=4 sw=4:


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
#include "gwfakeserver.h"
#include "gwprotocol.h"

#include "gwcontact.h"

GroupWiseContact* GroupWiseContact::contactFromFields( KopeteAccount* account, KopeteMetaContact *parent, const Field::MultiField & contact )
{
	if ( contact.tag() != NM_A_FA_CONTACT )
		return 0;

	Field::FieldList fields = contact.fields();
	Field::FieldBase* current = 0;
	int objectId, parentId, sequence;
	QString displayName, dn;
	// sequence number, object and parent IDs are a numeric values but are stored as strings...
	Field::FieldListIterator it;
	Field::FieldListIterator end = fields.end();
	if ( ( it = fields.find ( NM_A_SZ_OBJECT_ID ) ) != end )
		objectId = static_cast<Field::SingleField*>( *it )->value().toString().toInt();
	if ( ( it = fields.find ( NM_A_SZ_PARENT_ID ) ) != end )
		parentId = static_cast<Field::SingleField*>( *it )->value().toString().toInt();
	if ( ( it = fields.find ( NM_A_SZ_SEQUENCE_NUMBER ) ) != end )
		sequence = static_cast<Field::SingleField*>( *it )->value().toString().toInt();
	if ( ( it = fields.find ( NM_A_SZ_DISPLAY_NAME ) ) != end )
		displayName = static_cast<Field::SingleField*>( *it )->value().toString();
	if ( ( it = fields.find ( NM_A_SZ_DN ) ) != end )
		dn = static_cast<Field::SingleField*>( *it )->value().toString();

	return new GroupWiseContact( account, dn, parent, displayName, objectId, parentId, sequence );
}

GroupWiseContact::GroupWiseContact( KopeteAccount* account, const QString &dn, 
			KopeteMetaContact *parent, 
			const QString &displayName, const int objectId, const int parentId, const int sequence )
: KopeteContact( account, dn, parent ), m_objectId( objectId ), m_parentId( parentId ),
  m_sequence( sequence )
{
	kdDebug( 14220 ) << k_funcinfo << " dn: " << dn << endl;
	rename( displayName );
}

GroupWiseContact::~GroupWiseContact()
{
}

void GroupWiseContact::updateDetailsFromFields( const Field::MultiField & details )
{
	// read the supplied fields, set metadata and status.
	if ( details.tag() != NM_A_FA_USER_DETAILS )
		return;
	Field::FieldList fields = details.fields();
	Field::FieldBase* current = 0;
	QString cn, dn, givenName, surname, fullName, awayMessage, authAttribute;
	int status;
	Field::FieldListIterator it;
	Field::FieldListIterator end = fields.end();
	// TODO: not sure what this means, ask Mike
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
	
	setProperty( protocol()->propCN, cn );
	setProperty( protocol()->propGivenName, givenName );
	setProperty( protocol()->propLastName, surname );
	setProperty( protocol()->propFullName, fullName );
	setProperty( protocol()->propAwayMessage, awayMessage );
}

GroupWiseProtocol *GroupWiseContact::protocol()
{
	return static_cast<GroupWiseProtocol *>( KopeteContact::protocol() );
}

bool GroupWiseContact::isReachable()
{
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
	kdDebug( 14220 ) << k_funcinfo << endl;
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
	kdDebug( 14220 ) << k_funcinfo << endl;
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

#include "gwcontact.moc"

// vim: set noet ts=4 sts=4 sw=4:


/*
  oscarcontact.cpp  -  Oscar Protocol Plugin

  Copyright (c) 2002 by Tom Linsky <twl6@po.cwru.edu>
  Kopete    (c) 2002-2003 by the Kopete developers  <kopete-devel@kde.org>

  *************************************************************************
  *                                                                       *
  * This program is free software; you can redistribute it and/or modify  *
  * it under the terms of the GNU General Public License as published by  *
  * the Free Software Foundation; either version 2 of the License, or     *
  * (at your option) any later version.                                   *
  *                                                                       *
  *************************************************************************
*/

#include "oscarcontact.h"

#include <time.h>

#include <qapplication.h>

#include <kaction.h>
#include <kdebug.h>
#include <klocale.h>
#include <kmessagebox.h>

#include <kdeversion.h>

#include "kopeteaccount.h"
#include "kopetemessagemanagerfactory.h"
#include "kopetemetacontact.h"
#include "kopetecontactlist.h"
#include "kopetegroup.h"
#include "kopeteuiglobal.h"
#include <kopeteglobal.h>

#include "oscaraccount.h"
#include "client.h"
#include "ssimanager.h"
#include "oscarutils.h"

#include <assert.h>

OscarContact::OscarContact( Kopete::Account* account, const QString& name,
                            Kopete::MetaContact* parent, const QString& icon, const SSI& ssiItem )
: Kopete::Contact( account, name, parent, icon )
{
	mAccount = static_cast<OscarAccount*>(account);
	mName = name;
	mMsgManager = 0L;
	m_ssiItem = ssiItem;
	connect( this, SIGNAL( updatedSSI() ), this, SLOT( updateSSIItem() ) );
}

OscarContact::~OscarContact()
{
}

void OscarContact::serialize(QMap<QString, QString> &serializedData,
                             QMap<QString, QString> &/*addressBookData*/)
{
	serializedData["ssi_name"] = m_ssiItem.name();
	serializedData["ssi_type"] = QString::number( m_ssiItem.type() );
	serializedData["ssi_gid"] = QString::number( m_ssiItem.gid() );
	serializedData["ssi_bid"] = QString::number( m_ssiItem.bid() );
	serializedData["ssi_alias"] = m_ssiItem.alias();
	serializedData["ssi_waitingAuth"] = m_ssiItem.waitingAuth() ? QString::fromLatin1( "true" ) : QString::fromLatin1( "false" );
}

bool OscarContact::isOnServer() const
{
	return ( m_ssiItem.type() != 0xFFFF );
}

void OscarContact::setSSIItem( const Oscar::SSI& ssiItem )
{
	m_ssiItem = ssiItem;
	emit updatedSSI();
}

Oscar::SSI OscarContact::ssiItem() const
{
	return m_ssiItem;
}

Kopete::ChatSession* OscarContact::manager( CanCreateFlags canCreate )
{
	if ( !mMsgManager && canCreate )
	{
		/*kdDebug(14190) << k_funcinfo <<
			"Creating new ChatSession for contact '" << displayName() << "'" << endl;*/

		QPtrList<Kopete::Contact> theContact;
		theContact.append(this);

		mMsgManager = Kopete::ChatSessionManager::self()->create(account()->myself(), theContact, protocol());

		// This is for when the user types a message and presses send
		connect(mMsgManager, SIGNAL( messageSent( Kopete::Message&, Kopete::ChatSession * ) ),
		        this, SLOT( slotSendMsg( Kopete::Message&, Kopete::ChatSession * ) ) );

		// For when the message manager is destroyed
		connect(mMsgManager, SIGNAL( destroyed() ),
		        this, SLOT( chatSessionDestroyed() ) );

/*		connect(mMsgManager, SIGNAL( myselfTyping( bool ) ),
		        this, SLOT( slotTyping( bool ) ) );*/
	}
	return mMsgManager;
}

void OscarContact::deleteContact()
{
	mAccount->engine()->removeContact( contactId() );
	deleteLater();
}

void OscarContact::chatSessionDestroyed()
{
	mMsgManager = 0L;
}

// Called when the metacontact owning this contact has changed groups
void OscarContact::sync(unsigned int flags)
{
	/* 
	 * If the contact is waiting for auth, we do nothing
	 * If the contact has changed groups, then we update the server
	 *   adding the group if it doesn't exist, changing the ssi item
	 *   contained in the client and updating the contact's ssi item
	 * Otherwise, we don't do much
	 */
	if ( flags & Kopete::Contact::MovedBetweenGroup == Kopete::Contact::MovedBetweenGroup )
	{

		if ( m_ssiItem.waitingAuth() )
		{
			kdDebug(OSCAR_GEN_DEBUG) << k_funcinfo << "Contact still requires auth. Doing nothing" << endl;
			return;
		}
		
		kdDebug(OSCAR_GEN_DEBUG) << k_funcinfo << "Moving a contact between groups" << endl;
		SSIManager* ssiManager = mAccount->engine()->ssiManager();
		SSI oldGroup = ssiManager->findGroup( m_ssiItem.gid() );
		Kopete::Group* newGroup = metaContact()->groups().first();
		if ( newGroup->displayName() == oldGroup.name() )
			return; //we didn't really move
		
		if ( !ssiManager->findGroup( newGroup->displayName() ) )
		{ //we don't have the group on the server
			kdDebug(OSCAR_GEN_DEBUG) << k_funcinfo << "the group '" << newGroup->displayName() << "' is not on the server"
				<< "adding it" << endl;
			mAccount->engine()->addGroup( newGroup->displayName() );
		}
		
		SSI newSSIGroup = ssiManager->findGroup( newGroup->displayName() );
		if ( !newSSIGroup )
		{
			kdWarning(OSCAR_GEN_DEBUG) << k_funcinfo << newSSIGroup.name() << " not found on SSI list after addition!" << endl;
			return;
		}
		
		mAccount->engine()->changeContactGroup( contactId(), newGroup->displayName() );
		SSI newItem( m_ssiItem.name(), newSSIGroup.gid(), m_ssiItem.bid(), m_ssiItem.type(),
		             m_ssiItem.tlvList(), m_ssiItem.tlvListLength() );
		setSSIItem( newItem );
	}
	return;
}

void OscarContact::userInfoUpdated( const QString& contact, const UserDetails& details  )
{
	Q_UNUSED( contact );
	setIdleTime( details.idleTime() );
	m_warningLevel = details.warningLevel();
}

void OscarContact::slotSendMsg( Kopete::Message& msg, Kopete::ChatSession* session )
{
	//Why is this unused?
	Q_UNUSED( session );
	Oscar::Message message;

	message.setText( msg.plainBody() );
	
	message.setTimestamp( msg.timestamp() );
	message.setSender( mAccount->accountId() );
	message.setReceiver( mName );
	message.setType( 0x01 );
	
	//TODO add support for type 2 messages
	/*if ( msg.type() == Kopete::Message::PlainText )
		message.setType( 0x01 );
	else
		message.setType( 0x02 );*/
	//TODO: we need to check for channel 0x04 messages too;
	
	mAccount->engine()->sendMessage( message );
	manager(Kopete::Contact::CanCreate)->appendMessage(msg);
	manager(Kopete::Contact::CanCreate)->messageSucceeded();
}

void OscarContact::startedTyping()
{
	if ( mMsgManager )
		mMsgManager->receivedTypingMsg( this, true );
}

void OscarContact::stoppedTyping()
{
	if ( mMsgManager )
		mMsgManager->receivedTypingMsg( this, false );
}

#include "oscarcontact.moc"
//kate: tab-width 4; indent-mode csands;

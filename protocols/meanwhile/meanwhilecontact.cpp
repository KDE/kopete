/*
    meanwhilecontact.cpp - a meanwhile contact

    Copyright (c) 2003-2004 by Sivaram Gottimukkala  <suppandi@gmail.com>

    Kopete    (c) 2002-2004 by the Kopete developers <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/

#include <kaction.h>
#include <kdebug.h>
#include <klocale.h>

#include "kopeteaccount.h"
#include "kopetemessagemanagerfactory.h"
#include "kopetemetacontact.h"

#include "meanwhileprotocol.h"
#include "meanwhileserver.h"
#include "meanwhileaccount.h"
#include "meanwhilecontact.h"
#include "meanwhileplugin.h"

MeanwhileContact::MeanwhileContact( 
						QString _userId , 
						QString _nickname,
						MeanwhileAccount *_account,
						Kopete::MetaContact *_parent )
	: Kopete::Contact( _account, _userId, _parent )
{
	setDisplayName( _nickname );
	m_msgManager = 0L;
	meanwhileId = _userId;

	setOnlineStatus( MeanwhileProtocol::protocol()->meanwhileOffline );
}

MeanwhileContact::~MeanwhileContact()
{
}

bool MeanwhileContact::isReachable()
{
    return true;
}

void MeanwhileContact::serialize( 
			QMap< QString, 
			QString > &serializedData, 
			QMap< QString, 
			QString > & addressBookData )
{
	Kopete::Contact::serialize(serializedData, addressBookData);
}

Kopete::ChatSession* MeanwhileContact::manager( Kopete::Contact::CanCreateFlags canCreate  )
{
	if ( m_msgManager || canCreate != Kopete::Contact::CanCreate )
	{
		return m_msgManager;
	}
	else
	{
		QPtrList<Kopete::Contact> contacts;
		contacts.append(this);

		m_msgManager = 
				Kopete::ChatSessionManager::self()->create(
									account()->myself(), 
									contacts, protocol());

		connect(m_msgManager, 
				SIGNAL(messageSent(Kopete::Message&, Kopete::ChatSession*)),
				this, SLOT( sendMessage( Kopete::Message& ) ) );

		connect(m_msgManager, SIGNAL(destroyed()), 
				this, SLOT(slotChatSessionDestroyed()));

		connect(m_msgManager, SIGNAL(myselfTyping(bool)),
				this, SLOT(slotMeTypingMsg(bool)));

		return m_msgManager;
	}
}


QPtrList<KAction> *MeanwhileContact::customContextMenuActions() 
{
	return 0L;
}

void MeanwhileContact::showContactSettings()
{
}

void MeanwhileContact::slotUserInfo()
{
	MeanwhileAccount *theAccount = static_cast<MeanwhileAccount *>( account());
	theAccount->infoPlugin->showUserInfo(meanwhileId);
}

void MeanwhileContact::sendMessage( Kopete::Message &message )
{
	Kopete::ContactPtrList m_them = manager(Kopete::Contact::CanCreate)->members();
    Kopete::Contact *target = m_them.first();
	(static_cast<MeanwhileAccount *>( account() ))->server->sendIm(
			static_cast<MeanwhileContact*>(target),
			message.plainBody() ); 
	manager(Kopete::Contact::CanCreate)->appendMessage( message );
	manager(Kopete::Contact::CanCreate)->messageSucceeded();
}

void MeanwhileContact::receivedMessage( const QString &message )
{
	Kopete::Message *newMessage;
	Kopete::ContactPtrList contactList;
	account();
	contactList.append( account()->myself() );
	newMessage = new Kopete::Message( this, contactList, 
							message, Kopete::Message::Inbound );

	manager(Kopete::Contact::CanCreate)->appendMessage (*newMessage);

	delete newMessage;
}

void MeanwhileContact::slotChatSessionDestroyed()
{
	m_msgManager = 0L;
}

void MeanwhileContact::slotMeTypingMsg(bool isTyping)
{
	Kopete::ContactPtrList m_them = manager(Kopete::Contact::CanCreate)->members();
    Kopete::Contact *target = m_them.first();
	(static_cast<MeanwhileAccount *>( account() ))->server->sendTyping(
			static_cast<MeanwhileContact*>(target),isTyping);
}

#include "meanwhilecontact.moc"

// vim: set noet ts=4 sts=4 sw=4:


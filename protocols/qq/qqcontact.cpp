/*
    qqcontact.cpp - Kopete QQ Protocol

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

#include "qqcontact.h"

#include <kaction.h>
#include <kdebug.h>
#include <klocale.h>

#include "kopeteaccount.h"
#include "kopetechatsessionmanager.h"
#include "kopetemetacontact.h"

#include "qqaccount.h"
#include "qqfakeserver.h"
#include "qqprotocol.h"
//Added by qt3to4:
#include <QList>

QQContact::QQContact( Kopete::Account* _account, const QString &uniqueName,
		const QQContactType type, const QString &displayName, Kopete::MetaContact *parent )
: Kopete::Contact( _account, uniqueName, parent )
{
	kDebug( 14210 ) << k_funcinfo << " uniqueName: " << uniqueName << ", displayName: " << displayName << endl;
	m_type = type;
	// FIXME: ? setDisplayName( displayName );
	m_msgManager = 0L;

	setOnlineStatus( QQProtocol::protocol()->qqOffline );
}

QQContact::~QQContact()
{
}

bool QQContact::isReachable()
{
    return true;
}

void QQContact::serialize( QMap< QString, QString > &serializedData, QMap< QString, QString > & /* addressBookData */ )
{
    QString value;
	switch ( m_type )
	{
	case Null:
		value = "null";
	case Echo:
		value = "echo";
	}
	serializedData[ "contactType" ] = value;
}

Kopete::ChatSession* QQContact::manager( CanCreateFlags )
{
	kDebug( 14210 ) << k_funcinfo << endl;
	if ( m_msgManager )
	{
		return m_msgManager;
	}
	else
	{
		QList<Kopete::Contact*> contacts;
		contacts.append(this);
		m_msgManager = Kopete::ChatSessionManager::self()->create(account()->myself(), contacts, protocol());
		connect(m_msgManager, SIGNAL(messageSent(Kopete::Message&, Kopete::ChatSession*)),
				this, SLOT( sendMessage( Kopete::Message& ) ) );
		connect(m_msgManager, SIGNAL(destroyed()), this, SLOT(slotChatSessionDestroyed()));
		return m_msgManager;
	}
}


QList<KAction *> *QQContact::customContextMenuActions() //OBSOLETE
{
	//FIXME!!!  this function is obsolete, we should use XMLGUI instead
	/*m_actionCollection = new KActionCollection( this, "userColl" );
	m_actionPrefs = new KAction(i18n( "&Contact Settings" ), 0, this,
			SLOT( showContactSettings( )), m_actionCollection, "contactSettings" );

	return m_actionCollection;*/
	return 0L;
}

void QQContact::showContactSettings()
{
	//QQContactSettings* p = new QQContactSettings( this );
	//p->show();
}

void QQContact::sendMessage( Kopete::Message &message )
{
	kDebug( 14210 ) << k_funcinfo << endl;
	// convert to the what the server wants
	// For this 'protocol', there's nothing to do
	// send it
	//static_cast<QQAccount *>( account() )->server()->sendMessage(
	//		message.to().first()->contactId(),
	//		message.plainBody() );
	// give it back to the manager to display
	manager()->appendMessage( message );
	// tell the manager it was sent successfully
	manager()->messageSucceeded();
}

void QQContact::receivedMessage( const QString &message )
{
	// Create a Kopete::Message
	Kopete::Message *newMessage;
	Kopete::ContactPtrList contactList;
	account();
	contactList.append( account()->myself() );
	newMessage = new Kopete::Message( this, contactList, message, Kopete::Message::Inbound );

	// Add it to the manager
	manager()->appendMessage (*newMessage);

	delete newMessage;
}

void QQContact::slotChatSessionDestroyed()
{
	//FIXME: the chat window was closed?  Take appropriate steps.
	m_msgManager = 0L;
}

#include "qqcontact.moc"

// vim: set noet ts=4 sts=4 sw=4:


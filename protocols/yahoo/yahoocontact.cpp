/*
    yahoocontact.cpp - Yahoo Contact

    Copyright (c) 2003-2004 by Matt Rogers <matt.rogers@kdemail.net>
    Copyright (c) 2002 by Duncan Mac-Vicar Prett <duncan@kde.org>

    Portions based on code by Bruno Rodrigues <bruno.rodrigues@litux.org>

    Copyright (c) 2002 by the Kopete developers  <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/
#include "kopetegroup.h"
#include "kopetemessagemanager.h"
#include "kopeteonlinestatus.h"
#include "kopetemetacontact.h"
#include "kopetemessagemanagerfactory.h"
#include "kopetemetacontact.h"

// Local Includes
#include "yahoocontact.h"
#include "yahooaccount.h"

// QT Includes
#include <qregexp.h>

// KDE Includes
#include <kdebug.h>
#include <kapplication.h>
#include <krun.h>
#include <kmessagebox.h>

YahooContact::YahooContact( YahooAccount *account, const QString &userId, const QString &fullName, Kopete::MetaContact *metaContact )
	: Kopete::Contact( account, userId, metaContact )
{
	kdDebug(14180) << k_funcinfo << endl;

	m_userId = userId;
	if ( metaContact )
		m_groupName = metaContact->groups().getFirst()->displayName();
	m_manager = 0L;
	m_account = account;

	// Update ContactList
	setDisplayName( fullName );
	setOnlineStatus( static_cast<YahooProtocol*>( m_account->protocol() )->Offline );

	if ( m_account->haveContactList() )
		syncToServer();
}

YahooContact::~YahooContact()
{
}

void YahooContact::serialize(QMap<QString, QString> &serializedData, QMap<QString, QString> &addressBookData)
{
	//kdDebug(14180) << k_funcinfo << endl;

	Kopete::Contact::serialize(serializedData, addressBookData);
}

void YahooContact::syncToServer()
{
	kdDebug(14180) << k_funcinfo  << endl;
	if(!m_account->isConnected()) return;

	if ( !m_account->isOnServer(m_userId) && !metaContact()->isTemporary() )
	{	kdDebug(14180) << "Contact " << m_userId << " doesn't exist on server-side. Adding..." << endl;

		Kopete::GroupList groupList = metaContact()->groups();
		for( Kopete::Group *g = groupList.first(); g; g = groupList.next() )
			m_account->yahooSession()->addBuddy(m_userId, g->displayName() );
	}
}

void YahooContact::syncGroups()
{
	if ( !m_account->isConnected() )
		return;

	if ( !m_account->isOnServer( contactId() ) )
	{
		//TODO: Share this code with the above function
		kdDebug(14180) << k_funcinfo << "Contact isn't on the server. Adding..." << endl;
		Kopete::GroupList groupList = metaContact()->groups();
		for ( Kopete::Group *g = groupList.first(); g; g = groupList.next() )
			m_account->yahooSession()->addBuddy(m_userId, g->displayName() );
	}
	else
	{
		QString newGroup = metaContact()->groups().first()->displayName();
		m_account->yahooSession()->changeBuddyGroup( contactId(), m_groupName, newGroup );
		m_groupName = newGroup;
	}
}


bool YahooContact::isOnline() const
{
	//kdDebug(14180) << k_funcinfo << endl;
	return onlineStatus().status() != Kopete::OnlineStatus::Offline && onlineStatus().status() != Kopete::OnlineStatus::Unknown;
}

bool YahooContact::isReachable()
{
	//kdDebug(14180) << k_funcinfo << endl;
	if ( m_account->isConnected() )
		return true;
	else
		return false;
}

Kopete::MessageManager *YahooContact::manager( bool )
{
	if( !m_manager )
	{
		Kopete::ContactPtrList m_them;
		m_them.append( this );
		m_manager = Kopete::MessageManagerFactory::self()->create( m_account->myself(), m_them, protocol() );
		connect( m_manager, SIGNAL( destroyed() ), this, SLOT( slotMessageManagerDestroyed() ) );
		connect( m_manager, SIGNAL( messageSent ( Kopete::Message&, Kopete::MessageManager* ) ), this, SLOT( slotSendMessage( Kopete::Message& ) ) );
		connect( m_manager, SIGNAL( typingMsg( bool) ), this, SLOT( slotTyping( bool ) ) );
		connect( m_account, SIGNAL( receivedTypingMsg( const QString &, bool ) ), m_manager, SLOT( receivedTypingMsg( const QString&, bool ) ) );
	}

	return m_manager;
}

void YahooContact::slotSendMessage( Kopete::Message &message )
{
	kdDebug(14180) << k_funcinfo << endl;

	// Yahoo does not understand XML/HTML message data, so send plain text
	// instead.  (Yahoo has its own format for "rich text".)
	QString messageText = message.plainBody();
	kdDebug(14180) << "Sending message: " << messageText << endl;

	Kopete::ContactPtrList m_them = manager()->members();
	Kopete::Contact *target = m_them.first();

	m_account->yahooSession()->sendIm( static_cast<YahooContact*>(m_account->myself())->m_userId,
		static_cast<YahooContact *>(target)->m_userId, messageText );

	// append message to window
	manager()->appendMessage(message);
	manager()->messageSucceeded();
}

void YahooContact::slotTyping(bool isTyping_ )
{
	Kopete::ContactPtrList m_them = manager()->members();
	Kopete::Contact *target = m_them.first();


	m_account->yahooSession()->sendTyping( static_cast<YahooContact*>(m_account->myself())->m_userId,
		static_cast<YahooContact*>(target)->m_userId, isTyping_ );
}

void YahooContact::slotMessageManagerDestroyed()
{
	m_manager = 0L;
}

QPtrList<KAction> *YahooContact::customContextMenuActions()
{
	//kdDebug(14180) << k_funcinfo << endl;
	return 0L;
}

void YahooContact::slotUserInfo()
{
	//kdDebug(14180) << k_funcinfo << endl;

	QString profileSiteString = QString::fromLatin1("http://profiles.yahoo.com/") + m_userId;
	KRun::runURL( KURL( profileSiteString ) , "text/html" );
}

void YahooContact::slotSendFile()
{
	kdDebug(14180) << k_funcinfo << endl;
}

void YahooContact::slotDeleteContact()
{
	kdDebug(14180) << k_funcinfo << endl;
	//my ugliest hack yet. how many levels of indirection do I want? ;)
	if ( m_account->isConnected() )
		m_account->yahooSession()->removeBuddy(m_userId, m_groupName);

	Kopete::Contact::slotDeleteContact();
}
#include "yahoocontact.moc"

// vim: set noet ts=4 sts=4 sw=4:


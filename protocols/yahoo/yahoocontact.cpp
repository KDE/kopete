/*
    yahoocontact.cpp - Yahoo Contact

    Copyright (c) 2003 by Matt Rogers <mattrogers@sbcglobal.net>
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

YahooContact::YahooContact(KopeteAccount *account, const QString &userId, const QString &fullName, KopeteMetaContact *metaContact)
	: KopeteContact(account, userId, metaContact)
{
	kdDebug(14180) << "YahooContact::YahooContact(" << userId << ", " << fullName << ")" << endl;

	m_userId = userId;
	if ( metaContact )
		m_groupName = metaContact->groups().getFirst()->displayName();
	m_manager = 0L;
	m_status.setStatus(YahooStatus::Offline);
	m_account = static_cast<YahooAccount*>(account);

	// Update ContactList
	setDisplayName(fullName);
	setOnlineStatus(m_status.ys2kos());

	// XXX initActions();

//	QObject::connect (this , SIGNAL( moved(KopeteMetaContact*,KopeteContact*) ), this, SLOT (slotMovedToMetaContact() ));
//	QObject::connect (metaContact , SIGNAL( aboutToSave(KopeteMetaContact*) ), pluginInstance, SLOT (serialize(KopeteMetaContact*) ));
	//TODO: Probably doesn't save contacts now!

	if(m_account->haveContactList())
		syncToServer();
}

YahooContact::~YahooContact()
{
}

void YahooContact::serialize(QMap<QString, QString> &serializedData, QMap<QString, QString> &addressBookData)
{
	kdDebug(14180) << k_funcinfo << endl;

	KopeteContact::serialize(serializedData, addressBookData);
}

void YahooContact::setYahooStatus( YahooStatus::Status status_, const QString &msg, int /*away*/)
{
	kdDebug(14180) << "Yahoo::setYahooStatus( " << status_ << ", " << msg << ")" << endl;
	if (status_ == 13)
		m_status.setStatus(status_, msg);
	else
		m_status.setStatus(status_);
	setOnlineStatus( m_status.ys2kos() );
}

/*
void YahooContact::slotUpdateStatus(QString status, QString statusText)
{
	kdDebug(14180) << "[YahooContact::slotUpdateStatus(" << status << ")]" << endl;
	kdDebug(14180) "Buddy  - updating " << handle << " to " << status << "." << endl;
	if (status != QString("")) {
		mStatus = status;
		kdDebug(14180) << "Yahoo plugin: Updating status." << endl;
	}
	mStatusText = statusText;
	setOnlineStatus( m_status.ys2kos() );
}
*/

void YahooContact::syncToServer()
{
	kdDebug(14180) << k_funcinfo  << endl;
	if(!m_account->isConnected()) return;

	if ( !m_account->isOnServer(m_userId) && !metacontact()->isTemporary() )
	{	kdDebug(14180) << "Contact " << m_userId << " doesn't exist on server-side. Adding..." << endl;

		KopeteGroupList groupList = metaContact()->groups();
		for( KopeteGroup *g = groupList.first(); g; g = groupList.next() )
			m_account->yahooSession()->addBuddy(m_userId, g->displayName() );
	}
}

bool YahooContact::isOnline() const
{
	//kdDebug(14180) << k_funcinfo << endl;
	return onlineStatus().status() != KopeteOnlineStatus::Offline && onlineStatus().status() != KopeteOnlineStatus::Unknown;
}

bool YahooContact::isReachable()
{
	//kdDebug(14180) << k_funcinfo << endl;
	if (m_account->isConnected())
		return true;
	else
		return false;
}

KopeteMessageManager *YahooContact::manager( bool )
{
	if( !m_manager )
	{
		KopeteContactPtrList m_them;
		m_them.append( this );
		m_manager = KopeteMessageManagerFactory::factory()->create( m_account->myself(), m_them, protocol() );
		connect( m_manager, SIGNAL( destroyed() ), this, SLOT( slotMessageManagerDestroyed() ) );
		connect( m_manager, SIGNAL( messageSent(KopeteMessage &, KopeteMessageManager *) ), this, SLOT( slotSendMessage(KopeteMessage &) ) );
		connect( m_manager, SIGNAL( typingMsg(bool) ), this, SLOT(slotTyping(bool) ) );
		connect( m_account, SIGNAL( receivedTypingMsg(const QString &, bool) ), m_manager, SLOT( receivedTypingMsg(const QString &, bool) ) );
	}

	return m_manager;
}

void YahooContact::slotSendMessage(KopeteMessage &message)
{
	kdDebug(14180) << k_funcinfo << endl;

	// Yahoo does not understand XML/HTML message data, so send plain text
	// instead.  (Yahoo has its own format for "rich text".)
	QString messageText = message.plainBody();
	kdDebug(14180) << "Sending message: " << messageText << endl;
  
	KopeteContactPtrList m_them = manager()->members();
	KopeteContact *target = m_them.first();

	m_account->yahooSession()->sendIm( static_cast<YahooContact*>(m_account->myself())->m_userId, 
		static_cast<YahooContact *>(target)->m_userId, messageText );

	// append message to window
	manager()->appendMessage(message);
	manager()->messageSucceeded();
}

void YahooContact::slotTyping(bool isTyping_ )
{
	KopeteContactPtrList m_them = manager()->members();
	KopeteContact *target = m_them.first();
	

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

	QString profileSiteString = "http://profiles.yahoo.com/" + m_userId;
	//kdDebug(14180) << "Yahoo profile site string: " << profileSiteString << endl;
	kapp->invokeBrowser(profileSiteString);
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

	KopeteContact::slotDeleteContact();
}
#include "yahoocontact.moc"

// vim: set noet ts=4 sts=4 sw=4:


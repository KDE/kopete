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

// Local Includes
#include "yahoocontact.h"
#include "yahooaccount.h"

// QT Includes
#include <qregexp.h>

// KDE Includes
#include <kdebug.h>
#include <kapplication.h>

YahooContact::YahooContact(KopeteAccount *account, const QString &userId, const QString &fullName, KopeteMetaContact *metaContact, KopeteContact::AddMode mode )
	: KopeteContact(account, userId, metaContact, mode )
{
	kdDebug(14180) << "YahooContact::YahooContact(" << userId << ", " << fullName << ")" << endl;

	m_userId = userId;
	m_manager = 0L;
	m_status.setStatus(YahooStatus::Offline);

	// Update ContactList
	setDisplayName(fullName);
	setOnlineStatus(m_status.ys2kos());

	// XXX initActions();

//	QObject::connect (this , SIGNAL( moved(KopeteMetaContact*,KopeteContact*) ), this, SLOT (slotMovedToMetaContact() ));
//	QObject::connect (metaContact , SIGNAL( aboutToSave(KopeteMetaContact*) ), pluginInstance, SLOT (serialize(KopeteMetaContact*) ));
	//TODO: Probably doesn't save contacts now!

	if(static_cast<YahooAccount *>(account)->haveContactList())
		syncToServer();
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
	YahooAccount* yAccount = static_cast<YahooAccount*> (account());

	kdDebug(14180) << k_funcinfo<< endl;
	if(!yAccount->isConnected()) return;

	if(!yAccount->isOnServer(m_userId))
	{	kdDebug(14180) << "Contact " << m_userId << " doesn't exist on server-side. Adding..." << endl;

		KopeteGroupList groupList = metaContact()->groups();
		for( KopeteGroup *g = groupList.first(); g; g = groupList.next() )
			yAccount->yahooSession()->addBuddy(m_userId, g->displayName() );
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
	return true;
}

KopeteMessageManager *YahooContact::manager( bool )
{
	if( !m_manager )
	{
		KopeteContactPtrList m_them;
		m_them.append( this );
		m_manager = KopeteMessageManagerFactory::factory()->create( account()->myself(), m_them, protocol() );
		connect( m_manager, SIGNAL( destroyed() ), this, SLOT( slotMessageManagerDestroyed() ) );
		connect( m_manager, SIGNAL( messageSent(KopeteMessage &, KopeteMessageManager *) ), this, SLOT( slotSendMessage(KopeteMessage &) ) );
		connect( m_manager, SIGNAL( typingMsg(bool) ), this, SLOT(slotTyping(bool) ) );
		connect( static_cast<YahooAccount *>(account()), SIGNAL( receivedTypingMsg(const QString &, bool) ), m_manager, SLOT( receivedTypingMsg(const QString &, bool) ) );
	}

	return m_manager;
}

void YahooContact::slotSendMessage(KopeteMessage &message)
{
	kdDebug(14180) << k_funcinfo << endl;

	QString taintedHTML = message.escapedBody();
	taintedHTML = taintedHTML.replace(QString::fromLatin1("<br/>"), QString::fromLatin1("<br>"));

	kdDebug(14180) << "Sending message: " << taintedHTML << endl;

	KopeteContactPtrList m_them = manager()->members();
	KopeteContact *target = m_them.first();
	YahooAccount *i = static_cast<YahooAccount *>(account());

	kdDebug(14180) << "Yahoo: Sending message from " << static_cast<YahooContact *>(i->myself())->m_userId << ", to " << static_cast<YahooContact *>(target)->m_userId << endl;
	i->yahooSession()->sendIm( static_cast<YahooContact *>(i->myself())->m_userId, static_cast<YahooContact *>(target)->m_userId, taintedHTML );

	// append message to window
	manager()->appendMessage(message);
	manager()->messageSucceeded();
}

void YahooContact::slotTyping(bool isTyping_ )
{
	KopeteContactPtrList m_them = manager()->members();
	KopeteContact *target = m_them.first();
	YahooAccount *i = static_cast<YahooAccount *>(account());

	kdDebug(14180) << "Yahoo: Sending typing notification from " << static_cast<YahooContact *>(i->myself())->m_userId << " to " << static_cast<YahooContact *>(target)->m_userId << endl;
	i->yahooSession()->sendTyping( static_cast<YahooContact *>(i->myself())->m_userId, static_cast<YahooContact *>(target)->m_userId, isTyping_ );
}

void YahooContact::slotMessageManagerDestroyed()
{
	m_manager = 0L;
}

KActionCollection *YahooContact::customContextMenuActions()
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

#include "yahoocontact.moc"

// vim: set noet ts=4 sts=4 sw=4:


/*
    yahoocontact.cpp - Yahoo Contact

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
#include "kopeteonlinestatus.h"

// Local Includes
#include "yahoodebug.h"
#include "yahoostatus.h"
#include "yahoocontact.h"
#include "yahooaccount.h"

// QT Includes
#include <qstring.h>

// KDE Includes
#include <kdebug.h>
#include <kmessagebox.h>

YahooContact::YahooContact(KopeteAccount *account, const QString &userId, const QString &fullName, KopeteMetaContact *metaContact)
	: KopeteContact(account, userId, metaContact)
{
	kdDebug(14180) << "YahooContact::YahooContact(" << userId << ", " << fullName << ")" << endl;

	m_userId = userId;
	m_manager = 0L;
	m_status.setStatus(YahooStatus::Offline);

	// Update ContactList
	setDisplayName(fullName);
	setOnlineStatus(m_status.translate());

	// XXX initActions();

	QObject::connect (this , SIGNAL( moved(KopeteMetaContact*,KopeteContact*) ), this, SLOT (slotMovedToMetaContact() ));
//	QObject::connect (metaContact , SIGNAL( aboutToSave(KopeteMetaContact*) ), pluginInstance, SLOT (serialize(KopeteMetaContact*) ));	
	//TODO: Probably doesn't save contacts now!

	if(static_cast<YahooAccount *>(account)->haveContactList())
		syncToServer();
}

void YahooContact::serialize(QMap<QString, QString> &serializedData, QMap<QString, QString> &addressBookData)
{
	kdDebug(14180) << "Yahoo::serialize(...)" << endl;
	
	KopeteContact::serialize(serializedData, addressBookData);
}

void YahooContact::setYahooStatus( YahooStatus::Status status_, const QString &msg, int /*away*/)
{
	kdDebug(14180) << "Yahoo::setYahooStatus( " << status_ << ", " << msg << ")" << endl;
	m_status.setStatus(status_);
	setOnlineStatus( m_status.translate() );
}

/*
void YahooContact::slotUpdateStatus(QString status, QString statusText = QString::null)
{
	kdDebug(14180) << "[YahooContact::slotUpdateStatus(" << status << ")]" << endl;
	kdDebug(14180) "Buddy  - updating " << handle << " to " << status << "." << endl;
	if (status != QString("")) {
		mStatus = status;
		kdDebug(14180) << "Yahoo plugin: Updating status." << endl;
	}
	mStatusText = statusText;
	setOnlineStatus( m_status.translate() );
}
*/

void YahooContact::syncToServer()
{
	kdDebug(14180) << "[YahooContact::syncToServer()]" << endl;
	if(!static_cast<YahooAccount *>(account())->isConnected()) return;
		
	if(!static_cast<YahooAccount *>(account())->isOnServer(m_userId))
	{	kdDebug(14180) << "Contact " << m_userId << " doesn't exist on server-side. Adding..." << endl;
		QStringList theGroups = metaContact()->groups().toStringList();
		if(!theGroups.size()) theGroups += "Exported Kopete contacts";
		for(unsigned j = 0; j < theGroups.size(); j++)
			static_cast<YahooAccount *>(account())->yahooSession()->addBuddy(m_userId, theGroups[j]);
	}
}

bool YahooContact::isOnline() const
{
	kdDebug(14180) << "[YahooContact::isOnline()]" << endl;
	return onlineStatus().status() != KopeteOnlineStatus::Offline && onlineStatus().status() != KopeteOnlineStatus::Unknown;
}

bool YahooContact::isReachable()
{
	kdDebug(14180) << "[YahooContact::isReachable()]" << endl;
	return true;
}

/*QString YahooContact::accountId() const
{
	kdDebug(14180) << "[YahooContact::accountId()]" << endl;
	return m_userId;
}
*/
KopeteMessageManager *YahooContact::manager( bool )
{
	if( !m_manager )
	{
		KopeteContactPtrList m_them;
		m_them.append( this );
		m_manager = KopeteMessageManagerFactory::factory()->create(protocol()->myself(), m_them, protocol() );
		connect( m_manager, SIGNAL( destroyed() ), this, SLOT( slotMessageManagerDestroyed() ) );
		connect( m_manager, SIGNAL( messageSent(KopeteMessage &, KopeteMessageManager *) ), this, SLOT( slotSendMessage(KopeteMessage &) ) );
	}

	return m_manager;
}

void YahooContact::slotSendMessage(KopeteMessage &message)
{
	kdDebug(14180) << "[YahooContact::slotSendMessage(" << message.escapedBody() << ")]" << endl;
	
	KopeteContactPtrList m_them = manager()->members();
	KopeteContact *target = m_them.first();
	YahooAccount *i = static_cast<YahooAccount *>(account());
		
	kdDebug(14180) << "Yahoo: Sending message from " << static_cast<YahooContact *>(i->myself())->m_userId << ", to " << static_cast<YahooContact *>(target)->m_userId << endl;
	i->yahooSession()->sendIm( static_cast<YahooContact *>(i->myself())->m_userId, static_cast<YahooContact *>(target)->m_userId, message.escapedBody() );

	// append message to window
	manager()->appendMessage(message);
	manager()->messageSucceeded();
}

void YahooContact::slotTyping(bool isTyping_ )
{
	KopeteContactPtrList m_them = manager()->members();
	KopeteContact *target = m_them.first();
	YahooAccount *i = static_cast<YahooAccount *>(account());
	
	kdDebug(14180) << "Yahoo: Sending typing notification from " << static_cast<YahooContact *>(i->myself())->m_userId << ", to " << static_cast<YahooContact *>(target)->m_userId << endl;
	i->yahooSession()->sendTyping( static_cast<YahooContact *>(i->myself())->m_userId, static_cast<YahooContact *>(target)->m_userId, isTyping_ );
}

void YahooContact::slotMessageManagerDestroyed()
{
	m_manager = 0L;
}

KActionCollection *YahooContact::customContextMenuActions()
{
	kdDebug(14180) << "[YahooContact::customContextMenuActions()]" << endl;
	return 0L;
}

void YahooContact::slotViewHistory()
{
	kdDebug(14180) << "[YahooContact::slotViewHistory()]" << endl;
}

void YahooContact::slotDeleteContact()
{
	kdDebug(14180) << "[YahooContact::slotDeleteContact()]" << endl;
}

void YahooContact::slotUserInfo()
{
	kdDebug(14180) << "[YahooContact::slotUserInfo()]" << endl;
}

void YahooContact::slotSendFile()
{
	kdDebug(14180) << "[YahooContact::slotSendFile()]" << endl;
}

#include "yahoocontact.moc"

// vim: set noet ts=4 sts=4 sw=4:


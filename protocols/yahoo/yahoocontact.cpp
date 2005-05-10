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
#include "kopetechatsession.h"
#include "kopeteonlinestatus.h"
#include "kopetemetacontact.h"
#include "kopetechatsessionmanager.h"
#include "kopetemetacontact.h"
#include "kopeteuiglobal.h"

// Local Includes
#include "yahoocontact.h"
#include "yahooaccount.h"
#include "yahoowebcamdialog.h"

// QT Includes
#include <qregexp.h>

// KDE Includes
#include <kdebug.h>
#include <kaction.h>
#include <kapplication.h>
#include <klocale.h>
#include <krun.h>
#include <kshortcut.h>
#include <kmessagebox.h>

YahooContact::YahooContact( YahooAccount *account, const QString &userId, const QString &fullName, Kopete::MetaContact *metaContact )
	: Kopete::Contact( account, userId, metaContact )
{
	//kdDebug(14180) << k_funcinfo << endl;

	m_userId = userId;
	if ( metaContact )
		m_groupName = metaContact->groups().getFirst()->displayName();
	m_manager = 0L;
	m_account = account;

	// Update ContactList
	setNickName( fullName );
	setOnlineStatus( static_cast<YahooProtocol*>( m_account->protocol() )->Offline );

	if ( m_account->haveContactList() )
		syncToServer();
	
	m_webcamDialog = 0L;
	m_webcamAction = 0L;
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

void YahooContact::sync(unsigned int flags)
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
		if ( flags & Kopete::Contact::MovedBetweenGroup )
		{
			kdDebug(14180) << k_funcinfo << "contact changed groups. moving on server" << endl;
			m_account->yahooSession()->changeBuddyGroup( contactId(), m_groupName, newGroup );
			m_groupName = newGroup;
		}
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

Kopete::ChatSession *YahooContact::manager( Kopete::Contact::CanCreateFlags canCreate )
{
	if( !m_manager && canCreate)
	{
		Kopete::ContactPtrList m_them;
		m_them.append( this );
		m_manager = Kopete::ChatSessionManager::self()->create( m_account->myself(), m_them, protocol() );
		connect( m_manager, SIGNAL( destroyed() ), this, SLOT( slotChatSessionDestroyed() ) );
		connect( m_manager, SIGNAL( messageSent ( Kopete::Message&, Kopete::ChatSession* ) ), this, SLOT( slotSendMessage( Kopete::Message& ) ) );
		connect( m_manager, SIGNAL( myselfTyping( bool) ), this, SLOT( slotTyping( bool ) ) );
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

	Kopete::ContactPtrList m_them = manager(Kopete::Contact::CanCreate)->members();
	Kopete::Contact *target = m_them.first();

	m_account->yahooSession()->sendIm( static_cast<YahooContact*>(m_account->myself())->m_userId,
		static_cast<YahooContact *>(target)->m_userId, messageText );

	// append message to window
	manager(Kopete::Contact::CanCreate)->appendMessage(message);
	manager(Kopete::Contact::CanCreate)->messageSucceeded();
}

void YahooContact::slotTyping(bool isTyping_ )
{
	Kopete::ContactPtrList m_them = manager(Kopete::Contact::CanCreate)->members();
	Kopete::Contact *target = m_them.first();


	m_account->yahooSession()->sendTyping( static_cast<YahooContact*>(m_account->myself())->m_userId,
		static_cast<YahooContact*>(target)->m_userId, isTyping_ );
}

void YahooContact::slotChatSessionDestroyed()
{
	m_manager = 0L;
}

QPtrList<KAction> *YahooContact::customContextMenuActions()
{
	
	QPtrList<KAction> *actionCollection = new QPtrList<KAction>();
	if ( !m_webcamAction )
	{
		m_webcamAction = new KAction( i18n( "View &Webcam" ), "camera_unmount", KShortcut(),
		                              this, SLOT( requestWebcam() ), this, "view_webcam" );
	}
	
	actionCollection->append( m_webcamAction );
	return actionCollection;
	

	//return 0L;
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

void YahooContact::requestWebcam()
{
	if ( !m_webcamDialog )
	{
		m_webcamDialog = new YahooWebcamDialog( this, Kopete::UI::Global::mainWidget() );
		QObject::connect( m_webcamDialog, SIGNAL( closeClicked() ), this, SLOT( closeWebcamDialog() ) );
	}
	
	QObject::connect( m_account->yahooSession(), SIGNAL( webcamClosed( const QString&, int ) ),
	                  this, SLOT( webcamClosed( const QString&, int ) ) );
	QObject::connect( m_account->yahooSession(), SIGNAL( receivedWebcamImage( const QPixmap& ) ),
	                  this, SIGNAL( receivedWebcamImage( const QPixmap& ) ) );
	m_account->yahooSession()->requestWebcam( contactId() );
}

void YahooContact::closeWebcamDialog()
{
	m_account->yahooSession()->closeWebcam( contactId() );
	m_webcamDialog->delayedDestruct();
}

void YahooContact::webcamClosed( const QString& contact, int reason )
{
	if ( contact != contactId() )
		return;
	
	emit webcamClosed( reason );
}


void YahooContact::deleteContact()
{
	kdDebug(14180) << k_funcinfo << endl;
	//my ugliest hack yet. how many levels of indirection do I want? ;)
	if ( m_account->isConnected() )
		m_account->yahooSession()->removeBuddy(m_userId, m_groupName);

	Kopete::Contact::deleteContact();
}
#include "yahoocontact.moc"

// vim: set noet ts=4 sts=4 sw=4:
//kate: space-indent off; replace-tabs off; indent-mode csands;


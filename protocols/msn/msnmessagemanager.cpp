/***************************************************************************
                          msnmessagemanager.cpp  -  description
                             -------------------
    begin                : dim oct 20 2002
    copyright            : (C) 2002 by Olivier Goffart
    email                : kopete-devel@kde.org
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <kdebug.h>
#include <klocale.h>
#include <kaction.h>
#include <klineeditdlg.h>
 
#include "kopete.h"
#include "kopetemessagemanagerfactory.h"
#include "kopetecontactlist.h"
#include "kopetemetacontact.h"
 
#include "msnmessagemanager.h"
#include "msncontact.h"
#include "msnswitchboardsocket.h"
#include "msnprotocol.h"


MSNMessageManager::MSNMessageManager(const KopeteContact *user, KopeteContactPtrList others,QString logFile, const char *name)
		: KopeteMessageManager(user,others,MSNProtocol::protocol(),0,logFile,ChatWindow,MSNProtocol::protocol(),name )
{
	kopeteapp->sessionFactory()->addKopeteMessageManager(this);
	m_chatService=0l;
	m_msgQueued=0L;
	m_actions=0L;

	connect( this, SIGNAL( messageSent( const KopeteMessage&, KopeteMessageManager* ) ),
			this, SLOT( slotMessageSent( const KopeteMessage& , KopeteMessageManager*) ) );

}
MSNMessageManager::~MSNMessageManager()
{
}

void MSNMessageManager::createChat(QString handle, QString address, QString auth, QString ID)
{
	if(m_chatService)
	{
		kdDebug() << "MSNMessageManager::createChat - Service already exists, disconnect thmem " <<endl;
		m_chatService->slotCloseSession();
	}

	setCanBeDeleted(false);

	m_chatService = new MSNSwitchBoardSocket();
	m_chatService->setHandle( MSNProtocol::protocol()->myself()->id() );
	m_chatService->setMsgHandle( handle );
	m_chatService->connectToSwitchBoard( ID, address, auth );

	connect( m_chatService, SIGNAL( updateChatMember(QString,QString,bool)),
			this, SLOT( slotUpdateChatMember(QString,QString,bool) ) );
	connect( m_chatService, SIGNAL( msgReceived( const KopeteMessage & ) ),
			this, SLOT( appendMessage( const KopeteMessage & ) ) );
	connect( m_chatService, SIGNAL( switchBoardClosed() ),
			this, SLOT( slotSwitchBoardClosed() ) );
	connect( m_chatService, SIGNAL( userTypingMsg( QString ) ),
			this, SLOT( slotUserTypingMsg( QString  ) ) );

	if(m_msgQueued)
	{
		m_chatService->slotSendMsg( *m_msgQueued );
		delete m_msgQueued;
		m_msgQueued=0L;
	}
}

void MSNMessageManager::slotUpdateChatMember(QString handle, QString publicName, bool add)
{
	MSNContact *c=MSNProtocol::protocol()->contact(handle);
	if( add && !c )
	{
		KopeteMetaContact *m = KopeteContactList::contactList()->findContact( MSNProtocol::protocol()->id(), QString::null, handle );
		if(m)
		{
			KopeteContact *kc=m->findContact( MSNProtocol::protocol()->id(), QString::null, handle );
			c=static_cast<MSNContact*>(kc);
			kdDebug() << "MSNMessageManager::slotUpdateChatMember : WARNING - KopeteContact was found, but not on the protocl"  << endl;
			//MSNProtocol::protocol()->contacts().insert( handle, c );
			
		}
		else
		{
			m=new KopeteMetaContact();
			m->setTemporary(true);
			QString protocolid = MSNProtocol::protocol()->id();

			c = new MSNContact( protocolid, handle, publicName, QString::null, m );
			connect( c, SIGNAL( contactDestroyed( KopeteContact * ) ),
				MSNProtocol::protocol(), SLOT( slotContactDestroyed( KopeteContact * ) ) );

			m->addContact( c, QStringList() );
			KopeteContactList::contactList()->addMetaContact(m);

			MSNProtocol::protocol()->contacts().insert( handle, c );
		}
	}

	if(add)
		addContact(c);
	else
		removeContact(c);

}

void MSNMessageManager::slotSwitchBoardClosed()
{
	m_chatService->deleteLater();
	m_chatService=0l;
	setCanBeDeleted(true);
}


void MSNMessageManager::slotUserTypingMsg( QString handle )
{
	MSNContact *c=MSNProtocol::protocol()->contact(handle);
	if(!c)
	{
		kdDebug() << "MSNMessageManager::slotUserTypingMsg : WARNING - KopeteContact not found"  << endl;
		return;
	}
	userTypingMsg(c);
}

void MSNMessageManager::slotTyping(bool t)
{
	if(t)
	{
		if(m_chatService)
			m_chatService->slotTypingMsg();
		else
		{
			MSNProtocol::protocol()->slotStartChatSession( QPtrList<KopeteContact>(members()).first()->id() );
		}
	}
}

void MSNMessageManager::slotMessageSent(const KopeteMessage &message,KopeteMessageManager *)
{
	if(m_chatService)
		m_chatService->slotSendMsg(message);
	else // There's no switchboard available, so we must create a new one!
	{
		MSNProtocol::protocol()->slotStartChatSession( message.to().first()->id() );
		m_msgQueued=new KopeteMessage(message);
	}
}

KActionCollection * MSNMessageManager::chatActions()
{
	delete m_actions;
	
	m_actions= new KActionCollection(this);

	KAction *actionClose = new KAction( i18n ("Leave the chat"), 0,
		this, SLOT( slotCloseSession() ), m_actions, "actionClose" );
	m_actions->insert( actionClose );

	KListAction *actionInvite=new KListAction(i18n("&Invite"),"",0, m_actions ,"actionInvite");
	QStringList sl;
	QMap<QString, MSNContact*>_contacts= MSNProtocol::protocol()->contacts();
	QMap<QString, MSNContact*>::Iterator it;
	for ( it = _contacts.begin(); it != _contacts.end() ; ++it)
	{
		if((*it)->isOnline() && !members().contains(*it))
		{
			sl.append((*it)->id());
		}
	}
	sl.append( otherString=i18n("Other...") );
	actionInvite->setItems( sl );
	connect( actionInvite, SIGNAL( activated(const QString&) ), this, SLOT(slotInviteContact(const QString &)) );
	m_actions->insert(actionInvite);

	return m_actions;

}


void MSNMessageManager::slotCloseSession()
{
	if(m_chatService)
		m_chatService->slotCloseSession();
}

void MSNMessageManager::slotInviteContact(const QString &_handle)
{
	QString handle=_handle;
	if(handle==otherString)
	{
		bool ok;
		handle = KLineEditDlg::getText(i18n( "MSN Plugin" ),
			i18n( "Please enter the email address of the person you want to invite" ),
			QString::null, &ok );
		if( !ok )
			return;
	}
	if(m_chatService)
		m_chatService->slotInviteContact(handle);
	else
		MSNProtocol::protocol()->slotStartChatSession( handle );	
}


#include "msnmessagemanager.moc"

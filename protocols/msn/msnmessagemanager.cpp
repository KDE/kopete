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

#include <qtimer.h>

#include <kdebug.h>
#include <klocale.h>
#include <kaction.h>
#include <klineeditdlg.h>
#include <kmessagebox.h>
 
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
//	m_msgQueued=0L;
	m_actions=0L;

	connect( this, SIGNAL( messageSent( const KopeteMessage&, KopeteMessageManager* ) ),
			this, SLOT( slotMessageSent( const KopeteMessage& , KopeteMessageManager*) ) );

	m_timerOn=false;

}
MSNMessageManager::~MSNMessageManager()
{
	//force to disconnect the switchboard
	if(m_chatService)
		delete m_chatService;
}

void MSNMessageManager::createChat(QString handle, QString address, QString auth, QString ID)
{
	if(m_chatService)
	{
		kdDebug() << "MSNMessageManager::createChat - Service already exists, disconnect thmem " <<endl;
		delete m_chatService;
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
	connect( m_chatService, SIGNAL( msgAcknowledgement(unsigned int, bool) ),
			this, SLOT( slotAcknowledgement(unsigned int, bool) ) );

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

			c = new MSNContact(  handle, publicName, QString::null, m );
			connect( c, SIGNAL( contactDestroyed( KopeteContact * ) ),
				MSNProtocol::protocol(), SLOT( slotContactDestroyed( KopeteContact * ) ) );

			m->addContact( c );
			KopeteContactList::contactList()->addMetaContact(m);

			MSNProtocol::protocol()->contacts().insert( handle, c );
		}
	}

	if(add)
	{
		addContact(c);
		if(!m_messagesQueue.empty()) sendMessageQueue();
	}
	else if(c)
		removeContact(c);
}

void MSNMessageManager::slotSwitchBoardClosed()
{
	kdDebug() << "MSNMessageManager::slotSwitchBoardClosed"  << endl;
	delete m_chatService; //->deleteLater();
	m_chatService=0l;

	for ( QMap<unsigned int , KopeteMessage>::iterator it = m_messagesSent.begin(); it!=m_messagesSent.end(); it = m_messagesSent.begin() )
	{
		KopeteMessage m=it.data();
		QString body=i18n("The following message has not been sent correctly: \n%1").arg(m.plainBody());
		appendMessage(KopeteMessage(m.to().first() , members() , body , KopeteMessage::Internal, KopeteMessage::PlainText));

		m_messagesSent.remove(it);
	}


	setCanBeDeleted(true);
}


void MSNMessageManager::slotUserTypingMsg( QString handle )
{
	MSNContact *c=MSNProtocol::protocol()->contact(handle.lower());
	if(!c)
	{
		kdDebug() << "MSNMessageManager::slotUserTypingMsg : WARNING - KopeteContact not found"  << endl;
		return;
	}
	userTypingMsg(c);
	typingMap[c]=QTime::currentTime();
	if(!m_timerOn)
	{
		m_timerOn=true;
		QTimer::singleShot( 7000, this, SLOT(slotTimer()) );
	}
}

void MSNMessageManager::slotTyping(bool t)
{
	if(t)
	{
		if(m_chatService)
		{
			m_chatService->slotTypingMsg();
			if(!m_timerOn)
			{
				m_timerOn=true;
				QTimer::singleShot( 4000, this, SLOT(slotTimer()) );
			}
		}
		else
		{
			MSNProtocol::protocol()->slotStartChatSession( QPtrList<KopeteContact>(members()).first()->id() );
		}
	}
}

void MSNMessageManager::slotMessageSent(const KopeteMessage &message,KopeteMessageManager *)
{
	if(m_chatService)
	{
		int id= m_chatService->sendMsg(message);
		if(id == -1)
		{
			m_messagesQueue.append(message);
			kdDebug() << "MSNMessageManager::slotMessageSent: message added to the queue" <<endl;
		}
		else
		{
			m_messagesSent.insert( id, message );
			KopeteMessage msg2=message;
			msg2.setBg(QColor()); // BGColor is not send, don't show it on chatwindow
			appendMessage(msg2);
			// send the own msg to chat window
		}
	}
	else // There's no switchboard available, so we must create a new one!
	{
		MSNProtocol::protocol()->slotStartChatSession( message.to().first()->id() );
		m_messagesQueue.append(message);
		//m_msgQueued=new KopeteMessage(message);
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
	if( handle.contains('@') != 1 || handle.contains('.') <1)
	{
			KMessageBox::error(0l, i18n("<qt>You must enter a valide e-mail adress</qt>"), i18n("MSN Plugin"));
			return;
	}
	handle=handle.lower();
	if(m_chatService)
		m_chatService->slotInviteContact(handle);
	else
		MSNProtocol::protocol()->slotStartChatSession( handle );	
}

void MSNMessageManager::sendMessageQueue() 
{
	if(!m_chatService)
	{
		kdDebug() << "MSNMessageManager::sendMessageQueue: service doesn't exist" <<endl;
		return;
	}
	kdDebug() << "MSNMessageManager::sendMessageQueue: " << m_messagesQueue.count() <<endl;
	for ( QValueList<KopeteMessage>::iterator it = m_messagesQueue.begin(); it!=m_messagesQueue.end(); it = m_messagesQueue.begin() )
	{
		//m_chatService->sendMsg( *it)  ;
		slotMessageSent(*it , this);
		m_messagesQueue.remove(it);
	}
}

void MSNMessageManager::slotAcknowledgement(unsigned int id, bool ack)
{
	if(!ack)
	{
		KopeteMessage m=m_messagesSent[id];
		QString body=i18n("The following message has not been sent correctly: \n%1").arg(m.plainBody());
		appendMessage(KopeteMessage(m.to().first() , members() , body , KopeteMessage::Internal, KopeteMessage::PlainText));
	}

	m_messagesSent.remove(id);
}


void MSNMessageManager::slotTimer()
{
	m_timerOn=false;
	if(!currentMessage().plainBody().isEmpty())
		if(m_chatService)
		{
			m_chatService->slotTypingMsg();
			m_timerOn=true;
		}

	QMap<const KopeteContact*,QTime>::Iterator it;
	for( it = typingMap.begin(); it != typingMap.end(); ++it )
	{
		if ( it.data() <= QTime::currentTime().addSecs(-6) )
		{
			userTypingMsg(it.key(),false);
		}
		else
			m_timerOn=true;
	}

	if(m_timerOn)
		QTimer::singleShot( 4000, this, SLOT(slotTimer()) );
	else
		typingMap.clear();
}

#include "msnmessagemanager.moc"

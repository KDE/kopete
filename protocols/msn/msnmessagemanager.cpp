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

#include <kaction.h>
#include <kdebug.h>
#include <klineeditdlg.h>
#include <klocale.h>
#include <kmessagebox.h>

#include "kopete.h"
#include "kopetecontactlist.h"
#include "kopetemessagemanagerfactory.h"
#include "kopetemetacontact.h"
#include "kopetetransfermanager.h"

#include "msncontact.h"
#include "msnfiletransfersocket.h"
#include "msnmessagemanager.h"
#include "msnprotocol.h"
#include "msnswitchboardsocket.h"

MSNMessageManager::MSNMessageManager( const KopeteContact *user,
	KopeteContactPtrList others, const char *name )
: KopeteMessageManager( user, others, MSNProtocol::protocol(), 0,
	ChatWindow, MSNProtocol::protocol(), name )
{
	KopeteMessageManagerFactory::factory()->addKopeteMessageManager( this );
	m_chatService = 0l;
//	m_msgQueued = 0L;
	m_actions = 0L;

	connect( this, SIGNAL( messageSent( const KopeteMessage&,
		KopeteMessageManager* ) ),
		this, SLOT( slotMessageSent( const KopeteMessage&,
		KopeteMessageManager* ) ) );
	connect( kopeteapp->transferManager(),
		SIGNAL( accepted( KopeteTransfer *, const QString& ) ),
		this,
		SLOT( slotFileTransferAccepted( KopeteTransfer *, const QString& ) ) );
	connect( kopeteapp->transferManager(),
		SIGNAL( refused( const KopeteFileTransferInfo & ) ),
		this,
		SLOT( slotFileTransferRefused( const KopeteFileTransferInfo & ) ) );

	m_timerOn = false;
}

MSNMessageManager::~MSNMessageManager()
{
	//force to disconnect the switchboard
	if(m_chatService)
		delete m_chatService;

	QMap<unsigned long int, MSNFileTransferSocket*>::Iterator it;
	for( it = m_invitations.begin(); it != m_invitations.end() ; it = m_invitations.begin())
	{
		m_invitations.remove( it );
		delete *it;
	}
}

void MSNMessageManager::createChat( const QString &handle,
	const QString &address, const QString &auth, const QString &ID )
{
	if( m_chatService )
	{
		kdDebug() << "MSNMessageManager::createChat: "
			<< "Service already exists, disconnect them." << endl;
		delete m_chatService;
	}

	setCanBeDeleted( false );

	m_chatService = new MSNSwitchBoardSocket();
	m_chatService->setHandle( MSNProtocol::protocol()->myself()->id() );
	m_chatService->setMsgHandle( handle );
	m_chatService->connectToSwitchBoard( ID, address, auth );

	connect( m_chatService, SIGNAL( updateChatMember(const QString&,const QString&,bool)),
			this, SLOT( slotUpdateChatMember(const QString&,const QString&,bool) ) );
	connect( m_chatService, SIGNAL( msgReceived( const KopeteMessage & ) ),
			this, SLOT( appendMessage( const KopeteMessage & ) ) );
	connect( m_chatService, SIGNAL( switchBoardClosed() ),
			this, SLOT( slotSwitchBoardClosed() ) );
	connect( m_chatService, SIGNAL( userTypingMsg( const QString& ) ),
			this, SLOT( slotUserTypingMsg( const QString&  ) ) );
	connect( m_chatService, SIGNAL( msgAcknowledgement(unsigned int, bool) ),
			this, SLOT( slotAcknowledgement(unsigned int, bool) ) );
	connect( m_chatService, SIGNAL( invitation( const QString&, const QString& ) ),
			this, SLOT( slotInvitation( const QString&, const QString& ) ) );

}

void MSNMessageManager::slotUpdateChatMember(const QString &handle, const QString &publicName, bool add)
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
		if(!m_messagesQueue.empty())
			sendMessageQueue();
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

	if(!m_chatService && m_invitations.isEmpty())
		setCanBeDeleted(true);
}


void MSNMessageManager::slotUserTypingMsg( const QString &handle )
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
			msg2.setBody(message.plainBody() , KopeteMessage::PlainText);
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
			KMessageBox::error(0l, i18n("<qt>You must enter a valid e-mail address</qt>"), i18n("MSN Plugin"));
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


void MSNMessageManager::slotInvitation(const QString &handle, const QString &msg)
{
	MSNContact *c=MSNProtocol::protocol()->contact(handle);
	if(!c)
		return;

	QRegExp rx("Invitation-Cookie: ([0-9]*)");
	rx.search(msg);
	long unsigned int cookie=rx.cap(1).toUInt();

	if( msg.contains("Invitation-Command: ACCEPT") )
	{
		if(m_invitations.contains(cookie))
		{
			MSNFileTransferSocket *MFTS=m_invitations[cookie];
			if(MFTS && MFTS->incomming())
			{
				rx=QRegExp("IP-Address: ([0-9\\.]*)");
				rx.search(msg);
				QString ip_adress = rx.cap(1);
				rx=QRegExp("AuthCookie: ([0-9]*)");
				rx.search(msg);
				QString authcook = rx.cap(1);
				rx=QRegExp("Port: ([0-9]*)");
				rx.search(msg);
				QString port = rx.cap(1);

				kdDebug() << " MSNMessageManager::slotInvitation: filetransfer: - ip:" <<ip_adress <<" : " <<port <<" -authcook: " <<authcook<<  endl;

				MFTS->setAuthCookie(authcook);
				MFTS->connect(ip_adress, port.toUInt());
			}
			else
			{
				unsigned long int auth = (rand()%(999999))+1;
				MFTS->setAuthCookie(QString::number(auth));

				MFTS->setKopeteTransfer(kopeteapp->transferManager()->addTransfer(c, MFTS->fileName(), MFTS->size(),  c->displayName(), KopeteFileTransferInfo::Outgoing));

				QCString message=QString(
						"MIME-Version: 1.0\r\n"
						"Content-Type: text/x-msmsgsinvite; charset=UTF-8\r\n"
						"\r\n"
						"Invitation-Command: ACCEPT\r\n"
						"Invitation-Cookie: " + QString::number(cookie) + "\r\n"
						"IP-Address: " + m_chatService->getLocalIP() + "\r\n"
						"Port: 6891\r\n"
						"AuthCookie: "+QString::number(auth)+"\r\n"
						"Launch-Application: FALSE\r\n"
						"Request-Data: IP-Address:\r\n\r\n").utf8();
				m_chatService->sendCommand( "MSG" , "N", true, message );

				MFTS->listen(6891);

			}
		}
	}
	else  if( msg.contains("Invitation-Command: CANCEL") )
	{
		if(m_invitations.contains(cookie))
		{
			kdDebug() << " MSNMessageManager::slotInvitation: canceled "<<  endl;
			MSNFileTransferSocket *MFTS=m_invitations[cookie];
			if(MFTS && MFTS->incomming())
			{
				MFTS->setCookie(0);
				m_invitations.remove(cookie);
			}
			else
			{
				m_invitations.remove(cookie);
				delete MFTS;
			}
		}
	}
	else  if( msg.contains("Invitation-Command: INVITE") )
	{
		if( msg.contains("Application-File:") )  //not "Application-Name: File Transfer" because the File Transfer label is sometimes translate
		{
			rx=QRegExp("Application-File: ([A-Za-z0-9@\\._\\- ]*)");
			rx.search(msg);
			QString filename = rx.cap(1);
			rx=QRegExp("Application-FileSize: ([0-9]*)");
			rx.search(msg);
			unsigned long int filesize= rx.cap(1).toUInt();

			MSNFileTransferSocket *MFTS=new MSNFileTransferSocket(true,this);
			MFTS->setCookie(cookie);
			connect(MFTS, SIGNAL( done(MSNFileTransferSocket*) ) , this , SLOT( slotFileTransferDone(MSNFileTransferSocket*) ));
			m_invitations.insert( cookie  , MFTS);
			kopeteapp->transferManager()->askIncommingTransfer(c , filename,  filesize, QString::null, MFTS );
		}
		else
		{
			rx=QRegExp("Application-Name: ([A-Za-z0-9@._\\- ]*)");
			rx.search(msg);
			QString invitname = rx.cap(1);

			QString body=i18n("%1 has sent an unimplemented invitation, the invitation was rejected.\nThe invitation was: %2").arg(c->displayName()).arg(invitname);
			appendMessage(KopeteMessage(MSNProtocol::protocol()->contact(handle) , members() , body , KopeteMessage::Internal, KopeteMessage::PlainText));
		}
	}
}

void MSNMessageManager::slotFileTransferAccepted(KopeteTransfer *trans, const QString& fileName)
{
	if(!members().contains(trans->info().contact()))
		return;

	MSNFileTransferSocket *MFTS=dynamic_cast<MSNFileTransferSocket*>((MSNFileTransferSocket*)(trans->info().internalId()));
	if(!MFTS)
		return;

	if(MFTS->cookie()==0)
	{
		delete MFTS;
		trans->setError(KopeteTransfer::CanceledRemote);
		return;
	}

	if(m_chatService)
	{
		MFTS->setFileName(fileName);
		MFTS->setKopeteTransfer(trans);

		QCString message=QString(
			"MIME-Version: 1.0\r\n"
			"Content-Type: text/x-msmsgsinvite; charset=UTF-8\r\n"
			"\r\n"
			"Invitation-Command: ACCEPT\r\n"
			"Invitation-Cookie: " + QString::number(MFTS->cookie()) + "\r\n"
			"Launch-Application: FALSE\r\n"
			"Request-Data: IP-Address:\r\n"  ).utf8();
		m_chatService->sendCommand( "MSG" , "N", true, message );
	}
	else
	{
		m_invitations.remove(MFTS->cookie());
		delete MFTS;

		if( m_invitations.isEmpty())
			setCanBeDeleted(true);
	}
}

void MSNMessageManager::slotFileTransferRefused(const KopeteFileTransferInfo &info)
{
	if(!members().contains(info.contact()))
		return;

	MSNFileTransferSocket *MFTS=dynamic_cast<MSNFileTransferSocket*>((MSNFileTransferSocket*)(info.internalId()));
	if(!MFTS)
		return;

	m_invitations.remove(MFTS->cookie());
	delete MFTS;

	if(m_chatService)
	{
		QCString message=QString(
			"MIME-Version: 1.0\r\n"
			"Content-Type: text/x-msmsgsinvite; charset=UTF-8\r\n"
			"\r\n"
			"Invitation-Command: CANCEL\r\n"
			"Invitation-Cookie: " + QString::number(MFTS->cookie()) + "\r\n"
			"Cancel-Code: REJECT").utf8();
		m_chatService->sendCommand( "MSG" , "N", true, message );
	}
	else if( m_invitations.isEmpty())
		setCanBeDeleted(true);

}

void MSNMessageManager::slotFileTransferDone(MSNFileTransferSocket* MFTS)
{
	m_invitations.remove(MFTS->cookie());
//	MFTS->deleteLater();
	delete MFTS;
	if(!m_chatService && m_invitations.isEmpty())
		setCanBeDeleted(true);
}

void MSNMessageManager::sendFile(const QString& file)
{
	if(m_chatService)
	{
		unsigned long int cookie = (rand()%(999999))+1;
		MSNFileTransferSocket *MFTS=new MSNFileTransferSocket(false,this);
		MFTS->setCookie(cookie);
		connect(MFTS, SIGNAL( done(MSNFileTransferSocket*) ) , this , SLOT( slotFileTransferDone(MSNFileTransferSocket*) ));
		m_invitations.insert( cookie  , MFTS);
		MFTS->setFileName(file);

		QCString message=QString(
			"MIME-Version: 1.0\r\n"
			"Content-Type: text/x-msmsgsinvite; charset=UTF-8\r\n"
			"\r\n"
			"Application-Name: File Transfer\r\n" // +i18n("File Transfer")
			"Application-GUID: {5D3E02AB-6190-11d3-BBBB-00C04F795683}\r\n"
			"Invitation-Command: INVITE\r\n"
			"Invitation-Cookie: " +QString::number(cookie) +"\r\n"
			"Application-File: "+file.right( file.length() - file.findRev( QRegExp("/") ) - 1 )+"\r\n"
			"Application-FileSize: "+ QString::number(MFTS->size()) +"\r\n\r\n").utf8();

		m_chatService->sendCommand( "MSG" , "N", true, message );
	}
}


#include "msnmessagemanager.moc"


/*
    msnmessagemanager.cpp - MSN Message Manager

    Copyright (c) 2002 by Olivier Goffart        <ogoffart@tiscalinet.be>

    Kopete    (c) 2002 by the Kopete developers  <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/

#include <qtimer.h>

#include <kaction.h>
#include <kdebug.h>
#include <klineeditdlg.h>
#include <klocale.h>
#include <kmessagebox.h>

#include "kopetecontactaction.h"
#include "kopetecontactlist.h"
#include "kopetemessagemanagerfactory.h"
#include "kopetemetacontact.h"
#include "kopetetransfermanager.h"

#include "msncontact.h"
#include "msnfiletransfersocket.h"
#include "msnmessagemanager.h"
#include "msnprotocol.h"
#include "msnaccount.h"
#include "msnswitchboardsocket.h"

MSNMessageManager::MSNMessageManager( KopeteProtocol *protocol, const KopeteContact *user,
	KopeteContactPtrList others, const char *name )
: KopeteMessageManager( user, others, protocol, 0, protocol, name )
{
	m_protocol = protocol;
	KopeteMessageManagerFactory::factory()->addKopeteMessageManager( this );
	m_chatService = 0l;
//	m_msgQueued = 0L;
	m_actions = 0L;

	connect( this, SIGNAL( messageSent( KopeteMessage&,
		KopeteMessageManager* ) ),
		this, SLOT( slotMessageSent( KopeteMessage&,
		KopeteMessageManager* ) ) );
	connect( KopeteTransferManager::transferManager(),
		SIGNAL( accepted( KopeteTransfer *, const QString& ) ),
		this,
		SLOT( slotFileTransferAccepted( KopeteTransfer *, const QString& ) ) );
	connect( KopeteTransferManager::transferManager(),
		SIGNAL( refused( const KopeteFileTransferInfo & ) ),
		this,
		SLOT( slotFileTransferRefused( const KopeteFileTransferInfo & ) ) );
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
		kdDebug(14140) << "MSNMessageManager::createChat: "
			<< "Service already exists, disconnect them." << endl;
		delete m_chatService;
	}

	setCanBeDeleted( false );

	m_chatService = new MSNSwitchBoardSocket( static_cast<MSNIdentity*>( user()->identity() ) );
	m_chatService->setHandle( user()->identity()->identityId() );
	m_chatService->setMsgHandle( handle );
	m_chatService->connectToSwitchBoard( ID, address, auth );

	connect( m_chatService, SIGNAL( updateChatMember(const QString&,const QString&,bool)),
		this, SLOT( slotUpdateChatMember(const QString&,const QString&,bool) ) );
	connect( m_chatService, SIGNAL( msgReceived( KopeteMessage & ) ),
		this, SLOT( appendMessage( KopeteMessage & ) ) );
	connect( m_chatService, SIGNAL( switchBoardClosed() ),
		this, SLOT( slotSwitchBoardClosed() ) );
	connect( m_chatService, SIGNAL( receivedTypingMsg( const QString &, bool ) ),
		this, SLOT( receivedTypingMsg( const QString &, bool ) ) );
	connect( this, SIGNAL( typingMsg( bool ) ),
		m_chatService, SLOT( sendTypingMsg( bool ) ) );
	connect( m_chatService, SIGNAL( msgAcknowledgement(unsigned int, bool) ),
		this, SLOT( slotAcknowledgement(unsigned int, bool) ) );
	connect( m_chatService, SIGNAL( invitation( const QString&, const QString& ) ),
		this, SLOT( slotInvitation( const QString&, const QString& ) ) );
}

void MSNMessageManager::slotUpdateChatMember(const QString &handle, const QString &publicName, bool add)
{
	if( add && !user()->identity()->contacts()[ handle ] )
		user()->identity()->addContact( handle, publicName, 0L, QString::null, true );
		
	MSNContact *c = static_cast<MSNContact*>( user()->identity()->contacts()[ handle ] );

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
	kdDebug(14140) << "MSNMessageManager::slotSwitchBoardClosed"  << endl;
	delete m_chatService; //->deleteLater();
	m_chatService=0l;

	for ( QMap<unsigned int , KopeteMessage>::iterator it = m_messagesSent.begin(); it!=m_messagesSent.end(); it = m_messagesSent.begin() )
	{
		KopeteMessage m=it.data();
		QString body=i18n("The following message has not been sent correctly: \n%1").arg(m.plainBody());
		KopeteMessage msg = KopeteMessage(m.to().first() , members() , body , KopeteMessage::Internal, KopeteMessage::PlainText);
		appendMessage(msg);

		m_messagesSent.remove(it);
	}
}

void MSNMessageManager::slotMessageSent(KopeteMessage &message,KopeteMessageManager *)
{
 	if(m_chatService)
	{
		int id = m_chatService->sendMsg(message);
		if(id == -1)
		{
			m_messagesQueue.append(message);
			kdDebug(14140) << "MSNMessageManager::slotMessageSent: message added to the queue" <<endl;
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
		static_cast<MSNIdentity*>( user()->identity() )->slotStartChatSession( message.to().first()->contactId() );
		m_messagesQueue.append(message);
//		sendMessageQueue();
		//m_msgQueued=new KopeteMessage(message);
	}
}

KActionCollection * MSNMessageManager::chatActions()
{
	delete m_actions;

	m_actions = new KActionCollection( this );

	kdDebug(14140) << "MSNMessageManager::chatActions"  <<endl;

	KActionMenu *actionInvite = new KActionMenu( i18n( "&Invite" ), m_actions , "actionInvite" );
	QPtrList<KopeteContact> availableContacts = KopeteContactList::contactList()->onlineContacts( protocol()->pluginId() );
	QPtrListIterator<KopeteContact> it( availableContacts );
	for( ; it.current(); ++it )
	{
		if( !members().contains( it.current() ) )
		{
			actionInvite->insert( new KopeteContactAction( it.current(), this,
				SLOT( slotInviteContact( KopeteContact * ) ), actionInvite ) );
		}
	}
	actionInvite->insert( new KAction( i18n ("Other ..."), 0, this, SLOT( slotInviteOtherContact() ), actionInvite, "actionOther" ));
	
	m_actions->insert( actionInvite );

	return m_actions;
}

void MSNMessageManager::slotCloseSession()
{
	kdDebug(14140) << "MSNMessageManager::slotCloseSession: " << m_chatService <<endl;
	if(m_chatService)
		m_chatService->slotCloseSession();
}

void MSNMessageManager::slotInviteContact( KopeteContact *contact )
{
	if( m_chatService )
		m_chatService->slotInviteContact( contact->contactId() );
	else
		static_cast<MSNIdentity*>( user()->identity() )->slotStartChatSession( contact->contactId() );
}


void MSNMessageManager::slotInviteOtherContact()
{
	bool ok;
	QString handle = KLineEditDlg::getText(i18n( "MSN Plugin" ),
			i18n( "Please enter the email address of the person you want to invite:" ),
			QString::null, &ok );
	if( !ok )
		return;
	
	if( handle.contains('@') != 1 || handle.contains('.') <1)
	{
			KMessageBox::error(0l, i18n("<qt>You must enter a valid e-mail address</qt>"), i18n("MSN Plugin"));
			return;
	}

	if( m_chatService )
		m_chatService->slotInviteContact( handle );
	else
		static_cast<MSNIdentity*>( user()->identity() )->slotStartChatSession( handle );
}


void MSNMessageManager::sendMessageQueue()
{
	if(!m_chatService)
	{
		kdDebug(14140) << "MSNMessageManager::sendMessageQueue: service doesn't exist" <<endl;
		return;
	}
	kdDebug(14140) << "MSNMessageManager::sendMessageQueue: " << m_messagesQueue.count() <<endl;
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
		KopeteMessage msg = KopeteMessage(m.to().first() , members() , body , KopeteMessage::Internal, KopeteMessage::PlainText);
		appendMessage(msg);
	}
	else
	{
		emit( messageSuccess() );
	}
	m_messagesSent.remove(id);
}

void MSNMessageManager::slotInvitation(const QString &handle, const QString &msg)
{
	//FIXME! a contact from another identity can send a file
	MSNContact *c = static_cast<MSNContact*>( user()->identity()->contacts()[ handle ] );
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
			if(MFTS && MFTS->incoming())
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

				kdDebug(14140) << " MSNMessageManager::slotInvitation: filetransfer: - ip:" <<ip_adress <<" : " <<port <<" -authcook: " <<authcook<<  endl;

				MFTS->setAuthCookie(authcook);
				MFTS->connect(ip_adress, port.toUInt());
			}
			else
			{
				unsigned long int auth = (rand()%(999999))+1;
				MFTS->setAuthCookie(QString::number(auth));

				MFTS->setKopeteTransfer(KopeteTransferManager::transferManager()->addTransfer(c, MFTS->fileName(), MFTS->size(),  c->displayName(), KopeteFileTransferInfo::Outgoing));

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
			kdDebug(14140) << " MSNMessageManager::slotInvitation: canceled "<<  endl;
			MSNFileTransferSocket *MFTS=m_invitations[cookie];
			if(MFTS && MFTS->incoming())
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
			rx=QRegExp("Application-File: ([^\\r\\n]*)");
			rx.search(msg);
			QString filename = rx.cap(1);
			rx=QRegExp("Application-FileSize: ([0-9]*)");
			rx.search(msg);
			unsigned long int filesize= rx.cap(1).toUInt();

			MSNFileTransferSocket *MFTS=new MSNFileTransferSocket(m_protocol,true,this);
			MFTS->setCookie(cookie);
			connect(MFTS, SIGNAL( done(MSNFileTransferSocket*) ) , this , SLOT( slotFileTransferDone(MSNFileTransferSocket*) ));
			m_invitations.insert( cookie  , MFTS);
			KopeteTransferManager::transferManager()->askIncomingTransfer( c , filename, filesize, QString::null, MFTS );
		}
		else
		{
			rx=QRegExp("Application-Name: ([^\\r\\n]*)");
			rx.search(msg);
			QString invitname = rx.cap(1);

			QString body=i18n("%1 has sent an unimplemented invitation, the invitation was rejected.\nThe invitation was: %2").arg(c->displayName()).arg(invitname);
			KopeteMessage tmpMsg = KopeteMessage( c , members() , body , KopeteMessage::Internal, KopeteMessage::PlainText);
			appendMessage(tmpMsg);
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
		MFTS->setFile(fileName);
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

void MSNMessageManager::sendFile(const QString &fileLocation, const QString &fileName, 
	long unsigned int fileSize) 
{
	QString theFileName;
	
	if(m_chatService)
	{
		//If the alternate filename is null, then get the filename from the location
		if( fileName.isNull() ) {
			theFileName = fileLocation.right( fileLocation.length()
				- fileLocation.findRev( QRegExp("/") ) - 1 );
		} else {
			theFileName = fileName;
		}
			
		unsigned long int cookie = (rand()%(999999))+1;
		MSNFileTransferSocket *MFTS=new MSNFileTransferSocket(m_protocol,false,this);
		MFTS->setCookie(cookie);
		connect(MFTS, SIGNAL( done(MSNFileTransferSocket*) ) , this , SLOT( slotFileTransferDone(MSNFileTransferSocket*) ));
		m_invitations.insert( cookie  , MFTS);
		
		//Call the setFile command to let the MFTS know what file we are sending
		MFTS->setFile(fileLocation, fileSize);

		QCString message=QString(
			"MIME-Version: 1.0\r\n"
			"Content-Type: text/x-msmsgsinvite; charset=UTF-8\r\n"
			"\r\n"
			"Application-Name: File Transfer\r\n" // +i18n("File Transfer")
			"Application-GUID: {5D3E02AB-6190-11d3-BBBB-00C04F795683}\r\n"
			"Invitation-Command: INVITE\r\n"
			"Invitation-Cookie: " +QString::number(cookie) +"\r\n"
			"Application-File: "+ theFileName +"\r\n"
			"Application-FileSize: "+ QString::number(MFTS->size()) +"\r\n\r\n").utf8();

		m_chatService->sendCommand( "MSG" , "N", true, message );
	}
}


#include "msnmessagemanager.moc"

// vim: set noet ts=4 sts=4 sw=4:


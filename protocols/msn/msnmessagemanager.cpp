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


#include <kdebug.h>
#include <klineeditdlg.h>
#include <klocale.h>
#include <kmessagebox.h>

#include "kopetecontactaction.h"
#include "kopetecontactlist.h"
#include "kopetemessagemanagerfactory.h"

#include "msncontact.h"
#include "msnfiletransfersocket.h"
#include "msnmessagemanager.h"
#include "msnaccount.h"
#include "msnswitchboardsocket.h"

#include "msnvoiceinvitation.h"

#if !defined NDEBUG
#include "msndebugrawcmddlg.h"
#endif

MSNMessageManager::MSNMessageManager( KopeteProtocol *protocol, const KopeteContact *user,
	KopeteContactPtrList others, const char *name )
: KopeteMessageManager( user, others, protocol, 0, protocol, name )
{
	KopeteMessageManagerFactory::factory()->addKopeteMessageManager( this );
	m_chatService = 0l;
//	m_msgQueued = 0L;
	m_actions = 0L;

	connect( this, SIGNAL( messageSent( KopeteMessage&,
		KopeteMessageManager* ) ),
		this, SLOT( slotMessageSent( KopeteMessage&,
		KopeteMessageManager* ) ) );

	connect( this, SIGNAL( invitation(MSNInvitation*& ,  const QString & , long unsigned int , MSNMessageManager*  , MSNContact*  ) ) ,
		protocol,  SIGNAL( invitation(MSNInvitation*& ,  const QString & , long unsigned int , MSNMessageManager*  , MSNContact*  ) ) );
}

MSNMessageManager::~MSNMessageManager()
{
	//force to disconnect the switchboard
	if(m_chatService)
		delete m_chatService;

	QMap<unsigned long int, MSNInvitation*>::Iterator it;
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

//	uncomment this line if you don't want to the peer know when you close the window
//	setCanBeDeleted( false );

	m_chatService = new MSNSwitchBoardSocket( static_cast<MSNAccount*>( user()->account() ) );
	m_chatService->setHandle( user()->account()->accountId() );
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
	if( add && !user()->account()->contacts()[ handle ] )
		user()->account()->addContact( handle, publicName, 0L, QString::null, true );

	MSNContact *c = static_cast<MSNContact*>( user()->account()->contacts()[ handle ] );

	if( add && c->displayName() != publicName)
	{
		c->rename(publicName);
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

	if(m_invitations.isEmpty())
		setCanBeDeleted( true );
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
			message.setBg(QColor()); // clear the bgColor
			message.setBody(message.plainBody() , KopeteMessage::PlainText ); //clear every custom tag which are not sent
			appendMessage(message); // send the own msg to chat window
		}
	}
	else // There's no switchboard available, so we must create a new one!
	{
		static_cast<MSNAccount*>( user()->account() )->slotStartChatSession( message.to().first()->contactId() );
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

	#if !defined NDEBUG
	KActionMenu *debugMenu = new KActionMenu( "Debug", m_actions );
	debugMenu->insert( new KAction( i18n( "Send Raw C&ommand..." ), 0, this, SLOT( slotDebugRawCommand() ), debugMenu, "m_debugRawCommand" ) );
	debugMenu->insert( new KAction( i18n( "Voice Chat" ), 0, this, SLOT( slotVoiceChat() ), debugMenu, "m_voiceChat" ) );
	m_actions->insert( debugMenu );
	#endif

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
		static_cast<MSNAccount*>( user()->account() )->slotStartChatSession( contact->contactId() );
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
		static_cast<MSNAccount*>( user()->account() )->slotStartChatSession( handle );
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
	//FIXME! a contact from another account can send a file
	MSNContact *c = static_cast<MSNContact*>( user()->account()->contacts()[ handle ] );
	if(!c)
		return;

	QRegExp rx("Invitation-Cookie: ([0-9]*)");
	rx.search(msg);
	long unsigned int cookie=rx.cap(1).toUInt();

	if(m_invitations.contains(cookie))
	{
		MSNInvitation *msnI=m_invitations[cookie];
		msnI->parseInvitation(msg);
	}
	else if( msg.contains("Invitation-Command: INVITE") )
	{
		if( msg.contains(MSNFileTransferSocket::applicationID()) )
		{
			MSNFileTransferSocket *MFTS=new MSNFileTransferSocket(user()->account()->accountId(),c,true,this);
			connect(MFTS, SIGNAL( done(MSNInvitation*) ) , this , SLOT( invitationDone(MSNInvitation*) ));
			m_invitations.insert( cookie  , MFTS);
			MFTS->parseInvitation(msg);
			setCanBeDeleted(false);
		}
		else if( msg.contains(MSNVoiceInvitation::applicationID()) )
		{
			MSNVoiceInvitation *msnVI=new MSNVoiceInvitation(true,c,this);
			connect(msnVI, SIGNAL( done(MSNInvitation*) ) , this , SLOT( invitationDone(MSNInvitation*) ));
			m_invitations.insert( cookie  , msnVI);
			msnVI->parseInvitation(msg);
			setCanBeDeleted(false);
		}
		else
		{
			MSNInvitation *i=0l;
			emit invitation( i , msg, cookie, this, c );
			if(i)
			{
				m_invitations.insert( cookie  , i );
				//don't delete this if all invitation are not done
				setCanBeDeleted(false);
			}
			else
			{
				rx=QRegExp("Application-Name: ([^\\r\\n]*)");
				rx.search(msg);
				QString invitname = rx.cap(1);

				QString body=i18n("%1 has sent an unimplemented invitation, the invitation was rejected.\nThe invitation was: %2").arg(c->displayName()).arg(invitname);
				KopeteMessage tmpMsg = KopeteMessage( c , members() , body , KopeteMessage::Internal, KopeteMessage::PlainText);
				appendMessage(tmpMsg);

				m_chatService->sendCommand( "MSG" , "N", true, MSNInvitation::unimplemented(cookie) );
			}
		}
	}
}

void MSNMessageManager::invitationDone(MSNInvitation* MFTS)
{
	kdDebug(14140) << k_funcinfo <<endl;
	m_invitations.remove(MFTS->cookie());
//	MFTS->deleteLater();
	delete MFTS;
	if(!m_chatService && m_invitations.isEmpty())
		setCanBeDeleted(true);
}

void MSNMessageManager::sendFile(const QString &fileLocation, const QString &/*fileName*/,
	long unsigned int fileSize)
{
	if(m_chatService)
	{
		//If the alternate filename is null, then get the filename from the location (FIXME)
		/*QString theFileName;
		if( fileName.isNull() ) {
			theFileName = fileLocation.right( fileLocation.length()
				- fileLocation.findRev( QRegExp("/") ) - 1 );
		} else {
			theFileName = fileName;
		}*/

		QPtrList<KopeteContact>contacts=members();
		MSNFileTransferSocket *MFTS=new MSNFileTransferSocket(user()->account()->accountId(),contacts.first(), false,this);
		connect(MFTS, SIGNAL( done(MSNInvitation*) ) , this , SLOT( invitationDone(MSNInvitation*) ));
		m_invitations.insert( MFTS->cookie()  , MFTS);

		//Call the setFile command to let the MFTS know what file we are sending
		MFTS->setFile(fileLocation, fileSize);

		m_chatService->sendCommand( "MSG" , "N", true, MFTS->invitationHead() );
	}
}

void MSNMessageManager::slotVoiceChat()
{
	if(m_chatService)
	{
		QPtrList<KopeteContact>contacts=members();
		MSNVoiceInvitation *msnVI=new MSNVoiceInvitation(false,static_cast<MSNContact*>(contacts.first()),this);
		connect(msnVI, SIGNAL( done(MSNInvitation*) ) , this , SLOT( invitationDone(MSNInvitation*) ));
		m_invitations.insert( msnVI->cookie() , msnVI);

		m_chatService->sendCommand( "MSG" , "N", true, msnVI->invitationHead() );
	}
}

void MSNMessageManager::slotDebugRawCommand()
{
#if !defined NDEBUG
	if ( !m_chatService )
		return;

	MSNDebugRawCmdDlg *dlg = new MSNDebugRawCmdDlg( 0L );
	int result = dlg->exec();
	if( result == QDialog::Accepted && m_chatService )
	{
		m_chatService->sendCommand( dlg->command(), dlg->params(),
					dlg->addId(), dlg->msg().replace("\n","\r\n").utf8() );
	}
	delete dlg;
#endif
}


#include "msnmessagemanager.moc"

// vim: set noet ts=4 sts=4 sw=4:


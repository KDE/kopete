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
#include <kpopupmenu.h>
#include <kconfig.h>

#include "kopetecontactaction.h"
#include "kopetecontactlist.h"
#include "kopetemessagemanagerfactory.h"

#include "msncontact.h"
#include "msnfiletransfersocket.h"
#include "msnmessagemanager.h"
#include "msnaccount.h"
#include "msnswitchboardsocket.h"

#if !defined NDEBUG
#include "msndebugrawcmddlg.h"
#endif

MSNMessageManager::MSNMessageManager( KopeteProtocol *protocol, const KopeteContact *user,
	KopeteContactPtrList others, const char *name )
: KopeteMessageManager( user, others, protocol, 0, name )
{
	KopeteMessageManagerFactory::factory()->addKopeteMessageManager( this );
	m_chatService = 0l;
//	m_msgQueued = 0L;

	connect( this, SIGNAL( messageSent( KopeteMessage&,
		KopeteMessageManager* ) ),
		this, SLOT( slotMessageSent( KopeteMessage&,
		KopeteMessageManager* ) ) );

	connect( this, SIGNAL( invitation(MSNInvitation*& ,  const QString & , long unsigned int , MSNMessageManager*  , MSNContact*  ) ) ,
		protocol,  SIGNAL( invitation(MSNInvitation*& ,  const QString & , long unsigned int , MSNMessageManager*  , MSNContact*  ) ) );


	m_actionInvite = new KActionMenu( i18n( "&Invite" ), actionCollection() , "msnInvite" );
	connect ( m_actionInvite->popupMenu() , SIGNAL( aboutToShow() ) , this , SLOT(slotActionInviteAboutToShow() ) ) ;

	#if !defined NDEBUG
	new KAction( i18n( "Send Raw C&ommand..." ), 0, this, SLOT( slotDebugRawCommand() ), actionCollection(), "msnDebugRawCommand" ) ;
	#endif


	setXMLFile("msnchatui.rc");
}

MSNMessageManager::~MSNMessageManager()
{
	//force to disconnect the switchboard
	//not needed since the m_chatService has us as parent
	//	if(m_chatService)
	//		delete m_chatService;

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

	m_chatService = new MSNSwitchBoardSocket( static_cast<MSNAccount*>( user()->account() ) , this);
	m_chatService->setHandle( user()->account()->accountId() );
	m_chatService->setMsgHandle( handle );
	m_chatService->connectToSwitchBoard( ID, address, auth );

	connect( m_chatService, SIGNAL( userJoined(const QString&,const QString&,bool)),
		this, SLOT( slotUserJoined(const QString&,const QString&,bool) ) );
	connect( m_chatService, SIGNAL( userLeft(const QString&,const QString&)),
		this, SLOT( slotUserLeft(const QString&,const QString&) ) );
	connect( m_chatService, SIGNAL( msgReceived( KopeteMessage & ) ),
		this, SLOT( slotMessageReceived( KopeteMessage & ) ) );
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

void MSNMessageManager::slotUserJoined( const QString &handle, const QString &publicName, bool IRO )
{
	if( !account()->contacts()[ handle ] )
		account()->addContact( handle, publicName, 0L, QString::null, true );

	MSNContact *c = static_cast<MSNContact*>( account()->contacts()[ handle ] );

	if( c->displayName() != publicName)
		c->rename(publicName);

	addContact(c , IRO); // don't show notificaions when we join wesalef
	if(!m_messagesQueue.empty())
		sendMessageQueue();
}

void MSNMessageManager::slotUserLeft( const QString &handle, const QString& reason )
{
	MSNContact *c = static_cast<MSNContact*>( user()->account()->contacts()[ handle ] );
	if(c)
		removeContact(c, reason );
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

void MSNMessageManager::slotMessageReceived( KopeteMessage &msg )
{
	if(msg.plainBody().startsWith("AutoMessage: "))
	{
		//FIXME: HardCodded color are not so good
		msg.setFg(QColor("SlateGray3"));
		QFont f;
		f.setItalic(true);
		msg.setFont(f);
	}
	appendMessage(msg);
	if(account()->isAway() && !static_cast<MSNAccount*>(account())->awayReason().isEmpty())
	{
		KGlobal::config()->setGroup("MSN");
		if( KGlobal::config()->readBoolEntry( "SendAwayMessages", true ) &&
			( !m_awayMessageTime.isValid()  ||  m_awayMessageTime.elapsed() > 1000* KGlobal::config()->readNumEntry( "AwayMessagesSeconds", 90 ))  )
		{
			//Don't translate "Auto-Message:" This string is caught by MSN Plus! , (and also by kopete now)
			KopeteMessage msg2(user(), members() , "AutoMessage: " + static_cast<MSNAccount*>(account())->awayReason() , KopeteMessage::Outbound );
			msg2.setFg(QColor("SlateGray3"));
			QFont f;
			f.setItalic(true);
			msg2.setFont(f);
			slotMessageSent(msg2,this);
			m_awayMessageTime.restart();
		}
	}
}


void MSNMessageManager::slotActionInviteAboutToShow()
{
	// We can't simply insert  KAction in this menu bebause we don't know when to delete them.
	//  items inserted with insert items are automaticaly deleted when we call clear

	m_inviteactions.setAutoDelete(true);
	m_inviteactions.clear();

	m_actionInvite->popupMenu()->clear();

	QPtrList<KopeteContact> availableContacts = KopeteContactList::contactList()->onlineContacts( protocol()->pluginId() );
	QPtrListIterator<KopeteContact> it( availableContacts );
	for( ; it.current(); ++it )
	{
		if( !members().contains( it.current() ) )
		{
			KAction *a=new KopeteContactAction( it.current(), this,
				SLOT( slotInviteContact( KopeteContact * ) ), m_actionInvite );
			m_actionInvite->insert( a );
			m_inviteactions.append( a ) ;

		}
	}
	KAction *b=new KAction( i18n ("Other ..."), 0, this, SLOT( slotInviteOtherContact() ), m_actionInvite, "actionOther" );
	m_actionInvite->insert( b );
	m_inviteactions.append( b ) ;
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
				QString inviteName = rx.cap( 1 );

				QString body = i18n(
					"%1 has sent an unimplemented invitation, the invitation was rejected.\n"
					"The invitation was: %2" ).
#if QT_VERSION < 0x030200
					arg( c->displayName() ).arg( inviteName );
#else
					arg( c->displayName(), inviteName );
#endif
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

		//Call the setFile command to let the MFTS know what file we are sending
		MFTS->setFile(fileLocation, fileSize);

		initInvitation(MFTS);
	}
}

void MSNMessageManager::initInvitation(MSNInvitation* invitation)
{
	if(m_chatService)
	{
		connect(invitation->object(), SIGNAL( done(MSNInvitation*) ) , this , SLOT( invitationDone(MSNInvitation*) ));
		m_invitations.insert( invitation->cookie() , invitation);

		m_chatService->sendCommand( "MSG" , "N", true, invitation->invitationHead() );
	}
	else
	{
		//TODO: connect the switchboard, and add the invitation to a quee
		//FIXME: show a message box
		delete invitation;
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


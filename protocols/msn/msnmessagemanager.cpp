/*
    msnmessagemanager.cpp - MSN Message Manager

    Copyright (c) 2002-2004 by Olivier Goffart        <ogoffart@tiscalinet.be>

    Kopete    (c) 2002-2004 by the Kopete developers  <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/

#include "msnmessagemanager.h"

#include <qlabel.h>
#include <qimage.h>
#include <qtooltip.h>
#include <qfile.h>

#include <kconfig.h>
#include <kdebug.h>
#include <kinputdialog.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kpopupmenu.h>
#include <ktempfile.h>
#include <kmainwindow.h>
#include <ktoolbar.h>
#include <krun.h>

#include "kopetecontactaction.h"
#include "kopetemetacontact.h"
#include "kopetecontactlist.h"
#include "kopetemessagemanagerfactory.h"
#include "kopeteuiglobal.h"
#include "kopeteglobal.h"
#include "kopeteview.h"

#include "msncontact.h"
#include "msnfiletransfersocket.h"
#include "msnaccount.h"
#include "msnswitchboardsocket.h"

#if !defined NDEBUG
#include "msndebugrawcmddlg.h"
#endif

MSNChatSession::MSNChatSession( Kopete::Protocol *protocol, const Kopete::Contact *user,
	Kopete::ContactPtrList others, const char *name )
: Kopete::ChatSession( user, others, protocol,  name )
{
	Kopete::ChatSessionManager::self()->registerChatSession( this );
	m_chatService = 0l;
//	m_msgQueued = 0L;
	m_newSession = true;

	setInstance(protocol->instance());

	connect( this, SIGNAL( messageSent( Kopete::Message&,
		Kopete::ChatSession* ) ),
		this, SLOT( slotMessageSent( Kopete::Message&,
		Kopete::ChatSession* ) ) );

	connect( this, SIGNAL( invitation(MSNInvitation*& ,  const QString & , long unsigned int , MSNChatSession*  , MSNContact*  ) ) ,
		protocol,  SIGNAL( invitation(MSNInvitation*& ,  const QString & , long unsigned int , MSNChatSession*  , MSNContact*  ) ) );


	m_actionInvite = new KActionMenu( i18n( "&Invite" ), actionCollection() , "msnInvite" );
	connect ( m_actionInvite->popupMenu() , SIGNAL( aboutToShow() ) , this , SLOT(slotActionInviteAboutToShow() ) ) ;

	#if !defined NDEBUG
	new KAction( i18n( "Send Raw C&ommand..." ), 0, this, SLOT( slotDebugRawCommand() ), actionCollection(), "msnDebugRawCommand" ) ;
	#endif

	MSNContact *c = static_cast<MSNContact*>( others.first() );
	(new KAction( i18n( "Request Display Picture" ), "image", 0,  this, SLOT( slotRequestPicture() ), actionCollection(), "msnRequestDisplayPicture" ))->setEnabled(!c->object().isEmpty());

	if ( !c->object().isEmpty() )
	{
		
		connect( c, SIGNAL( displayPictureChanged() ), this, SLOT( slotDisplayPictureChanged() ) );
		m_image = new QLabel( 0L, "kde toolbar widget" );
		new KWidgetAction( m_image, i18n( "MSN Display Picture" ), 0, this, SLOT( slotRequestPicture() ), actionCollection(), "msnDisplayPicture" );
		if(c->hasProperty(Kopete::Global::Properties::self()->photo().key())  )
		{
			//if the view doesn't exist yet, we will be unable to get the size of the toolbar
			// so when the view will exist, we will show the displaypicture.
			//How to know when a our view is created?  We can't.
			// but chances are the next created view will be for this KMM
			// And if it is not?  never mind. the icon will just be sized 22x22
			connect( Kopete::ChatSessionManager::self() , SIGNAL(viewActivated(KopeteView* )) , this, SLOT(slotDisplayPictureChanged()) );
			//it's viewActivated and not viewCreated because the view get his mainwindow only when it is shown.
		}
	}
	else
	{
		m_image = 0L;
	}

	setXMLFile("msnchatui.rc");
	
	setMayInvite( true );
}

MSNChatSession::~MSNChatSession()
{
	delete m_image;
	//force to disconnect the switchboard
	//not needed since the m_chatService has us as parent
	//	if(m_chatService)
	//		delete m_chatService;

	QMap<unsigned long int, MSNInvitation*>::Iterator it;
	for( it = m_invitations.begin(); it != m_invitations.end() ; it = m_invitations.begin())
	{
		delete *it;
		m_invitations.remove( it );
	}
}

void MSNChatSession::createChat( const QString &handle,
	const QString &address, const QString &auth, const QString &ID )
{
	/** disabled because i don't want to reopen a chatwindow if we just closed it
	 * and the contact take much time to type his message
	 m_newSession= !(ID.isEmpty());
	*/
	
	if( m_chatService )
	{
		kdDebug(14140) << k_funcinfo << "Service already exists, disconnect them." << endl;
		delete m_chatService;
	}

//	uncomment this line if you don't want to the peer know when you close the window
//	setCanBeDeleted( false );

	m_chatService = new MSNSwitchBoardSocket( static_cast<MSNAccount*>( myself()->account() ) , this);
	m_chatService->setHandle( myself()->account()->accountId() );
	m_chatService->setMsgHandle( handle );
	m_chatService->connectToSwitchBoard( ID, address, auth );

	connect( m_chatService, SIGNAL( userJoined(const QString&,const QString&,bool)),
		this, SLOT( slotUserJoined(const QString&,const QString&,bool) ) );
	connect( m_chatService, SIGNAL( userLeft(const QString&,const QString&)),
		this, SLOT( slotUserLeft(const QString&,const QString&) ) );
	connect( m_chatService, SIGNAL( msgReceived( Kopete::Message & ) ),
		this, SLOT( slotMessageReceived( Kopete::Message & ) ) );
	connect( m_chatService, SIGNAL( switchBoardClosed() ),
		this, SLOT( slotSwitchBoardClosed() ) );
	connect( m_chatService, SIGNAL( receivedTypingMsg( const QString &, bool ) ),
		this, SLOT( receivedTypingMsg( const QString &, bool ) ) );
	connect( this, SIGNAL( myselfTyping( bool ) ),
		m_chatService, SLOT( sendTypingMsg( bool ) ) );
	connect( m_chatService, SIGNAL( msgAcknowledgement(unsigned int, bool) ),
		this, SLOT( slotAcknowledgement(unsigned int, bool) ) );
	connect( m_chatService, SIGNAL( invitation( const QString&, const QString& ) ),
		this, SLOT( slotInvitation( const QString&, const QString& ) ) );
}

void MSNChatSession::slotUserJoined( const QString &handle, const QString &publicName, bool IRO )
{
	if( !account()->contacts()[ handle ] )
		account()->addContact( handle, QString::null, 0L, Kopete::Account::Temporary);

	MSNContact *c = static_cast<MSNContact*>( account()->contacts()[ handle ] );

	c->setProperty( Kopete::Global::Properties::self()->nickName() , publicName);

	addContact(c , IRO); // don't show notificaions when we join wesalef
	if(!m_messagesQueue.empty() || !m_invitations.isEmpty())
		sendMessageQueue();

	KConfig *config = KGlobal::config();
	config->setGroup( "MSN" );
	if ( members().count()==1 && config->readBoolEntry( "AutoDownloadPicture", true ) && !c->object().isEmpty() && !c->hasProperty(Kopete::Global::Properties::self()->photo().key()))
		slotRequestPicture();
}

void MSNChatSession::slotUserLeft( const QString &handle, const QString& reason )
{
	MSNContact *c = static_cast<MSNContact*>( myself()->account()->contacts()[ handle ] );
	if(c)
		removeContact(c, reason );
}



void MSNChatSession::slotSwitchBoardClosed()
{
	//kdDebug(14140) << "MSNChatSession::slotSwitchBoardClosed"  << endl;
	m_chatService->deleteLater();
	m_chatService=0l;

	for ( QMap<unsigned int , Kopete::Message>::iterator it = m_messagesSent.begin(); it!=m_messagesSent.end(); it = m_messagesSent.begin() )
	{
		Kopete::Message m=it.data();
		QString body=i18n("The following message has not been sent correctly: \n%1").arg(m.plainBody());
		Kopete::Message msg = Kopete::Message(m.to().first() , members() , body , Kopete::Message::Internal, Kopete::Message::PlainText);
		appendMessage(msg);

		m_messagesSent.remove(it);
	}

	if(m_invitations.isEmpty())
		setCanBeDeleted( true );
}

void MSNChatSession::slotMessageSent(Kopete::Message &message,Kopete::ChatSession *)
{
	m_newSession=false;
 	if(m_chatService)
	{
		int id = m_chatService->sendMsg(message);
		if(id == -1)
		{
			m_messagesQueue.append(message);
			kdDebug(14140) << k_funcinfo << "Message added to the queue" <<endl;
		}
		else if( id== -2 ) //the message has not been sent
		{
			//FIXME:  tell the what window the message has been processed. but we havent't sent it 
			messageSucceeded();  //that should stop the blonking icon.
		}
		else if( id == -3) //the message has been sent as an immge
		{
			appendMessage(message); 
			messageSucceeded();
		}
		else
		{
			m_messagesSent.insert( id, message );
			message.setBg(QColor()); // clear the bgColor
			message.setBody(message.plainBody() , Kopete::Message::PlainText ); //clear every custom tag which are not sent
			appendMessage(message); // send the own msg to chat window
		}
	}
	else // There's no switchboard available, so we must create a new one!
	{
		static_cast<MSNAccount*>( myself()->account() )->slotStartChatSession( message.to().first()->contactId() );
		m_messagesQueue.append(message);
//		sendMessageQueue();
		//m_msgQueued=new Kopete::Message(message);
	}
}

void MSNChatSession::slotMessageReceived( Kopete::Message &msg )
{
	m_newSession=false;
	if( msg.plainBody().startsWith( "AutoMessage: " ) )
	{
		//FIXME: HardCodded color are not so good
		msg.setFg( QColor( "SlateGray3" ) );
		QFont f;
		f.setItalic( true );
		msg.setFont( f );
	}
	appendMessage( msg );
	if( account()->isAway() && !static_cast<MSNAccount *>( account() )->awayReason().isEmpty() )
	{
		KConfig *config = KGlobal::config();
		config->setGroup( "MSN" );
		if ( config->readBoolEntry( "SendAwayMessages", false ) &&
			( !m_awayMessageTime.isValid() ||
			m_awayMessageTime.elapsed() > 1000 * config->readNumEntry( "AwayMessagesSeconds", 90 ) )  )
		{
			// Don't translate "Auto-Message:" This string is caught by MSN Plus! (and also by kopete now)
			Kopete::Message msg2( myself(), members(),
				"AutoMessage: " + static_cast<MSNAccount *>( account() )->awayReason(), Kopete::Message::Outbound );
			msg2.setFg( QColor( "SlateGray3" ) );
			QFont f;
			f.setItalic( true );
			msg2.setFont( f );
			slotMessageSent( msg2, this );
			m_awayMessageTime.restart();
		}
	}
}

void MSNChatSession::slotActionInviteAboutToShow()
{
	// We can't simply insert  KAction in this menu bebause we don't know when to delete them.
	//  items inserted with insert items are automatically deleted when we call clear

	m_inviteactions.setAutoDelete(true);
	m_inviteactions.clear();

	m_actionInvite->popupMenu()->clear();

	
	QDictIterator<Kopete::Contact> it( account()->contacts() );
	for( ; it.current(); ++it )
	{
		if( !members().contains( it.current() ) && it.current()->isOnline() && it.current() != myself() )
		{
			KAction *a=new KopeteContactAction( it.current(), this,
				SLOT( slotInviteContact( Kopete::Contact * ) ), m_actionInvite );
			m_actionInvite->insert( a );
			m_inviteactions.append( a ) ;
		}
	}
	KAction *b=new KAction( i18n ("Other..."), 0, this, SLOT( slotInviteOtherContact() ), m_actionInvite, "actionOther" );
	m_actionInvite->insert( b );
	m_inviteactions.append( b ) ;
}

void MSNChatSession::slotCloseSession()
{
	kdDebug(14140) << k_funcinfo  << m_chatService <<endl;
	if(m_chatService)
		m_chatService->slotCloseSession();
}

void MSNChatSession::slotInviteContact( Kopete::Contact *contact )
{
	if(contact)
		inviteContact( contact->contactId() );
}

void MSNChatSession::inviteContact(const QString &contactId)
{
	if( m_chatService )
		m_chatService->slotInviteContact( contactId );
	else
		static_cast<MSNAccount*>( myself()->account() )->slotStartChatSession( contactId );
}

void MSNChatSession::slotInviteOtherContact()
{
	bool ok;
	QString handle = KInputDialog::getText(i18n( "MSN Plugin" ),
			i18n( "Please enter the email address of the person you want to invite:" ),
			QString::null, &ok );
	if( !ok )
		return;

	if( handle.contains('@') != 1 || handle.contains('.') <1)
	{
			KMessageBox::queuedMessageBox( Kopete::UI::Global::mainWidget(), KMessageBox::Sorry,
					i18n("<qt>You must enter a valid email address.</qt>"), i18n("MSN Plugin"));
			return;
	}

	inviteContact(handle);
}


void MSNChatSession::sendMessageQueue()
{
	if(!m_chatService)
	{
		kdDebug(14140) <<k_funcinfo << "Service doesn't exist" <<endl;
		return;
	}
//	kdDebug(14140) << "MSNChatSession::sendMessageQueue: " << m_messagesQueue.count() <<endl;
	for ( QValueList<Kopete::Message>::iterator it = m_messagesQueue.begin(); it!=m_messagesQueue.end(); it = m_messagesQueue.begin() )
	{
		//m_chatService->sendMsg( *it)  ;
		slotMessageSent(*it , this);
		m_messagesQueue.remove(it);
	}


	QMap<unsigned long int, MSNInvitation*>::Iterator it;
	for( it = m_invitations.begin(); it != m_invitations.end() ; ++it)
	{
		if(! (*it)->incoming() && (*it)->state()<MSNInvitation::Invited)
		{
			m_chatService->sendCommand( "MSG" , "N", true, (*it)->invitationHead().utf8() );
			(*it)->setState(MSNInvitation::Invited);
		}
	}
}

void MSNChatSession::slotAcknowledgement(unsigned int id, bool ack)
{
	if ( !m_messagesSent.contains( id ) )
	{
		// This is maybe a ACK/NAK for a non-messaging message
		return;
	}

	if ( !ack )
	{
		Kopete::Message m = m_messagesSent[ id ];
		QString body = i18n( "The following message has not been sent correctly:\n%1" ).arg( m.plainBody() );
		Kopete::Message msg = Kopete::Message( m.to().first(), members(), body, Kopete::Message::Internal, Kopete::Message::PlainText );
		appendMessage( msg );
		//stop the stupid animation
		messageSucceeded();  
	}
	else
	{
		messageSucceeded();
	}

	m_messagesSent.remove( id );
}

void MSNChatSession::slotInvitation(const QString &handle, const QString &msg)
{
	//FIXME! a contact from another account can send a file
	MSNContact *c = static_cast<MSNContact*>( myself()->account()->contacts()[ handle ] );
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
			MSNFileTransferSocket *MFTS=new MSNFileTransferSocket(myself()->account()->accountId(),c,true,this);
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
					"The invitation was: %2" )
						.arg( c->property( Kopete::Global::Properties::self()->nickName()).value().toString(), inviteName );
				Kopete::Message tmpMsg = Kopete::Message( c , members() , body , Kopete::Message::Internal, Kopete::Message::PlainText);
				appendMessage(tmpMsg);

				m_chatService->sendCommand( "MSG" , "N", true, MSNInvitation::unimplemented(cookie) );
			}
		}
	}
}

void MSNChatSession::invitationDone(MSNInvitation* MFTS)
{
	kdDebug(14140) << k_funcinfo <<endl;
	m_invitations.remove(MFTS->cookie());
//	MFTS->deleteLater();
	delete MFTS;
	if(!m_chatService && m_invitations.isEmpty())
		setCanBeDeleted(true);
}

void MSNChatSession::sendFile(const QString &fileLocation, const QString &/*fileName*/,
	long unsigned int fileSize)
{
//	if(m_chatService)
//	{
		//If the alternate filename is null, then get the filename from the location (FIXME)
		/*QString theFileName;
		if( fileName.isNull() ) {
			theFileName = fileLocation.right( fileLocation.length()
				- fileLocation.findRev( '/' ) ) - 1 );
		} else {
			theFileName = fileName;
		}*/

		QPtrList<Kopete::Contact>contacts=members();
		MSNFileTransferSocket *MFTS=new MSNFileTransferSocket(myself()->account()->accountId(),contacts.first(), false,this);

		//Call the setFile command to let the MFTS know what file we are sending
		MFTS->setFile(fileLocation, fileSize);

		initInvitation(MFTS);
//	}
}

void MSNChatSession::initInvitation(MSNInvitation* invitation)
{
	connect(invitation->object(), SIGNAL( done(MSNInvitation*) ) , this , SLOT( invitationDone(MSNInvitation*) ));
	m_invitations.insert( invitation->cookie() , invitation);

	if(m_chatService)
	{
		m_chatService->sendCommand( "MSG" , "N", true, invitation->invitationHead().utf8() );
		invitation->setState(MSNInvitation::Invited);
	}
	else
	{
		QPtrList<Kopete::Contact> mb=members();
		static_cast<MSNAccount*>( account() )->slotStartChatSession( mb.first()->contactId() );
	}
}

void MSNChatSession::slotRequestPicture()
{
	QPtrList<Kopete::Contact> mb=members();
	MSNContact *c = static_cast<MSNContact*>( mb.first() );
	if(!c)
	 return;
	
	if( !c->hasProperty(Kopete::Global::Properties::self()->photo().key()))
	{
		if(m_chatService)
		{
			if( !c->object().isEmpty() )
				m_chatService->requestDisplayPicture();
		}
		else
			static_cast<MSNAccount*>( account() )->slotStartChatSession( mb.first()->contactId() );
	}
	else
	{ //we already have the picture, just show it.
		KRun::runURL( KURL::fromPathOrURL( c->property(Kopete::Global::Properties::self()->photo()).value().toString() ), "image/png" );
	}

}

void MSNChatSession::slotDisplayPictureChanged()
{
	const MSNContact *c = static_cast<const MSNContact *>( members().getFirst() );
	if ( c && m_image )
	{
		if(c->hasProperty(Kopete::Global::Properties::self()->photo().key()))
		{
			int sz=22;
			// get the size of the toolbar were the aciton is plugged.
			//  if you know a better way to get the toolbar, let me know
			KMainWindow *w= view(false) ? dynamic_cast<KMainWindow*>( view(false)->mainWidget()->topLevelWidget() ) : 0L;
			if(w)
			{
				//We connected that in the constructor.  we don't need to keep this slot active.
				disconnect( Kopete::ChatSessionManager::self() , SIGNAL(viewActivated(KopeteView* )) , this, SLOT(slotDisplayPictureChanged()) );
			
				QPtrListIterator<KToolBar>  it=w->toolBarIterator() ;
				KAction *imgAction=actionCollection()->action("msnDisplayPicture");
				if(imgAction)  while(it)
				{
					KToolBar *tb=*it;
					if(imgAction->isPlugged(tb))
					{
						sz=tb->iconSize();
						//ipdate if the size of the toolbar change.
						disconnect(tb, SIGNAL(modechange()), this, SLOT(slotDisplayPictureChanged()));
						connect(tb, SIGNAL(modechange()), this, SLOT(slotDisplayPictureChanged()));
						break;
					}
					++it;
				}
			}
			QString imgURL=c->property(Kopete::Global::Properties::self()->photo()).value().toString();
			QImage scaledImg = QPixmap( imgURL ).convertToImage().smoothScale( sz, sz );
			m_image->setPixmap( scaledImg );
			QToolTip::add( m_image, "<qt><img src=\"" + imgURL + "\"></qt>" );
		}
		else 
		{
			KConfig *config = KGlobal::config();
			config->setGroup( "MSN" );
			if ( config->readBoolEntry( "AutoDownloadPicture", true ) && !c->object().isEmpty() )
				slotRequestPicture();
		}
	}
}

void MSNChatSession::slotDebugRawCommand()
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


void MSNChatSession::receivedTypingMsg( const QString &contactId, bool b )
{
	MSNContact *c = dynamic_cast<MSNContact *>( account()->contacts()[ contactId ] );
	if(c && m_newSession &&  !view(false))
	{
		//this was originaly in MSNAccount::slotCreateChat
		KGlobal::config()->setGroup( "MSN" );
		bool notifyNewChat = KGlobal::config()->readBoolEntry( "NotifyNewChat", false );
		if (  notifyNewChat  )
		{
			// this internal message should open the window if they not exist
			QString body = i18n( "%1 has started a chat with you" ).arg( c->metaContact()->displayName() );
			Kopete::Message tmpMsg = Kopete::Message( c, members(), body, Kopete::Message::Internal, Kopete::Message::PlainText );
			appendMessage( tmpMsg );
		}
	}
	m_newSession=false;
	if(c)
		Kopete::ChatSession::receivedTypingMsg(c,b);
}

#include "msnmessagemanager.moc"

// vim: set noet ts=4 sts=4 sw=4:


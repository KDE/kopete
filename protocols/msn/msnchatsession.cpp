/*
    msnchatsession.cpp - MSN Message Manager

    Copyright (c) 2002-2005 by Olivier Goffart        <ogoffart at kde.org>

    Kopete    (c) 2002-2005 by the Kopete developers  <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/

#include "msnchatsession.h"

#include <QLabel>
#include <QImage>

#include <qfile.h>
#include <qiconset.h>
#include <QPixmap>

#include <kaction.h>
#include <kactionmenu.h>
#include <kconfig.h>
#include <kdebug.h>
#include <kglobal.h>
#include <kinputdialog.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kmenu.h>
#include <ktemporaryfile.h>
#include <kxmlguiwindow.h>
#include <kcomponentdata.h>
#include <kactioncollection.h>
#include <ktoolbar.h>
#include <krun.h>
#include <kicon.h>

#include "kopetecontactaction.h"
#include "kopetemetacontact.h"
#include "kopetecontactlist.h"
#include "kopetechatsessionmanager.h"
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
	Kopete::ContactPtrList others )
: Kopete::ChatSession( user, others, protocol )
{
	Kopete::ChatSessionManager::self()->registerChatSession( this );
	m_chatService = 0l;
	m_timeoutTimer =0L;
	m_newSession = true;
	m_connectionTry=0;

	setComponentData(protocol->componentData());

	connect( this, SIGNAL( messageSent( Kopete::Message&,
		Kopete::ChatSession* ) ),
		this, SLOT( slotMessageSent( Kopete::Message&,
		Kopete::ChatSession* ) ) );

	connect( this, SIGNAL( invitation(MSNInvitation*& ,  const QString & , unsigned long int , MSNChatSession*  , MSNContact*  ) ) ,
		protocol,  SIGNAL( invitation(MSNInvitation*& ,  const QString & , unsigned long int , MSNChatSession*  , MSNContact*  ) ) );


	m_actionInvite = new KActionMenu( KIcon("kontact_contacts"), i18n( "&Invite" ), this );
        actionCollection()->addAction( "msnInvite", m_actionInvite );
	connect ( m_actionInvite->menu() , SIGNAL( aboutToShow() ) , this , SLOT(slotActionInviteAboutToShow() ) ) ;

	#if !defined NDEBUG
	KAction* rawCmd = new KAction( i18n( "Send Raw C&ommand..." ), this );
        actionCollection()->addAction( "msnDebugRawCommand", rawCmd ) ;
	connect( rawCmd, SIGNAL(triggered()), this, SLOT(slotDebugRawCommand()) );
	#endif

	m_actionNudge=new KAction( KIcon("preferences-desktop-notification-bell"), i18n( "Send Nudge" ), this );
        actionCollection()->addAction( "msnSendNudge", m_actionNudge ) ;
	connect( m_actionNudge, SIGNAL(triggered(bool)), this, SLOT(slotSendNudge()) );

	// Invite to receive webcam action
	m_actionWebcamReceive=new KAction( KIcon("webcamreceive"), i18n( "View Contact's Webcam" ), this );
        actionCollection()->addAction( "msnWebcamReceive", m_actionWebcamReceive ) ;
	connect( m_actionWebcamReceive, SIGNAL(triggered(bool)), this, SLOT(slotWebcamReceive()) );

	//Send webcam action
	m_actionWebcamSend=new KAction( KIcon("webcamsend"), i18n( "Send Webcam" ), this );
        actionCollection()->addAction( "msnWebcamSend", m_actionWebcamSend ) ;
	connect( m_actionWebcamSend, SIGNAL(triggered(bool)), this, SLOT(slotWebcamSend()) );

	MSNContact *c = static_cast<MSNContact*>( others.first() );
	KAction* requestPicture = new KAction( KIcon("image-x-generic"), i18n( "Request Display Picture" ), this );
        actionCollection()->addAction( "msnRequestDisplayPicture", requestPicture );
	requestPicture->setEnabled(!c->object().isEmpty());
	connect( requestPicture, SIGNAL(triggered()), this, SLOT(slotRequestPicture()) );

	if ( !c->object().isEmpty() )
	{

		connect( c, SIGNAL( displayPictureChanged() ), this, SLOT( slotDisplayPictureChanged() ) );
		m_image = new QLabel( 0L );
		KAction *imageAction = new KAction( i18n( "MSN Display Picture" ), this );
                actionCollection()->addAction( "msnDisplayPicture", imageAction );
		imageAction->setDefaultWidget( m_image );
		connect( imageAction, SIGNAL( triggered() ), this, SLOT( slotRequestPicture() ) );

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
		m_invitations.erase( it );
	}
	qDeleteAll(m_inviteactions);
}

void MSNChatSession::createChat( const QString &handle,
	const QString &address, const QString &auth, const QString &ID )
{
	/* disabled because i don't want to reopen a chat window if we just closed it
	 * and the contact take much time to type his message
	 m_newSession= !(ID.isEmpty());
	*/

	if( m_chatService )
	{
		kDebug(14140) << "Service already exists, disconnect them.";
		delete m_chatService;
	}

//	uncomment this line if you don't want to the peer know when you close the window
//	setCanBeDeleted( false );

	m_chatService = new MSNSwitchBoardSocket( static_cast<MSNAccount*>( myself()->account() ) , this);
	m_chatService->setUseHttpMethod( static_cast<MSNAccount*>( myself()->account() )->useHttpMethod() );
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

    KConfigGroup *config=account()->configGroup();
	if ( config->readEntry( "SendTypingNotification", true ) )
	{
		connect( this, SIGNAL( myselfTyping( bool ) ),
			m_chatService, SLOT( sendTypingMsg( bool ) ) );
	}

	connect( m_chatService, SIGNAL( msgAcknowledgement(unsigned int, bool) ),
		this, SLOT( slotAcknowledgement(unsigned int, bool) ) );
	connect( m_chatService, SIGNAL( invitation( const QString&, const QString& ) ),
		this, SLOT( slotInvitation( const QString&, const QString& ) ) );
	connect( m_chatService, SIGNAL( nudgeReceived(const QString&) ),
		this, SLOT( slotNudgeReceived(const QString&) ) );
	connect( m_chatService, SIGNAL( errorMessage(int, const QString& ) ), static_cast<MSNAccount *>(myself()->account()), SLOT( slotErrorMessageReceived(int, const QString& ) ) );

	if(!m_timeoutTimer)
	{
		m_timeoutTimer=new QTimer(this);
		connect( m_timeoutTimer , SIGNAL(timeout()), this , SLOT(slotConnectionTimeout() ) );
	}
	m_timeoutTimer->setSingleShot(true);
	m_timeoutTimer->start(20000);
}

void MSNChatSession::slotUserJoined( const QString &handle, const QString &publicName, bool IRO )
{
	delete m_timeoutTimer;
	m_timeoutTimer=0L;

	if( !account()->contacts()[ handle ] )
		account()->addContact( handle, QString(), 0L, Kopete::Account::Temporary);

	MSNContact *c = static_cast<MSNContact*>( account()->contacts()[ handle ] );

	c->setProperty( Kopete::Global::Properties::self()->nickName() , publicName);

	if(c->clientFlags() & MSNProtocol::MSNC4 )
	{
		m_actionNudge->setEnabled(true);
	}

	addContact(c , IRO); // don't show notificaions when we join wesalef
	if(!m_messagesQueue.empty() || !m_invitations.isEmpty())
		sendMessageQueue();

    KConfigGroup *config=account()->configGroup();
	if ( members().count()==1 && config->readEntry( "DownloadPicture", 1 ) >= 1 && !c->object().isEmpty() && !c->hasProperty(Kopete::Global::Properties::self()->photo().key()))
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
	//kDebug(14140) << "MSNChatSession::slotSwitchBoardClosed";
	m_chatService->deleteLater();
	m_chatService=0l;

	cleanMessageQueue( i18n("Connection closed") );

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
			kDebug(14140) << "Message added to the queue";
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
			message.setBackgroundColor(QColor()); // clear the bgColor
			message.setPlainBody(message.plainBody() ); //clear every custom tag which are not sent
			appendMessage(message); // send the own msg to chat window
		}
	}
	else // There's no switchboard available, so we must create a new one!
	{
		startChatSession();
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
		msg.setForegroundColor( QColor( "SlateGray3" ) );
		QFont f;
		f.setItalic( true );
		msg.setFont( f );
	}
	appendMessage( msg );
}

void MSNChatSession::slotActionInviteAboutToShow()
{
	// We can't simply insert  KAction in this menu bebause we don't know when to delete them.
	//  items inserted with insert items are automatically deleted when we call clear

	qDeleteAll(m_inviteactions);
	m_inviteactions.clear();

	m_actionInvite->menu()->clear();


	QHash<QString, Kopete::Contact*> contactList = account()->contacts();
	QHash<QString, Kopete::Contact*>::Iterator it, itEnd = contactList.end();
	for( it = contactList.begin(); it != itEnd; ++it )
	{
		if( !members().contains( it.value() ) && it.value()->isOnline() && it.value() != myself() )
		{
			KAction *a = new Kopete::UI::ContactAction( it.value(), actionCollection() );
			m_actionInvite->addAction( a );
			m_inviteactions.append( a ) ;
		}
	}
	KAction *b = new KAction( i18n ("Other..."), actionCollection() );
        actionCollection()->addAction( "actionOther", b );
	QObject::connect( b, SIGNAL( triggered( bool ) ),
	                  this, SLOT( slotInviteOtherContact() ) );
	m_actionInvite->addAction( b );
	m_inviteactions.append( b ) ;
}

void MSNChatSession::slotCloseSession()
{
	kDebug(14140) << m_chatService;
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
		startChatSession();
}

void MSNChatSession::slotInviteOtherContact()
{
	bool ok;
	QString handle = KInputDialog::getText(i18n( "MSN Plugin" ),
			i18n( "Please enter the email address of the person you want to invite:" ),
			QString(), &ok );
	if( !ok )
		return;

	if( handle.count(QChar('@')) != 1 || handle.count(QChar('.')) <1)
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
		kDebug(14140) << "Service doesn't exist";
		return;
	}
//	kDebug(14140) << "MSNChatSession::sendMessageQueue: " << m_messagesQueue.count();
	QList<Kopete::Message>::Iterator it;
	for ( it = m_messagesQueue.begin(); it!=m_messagesQueue.end(); it = m_messagesQueue.begin() )
	{
		//m_chatService->sendMsg( *it)  ;
		slotMessageSent(*it , this);
		m_messagesQueue.erase(it);
	}


	QMap<unsigned long int, MSNInvitation*>::Iterator mapIt;
	for( mapIt = m_invitations.begin(); mapIt != m_invitations.end() ; ++mapIt)
	{
		if(! (*mapIt)->incoming() && (*mapIt)->state()<MSNInvitation::Invited)
		{
			m_chatService->sendCommand( "MSG" , "N", true, (*mapIt)->invitationHead().toUtf8() );
			(*mapIt)->setState(MSNInvitation::Invited);
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
		QString body = i18n( "The following message has not been sent correctly:\n%1", m.plainBody() );
		Kopete::Message msg = Kopete::Message( m.to().first(), members() );
		msg.setDirection( Kopete::Message::Internal );
		msg.setPlainBody( body );

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
	rx.indexIn(msg);
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
				rx.indexIn(msg);
				QString inviteName = rx.cap( 1 );

				QString body = i18n(
					"%1 has sent an unimplemented invitation which was rejected.\n"
					"The invitation was: %2",
						c->property( Kopete::Global::Properties::self()->nickName()).value().toString(), inviteName );
				Kopete::Message tmpMsg = Kopete::Message( c , members() );
				tmpMsg.setDirection( Kopete::Message::Internal );
				tmpMsg.setPlainBody( body );

				appendMessage(tmpMsg);

				m_chatService->sendCommand( "MSG" , "N", true, MSNInvitation::unimplemented(cookie) );
			}
		}
	}
}

void MSNChatSession::invitationDone(MSNInvitation* MFTS)
{
	kDebug(14140) ;
	m_invitations.remove(MFTS->cookie());
//	MFTS->deleteLater();
	delete MFTS;
	if(!m_chatService && m_invitations.isEmpty())
		setCanBeDeleted(true);
}

void MSNChatSession::sendFile(const QString &fileLocation, const QString &/*fileName*/,
	long unsigned int fileSize)
{
	// TODO create a switchboard to send the file is one is not available.
	if(m_chatService && (*members().begin()))
	{
		m_chatService->PeerDispatcher()->sendFile(fileLocation, (qint64)fileSize, (*members().begin())->contactId());
	}
}

void MSNChatSession::initInvitation(MSNInvitation* invitation)
{
	connect(invitation->object(), SIGNAL( done(MSNInvitation*) ) , this , SLOT( invitationDone(MSNInvitation*) ));
	m_invitations.insert( invitation->cookie() , invitation);

	if(m_chatService)
	{
		m_chatService->sendCommand( "MSG" , "N", true, invitation->invitationHead().toUtf8() );
		invitation->setState(MSNInvitation::Invited);
	}
	else
	{
		startChatSession();
	}
}

void MSNChatSession::slotRequestPicture()
{
	QList<Kopete::Contact*> mb=members();
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
		else if(myself()->onlineStatus().isDefinitelyOnline()  && myself()->onlineStatus().status() != Kopete::OnlineStatus::Invisible )
			startChatSession();
	}
	else
	{ //we already have the picture, just show it.
		KRun::runUrl( KUrl( c->property(Kopete::Global::Properties::self()->photo()).value().toString() ), "image/png" , 0l );
	}

}

void MSNChatSession::slotDisplayPictureChanged()
{
	QList<Kopete::Contact*> mb=members();
	MSNContact *c = static_cast<MSNContact *>( mb.first() );
	if ( c && m_image )
	{
		if(c->hasProperty(Kopete::Global::Properties::self()->photo().key()))
		{
#ifdef __GNUC__
#warning Port or remove this KToolBar hack
#endif
#if 0
			int sz=22;
			// get the size of the toolbar were the aciton is plugged.
			//  if you know a better way to get the toolbar, let me know
			KXmlGuiWindow *w= view(false) ? dynamic_cast<KXmlGuiWindow*>( view(false)->mainWidget()->topLevelWidget() ) : 0L;
			if(w)
			{
				//We connected that in the constructor.  we don't need to keep this slot active.
				disconnect( Kopete::ChatSessionManager::self() , SIGNAL(viewActivated(KopeteView* )) , this, SLOT(slotDisplayPictureChanged()) );


				KAction *imgAction=actionCollection()->action("msnDisplayPicture");
				if(imgAction)
				{
					QList<KToolBar*> toolbarList = w->toolBarList();
					QList<KToolBar*>::Iterator it, itEnd = toolbarList.end();
					for(it = toolbarList.begin(); it != itEnd; ++it)
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
					}
				}

			}

			QString imgURL=c->property(Kopete::Global::Properties::self()->photo()).value().toString();
			QImage scaledImg = QPixmap( imgURL ).toImage().smoothScale( sz, sz );
			if(!scaledImg.isNull())
				m_image->setPixmap( QPixmap::fromImage(scaledImg) );
			else
			{ //the image has maybe not been transfered correctly..  force to download again
				c->removeProperty(Kopete::Global::Properties::self()->photo());
				//slotDisplayPictureChanged(); //don't do that or we might end in a infinite loop
			}
			m_image->setToolTip( "<qt><img src=\"" + imgURL + "\"></qt>" );
#endif
		}
		else
		{
            KConfigGroup *config=account()->configGroup();
			if ( config->readEntry( "DownloadPicture", 1 ) >= 1 && !c->object().isEmpty() )
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
					dlg->addId(), dlg->msg().replace('\n',"\r\n").toUtf8() );
	}
	delete dlg;
#endif
}


void MSNChatSession::receivedTypingMsg( const QString &contactId, bool b )
{
	MSNContact *c = dynamic_cast<MSNContact *>( account()->contacts()[ contactId ] );
	if(c && m_newSession &&  !view(false))
	{
		//this was originally in MSNAccount::slotCreateChat
        KConfigGroup *config=account()->configGroup();
        if (  config->readEntry( "NotifyNewChat", false )  )
		{
			// this internal message should open the window if they not exist
			QString body = i18n( "%1 has started a chat with you", c->metaContact()->displayName() );
			Kopete::Message tmpMsg = Kopete::Message( c, members() );
			tmpMsg.setDirection( Kopete::Message::Internal );
			tmpMsg.setPlainBody( body );
			appendMessage( tmpMsg );
		}
	}
	m_newSession=false;
	if(c)
		Kopete::ChatSession::receivedTypingMsg(c,b);
}

void MSNChatSession::slotSendNudge()
{
	if(m_chatService)
	{
		m_chatService->sendNudge();
		Kopete::Message msg = Kopete::Message( myself(), members() );
		msg.setDirection( Kopete::Message::Outbound );
		msg.setType( Kopete::Message::TypeAction );
		msg.setPlainBody( i18n("has sent a nudge") );

		appendMessage( msg );

	}
}


void MSNChatSession::slotNudgeReceived(const QString& handle)
{
	Kopete::Contact *c = account()->contacts()[ handle ] ;
	if(!c)
		c=members().first();
	Kopete::Message msg = Kopete::Message(c, myself() );
	msg.setDirection( Kopete::Message::Inbound );
	msg.setType( Kopete::Message::TypeAction );
	msg.setPlainBody( i18n("has sent you a nudge") );
	appendMessage( msg );
	// Emit the nudge/buzz notification (configured by user).
	emitNudgeNotification();
}


void MSNChatSession::slotWebcamReceive()
{
	if(m_chatService && !members().isEmpty())
	{
#if 0
		m_chatService->PeerDispatcher()->startWebcam(myself()->contactId() , members().first()->contactId() , true);
#endif
	}
}

void MSNChatSession::slotWebcamSend()
{
	kDebug(14140) ;
	if(m_chatService && !members().isEmpty())
	{
#if 0
		m_chatService->PeerDispatcher()->startWebcam(myself()->contactId() , members().first()->contactId() , false);
#endif
	}
}


void MSNChatSession::slotSendFile()
{
	static_cast<MSNContact *>(members().first())->sendFile();
}

void MSNChatSession::startChatSession()
{
	QList<Kopete::Contact*> mb=members();
	static_cast<MSNAccount*>( account() )->slotStartChatSession( mb.first()->contactId() );

	if(!m_timeoutTimer)
	{
		m_timeoutTimer=new QTimer(this);
		connect( m_timeoutTimer , SIGNAL(timeout()), this , SLOT(slotConnectionTimeout() ) );
	}
	m_timeoutTimer->setSingleShot(true);
	m_timeoutTimer->start(20000);
}


void MSNChatSession::cleanMessageQueue( const QString & reason )
{
	delete m_timeoutTimer;
	m_timeoutTimer=0L;

	uint nb=m_messagesQueue.count()+m_messagesSent.count();
	if(nb==0)
		return;
	else if(nb==1)
	{
		Kopete::Message m;
		if(m_messagesQueue.count() == 1)
			m=m_messagesQueue.first();
		else
			m=m_messagesSent.begin().value();

		QString body=i18n("The following message has not been sent correctly  (%1): \n%2", reason, m.plainBody());
		Kopete::Message msg = Kopete::Message(m.to().first(), members() );
		msg.setDirection( Kopete::Message::Internal );
		msg.setPlainBody( body );
		appendMessage(msg);
	}
	else
	{
		Kopete::Message m;
		QString body=i18n("These messages have not been sent correctly (%1): <br /><ul>", reason);

		QMap<unsigned int, Kopete::Message>::Iterator mapIt;
		for (  mapIt = m_messagesSent.begin(); mapIt!=m_messagesSent.end(); mapIt = m_messagesSent.begin() )
		{
			m=mapIt.value();
			body+= "<li>"+m.escapedBody()+"</li>";
			m_messagesSent.erase(mapIt);
		}
		QList<Kopete::Message>::Iterator messageIt;
		for ( messageIt = m_messagesQueue.begin(); messageIt!=m_messagesQueue.end(); messageIt = m_messagesQueue.begin() )
		{
			m=(*messageIt);
			body+= "<li>"+m.escapedBody()+"</li>";
			m_messagesQueue.erase(messageIt);
		}
		body+="</ul>";
		Kopete::Message msg = Kopete::Message(m.to().first(), members());
		msg.setDirection( Kopete::Message::Internal );
		msg.setHtmlBody( body );
		appendMessage(msg);

	}
	m_messagesQueue.clear();
	m_messagesSent.clear();
	messageSucceeded(); //stop stupid animation
}

void MSNChatSession::slotConnectionTimeout()
{
	m_connectionTry++;
	if(m_chatService)
	{
		disconnect(m_chatService , 0 , this , 0 );
		m_chatService->deleteLater();
		m_chatService=0L;
	}

	if( m_connectionTry > 3 )
	{
		cleanMessageQueue( i18n("Impossible to establish the connection") );
		delete m_timeoutTimer;
		m_timeoutTimer=0L;
		return;
	}
	startChatSession();

}

#include "msnchatsession.moc"

// vim: set noet ts=4 sts=4 sw=4:


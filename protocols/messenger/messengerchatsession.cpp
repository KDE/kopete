/*
    messengerchatsession.cpp - Messenger Message Manager

    Copyright (c) 2007		by Zhang Panyong        <pyzhang8@gmail.com>
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
MessengerChatSession::MessengerChatSession ( MessengerProtocol *protocol, const Kopete::Contact *user,
	Kopete::ContactPtrList others )
: Kopete::ChatSession( user, others, protocol )
{
	Kopete::ChatSessionManager::self()->registerChatSession( this );
	setComponentData(protocol->componentData());

	m_actionInvite = new KActionMenu( KIcon("kontact_contacts"), i18n( "&Invite" ), this );
		actionCollection()->addAction( "messengerInvite", m_actionInvite );
	connect ( m_actionInvite->menu() , SIGNAL( aboutToShow() ) , this , SLOT(slotActionInviteAboutToShow() ) ) ;

	#if !defined NDEBUG
	KAction* rawCmd = new KAction( i18n( "Send Raw C&ommand..." ), this );
		actionCollection()->addAction( "messengerDebugRawCommand", rawCmd ) ;
	connect( rawCmd, SIGNAL(triggered()), this, SLOT(slotDebugRawCommand()) );
	#endif

	m_actionNudge=new KAction( KIcon("bell"), i18n( "Send Nudge" ), this );
		actionCollection()->addAction( "messengerSendNudge", m_actionNudge ) ;
	connect( m_actionNudge, SIGNAL(triggered(bool)), this, SLOT(slotSendNudge()) );

	m_actionSendFile = new KAction( KIcon("attach"), i18n( "Send File" ), this );
		actionCollection()->addAction( "messengerSendFile", sendFileAction );
	connect( sendFileAction, SIGNAL( triggered() ), this, SLOT( slotSendFile() ) );

	// Invite to receive webcam action
	m_actionWebcamReceive=new KAction( KIcon("webcamreceive"), i18n( "View Contact's Webcam" ), this );
		actionCollection()->addAction( "messengerWebcamReceive", m_actionWebcamReceive ) ;
	connect( m_actionWebcamReceive, SIGNAL(triggered(bool)), this, SLOT(slotWebcamReceive()) );

	//Send webcam action
	m_actionWebcamSend=new KAction( KIcon("webcamsend"), i18n( "Send Webcam" ), this );
		actionCollection()->addAction( "messengerWebcamSend", m_actionWebcamSend ) ;
	connect( m_actionWebcamSend, SIGNAL(triggered(bool)), this, SLOT(slotWebcamSend()) );

	MessengerContact *c = static_cast<MessengerContact*>( others.first() );
	KAction* requestPicture = new KAction( KIcon("image"), i18n( "Request Display Picture" ), this );
		actionCollection()->addAction( "messengerRequestDisplayPicture", requestPicture );
	requestPicture->setEnabled(!c->object().isEmpty());
	connect( requestPicture, SIGNAL(triggered()), this, SLOT(slotRequestPicture()) );

	if ( !c->object().isEmpty() )
	{
		connect( c, SIGNAL( displayPictureChanged() ), this, SLOT( slotDisplayPictureChanged() ) );
		m_image = new QLabel( 0L );
		KAction *imageAction = new KAction( i18n( "Messenger Display Picture" ), this );
                actionCollection()->addAction( "msnDisplayPicture", imageAction );
		imageAction->setDefaultWidget( m_image );
		connect( imageAction, SIGNAL( triggered() ), this, SLOT( slotRequestPicture() ) );

		if(c->hasProperty(Kopete::Global::Properties::self()->photo().key())  )
		{
			connect( Kopete::ChatSessionManager::self() , SIGNAL(viewActivated(KopeteView* )) , this, SLOT(slotDisplayPictureChanged()) );
		}
	}
	else
	{
		m_image = 0L;
	}

	setXMLFile("messengerchatui.rc");
	setMayInvite( true );
}

MessengerChatSession::~MessengerChatSession()
{
	delete m_image;
}

void MessengerChatSession::slotActionInviteAboutToShow()
{

}

void MessengerChatSession::slotDebugRawCommand()
{

}

void MessengerChatSession::slotSendNudge()
{
}

void MessengerChatSession::slotWebcamReceive()
{
	account()->client()->
}

void MessengerChatSession::slotWebcamSend()
{
}

void MessengerChatSession::slotRequestPicture()
{

}

#include "messengerchatsession.moc"

// vim: set noet ts=4 sts=4 sw=4:


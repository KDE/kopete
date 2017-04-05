/*
    yahoochatsession.cpp - Yahoo! Message Manager

    Copyright (c) 2005 by André Duffeck        <duffeck@kde.org>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/

#include "yahoochatsession.h"

#include <qlabel.h>
#include <qimage.h>

#include <qfile.h>
#include <qicon.h>
#include <QPixmap>
#include <QList>

#include <kconfig.h>
#include "yahoo_protocol_debug.h"
#include <KLocalizedString>
#include <kmessagebox.h>
#include <QMenu>
#include <ktemporaryfile.h>
#include <kxmlguiwindow.h>
#include <ktoolbar.h>
#include <krun.h>
#include <QIcon>

#include "kopetecontactaction.h"
#include "kopetemetacontact.h"
#include "kopetecontactlist.h"
#include "kopetechatsessionmanager.h"
#include "kopeteuiglobal.h"
#include "kopeteglobal.h"
#include "kopeteview.h"

#include "yahoocontact.h"
#include "yahooaccount.h"
#include <kactioncollection.h>

YahooChatSession::YahooChatSession( Kopete::Protocol *protocol, const Kopete::Contact *user,
	Kopete::ContactPtrList others )
: Kopete::ChatSession( user, others, protocol )
{
    setComponentName(QStringLiteral("yahoo_protocol"), i18n("Kopete"));
	qCDebug(YAHOO_PROTOCOL_LOG) ;
	Kopete::ChatSessionManager::self()->registerChatSession( this );
    //

	// Add Actions
    QAction *buzzAction = new QAction( QIcon::fromTheme(QStringLiteral("bell")), i18n( "Buzz Contact" ), this );
        actionCollection()->addAction( QStringLiteral("yahooBuzz"), buzzAction );
    buzzAction->setShortcut( QKeySequence(QStringLiteral("Ctrl+G")) );
	connect( buzzAction, SIGNAL(triggered(bool)), this, SLOT(slotBuzzContact()) );

    QAction *userInfoAction = new QAction( QIcon::fromTheme(QStringLiteral("help-about")), i18n( "Show User Info" ), this );
        actionCollection()->addAction( QStringLiteral("yahooShowInfo"),  userInfoAction) ;
	connect( userInfoAction, SIGNAL(triggered(bool)), this, SLOT(slotUserInfo()) );

    QAction *receiveWebcamAction = new QAction( QIcon::fromTheme(QStringLiteral("webcamreceive")), i18n( "Request Webcam" ), this );
        actionCollection()->addAction( QStringLiteral("yahooRequestWebcam"),  receiveWebcamAction) ;
	connect( receiveWebcamAction, SIGNAL(triggered(bool)), this, SLOT(slotRequestWebcam()) );

    QAction *sendWebcamAction = new QAction( QIcon::fromTheme(QStringLiteral("webcamsend")), i18n( "Invite to view your Webcam" ), this );
        actionCollection()->addAction( QStringLiteral("yahooSendWebcam"),  sendWebcamAction) ;
	connect( sendWebcamAction, SIGNAL(triggered(bool)), this, SLOT(slotInviteWebcam()) );

	YahooContact *c = static_cast<YahooContact*>( others.first() );
	connect( c, SIGNAL(displayPictureChanged()), this, SLOT(slotDisplayPictureChanged()) );
	m_image = new QLabel( 0L );
	m_image->setObjectName( QStringLiteral("kde toolbar widget") );
    QAction *imageAction = new QAction( i18n( "Yahoo Display Picture" ), this );
        actionCollection()->addAction( QStringLiteral("yahooDisplayPicture"), imageAction );
    //KF5 FIXME imageAction->setDefaultWidget( m_image );
	connect( imageAction, SIGNAL(triggered()), this, SLOT(slotDisplayPictureChanged()) );

	if(c->hasProperty(Kopete::Global::Properties::self()->photo().key())  )
	{
		connect( Kopete::ChatSessionManager::self() , SIGNAL(viewActivated(KopeteView*)) , this, SLOT(slotDisplayPictureChanged()) );
	}
	else
	{
		m_image = nullptr;
	}

	setXMLFile(QStringLiteral("yahooimui.rc"));
}

YahooChatSession::~YahooChatSession()
{
	delete m_image;
}

void YahooChatSession::slotBuzzContact()
{
	qCDebug(YAHOO_PROTOCOL_LOG) ;
	QList<Kopete::Contact*>contacts = members();
	static_cast<YahooContact *>(contacts.first())->buzzContact();
}

void YahooChatSession::slotUserInfo()
{
	qCDebug(YAHOO_PROTOCOL_LOG) ;
	QList<Kopete::Contact*>contacts = members();
	static_cast<YahooContact *>(contacts.first())->slotUserInfo();
}

void YahooChatSession::slotRequestWebcam()
{
	qCDebug(YAHOO_PROTOCOL_LOG) ;
	QList<Kopete::Contact*>contacts = members();
	static_cast<YahooContact *>(contacts.first())->requestWebcam();
}

void YahooChatSession::slotInviteWebcam()
{
	qCDebug(YAHOO_PROTOCOL_LOG) ;
	QList<Kopete::Contact*>contacts = members();
	static_cast<YahooContact *>(contacts.first())->inviteWebcam();
}

void YahooChatSession::slotSendFile()
{
	qCDebug(YAHOO_PROTOCOL_LOG) ;
	QList<Kopete::Contact*>contacts = members();
	static_cast<YahooContact *>(contacts.first())->sendFile();
}

void YahooChatSession::slotDisplayPictureChanged()
{
	qCDebug(YAHOO_PROTOCOL_LOG) ;
	QList<Kopete::Contact*> mb=members();
	YahooContact *c = static_cast<YahooContact *>( mb.first() );
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
				disconnect( Kopete::ChatSessionManager::self() , SIGNAL(viewActivated(KopeteView*)) , this, SLOT(slotDisplayPictureChanged()) );

                QAction *imgAction=actionCollection()->action("yahooDisplayPicture");
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
						++it;
					}
				}
			}

			QString imgURL=c->property(Kopete::Global::Properties::self()->photo()).value().toString();
			QImage scaledImg = QPixmap( imgURL ).toImage().smoothScale( sz, sz );
			if(!scaledImg.isNull())
				m_image->setPixmap( QPixmap(scaledImg) );
			else
			{ //the image has maybe not been transferred correctly..  force to download again
				c->removeProperty(Kopete::Global::Properties::self()->photo());
				//slotDisplayPictureChanged(); //don't do that or we might end in a infinite loop
			}
			m_image->setToolTip( "<qt><img src=\"" + imgURL + "\"></qt>" );
#endif
		}
	}
}


/*
    yahoochatsession.cpp - Yahoo! Message Manager

    Copyright (c) 2005 by André Duffeck        <andre@duffeck.de>

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
#include <qtooltip.h>
#include <qfile.h>
#include <qiconset.h>

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
#include <kiconloader.h>

#include "kopetecontactaction.h"
#include "kopetemetacontact.h"
#include "kopetecontactlist.h"
#include "kopetechatsessionmanager.h"
#include "kopeteuiglobal.h"
#include "kopeteglobal.h"
#include "kopeteview.h"

#include "yahoocontact.h"
#include "yahooaccount.h"

YahooChatSession::YahooChatSession( Kopete::Protocol *protocol, const Kopete::Contact *user,
	Kopete::ContactPtrList others, const char *name )
: Kopete::ChatSession( user, others, protocol,  name )
{
	kdDebug(YAHOO_GEN_DEBUG) << k_funcinfo << endl;
	Kopete::ChatSessionManager::self()->registerChatSession( this );
	setInstance(protocol->instance());

	// Add Actions
	new KAction( i18n( "Buzz Contact" ), QIconSet(BarIcon("bell")), "Ctrl+G", this, SLOT( slotBuzzContact() ), actionCollection(), "yahooBuzz" ) ;
	new KAction( i18n( "Show User Info" ), QIconSet(BarIcon("idea")), 0, this, SLOT( slotUserInfo() ), actionCollection(), "yahooShowInfo" ) ;
	new KAction( i18n( "Request Webcam" ), QIconSet(BarIcon("webcamreceive")), 0, this, SLOT( slotRequestWebcam() ), actionCollection(), "yahooRequestWebcam" ) ;
	new KAction( i18n( "Invite to view your Webcam" ), QIconSet(BarIcon("webcamsend")), 0, this, SLOT( slotInviteWebcam() ), actionCollection(), "yahooSendWebcam" ) ;
	new KAction( i18n( "Send File" ), QIconSet(BarIcon("attach")), 0, this, SLOT( slotSendFile() ), actionCollection(), "yahooSendFile" );

	YahooContact *c = static_cast<YahooContact*>( others.first() );
	connect( c, SIGNAL( displayPictureChanged() ), this, SLOT( slotDisplayPictureChanged() ) );
	m_image = new QLabel( 0L, "kde toolbar widget" );
	new KWidgetAction( m_image, i18n( "Yahoo Display Picture" ), 0, this, SLOT( slotDisplayPictureChanged() ), actionCollection(), "yahooDisplayPicture" );
	if(c->hasProperty(Kopete::Global::Properties::self()->photo().key())  )
	{
		connect( Kopete::ChatSessionManager::self() , SIGNAL(viewActivated(KopeteView* )) , this, SLOT(slotDisplayPictureChanged()) );
	}
	else
	{
		m_image = 0L;
	}

	setXMLFile("yahoochatui.rc");
}

YahooChatSession::~YahooChatSession()
{
	delete m_image;
}

void YahooChatSession::slotBuzzContact()
{
	kdDebug(YAHOO_GEN_DEBUG) << k_funcinfo << endl;
	QPtrList<Kopete::Contact>contacts = members();
	static_cast<YahooContact *>(contacts.first())->buzzContact();
}

void YahooChatSession::slotUserInfo()
{
	kdDebug(YAHOO_GEN_DEBUG) << k_funcinfo << endl;
	QPtrList<Kopete::Contact>contacts = members();
	static_cast<YahooContact *>(contacts.first())->slotUserInfo();
}

void YahooChatSession::slotRequestWebcam()
{
	kdDebug(YAHOO_GEN_DEBUG) << k_funcinfo << endl;
	QPtrList<Kopete::Contact>contacts = members();
	static_cast<YahooContact *>(contacts.first())->requestWebcam();
}

void YahooChatSession::slotInviteWebcam()
{
	kdDebug(YAHOO_GEN_DEBUG) << k_funcinfo << endl;
	QPtrList<Kopete::Contact>contacts = members();
	static_cast<YahooContact *>(contacts.first())->inviteWebcam();
}

void YahooChatSession::slotSendFile()
{
	kdDebug(YAHOO_GEN_DEBUG) << k_funcinfo << endl;
	QPtrList<Kopete::Contact>contacts = members();
	static_cast<YahooContact *>(contacts.first())->sendFile();
}

void YahooChatSession::slotDisplayPictureChanged()
{
	kdDebug(YAHOO_GEN_DEBUG) << k_funcinfo << endl;
	QPtrList<Kopete::Contact> mb=members();
	YahooContact *c = static_cast<YahooContact *>( mb.first() );
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
				KAction *imgAction=actionCollection()->action("yahooDisplayPicture");
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
			if(!scaledImg.isNull())
				m_image->setPixmap( scaledImg );
			else
			{ //the image has maybe not been transfered correctly..  force to download again
				c->removeProperty(Kopete::Global::Properties::self()->photo());
				//slotDisplayPictureChanged(); //don't do that or we might end in a infinite loop
			}
			QToolTip::add( m_image, "<qt><img src=\"" + imgURL + "\"></qt>" );
		}
	}
}

#include "yahoochatsession.moc"

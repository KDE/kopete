/*
    historyguiclient.cpp

    Copyright (c) 2003 by Olivier Goffart        <ogoffart@tiscalinet.be>
    Kopete    (c) 2003 by the Kopete developers  <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/
#include "historyguiclient.h"

#include "historylogger.h"

#include "kopetemessagemanager.h"
#include "kopetecontact.h"
#include "kopeteview.h"

#include <kaction.h>
#include <kconfig.h>
#include <klocale.h>


HistoryGUIClient::HistoryGUIClient(KopeteMessageManager *parent , const char *name)
 : QObject(parent, name) , KXMLGUIClient(parent)
{
	m_manager=parent;
	if(!m_manager || m_manager->members().isEmpty())
		deleteLater(); //we refuse to build this client, it is based on wrong parametters

	QPtrList<KopeteContact> mb=m_manager->members();
	m_logger=new HistoryLogger( mb.first() , this );

	new KAction( i18n("History Last" ), QString::fromLatin1( "finish" ), 0, this, SLOT(slotLast()), actionCollection() , "historyLast" );
	new KAction( i18n("History Previous" ), QString::fromLatin1( "back" ), ALT+SHIFT+Key_Left, this, SLOT(slotPrevious()), actionCollection() , "historyPrevious" );
	new KAction( i18n("History Next" ), QString::fromLatin1( "forward" ), ALT+SHIFT+Key_Right, this, SLOT(slotNext()), actionCollection() , "historyNext");

	//it should be nice to use these std actions, but the shurtcut make conflict with the actual change tab shortcut  ( SHIFT +  left/right)
	//KStdAction::back( this, SLOT(slotPrevious()), actionCollection() , "historyPrevious" );
	//KStdAction::forward( this, SLOT(slotNext()), actionCollection() , "historyNext" );

	setXMLFile("historychatui.rc");

	KGlobal::config()->setGroup("History Plugin");
	m_autoChatWindow=KGlobal::config()->readBoolEntry("Auto chatwindow" , false );
	m_nbAutoChatWindow=KGlobal::config()->readNumEntry( "Number Auto chatwindow" , 7) ;
	m_nbChatWindow=KGlobal::config()->readNumEntry( "Number ChatWindow", 20) ;
}


HistoryGUIClient::~HistoryGUIClient()
{
}


void HistoryGUIClient::slotPrevious()
{
	QPtrList<KopeteContact> mb=m_manager->members();
	/*if(!m_loggers.contains(m_currentMessageManager))
	{
		m_loggers.insert(m_currentMessageManager , new HistoryLogger(mb.first() , m_prefs->historyColor(), this));
		connect( m_currentMessageManager , SIGNAL(closing(KopeteMessageManager*)) , this , SLOT(slotKMMClosed(KopeteMessageManager*)));
	}*/

	KopeteView *m_currentView=m_manager->view(true);
	m_currentView->clear();
	//HistoryLogger *l=m_loggers[m_currentMessageManager];
	m_currentView->appendMessages( m_logger->readMessages(m_nbChatWindow , mb.first() /*FIXME*/ , HistoryLogger::AntiChronological , true));
//	int pos=l->currentPos();
//	if(pos==-1)
//		pos=l->totalMessages();
//	connect(l, SIGNAL( addMessage( KopeteMessage::MessageDirection , QString , QString , QString  ) ) , this , SLOT (addMessage( KopeteMessage::MessageDirection , QString , QString , QString  ) ));
	//l->readLog(pos-m_prefs->nbChatwindow() , m_prefs->nbChatwindow());
//	disconnect(l, SIGNAL( addMessage( KopeteMessage::MessageDirection , QString , QString , QString  ) ) , this , SLOT (addMessage( KopeteMessage::MessageDirection , QString , QString , QString  ) ));


}

void HistoryGUIClient::slotLast()
{
	QPtrList<KopeteContact> mb=m_manager->members();
	/*if(!m_loggers.contains(m_currentMessageManager))
	{
		m_loggers.insert(m_currentMessageManager , new HistoryLogger(mb.first() , m_prefs->historyColor(), this));
		connect( m_currentMessageManager , SIGNAL(closing(KopeteMessageManager*)) , this , SLOT(slotKMMClosed(KopeteMessageManager*)));
	}*/

	KopeteView *m_currentView=m_manager->view(true);
	m_currentView->clear();
	//HistoryLogger *l=m_loggers[m_currentMessageManager];
	m_logger->setPositionToLast();
	m_currentView->appendMessages( m_logger->readMessages(m_nbChatWindow , mb.first() /*FIXME*/ , HistoryLogger::AntiChronological , true ));
/*
	int pos=l->totalMessages();
	connect(l, SIGNAL( addMessage( KopeteMessage::MessageDirection , QString , QString , QString  ) ) , this , SLOT (addMessage( KopeteMessage::MessageDirection , QString , QString , QString  ) ));
	l->readLog(pos-m_prefs->nbChatwindow() , m_prefs->nbChatwindow());
	disconnect(l, SIGNAL( addMessage( KopeteMessage::MessageDirection , QString , QString , QString  ) ) , this , SLOT (addMessage( KopeteMessage::MessageDirection , QString , QString , QString  ) ));*/
}
void HistoryGUIClient::slotNext()
{
	QPtrList<KopeteContact> mb=m_manager->members();
	/*if(!m_loggers.contains(m_currentMessageManager))
	{
		m_loggers.insert(m_currentMessageManager , new HistoryLogger(mb.first() , m_prefs->historyColor(), this));
		connect( m_currentMessageManager , SIGNAL(closing(KopeteMessageManager*)) , this , SLOT(slotKMMClosed(KopeteMessageManager*)));
	}*/
	KopeteView *m_currentView=m_manager->view(true);
	m_currentView->clear();
	//HistoryLogger *l=m_loggers[m_currentMessageManager];
	m_currentView->appendMessages( m_logger->readMessages(m_nbChatWindow , mb.first() /*FIXME*/ , HistoryLogger::Chronological , false));
	/*
	int pos=l->currentPos();
	if(pos==-1)
		pos=l->totalMessages();
	connect(l, SIGNAL( addMessage( KopeteMessage::MessageDirection , QString , QString , QString  ) ) , this , SLOT (addMessage( KopeteMessage::MessageDirection , QString , QString , QString  ) ));
	l->readLog(pos+m_prefs->nbChatwindow() , m_prefs->nbChatwindow());
	disconnect(l, SIGNAL( addMessage( KopeteMessage::MessageDirection , QString , QString , QString  ) ) , this , SLOT (addMessage( KopeteMessage::MessageDirection , QString , QString , QString  ) ));*/
}

#include "historyguiclient.moc"

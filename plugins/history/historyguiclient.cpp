/*
    historyguiclient.cpp

    Copyright (c) 2003-2004 by Olivier Goffart        <ogoffart@tiscalinet.be>
    Kopete    (c) 2003-2004 by the Kopete developers  <kopete-devel@kde.org>

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
#include <kgenericfactory.h>

class HistoryPlugin;


HistoryGUIClient::HistoryGUIClient(KopeteMessageManager *parent , const char *name)
 : QObject(parent, name) , KXMLGUIClient(parent)
{
	setInstance(KGenericFactory<HistoryPlugin>::instance());

	m_manager=parent;
	if(!m_manager || m_manager->members().isEmpty())
		deleteLater(); //we refuse to build this client, it is based on wrong parametters

	QPtrList<KopeteContact> mb=m_manager->members();
	m_logger=new HistoryLogger( mb.first() , this );

	actionLast=new KAction( i18n("History Last" ), QString::fromLatin1( "finish" ), 0, this, SLOT(slotLast()), actionCollection() , "historyLast" );
	actionPrev=new KAction( i18n("History Previous" ), QString::fromLatin1( "back" ), ALT+SHIFT+Key_Left, this, SLOT(slotPrevious()), actionCollection() , "historyPrevious" );
	actionNext=new KAction( i18n("History Next" ), QString::fromLatin1( "forward" ), ALT+SHIFT+Key_Right, this, SLOT(slotNext()), actionCollection() , "historyNext");

	//it should be nice to use these std actions, but the shurtcut make conflict with the actual change tab shortcut  ( SHIFT +  left/right)
	//KStdAction::back( this, SLOT(slotPrevious()), actionCollection() , "historyPrevious" );
	//KStdAction::forward( this, SLOT(slotNext()), actionCollection() , "historyNext" );

	/** we are generaly at last when begining */
	actionPrev->setEnabled(true);
	actionNext->setEnabled(false);
	actionLast->setEnabled(false);

	setXMLFile("historychatui.rc");

	KGlobal::config()->setGroup("History Plugin");
	m_autoChatWindow=KGlobal::config()->readBoolEntry("Auto_chatwindow" , false );
	//m_nbAutoChatWindow=KGlobal::config()->readNumEntry( "Number_Auto_chatwindow" , 7) ;
	m_nbChatWindow=KGlobal::config()->readNumEntry( "Number_ChatWindow", 20) ;
}


HistoryGUIClient::~HistoryGUIClient()
{
}


void HistoryGUIClient::slotPrevious()
{
	KopeteView *m_currentView=m_manager->view(true);
	m_currentView->clear();

	QPtrList<KopeteContact> mb=m_manager->members();
	QValueList<KopeteMessage> msgs= m_logger->readMessages(m_nbChatWindow , mb.first() /*FIXME*/ , HistoryLogger::AntiChronological , true);
	actionPrev->setEnabled(msgs.count() == m_nbChatWindow);
	actionNext->setEnabled(true);
	actionLast->setEnabled(true);

	m_currentView->appendMessages( msgs );
}

void HistoryGUIClient::slotLast()
{

	KopeteView *m_currentView=m_manager->view(true);
	m_currentView->clear();

	QPtrList<KopeteContact> mb=m_manager->members();
	m_logger->setPositionToLast();
	QValueList<KopeteMessage> msgs= m_logger->readMessages(m_nbChatWindow , mb.first() /*FIXME*/ , HistoryLogger::AntiChronological , true);
	actionPrev->setEnabled(true);
	actionNext->setEnabled(false);
	actionLast->setEnabled(false);

	m_currentView->appendMessages( msgs );
}
void HistoryGUIClient::slotNext()
{
	KopeteView *m_currentView=m_manager->view(true);
	m_currentView->clear();

	QPtrList<KopeteContact> mb=m_manager->members();
	QValueList<KopeteMessage> msgs= m_logger->readMessages(m_nbChatWindow , mb.first() /*FIXME*/ , HistoryLogger::Chronological , false);
	actionPrev->setEnabled(true);
	actionNext->setEnabled(msgs.count() == m_nbChatWindow);
	actionLast->setEnabled(msgs.count() == m_nbChatWindow);

	m_currentView->appendMessages( msgs );
}

#include "historyguiclient.moc"

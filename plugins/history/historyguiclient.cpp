
/*
    historyguiclient.cpp

    Copyright (c) 2003-2004 by Olivier Goffart        <ogoffart@kde.org>
    Copyright (c) 2009 by Kaushik Saurabh        <roideuniverse@gmailcom>

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

#include <QtCore/QList>
#include <QtGui/QTextCursor>
#include <QtGui/QTextDocument>

#include <kaction.h>
#include <kstandardaction.h>
#include <klocale.h>
#include <kgenericfactory.h>
#include <kicon.h>
#include <kactioncollection.h>

#include "kopetechatsession.h"
#include "kopetechatsessionmanager.h"
#include "kopetecontact.h"
#include "kopeteview.h"

#include "historylogger.h"
#include "historyconfig.h"
#include "historydialog.h"
#include "historyplugin.h"
#include <kopeteaccount.h>

HistoryGUIClient::HistoryGUIClient ( HistoryPlugin *hPlugin,Kopete::ChatSession *parent )
		: QObject ( parent ), KXMLGUIClient ( parent ) , m_hPlugin(hPlugin) , m_manager(parent)
{
	kDebug() << "  ";
	setComponentData ( KGenericFactory<HistoryPlugin>::componentData() );

	// Refuse to build this client, it is based on wrong parameters
	if ( !m_manager || m_manager->members().isEmpty() )
		deleteLater();

	QList<Kopete::Contact*> mb=m_manager->members();
	Kopete::Contact *con = mb.first();
	
	Akonadi::Collection coll;
	coll = m_hPlugin->getCollection(con->account()->accountId(), con->contactId() );	
	
	m_logger=new HistoryLogger (m_hPlugin, mb.first() , this );

	actionLast = new KAction ( KIcon ( "go-last" ), i18n ( "Latest History" ), this );
	actionCollection()->addAction ( "historyLast", actionLast );
	connect ( actionLast, SIGNAL ( triggered ( bool ) ), this, SLOT ( slotLast() ) );

	actionPrev = KStandardAction::back ( this, SLOT ( slotPrevious() ), this );
	actionCollection()->addAction ( "historyPrevious", actionPrev );

	actionNext = KStandardAction::forward ( this, SLOT ( slotNext() ), this );
	actionCollection()->addAction ( "historyNext", actionNext );

	KAction *viewChatHistory = new KAction( KIcon("view-history"), i18n("View &History" ), this );
	actionCollection()->addAction( "viewChatHistory", viewChatHistory );
	viewChatHistory->setShortcut(KShortcut (Qt::CTRL + Qt::Key_H));

	connect(viewChatHistory, SIGNAL(triggered(bool)), this, SLOT(slotViewHistory()));

	KAction *actionQuote = new KAction ( KIcon ( "go-last" ),i18n ( "Quote Last Message" ), this );
	actionCollection()->addAction ( "historyQuote",actionQuote );
	connect ( actionQuote,SIGNAL ( triggered ( bool ) ),this,SLOT ( slotQuote() ) );

	// we are generally at last when beginning
	actionPrev->setEnabled ( true );
	actionNext->setEnabled ( false );
	actionLast->setEnabled ( false );

	setXMLFile ( "historychatui.rc" );
}


HistoryGUIClient::~HistoryGUIClient()
{
}


void HistoryGUIClient::slotPrevious()
{
	kDebug() <<"\n HistoryGUIClient::Slot previous";
//	KopeteView *m_currentView = m_manager->view ( true );

	QList<Kopete::Contact*> mb = m_manager->members();
	connect(m_logger,SIGNAL(readMessagesDoneSignal(QList<Kopete::Message>)),this,SLOT(slotPrevious2(QList<Kopete::Message>)) );

	m_logger->readMessages (
	                                  HistoryConfig::number_ChatWindow(), /*mb.first()*/ 0L,
	                                  HistoryLogger::AntiChronological, true );
}
void HistoryGUIClient::slotPrevious2(QList<Kopete::Message> msgsx)
{
	disconnect(m_logger,SIGNAL(readMessagesDoneSignal(QList<Kopete::Message>)),this,SLOT(slotPrevious2(QList<Kopete::Message>)) );
	kDebug() <<" ";
	KopeteView *m_currentView = m_manager->view ( true );
	m_currentView->clear();

	QList<Kopete::Message> msgs = msgsx;
	if ( msgs.isEmpty() )
	    kDebug() << "the messages is empty";

	actionPrev->setEnabled ( msgs.count() == HistoryConfig::number_ChatWindow() );
	actionNext->setEnabled ( true );
	actionLast->setEnabled ( true );

	m_currentView->appendMessages ( msgs );
}

void HistoryGUIClient::slotLast()
{
	kDebug() <<" ";
//	KopeteView *m_currentView = m_manager->view ( true );

	QList<Kopete::Contact*> mb = m_manager->members();
	m_logger->setPositionToLast();

	connect(m_logger,SIGNAL(readMessagesDoneSignal(QList<Kopete::Message>)), this, SLOT(slotLast2(QList<Kopete::Message>)) );
	m_logger->readMessages (
	                                  HistoryConfig::number_ChatWindow(), /*mb.first()*/ 0L,
	                                  HistoryLogger::AntiChronological, true, true, true );
					  
	kDebug() << "CALLING LAST WITH Val="<<HistoryConfig::number_ChatWindow();
}
void HistoryGUIClient::slotLast2(QList<Kopete::Message> msgsx)
{
	disconnect(m_logger,SIGNAL(readMessagesDoneSignal(QList<Kopete::Message>)), this, SLOT(slotLast2(QList<Kopete::Message>)) );
	kDebug() <<"slotLast2";
	KopeteView *m_currentView = m_manager->view ( true );
	m_currentView->clear();

	QList<Kopete::Message> msgs = msgsx;
	actionPrev->setEnabled ( true );
	actionNext->setEnabled ( false );
	actionLast->setEnabled ( false );

	m_currentView->appendMessages ( msgs );
}


void HistoryGUIClient::slotNext()
{
	kDebug() <<"\nHistoryGUIClient::slotNext";
//	KopeteView *m_currentView = m_manager->view ( true );

	QList<Kopete::Contact*> mb = m_manager->members();
	connect(m_logger,SIGNAL(readMessagesDoneSignal(QList<Kopete::Message>)), this, SLOT(slotNext2(QList<Kopete::Message>)) );
	m_logger->readMessages (
	                                  HistoryConfig::number_ChatWindow(), /*mb.first()*/ 0L,
	                                  HistoryLogger::Chronological, false );
}
void HistoryGUIClient::slotNext2(QList<Kopete::Message> msgsx )
{	
	kDebug() <<"slotnext2";
	disconnect(m_logger,SIGNAL(readMessagesDoneSignal(QList<Kopete::Message>)), this, SLOT(slotNext2(QList<Kopete::Message>)) );
	KopeteView *m_currentView = m_manager->view ( true );
	m_currentView->clear();
	
	QList<Kopete::Message> msgs = msgsx;
	actionPrev->setEnabled ( true );
	actionNext->setEnabled ( msgs.count() == HistoryConfig::number_ChatWindow() );
	actionLast->setEnabled ( msgs.count() == HistoryConfig::number_ChatWindow() );

	m_currentView->appendMessages ( msgs );
}

void HistoryGUIClient::slotQuote()
{
	kDebug() <<"HistoryGUIClient::slotQuote";
	KopeteView *m_currentView = m_manager->view ( true );

	if ( !m_currentView )
		return;

	m_logger->setPositionToLast();
	connect(m_logger,SIGNAL(readMessagesDoneSignal(QList<Kopete::Message>)), this, SLOT(slotQuote2(QList<Kopete::Message>)) );
	Akonadi:: Collection coll;
	m_logger->readMessages (
	                                  HistoryConfig::number_ChatWindow(), /*mb.first()*/ 0L,
	                                  HistoryLogger::AntiChronological, true ,true, true);
}
void HistoryGUIClient::slotQuote2(QList<Kopete::Message> msgsx)
{
	kDebug() <<"slot quote2";
	disconnect(m_logger,SIGNAL(readMessagesDoneSignal(QList<Kopete::Message>)), this, SLOT(slotQuote2(QList<Kopete::Message>)) );
	QList<Kopete::Message> msgs = msgsx;
	
	Kopete::Message msg = m_manager->view()->currentMessage();
	QString body = msgs.isEmpty() ? "" : msgs.last().plainBody();
	kDebug(14310) << "Quoting last message " << body;

	body = body.replace('\n', "\n> ");
	body.prepend ("> ");
	body.append ("\n");

	msg.setPlainBody ( body );
	m_manager->view()->setCurrentMessage ( msg );
}

void HistoryGUIClient::slotViewHistory()
{
	kDebug() <<"HistoryGUIClient::slotViewHistory";
	// Original Code, but this any segfault if anything in this pipe is NULL - Tejas Dinkar
	//Kopete::MetaContact *m = Kopete::ChatSessionManager::self()->activeView()->msgManager()->members().first()->metaContact();
	
	//The same as above but with some checking
	KopeteView *view= Kopete::ChatSessionManager::self()->activeView();
	if (!view) {
		kDebug()<<"Unable to Get Active View!";
		return;
	}

	Kopete::ChatSession *session = view->msgManager();
	if (!session) {
		kDebug()<<"Unable to Get Active Session!";
		return;
	}

	Kopete::Contact *contact = session->members().first();
	if (!contact) {
		kDebug()<<"Unable to get contact!";
		return;
	}

	Kopete::MetaContact *m = contact->metaContact();

	if(m)
	{
		HistoryDialog* dialog = new HistoryDialog(m_hPlugin, m);
		dialog->setObjectName( QLatin1String("HistoryDialog") );
	}
}

#include "historyguiclient.moc"

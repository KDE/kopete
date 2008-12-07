/*
    historyguiclient.cpp

    Copyright (c) 2003-2004 by Olivier Goffart        <ogoffart@kde.org>

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
#include "historyconfig.h"
#include "historyplugin.h"

#include "kopetechatsession.h"
#include "kopetecontact.h"
#include "kopeteview.h"

#include <kaction.h>
#include <kstandardaction.h>
#include <klocale.h>
#include <kgenericfactory.h>
#include <kicon.h>


#include <QList>
#include <kactioncollection.h>

#include <QTextCursor>
#include <QTextDocument>

HistoryGUIClient::HistoryGUIClient ( Kopete::ChatSession *parent )
		: QObject ( parent ), KXMLGUIClient ( parent )
{
	setComponentData ( KGenericFactory<HistoryPlugin>::componentData() );

	m_manager = parent;

	// Refuse to build this client, it is based on wrong parameters
	if ( !m_manager || m_manager->members().isEmpty() )
		deleteLater();

	QList<Kopete::Contact*> mb=m_manager->members();
	m_logger=new HistoryLogger ( mb.first() , this );

	actionLast = new KAction ( KIcon ( "go-last" ), i18n ( "Latest History" ), this );
	actionCollection()->addAction ( "historyLast", actionLast );
	connect ( actionLast, SIGNAL ( triggered ( bool ) ), this, SLOT ( slotLast() ) );
	actionPrev = KStandardAction::back ( this, SLOT ( slotPrevious() ), this );
	actionCollection()->addAction ( "historyPrevious", actionPrev );
	actionNext = KStandardAction::forward ( this, SLOT ( slotNext() ), this );
	actionCollection()->addAction ( "historyNext", actionNext );

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
	KopeteView *m_currentView = m_manager->view ( true );
	m_currentView->clear();

	QList<Kopete::Contact*> mb = m_manager->members();
	QList<Kopete::Message> msgs = m_logger->readMessages (
	                                  HistoryConfig::number_ChatWindow(), /*mb.first()*/ 0L,
	                                  HistoryLogger::AntiChronological, true );

	actionPrev->setEnabled ( msgs.count() == HistoryConfig::number_ChatWindow() );
	actionNext->setEnabled ( true );
	actionLast->setEnabled ( true );

	m_currentView->appendMessages ( msgs );
}

void HistoryGUIClient::slotLast()
{
	KopeteView *m_currentView = m_manager->view ( true );
	m_currentView->clear();

	QList<Kopete::Contact*> mb = m_manager->members();
	m_logger->setPositionToLast();
	QList<Kopete::Message> msgs = m_logger->readMessages (
	                                  HistoryConfig::number_ChatWindow(), /*mb.first()*/ 0L,
	                                  HistoryLogger::AntiChronological, true );

	actionPrev->setEnabled ( true );
	actionNext->setEnabled ( false );
	actionLast->setEnabled ( false );

	m_currentView->appendMessages ( msgs );
}


void HistoryGUIClient::slotNext()
{
	KopeteView *m_currentView = m_manager->view ( true );
	m_currentView->clear();

	QList<Kopete::Contact*> mb = m_manager->members();
	QList<Kopete::Message> msgs = m_logger->readMessages (
	                                  HistoryConfig::number_ChatWindow(), /*mb.first()*/ 0L,
	                                  HistoryLogger::Chronological, false );

	actionPrev->setEnabled ( true );
	actionNext->setEnabled ( msgs.count() == HistoryConfig::number_ChatWindow() );
	actionLast->setEnabled ( msgs.count() == HistoryConfig::number_ChatWindow() );

	m_currentView->appendMessages ( msgs );
}

void HistoryGUIClient::slotQuote()
{
	KopeteView *m_currentView = m_manager->view ( true );

	if ( !m_currentView )
		return;

	m_logger->setPositionToLast();
	QList<Kopete::Message> msgs = m_logger->readMessages (
	                                  HistoryConfig::number_ChatWindow(), /*mb.first()*/ 0L,
	                                  HistoryLogger::AntiChronological, true );

	Kopete::Message msg = m_manager->view()->currentMessage();
	QString body = msgs.isEmpty() ? "" : msgs.last().plainBody();
	kDebug(14310) << "Quoting last message " << body;

	body = body.replace('\n', "\n> ");
	body.prepend ("> ");
	body.append ("\n");

	msg.setPlainBody ( body );
	m_manager->view()->setCurrentMessage ( msg );
}

#include "historyguiclient.moc"

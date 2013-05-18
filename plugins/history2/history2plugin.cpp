/*
    history2plugin.cpp

    Copyright (c) 2003-2004 by Olivier Goffart        <ogoffart@kde.org>
    Copyright (c) 2003 by Stefan Gehn                 <metz@gehn.net>

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

#include "history2plugin.h"

#include <QtCore/QList>

#include <kgenericfactory.h>
#include <kaboutdata.h>
#include <kaction.h>
#include <kmessagebox.h>
#include <kplugininfo.h>
#include <kdeversion.h>
#include <kicon.h>
#include <kactioncollection.h>

#include "kopetechatsessionmanager.h"
#include "kopetecontact.h"
#include "kopetemetacontact.h"
#include "kopeteview.h"
#include "kopetecontactlist.h"
#include "kopeteuiglobal.h"
#include "kopetemessageevent.h"
#include "kopeteviewplugin.h"

#include "history2dialog.h"
#include "history2logger.h"
#include "history2guiclient.h"
#include "history2config.h"

typedef KGenericFactory<History2Plugin> History2PluginFactory;
static const KAboutData aboutdata("kopete_history2", 0, ki18n("History2") , "1.0" );
K_EXPORT_COMPONENT_FACTORY( kopete_history2, History2PluginFactory( &aboutdata )  )

History2Plugin::History2Plugin( QObject *parent, const QStringList & /* args */ )
: Kopete::Plugin( History2PluginFactory::componentData(), parent ), m_loggerFactory( this )
{
	KAction *viewMetaContactHistory = new KAction( KIcon("view-history"), i18n("View &History" ), this );
	actionCollection()->addAction( "viewMetaContactHistory", viewMetaContactHistory );
	viewMetaContactHistory->setShortcut(KShortcut(Qt::CTRL + Qt::Key_H));
	connect(viewMetaContactHistory, SIGNAL(triggered(bool)), this, SLOT(slotViewHistory()));
	viewMetaContactHistory->setEnabled(
		Kopete::ContactList::self()->selectedMetaContacts().count() == 1 );

	connect(Kopete::ContactList::self(), SIGNAL(metaContactSelected(bool)),
		viewMetaContactHistory, SLOT(setEnabled(bool)));

	connect(Kopete::ChatSessionManager::self(), SIGNAL(viewCreated(KopeteView*)),
		this, SLOT(slotViewCreated(KopeteView*)));

	connect(this, SIGNAL(settingsChanged()), this, SLOT(slotSettingsChanged()));

	setXMLFile("history2ui.rc");

	// Add GUI action to all existing kmm objects
	// (Needed if the plugin is enabled while kopete is already running)
	QList<Kopete::ChatSession*> sessions = Kopete::ChatSessionManager::self()->sessions();
	for (QList<Kopete::ChatSession*>::Iterator it= sessions.begin(); it!=sessions.end() ; ++it)
	{
	  if(!m_loggers.contains(*it))
		{
			m_loggers.insert(*it, new History2GUIClient( *it ) );
			connect( *it, SIGNAL(closing(Kopete::ChatSession*)),
				this, SLOT(slotKMMClosed(Kopete::ChatSession*)));
		}
	}
}


History2Plugin::~History2Plugin()
{
}


void History2MessageLogger::handleMessage( Kopete::MessageEvent *event )
{
	if (history2)
		history2->messageDisplayed( event->message() );

	MessageHandler::handleMessage( event );
}

void History2Plugin::messageDisplayed(const Kopete::Message &m)
{
	if(m.direction()==Kopete::Message::Internal || !m.manager() ||
	   (m.type() == Kopete::Message::TypeFileTransferRequest && m.plainBody().isEmpty()) )
		return;

	if(!m_loggers.contains(m.manager()))
	{
		m_loggers.insert(m.manager() , new History2GUIClient( m.manager() ) );
		connect(m.manager(), SIGNAL(closing(Kopete::ChatSession*)),
			this, SLOT(slotKMMClosed(Kopete::ChatSession*)));
	}

	QList<Kopete::Contact*> mb=m.manager()->members();
	History2Logger::instance()->appendMessage(m, mb.first());

	m_lastmessage=m;
}


void History2Plugin::slotViewHistory()
{
	Kopete::MetaContact *m=Kopete::ContactList::self()->selectedMetaContacts().first();
	if(m)
	{
		//int lines = History2Config::number_ChatWindow();

		// TODO: Keep track of open dialogs and raise instead of
		// opening a new (duplicated) one
		History2Dialog* dialog = new History2Dialog(m);
		dialog->setObjectName( QLatin1String("HistoryDialog") );
	}
}


void History2Plugin::slotViewCreated( KopeteView* v )
{
	if(v->plugin()->pluginInfo().pluginName() != QString::fromLatin1("kopete_chatwindow") )
		return;  //Email chat windows are not supported.

	bool autoChatWindow = History2Config::auto_chatwindow();
	int nbAutoChatWindow = History2Config::number_Auto_chatwindow();

	KopeteView *m_currentView = v;
	Kopete::ChatSession *m_currentChatSession = v->msgManager();

	if(!m_currentChatSession)
		return; //i am sorry

	const Kopete::ContactPtrList& mb = m_currentChatSession->members();

	if(!m_loggers.contains(m_currentChatSession))
	{
		m_loggers.insert(m_currentChatSession , new History2GUIClient( m_currentChatSession ) );
		connect( m_currentChatSession, SIGNAL(closing(Kopete::ChatSession*)),
			this , SLOT(slotKMMClosed(Kopete::ChatSession*)));
	}

	if(!autoChatWindow || nbAutoChatWindow == 0)
		return;

	QList<Kopete::Message> msgs = History2Logger::instance()->readMessages(nbAutoChatWindow, 0,
		mb.first()->metaContact());

	// make sure the last message is not the one which will be appened right
	// after the view is created (and which has just been logged in)
	if(!msgs.isEmpty() && (msgs.last().plainBody() == m_lastmessage.plainBody()) &&
		(m_lastmessage.manager() == m_currentChatSession))
	{
		msgs.takeLast();
	}

	m_currentView->appendMessages( msgs );
}


void History2Plugin::slotKMMClosed( Kopete::ChatSession* kmm)
{
	m_loggers[kmm]->deleteLater();
	m_loggers.remove(kmm);
}

void History2Plugin::slotSettingsChanged()
{
	History2Config::self()->readConfig();
}

#include "history2plugin.moc"

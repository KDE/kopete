/*
    historyplugin.cpp

    Copyright (c) 2003-2004 by Olivier Goffart        <ogoffart@tiscalinet.be>
              (c) 2003 by Stefan Gehn                 <metz AT gehn.net>
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

#include <kgenericfactory.h>
#include <kaboutdata.h>
#include <kaction.h>
#include <kmessagebox.h>
//#include <kconfig.h>
#include <kdeversion.h>

#include "kopetemessagemanagerfactory.h"
#include "kopetemetacontact.h"
#include "kopeteview.h"
#include "kopetecontactlist.h"
#include "kopeteuiglobal.h"
#include "kopetemessageevent.h"

#include "historydialog.h"
#include "historyplugin.h"
#include "historylogger.h"
#include "historyguiclient.h"
#include "historyconfig.h"

typedef KGenericFactory<HistoryPlugin> HistoryPluginFactory;
#if KDE_IS_VERSION(3,2,90)
static const KAboutData aboutdata("kopete_history", I18N_NOOP("History") , "1.0" );
K_EXPORT_COMPONENT_FACTORY( kopete_history, HistoryPluginFactory( &aboutdata )  )
#else
K_EXPORT_COMPONENT_FACTORY( kopete_history, HistoryPluginFactory("kopete_history")  )
#endif

HistoryPlugin::HistoryPlugin( QObject *parent, const char *name, const QStringList & /* args */ )
: Kopete::Plugin( HistoryPluginFactory::instance(), parent, name ), m_loggerFactory( this )
{
	KAction *viewMetaContactHistory = new KAction( i18n("View &History" ),
		QString::fromLatin1( "history" ), 0, this, SLOT(slotViewHistory()),
		actionCollection(), "viewMetaContactHistory" );
	viewMetaContactHistory->setEnabled(
		Kopete::ContactList::self()->selectedMetaContacts().count() == 1 );

	connect(Kopete::ContactList::self(), SIGNAL(metaContactSelected(bool)),
		viewMetaContactHistory, SLOT(setEnabled(bool)));

	connect(Kopete::MessageManagerFactory::self(), SIGNAL(viewCreated(KopeteView*)),
		this, SLOT(slotViewCreated(KopeteView*)));

	connect(this, SIGNAL(settingsChanged()), this, SLOT(slotSettingsChanged()));

	setXMLFile("historyui.rc");
	if(detectOldHistory())
	{
		if(
			KMessageBox::questionYesNo(Kopete::UI::Global::mainWidget(),
				i18n( "Old history files from Kopete 0.6.x or older has been detected.\n"
				"Do you want to import and convert it to the new history format?" ),
				i18n( "History Plugin" ) ) == KMessageBox::Yes )
		{
			convertOldHistory();
		}
	}

	// Add GUI action to all existing kmm objects
	// (Needed if the plugin is enabled while kopete is already running)
	QIntDict<Kopete::MessageManager> sessions = Kopete::MessageManagerFactory::self()->sessions();
	QIntDictIterator<Kopete::MessageManager> it( sessions );
	for ( ; it.current() ; ++it )
	{
		if(!m_loggers.contains(it.current()))
		{
			m_loggers.insert(it.current(), new HistoryGUIClient( it.current() ) );
			connect( it.current(), SIGNAL(closing(Kopete::MessageManager*)),
				this, SLOT(slotKMMClosed(Kopete::MessageManager*)));
		}
	}
}


HistoryPlugin::~HistoryPlugin()
{
}


void HistoryMessageLogger::handleMessage( Kopete::MessageEvent *event )
{
	history->messageDisplayed( event->message() );
	MessageHandler::handleMessage( event );
}

void HistoryPlugin::messageDisplayed(const Kopete::Message &m)
{
	if(m.direction()==Kopete::Message::Internal || !m.manager())
		return;

	if(!m_loggers.contains(m.manager()))
	{
		m_loggers.insert(m.manager() , new HistoryGUIClient( m.manager() ) );
		connect(m.manager(), SIGNAL(closing(Kopete::MessageManager*)),
			this, SLOT(slotKMMClosed(Kopete::MessageManager*)));
	}

	HistoryLogger *l=m_loggers[m.manager()]->logger();
	if(l)
	{
		QPtrList<Kopete::Contact> mb=m.manager()->members();
		l->appendMessage(m,mb.first());
	}

	m_lastmessage=m;
}


void HistoryPlugin::slotViewHistory()
{
	Kopete::MetaContact *m=Kopete::ContactList::self()->selectedMetaContacts().first();
	if(m)
	{
		int lines = HistoryConfig::number_ChatWindow();

		// TODO: Keep track of open dialogs and raise instead of
		// opening a new (duplicated) one
		new HistoryDialog(m, lines);
	}
}


void HistoryPlugin::slotViewCreated( KopeteView* v )
{
//	kdDebug(14310) << k_funcinfo << "called." << endl;
	if (v->viewType() == Kopete::Message::Email)
		return;  //Email chat windows are not supported.

	bool autoChatWindow = HistoryConfig::auto_chatwindow();
	int nbAutoChatWindow = HistoryConfig::number_Auto_chatwindow();
//	kdDebug(14310) << k_funcinfo << "show old chat in chatwindow = " << autoChatWindow << " Number of old lines to show = " << nbAutoChatWindow << endl;

	KopeteView *m_currentView = v;
	Kopete::MessageManager *m_currentMessageManager = v->msgManager();
	QPtrList<Kopete::Contact> mb = m_currentMessageManager->members();

	if(!m_currentMessageManager)
		return; //i am sorry

	if(!m_loggers.contains(m_currentMessageManager))
	{
		m_loggers.insert(m_currentMessageManager , new HistoryGUIClient( m_currentMessageManager ) );
		connect( m_currentMessageManager, SIGNAL(closing(Kopete::MessageManager*)),
			this , SLOT(slotKMMClosed(Kopete::MessageManager*)));
	}

	if(!autoChatWindow || nbAutoChatWindow == 0)
		return;

	HistoryLogger *logger = m_loggers[m_currentMessageManager]->logger();

	logger->setPositionToLast();

	QValueList<Kopete::Message> msgs = logger->readMessages(nbAutoChatWindow,
		mb.first() /*FIXME*/, HistoryLogger::AntiChronological, true, true);

	// make sure the last message is not the one which will be appened right
	// after the view is created (and which has just been logged in)
	if(
		(msgs.last().plainBody() == m_lastmessage.plainBody()) &&
		(m_lastmessage.manager() == m_currentMessageManager))
	{
		msgs.remove(msgs.fromLast());
	}

	m_currentView->appendMessages( msgs );
}


void HistoryPlugin::slotKMMClosed( Kopete::MessageManager* kmm)
{
	m_loggers[kmm]->deleteLater();
	m_loggers.remove(kmm);
}

void HistoryPlugin::slotSettingsChanged()
{
	kdDebug(14310) << k_funcinfo << "RELOADING CONFIG" << endl;
	HistoryConfig::self()->readConfig();
}

#include "historyplugin.moc"

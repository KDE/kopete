/*
    historyplugin.cpp

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

#include <kdebug.h>
#include <kgenericfactory.h>
#include <kaboutdata.h>
#include <kaction.h>
#include <kmessagebox.h>
#include <kconfig.h>
#include <kdeversion.h>

#include "kopetemessagemanagerfactory.h"
#include "kopetemetacontact.h"
#include "kopeteview.h"
#include "kopetecontactlist.h"
#include "kopeteuiglobal.h"

#include "historydialog.h"
#include "historyplugin.h"
#include "historylogger.h"
#include "historyguiclient.h"

typedef KGenericFactory<HistoryPlugin> HistoryPluginFactory;
#if KDE_IS_VERSION(3,2,90)
static const KAboutData aboutdata("kopete_history", I18N_NOOP("History") , "1.0" );
K_EXPORT_COMPONENT_FACTORY( kopete_history, HistoryPluginFactory( &aboutdata )  )
#else
K_EXPORT_COMPONENT_FACTORY( kopete_history, HistoryPluginFactory("kopete_history")  )
#endif

HistoryPlugin::HistoryPlugin( QObject *parent, const char *name, const QStringList & /* args */ )
: KopetePlugin( HistoryPluginFactory::instance(), parent, name )
{
	connect( KopeteMessageManagerFactory::factory(), SIGNAL( aboutToDisplay( KopeteMessage & ) ), this, SLOT( slotMessageDisplayed( KopeteMessage & ) ) );
	connect( KopeteMessageManagerFactory::factory(), SIGNAL( viewCreated( KopeteView* ) ), this, SLOT( slotViewCreated( KopeteView* ) ) );


	KAction *viewMetaContactHistory= new KAction( i18n("View &History" ), QString::fromLatin1( "history" ), 0, this, SLOT(slotViewHistory()), actionCollection() , "viewMetaContactHistory" );
	connect ( KopeteContactList::contactList() , SIGNAL( metaContactSelected(bool)) , viewMetaContactHistory , SLOT(setEnabled(bool)));
	viewMetaContactHistory->setEnabled(KopeteContactList::contactList()->selectedMetaContacts().count()==1 );

	setXMLFile("historyui.rc");

	if(detectOldHistory())
	{
		if( KMessageBox::questionYesNo( Kopete::UI::Global::mainWidget() , i18n( "Old history files from Kopete 0.6.x or older has been detected.\n"
				"Do you want to import and convert it to the new history format?" ) , i18n( "History Plugin" ) ) == KMessageBox::Yes )
		{
			convertOldHistory();
		}
	}

	KConfig *config = KGlobal::config();
	config->setGroup("History Plugin");
	config->writeEntry("Version",  "0.8" );

	//Add GUI action to all already existing kmm (if the plugin is launched when kopete already rining)
	QIntDict<KopeteMessageManager> sessions = KopeteMessageManagerFactory::factory()->sessions();
	QIntDictIterator<KopeteMessageManager> it( sessions );
	for ( ; it.current() ; ++it )
	{
		if(!m_loggers.contains(it.current()))
		{
			m_loggers.insert(it.current() , new HistoryGUIClient( it.current() ) );
			connect( it.current() , SIGNAL(closing(KopeteMessageManager*)) , this , SLOT(slotKMMClosed(KopeteMessageManager*)));
		}
	}
}

HistoryPlugin::~HistoryPlugin()
{
}

void HistoryPlugin::slotMessageDisplayed(KopeteMessage &m)
{
	if(m.direction()==KopeteMessage::Internal)
		return;

	if(!m.manager())
		return; //i am sorry

	if(!m_loggers.contains(m.manager()))
	{
		//m_loggers.insert(m.manager() , new HistoryLogger(mb.first() , m_prefs->historyColor() ,  this));
		m_loggers.insert(m.manager() , new HistoryGUIClient( m.manager() ) );
		connect( m.manager() , SIGNAL(closing(KopeteMessageManager*)) , this , SLOT(slotKMMClosed(KopeteMessageManager*)));
	}

	HistoryLogger *l=m_loggers[m.manager()]->logger();
	if(l)
	{
		QPtrList<KopeteContact> mb=m.manager()->members();
		l->appendMessage(m,mb.first());
	}

	m_lastmessage=m;
}


void HistoryPlugin::slotViewHistory()
{
	KopeteMetaContact *m=KopeteContactList::contactList()->selectedMetaContacts().first();
	if(m)
		new HistoryDialog( m, true , 50 ); //, Kopete::UI::Global::mainWidget(), "KopeteHistoryDialog" );
}

void HistoryPlugin::slotViewCreated( KopeteView* v )
{
	KGlobal::config()->setGroup("History Plugin");
	bool autoChatWindow=KGlobal::config()->readBoolEntry("Auto_chatwindow" , false );
	int nbAutoChatWindow=KGlobal::config()->readNumEntry( "Number_Auto_chatwindow" , 7) ;
//	m_nbChatWindow=KGlobal::config()->readNumEntry( "Number_ChatWindow", 20) ;

	KopeteMessageManager *m_currentMessageManager=v->msgManager();
	QPtrList<KopeteContact> mb=m_currentMessageManager->members();
	KopeteView *m_currentView=v;

	if(!m_currentMessageManager)
		return; //i am sorry

	if(!m_loggers.contains(m_currentMessageManager))
	{
		m_loggers.insert(m_currentMessageManager , new HistoryGUIClient( m_currentMessageManager ) );
		connect( m_currentMessageManager , SIGNAL(closing(KopeteMessageManager*)) , this , SLOT(slotKMMClosed(KopeteMessageManager*)));
	}

	if(!autoChatWindow ||  nbAutoChatWindow == 0)
		return;

	HistoryLogger *l=m_loggers[m_currentMessageManager]->logger();
	l->setPositionToLast();
	QValueList<KopeteMessage> msgs= l->readMessages(nbAutoChatWindow , mb.first() /*FIXME*/ , HistoryLogger::AntiChronological , true );

	// make sure the last message is not the one which will be appened right after the view is created (and which has just been logged in)
	if(msgs.last().plainBody() == m_lastmessage.plainBody() &&  m_lastmessage.manager() == m_currentMessageManager)
		msgs.remove(msgs.fromLast());

	m_currentView->appendMessages( msgs );
}

void HistoryPlugin::slotKMMClosed( KopeteMessageManager* kmm)
{
	m_loggers[kmm]->deleteLater();
	m_loggers.remove(kmm);
}

#include "historyplugin.moc"


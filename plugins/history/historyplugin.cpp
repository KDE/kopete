/*
    historyplugin.cpp

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


#include <kdebug.h>
#include <kgenericfactory.h>
#include <kaction.h>
#include <kmessagebox.h>

#include "kopetemessagemanagerfactory.h"
#include "kopetemetacontact.h"
#include "kopeteview.h"
#include "kopetecontactlist.h"

#include "historydialog.h"
#include "historyplugin.h"
#include "historylogger.h"
#include "historypreferences.h"
#include "historyguiclient.h"



K_EXPORT_COMPONENT_FACTORY( kopete_history, KGenericFactory<HistoryPlugin> );

HistoryPlugin::HistoryPlugin( QObject *parent, const char *name, const QStringList &/*args*/ )
		: KopetePlugin( parent, name )
{
	m_collection=0L;
	connect( KopeteMessageManagerFactory::factory(), SIGNAL( aboutToDisplay( KopeteMessage & ) ), this, SLOT( slotMessageDisplayed( KopeteMessage & ) ) );
	connect( KopeteMessageManagerFactory::factory(), SIGNAL( viewCreated( KopeteView* ) ), this, SLOT( slotViewCreated( KopeteView* ) ) );

	m_prefs=new HistoryPreferences(this);


	KAction *viewMetaContactHistory= new KAction( i18n("View &History" ), QString::fromLatin1( "history" ), 0, this, SLOT(slotViewHistory()), actionCollection() , "viewMetaContactHistory" );
	connect ( KopeteContactList::contactList() , SIGNAL( metaContactSelected(bool)) , viewMetaContactHistory , SLOT(setEnabled(bool)));
	viewMetaContactHistory->setEnabled(KopeteContactList::contactList()->selectedMetaContacts().count()==1 );

	setXMLFile("historyui.rc");

	if(detectOldHistory())
	{
		if( KMessageBox::questionYesNo( 0L , i18n( "Old History files from Kopete 0.6.x or older has been detected\n"
				"Do you want to import and convert it to the new history format?" ) , i18n( "History Plugin" ) ) == KMessageBox::Yes )
		{
			convertOldHistory();
		}
	}

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
		//QPtrList<KopeteContact> mb=m.manager()->members();
		//m_loggers.insert(m.manager() , new HistoryLogger(mb.first() , m_prefs->historyColor() ,  this));
		m_loggers.insert(m.manager() , new HistoryGUIClient( m.manager() ) );
		connect( m.manager() , SIGNAL(closing(KopeteMessageManager*)) , this , SLOT(slotKMMClosed(KopeteMessageManager*)));
	}

	HistoryLogger *l=m_loggers[m.manager()]->logger();
	if(l)
		l->appendMessage(m);



///////////////////////////  SAVE TO KOPETE 0.6 LOGS ////REMOVE!!!!!!!
/*
	if(m.direction()==KopeteMessage::Internal)
		return;

	const KopeteContact *c=(m.direction()==KopeteMessage::Outbound) ? m.to().first() : m.from()  ;
	if(!c)
		return;

	// Replace '.', '/' and '~' in the user id with '-' to avoid possible
	// directory traversal, although appending '.log' and the rest of the
	// code should really make overwriting files possible anyway.
	QString filename = QString::fromLatin1( "kopete/" ) + c->protocol()->pluginId() +
		QString::fromLatin1( "/" ) + c->contactId().replace( QRegExp( QString::fromLatin1( "[./~]" ) ),
		QString::fromLatin1( "-" ) ) + QString::fromLatin1( ".log" );
	//FIXME: problem with group chat.

 // Code from the KopeteLogger in 0.6 ( kopete/libkopete/kopetemessagelog.cpp )
 //  by Ryan Cumming <ryan@kde.org>

	QFileInfo fileInfo(filename);

	QString realFileName = locateLocal( "data", fileInfo.dirPath() + QString::fromLatin1( "/" ) );

	QDir dir;
	if (dir.exists(realFileName) == false)
		dir.mkdir(realFileName);

	realFileName.append(fileInfo.fileName());

//	kdDebug() << k_funcinfo << "Opening log file " << realFileName << endl;

	QFile mLogFile;
	mLogFile.setName(realFileName);
	mLogFile.open(IO_ReadWrite | IO_Append);

	if(mLogFile.status() != IO_Ok)
	{
		kdWarning() << k_funcinfo << "Unable to open message log file" << realFileName << endl;
		return;
	}

	QTextStream stream(&mLogFile);
	stream.setEncoding(QTextStream::UnicodeUTF8);

	// This is a new message
	if (m.direction() == KopeteMessage::Inbound)
	{
		// Incoming message
		stream << "<message direction=\"inbound\">" << endl;
	}
	else if (m.direction() == KopeteMessage::Outbound)
	{
		// Outgoing message
		stream << "<message direction=\"outbound\">" << endl;
	}
	else {
		// Unknown direction (probably shouldn't happen)
		stream << "<message>" << endl;
	}

	if (m.from() && !m.from()->displayName().isNull())
	{
		// Log the source user
		stream << "\t<srcnick>" << QStyleSheet::escape(m.from()->displayName()) << "</srcnick>" << endl;
	}

	const KopeteContact *to = m.to().first();
	if (to && !to->displayName().isNull())
	{
		// Log the destination user
		stream << "\t<destnick>" << QStyleSheet::escape(to->displayName()) << "</destnick>" << endl;
	}

	if (m.timestamp().isValid())
	{
		// Log the time
		stream << "\t<date>" << QStyleSheet::escape(m.timestamp().toString()) << "</date>" << endl;
	}

	// Time for the actual message
	stream << "\t<body>" << endl;
	stream << "\t\t" << QStyleSheet::escape(m.plainBody()) << endl;
	stream << "\t</body>" << endl;

	stream << "</message>" << endl;
	stream << endl;

	// Sync to disk
	stream.device()->flush();
*/
}


void HistoryPlugin::slotViewHistory()
{
	KopeteMetaContact *m=KopeteContactList::contactList()->selectedMetaContacts().first();
	if(m)
		new HistoryDialog( m, true , 50 ); //, qApp->mainWidget(), "KopeteHistoryDialog" );
}

void HistoryPlugin::slotViewCreated( KopeteView* v )
{
	if(m_prefs->nbAutoChatwindow() == 0)
		return;

	KopeteMessageManager *m_currentMessageManager=v->msgManager();
	QPtrList<KopeteContact> mb=m_currentMessageManager->members();
	KopeteView *m_currentView=v;

	if(!m_currentMessageManager)
		return; //i am sorry

	if(!m_loggers.contains(m_currentMessageManager))
	{
		//QPtrList<KopeteContact> mb=m.manager()->members();
		//m_loggers.insert(m.manager() , new HistoryLogger(mb.first() , m_prefs->historyColor() ,  this));
		m_loggers.insert(m_currentMessageManager , new HistoryGUIClient( m_currentMessageManager ) );
		connect( m_currentMessageManager , SIGNAL(closing(KopeteMessageManager*)) , this , SLOT(slotKMMClosed(KopeteMessageManager*)));
	}

	HistoryLogger *l=m_loggers[m_currentMessageManager]->logger();
	l->setPositionToLast();
	m_currentView->appendMessages( l->readMessages(m_prefs->nbAutoChatwindow() , mb.first() /*FIXME*/ , HistoryLogger::AntiChronological , true ));

	/*
	connect(l, SIGNAL( addMessage( KopeteMessage::MessageDirection , QString , QString , QString  ) ) , this , SLOT (addMessage( KopeteMessage::MessageDirection , QString , QString , QString  ) ));
	l->readLog(l->totalMessages()-m_prefs->nbAutoChatwindow() , m_prefs->nbAutoChatwindow());
	disconnect(l, SIGNAL( addMessage( KopeteMessage::MessageDirection , QString , QString , QString  ) ) , this , SLOT (addMessage( KopeteMessage::MessageDirection , QString , QString , QString  ) ));
*/
}

void HistoryPlugin::slotKMMClosed( KopeteMessageManager* kmm)
{
	m_loggers[kmm]->deleteLater();
	m_loggers.remove(kmm);
}

#include "historyplugin.moc"


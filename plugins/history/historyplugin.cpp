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

#include <qregexp.h>
#include <qstylesheet.h>
#include <qdir.h>

#include <kdebug.h>
#include <kgenericfactory.h>
#include <kstandarddirs.h>
#include <kaction.h>

#include "kopetemessagemanagerfactory.h"
#include "kopeteprotocol.h"
#include "kopetemetacontact.h"
#include "historylogger.h"
#include "kopeteview.h"
#include "historypreferences.h"

#include "historydialog.h"
#include "historyplugin.h"



K_EXPORT_COMPONENT_FACTORY( kopete_history, KGenericFactory<HistoryPlugin> );

HistoryPlugin::HistoryPlugin( QObject *parent, const char *name, const QStringList &/*args*/ )
		: KopetePlugin( parent, name )
{
	m_collection=0L;
	connect( KopeteMessageManagerFactory::factory(), SIGNAL( aboutToDisplay( KopeteMessage & ) ), this, SLOT( slotMessageDisplayed( KopeteMessage & ) ) );
	connect( KopeteMessageManagerFactory::factory(), SIGNAL( viewCreated( KopeteView* ) ), this, SLOT( slotViewCreated( KopeteView* ) ) );

	m_prefs=new HistoryPreferences(this);
}

HistoryPlugin::~HistoryPlugin()
{
}

void HistoryPlugin::slotMessageDisplayed(KopeteMessage &m)
{
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

	/* Sync to disk */
	stream.device()->flush();

}

KActionCollection *HistoryPlugin::customContextMenuActions(KopeteMetaContact *m)
{
	delete m_collection;

	m_collection = new KActionCollection(this);

	m_collection->insert(new KAction( i18n("View &History" ), QString::fromLatin1( "history" ), 0, this, SLOT(slotViewHistory()), m_collection ));
	m_currentMetaContact=m;
	return m_collection;
}

KActionCollection *HistoryPlugin::customChatActions(KopeteMessageManager *KMM)
{
	delete m_collection;
	m_collection = new KActionCollection(this);

	m_collection->insert(new KAction( i18n("History Last" ), QString::fromLatin1( "history" ), 0, this, SLOT(slotLast()), m_collection ));
	m_collection->insert(new KAction( i18n("History Previous" ), QString::fromLatin1( "history" ), 0, this, SLOT(slotPrevious()), m_collection ));
	m_collection->insert(new KAction( i18n("History Next" ), QString::fromLatin1( "history" ), 0, this, SLOT(slotNext()), m_collection ));

	m_currentMessageManager=KMM;
	return m_collection;
}



void HistoryPlugin::slotViewHistory()
{
	new HistoryDialog( m_currentMetaContact, true , 50 ); //, qApp->mainWidget(), "KopeteHistoryDialog" );
}

void HistoryPlugin::slotPrevious()
{
	if(!m_loggers.contains(m_currentMessageManager))
	{
		QPtrList<KopeteContact> mb=m_currentMessageManager->members();
		m_loggers.insert(m_currentMessageManager , new HistoryLogger(mb.first() , this));
		connect( m_currentMessageManager , SIGNAL(closing(KopeteMessageManager*)) , this , SLOT(slotKMMClosed(KopeteMessageManager*)));
	}

	m_currentView=m_currentMessageManager->view();
	m_currentView->clear();
	HistoryLogger *l=m_loggers[m_currentMessageManager];
	int pos=l->currentPos();
	if(pos==-1)
		pos=l->totalMessages();
	connect(l, SIGNAL( addMessage( KopeteMessage::MessageDirection , QString , QString , QString  ) ) , this , SLOT (addMessage( KopeteMessage::MessageDirection , QString , QString , QString  ) ));
	l->readLog(pos-m_prefs->nbChatwindow() , m_prefs->nbChatwindow());
	disconnect(l, SIGNAL( addMessage( KopeteMessage::MessageDirection , QString , QString , QString  ) ) , this , SLOT (addMessage( KopeteMessage::MessageDirection , QString , QString , QString  ) ));
}

void HistoryPlugin::slotLast()
{
	if(!m_loggers.contains(m_currentMessageManager))
	{
		QPtrList<KopeteContact> mb=m_currentMessageManager->members();
		m_loggers.insert(m_currentMessageManager , new HistoryLogger(mb.first() , this));
		connect( m_currentMessageManager , SIGNAL(closing(KopeteMessageManager*)) , this , SLOT(slotKMMClosed(KopeteMessageManager*)));
	}

	m_currentView=m_currentMessageManager->view();
	m_currentView->clear();
	HistoryLogger *l=m_loggers[m_currentMessageManager];
	int pos=l->totalMessages();
	connect(l, SIGNAL( addMessage( KopeteMessage::MessageDirection , QString , QString , QString  ) ) , this , SLOT (addMessage( KopeteMessage::MessageDirection , QString , QString , QString  ) ));
	l->readLog(pos-m_prefs->nbChatwindow() , m_prefs->nbChatwindow());
	disconnect(l, SIGNAL( addMessage( KopeteMessage::MessageDirection , QString , QString , QString  ) ) , this , SLOT (addMessage( KopeteMessage::MessageDirection , QString , QString , QString  ) ));
}
void HistoryPlugin::slotNext()
{
	if(!m_loggers.contains(m_currentMessageManager))
	{
		QPtrList<KopeteContact> mb=m_currentMessageManager->members();
		m_loggers.insert(m_currentMessageManager , new HistoryLogger(mb.first() , this));
		connect( m_currentMessageManager , SIGNAL(closing(KopeteMessageManager*)) , this , SLOT(slotKMMClosed(KopeteMessageManager*)));
	}

	m_currentView=m_currentMessageManager->view();
	m_currentView->clear();
	HistoryLogger *l=m_loggers[m_currentMessageManager];
	int pos=l->currentPos();
	if(pos==-1)
		pos=l->totalMessages();
	connect(l, SIGNAL( addMessage( KopeteMessage::MessageDirection , QString , QString , QString  ) ) , this , SLOT (addMessage( KopeteMessage::MessageDirection , QString , QString , QString  ) ));
	l->readLog(pos+m_prefs->nbChatwindow() , m_prefs->nbChatwindow());
	disconnect(l, SIGNAL( addMessage( KopeteMessage::MessageDirection , QString , QString , QString  ) ) , this , SLOT (addMessage( KopeteMessage::MessageDirection , QString , QString , QString  ) ));
}

void HistoryPlugin::addMessage( KopeteMessage::MessageDirection dir, QString /* nick */, QString /* date */, QString body )
{
	//TODO: use nick and date
	QPtrList<KopeteContact> mb=m_currentMessageManager->members();
	KopeteMessage msg( (dir==KopeteMessage::Inbound) ? mb.first() : m_currentMessageManager->user() , m_currentMessageManager->members() , body , dir );
	msg.setFg(m_prefs->historyColor());
	m_currentView->messageReceived(msg);
}

void HistoryPlugin::slotViewCreated( KopeteView* v )
{
	if(m_prefs->nbAutoChatwindow() == 0)
		return;

	m_currentMessageManager=v->msgManager();
	m_currentView=v;

	if(!m_loggers.contains(v->msgManager()))
	{
		QPtrList<KopeteContact> mb=m_currentMessageManager->members();
		m_loggers.insert(v->msgManager() , new HistoryLogger(mb.first() , this));
		connect( v->msgManager() , SIGNAL(closing(KopeteMessageManager*)) , this , SLOT(slotKMMClosed(KopeteMessageManager*)));
	}

	HistoryLogger *l=m_loggers[m_currentMessageManager];
	connect(l, SIGNAL( addMessage( KopeteMessage::MessageDirection , QString , QString , QString  ) ) , this , SLOT (addMessage( KopeteMessage::MessageDirection , QString , QString , QString  ) ));
	l->readLog(l->totalMessages()-m_prefs->nbAutoChatwindow() , m_prefs->nbAutoChatwindow());
	disconnect(l, SIGNAL( addMessage( KopeteMessage::MessageDirection , QString , QString , QString  ) ) , this , SLOT (addMessage( KopeteMessage::MessageDirection , QString , QString , QString  ) ));

}

void HistoryPlugin::slotKMMClosed( KopeteMessageManager* kmm)
{
	delete m_loggers[kmm];
	m_loggers.remove(kmm);
}




#include "historyplugin.moc"




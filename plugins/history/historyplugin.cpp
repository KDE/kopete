/*
    historyplugin.cpp

    Copyright (c) 2003-2004 by Olivier Goffart        <ogoffart@kde.org>
    Copyright (c) 2003 by Stefan Gehn                 <metz@gehn.net>
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

#include "historyplugin.h"

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
#include "kopetemetacontact.h"
#include "kopeteview.h"
#include "kopetecontactlist.h"
#include "kopeteuiglobal.h"
#include "kopetemessageevent.h"
#include "kopeteviewplugin.h"

#include "historydialog.h"
#include "historylogger.h"
#include "historyguiclient.h"
#include "historyconfig.h"

typedef KGenericFactory<HistoryPlugin> HistoryPluginFactory;
static const KAboutData aboutdata("kopete_history", 0, ki18n("History") , "1.0" );
K_EXPORT_COMPONENT_FACTORY( kopete_history, HistoryPluginFactory( &aboutdata )  )

HistoryPlugin::HistoryPlugin( QObject *parent, const QStringList & /* args */ )
        : Kopete::Plugin( HistoryPluginFactory::componentData(), parent ), m_loggerFactory( this )
{
    kDebug(14310) << "\n********History Plugin Constructor\n\n ";

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

    setXMLFile("historyui.rc");
    if (detectOldHistory())
    {
        if (
            KMessageBox::questionYesNo(Kopete::UI::Global::mainWidget(),
                                       i18n( "Old history files from Kopete 0.6.x or older has been detected.\n"
                                             "Do you want to import and convert them to the new history format?" ),
                                       i18n( "History Plugin" ), KGuiItem( i18n("Import && Convert") ), KGuiItem( i18n("Do Not Import") ) ) == KMessageBox::Yes )
        {
            convertOldHistory();
        }
    }

    // Add GUI action to all existing kmm objects KMM == chat session
    // (Needed if the plugin is enabled while kopete is already running)
    QList<Kopete::ChatSession*> sessions = Kopete::ChatSessionManager::self()->sessions();
    for (QList<Kopete::ChatSession*>::Iterator it= sessions.begin(); it!=sessions.end() ; ++it)
    {
        if (!m_loggers.contains(*it))
        {
            kDebug()<<"\n*still in historyplugin constructor before new historyGUIClinet \n\n";
            m_loggers.insert(*it, new HistoryGUIClient( *it ) );
            connect( *it, SIGNAL(closing(Kopete::ChatSession*)),
                     this, SLOT(slotKMMClosed(Kopete::ChatSession*)));
        }
    }
    kDebug(14310)<<"\n****exiting KKKKKKKKKKKKKKKKKKK constructor";
}


HistoryPlugin::~HistoryPlugin()
{
}


void HistoryMessageLogger::handleMessage( Kopete::MessageEvent *event )
{
    kDebug(14310) << "\n***historyplugin.cpp HistoryMessageLogger::handleMessage\n\n ";
    kDebug(14310)<<"\n*from the event a copy of message is passed to message displayed function of historyPlugin\n";
    connect(history,SIGNAL(messageDisplayedDoneSignal()), this, SLOT(handleMessage2()) );
    m_event=event;
    if (history)
        history->messageDisplayed( event->message() );
    
//    MessageHandler::handleMessage( m_event );

}

void HistoryMessageLogger::handleMessage2()
{
    disconnect(history,SIGNAL(messageDisplayedDoneSignal()),this,SLOT(handleMessage2()));
    kDebug()<<"\nBefore *** MessageHnadle::handleMessage(event) executed";
    if (!m_event.isNull() )
      MessageHandler::handleMessage( m_event );
    
    kDebug()<<"\n*** MessageHnadle::handleMessage(event) executed \n";
    
//    m_event= Kopete::MessageEvent::Nothing;
//    disconnect(history,SIGNAL(messageDisplayedDoneSignal()),0,0);
}

void HistoryPlugin::messageDisplayed(const Kopete::Message &m)
{
    kDebug(14310) << "\n\n*** entered the function messageDisplayed";
    if (m.direction()==Kopete::Message::Internal || !m.manager() ||
            (m.type() == Kopete::Message::TypeFileTransferRequest && m.plainBody().isEmpty()) )
        return;

    if (!m_loggers.contains(m.manager()))
    {
        kDebug(14310)<<"\n*** m_loggers(qmap-session-GUIclient) has no manager(==chat session)*so insert it";
        kDebug()<<"\ninsertion of manager means a new session-> a GUIClinet is born";
        m_loggers.insert(m.manager() , new HistoryGUIClient( m.manager() ) );

        kDebug()<<"\nmanager has been inserted\n a GUIclient has been born";

        connect(m.manager(), SIGNAL(closing(Kopete::ChatSession*)),
                this, SLOT(slotKMMClosed(Kopete::ChatSession*)));
    }
    
    m_lastmessage=m;
    
    HistoryLogger *l=m_loggers[m.manager()]->logger();
//    kDebug() << "disconnect(l, SIGNAL(appendMessageDoneSignal()), 0,0)";
//    disconnect(l, SIGNAL(appendMessageDoneSignal()), 0,0) ;
    if (l)
    {

        QList<Kopete::Contact*> mb = m.manager()->members();
	kDebug() <<"\n calling append message";
	connect (l,SIGNAL(appendMessageDoneSignal()), this, SIGNAL(messageDisplayedDoneSignal()) );
        l->appendMessage(m,mb.first());

    }

}


void HistoryPlugin::slotViewHistory()
{
    kDebug() << "HistoryPlugin::slotViewHistory\n\n";
    Kopete::MetaContact *m=Kopete::ContactList::self()->selectedMetaContacts().first();
    if (m)
    {
        //int lines = HistoryConfig::number_ChatWindow();

        // TODO: Keep track of open dialogs and raise instead of
        // opening a new (duplicated) one
        HistoryDialog* dialog = new HistoryDialog(m);
        dialog->setObjectName( QLatin1String("HistoryDialog") );
    }
    else kDebug() << "m not found. will not proceed";
}


void HistoryPlugin::slotViewCreated( KopeteView* v )
{
    kDebug(14310) << "***HistoryPlugin::slotViewCreated\n\n";
    if (v->plugin()->pluginInfo().pluginName() != QString::fromLatin1("kopete_chatwindow") )
        return;  //Email chat windows are not supported.

    bool autoChatWindow = HistoryConfig::auto_chatwindow();
    int nbAutoChatWindow = HistoryConfig::number_Auto_chatwindow();

    KopeteView *m_currentView = v;
    Kopete::ChatSession *m_currentChatSession = v->msgManager();
    
    m_currentChatSessionx=m_currentChatSession;
    m_currentViewx=m_currentView;

    if (!m_currentChatSession)
        return; //i am sorry

    const Kopete::ContactPtrList& mb = m_currentChatSession->members();

    if (!m_loggers.contains(m_currentChatSession))
    {	kDebug() << "m_logger dosent contain current chat session, so inserting";
        m_loggers.insert(m_currentChatSession , new HistoryGUIClient( m_currentChatSession ) );
        connect( m_currentChatSession, SIGNAL(closing(Kopete::ChatSession*)),
                 this , SLOT(slotKMMClosed(Kopete::ChatSession*)));
    }

    if (!autoChatWindow || nbAutoChatWindow == 0)
        return;

    HistoryLogger *logger = m_loggers[m_currentChatSession]->logger();
    m_loggerx=logger;

    logger->setPositionToLast();
    kDebug() << "\ncalling readmessages and connectin with sig";
    connect(logger,SIGNAL(readMessagesDoneSignal()), this,SLOT(slotViewCreated2()));
    logger->readMessages(nbAutoChatWindow,mb.first(), HistoryLogger::AntiChronological, true, true);
    
}
void HistoryPlugin::slotViewCreated2()
{
    disconnect(m_loggerx,SIGNAL(readMessagesDoneSignal()), this,SLOT(slotViewCreated2()));
    kDebug()<<"slot view created2 disconnected";
    QList<Kopete::Message> msgs = m_loggerx->retrunReadMessages();
    // make sure the last message is not the one which will be appened right
    // after the view is created (and which has just been logged in)
    if (!msgs.isEmpty() && (msgs.last().plainBody() == m_lastmessage.plainBody()) &&
            (m_lastmessage.manager() == m_currentChatSessionx))
    {
        msgs.takeLast();
    }

    m_currentViewx->appendMessages( msgs );
}


void HistoryPlugin::slotKMMClosed( Kopete::ChatSession* kmm)
{
    kDebug(14310) << "\n\nHistoryPlugin::slotKMMClosed\n\n";
    m_loggers[kmm]->deleteLater();
    m_loggers.remove(kmm);
}

void HistoryPlugin::slotSettingsChanged()
{
    kDebug() << "\n\nHistoryPlugin::slotSettingsChanged\n\n";

    kDebug(14310) << "RELOADING CONFIG";
    HistoryConfig::self()->readConfig();
}

#include "historyplugin.moc"

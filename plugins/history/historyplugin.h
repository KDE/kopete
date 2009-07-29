/*
    historyplugin.h

    Copyright (c) 2003-2005 by Olivier Goffart       <ogoffart at kde.org>
              (c) 2003 by Stefan Gehn                 <metz AT gehn.net>
	      (c) 2009 by Kaushik Saurabh        <roideuniverse@gmailcom>
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

#ifndef HISTORYPLUGIN_H
#define HISTORYPLUGIN_H

#include <QtCore/QPointer>
#include <QtCore/QMap>

#include <kdebug.h>
#include "kopeteplugin.h"
#include "kopetemessagehandler.h"
#include "historylogger.h"

class KopeteView;
namespace Kopete {
class ChatSession;
}

class HistoryGUIClient;
class HistoryPlugin;

/**
 * @author Richard Smith
 */
class HistoryMessageLogger : public Kopete::MessageHandler
{
    QPointer<HistoryPlugin> history;
    QPointer<Kopete::MessageEvent> m_event;
public:
    HistoryMessageLogger( HistoryPlugin *history ) : history(history)
    {
        kDebug(14310) <<"\n history message logger constructor";
    }
    void handleMessage( Kopete::MessageEvent *event );
//	void handleMessage2();
private:
    Q_OBJECT
signals:
    void handleMessageSignal();
private slots:
    void handleMessage2();
};

class HistoryMessageLoggerFactory : public Kopete::MessageHandlerFactory
{
    HistoryPlugin *history;
public:
    explicit HistoryMessageLoggerFactory( HistoryPlugin *history ) : history(history) {
        qDebug() <<"\n history message logger factory constructor";
    }

    Kopete::MessageHandler *create( Kopete::ChatSession * /*manager*/, Kopete::Message::MessageDirection direction )
    {
        qDebug()<<"\n**** as expected you have ENTERED the Message handle create\n\n";
        if ( direction != Kopete::Message::Inbound )
            return 0;
        qDebug()<<"\n***returning new histor(HistoryMessageLogger)  instance";
        return new HistoryMessageLogger(history);
    }
    int filterPosition( Kopete::ChatSession *, Kopete::Message::MessageDirection )
    {
        qDebug()<<"\n*** filterPosition function entered";
        return Kopete::MessageHandlerFactory::InStageToSent+5;
    }
};

/**
  * @author Olivier Goffart
  */
class HistoryPlugin : public Kopete::Plugin
{
    Q_OBJECT
public:
    HistoryPlugin( QObject *parent, const QStringList &args );
    ~HistoryPlugin();

    /**
     * convert the Kopete 0.6 / 0.5 history to the new format
     */
    static void convertOldHistory();
    /**
     * return true if an old history has been detected, and no new ones
     */
    static bool detectOldHistory();

    void messageDisplayed(const Kopete::Message &msg);

private slots:
    void slotViewCreated( KopeteView* );
    void slotViewCreated2();
    void slotViewHistory();
    void slotKMMClosed( Kopete::ChatSession* );
    void slotSettingsChanged();
    
    void collectionFetch(Akonadi::Collection::List);
    void monitorCollection(Akonadi::Collection);
    void slotJobDone(KJob *);

private:
    HistoryMessageLoggerFactory m_loggerFactory;
    QMap<Kopete::ChatSession*,HistoryGUIClient*> m_loggers;
    Kopete::Message m_lastmessage;
    
    HistoryLogger *m_loggerx;
    Kopete::ChatSession *m_currentChatSessionx;
    KopeteView *m_currentViewx;
    
    Kopete::ChatSession * m_kmm;
    
    Akonadi::Collection m_baseCollection;
    QHash<QString, Akonadi::Collection> m_mapContactCollection;

    void m();
signals:
    void messageDisplayedDoneSignal();

};

#endif



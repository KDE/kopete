/*
    historyplugin.h

    Copyright (c) 2003-2005 by Olivier Goffart       <ogoffart at kde.org>
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

#ifndef HISTORYPLUGIN_H
#define HISTORYPLUGIN_H

#include <QtCore/QPointer>
#include <QtCore/QMap>

#include "kopeteplugin.h"
#include "kopetemessagehandler.h"

class KopeteView;
namespace Kopete { class ChatSession; }

class HistoryGUIClient;
class HistoryPlugin;

/**
 * @author Richard Smith
 */
class HistoryMessageLogger : public Kopete::MessageHandler
{
	QPointer<HistoryPlugin> history;
public:
	HistoryMessageLogger( HistoryPlugin *history ) : history(history) {}
	void handleMessage( Kopete::MessageEvent *event );
};

class HistoryMessageLoggerFactory : public Kopete::MessageHandlerFactory
{
	HistoryPlugin *history;
public:
	explicit HistoryMessageLoggerFactory( HistoryPlugin *history ) : history(history) {}
	Kopete::MessageHandler *create( Kopete::ChatSession * /*manager*/, Kopete::Message::MessageDirection direction )
	{
		if( direction != Kopete::Message::Inbound )
			return 0;
		return new HistoryMessageLogger(history);
	}
	int filterPosition( Kopete::ChatSession *, Kopete::Message::MessageDirection )
	{
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
		void slotViewHistory();
		void slotKMMClosed( Kopete::ChatSession* );
		void slotSettingsChanged();

	private:
		HistoryMessageLoggerFactory m_loggerFactory;
		QMap<Kopete::ChatSession*,HistoryGUIClient*> m_loggers;
		Kopete::Message m_lastmessage;
};

#endif



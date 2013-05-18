/*
    history2plugin.h

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

class History2GUIClient;
class History2Plugin;

/**
 * @author Richard Smith
 */
class History2MessageLogger : public Kopete::MessageHandler
{
	QPointer<History2Plugin> history2;
public:
	History2MessageLogger( History2Plugin *history2 ) : history2(history2) {}
	void handleMessage( Kopete::MessageEvent *event );
};

class History2MessageLoggerFactory : public Kopete::MessageHandlerFactory
{
	History2Plugin *history2;
public:
	explicit History2MessageLoggerFactory( History2Plugin *history2 ) : history2(history2) {}
	Kopete::MessageHandler *create( Kopete::ChatSession * /*manager*/, Kopete::Message::MessageDirection direction )
	{
		if( direction != Kopete::Message::Inbound )
			return 0;
		return new History2MessageLogger(history2);
	}
	int filterPosition( Kopete::ChatSession *, Kopete::Message::MessageDirection )
	{
		return Kopete::MessageHandlerFactory::InStageToSent+5;
	}
};

/**
  * @author Olivier Goffart
  */
class History2Plugin : public Kopete::Plugin
{
	Q_OBJECT
	public:
		History2Plugin( QObject *parent, const QStringList &args );
		~History2Plugin();

		void messageDisplayed(const Kopete::Message &msg);
		
	private slots:
		void slotViewCreated( KopeteView* );
		void slotViewHistory();
		void slotKMMClosed( Kopete::ChatSession* );
		void slotSettingsChanged();

	private:
		History2MessageLoggerFactory m_loggerFactory;
		QMap<Kopete::ChatSession*,History2GUIClient*> m_loggers;
		Kopete::Message m_lastmessage;
};

#endif



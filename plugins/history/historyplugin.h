/*
    historyplugin.h

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

#ifndef HISTORYPLUGIN_H
#define HISTORYPLUGIN_H

#include <qobject.h>
#include <qmap.h>
#include <qstring.h>

#include "kopeteplugin.h"

class KopeteMessage;
class KopeteView;
class KActionCollection;
class KopeteMetaContact;
class KopeteMessageManager;
class HistoryLogger;
class HistoryPreferences;


/**
  * @author Olivier Goffart
  */
class HistoryPlugin : public KopetePlugin
{
	Q_OBJECT
public:
	HistoryPlugin( QObject *parent, const char *name, const QStringList &args );
	~HistoryPlugin();

	virtual KActionCollection *customContextMenuActions(KopeteMetaContact *m);
	virtual KActionCollection *customChatActions(KopeteMessageManager *KMM);


private slots:
	void slotMessageDisplayed(KopeteMessage &msg);
	void slotViewCreated( KopeteView* );
	void slotViewHistory();
	void slotPrevious();
	void slotLast();
	void slotNext();
	void slotKMMClosed( KopeteMessageManager* );

	void addMessage(KopeteMessage::MessageDirection dir, QString nick, QString date, QString body);

private:
	KopeteMetaContact *m_currentMetaContact;
	KActionCollection *m_collection;
	KopeteMessageManager *m_currentMessageManager;
	KopeteView *m_currentView;
	QMap<KopeteMessageManager*,HistoryLogger*> m_loggers;
	HistoryPreferences *m_prefs;
};

#endif



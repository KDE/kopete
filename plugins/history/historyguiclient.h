/*
    historyguiclient.h

    Copyright (c) 2003 by Olivier Goffart        <ogoffart@kde.org>
    Copyright (c) 2009 by Kaushik Saurabh        <roideuniverse@gmailcom>

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
#ifndef HISTORYGUICLIENT_H
#define HISTORYGUICLIENT_H

#include <QtCore/QObject>

#include <kxmlguiclient.h>
#include <Akonadi/Collection>

class KAction;

namespace Kopete { class ChatSession; }

class HistoryLogger;

/**
 *@author Olivier Goffart
 */
class HistoryGUIClient : public QObject , public KXMLGUIClient
{
Q_OBJECT
public:
	HistoryGUIClient(QHash<QString,Akonadi::Collection> &collmap , Kopete::ChatSession *parent = 0);
	~HistoryGUIClient();

	HistoryLogger *logger() const { return m_logger; }

private slots:
	void slotPrevious();
	void slotLast();
	void slotNext();
	void slotQuote();
	void slotViewHistory();

private:
	HistoryLogger *m_logger;
	Kopete::ChatSession *m_manager;
	QHash<QString,Akonadi::Collection> m_collectionmap;
	//bool m_autoChatWindow;
	//int m_nbAutoChatWindow;
	//unsigned int m_nbChatWindow;

	KAction *actionPrev;
	KAction *actionNext;
	KAction *actionLast;
};

#endif

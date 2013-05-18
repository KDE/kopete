/*
    history2guiclient.h

    Copyright (c) 2003 by Olivier Goffart        <ogoffart@kde.org>

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

class KAction;

namespace Kopete { class ChatSession; }

class History2Logger;

/**
 *@author Olivier Goffart
 */
class History2GUIClient : public QObject , public KXMLGUIClient
{
Q_OBJECT
public:
	History2GUIClient(Kopete::ChatSession *parent = 0);
	~History2GUIClient();


private slots:
	void slotPrevious();
	void slotLast();
	void slotNext();
	void slotQuote();
	void slotViewHistory2();

private:

	Kopete::ChatSession *m_manager;
	//bool m_autoChatWindow;
	//int m_nbAutoChatWindow;
	//unsigned int m_nbChatWindow;

	KAction *actionPrev;
	KAction *actionNext;
	KAction *actionLast;

	int offset;
};

#endif

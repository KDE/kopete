/*
    chatmemberslistwidget.h - Chat Members List Widget

    Copyright (c) 2004      by Richard Smith         <kde@metafoo.co.uk>

    Kopete    (c) 2002-2004 by the Kopete developers <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/

#ifndef SIDEBARWIDGET_H
#define SIDEBARWIDGET_H

#include <QDockWidget>

namespace Kopete
{
class ChatSession;
}

class QTabWidget;
class KopeteChatWindow;
class ChatMembersListWidget;
class KHTMLPart;

/**
 * @author Richard Smith <kde@metafoo.co.uk>
 */
class SidebarWidget : public QDockWidget
{
	Q_OBJECT
public:
	SidebarWidget( KopeteChatWindow *parent );
	virtual ~SidebarWidget();

	Kopete::ChatSession *session() { return m_session; }
	
private slots:
	void setChatSession( Kopete::ChatSession *session );
private:
	// Stores the current session the sidebar is using
	Kopete::ChatSession *m_session;
	KopeteChatWindow *m_chatWindow;

	
	// UI
	QTabWidget *m_tabWidget;
	// TabWidget pages :
	KHTMLPart *m_htmlInfoZone;
	ChatMembersListWidget *m_memberslist;
};


#endif

// vim: set noet ts=4 sts=4 sw=4:

